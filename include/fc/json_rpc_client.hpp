#pragma once
#include <fc/actor.hpp>
#include <fc/json_rpc_connection.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/facilities/empty.hpp>


namespace fc { namespace json {
        //static std::function<fc::future<R>( BOOST_PP_ENUM_PARAMS(n,A) ) >  

  namespace detail {
    struct rpc_member {
        #define RPC_MEMBER_FUNCTOR(z,n,IS_CONST) \
        template<typename R, typename C, typename P BOOST_PP_ENUM_TRAILING_PARAMS( n, typename A)> \
        static fc::function<fc::future<R> BOOST_PP_ENUM_TRAILING_PARAMS(n,A) > \
            functor( P, R (C::*mem_func)(BOOST_PP_ENUM_PARAMS(n,A)) IS_CONST,  \
                      const rpc_connection::ptr& c = rpc_connection::ptr(), const char* name = nullptr ) { \
            return [=](BOOST_PP_ENUM_BINARY_PARAMS(n,A,a))->fc::future<R>{ \
                return c->invoke<R>( name, make_tuple(BOOST_PP_ENUM_PARAMS(n,a)) ); }; \
        } 
        BOOST_PP_REPEAT( 8, RPC_MEMBER_FUNCTOR, const )
        BOOST_PP_REPEAT( 8, RPC_MEMBER_FUNCTOR, BOOST_PP_EMPTY() )
        #undef RPC_MEMBER_FUNCTOR
    };

    struct vtable_visitor {
        vtable_visitor( rpc_connection::ptr& c ):_con(c){}
    
        template<typename Function, typename MemberPtr>
        void operator()( const char* name, Function& memb, MemberPtr m )const {
          memb = rpc_member::functor( nullptr, m, _con, name );
        }
        rpc_connection::ptr& _con;
    };
  };


  template<typename InterfaceType>
  class rpc_client : public ptr<InterfaceType,fc::json::detail::rpc_member> {
    public:
      rpc_client(){}
      rpc_client( const rpc_connection::ptr& c ):_con(c){
        init();
      }
      rpc_client( const rpc_client& c ):_con(c._con){}

      void set_connection( const rpc_connection::ptr& c ) {
        _con = c;
        init();
      }
      const rpc_connection& connection()const { return _con; }

    private:
      void init() {
          this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::json::detail::rpc_member>() );
          this->_vtable->template visit_other<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
      }
      rpc_connection::ptr  _con; 
  };

} } // fc::json
