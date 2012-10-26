#pragma once
#include <fc/actor.hpp>
#include <fc/json_rpc_connection.hpp>


namespace fc { namespace json {

  namespace detail {
    struct rpc_member {
        // TODO: expand for all method arity and constness....
        template<typename R, typename C, typename A1, typename P>
        static fc::function<fc::future<R>,A1> functor( P, 
                                                        R (C::*mem_func)(A1),
                                                        const rpc_connection& c = rpc_connection(), 
                                                        const char* name = nullptr 
                                                        ) {
          return [=](A1 a1)->fc::future<R>{ return c.invoke<R>( name, make_tuple(a1) ); };
        }
    };


    struct vtable_visitor {
        vtable_visitor( rpc_connection& c ):_con(c){}
    
        template<typename Function, typename MemberPtr>
        void operator()( const char* name, Function& memb, MemberPtr m )const {
          memb = rpc_member::functor( nullptr, m, _con, name );
        }
        rpc_connection& _con;
    };

  };


  template<typename InterfaceType>
  class rpc_client : public ptr<InterfaceType,fc::json::detail::rpc_member> {
    public:
      rpc_client(){}
      rpc_client( const rpc_connection& c ):_con(c){
        init();
      }
      rpc_client( const rpc_client& c ):_con(c._con){}

      void set_connection( const rpc_connection& c ) {
        _con = c;
        init();
      }
      const rpc_connection& connection()const { return _con; }

    private:
      void init() {
          this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::json::detail::rpc_member>() );
          this->_vtable->template visit<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
      }
      rpc_connection  _con; 
  };

} } // fc::json
