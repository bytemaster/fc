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
            rpc_tcp_connection* tcpc = con.get();
            my->cons.push_back(con);
            con->on_close( [=]() {
               for( int i = 0; i < my->cons.size(); ++i ) {
                  if( my->cons[i].get() == tcpc ) {
                    fc_swap( my->cons[i], my->cons.back() );
                    auto tmp = my->cons.back();
                    my->cons.pop_back();
                    fc::async([tmp](){slog("free con");});
                    // TODO: swap to end, pop back
                    return;
                  }
               }
            });
            con.reset(new rpc_tcp_connection() );
          }
        } catch ( ... ) {
          wlog( "tcp listen failed..." );
        }
      });
    }


  }
}
