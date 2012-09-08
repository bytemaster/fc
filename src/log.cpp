#include <fc/log.hpp>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

namespace fc {
  const char* short_name( const char* file_name ) { 
    const char* end = file_name + strlen(file_name);
    --end;
    while( end >= file_name ) {
      if( *end == '/' || *end == '\\' ) {
        return end + 1;
      }
      --end;
    }
    return file_name;
  } 

  #ifdef WIN32 
    #define isatty _isatty
    #define fileno _fileno
  #endif // WIN32 

  void log( const char* color, const char* file_name, size_t line_num, const char* method_name, const char* format, ... ) {
    if(isatty(fileno(stderr)))
      fprintf( stderr, "%s", color );

    fprintf( stderr, "%s:%zd  %s ",  short_name(file_name), line_num, method_name );
    va_list args;
    va_start(args,format);
    vfprintf( stderr, format, args );
    va_end(args);
    if (isatty(fileno(stderr)))
      fprintf( stderr, "%s", CONSOLE_DEFAULT );
    fprintf( stderr, "\n" );
  }

  /** used to add extra fields to be printed (thread,fiber,time,etc) */
  void add_log_field( void (*f)( ) ) {
  }

  void remove_log_field( void (*f)( ) ) {
  }
}
