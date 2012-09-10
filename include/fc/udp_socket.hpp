#ifndef _FC_UDP_SOCKET_HPP_
#define _FC_UDP_SOCKET_HPP_
#include <fc/utility.hpp>
#include <fc/fwd.hpp>

namespace fc {
  namespace ip {
    class endpoint;
  }

  class udp_socket {
    public:
      udp_socket();
      ~udp_socket();

      void   open();
      void   set_receive_buffer_size( size_t s );
      void   bind( const fc::ip::endpoint& );
      size_t receive_from( char* b, size_t l, fc::ip::endpoint& from );
      size_t send_to( const char* b, size_t l, const fc::ip::endpoint& to ); 
      void   close();

      void   connect( const fc::ip::endpoint& e );
      fc::ip::endpoint local_endpoint()const;

    private:
      class       impl;
      fwd<impl,32> my;
  };

}

#endif
