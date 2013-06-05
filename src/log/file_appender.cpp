#include <fc/log/file_appender.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/io/fstream.hpp>
#include <fc/variant.hpp>
#include <fc/reflect/variant.hpp>


namespace fc {
   class file_appender::impl : public fc::retainable {
      public:
         config                      cfg;
         ofstream                    out;
         boost::mutex        slock;
   };
   file_appender::config::config( const fc::path& p  )
   :format( "${timestamp} ${thread_name} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
   filename(p),flush(true),truncate(true){}

   file_appender::file_appender( const variant& args )
   :my( new impl() )
   {
      try {
         my->cfg = args.as<config>(); 
         my->out.open( my->cfg.filename.string().c_str() );
      } catch ( ... ) {
         //elog( "%s", fc::except_str().c_str() );
      }
   }
   file_appender::~file_appender(){}

   void file_appender::log( const log_message& m )
   {
      fc::string message = fc::format_string( m.get_format(), m.get_data() );
      fc::variant lmsg(m);

      fc::string fmt_str = fc::format_string( my->cfg.format, mutable_variant_object(lmsg.get_object())( "message", message)  );
      {
        fc::scoped_lock<boost::mutex> lock(my->slock);
        my->out << fmt_str << "\n";
      }
      if( my->cfg.flush ) my->out.flush();
   }
}
