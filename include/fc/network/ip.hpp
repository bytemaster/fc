#pragma once
#include <fc/string.hpp>
#include <fc/crypto/sha1.hpp>

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
        friend bool operator< ( const endpoint& a, const endpoint& b )
        {
           return  uint32_t(a.get_address()) < uint32_t(b.get_address()) ||
                   (uint32_t(a.get_address()) == uint32_t(b.get_address()) &&
                    uint32_t(a.port()) < uint32_t(b.port()));
        }
    
      private:
        /**
         *  The compiler pads endpoint to a full 8 bytes, so while
         *  a port number is limited in range to 16 bits, we specify
         *  a full 32 bits so that memcmp can be used with sizeof(), 
         *  otherwise 2 bytes will be 'random' and you do not know 
         *  where they are stored.
         */
        uint32_t _port; 
        address  _ip;
    };
  }
  class variant;
  void to_variant( const ip::endpoint& var,  variant& vo );
  void from_variant( const variant& var,  ip::endpoint& vo );

  void to_variant( const ip::address& var,  variant& vo );
  void from_variant( const variant& var,  ip::address& vo );
}
namespace std
{
    template<typename T> struct hash;

    template<>
    struct hash<fc::ip::endpoint>
    {
       size_t operator()( const fc::ip::endpoint& e )const;
    };
}
