#pragma once
#include <fc/json.hpp>
#include <fc/future.hpp>
#include <fc/function.hpp>
#include <fc/ptr.hpp>

namespace fc {  namespace json {
  class rpc_connection;

  struct rpc_server_method : public fc::retainable {
    typedef fc::shared_ptr<rpc_server_method> ptr;
    virtual value call( const value& v ) = 0;
  };

  namespace detail {
    struct pending_result : virtual public promise_base {
      typedef shared_ptr<pending_result> ptr;
      virtual void handle_result( const fc::value& ) = 0;
      void handle_error( const fc::string& );
      int64_t             id;
      pending_result::ptr next;
      protected:
        ~pending_result(){}
    };
    template<typename T>
    struct pending_result_impl : virtual public promise<T>, virtual public pending_result {   
       virtual void handle_result( const fc::value& s ) {
          this->set_value( value_cast<T>(s) );
       }
      protected:
        ~pending_result_impl(){}
    };
    template<>
    struct pending_result_impl<void> : virtual public promise<void>, virtual public pending_result {   
       virtual void handle_result( const fc::value& ) {
          set_value();
       }
      protected:
        ~pending_result_impl(){}
    };

    template<typename T>
    struct named_param {
      typedef fc::false_type type;
      static T cast( const value& v ) { return fc::value_cast<T>(v); }

      template<typename X>
      static value to_value( X&& x ) { return value(fc::forward<X>(x)); }
    };

    #define FC_JSON_NAMED_PARAMS( T ) \
    namespace fc { namespace json {namespace detail { \
    template<typename T> \
    struct named_param< fc::tuple<T> > { \
      typedef fc::true_type type; \
      static tuple<T> cast( const value& v ) { return make_tuple(fc::value_cast<T>(v)); } \
      template<typename X> \
      static value to_value( X&& x ) { return value( x.a0 ); }\
    }; \
    template<typename T> \
    struct named_param< fc::tuple<T&> > { \
      typedef fc::true_type type; \
      static tuple<T> cast( const value& v ) { return make_tuple(fc::value_cast<T>(v)); } \
      template<typename X> \
      static value to_value( X&& x ) { return value( x.a0 ); }\
    }; \
    } } }

    template<typename R, typename ArgsTuple, typename Signature>
    struct rpc_server_method_impl : public rpc_server_method {
      rpc_server_method_impl( const std::function<Signature>& f ):func(f){} 
      virtual value call( const value& v ) {
        return value(  call_fused(func, named_param<ArgsTuple>::cast(v) ) ); 
      }
      std::function<Signature> func;
    };

    template<typename InterfaceType>
    struct add_method_visitor {
      public:
        add_method_visitor( const fc::ptr<InterfaceType>& p, fc::json::rpc_connection& c ):_ptr(p),_con(c){}

        template<typename R>
        void operator()( const char* name, std::function<R()>& meth);
        template<typename R, typename A1>
        void operator()( const char* name, std::function<R(A1)>& meth);
        template<typename R, typename A1, typename A2>
        void operator()( const char* name, std::function<R(A1,A2)>& meth);

        const fc::ptr<InterfaceType>& _ptr;
        fc::json::rpc_connection&     _con;
    };
  }

  /**
   *  This is the base JSON RPC connection that handles the protocol
   *  level issues.  It does not implement a transport which should
   *  be provided separately and use the handle_message and set_send_delegate
   *  methods to manage the protocol.
   */
  class rpc_connection : public fc::retainable {
    public:
      rpc_connection();
      rpc_connection(const rpc_connection&);
      rpc_connection(rpc_connection&&);
      ~rpc_connection();
      rpc_connection& operator=(const rpc_connection&);
      rpc_connection& operator=(rpc_connection&&);

      typedef fc::shared_ptr<rpc_connection> ptr;

      void  cancel_pending_requests();

      template<typename R, typename Args >
      future<R> invoke( const fc::string& method, Args&& a = nullptr  ){
        auto r = new detail::pending_result_impl<R>();
        typename promise<R>::ptr rtn( r, true );
        invoke( detail::pending_result::ptr(r), method, 
            value(detail::named_param<typename fc::deduce<Args>::type>::to_value(a)) );
        return rtn;
      }

      template<typename InterfaceType>
      void add_interface( const fc::ptr<InterfaceType>& it ) {
        it->template visit<InterfaceType>( detail::add_method_visitor<InterfaceType>( it, *this ) );
      }

      void add_method( const fc::string& name, const fc::json::rpc_server_method::ptr& func );

    protected:
      void         handle_message( const value& m );
      virtual void send_invoke( uint64_t id, const fc::string& m, value&& param ) = 0;
      virtual void send_error( uint64_t id, int64_t code, const fc::string& msg ) = 0;
      virtual void send_result( uint64_t id, value&& r ) = 0;


    private:
      void invoke( detail::pending_result::ptr&& p, const fc::string& m, value&& param );
      void add_method( const fc::string& name, rpc_server_method::ptr&& m );
      template<typename InterfaceType>
      friend struct detail::add_method_visitor;

      class impl;
      fc::shared_ptr<class impl> my;
  };

  namespace detail {

    template<typename InterfaceType>
    template<typename R, typename A1>
    void add_method_visitor<InterfaceType>::operator()( const char* name, std::function<R(A1)>& meth)    {
        _con.add_method( name, rpc_server_method::ptr( new rpc_server_method_impl<R,tuple<A1>,R(A1) >(meth) ) );
    }
    template<typename InterfaceType>
    template<typename R, typename A1, typename A2>
    void add_method_visitor<InterfaceType>::operator()( const char* name, std::function<R(A1,A2)>& meth)    {
        _con.add_method( name, rpc_server_method::ptr( new rpc_server_method_impl<R,tuple<A1,A2>,R(A1,A2) >(meth) ) );
    }
    template<typename InterfaceType>
    template<typename R>
    void add_method_visitor<InterfaceType>::operator()( const char* name, std::function<R()>& meth)    {
        _con.add_method( name, rpc_server_method::ptr( new rpc_server_method_impl<R,tuple<>,R() >(meth) ) );
    }

  } // namespace detail

} } // fc::json

