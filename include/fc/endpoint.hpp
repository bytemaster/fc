#pragma once
#include <fc/string.hpp>

namespace fc {

  namespace ip {
    class address {
      public:
        address( uint32_t _ip = 0 );
        address( const fc::string& s );


        address& operator=( const fc::string& s );
        operator fc::string()const;

        uint32_t ip()const { return _ip; }


      private:
        uint32_t _ip;
    };
    
    class endpoint {
      public:
        endpoint();
        endpoint( const fc::string& i, uint16_t p );
        endpoint( const address& i,    uint16_t p );

        uint16_t        port()const        { return _port; }
        fc::ip::address get_address()const { return _ip;   }
    
      private:
        uint16_t _port;
        address  _ip;
    };
  }
}
