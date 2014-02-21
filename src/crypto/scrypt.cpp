#include <algorithm>

#include <fc/crypto/openssl.hpp>
#include <fc/exception/exception.hpp>

#include <openssl/evp.h>

#define SCRYPT_SALSA 1
#define SCRYPT_SHA256 1

#include "code/scrypt-jane-portable.h"
#include "code/scrypt-jane-romix.h"

namespace fc {

   void scrypt_derive_key( const std::vector<unsigned char> &passphrase, const std::vector<unsigned char> &salt,
                           unsigned int n, unsigned int r, unsigned int p, std::vector<unsigned char> &key )
   {
      unsigned int chunk_bytes = SCRYPT_BLOCK_BYTES * r * 2;
      std::vector<unsigned char> yx((p+1) * chunk_bytes);

      unsigned char *Y = &yx[0];
      unsigned char *X = &yx[chunk_bytes];

      if(PKCS5_PBKDF2_HMAC( (const char*)&passphrase[0], passphrase.size(),
                            &salt[0], salt.size(), 1,
                            EVP_sha256(), chunk_bytes * p, X) != 1 )
      {
         std::fill( yx.begin(), yx.end(), 0 );
         FC_THROW_EXCEPTION( exception, "error generating key material",
                             ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
      }

      std::vector<unsigned char> v(n * chunk_bytes);

      for( unsigned int i = 0; i < p; i++ )
         scrypt_ROMix_basic( (uint32_t*)(X+(chunk_bytes*i)), (uint32_t*)Y, (uint32_t*)&v[0], n, r );

      if(PKCS5_PBKDF2_HMAC( (const char*)&passphrase[0], passphrase.size(),
                            X, chunk_bytes * p, 1,
                            EVP_sha256(), key.size(), &key[0]) != 1 )
      {
         std::fill( yx.begin(), yx.end(), 0 );
         std::fill( v.begin(), v.end(), 0 );
         FC_THROW_EXCEPTION( exception, "error generating key material",
                             ("s", ERR_error_string( ERR_get_error(), nullptr) ) );
      }

      std::fill( yx.begin(), yx.end(), 0 );
      std::fill( v.begin(), v.end(), 0 );
   }

} // namespace fc
