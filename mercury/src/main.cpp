#include <cstdio>

#include <io/FilePointer.hpp>
#include <io/Inet4Address.hpp>
#include <io/Resolv.hpp>
#include <io/ServerSocket.hpp>
#include <io/Socket.hpp>
#include <io/StringWriter.hpp>
#include <io/binary/StreamGetter.hpp>

#include <json/Json.hpp>
#include <json/JsonMercuryTempl.hpp>

#include <log/LogManager.hpp>
#include <log/LogServer.hpp>

#include <mercury/Mercury.hpp>

#include <os/Condition.hpp>

#include <sys/wait.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

using namespace logger ;
using namespace std ;
using namespace io ;
using namespace os ;
using namespace json ;

byte mercury_magic_cookie[32] = {
    0xe2, 0x1a, 0x14, 0xc8, 0xa2, 0x35, 0x0a, 0x92,
    0xaf, 0x1a, 0xff, 0xd6, 0x35, 0x2b, 0xa4, 0xf3, 
    0x97, 0x79, 0xaf, 0xb5, 0xc1, 0x23, 0x43, 0xf0,
    0xf7, 0x14, 0x17, 0x62, 0x53, 0x4a, 0xa9, 0x7e, 
};

Mutex* g_mutex;
Condition* g_cond;

class MercuryConfig {
public:
    MercuryConfig():
        logEverything(false),
        controller_url("http://127.0.0.1:5000/"),
        bind_ip(new Inet4Address("127.0.0.1", 8639)),
        log_out(NULL),
        colors(true),
        default_level(NULL),
        log_server(NULL)
        {}

    bool logEverything;
    std::string controller_url;
    SocketAddress* bind_ip;
    BaseIO* log_out;
    bool colors;
    LogLevel* default_level;
    
    SocketAddress* log_server;
};

template<>
struct JsonBasicConvert<MercuryConfig> {
    static MercuryConfig convert(const json::Json& jsn) {
        MercuryConfig ret;
        ret.logEverything = jsn.hasAttribute("logEverything") && jsn["logEverything"] != 0;
        SocketAddress* old;

        if(jsn.hasAttribute("controller_url")) {
            ret.controller_url = jsn["controller_url"].stringValue();
        }
        if(jsn.hasAttribute("bind_ip")) {
            old = ret.bind_ip;
            ret.bind_ip = jsn["bind_ip"].convert<SocketAddress*>();
            delete old;
        }

        if(jsn.hasAttribute("logFile")) {
            FILE* f = fopen(jsn["logFile"].stringValue().c_str(), "w");
            if(f) {
                io::FilePointer* fp = new io::FilePointer(f);
                ret.log_out = fp;
            } else {
                fprintf(stderr, "Unable to open log file. Logging to stdout\n");
            }
        }

        if(jsn.hasAttribute("defaultLogLevel")) {
            ret.default_level = LogLevel::getLogLevelByName(jsn["defaultLogLevel"].stringValue().c_str());
        }

        if(jsn.hasAttribute("logColors")) {
            ret.colors = jsn["logColors"] != 0;
        }

        if(jsn.hasAttribute("logServerBindAddress")) {
            ret.log_server = jsn["logServerBindAddress"].convert<SocketAddress*>();
            if(ret.log_server->linkProtocol() == AF_UNIX) {
                dynamic_cast<UnixAddress*>(ret.log_server)->unlink();
            }
        }

        return ret;
    }
};

void sigchld_hdlr(int sig) {
    ScopedLock __sl(*g_mutex);
    g_cond->signal();
}

void start_logging_service(int fd) {
    /* start a logging service which will read on
     * fd and wait for input */
    pid_t child;
    if(!(child = fork())) {
        LogContext::unlock();

        LogContext& log = LogManager::instance().getLogContext("Child", "StdErr");
        log.printfln(DEBUG, "Error logger started");
        FILE* f = fdopen(fd, "r");
        char read[1024];
        read[1023] = 0;
        while(true) {
            fgets(read, sizeof(read)-1, f);
            log.printfln(ERROR, "%s", read);
        }
    }

    if(child < 0) {
        perror("fork() failed");
    }
}

int startMercury(LogContext& m_log, MercuryConfig& config) {
    StreamServerSocket sock;
    SocketAddress* bind_addr = config.bind_ip;

    int error_pipe[2];
    if(pipe(error_pipe)) {
        perror("Error on pipe()\n");
        return 1;
    }

    /* Redirect stderr to the error_pipe. This will make it
     * so we can still see stderr messages in the log */
    dup2(error_pipe[1], STDERR_FILENO);

    /* Start the service that is only resposible for
     * logging the stderr of the this process and
     * its other children */
    start_logging_service(error_pipe[0]);

    m_log.printfln(INFO, "Binding to address: %s", bind_addr->toString().c_str());
    sock.bind(*bind_addr);
    sock.listen(1);

    while(true) {
        StreamSocket* client = sock.accept();
        m_log.printfln(INFO, "Connection accepted");

        /* simply get 64 bytes from the client and
         * then close the connection */
        byte recieve[64];
        client->read(recieve, sizeof(recieve));
        delete client;

        /* make sure the cookie is equal to what we expect */
        if(std::equal(recieve, recieve + 32, mercury_magic_cookie)) {
            /* the cookie is equal to what we expect
             * so continue with the fork() */
            m_log.printfln(INFO, "Magic cookie accepted, forking new process");
            pid_t child;

            if(!(child = fork())) {
                LogContext::unlock();

                /* report all stderr to the error pipe */
                fprintf(stderr, "Test stderr\n");
                fflush(stderr);
                /* Child process. Setup the config and boot
                 * the merucry state machine */
                mercury::Config conf;
                std::copy(recieve + 32, recieve + 64, conf.mercury_id);
                conf.controller_url = config.controller_url;
                mercury::Proxy& process = mercury::ProxyHolder::instance();

                /* start mercury */
                process.start(conf, NULL);
                process.waitForExit();

                exit(0);
            } else {

                /* parent */
                m_log.printfln(INFO, "Child started pid=%d", child);
                int res = 0;
                pid_t pid;

                /* Wait for the child to exit. this
                 * condition will be signaled by the
                 * sigchld handler */
                if(!g_cond->timedwait(*g_mutex, 300 SECS)) {
                    /* The child is taking too long to return
                     * kill it */
                    m_log.printfln(WARN, "Child timeout, sending SIGTERM");
                    kill(child, SIGTERM);
                    if(!g_cond->timedwait(*g_mutex, 10 SECS)) {
                        /* The child still isn't dead */
                        m_log.printfln(WARN, "Child hard timeout, sending SIGKILL");
                        kill(child, SIGKILL);
                    }
                }

                if((pid = waitpid(child, &res, WNOHANG))) {
                    if(pid == -1) {
                        m_log.printfln(ERROR, "Error in waitpid %s", strerror(errno));
                    } else {
                        m_log.printfln(INFO, "Reap child (%d)\n", (int)pid);
                    }
                }

                if(WIFSIGNALED(res)) {
                    m_log.printfln(WARN, "Child was signaled with %d", WTERMSIG(res));
                }

                if(WEXITSTATUS(res)) {
                    m_log.printfln(WARN, "Child returned abnormal status: %d", WEXITSTATUS(res));
                }

                if(WIFSTOPPED(res)) {
                    m_log.printfln(WARN, "Child timed out and was killed by parent");
                }
            }
        } else {
            m_log.printfln(ERROR, "Bad magic cookie received. Timeout for 5 seconds");
            sleep(5);
            m_log.printfln(INFO, "Timeout complete");
        }
    }

    return 0;
}

int main( int argc, char** argv ) {
    (void) argc ; (void) argv ;

    g_mutex = new Mutex();
    g_cond = new Condition();

    ScopedLock __sl(*g_mutex);

    MercuryConfig conf;
    SocketAddress* old = conf.bind_ip;
    signal(SIGCHLD, sigchld_hdlr);

    try {
        Json* jsn = Json::fromFile("config.json");
        conf = jsn->convert<MercuryConfig>();
        delete old;
    } catch(Exception& exp) {
        fprintf(stderr, "Unable to load config file: %s\n", exp.getMessage());
    };

    if(conf.log_out) {
        LogManager::instance().addLogOutput(conf.log_out, conf.colors);
    }

    if(conf.logEverything) {
        LogManager::instance().logEverything();
    }

    if(conf.default_level) {
        LogManager::instance().setDefaultLevel(*conf.default_level);
    }

    try {
        if(conf.log_server) {
            LogServer* log_server = new LogServer(*conf.log_server);
            Thread* __th = new Thread(*log_server);
            __th->start();
        }
    
    
        LogContext& m_log = LogManager::instance().getLogContext("Main", "Main");
        m_log.printfln(INFO, "Mercury started");
        return startMercury(m_log, conf);
    } catch(Exception& exp) {
        LogContext& m_log = LogManager::instance().getLogContext("Main", "Main");
        m_log.printfln(FATAL, "Uncaught exception: %s", exp.getMessage());
        return 127;
    }
}
