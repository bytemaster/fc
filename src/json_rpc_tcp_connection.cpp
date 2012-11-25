#include <fc/json_rpc_tcp_connection.hpp>
#include <fc/tcp_socket.hpp>

namespace fc {

  namespace json {
    class rpc_tcp_connection::impl : public fc::retainable {
      public:
          tcp_socket sock;
          ~impl(){ }
    };
    
    rpc_tcp_connection::rpc_tcp_connection()
    :my( new impl() ){
    }
    rpc_tcp_connection::rpc_tcp_connection( const rpc_tcp_connection& c )
    :rpc_stream_connection(c),my(c.my){}

    rpc_tcp_connection::~rpc_tcp_connection(){
      close();
    }

    void rpc_tcp_connection::connect_to( const fc::ip::endpoint& ep ) {
      my->sock.connect_to(ep);
      open( my->sock, my->sock );
    }
    void rpc_tcp_connection::start() {
      open( my->sock, my->sock );
    }
    void rpc_tcp_connection::close() {
      rpc_stream_connection::close();
      //my->sock.close();
    }

    tcp_socket& rpc_tcp_connection::get_socket()const { return my->sock; }


  }

}
