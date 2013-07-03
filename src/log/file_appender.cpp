#include <fc/log/file_appender.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/io/fstream.hpp>
#include <fc/variant.hpp>
#include <fc/reflect/variant.hpp>
#include <iomanip>
#include <sstream>


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

   // MS THREAD METHOD  MESSAGE \t\t\t File:Line
   void file_appender::log( const log_message& m )
   {
      std::stringstream line;
      line << (m.get_context().get_timestamp().time_since_epoch().count() % (1000ll*1000ll*60ll*60))/1000 <<"ms ";
      line << std::setw( 10 ) << m.get_context().get_thread_name().substr(0,9).c_str() <<" ";

      auto me = m.get_context().get_method();
      // strip all leading scopes...
      if( me.size() )
      {
         uint32_t p = 0;
         for( uint32_t i = 0;i < me.size(); ++i )
         {
             if( me[i] == ':' ) p = i;
         }

         if( me[p] == ':' ) ++p;
         line << std::setw( 20 ) << m.get_context().get_method().substr(p,20).c_str() <<" ";
      }
      line << "] ";
      fc::string message = fc::format_string( m.get_format(), m.get_data() );
      line << message.c_str();


      //fc::variant lmsg(m);

     // fc::string fmt_str = fc::format_string( my->cfg.format, mutable_variant_object(m.get_context())( "message", message)  );
      {
        fc::scoped_lock<boost::mutex> lock(my->slock);
        my->out << line.str() << "\t\t\t" << m.get_context().get_file() <<":"<<m.get_context().get_line_number()<<"\n";
      }
      if( my->cfg.flush ) my->out.flush();
   }
}
