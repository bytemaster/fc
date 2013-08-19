#include <fc/network/http/server.hpp>
#include <fc/thread/thread.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/io/sstream.hpp>
#include <fc/io/stdio.hpp>
#include <fc/log/logger.hpp>


namespace fc { namespace http {

  class server::response::impl : public fc::retainable
  {
    public:
      impl( const fc::http::connection_ptr& c, const std::function<void()>& cont = std::function<void()>() )
      :body_bytes_sent(0),body_length(0),con(c),handle_next_req(cont)
      {}

      void send_header() {
         fc::stringstream ss;
         ss << "HTTP/1.1 " << rep.status << " ";
         switch( rep.status ) {
            case fc::http::reply::OK: ss << "OK\n\r"; break;
            case fc::http::reply::RecordCreated: ss << "Record Created\n\r"; break;
            case fc::http::reply::NotFound: ss << "Not Found\n\r"; break;
            case fc::http::reply::Found: ss << "Found\n\r"; break;
            case fc::http::reply::InternalServerError: ss << "Internal Server Error\r\n"; break;
         }
         for( uint32_t i = 0; i < rep.headers.size(); ++i ) {
            ss << rep.headers[i].key <<": "<<rep.headers[i].val <<"\r\n";
         }
         ss << "Content-Length: "<<body_length<<"\r\n\r\n";
         auto s = ss.str();
         fc::cerr<<s<<"\n";
         con->get_socket().write( s.c_str(), s.size() );
      }

      http::reply           rep;
      int64_t               body_bytes_sent;
      uint64_t              body_length;
      http::connection_ptr      con;
      std::function<void()> handle_next_req;
  };


  class server::impl 
  {
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
            while( !accept_complete.canceled() )
            {
              http::connection_ptr con = std::make_shared<http::connection>();
              tcp_serv.accept( con->get_socket() );
              ilog( "Accept Connection" );
              fc::async( [=](){ handle_connection( con, on_req ); } );
            }
      }

      void handle_connection( const http::connection_ptr& c,  
                              std::function<void(const http::request&, const server::response& s )> do_on_req ) {
         try {
             http::server::response rep( fc::shared_ptr<response::impl>( new response::impl(c, [=](){ this->handle_connection(c,do_on_req); } ) ) );
             auto req = c->read_request();
             if( do_on_req ) do_on_req( req, rep );
             c->get_socket().close();
          } catch ( fc::exception& e ) {
             wlog( "unable to read request ${1}", ("1", e.to_detail_string() ) );//fc::except_str().c_str());
          }
          wlog( "done handle connection" );
      }
      std::function<void(const http::request&, const server::response& s )> on_req;
      fc::tcp_server                                                        tcp_serv;
  };



  server::server(){}
  server::server( uint16_t port ) :my( new impl(port) ){}
  server::server( server&& s ):my(fc::move(s.my)){}

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

  void server::response::add_header( const fc::string& key, const fc::string& val )const {
     wlog( "Attempt to add header after sending headers" );
     my->rep.headers.push_back( fc::http::header( key, val ) );
  }
  void server::response::set_status( const http::reply::status_code& s )const {
     if( my->body_bytes_sent != 0 ) {
       wlog( "Attempt to set status after sending headers" );
     }
     my->rep.status = s;
  }
  void server::response::set_length( uint64_t s )const {
    if( my->body_bytes_sent != 0 ) {
      wlog( "Attempt to set length after sending headers" );
    }
    my->body_length = s; 
  }
  void server::response::write( const char* data, uint64_t len )const {
    if( my->body_bytes_sent + len > my->body_length ) {
      wlog( "Attempt to send to many bytes.." );
      len = my->body_bytes_sent + len - my->body_length;
    }
    if( my->body_bytes_sent == 0 ) {
      my->send_header();
    }
    my->body_bytes_sent += len;
    my->con->get_socket().write( data, static_cast<size_t>(len) ); 
    if( my->body_bytes_sent == int64_t(my->body_length) ) {
      if( my->handle_next_req ) {
        ilog( "handle next request..." );
        //fc::async( std::function<void()>(my->handle_next_req) );
        fc::async( my->handle_next_req );
      }
    }
  }

  server::response::~response(){}
  void server::on_request( const std::function<void(const http::request&, const server::response& s )>& cb )
  { my->on_req = cb; }




} }
