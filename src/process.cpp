#include <fc/process.hpp>
#include <fc/iostream.hpp>
#include <fc/iostream_wrapper.hpp>
#include <fc/asio.hpp>
#include <fc/filesystem.hpp>
#include <fc/vector.hpp>
#include <boost/process.hpp>
#include <boost/iostreams/stream.hpp>

namespace fc {

  namespace bp = boost::process;
  namespace io = boost::iostreams;

  class process_sink : public io::sink {
    public:
      struct category : io::sink::category, io::flushable_tag {};
      typedef char      type;

      process_sink( std::shared_ptr<bp::pipe>& p ):m_in(p){}
  
      std::streamsize write( const char* s, std::streamsize n ) {
       if( !m_in ) return -1;
       return static_cast<std::streamsize>(fc::asio::write( *m_in, 
                                boost::asio::const_buffers_1( s, static_cast<size_t>(n) ) ));
      }
      void close() { if(m_in) m_in->close(); }
      bool flush() { return true; }
  
    private:
      std::shared_ptr<bp::pipe>&      m_in;
  };

  class process_source : public io::source {
    public:
      typedef char      type;

      process_source(  std::shared_ptr<bp::pipe>& pi )
      :m_pi(pi){}

      std::streamsize read( char* s, std::streamsize n ) {
        if( !m_pi ) return -1;
        try {
            return static_cast<std::streamsize>(fc::asio::read_some( *m_pi, boost::asio::buffer( s, static_cast<size_t>(n) ) ));
        } catch ( const boost::system::system_error& e ) {
         // wlog( "%s", fc::except_str().c_str() );
          if( e.code() == boost::asio::error::eof ) 
              return -1;
          wlog( "%s", fc::except_str().c_str() );
          throw;
        } catch ( ... ) {
          //wlog( "%s", fc::except_str().c_str() );
          return -1;
        }
      }
    private:
      std::shared_ptr<bp::pipe>& m_pi;
  };
} // namespace fc

FC_START_SHARED_IMPL( fc::process )
  public:
  impl()
  :stat( fc::asio::default_io_service() ),
    std_out(process_source(outp)),
   std_err(process_source(errp)),
   std_in(process_sink(inp)),
   _ins(std_in),
   _outs(std_out),
   _errs(std_err){}

  ~impl() {
    try {
      if( inp ) {
        inp->close();
      }
      if( _exited.valid() && !_exited.ready()) {
         //child->terminate();
         _exited.wait();
      }
    }catch(...) {
      wlog( "caught exception cleaning up process: %s", fc::except_str().c_str() );
    }
  }

  std::shared_ptr<bp::child> child;
  std::shared_ptr<bp::pipe>  outp;
  std::shared_ptr<bp::pipe>  errp;
  std::shared_ptr<bp::pipe>  inp;

  bp::status                 stat;
  bp::context                pctx;

  // provide useful buffering 
  io::stream<fc::process_source>   std_out;
  io::stream<fc::process_source>   std_err;
  io::stream<fc::process_sink>     std_in;

  fc::future<int>                  _exited;

  // adapt to ostream and istream interfaces
  fc::ostream_wrapper              _ins;
  fc::istream_wrapper              _outs;
  fc::istream_wrapper              _errs;
FC_END_SHARED_IMPL
#include <fc/shared_impl.cpp>

namespace fc {

FC_REFERENCE_TYPE_IMPL( process )


fc::future<int> process::exec( const fc::path& exe, fc::vector<fc::string>&& args, 
                                            const fc::path& work_dir, int opt  ) {

  my->pctx.work_dir = work_dir.string();
    
  if( opt&open_stdout)
      my->pctx.streams[boost::process::stdout_id] = bp::behavior::async_pipe();
  else 
      my->pctx.streams[boost::process::stdout_id] = bp::behavior::null();


  if( opt& open_stderr )
      my->pctx.streams[boost::process::stderr_id] = bp::behavior::async_pipe();
  else
      my->pctx.streams[boost::process::stderr_id] = bp::behavior::null();

  if( opt& open_stdout )
      my->pctx.streams[boost::process::stdin_id]  = bp::behavior::async_pipe();
  else
      my->pctx.streams[boost::process::stdin_id]  = bp::behavior::close();

  std::vector<std::string> a;
  a.reserve(size_t(args.size()));
  for( uint32_t i = 0; i < args.size(); ++i ) {
    a.push_back( args[i] ); 
  }
  my->child.reset( new bp::child( bp::create_child( exe.string(), fc::move(a), my->pctx ) ) );

  if( opt & open_stdout ) {
     bp::handle outh = my->child->get_handle( bp::stdout_id );
     my->outp.reset( new bp::pipe( fc::asio::default_io_service(), outh.release() ) );
  }
  if( opt & open_stderr ) {
     bp::handle errh = my->child->get_handle( bp::stderr_id );
     my->errp.reset( new bp::pipe( fc::asio::default_io_service(), errh.release() ) );
  }
  if( opt & open_stdin ) {
     bp::handle inh  = my->child->get_handle( bp::stdin_id );
     my->inp.reset(  new bp::pipe( fc::asio::default_io_service(), inh.release()  ) );
  }


  promise<int>::ptr p(new promise<int>("process"));
  my->stat.async_wait(  my->child->get_id(), [=]( const boost::system::error_code& ec, int exit_code )
    {
      //slog( "process::result %d", exit_code );
      if( !ec ) {
          #ifdef BOOST_POSIX_API
          try {
             if( WIFEXITED(exit_code) )
                 p->set_value(  WEXITSTATUS(exit_code) );
             else {
                 FC_THROW_MSG( "process exited with: %s ", strsignal(WTERMSIG(exit_code)) );
             }
          } catch ( ... ) {
             p->set_exception( fc::current_exception() ); 
          }
          #else
          p->set_value(exit_code);
          #endif
       }
       else p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
    });
  return my->_exited = p;
}

/**
 *  Forcefully kills the process.
 */
void process::kill() {
  my->child->terminate();
}

/**
 *  @brief returns a stream that writes to the process' stdin
 */
fc::ostream& process::in_stream() {
  return my->_ins;
}

/**
 *  @brief returns a stream that reads from the process' stdout
 */
fc::istream& process::out_stream() {
  return my->_outs;
}
/**
 *  @brief returns a stream that reads from the process' stderr
 */
fc::istream& process::err_stream() {
  return my->_errs;
}

}
