#include <io/Inet6Address.hpp>

#include <algorithm>

using namespace std;

namespace io {

Inet6Address Inet6Address::fromString( const char* ipv6, u16_t port ) {
    u8_t nil[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    Inet6Address ret(nil, port);

    if (inet_pton(AF_INET6, ipv6, &ret.m_addr.sin6_addr) == 1)
        return ret;

    throw Inet6ParseException("Failed to parse ipv6 address");  
}

Inet6Address::Inet6Address(u8_t addr[16], u16_t port) {
    fill( (byte*)&m_addr, (byte*)&m_addr + sizeof(m_addr), 0 );
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = port;
    copy( addr, addr + 16, m_addr.sin6_addr.s6_addr );
}

}