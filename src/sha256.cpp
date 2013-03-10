#include <fc/hex.hpp>
#include <fc/sha256.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/sha.h>
#include <string.h>
#include <fc/filesystem.hpp>
#include <fc/interprocess/file_mapping.hpp>
#include <fc/value.hpp>
#include <fc/value_cast.hpp>

namespace fc {
  
  sha256::sha256() { memset( _hash, 0, sizeof(_hash) ); }
  sha256::sha256( const fc::string& hex_str ) {
    fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
  }

  fc::string sha256::str()const {
    return to_hex( (char*)_hash, sizeof(_hash) );
  }
  sha256::operator fc::string()const { return  str(); }

  char* sha256::data()const { return (char*)&_hash[0]; }


  struct sha256::encoder::impl {
     SHA_CTX ctx;
  };

  sha256::encoder::~encoder() {}
  sha256::encoder::encoder() {
    reset();
  }

  sha256 sha256::hash( const char* d, uint32_t dlen ) {
    encoder e;
    e.write(d,dlen);
    return e.result();
  }
  sha256 sha256::hash( const fc::string& s ) {
    return hash( s.c_str(), s.size() );
  }
  sha256 sha256::hash( const fc::path& s ) {
    file_mapping fmap( s.string().c_str(), read_only );
    size_t       fsize = file_size(s);
    mapped_region mr( fmap, fc::read_only, 0, fsize );

    const char* pos = reinterpret_cast<const char*>(mr.get_address());
    return hash( pos, fsize );
  }

  void sha256::encoder::write( const char* d, uint32_t dlen ) {
    SHA1_Update( &my->ctx, d, dlen); 
  }
  sha256 sha256::encoder::result() {
    sha256 h;
    SHA1_Final((uint8_t*)h.data(), &my->ctx );
    return h;
  }
  void sha256::encoder::reset() {
    SHA1_Init( &my->ctx);  
  }

  fc::sha256 operator << ( const fc::sha256& h1, uint32_t i ) {
    fc::sha256 result;
    uint8_t* r = (uint8_t*)result._hash;
    uint8_t* s = (uint8_t*)h1._hash;
    for( uint32_t p = 0; p < sizeof(h1._hash)-1; ++p )
        r[p] = s[p] << i | (s[p+1]>>(8-i));
    r[31] = s[31] << i;
    return result;
  }
  fc::sha256 operator ^ ( const fc::sha256& h1, const fc::sha256& h2 ) {
    fc::sha256 result;
    result._hash[0] = h1._hash[0] ^ h2._hash[0];
    result._hash[1] = h1._hash[1] ^ h2._hash[1];
    result._hash[2] = h1._hash[2] ^ h2._hash[2];
    result._hash[3] = h1._hash[3] ^ h2._hash[3];
    return result;
  }
  bool operator >= ( const fc::sha256& h1, const fc::sha256& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
  }
  bool operator > ( const fc::sha256& h1, const fc::sha256& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
  }
  bool operator < ( const fc::sha256& h1, const fc::sha256& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
  }
  bool operator != ( const fc::sha256& h1, const fc::sha256& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) != 0;
  }
  bool operator == ( const fc::sha256& h1, const fc::sha256& h2 ) {
    return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) == 0;
  }

  void pack( fc::value& v, const fc::sha256& s ) {
      v = fc::string(s);
  }
  void unpack( const fc::value& v, fc::sha256& s ) {
      s = sha256(fc::value_cast<fc::string>(v));
  }
  
} // namespace fc

