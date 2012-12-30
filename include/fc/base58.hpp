#pragma once
#include <fc/string.hpp>

namespace fc {
    fc::string to_base58( const char* d, size_t s );
    fc::vector<char> from_base58( const fc::string& base58_str );
    size_t from_base58( const fc::string& base58_str, char* out_data, size_t out_data_len );
}
