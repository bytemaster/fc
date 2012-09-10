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
        operator uint32_t()const;

        friend bool operator==( const address& a, const address& b );
      private:
        uint32_t _ip;
    };
    
    class endpoint {
      public:
        endpoint();
        endpoint( const address& i, uint16_t p = 0);

        /** Converts "IP:PORT" to an endpoint */
        static endpoint from_string( const string& s );
        /** returns "IP:PORT" */
        operator string()const;

        void           set_port(uint16_t p ) { _port = p; }
        uint16_t       port()const;
        const address& get_address()const;

        friend bool operator==( const endpoint& a, const endpoint& b );
    
      private:
        uint16_t _port;
        address  _ip;
    };
  }
}
#endif // _FC_ENDPOINT_HPP_
