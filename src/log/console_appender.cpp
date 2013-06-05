#include <fc/log/console_appender.hpp>
#include <fc/log/log_message.hpp>
#include <fc/thread/unique_lock.hpp>
#include <fc/string.hpp>
#include <fc/variant.hpp>
#include <fc/reflect/variant.hpp>
#ifndef WIN32
#include <unistd.h>
#endif
#include <boost/thread/mutex.hpp>
#define COLOR_CONSOLE 1
#include "console_defines.h"
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>

namespace fc {
   console_appender::console_appender( const variant& args ) 
   {
      try
      {
         cfg = args.as<config>();//fc::variant_cast<config>(args);
         for( int i = 0; i < log_level::off+1; ++i )
            lc[i] = color::console_default;
         for( auto itr = cfg.level_colors.begin(); itr != cfg.level_colors.end(); ++itr )
            lc[itr->level] = itr->color;
      } 
      catch ( exception& e )
      {
         fc::cerr<<e.to_detail_string()<<"\n";
         throw;
      }
   }

   const char* get_console_color(console_appender::color::type t ) {
   #ifndef _MSC_VER
      switch( t ) {
         case console_appender::color::red: return CONSOLE_RED;
         case console_appender::color::green: return CONSOLE_GREEN;
         case console_appender::color::brown: return CONSOLE_BROWN;
         case console_appender::color::blue: return CONSOLE_BLUE;
         case console_appender::color::magenta: return CONSOLE_MAGENTA;
         case console_appender::color::cyan: return CONSOLE_CYAN;
         case console_appender::color::white: return CONSOLE_WHITE;
         case console_appender::color::console_default:
         default:
            return CONSOLE_DEFAULT;
      }
   #else 
      return "";
   #endif 
   }


   boost::mutex& log_mutex() {
    static boost::mutex m; return m;
   }
   const char* console_appender::get_color( log_level l )const {
      return get_console_color( lc[l] ); 
   }
   void console_appender::log( const log_message& m ) {
      fc::string message = fc::format_string( m.get_format(), m.get_data() );
      fc::variant lmsg(m);

      FILE* out = stream::std_error ? stderr : stdout;

      fc::string fmt_str = fc::format_string( cfg.format, mutable_variant_object(m.get_context())( "message", message)  );

      fc::unique_lock<boost::mutex> lock(log_mutex());
      #ifndef WIN32
      if(isatty(fileno(out))) fprintf( out, "\r%s", get_color( m.get_context().get_log_level() ) );
      #endif

      fprintf( out, "%s", fmt_str.c_str() ); 

      #ifndef WIN32
      if(isatty(fileno(out))) fprintf( out, "\r%s", CONSOLE_DEFAULT );
      #endif
      fprintf( out, "\n" );
      if( cfg.flush ) fflush( out );
   }

}
