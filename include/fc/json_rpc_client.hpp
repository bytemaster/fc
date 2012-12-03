#pragma once
#include <fc/actor.hpp>
#include <fc/json_rpc_connection.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/facilities/empty.hpp>


namespace fc { namespace json {

  namespace detail {
    struct rpc_member {
       #ifdef BOOST_NO_VARIADIC_TEMPLATES
         #define RPC_MEMBER_FUNCTOR(z,n,IS_CONST) \
         template<typename R, typename C, typename P BOOST_PP_ENUM_TRAILING_PARAMS( n, typename A)> \
         static std::function<fc::future<R>( BOOST_PP_ENUM_PARAMS(n,A) ) > \
             functor( P, R (C::*mem_func)(BOOST_PP_ENUM_PARAMS(n,A)) IS_CONST,  \
                       const rpc_connection::ptr& c = rpc_connection::ptr(), const char* name = nullptr ) { \
             return [=](BOOST_PP_ENUM_BINARY_PARAMS(n,A,a))->fc::future<R>{ \
                 return c->invoke<R>( name, make_tuple(BOOST_PP_ENUM_PARAMS(n,a)) ); }; \
         } 
         BOOST_PP_REPEAT( 8, RPC_MEMBER_FUNCTOR, const )
         BOOST_PP_REPEAT( 8, RPC_MEMBER_FUNCTOR, BOOST_PP_EMPTY() )
         #undef RPC_MEMBER_FUNCTOR

       #else
         template<typename R, typename C, typename P, typename... Args>
         static std::function<fc::future<R>(Args...)> functor( P&& p, R (C::*mem_func)(Args...), 
                       const rpc_connection::ptr& c = rpc_connection::ptr(), const char* name = nullptr ) { 
            return [=](Args... args)->fc::future<R>{ 
                return c->invoke<R>( name, make_tuple(args...) ); }; 
         }
         template<typename R, typename C, typename P, typename... Args>
         static std::function<fc::future<R>(Args...)> functor( P&& p, R (C::*mem_func)(Args...)const,
                       const rpc_connection::ptr& c = rpc_connection::ptr(), const char* name = nullptr ) { 
            return [=](Args... args)->fc::future<R>{ 
                return c->invoke<R>( name, make_tuple(args...) ); }; 
         }
       #endif
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
  class rpc_client : public actor<InterfaceType> { //ptr<InterfaceType,fc::json::detail::rpc_member> {
    public:
      rpc_client(){}
      rpc_client( const rpc_connection::ptr& c ){ set_connection(c); }
      //rpc_client( const rpc_client& c ):_con(c._con){}

      void set_connection( const rpc_connection::ptr& c ) {
        _con = c;
        this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::detail::actor_member>() );
        this->_vtable->template visit_other<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
      }
      const rpc_connection& connection()const { return _con; }

    private:
      rpc_connection::ptr  _con; 
  };

} } // fc::json
