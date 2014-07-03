#include <fc/compress/lzma.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/variant.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <iomanip>
#include <sstream>

namespace fc {
   class file_appender::impl : public fc::retainable {
      public:
         config              cfg;
         ofstream            out;
         boost::mutex        slock;
         time_point_sec      current_file_start_time;

         time_point_sec get_file_start_time( const time_point_sec& timestamp, const microseconds& interval )
         {
             const auto interval_seconds = interval.to_seconds();
             const auto file_number = timestamp.sec_since_epoch() / interval_seconds;
             return time_point_sec( file_number * interval_seconds );
         }

         string timestamp_to_string( const time_point_sec& timestamp )
         {
             auto ptime = boost::posix_time::from_time_t( time_t ( timestamp.sec_since_epoch() ) );
             return boost::posix_time::to_iso_string( ptime );
         }

         time_point_sec string_to_timestamp( const string& str )
         {
             return time_point::from_iso_string( str );
         }

         void rotate_files( bool initializing = false )
         {
             if( !cfg.rotate ) return;
             const auto now = time_point::now();
             const auto start_time = get_file_start_time( now, cfg.rotation_interval );
             if( !initializing )
             {
                 if( start_time <= current_file_start_time ) return;
                 out.close();
             }

             auto log_filename = cfg.filename.string();
             const auto timestamp_string = timestamp_to_string( start_time );

             /* Delete old log files */
             const auto limit_time = now - cfg.rotation_limit;
             auto itr = directory_iterator( fc::path( log_filename ).parent_path() );
             for( ; itr != directory_iterator(); itr++ )
             {
                 const auto current_filename = itr->string();
                 auto current_pos = current_filename.find( log_filename );
                 if( current_pos != 0 ) continue;
                 current_pos = log_filename.size() + 1;
                 const auto current_timestamp = string( current_filename.begin() + current_pos,
                                                        current_filename.begin() + current_pos + timestamp_string.size() );
                 try
                 {
                     if( string_to_timestamp( current_timestamp ) < limit_time )
                     {
                         remove_all( current_filename );
                     }
                     else if( cfg.rotation_compression )
                     {
                         if( current_filename.substr( current_filename.size() - 3 ) == ".7z" ) continue;
                         lzma_compress_file( current_filename, current_filename + ".7z" );
                         remove_all( current_filename );
                     }
                 }
                 catch( ... )
                 {
                 }
             }

             current_file_start_time = start_time;
             log_filename += "." + timestamp_string;
             out.open( log_filename.c_str() );
         }
   };
   file_appender::config::config( const fc::path& p  )
   :format( "${timestamp} ${thread_name} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
   filename(p),flush(true),truncate(true),rotate(false),rotation_compression(false){}

   file_appender::file_appender( const variant& args )
   :my( new impl() )
   {
      std::string log_filename;
      try
      {
         my->cfg = args.as<config>(); 
         log_filename = my->cfg.filename.string();

         fc::create_directories( fc::path( log_filename ).parent_path() );

         if( !my->cfg.rotate ) my->out.open( log_filename.c_str() );
         else my->rotate_files( true );
      }
      catch( ... )
      {
         std::cerr << "error opening log file: " << log_filename << "\n";
         //elog( "%s", fc::except_str().c_str() );
      }
   }
   file_appender::~file_appender(){}

   // MS THREAD METHOD  MESSAGE \t\t\t File:Line
   void file_appender::log( const log_message& m )
   {
      std::stringstream line;
      //line << (m.get_context().get_timestamp().time_since_epoch().count() % (1000ll*1000ll*60ll*60))/1000 <<"ms ";
      line << std::string(m.get_context().get_timestamp()) << " ";
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

      /* Write to log file (rotating file beforehand if necessary) */
      {
        fc::scoped_lock<boost::mutex> lock( my->slock );
        if( my->cfg.rotate ) my->rotate_files();
        my->out << line.str() << "\t\t\t" << m.get_context().get_file() <<":"<<m.get_context().get_line_number()<<"\n";
        if( my->cfg.flush ) my->out.flush();
      }
   }
}
