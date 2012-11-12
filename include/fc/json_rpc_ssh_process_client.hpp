#pragma once
#include <fc/json_rpc_client.hpp>
#include <fc/ssh/process.hpp>

namespace fc { namespace json {

  template<typename InterfaceType>
  class rpc_ssh_process_client : public ptr<InterfaceType,fc::json::detail::rpc_member> {
    public:
      rpc_ssh_process_client(){}
      bool valid()const { return proc.valid(); }

      rpc_ssh_process_client( const fc::ssh::process& proc ) 
      {
         con.reset( new fc::json::rpc_stream_connection( proc.out_stream(), proc.in_stream() ) );
         this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::json::detail::rpc_member>() );
         this->_vtable->template visit<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
      }

      rpc_ssh_process_client& operator = ( const fc::ssh::process& proc )  {
         con.reset( new fc::json::rpc_stream_connection( proc.out_stream(), proc.in_stream() ) );
         this->_vtable.reset(new fc::detail::vtable<InterfaceType,fc::json::detail::rpc_member>() );
         this->_vtable->template visit<InterfaceType>( fc::json::detail::vtable_visitor(_con) );
         return *this;
      }

      /**
       *  @brief returns a stream that reads from the process' stderr
       */
      fc::istream& err_stream() { return proc.err_stream(); }

      fc::ssh::process& get_ssh_process() { return proc; }
    private:
      fc::ssh::process        proc;
      fc::json::rpc_stream_connection::ptr _con;

  };
} }
