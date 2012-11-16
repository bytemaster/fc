#include <fc/hex.hpp>
#include <fc/sha1.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/sha.h>
#include <string.h>
#include <fc/filesystem.hpp>
#include <fc/interprocess/file_mapping.hpp>

namespace fc {
  
  sha1::sha1() { memset( _hash, 0, sizeof(_hash) ); }
  sha1::sha1( const fc::string& hex_str ) {
    fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
  }

  fc::string sha1::str()const {
    return to_hex( (char*)_hash, sizeof(_hash) );
  }
  sha1::operator fc::string()const { return  str(); }

  char* sha1::data()const { return (char*)&_hash[0]; }


  struct sha1::encoder::impl {
     SHA_CTX ctx;
  };

  sha1::encoder::~encoder() {}
  sha1::encoder::encoder() {
    reset();
  }

  sha1 sha1::hash( const char* d, uint32_t dlen ) {
    encoder e;
    e.write(d,dlen);
    return e.result();
  }
  sha1 sha1::hash( const fc::string& s ) {
    return hash( s.c_str(), s.size() );
  }
  sha1 sha1::hash( const fc::path& s ) {
    file_mapping fmap( s.string().c_str(), read_only );
    size_t       fsize = file_size(s);
    mapped_region mr( fmap, fc::read_only, 0, fsize );

    const char* pos = reinterpret_cast<const char*>(mr.get_address());
    return hash( pos, fsize );
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

  fc::sha1 operator << ( const fc::sha1& h1, uint32_t i ) {
    fc::sha1 result;
    uint8_t* r = (uint8_t*)result._hash;
    uint8_t* s = (uint8_t*)h1._hash;
    for( uint32_t p = 0; p < sizeof(h1._hash)-1; ++p )
        r[p] = s[p] << i | (s[p+1]>>(8-i));
    r[19] = s[19] << i;
    return result;
  }
  fc::sha1 operator ^ ( const fc::sha1& h1, const fc::sha1& h2 ) {
    fc::sha1 result;
    result._hash[0] = h1._hash[0] ^ h2._hash[0];
    result._hash[1] = h1._hash[1] ^ h2._hash[1];
    result._hash[2] = h1._hash[2] ^ h2._hash[2];
    result._hash[3] = h1._hash[3] ^ h2._hash[3];
    result._hash[4] = h1._hash[4] ^ h2._hash[4];
    return result;
  }
  bool operator >= ( const fc::sha1& h1, const fc::sha1& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
  }
  bool operator > ( const fc::sha1& h1, const fc::sha1& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
  }
  bool operator < ( const fc::sha1& h1, const fc::sha1& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
  }
  bool operator != ( const fc::sha1& h1, const fc::sha1& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) != 0;
  }
  bool operator == ( const fc::sha1& h1, const fc::sha1& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) == 0;
  }
  
} // namespace fc

