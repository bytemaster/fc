#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>

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
    try {
        if( is_open() )
        { 
          my->_sock.close();
        }
    } FC_RETHROW_EXCEPTIONS( warn, "error closing tcp socket" );
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
        try {
          _accept.close();
        } 
        catch ( boost::system::system_error& e )
        {
           wlog( "unexpected exception ${e}", ("e", fc::except_str()) );
        }
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


  void tcp_server::accept( tcp_socket& s ) 
  {
    try
    {
      FC_ASSERT( my != nullptr );
      fc::asio::tcp::accept( my->_accept, s.my->_sock  ); 
    } FC_RETHROW_EXCEPTIONS( warn, "Unable to accept connection on socket." );
  }

  void tcp_server::listen( uint16_t port ) 
  {
    if( my ) delete my;
    my = new impl(port);
  }



} // namespace fc 
