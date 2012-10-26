#pragma once
#include <fc/shared_impl.hpp>
#include <fc/future.hpp>

namespace fc {
  class istream;
  class ostream;
  class path;
  class string;
  template<typename> class vector;

  /**
   *  @brief start and manage an external process
   *
   *  @note this class implements reference semantics.
   */
  class process  {
    public:
      enum exec_opts {
        open_none   = 0,
        open_stdin  = 0x01, 
        open_stdout = 0x02, 
        open_stderr = 0x04,
        open_all    = open_stdin|open_stdout|open_stderr,
      };
      /**
       *  Return a new process executing the specified exe with the specified args.
       */
      fc::future<int> exec( const fc::path& exe, int opt = open_all );
      fc::future<int> exec( const fc::path& exe, const fc::path& wd, int opt = open_all );
      fc::future<int> exec( const fc::path& exe, fc::vector<fc::string>&& args , int opt = open_all );
      fc::future<int> exec( const fc::path& exe, fc::vector<fc::string>&& args, const fc::path& wd, int opt = open_all  );

      /**
       *  Forcefully kills the process.
       */
      void kill();
      
      /**
       *  @brief returns a stream that writes to the process' stdin
       */
      fc::ostream& in_stream();

      /**
       *  @brief returns a stream that reads from the process' stdout
       */
      fc::istream& out_stream();
      /**
       *  @brief returns a stream that reads from the process' stderr
       */
      fc::istream& err_stream();

      FC_REFERENCE_TYPE(process) 
  };

} // namespace fc
