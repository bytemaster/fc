#include <fc/network/ntp.hpp>
#include <fc/network/udp_socket.hpp>
#include <fc/network/resolve.hpp>
#include <fc/network/ip.hpp>
#include <fc/thread/thread.hpp>

#include <stdint.h>
#include "../byteswap.hpp"

#include <array>

namespace fc
{
  namespace detail {

     class ntp_impl 
     {
        public:
           ntp_impl() :_request_interval_sec( 60*60 /* 1 hr */) 
           { 
              _next_request_time = fc::time_point::now();
              _ntp_hosts.push_back( std::make_pair( "pool.ntp.org",123 ) );
           } 

           /** vector < host, port >  */
           std::vector< std::pair< std::string, uint16_t> > _ntp_hosts;
           fc::future<void>                                 _request_loop;
           fc::future<void>                                 _read_loop;
           udp_socket                                       _sock;
           uint32_t                                         _request_interval_sec;
           fc::time_point                                   _next_request_time;
           optional<fc::microseconds>                       _last_ntp_delta;

           void request_now()
           {
               for( auto item : _ntp_hosts )
               {
                  try 
                  {
                     ilog( "resolving... ${r}", ("r", item) );
                     auto eps = resolve( item.first, item.second );
                     for( auto ep : eps )
                     {
                        ilog( "sending request to ${ep}", ("ep",ep) );
                        std::array<unsigned char, 48> send_buf { {010,0,0,0,0,0,0,0,0} };
                        _sock.send_to( (const char*)send_buf.data(), send_buf.size(), ep );
                        break;
                     }
                  } 
                  // this could fail to resolve but we want to go on to other hosts..
                  catch ( const fc::exception& e )
                  {
                      elog(  "${e}", ("e",e.to_detail_string() ) ); 
                  }
               }
           } // request_now

           void request_loop()
           {
              while( !_request_loop.canceled() )
              { 
                  if( _next_request_time < fc::time_point::now() )
                  {
                     _next_request_time += fc::seconds( _request_interval_sec );
                     request_now();
                  }
                  fc::usleep( fc::seconds(1) ); // TODO: fix FC timers..
              } // while
           } // request_loop

           void read_loop()
           {
              while( !_read_loop.canceled() )
              {
                 fc::ip::endpoint from;
                 std::array<uint64_t, 1024> recv_buf;
                 try
                 {
                   _sock.receive_from( (char*)recv_buf.data(), recv_buf.size(), from );
                 } FC_RETHROW_EXCEPTIONS(error, "Error reading from NTP socket");

                 uint64_t receive_timestamp_net_order = recv_buf[4];
                 uint64_t receive_timestamp_host = bswap_64(receive_timestamp_net_order);
                 uint32_t fractional_seconds = receive_timestamp_host & 0xffffffff;
                 uint32_t microseconds = (uint32_t)(((((uint64_t)fractional_seconds) * 1000000) + (UINT64_C(1)<<31)) >> 32);
                 uint32_t seconds_since_1900 = receive_timestamp_host >> 32;
                 uint32_t seconds_since_epoch = seconds_since_1900 - 2208988800;

                 auto ntp_time = (fc::time_point() + fc::seconds(seconds_since_epoch) + fc::microseconds(microseconds));
                 if( ntp_time - fc::time_point::now() < fc::seconds(60*60*24) &&
                     fc::time_point::now() - ntp_time < fc::seconds(60*60*24) )
                 {
                    _last_ntp_delta =  ntp_time - fc::time_point::now();
                 }
                 else
                    elog( "NTP time is way off ${time}", ("time",ntp_time)("local",fc::time_point::now()) );
              }
           } // read_loop
     };

  } // namespace detail




  ntp::ntp()
  :my( new detail::ntp_impl() )
  {
     my->_sock.open();

     my->_request_loop = fc::async( [=](){ my->request_loop(); } );
     my->_read_loop    = fc::async( [=](){ my->read_loop(); } );
  }

  ntp::~ntp()
  {
    try {
        my->_request_loop.cancel();
        my->_read_loop.cancel();
        my->_sock.close();
        my->_request_loop.wait();
        my->_read_loop.wait();
    } 
    catch ( const fc::exception& )
    {
       // we expect canceled exceptions, but cannot throw
       // from destructor
    }
  }


  void ntp::add_server( const std::string& hostname, uint16_t port)
  {
     my->_ntp_hosts.push_back( std::make_pair(hostname,port) );
  }

  void ntp::set_request_interval( uint32_t interval_sec )
  {
     my->_request_interval_sec = interval_sec;
     my->_next_request_time = fc::time_point::now();
  }
  void ntp::request_now()
  {
     my->request_now();
  }

  optional<time_point> ntp::get_time()const
  {
     if( my->_last_ntp_delta )
        return fc::time_point::now() + *my->_last_ntp_delta;
     return optional<time_point>();
  }

}
