#include <fc/appender.hpp>
#include <fc/logger.hpp>
#include <fc/value.hpp>
#include <fc/unique_lock.hpp>
#include <unordered_map>
#include <string>
#include <fc/spin_lock.hpp>
#include <fc/scoped_lock.hpp>
#include <fc/console_defines.h>
#include <fc/log.hpp>
#include <fc/value_cast.hpp>
#include <boost/thread/mutex.hpp>

#ifndef WIN32
#include <unistd.h>
#endif

namespace fc {

   static fc::spin_lock appender_spinlock;
   std::unordered_map<std::string,appender::ptr>& get_appender_map() {
     static std::unordered_map<std::string,appender::ptr> lm;
     return lm;
   }
   std::unordered_map<std::string,appender_factory::ptr>& get_appender_factory_map() {
     static std::unordered_map<std::string,appender_factory::ptr> lm;
     return lm;
   }
   appender::ptr appender::get( const fc::string& s ) {
      scoped_lock<spin_lock> lock(appender_spinlock);
      return get_appender_map()[s];
   }
   bool  appender::register_appender( const fc::string& type, const appender_factory::ptr& f )
   {
      get_appender_factory_map()[type] = f;
      return true;
   }
   appender::ptr appender::create( const fc::string& name, const fc::string& type, const value& args  )
   {
      auto fact_itr = get_appender_factory_map().find(type);
      if( fact_itr == get_appender_factory_map().end() ) {
         wlog( "Unknown appender type '%s'", type.c_str() );
         return appender::ptr();
      }
      auto ap = fact_itr->second->create( args );
      get_appender_map()[name] = ap;
      return ap;
   }

   class console_appender : public appender{
    public:
         struct color {
             enum type {
                red,
                green,
                brown,
                blue,
                magenta,
                cyan,
                white,
                console_default,
             };
         };
         struct stream { enum type { std_out, std_error }; };

         struct level_color {
            level_color( log_level::type l=log_level::all, 
                         color::type c=color::console_default )
            :level(l),color(c){}

            log_level::type                   level;
            console_appender::color::type     color;
         };

         struct config {
            config()
            :format( "${when} ${thread} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
             stream(console_appender::stream::std_error),flush(true){}

            fc::string                         format;
            console_appender::stream::type     stream;
            fc::vector<level_color>            level_colors;
            bool                               flush;
         };

         config                      cfg;
         color::type                 lc[log_level::off+1];

         console_appender( const value& args );
         const char* get_color(color::type t ) {
            switch( t ) {
               case color::red: return CONSOLE_RED;
               case color::green: return CONSOLE_GREEN;
               case color::brown: return CONSOLE_BROWN;
               case color::blue: return CONSOLE_BLUE;
               case color::magenta: return CONSOLE_MAGENTA;
               case color::cyan: return CONSOLE_CYAN;
               case color::white: return CONSOLE_WHITE;
               case color::console_default:
               default:
                  return CONSOLE_DEFAULT;
            }
         }
         const char* get_color( log_level::type l ) {
            return get_color( lc[l] ); 
         }

         virtual void log( const log_message& m ) {
            fc::string message = fc::substitute( m.format, m.args );
            fc::value lmsg(m);

            FILE* out = stream::std_error ? stderr : stdout;
            fc::string fmt_str = fc::substitute( cfg.format, value(m).set( "message", message)  );

            fc::unique_lock<boost::mutex> lock(log_mutex());
            #ifndef WIN32
            if(isatty(fileno(out))) fprintf( out, "\r%s", get_color( m.level ) );
            #endif

            fprintf( out, "%s", fmt_str.c_str() ); 

            #ifndef WIN32
            if(isatty(fileno(out))) fprintf( out, "\r%s", CONSOLE_DEFAULT );
            #endif
            fprintf( out, "\n" );
            if( cfg.flush ) fflush( out );
         }
   };
} // namespace fc

FC_REFLECT_ENUM( fc::console_appender::stream::type, (std_out)(std_error) )
FC_REFLECT_ENUM( fc::console_appender::color::type, (red)(green)(brown)(blue)(magenta)(cyan)(white)(console_default) )
FC_REFLECT( fc::console_appender::level_color, (level)(color) )
FC_REFLECT( fc::console_appender::config, (format)(stream)(level_colors)(flush) )

namespace fc {
   console_appender::console_appender( const value& args ) {
      cfg = fc::value_cast<config>(args);
      for( int i = 0; i < log_level::off+1; ++i )
         lc[i] = color::console_default;
      for( auto itr = cfg.level_colors.begin(); itr != cfg.level_colors.end(); ++itr )
         lc[itr->level] = itr->color;
   }

   static bool reg_console_appender = appender::register_appender<console_appender>( "console" );
} // namespace fc
