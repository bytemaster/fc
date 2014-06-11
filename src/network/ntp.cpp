#include <fc/network/ntp.hpp>
#include <fc/network/udp_socket.hpp>
#include <fc/network/resolve.hpp>
#include <fc/network/ip.hpp>

#include <stdint.h>
#include "../byteswap.hpp"

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
     std::array<uint64_t, 1024> recv_buf;
     sock.receive_from( (char*)recv_buf.data(), recv_buf.size(), from );

     uint64_t receive_timestamp_net_order = recv_buf[4];
     uint64_t receive_timestamp_host = bswap_64(receive_timestamp_net_order);
     uint32_t fractional_seconds = receive_timestamp_host & 0xffffffff;
     uint32_t microseconds = (uint32_t)(((((uint64_t)fractional_seconds) * 1000000) + (UINT64_C(1)<<31)) >> 32);
     uint32_t seconds_since_1900 = receive_timestamp_host >> 32;
     uint32_t seconds_since_epoch = seconds_since_1900 - 2208988800;

     return fc::time_point() + fc::seconds(seconds_since_epoch) + fc::microseconds(microseconds);
  }

}
