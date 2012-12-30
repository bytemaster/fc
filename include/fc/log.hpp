#pragma once
#include <fc/utility.hpp>

namespace boost { class mutex; }

namespace fc {
  /** wrapper on printf */
  void log( const char* color, const char* file_name, size_t line_num, const char* method_name, const char* format, ... );

  /** used to add extra fields to be printed (thread,fiber,time,etc) */
  void add_log_field( void (*f)() );
  void remove_log_field( void (*f)() );

  boost::mutex& log_mutex();
}

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef WIN32
#define  COLOR_CONSOLE 1
#endif
#include <fc/console_defines.h>

#define dlog(...) do { fc::log(  CONSOLE_DEFAULT, __FILE__, __LINE__, __func__,  __VA_ARGS__ ); }while(false)
#define slog(...) do { fc::log(  CONSOLE_DEFAULT, __FILE__, __LINE__, __func__,  __VA_ARGS__ ); }while(false)
#define wlog(...) do { fc::log(  CONSOLE_BROWN,   __FILE__, __LINE__, __func__,  __VA_ARGS__ ); }while(false)
#define elog(...) do { fc::log(  CONSOLE_RED,     __FILE__, __LINE__, __func__,  __VA_ARGS__ ); }while(false)


