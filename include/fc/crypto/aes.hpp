#pragma once
#include <fc/sha512.hpp>

namespace fc {

std::vector<char> aes_encrypt( const fc::sha512& key, const std::vector<char>& plain_text  );
std::vector<char> aes_decrypt( const fc::sha512& key, const std::vector<char>& cipher_text );

} 
