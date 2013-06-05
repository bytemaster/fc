#pragma once
#include <fc/string.hpp>
#include <fc/utility.hpp>

namespace fc {
    uint8_t from_hex( char c );
    fc::string to_hex( const char* d, uint32_t s );

    /**
     *  @return the number of bytes decoded
     */
    size_t from_hex( const fc::string& hex_str, char* out_data, size_t out_data_len );
} 
