#include <fc/compress/lzma.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/thread/thread.hpp>
#include <fc/variant.hpp>
#include <boost/thread/mutex.hpp>
#include <iomanip>
#include <queue>
#include <sstream>

namespace fc {

   static const string  compression_extension( ".lzma" );

   class file_appender::impl : public fc::retainable {
      public:
         config                     cfg;
         ofstream                   out;
         boost::mutex               slock;

      private:
         future<void>               _rotation_task;
         time_point_sec             _current_file_start_time;
         std::unique_ptr<thread>    _compression_thread;

         time_point_sec get_file_start_time( const time_point_sec& timestamp, const microseconds& interval )
         {
             const auto interval_seconds = interval.to_seconds();
             const auto file_number = timestamp.sec_since_epoch() / interval_seconds;
             return time_point_sec( file_number * interval_seconds );
         }

         string timestamp_to_string( const time_point_sec& timestamp )
         {
             return timestamp.to_iso_string();
         }

         time_point_sec string_to_timestamp( const string& str )
         {
             return time_point::from_iso_string( str );
         }

         void compress_file( const string& filename )
         {
             FC_ASSERT( cfg.rotate && cfg.rotation_compression );
             FC_ASSERT( _compression_thread );
             if( !_compression_thread->is_current() )
             {
                 _compression_thread->async( [this, filename]() { compress_file( filename ); } ).wait();
                 return;
             }

             try
             {
                 lzma_compress_file( filename, filename + compression_extension );
                 remove_all( filename );
             }
             catch( ... )
             {
             }
         }

      public:
         impl( const config& c) : cfg( c )
         {
             if( cfg.rotate )
             {
                 FC_ASSERT( cfg.rotation_interval >= seconds( 1 ) );
                 FC_ASSERT( cfg.rotation_limit >= cfg.rotation_interval );

                 if( cfg.rotation_compression )
                     _compression_thread.reset( new thread( "compression") );

                 _rotation_task = async( [this]() { rotate_files( true ); } );
             }
         }

         ~impl()
         {
            try
            {
              _rotation_task.cancel_and_wait();
            }
            catch( ... )
            {
            }

            try
            {
              if( _compression_thread ) 
                _compression_thread->quit();
            }
            catch( ... )
            {
            }
         }

         void rotate_files( bool initializing = false )
         {
             FC_ASSERT( cfg.rotate );
             const auto now = time_point::now();
             const auto start_time = get_file_start_time( now, cfg.rotation_interval );
             const auto timestamp_string = timestamp_to_string( start_time );
             const auto link_filename = cfg.filename.string();
             const auto log_filename = link_filename + "." + timestamp_string;

             {
               fc::scoped_lock<boost::mutex> lock( slock );

               if( !initializing )
               {
                   if( start_time <= _current_file_start_time )
                   {
                       _rotation_task = schedule( [this]() { rotate_files(); }, _current_file_start_time + cfg.rotation_interval.to_seconds(), "log_rotation_task" );
                       return;
                   }

                   out.flush();
                   out.close();
               }

               out.open( log_filename.c_str() );
             }
             remove_all( link_filename );
             create_hard_link( log_filename, link_filename );

             /* Delete old log files */
             const auto limit_time = now - cfg.rotation_limit;
             auto itr = directory_iterator( fc::path( link_filename ).parent_path() );
             for( ; itr != directory_iterator(); itr++ )
             {
                 try
                 {
                     const auto current_filename = itr->string();
                     auto current_pos = current_filename.find( link_filename );
                     if( current_pos != 0 ) continue;
                     current_pos = link_filename.size() + 1;
                     const auto current_timestamp_str = string( current_filename.begin() + current_pos, /* substr not working */
                                                                current_filename.begin() + current_pos + timestamp_string.size() );
                     const auto current_timestamp = string_to_timestamp( current_timestamp_str );
                     if( current_timestamp < start_time )
                     {
                         if( current_timestamp < limit_time || file_size( current_filename ) <= 0 )
                         {
                             remove_all( current_filename );
                             continue;
                         }

                         if( !cfg.rotation_compression ) continue;
                         if( current_filename.find( compression_extension ) != string::npos ) continue;
                         compress_file( current_filename );
                     }
                 }
                 catch( ... )
                 {
                 }
             }

             _current_file_start_time = start_time;
             _rotation_task = schedule( [this]() { rotate_files(); }, _current_file_start_time + cfg.rotation_interval.to_seconds() );
         }
   };
   file_appender::config::config( const fc::path& p  )
   :format( "${timestamp} ${thread_name} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
   filename(p),flush(true),truncate(true),rotate(false),rotation_compression(true){}

   file_appender::file_appender( const variant& args )
   :my( new impl( args.as<config>() ) )
   {
      std::string log_filename;
      try
      {
         log_filename = my->cfg.filename.string();

         fc::create_directories( fc::path( log_filename ).parent_path() );

         if( !my->cfg.rotate ) my->out.open( log_filename.c_str() );
      }
      catch( ... )
      {
         std::cerr << "error opening log file: " << log_filename << "\n";
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

      {
        fc::scoped_lock<boost::mutex> lock( my->slock );
        my->out << line.str() << "\t\t\t" << m.get_context().get_file() <<":"<<m.get_context().get_line_number()<<"\n";
        if( my->cfg.flush ) my->out.flush();
      }
   }

} // fc
