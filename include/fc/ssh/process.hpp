#pragma once
#include <fc/shared_ptr.hpp>
#include <fc/string.hpp>

namespace fc { 

  class istream;
  class ostream;

  namespace ssh {

  class client;

  namespace detail {
    class process_impl;
  };

  /**
   *  Enables communication with a process executed via 
   *  client::exec().  
   *
   *  Process can only be created by mace::ssh::client.
   */
  class process { //: public fc::retainable {
    public:
      //typedef fc::shared_ptr<process> ptr;

      process( const process& p );
      process( process&& p );
      process();
      ~process();

      bool valid()const;

      /**
       *  Blocks until the result code of the process has been returned.
       */
      int result();
      /**
       *  @brief returns a stream that writes to the procss' stdin
       */
      fc::ostream& in_stream()const;
      /**
       *  @brief returns a stream that reads from the process' stdout
       */
      fc::istream& out_stream()const;
      /**
       *  @brief returns a stream that reads from the process' stderr
       */
      fc::istream& err_stream()const;

      process& operator=( const process& p );
    private:
      friend class client;
      process( client& c, const fc::string& cmd, const fc::string& pty_type = fc::string() );

      fc::shared_ptr<detail::process_impl> my;
  };
} } // fc::ssh
