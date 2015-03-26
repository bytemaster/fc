#include <fc/network/http/websocket.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/thread/thread.hpp>
#include <fc/asio.hpp>

namespace fc { namespace http {

   namespace detail {
      using websocketpp::connection_hdl;
      typedef websocketpp::server<websocketpp::config::asio> websocket_server_type;

      template<typename T>
      class websocket_connection_impl : public websocket_connection
      {
         public:
            websocket_connection_impl( T con )
            :_ws_connection(con){}

            virtual void send_message( const std::string& message )
            {
               _ws_connection->send( message );
            }

            T _ws_connection;
      };


      class websocket_server_impl
      {
         public:
            websocket_server_impl()
            :_server_thread( fc::thread::current() )
            {
               _server.init_asio(&fc::asio::default_io_service());
               _server.set_reuse_addr(true);
               _server.set_open_handler( [&]( connection_hdl hdl ){
                    _server_thread.async( [&](){
                         wlog( "on open server" );
                       auto new_con = std::make_shared<websocket_connection_impl<websocket_server_type::connection_ptr>>( _server.get_con_from_hdl(hdl) );
                       _connections[hdl] = _factory( new_con );
                    }).wait();
               });
               _server.set_message_handler( [&]( connection_hdl hdl, websocket_server_type::message_ptr msg ){
                    _server_thread.async( [&](){
                       auto current_con = _connections.find(hdl);
                       assert( current_con != _connections.end() );
                       wdump(("server")(msg->get_payload()));
                       current_con->second->on_message( msg->get_payload()  );
                    }).wait();
               });
               _server.set_close_handler( [&]( connection_hdl hdl ){
                    _server_thread.async( [&](){
                       _connections.erase( hdl );  
                    }).wait();
               });

               _server.set_fail_handler( [&]( connection_hdl hdl ){
                    _server_thread.async( [&](){
                       _connections.erase( hdl );  
                    }).wait();
               });
            }

            typedef std::map<connection_hdl, websocket_session_ptr,std::owner_less<connection_hdl> > con_map;

            con_map               _connections;
            fc::thread&           _server_thread;
            websocket_server_type _server;
            session_factory       _factory;
      };

      typedef websocketpp::client<websocketpp::config::asio_client> websocket_client_type; 
      typedef websocket_client_type::connection_ptr                 websocket_client_connection_type; 

      class websocket_client_impl 
      {
         public:
            typedef websocket_client_type::message_ptr message_ptr;

            websocket_client_impl()
            :_client_thread( fc::thread::current() )
            {
                _client.set_open_handler( [=]( websocketpp::connection_hdl hdl ){
                   elog( "default open client" );
                });
                _client.set_message_handler( [&]( connection_hdl hdl, message_ptr msg ){
                                             wlog("start wait");
                   _client_thread.async( [&](){
                       wdump((msg->get_payload()));
                      _session->on_message( msg->get_payload() );
                   }).wait();
                                             wlog("done wait");
                });
                _client.set_close_handler( [=]( connection_hdl hdl ){
                                             wlog("start wait");
                   _client_thread.async( [&](){ _session.reset(); } ).wait();
                                             wlog("done wait");
                });
                _client.set_fail_handler( [=]( connection_hdl hdl ){
                                             wlog("start wait");
                   _client_thread.async( [&](){ _session.reset(); } ).wait();
                                             wlog("done wait");
                });
                _client.init_asio( &fc::asio::default_io_service() );
            }
            fc::promise<void>::ptr             _connected;
            fc::thread&                        _client_thread;
            websocket_client_type              _client;
            websocket_session_ptr              _session;
            websocket_connection_ptr           _connection;
      };
   } // namespace detail

   websocket_server::websocket_server():my( new detail::websocket_server_impl() ) {}
   websocket_server::~websocket_server(){}

   void websocket_server::on_connection( const session_factory& factory )
   {
      my->_factory = factory;
   }

   void websocket_server::listen( uint16_t port )
   {
      my->_server.listen(port);
   }

   void websocket_server::start_accept() { 
      my->_server.start_accept(); 
   }

   websocket_client::websocket_client():my( new detail::websocket_client_impl() ) {}
   websocket_client::~websocket_client(){}
   websocket_session_ptr websocket_client::connect( const std::string& uri, const session_factory& factory )
   { try {
       wlog( "connecting to ${uri}", ("uri",uri));
       websocketpp::lib::error_code ec;

       my->_connected = fc::promise<void>::ptr( new fc::promise<void>("websocket::connect") );

       my->_client.set_open_handler( [=]( websocketpp::connection_hdl hdl ){
          auto con =  my->_client.get_con_from_hdl(hdl);
          my->_connection = std::make_shared<detail::websocket_connection_impl<detail::websocket_client_connection_type>>( con );
          my->_session = factory( my->_connection );
          my->_connected->set_value();
       });

       auto con = my->_client.get_connection( uri, ec );
       if( ec )
       {
          FC_ASSERT( !ec, "error: ${e}", ("e",ec.message()) );
       }
       my->_client.connect(con);
       my->_connected->wait();
       return my->_session;
   } FC_CAPTURE_AND_RETHROW( (uri) ) }

} } // fc::http
