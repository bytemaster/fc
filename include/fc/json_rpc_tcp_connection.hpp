#pragma once
#include <fc/json_rpc_stream_connection.hpp>

namespace fc { 
  class tcp_socket;
  namespace ip { class endpoint; }

  namespace json {
    class rpc_tcp_connection : public rpc_stream_connection {
      public:
          typedef fc::shared_ptr<rpc_tcp_connection> ptr;

          rpc_tcp_connection();
          rpc_tcp_connection( const rpc_tcp_connection& c );
          ~rpc_tcp_connection();

          void connect_to( const fc::ip::endpoint& e );
          void start(); 
          tcp_socket& get_socket()const;

          virtual void close();
        
      private:
          class impl;
          fc::shared_ptr<impl> my;
    };
  } // json
} // fc
