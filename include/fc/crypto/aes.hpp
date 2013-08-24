#pragma once
#include <fc/crypto/sha512.hpp>
#include <vector>

namespace fc {
int aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
                unsigned char *iv, unsigned char *ciphertext);

std::vector<char> aes_encrypt( const fc::sha512& key, const std::vector<char>& plain_text  );
std::vector<char> aes_decrypt( const fc::sha512& key, const std::vector<char>& cipher_text );

} 
