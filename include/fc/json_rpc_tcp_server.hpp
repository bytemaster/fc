#pragma once
#include <fc/json_rpc_connection.hpp>

namespace fc {

  namespace json {
      class rpc_tcp_server {
        private:
          template<typename Interface>
          struct add_method_visitor {
            add_method_visitor( fc::ptr<Interface>& p, rpc_connection& s ):_ptr(p),_rpcc(s) { }

            template<typename Functor>
            void operator()( const char* name, Functor& fun ) {
              _rpcc.add_method( name, fun );
            }

            fc::ptr<Interface>& _ptr;
            rpc_connection&     _rpcc;
          };

        public:
          rpc_tcp_server();
          ~rpc_tcp_server();

          template<typename Interface>
          void add_interface( const fc::ptr<Interface>& ptr ) {
            on_new_connection( [=]( rpc_connection& c ) {
              ptr->visit( detail::add_method_visitor<Interface>( ptr, c ) );
            });
          }

          void on_new_connection( const fc::function<void,rpc_connection&>& c );

          void listen( uint16_t port );
        
        private:
          class impl;
          impl* my;
      };
  }
}
