#pragma once
#include <fc/json.hpp>
#include <fc/future.hpp>
#include <fc/function.hpp>

namespace fc {  namespace json {
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

    struct server_method : public fc::retainable {
       typedef fc::shared_ptr<server_method> ptr;
       virtual value call( const value& param ) = 0;
    };
    template<typename R, typename Args>
    struct server_method_impl : public server_method {
       server_method_impl( const fc::function<R,Args>& a ):func(a){};
       virtual value call( const value& param ) {
          return value( func( value_cast<Args>(param) ) );
       }
       fc::function<R,Args> func;
    };

  } // namespace detail 

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
        invoke( detail::pending_result::ptr(r), method, value(fc::forward<Args>(a)) );
        return rtn;
      }

      template<typename R, typename Args >
      void add_method( const fc::string& name, const fc::function<R,Args>& a ) {
         this->add_method( name, detail::server_method::ptr(new detail::server_method_impl<R,Args>(a) ) );
      }

    protected:
      void         handle_message( const value& m );
      virtual void send_invoke( uint64_t id, const fc::string& m, value&& param );
      virtual void send_error( uint64_t id, int64_t code, const fc::string& msg );
      virtual void send_result( uint64_t id, value&& r );

    private:
      void invoke( detail::pending_result::ptr&& p, const fc::string& m, value&& param );
      void add_method( const fc::string& name, detail::server_method::ptr&& m );

      class impl;
      fc::shared_ptr<class impl> my;
  };

} } // fc::json

