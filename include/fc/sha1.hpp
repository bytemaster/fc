#ifndef _FC_SHA1_HPP_
#define _FC_SHA1_HPP_
#include <fc/fwd.hpp>
#include <fc/string.hpp>

namespace fc {

  class sha1 {
    public:
      sha1();
      explicit sha1( const fc::string& hex_str );

      fc::string str()const;

      operator fc::string()const;

      char*    data()const;

      static sha1 hash( const char* d, uint32_t dlen );
      static sha1 hash( const fc::string& );

      class encoder {
        public:
          encoder();

          void write( const char* d, uint32_t dlen );
          void reset();
          sha1 result();

        private:
          struct      impl;
          fwd<impl,8> my;
      };

      template<typename T>
      inline friend T& operator<<( T& ds, const sha1& ep ) {
        ds.write( (const char*)ep.hash, sizeof(ep.hash) );
        return ds;
      }

      template<typename T>
      inline friend T& operator>>( T& ds, sha1& ep ) {
        ds.read( (char*)ep.hash, sizeof(ep.hash) );
        return ds;
      }
      friend sha1 operator << ( const sha1& h1, uint32_t i );
      friend bool operator == ( const sha1& h1, const sha1& h2 );
      friend bool operator != ( const sha1& h1, const sha1& h2 );
      friend sha1 operator ^ ( const sha1& h1, const sha1& h2 );
      friend bool operator >= ( const sha1& h1, const sha1& h2 );
      friend bool operator > ( const sha1& h1, const sha1& h2 ); 

      uint32_t _hash[5]; 
  };

}

#endif // _FC_SHA1_HPP_

