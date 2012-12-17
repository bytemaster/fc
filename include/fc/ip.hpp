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
        address  _ip;
        /**
         *  The compiler pads endpoint to a full 8 bytes, so while
         *  a port number is limited in range to 16 bits, we specify
         *  a full 32 bits so that memcmp can be used with sizeof(), 
         *  otherwise 2 bytes will be 'random' and you do not know 
         *  where they are stored.
         */
        uint32_t _port; 
    };
  }
  class value;
  void pack( fc::value& , const fc::ip::endpoint&  );
  void unpack( const fc::value& , fc::ip::endpoint&  );
}
