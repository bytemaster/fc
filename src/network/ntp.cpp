#include <fc/network/ntp.hpp>
#include <fc/network/udp_socket.hpp>
#include <fc/network/resolve.hpp>
#include <fc/network/ip.hpp>
#include <fc/thread/thread.hpp>

#include <stdint.h>
#include "../byteswap.hpp"

#include <atomic>
#include <array>

namespace fc
{
  namespace detail {

     class ntp_impl 
     {
        public:
           /** vector < host, port >  */
           std::vector< std::pair< std::string, uint16_t> > _ntp_hosts;
           fc::future<void>                                 _read_loop_done;
           udp_socket                                       _sock;
           uint32_t                                         _request_interval_sec;
           fc::time_point                                   _last_request_time;

           std::atomic_bool                                 _last_ntp_delta_initialized;
           std::atomic<int64_t>                             _last_ntp_delta_microseconds;

           fc::thread                                       _ntp_thread;

           fc::future<void>                                 _request_time_task_done;

           ntp_impl() :
            _request_interval_sec( 60*60 /* 1 hr */),
            _last_ntp_delta_microseconds(0),
            _ntp_thread("ntp")
           { 
              _last_ntp_delta_initialized = false;
              _ntp_hosts.push_back( std::make_pair( "pool.ntp.org",123 ) );
           } 

           void request_now()
           {
               assert(_ntp_thread.is_current());
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
                        _last_request_time = fc::time_point::now();
                        _sock.send_to( (const char*)send_buf.data(), send_buf.size(), ep );
                        if (!_read_loop_done.valid() || _read_loop_done.ready())
                          _read_loop_done = async( [this](){ read_loop(); }, "ntp_read_loop" );
                        break;
                     }
                  } 
                  // this could fail to resolve but we want to go on to other hosts..
                  catch ( const fc::exception& e )
                  {
                      elog( "${e}", ("e",e.to_detail_string() ) ); 
                  }
               }
           } // request_now

           void request_time_task()
           {
              request_now();
              _request_time_task_done = _ntp_thread.schedule( [=](){ request_time_task(); }, 
                                                              fc::time_point::now() + fc::seconds(_request_interval_sec), 
                                                              "request_time_task" );
           } // request_loop

           void read_loop()
           {
              assert(_ntp_thread.is_current());
              while( !_read_loop_done.canceled() )
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

                 if( fc::time_point::now() - _last_request_time > fc::seconds(1) )
                    request_now();
                 else
                 {
                    auto ntp_time = (fc::time_point() + fc::seconds(seconds_since_epoch) + fc::microseconds(microseconds));
                    if( ntp_time - fc::time_point::now() < fc::seconds(60*60*24) &&
                        fc::time_point::now() - ntp_time < fc::seconds(60*60*24) )
                    {
                       _last_ntp_delta_microseconds = (ntp_time - fc::time_point::now()).count();
                       _last_ntp_delta_initialized = true;
                    }
                    else
                       elog( "NTP time is way off ${time}", ("time",ntp_time)("local",fc::time_point::now()) );
                 }
              }
           } // read_loop
     };

  } // namespace detail




  ntp::ntp()
  :my( new detail::ntp_impl() )
  {
     my->_sock.open();
     // if you start the read loop here, the recieve_from call will throw "invalid argument" on win32,
     // so instead we trigger the read loop after making our first request
     my->_request_time_task_done = my->_ntp_thread.async( [=](){ my->request_time_task(); }, "request_time_task" );
  }

  ntp::~ntp()
  {
    my->_ntp_thread.async([=](){
      try
      {
        my->_request_time_task_done.cancel_and_wait();
      }
      catch ( const fc::exception& e )
      {
        wlog( "Exception thrown while shutting down NTP's request_time_task, ignoring: ${e}", ("e",e) );
      }
      catch (...)
      {
        wlog( "Exception thrown while shutting down NTP's request_time_task, ignoring" );
      }
      
      try 
      {
        my->_read_loop_done.cancel();
        my->_sock.close();
        my->_read_loop_done.wait();
      } 
      catch ( const fc::canceled_exception& )
      {
      }
      catch ( const fc::exception& e )
      {
        wlog( "Exception thrown while shutting down NTP's read_loop, ignoring: ${e}", ("e",e) );
      }
      catch (...)
      {
        wlog( "Exception thrown while shutting down NTP's read_loop, ignoring" );
      }
    }).wait();
  }


  void ntp::add_server( const std::string& hostname, uint16_t port)
  {
     my->_ntp_thread.async( [=](){ my->_ntp_hosts.push_back( std::make_pair(hostname,port) ); }, "add_server" ).wait();
  }

  void ntp::set_request_interval( uint32_t interval_sec )
  {
     my->_request_interval_sec = interval_sec;
  }

  void ntp::request_now()
  {
     my->_ntp_thread.async( [=](){ my->request_now(); }, "request_now" ).wait();
  }

  optional<time_point> ntp::get_time()const
  {
     if( my->_last_ntp_delta_initialized )
        return fc::time_point::now() + fc::microseconds(my->_last_ntp_delta_microseconds);
     return optional<time_point>();
  }

}
