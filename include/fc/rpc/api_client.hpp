#pragma once
#include <fc/rpc/api_server.hpp>

namespace fc {

   template<typename Interface, typename Adaptor = identity_member>
   class api_client  : public api<Interface,Adaptor>
   {
      public:
         api_client( fc::api<api_server> server, uint64_t api_id = 0 )
         {
            (*this)->visit( api_visitor(this, api_id, std::make_shared<api<api_server>>(server)) ); 
         }

      private:
         struct api_visitor
         {
            api_client* _client = nullptr;
            uint32_t    _api_id;
            std::shared_ptr<api<api_server>> _server;

            api_visitor( api_client* c, uint32_t api_id, std::shared_ptr<api<api_server>> serv )
            :_client(c),_api_id(api_id),_server(std::move(serv))
            { wdump((int64_t(c))); }

            api_visitor() = delete;

            template<typename Result>
            static Result from_variant( const variant& v, Result&&, const std::shared_ptr<api<api_server>>&  )
            {
               return v.as<Result>();
            }

            template<typename ResultInterface, typename ResultAdaptor>
            static fc::api<ResultInterface,ResultAdaptor> from_variant( const variant& v, fc::api<ResultInterface,ResultAdaptor>&&,
                                                                 const std::shared_ptr<api<api_server>>&  serv
                                                                 )
            {
               return api_client<ResultInterface,ResultAdaptor>( serv, v.as_uint64() );
            }


            template<typename Result, typename... Args>
            void operator()( const char* name, std::function<Result(Args...)>& memb )const 
            {
                auto serv   = _server;
                auto api_id = _api_id;
                memb = [serv,api_id,name]( Args... args ) {
                   vtable<api_server,identity_member> test;
                    auto var_result = (*serv)->call( api_id, name, {args...} );
                    return from_variant( var_result, Result(), serv );
                };
            }
         };
   };
}
