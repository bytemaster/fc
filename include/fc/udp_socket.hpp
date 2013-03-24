#pragma once
#include <fc/utility.hpp>
#include <fc/shared_ptr.hpp>

namespace fc {
  namespace ip {
    class endpoint;
    class address;
  }

  /**
   *  The udp_socket class has reference semantics, all copies will
   *  refer to the same underlying socket.
   */
  class udp_socket {
    public:
      udp_socket();
      udp_socket( const udp_socket& s );
      ~udp_socket();

      void   open();
      void   set_receive_buffer_size( size_t s );
      void   bind( const fc::ip::endpoint& );
      size_t receive_from( char* b, size_t l, fc::ip::endpoint& from );
      size_t send_to( const char* b, size_t l, const fc::ip::endpoint& to ); 
      void   close();

      void   set_multicast_enable_loopback( bool );
      void   set_reuse_address( bool );
      void   join_multicast_group( const fc::ip::address& a );

      void   connect( const fc::ip::endpoint& e );
      fc::ip::endpoint local_endpoint()const;

    private:
      class                impl;
      fc::shared_ptr<impl> my;
  };

}
