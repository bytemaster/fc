#include <fc/file_appender.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/value.hpp>
#include <fc/value_cast.hpp>
#include <fc/fstream.hpp>

namespace fc {
   class file_appender::impl : public fc::retainable {
      public:
         config                      cfg;
         ofstream                    out;
         boost::mutex        slock;
   };
   file_appender::config::config( const fc::path& p  )
   :format( "${when} ${thread} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
   filename(p),flush(true),truncate(true){}

   file_appender::file_appender( const value& args )
   :my( new impl() )
   {
      try {
         my->cfg = fc::value_cast<config>(args);
         my->out.open( my->cfg.filename.string().c_str() );
      } catch ( ... ) {
         elog( "%s", fc::except_str().c_str() );
      }
   }
   file_appender::~file_appender(){}

   void file_appender::log( const log_message& m )
   {
      fc::string message = fc::substitute( m.format, m.args );
      fc::value lmsg(m);

      fc::string fmt_str = fc::substitute( my->cfg.format, value(m).set( "message", message)  );
      {
        fc::scoped_lock<boost::mutex> lock(my->slock);
        my->out << fmt_str << "\n";
      }
      if( my->cfg.flush ) my->out.flush();
   }
}
