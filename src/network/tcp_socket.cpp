#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>

#if defined _WIN32 || defined WIN32 || defined OS_WIN64 || defined _WIN64 || defined WIN64 || defined WINNT
# include <MSTcpIP.h>
#endif

namespace fc {

  class tcp_socket::impl {
    public:
      impl():_sock( fc::asio::default_io_service() ){}
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

  void tcp_socket::connect_to( const fc::ip::endpoint& remote_endpoint ) {
    fc::asio::tcp::connect(my->_sock, fc::asio::tcp::endpoint( boost::asio::ip::address_v4(remote_endpoint.get_address()), remote_endpoint.port() ) ); 
  }

  void tcp_socket::connect_to( const fc::ip::endpoint& remote_endpoint, const fc::ip::endpoint& local_endpoint ) {
    my->_sock = boost::asio::ip::tcp::socket(fc::asio::default_io_service(), 
                                             boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(local_endpoint.get_address()), 
                                                                                                        local_endpoint.port()));
    fc::asio::tcp::connect(my->_sock, fc::asio::tcp::endpoint( boost::asio::ip::address_v4(remote_endpoint.get_address()), remote_endpoint.port() ) ); 
  }

  void tcp_socket::enable_keep_alives(const fc::microseconds& interval)
  {
    if (interval.count())
    {
      boost::asio::socket_base::keep_alive option(true);
      my->_sock.set_option(option);
#if defined _WIN32 || defined WIN32 || defined OS_WIN64 || defined _WIN64 || defined WIN64 || defined WINNT
      struct tcp_keepalive keepalive_settings;
      keepalive_settings.onoff = 1;
      keepalive_settings.keepalivetime = interval.count() / fc::milliseconds(1).count();
      keepalive_settings.keepaliveinterval = interval.count() / fc::milliseconds(1).count();

      DWORD dwBytesRet = 0;
      if (WSAIoctl(my->_sock.native(), SIO_KEEPALIVE_VALS, &keepalive_settings, sizeof(keepalive_settings),
                   NULL, 0, &dwBytesRet, NULL, NULL) == SOCKET_ERROR)
        wlog("Error setting TCP keepalive values");
#else
# if !defined(__clang__) || (__clang_major__ >= 6)
      // This should work for modern Linuxes and for OSX >= Mountain Lion
      int timeout_sec = interval.count() / fc::seconds(1).count();
      if (setsockopt(my->_sock.native(), IPPROTO_TCP, 
#  if defined( __APPLE__ )
                     TCP_KEEPALIVE,
#  else
                     TCP_KEEPIDLE, 
#  endif
                     (char*)&timeout_sec, sizeof(timeout_sec)) < 0)
        wlog("Error setting TCP keepalive idle time");
      if (setsockopt(my->_sock.native(), IPPROTO_TCP, TCP_KEEPINTVL, 
                     (char*)&timeout_sec, sizeof(timeout_sec)) < 0)
        wlog("Error setting TCP keepalive interval");
# endif
#endif
    }
    else
    {
      boost::asio::socket_base::keep_alive option(false);
      my->_sock.set_option(option);
    }
  }

  class tcp_server::impl {
    public:
      impl(uint16_t port)
      :_accept( fc::asio::default_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) ){}

      impl(const fc::ip::endpoint& ep  )
      :_accept( fc::asio::default_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4( ep.get_address()), ep.port()) ){}

      ~impl(){
        try {
          _accept.close();
        } 
        catch ( boost::system::system_error& )
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
  void tcp_server::listen( const fc::ip::endpoint& ep ) 
  {
    if( my ) delete my;
    my = new impl(ep);
  }

  uint16_t tcp_server::get_port()const
  {
     return my->_accept.local_endpoint().port();
  }



} // namespace fc 
