#include <fc/network/http/websocket.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/logger/stub.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/thread/thread.hpp>
#include <fc/asio.hpp>

namespace fc { namespace http {

   namespace detail {

      struct asio_with_stub_log : public websocketpp::config::asio {

          typedef asio_with_stub_log type;
          typedef asio base;

          typedef base::concurrency_type concurrency_type;

          typedef base::request_type request_type;
          typedef base::response_type response_type;

          typedef base::message_type message_type;
          typedef base::con_msg_manager_type con_msg_manager_type;
          typedef base::endpoint_msg_manager_type endpoint_msg_manager_type;

          /// Custom Logging policies
          /*typedef websocketpp::log::syslog<concurrency_type,
              websocketpp::log::elevel> elog_type;
          typedef websocketpp::log::syslog<concurrency_type,
              websocketpp::log::alevel> alog_type;
          */
          //typedef base::alog_type alog_type;
          //typedef base::elog_type elog_type;
          typedef websocketpp::log::stub elog_type;
          typedef websocketpp::log::stub alog_type;

          typedef base::rng_type rng_type;

          struct transport_config : public base::transport_config {
              typedef type::concurrency_type concurrency_type;
              typedef type::alog_type alog_type;
              typedef type::elog_type elog_type;
              typedef type::request_type request_type;
              typedef type::response_type response_type;
              typedef websocketpp::transport::asio::basic_socket::endpoint
                  socket_type;
          };

          typedef websocketpp::transport::asio::endpoint<transport_config>
              transport_type;
          
          static const long timeout_open_handshake = 0;
      };




      using websocketpp::connection_hdl;
      typedef websocketpp::server<asio_with_stub_log> websocket_server_type;

      template<typename T>
      class websocket_connection_impl : public websocket_connection
      {
         public:
            websocket_connection_impl( T con )
            :_ws_connection(con){}

            virtual void send_message( const std::string& message )override
            {
               _ws_connection->send( message );
            }
            virtual void close( int64_t code, const std::string& reason  )override
            {
               _ws_connection->close(code,reason);
            }

            T _ws_connection;
      };

      class websocket_server_impl
      {
         public:
            websocket_server_impl()
            :_server_thread( fc::thread::current() )
            {
               _server.clear_access_channels( websocketpp::log::alevel::all );
               _server.init_asio(&fc::asio::default_io_service());
               _server.set_reuse_addr(true);
               _server.set_open_handler( [&]( connection_hdl hdl ){
                    _server_thread.async( [&](){
                       auto new_con = std::make_shared<websocket_connection_impl<websocket_server_type::connection_ptr>>( _server.get_con_from_hdl(hdl) );
                       _on_connection( _connections[hdl] = new_con );
                    }).wait();
               });
               _server.set_message_handler( [&]( connection_hdl hdl, websocket_server_type::message_ptr msg ){
                    _server_thread.async( [&](){
                       auto current_con = _connections.find(hdl);
                       assert( current_con != _connections.end() );
                       //wdump(("server")(msg->get_payload()));
                       current_con->second->on_message( msg->get_payload()  );
                    }).wait();
               });
               _server.set_close_handler( [&]( connection_hdl hdl ){
                    _server_thread.async( [&](){
                       _connections.erase( hdl );  
                    }).wait();
               });

               _server.set_fail_handler( [&]( connection_hdl hdl ){
                    if( _server.is_listening() )
                    {
                       _server_thread.async( [&](){
                          _connections.erase( hdl );  
                       }).wait();
                    }
               });
            }
            ~websocket_server_impl()
            {
               if( _server.is_listening() )
                  _server.stop_listening();
               auto cpy_con = _connections;
               for( auto item : cpy_con )
                  _server.close( item.first, 0, "server exit" );
            }

            typedef std::map<connection_hdl, websocket_connection_ptr,std::owner_less<connection_hdl> > con_map;

            con_map                  _connections;
            fc::thread&              _server_thread;
            websocket_server_type    _server;
            on_connection_handler    _on_connection;
            fc::promise<void>::ptr   _closed;
      };

      typedef websocketpp::client<asio_with_stub_log> websocket_client_type; 
      typedef websocket_client_type::connection_ptr  websocket_client_connection_type; 

      class websocket_client_impl 
      {
         public:
            typedef websocket_client_type::message_ptr message_ptr;

            websocket_client_impl()
            :_client_thread( fc::thread::current() )
            {
                _client.clear_access_channels( websocketpp::log::alevel::all );
                _client.set_message_handler( [&]( connection_hdl hdl, message_ptr msg ){
                   _client_thread.async( [&](){
                       // wdump((msg->get_payload()));
                      _connection->on_message( msg->get_payload() );
                   }).wait();
                });
                _client.set_close_handler( [=]( connection_hdl hdl ){
                   _client_thread.async( [&](){ _connection.reset(); } ).wait();
                   if( _closed ) _closed->set_value();
                });
                _client.set_fail_handler( [=]( connection_hdl hdl ){
                   auto con = _client.get_con_from_hdl(hdl);
                   auto message = con->get_ec().message();
                      _client_thread.async( [&](){ _connection.reset(); } ).wait();
                   if( _connected && !_connected->ready() ) 
                       _connected->set_exception( exception_ptr( new FC_EXCEPTION( exception, "${message}", ("message",message)) ) );
                   if( _closed ) 
                       _closed->set_value();
                });
                _client.init_asio( &fc::asio::default_io_service() );
            }
            ~websocket_client_impl()
            {
               if(_connection )
               {
                  _connection->close(0, "client closed");
                  _closed->wait();
               }
            }
            fc::promise<void>::ptr             _connected;
            fc::promise<void>::ptr             _closed;
            fc::thread&                        _client_thread;
            websocket_client_type              _client;
            websocket_connection_ptr           _connection;
      };
   } // namespace detail

   websocket_server::websocket_server():my( new detail::websocket_server_impl() ) {}
   websocket_server::~websocket_server(){}

   void websocket_server::on_connection( const on_connection_handler& handler )
   {
      my->_on_connection = handler;
   }

   void websocket_server::listen( uint16_t port )
   {
      my->_server.listen(port);
   }
   void websocket_server::listen( const fc::ip::endpoint& ep )
   {
      my->_server.listen( boost::asio::ip::tcp::endpoint( boost::asio::ip::address_v4(uint32_t(ep.get_address())),ep.port()) );
   }

   void websocket_server::start_accept() { 
      my->_server.start_accept(); 
   }

   websocket_client::websocket_client():my( new detail::websocket_client_impl() ) {}
   websocket_client::~websocket_client(){ }
   websocket_connection_ptr websocket_client::connect( const std::string& uri )
   { try {
       // wlog( "connecting to ${uri}", ("uri",uri));
       websocketpp::lib::error_code ec;

       my->_connected = fc::promise<void>::ptr( new fc::promise<void>("websocket::connect") );

       my->_client.set_open_handler( [=]( websocketpp::connection_hdl hdl ){
          auto con =  my->_client.get_con_from_hdl(hdl);
          my->_connection = std::make_shared<detail::websocket_connection_impl<detail::websocket_client_connection_type>>( con );
          my->_closed = fc::promise<void>::ptr( new fc::promise<void>("websocket::closed") );
          my->_connected->set_value();
       });

       auto con = my->_client.get_connection( uri, ec );
       if( ec )
       {
          FC_ASSERT( !ec, "error: ${e}", ("e",ec.message()) );
       }
       my->_client.connect(con);
       my->_connected->wait();
       return my->_connection;
   } FC_CAPTURE_AND_RETHROW( (uri) ) }

} } // fc::http
