#pragma once
#include <fc/fwd.hpp>
#include <fc/string.hpp>

namespace fc
{

class sha256 
{
  public:
    sha256();
    explicit sha256( const string& hex_str );

    string str()const;
    operator string()const;

    char*    data()const;

    static sha256 hash( const char* d, uint32_t dlen );
    static sha256 hash( const string& );

    template<typename T>
    static sha256 hash( const T& t ) 
    { 
      sha256::encoder e; 
      e << t; 
      return e.result(); 
    } 

    class encoder 
    {
      public:
        encoder();
        ~encoder();

        void write( const char* d, uint32_t dlen );
        void put( char c ) { write( &c, 1 ); }
        void reset();
        sha256 result();

      private:
        struct      impl;
        fc::fwd<impl,112> my;
    };

    template<typename T>
    inline friend T& operator<<( T& ds, const sha256& ep ) {
      ds.write( ep.data(), sizeof(ep) );
      return ds;
    }

    template<typename T>
    inline friend T& operator>>( T& ds, sha256& ep ) {
      ds.read( ep.data(), sizeof(ep) );
      return ds;
    }
    friend sha256 operator << ( const sha256& h1, uint32_t i       );
    friend bool   operator == ( const sha256& h1, const sha256& h2 );
    friend bool   operator != ( const sha256& h1, const sha256& h2 );
    friend sha256 operator ^  ( const sha256& h1, const sha256& h2 );
    friend bool   operator >= ( const sha256& h1, const sha256& h2 );
    friend bool   operator >  ( const sha256& h1, const sha256& h2 ); 
    friend bool   operator <  ( const sha256& h1, const sha256& h2 ); 
                             
    uint64_t _hash[4]; 
};

  class variant;
  void to_variant( const sha256& bi, variant& v );
  void from_variant( const variant& v, sha256& bi );

} // fc
