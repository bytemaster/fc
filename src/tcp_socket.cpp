#include <fc/tcp_socket.hpp>
#include <fc/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/log.hpp>
#include <fc/asio.hpp>
#include <fc/ip.hpp>

namespace fc {

  class tcp_socket::impl {
    public:
      impl():_sock( fc::asio::default_io_service() ){ slog( "sock %p", this); }
      ~impl(){
        slog( "~sock %p", this );
        if( _sock.is_open() ) _sock.close();
      }

      boost::asio::ip::tcp::socket _sock;
  };
  bool tcp_socket::is_open()const {
    return my->_sock.is_open();
  }

  tcp_socket::tcp_socket(){}

  tcp_socket::~tcp_socket(){ slog( "%p", &my); }

  void tcp_socket::flush() {}
  void tcp_socket::close() {
    my->_sock.close();
  }

  bool tcp_socket::eof()const {
    return !my->_sock.is_open();
  }

  fc::ostream&   tcp_socket::write( const char* buf, size_t len ) {
    boost::system::error_code ec;
    size_t w = my->_sock.write_some( boost::asio::buffer( buf, len ), ec );

    if( w < len ) {
      buf += w;
      len -= w;
    } 

    if( ec == boost::asio::error::would_block ) {
      promise<size_t>::ptr p(new promise<size_t>("tcp_socket::write"));
      boost::asio::async_write( my->_sock, boost::asio::buffer(buf, len),
                [=]( const boost::system::error_code& ec, size_t bt ) {
                    if( !ec ) p->set_value(bt);
                    else p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
                });
      p->wait();
    } else if( ec ) {
      wlog( "throw" );
      throw boost::system::system_error(ec);
    }
    return *this;
  }
  size_t tcp_socket::readsome( char* buf, size_t len ) {
    boost::system::error_code ec;
    size_t w = my->_sock.read_some( boost::asio::buffer( buf, len ), ec );
    if( ec == boost::asio::error::would_block ) {
      promise<size_t>::ptr p(new promise<size_t>("tcp_socket::write"));
      my->_sock.async_read_some( boost::asio::buffer(buf, len),
                [=]( const boost::system::error_code& ec, size_t bt ) {
                    if( !ec ) p->set_value(bt);
                    else p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
                });
      return p->wait();
    } else if (ec ) {
      throw boost::system::system_error(ec);
    }
    return w;
  }
  fc::istream& tcp_socket::read( char* buffer, size_t s ) {
    size_t r = readsome( buffer, s );
    while( r < s ) {
      r += readsome( buffer + r, s - r );
    }
    return *this;
  }
  void tcp_socket::connect_to( const fc::ip::endpoint& e ) {
    fc::asio::tcp::connect(my->_sock, fc::asio::tcp::endpoint( boost::asio::ip::address_v4(e.get_address()), e.port() ) ); 
  }

  class tcp_server::impl {
    public:
      impl(uint16_t port):_accept( fc::asio::default_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) ){}
      ~impl(){
        _accept.close();
      }

      boost::asio::ip::tcp::acceptor _accept;
  };
  void tcp_server::close() {
    if( my && my->_accept.is_open() ) my->_accept.close();
    delete my; my = nullptr;
  }
  tcp_server::tcp_server()
  :my(nullptr) {
  }
  tcp_server::~tcp_server() {
    delete my;
  }


  bool tcp_server::accept( tcp_socket& s ) {
    fc::promise<boost::system::error_code>::ptr p( new promise<boost::system::error_code>("mace::cmt::asio::tcp::accept") );
    my->_accept.async_accept( s.my->_sock, [=]( const boost::system::error_code& e ) {
                  p->set_value(e);
              } );
    auto ec = p->wait();
    if( !ec ) s.my->_sock.non_blocking(true);
    if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
    return true;
  }
  void tcp_server::listen( uint16_t port ) {
    if( my ) delete my;
    my = new impl(port);
  }



} // namespace fc 
