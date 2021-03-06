#include <io/ICMPSocket.hpp>
#include <test.h>
#include <resolv.h>
#include <netdb.h>

#include <unistd.h>
#include <string>
#include <cstring>

#include <io/Inet4Address.hpp>
#include <log/LogManager.hpp>

using namespace io;
using namespace std;
using namespace logger;

const char* host;

int runping( ICMPSocket& sock ) {
    sock.setTimeout(500 MILLIS);

	LogManager::instance().setEnableByDefault(true);
	LogContext& log = LogManager::instance().getLogContext("Ping","RunPing");

    string data( "Hello, World!" );
    byte mesg[128 - ICMPHEADER_SIZE];
    fill(mesg, mesg + sizeof(mesg), 0);
    copy(data.c_str(), data.c_str() + data.size(), mesg);

    size_t count = 0;

    LOG("Attempting to ping %s\n", host);

    ICMPPacket pckt;
	ICMPHeader hdr = ICMPHeader::Echo(5, count++);

    pckt.setHeader(hdr);
    pckt.setMessage(mesg, sizeof(mesg));

    ICMPPacket pkt;
    Inet4Address to_addr("127.0.0.1", 0);
    try {
        to_addr = Inet4Address::fromString(host, 0);
    } catch ( InetParseException& err ) {
        LOG("Unable to parse %s. Defaulting to 127.0.0.1", host);
    }


	try {
        TEST_BOOL( "SocketSend", sock.send(pckt, to_addr) > 0 );
        uptr<SocketAddress> r_addr;

        TEST_EQ_INT( "SocketReceive", sock.receive(pkt, r_addr.cleanref()), 0 );

        log.printfln(DEBUG, "Message");
        log.printHex(DEBUG, pkt.getMessage(), pkt.getMessgaeLength());
        TEST_EQ_INT( "PingMessage", strcmp((const char*)pkt.getMessage(), "Hello, World!"), 0 );

        try {
            sock.setTimeout(1 MICROS);
            sock.receive(pkt, r_addr.cleanref());
            TEST_BOOL( "TestTimeout",  false );
        } catch (Exception& e) {
            TEST_BOOL( "TestTimeout", true );
        }
	} catch ( Exception& e ) {
		log.printfln(ERROR, "Exception caught: %s", e.getMessage());
		return 1;
	}

    return 0;
}

int main(int argc, char** argv) {
    ICMPSocket sock;

    if ( geteuid() != 0 ) {
        printf("Need to be run as root\n");
        return 0;
    }

    if ( argc > 1 ) {
        host = argv[1];
    } else {
        host = "8.8.8.8";
    }

    TEST_BOOL( "ICMPSocket::init", sock.init() >= 0 );
    TEST_EQ_INT( "setNonBlocking", sock.setNonBlocking(), 0 );
    TEST_EQ_INT( "setBlocking", sock.setNonBlocking(false), 0 );

    TEST_FN( runping( sock ) );
}
