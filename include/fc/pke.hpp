#ifndef FC_PKE_HPP_
#define FC_PKE_HPP_
#include <fc/sha1.hpp>
#include <fc/vector.hpp>

/**
 *  Define common crypto methods and data types to abstract underlying implementation.  
 */
namespace fc {

  template<uint32_t KeySize=2048>
  struct signature {
      char data[KeySize/8];
      template<typename T,uint32_t KS>
      friend T& operator<<( T& ds, const fc::signature<KS>& sig )
      {
          ds.write(sig.data, KS/8 );
          return ds;
      }
      template<typename T,uint32_t KS>
      friend T& operator>>( T& ds, fc::signature<KS>& sig )
      {
          ds.read(sig.data, KS/8 );
          return ds;
      }
      bool operator != ( const signature& s )const {
        return memcmp( s.data, data, sizeof(data) ) != 0;
      }
      bool operator == ( const signature& s )const {
        return memcmp( s.data, data, sizeof(data) ) == 0;
      }
  };

  bool verify_data( const char* key, uint32_t key_size, uint32_t pe, const sha1& hc, const char* sig ); 
  bool sign_data( const fc::vector<char>& key, uint32_t key_size, uint32_t pe, const sha1& hc, char* sig ); 
  bool public_encrypt( const char* key, uint32_t key_size, uint32_t pe, const fc::vector<char>& in, fc::vector<char>& out );
  bool public_decrypt( const char* key, uint32_t key_size, uint32_t pe, const fc::vector<char>& in, fc::vector<char>& out );
  bool private_encrypt( const fc::vector<char>& key, uint32_t key_size, uint32_t pe, const fc::vector<char>& in, fc::vector<char>& out );
  bool private_decrypt( const fc::vector<char>& key, uint32_t key_size, uint32_t pe, const fc::vector<char>& in, fc::vector<char>& out );
  bool generate_keys( char* pubkey, fc::vector<char>& privkey, uint32_t key_size, uint32_t pe );

  template<uint32_t KeySize = 2048, uint32_t PublicExponent = 65537>
  struct private_key;

  template<uint32_t KeySize = 2048, uint32_t PublicExponent = 65537>
  struct public_key {
    public_key()                       { memset( key, 0, sizeof(key) );      }
    public_key( const public_key& pk ) { memcpy( key, pk.key, sizeof(key) ); }

    bool verify( const sha1& digest, const signature<KeySize>& sig )const {
        return verify_data( key, sizeof(key), PublicExponent, digest, sig.data );
    }
    bool encrypt( const fc::vector<char>& in, fc::vector<char>& out )const {
        return public_encrypt( key, KeySize, PublicExponent, in, out );
    }
    bool decrypt( const fc::vector<char>& in, fc::vector<char>& out )const {
        return public_decrypt( key, KeySize, PublicExponent, in, out );
    }
    
    public_key& operator = ( const public_key& pk ) {
        memcpy( key, pk.key, sizeof(key) );
        return *this;
    }
    bool operator == ( const public_key& pk )const {
        return 0 == memcmp( key, pk.key, sizeof(key) );
    }
    bool operator != ( const public_key& pk )const {
        return 0 != memcmp( key, pk.key, sizeof(key) );
    }
    bool operator > ( const public_key& pk )const {
       return memcmp( key, pk.key, sizeof(key) ) > 0;
    }
    bool operator < ( const public_key& pk )const {
       return memcmp( key, pk.key, sizeof(key) ) < 0;
    }

    template<typename T,uint32_t KS, uint32_t PE>
    inline friend T& operator<<( T& ds, const fc::public_key<KS,PE>& pk ) {
        ds.write(pk.key, KS/8 );
        return ds;
    }
    template<typename T,uint32_t KS, uint32_t PE>
    inline friend T& operator>>( T& ds, fc::public_key<KS,PE>& pk ) {
        ds.read( pk.key, KS/8 );
        return ds;
    }

    private:
        template<uint32_t KS, uint32_t PE>
        friend void generate_keys( public_key<KS,PE>& pub, private_key<KS,PE>& priv );

        char key[KeySize/8];
  };


  template<uint32_t KeySize, uint32_t PublicExponent>
  struct private_key {
    bool encrypt( const fc::vector<char>& in, fc::vector<char>& out )const {
      return private_encrypt( key, KeySize, PublicExponent, in, out );
    }
    bool decrypt( const fc::vector<char>& in, fc::vector<char>& out )const {
      return private_decrypt( key, KeySize, PublicExponent, in, out );
    }
    bool sign( const sha1& digest, signature<KeySize>& sig )const {
      return sign_data( key, KeySize, PublicExponent, digest, sig.data );
    }

    template<typename T,uint32_t KS, uint32_t PE>
    friend T& operator<<( T& ds, const fc::private_key<KS,PE>& pk ) {
      uint16_t s = pk.key.size();
      ds.write( (const char*)&s, sizeof(s) );
      ds.write( pk.key.data(), pk.key.size() );
      return ds;
    }
    template<typename T,uint32_t KS, uint32_t PE>
    friend T& operator>>( T& ds, fc::private_key<KS,PE>& pk ) {
      uint16_t s;
      ds.read((char*)&s,sizeof(s) );
      pk.key.resize(s);
      ds.read( pk.key.data(), pk.key.size() );
      return ds;
    }
    private:
      template<uint32_t KS, uint32_t PE>
      friend void generate_keys( public_key<KS,PE>& pub, private_key<KS,PE>& priv );
      fc::vector<char> key;
  };

  template<uint32_t KeySize, uint32_t PublicExponent>
  void generate_keys( public_key<KeySize,PublicExponent>& pub, private_key<KeySize,PublicExponent>& priv ) {
      generate_keys( pub.key, priv.key, KeySize, PublicExponent );
  }
  /*
  template<uint32_t KeySize>
  inline std::ostream& operator<< ( std::ostream& os, const signature<KeySize>& s ) {
      for( uint32_t i = 0; i < KeySize; ++i )
          os << std::hex << int(s.data[i]) << ' ';
      return os;
  }
  */

  typedef public_key<>  public_key_t;
  typedef private_key<> private_key_t;
  typedef signature<>   signature_t;

} // namespace fc


#endif
