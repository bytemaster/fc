#pragma once
#include <string>

namespace fc {
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_encode( const std::string& enc );
std::string base64_decode( const std::string& encoded_string);
}  // namespace fc
