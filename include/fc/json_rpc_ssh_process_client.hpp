#pragma once
#include <fc/json_rpc_client.hpp>
#include <fc/json_rpc_stream_connection.hpp>
#include <fc/ssh/process.hpp>

namespace fc { namespace json {

  template<typename InterfaceType>
  class rpc_ssh_process_client : public rpc_client<InterfaceType> {
    public:
      rpc_ssh_process_client(){}
      bool valid()const { return proc.valid(); }

      rpc_ssh_process_client( const fc::ssh::process& proc ) 
      :rpc_client<InterfaceType>(rpc_connection::ptr(new fc::json::rpc_stream_connection( proc.out_stream(), proc.in_stream() ) ) ){}

      rpc_ssh_process_client& operator = ( const fc::ssh::process& proc )  {
         this->set_connection( rpc_connection::ptr(new fc::json::rpc_stream_connection( proc.out_stream(), proc.in_stream() ) ) );
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
