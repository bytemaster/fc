#pragma once
#include <fc/rpc/api_connection.hpp>
#include <fc/rpc/state.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>

namespace fc { namespace rpc {

   class websocket_api : public api_connection, public fc::rpc::state, public fc::http::websocket_session
   {
      public:
         websocket_api( fc::http::websocket_connection_ptr c )
         :fc::http::websocket_session(c)
         {
            add_method( "call", [this]( const variants& args ) -> variant {
                      FC_ASSERT( args.size() == 3 && args[2].is_array() );
                      return this->receive_call( args[0].as_uint64(),
                                          args[1].as_string(),
                                          args[2].get_array() );
                                  });
         }

         virtual variant send_call( api_id_type api_id, 
                                    const string& method_name, 
                                    const variants& args = variants() ) override
         {
            auto request = this->start_remote_call(  "call", {api_id, method_name, args} );
            send_message( fc::json::to_string(request) );
            return wait_for_response( *request.id );
         }

      protected:
         virtual void on_message( const std::string& message )
         {
            auto var = fc::json::from_string(message);
            const auto& var_obj = var.get_object();
            if( var_obj.contains( "method" ) )
            {
               auto call = var.as<fc::rpc::request>();
               try {
                  auto result = local_call( call.method, call.params );
                  if( call.id )
                  {
                     send_message( fc::json::to_string( response( *call.id, result ) ) );
                  }
               }
               catch ( const fc::exception& e )
               {
                  if( call.id )
                  {
                     send_message( fc::json::to_string( response( *call.id,  error_object{ 1, e.to_detail_string(), fc::variant(e)}  ) ) );
                  }
               }
            }
            else 
            {
               auto reply = var.as<fc::rpc::response>();
               handle_reply( reply );
            }
         }
   };

} } // namespace fc::rpc
