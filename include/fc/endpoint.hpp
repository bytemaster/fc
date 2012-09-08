#ifndef _FC_IP_HPP_
#define _FC_IP_HPP_
#include <fc/string.hpp>

namespace fc {

  namespace ip {
    class address {
      public:
        address( uint32_t _ip = 0 );
        address( const fc::string& s );

        address& operator=( const fc::string& s );
        operator fc::string()const;

      private:
        uint32_t _ip;
    };
    
    class endpoint {
      public:
        endpoint();
        endpoint( const fc::string& i, uint16_t p );
        endpoint( const address& i,    uint16_t p );

        uint16_t    port()        { return _port; }
        ip::address get_address() { return _ip;   }
    
      private:
        uint16_t _port;
        address  _ip;
    };
  }
}
#endif // _FC_ENDPOINT_HPP_
