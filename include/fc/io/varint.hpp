#pragma once
#include <stdint.h>

namespace fc {

struct unsigned_int {
    unsigned_int( uint32_t v = 0 ):value(v){}

    template<typename T>
    unsigned_int( T v ):value(v){}

    //operator uint32_t()const { return value; }
    //operator uint64_t()const { return value; }

    template<typename T>
    operator T()const { return static_cast<T>(value); }

    unsigned_int& operator=( int32_t v ) { value = v; return *this; }
    
    uint32_t value;

    friend bool operator==( const unsigned_int& i, const uint32_t& v ) { return v == i.value; }
    friend bool operator!=( const unsigned_int& i, const uint32_t& v ) { return v != i.value; }
};

struct signed_int {
    signed_int( int32_t v = 0 ):value(v){}
    operator int32_t()const { return value; }
    template<typename T>
    signed_int& operator=( const T& v ) { value = v; return *this; }

    int32_t value;
};

class variant;

void to_variant( const signed_int& var,  variant& vo );
void from_variant( const variant& var,  signed_int& vo );
void to_variant( const unsigned_int& var,  variant& vo );
void from_variant( const variant& var,  unsigned_int& vo );

}  // namespace fc


