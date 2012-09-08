#include <fc/sha1.hpp>
#include <fc/hex.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/sha.h>
#include <string.h>

namespace fc {
  
  sha1::sha1() { memset( _hash, 0, sizeof(_hash) ); }
  sha1::sha1( const fc::string& hex_str ) {
    from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
  }

  fc::string sha1::str()const {
    return to_hex( (char*)_hash, sizeof(_hash) );
  }
  sha1::operator fc::string()const { return  str(); }

  sha1 operator << ( const sha1& h1, uint32_t i ) {
    sha1 result;
    uint8_t* r = (uint8_t*)result._hash;
    uint8_t* s = (uint8_t*)h1._hash;
    for( uint32_t p = 0; p < sizeof(h1._hash)-1; ++p )
        r[p] = s[p] << i | (s[p+1]>>(8-i));
    r[19] = s[19] << i;
    return result;
  }
  sha1 operator ^ ( const sha1& h1, const sha1 h2 ) {
    sha1 result;
    result._hash[0] = h1._hash[0] ^ h2._hash[0];
    result._hash[1] = h1._hash[1] ^ h2._hash[1];
    result._hash[2] = h1._hash[2] ^ h2._hash[2];
    result._hash[3] = h1._hash[3] ^ h2._hash[3];
    result._hash[4] = h1._hash[4] ^ h2._hash[4];
    return result;
  }
  bool operator >= ( const sha1& h1, const sha1 h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
  }
  bool operator > ( const sha1& h1, const sha1 h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
  }

  struct sha1::encoder::impl {
     SHA_CTX ctx;
  };

  sha1::encoder::encoder() {
    reset();
  }

  void sha1::encoder::write( const char* d, uint32_t dlen ) {
    SHA1_Update( &my->ctx, d, dlen); 
  }
  sha1 sha1::encoder::result() {
    sha1 h;
    SHA1_Final((uint8_t*)h.data(), &my->ctx );
    return h;
  }
  void sha1::encoder::reset() {
    SHA1_Init( &my->ctx);  
  }
  
}
