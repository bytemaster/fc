#pragma once
#include <fc/utility.hpp>
#include <fc/fwd.hpp>
#include <fc/io/iostream.hpp>

namespace fc {
  namespace ip { class endpoint; } 
  class tcp_socket : public virtual iostream 
  {
    public:
      tcp_socket();
      ~tcp_socket();

      void     connect_to( const fc::ip::endpoint& e );
      fc::ip::endpoint remote_endpoint()const;

      /// istream interface
      /// @{
      virtual size_t   readsome( char* buffer, size_t max );
      virtual bool     eof()const;
      /// @}

      /// ostream interface
      /// @{
      virtual size_t   writesome( const char* buffer, size_t len );
      virtual void     flush();
      virtual void     close();
      /// @}

      bool   is_open()const;

    private:
      friend class tcp_server;
      class impl;
      fc::fwd<impl,0x44> my;
  };
  typedef std::shared_ptr<tcp_socket> tcp_socket_ptr;

  
  class tcp_server 
  {
    public:
      tcp_server();
      ~tcp_server();

      void close();
      void accept( tcp_socket& s );
      void listen( uint16_t port );
    
    private:
      // non copyable
      tcp_server( const tcp_server& ); 
      tcp_server& operator=(const tcp_server& s );

      class impl;
      impl* my;
  };

} // namesapce fc

