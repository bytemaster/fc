#include <fc/console_appender.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/unique_lock.hpp>
#include <fc/value.hpp>
#include <fc/value_cast.hpp>
#ifndef WIN32
#include <unistd.h>
#endif

namespace fc {
   console_appender::console_appender( const value& args ) {
      cfg = fc::value_cast<config>(args);
      for( int i = 0; i < log_level::off+1; ++i )
         lc[i] = color::console_default;
      for( auto itr = cfg.level_colors.begin(); itr != cfg.level_colors.end(); ++itr )
         lc[itr->level] = itr->color;
   }
   const char* get_console_color(console_appender::color::type t ) {
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
   }
   const char* console_appender::get_color( log_level::type l )const {
      return get_console_color( lc[l] ); 
   }
   void console_appender::log( const log_message& m ) {
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

}
