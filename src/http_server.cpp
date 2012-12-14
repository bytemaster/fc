#include <fc/http/server.hpp>
#include <fc/thread.hpp>
#include <fc/tcp_socket.hpp>


namespace fc { namespace http {

  class server::response::impl : public fc::retainable {
    public:
      impl( const fc::http::connection& c, const std::function<void()>& cont = std::function<void()>() )
      :con(c),handle_next_req(cont)
      {}

      http::connection      con;
      std::function<void()> handle_next_req;
  };


  class server::impl : public fc::retainable {
    public:
      impl(){}
      impl(uint16_t p ) {
        tcp_serv.listen(p);
        accept_complete = fc::async([this](){ this->accept_loop(); });
      }
      fc::future<void> accept_complete;
      ~impl() {
        try {
          tcp_serv.close();
          accept_complete.wait();
        }catch(...){}
      }
      void accept_loop() {
          try {
            http::connection con;
            while( tcp_serv.accept( con.get_socket() ) ) {
              fc::async( [=](){ handle_connection( con, on_req ); } );
              con = http::connection();
            }
          } catch ( ... ) {
            wlog( "tcp listen failed...%s", fc::except_str().c_str() );
          }
      }

      void handle_connection( const http::connection& c,  
                              std::function<void(const http::request&, const server::response& s )> do_on_req ) {
         http::server::response rep( fc::shared_ptr<response::impl>( new response::impl(c, [=](){ this->handle_connection(c,do_on_req); } ) ) );
         auto req = c.read_request();
         if( do_on_req ) do_on_req( req, rep );
      }
      std::function<void(const http::request&, const server::response& s )> on_req;
      fc::tcp_server                                                        tcp_serv;
  };



  server::server(){}
  server::server( uint16_t port ) :my( new impl(port) ){}
  server::server( const server& s ):my(s.my){}
  server::server( server&& s ):my(fc::move(s.my)){}

  server& server::operator=(const server& s) { my = s.my; return *this; }
  server& server::operator=(server&& s)      { fc_swap(my,s.my); return *this; }

  server::~server(){}

  void server::listen( uint16_t p ) {
    my.reset( new impl(p) );
  }



  server::response::response(){}
  server::response::response( const server::response& s ):my(s.my){}
  server::response::response( server::response&& s ):my(fc::move(s.my)){}
  server::response::response( const fc::shared_ptr<server::response::impl>& m ):my(m){}

  server::response& server::response::operator=(const server::response& s) { my = s.my; return *this; }
  server::response& server::response::operator=(server::response&& s)      { fc_swap(my,s.my); return *this; }

  server::response::~response(){}




} }
