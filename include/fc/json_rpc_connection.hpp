#pragma once
#include <fc/json.hpp>
#include <fc/iostream.hpp>
#include <fc/future.hpp>
#include <fc/function.hpp>
#include <fc/ptr.hpp>

namespace fc {  namespace json {
  class rpc_connection;

  struct rpc_server_function : public fc::retainable {
    typedef fc::shared_ptr<rpc_server_function> ptr;
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


    template<typename R, typename Args>
    struct rpc_server_function_impl : public rpc_server_function {
      rpc_server_function_impl( const fc::function<R,Args>& f ):func(f){} 
      virtual value call( const value& v ) {
        return value( func( fc::value_cast<Args>( v ) ) ); 
      }
      fc::function<R,Args> func;
    };

    template<typename InterfaceType>
    struct add_method_visitor {
      public:
        add_method_visitor( const fc::ptr<InterfaceType>& p, fc::json::rpc_connection& c ):_ptr(p){}

        template<typename R, typename Args, typename Type>
        void operator()( const char* name, fc::function<R,Args>& meth, Type );

        const fc::ptr<InterfaceType>& _ptr;
        fc::json::rpc_connection&     _con;
    };
  }

  /**
   *  This class can be used to communicate via json-rpc over a pair of
   *  streams.
   *
   *  @note rpc_connection has reference semantics and all 'copies' will
   *  refer to the same underlying stream.
   */
  class rpc_connection {
    public:
      rpc_connection();
      /** note the life of i and o must be longer than rpc_connection's life */
      rpc_connection( istream& i, ostream& o );
      rpc_connection( rpc_connection&& c );
      rpc_connection( const rpc_connection& c );
      ~rpc_connection();

      rpc_connection& operator=(rpc_connection&& m);
      rpc_connection& operator=(const rpc_connection& m);

      /** note the life of i and o must be longer than rpc_connection's life */
      void init( istream& i, ostream& o );

      template<typename R, typename Args >
      future<R> invoke( const fc::string& method, Args&& a = nullptr  )const {
        auto r = new detail::pending_result_impl<R>();
        typename promise<R>::ptr rtn( r, true );
        invoke( detail::pending_result::ptr(r), method, value(fc::forward<Args>(a)) );
        return rtn;
      }

      template<typename InterfaceType>
      void add_interface( const fc::ptr<InterfaceType>& it ) {
        it->template visit<InterfaceType>( detail::add_method_visitor<InterfaceType>( it, *this ) );
      }

      void add_method( const fc::string& name, const fc::json::rpc_server_function::ptr& func );

    private:
      void invoke( detail::pending_result::ptr&& p, const fc::string& m, const value& param )const;
      class impl;
      fc::shared_ptr<class impl> my;
  };

  namespace detail {

    template<typename InterfaceType>
    template<typename R, typename Args, typename Type>
    void add_method_visitor<InterfaceType>::operator()( const char* name, fc::function<R,Args>& meth, Type ) {
        _con.add_method( name, rpc_server_function::ptr( new rpc_server_function_impl<R,Args>(meth) ) );
    }

  } // namespace detail

} } // fc::json

