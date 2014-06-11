#include <fc/network/ntp.hpp>
#include <fc/network/udp_socket.hpp>
#include <fc/network/resolve.hpp>
#include <fc/network/ip.hpp>

#if defined(_WIN32)
# include <WinSock2.h> // for ntohl()
#elif defined(__linux__)
# include <arpa/inet.h>
#endif

#include <array>

namespace fc
{
  static fc::ip::endpoint ntp_server;


  void ntp::set_server( const std::string& hostname, uint16_t port  )
  {
     auto eps = resolve( hostname, port );
     if(  eps.size() )
        ntp_server = eps.front();
  }

  fc::time_point ntp::get_time()
  {
     static bool init_ntp_server = false;
     if( !init_ntp_server )
     {
        set_server( "pool.ntp.org", 123 );
        init_ntp_server = true;
     }

     udp_socket sock;
     sock.open();

     std::array<unsigned char, 48> send_buf { {010,0,0,0,0,0,0,0,0} };

     sock.send_to( (const char*)send_buf.data(), send_buf.size(), ntp_server );

     fc::ip::endpoint from;
     std::array<unsigned long, 1024> recv_buf;
     sock.receive_from( (char*)recv_buf.data(), recv_buf.size(), from );

     return fc::time_point() + fc::seconds( ntohl((time_t)recv_buf[4]) - 2208988800U);
  }

}
