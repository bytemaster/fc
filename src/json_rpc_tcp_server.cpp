#include <fc/json_rpc_tcp_server.hpp>
#include <fc/json_rpc_tcp_connection.hpp>
#include <fc/tcp_socket.hpp>
#include <fc/thread.hpp>

namespace fc {
  namespace json {
    class rpc_tcp_server::impl {
      public:
        std::function<void(rpc_connection&)> on_con; 
        fc::tcp_server                       tcp_serv;
        fc::vector<rpc_tcp_connection::ptr>  cons;
    };
    rpc_tcp_server::rpc_tcp_server()
    :my( new impl() ){}
    rpc_tcp_server::~rpc_tcp_server() {
      delete my;
    }

    void rpc_tcp_server::on_new_connection( const std::function<void(rpc_connection&)>& c ) {
      my->on_con = c;
    }

    void rpc_tcp_server::listen( uint16_t port ) {
      
      my->tcp_serv.listen(port);
      fc::async([this](){
        try {
          rpc_tcp_connection::ptr con(new rpc_tcp_connection() );
          while( my->tcp_serv.accept( con->get_socket() ) ) {
            slog( "new connection!" );
            my->on_con( *con ); 
            con->start();
            my->cons.push_back(con);
            con.reset(new rpc_tcp_connection() );
          }
        } catch ( ... ) {
          wlog( "tcp listen failed..." );
        }
      });
    }


  }
}
