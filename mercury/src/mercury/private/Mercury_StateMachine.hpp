#ifndef MERCURY_STATEMACHINE_HPP_
#define MERCURY_STATEMACHINE_HPP_

/*
 * Author: jrahm
 * created: 2015/03/26
 * Mercury_StateMachine.hpp: <description>
 */

#include <log/LogManager.hpp>
#include <curl/AsyncCurl.hpp>

#include <io/binary/Base64Putter.hpp>
#include <io/binary/GlobPutter.hpp>

#include <proc/StateMachine.hpp>
#include "Mercury_JSON.hpp"

#include <mercury/dns/Test.hpp>
#include <mercury/Mercury.hpp>


using namespace logger;
using namespace curl;
using namespace std;
using namespace io;

namespace mercury {

enum State {
    IDLE,
    REQUEST_MADE,
    TIMEOUT,
    DNS_TEST_RUNNING
};

enum Stim {
    START,
    BAD_REQUEST,
    GOOD_REQUEST,
    WAIT_TIMEOUT,
    DNS_TEST_FINISHED
};

ENUM_TO_STRING(State, 4,
    "Idle", "RequestMade",
    "Timeout", "DnsTestStarted")

ENUM_TO_STRING(Stim, 5,
    "Start",
    "BadRequest", "GoodRequest",
    "WaitTimeout", "DnsTestFinished")

#define ID_SIZE 32

inline string html_escape(const string& in) {
    string::const_iterator itr;
    string out;

    for( itr = in.begin() ; itr != in.end() ; ++ itr ) {
        if( *itr == '+' ) {
            out += "%2b";
        } else {
            out.push_back( *itr );
        }
    }

    return out;
}

class Mercury;
class MercuryObserver: public CurlObserver, public dns::TestObserver {
    /* Interface */
};

void setup_curl(Curl& c, const char* url, const char* post_data) {
    c.setURL(url);
    c.setPostFields(post_data);
    c.setFollowLocation(true);
    c.setSSLHostVerifyEnabled(false);
    c.setSSLPeerVerifyEnabled(false);
}

class Mercury_StateMachine {
public:

Mercury_StateMachine(MercuryObserver* observer):
    m_log(LogManager::instance().getLogContext("Mercury", "StateMachine")) {
    m_observer = observer;
}

State onStart() {
    m_log.printfln(INFO, "Starting mercury");
    /* The if of this router */
    string id_enc = simpleBase64Encode(m_config.mercury_id, ID_SIZE);
    id_enc = html_escape(id_enc);
    m_log.printfln(DEBUG, "base64 encoded id: %s", id_enc.c_str());

    Curl request;

    m_post_fields = string("router_token=") + id_enc;
    m_current_url = m_config.controller_url + "/api/router/get_config";
    setup_curl(request, m_current_url.c_str(), m_post_fields.c_str());

    m_log.printfln(INFO, "sending curl request with data %s", m_post_fields.c_str());
    m_async_curl.sendRequest(request, m_observer);

    return REQUEST_MADE;
}

State onDnsComplete() {
    exit(0);
    return IDLE;
}

State onGoodRequest() {
    m_log.printfln(SUCCESS, "Good request made. Returned string %s", m_response);
    /* do the json stuff */

    MercuryTestConfig conf;
    bool fail;

    try {
        conf = parseMercuryTestConfig((char*)m_response, fail);
    } catch(json::JsonException& ex) {
        m_log.printfln(ERROR, "Error parsing JSON request: %s", ex.getMessage());
        return IDLE;
    }

    if(fail) {
        m_log.printfln(ERROR, "Server returned failure status. Timeout.");
        m_state_machine->setTimeoutStim(5 SECS, WAIT_TIMEOUT);
        return TIMEOUT;
    }

    m_log.printfln(SUCCESS, "Good configuration response");

    /* build the configuration and start the
     * test */
    dns::Test::instance().startTest(conf.dns_test_config, m_observer);

    /* Start the ping test */
    return DNS_TEST_RUNNING;
}

State onBadRequest() {
    m_log.printfln(WARN, "Bad request made.");
    m_state_machine->setTimeoutStim(5 SECS, WAIT_TIMEOUT);
    return TIMEOUT;
}

State onWaitTimeoutComplete() {
    m_log.printfln(INFO, "Timeout complete");
    exit(1);
    return IDLE;
}

State onPingTestFinished() {
    m_log.printfln(INFO, "Ping test complete. Sending results");
    /* TODO: Send results */
    return IDLE;
}
    

private:

/* these two classes really operate as the
 * same class, but for clarity are broken into
 * separate ones. */
friend class Mercury;

/* This will be an instance of mercury and will
 * stimulate the state machine */
MercuryObserver* m_observer;

/* The log for this state machine */
LogContext& m_log;

AsyncCurl m_async_curl;

ssize_t m_response_len;
byte* m_response;
StateMachine<Stim, Mercury_StateMachine, State>* m_state_machine;

std::string m_current_url; /* For garbage collection */
std::string m_post_fields;
Config m_config;
};

}

#endif /* MERCURY_STATEMACHINE_HPP_ */
