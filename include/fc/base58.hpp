#ifndef _FC_BASE58_HPP_
#define _FC_BASE58_HPP_
#include <fc/string.hpp>

namespace fc {
fc::string to_base58( const char* d, uint32_t s );
size_t from_base58( const fc::string& base58_str, char* out_data, size_t out_data_len );
}
#endif // _FC_BASE58_HPP_
