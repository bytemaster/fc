#ifndef _FC_BASE64_HPP
#define _FC_BASE64_HPP
#include <fc/string.hpp>

namespace fc {
fc::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
fc::string base64_encode( const fc::string& enc );
fc::string base64_decode( const fc::string& encoded_string);
}  // namespace fc
#endif // _FC_BASE64_HPP
