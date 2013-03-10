#pragma once
#include <fc/fwd.hpp>
#include <fc/string.hpp>
#include <fc/reflect.hpp>

namespace fc {
  class path;

  class sha256 {
    public:
      sha256();
      explicit sha256( const fc::string& hex_str );

      fc::string str()const;

      operator fc::string()const;

      char*    data()const;

      static sha256 hash( const char* d, uint32_t dlen );
      static sha256 hash( const fc::string& );
      static sha256 hash( const fc::path& );

      template<typename T>
      static sha256 hash( const T& t ) { sha256::encoder e; e << t; return e.result(); } 

      class encoder {
        public:
          encoder();
          ~encoder();

          void write( const char* d, uint32_t dlen );
          void put( char c ) { write( &c, 1 ); }
          void reset();
          sha256 result();

        private:
          struct      impl;
          fwd<impl,96> my;
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
      friend sha256 operator << ( const sha256& h1, uint32_t i );
      friend bool operator == ( const sha256& h1, const sha256& h2 );
      friend bool operator != ( const sha256& h1, const sha256& h2 );
      friend sha256 operator ^ ( const sha256& h1, const sha256& h2 );
      friend bool operator >= ( const sha256& h1, const sha256& h2 );
      friend bool operator > ( const sha256& h1, const sha256& h2 ); 
      friend bool operator < ( const sha256& h1, const sha256& h2 ); 

      uint64_t _hash[4]; 
  };

  class value;
  void pack( fc::value& , const fc::sha256&  );
  void unpack( const fc::value& , fc::sha256&  );
}


