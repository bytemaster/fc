#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>

namespace fc {

  class tcp_socket::impl {
    public:
      impl():_sock( fc::asio::default_io_service() ){  }
      ~impl(){
        if( _sock.is_open() ) _sock.close();
      }
      boost::asio::ip::tcp::socket _sock;
  };
  bool tcp_socket::is_open()const {
    return my->_sock.is_open();
  }

  tcp_socket::tcp_socket(){};

  tcp_socket::~tcp_socket(){};

  void tcp_socket::flush() {}
  void tcp_socket::close() {
    if( is_open() ) my->_sock.close();
  }

  bool tcp_socket::eof()const {
    return !my->_sock.is_open();
  }

  size_t   tcp_socket::writesome( const char* buf, size_t len ) {
    return fc::asio::write_some( my->_sock, boost::asio::buffer( buf, len ) );
  }

 fc::ip::endpoint tcp_socket::remote_endpoint()const
 {
   auto rep = my->_sock.remote_endpoint();
   return  fc::ip::endpoint(rep.address().to_v4().to_ulong(), rep.port() );
 }

  size_t tcp_socket::readsome( char* buf, size_t len ) {
    auto r =  fc::asio::read_some( my->_sock, boost::asio::buffer( buf, len ) );
    return r;
  }

  void tcp_socket::connect_to( const fc::ip::endpoint& e ) {
    fc::asio::tcp::connect(my->_sock, fc::asio::tcp::endpoint( boost::asio::ip::address_v4(e.get_address()), e.port() ) ); 
  }

  class tcp_server::impl {
    public:
      impl(uint16_t port):
      _accept( fc::asio::default_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) ){
      }
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
    try
    {
      if( !my ) return false;

      fc::asio::tcp::accept( my->_accept, s.my->_sock  ); 
      /*
      fc::promise<void>::ptr p( new promise<void>("tcp::accept") );
      my->_accept.async_accept( s.my->_sock, [=]( const boost::system::error_code& e ) {
                    p->set_value(e);
                } );
      auto ec = p->wait();
      if( ec ) FC_THROW_EXCEPTION( exception, "system error: ${message}", ("message", fc::string(boost::system::system_error(ec).what()) ));
      return true;
      */
      return true;
    } FC_RETHROW_EXCEPTIONS( warn, "Unable to accept connection on socket." );
  }
  void tcp_server::listen( uint16_t port ) {
    if( my ) delete my;
    my = new impl(port);
  }



} // namespace fc 
