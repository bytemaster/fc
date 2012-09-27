#include <fc/tcp_socket.hpp>
#include <fc/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>

namespace fc {

  class tcp_socket::impl {
    public:
      impl():_sock( fc::asio::default_io_service() ){}
      ~impl(){
        _sock.cancel();
      }

      boost::asio::ip::tcp::socket _sock;
  };

  tcp_socket::tcp_socket(){}

  tcp_socket::~tcp_socket(){}


  void   tcp_socket::write( const char* buf, size_t len ) {
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
      throw boost::system::system_error(ec);
    }
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
  size_t tcp_socket::read( char* buffer, size_t s ) {
    size_t r = readsome( buffer, s );
    while( r < s ) {
      r += readsome( buffer + r, s - r );
    }
    return r;
  }

  class tcp_server::impl {
    public:
      impl():_accept( fc::asio::default_io_service() ){}
      ~impl(){
        _accept.cancel();
      }

      boost::asio::ip::tcp::acceptor _accept;
  };

  tcp_server::tcp_server() {
  }
  tcp_server::~tcp_server() {
  }


  bool tcp_server::accept( tcp_socket& s ) {
    fc::promise<boost::system::error_code>::ptr p( 
        new promise<boost::system::error_code>("mace::cmt::asio::tcp::accept") );
    my->_accept.async_accept( s.my->_sock, 
        [=]( const boost::system::error_code& e ) {
          p->set_value(e);
        } );
    auto ec = p->wait();
    if( !ec ) s.my->_sock.non_blocking(true);
    if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
    return true;
  }
  void tcp_server::listen( uint16_t port ) {
    my->_accept.listen(port);
  }

} // namespace fc 
