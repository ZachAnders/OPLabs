#ifndef PUTTER_HPP_
#define PUTTER_HPP_

/*
 * Author: jrahm
 * created: 2014/11/09
 * Putter.hpp: <description>
 */

#include <types.h>
#include <vector>

#include <algorithm>
#include <string>

namespace io {

ABSTRACT class Putter {
public:
    virtual int putByte( byte b ) = 0;

    virtual inline int putInt16le( uint16_t u ) {
        return putByte( u & 0xFF ) ||
		       putByte( (u>>8) & 0xFF );
    }

    virtual inline int putInt16be( uint16_t u ) {
        return putByte( (u>>8) & 0xFF ) ||
		       putByte( u & 0xFF );
    }

    virtual inline int putInt32be( uint32_t u ) {
        return putInt16be( (u>>16) & 0xFFFF ) ||
		       putInt16be( u & 0xFFFF );
    }
    
    virtual inline int putInt32le( uint32_t u ) {
        return putInt16le( u & 0xFFFF ) ||
		       putInt16le( (u>>16) & 0xFFFF );
    }

    virtual inline int putInt64be( uint64_t u ) {
        return putInt32be( (u>>32) & 0xFFFFFFFF ) ||
		       putInt32be( u & 0xFFFFFFFF );
    }
    
    virtual inline int putInt64le( uint64_t u ) {
        return putInt32le( u & 0xFFFFFFFFL ) ||
		       putInt32le( (u>>32) & 0xFFFFFFFFL );
    }

    virtual int putBytes( const byte* in, size_t len );

    virtual inline ~Putter(){}
};

}

template <class T>
int putObject( io::Putter& putter, const T& v ) ;

inline int putObject( io::Putter& putter, const std::string& s ) {
    putter.putInt32le( s.size() );
    putter.putBytes( (const byte*) s.c_str(), s.size() );
    return 0;
}

#endif /* PUTTER_HPP_ */
