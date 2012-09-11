/**
 *  @file mace/cmt/log/log.hpp
 *  @brief Defines helpful console logging methods.
 */
#ifndef _BOOST_RPC_LOG_HPP_
#define _BOOST_RPC_LOG_HPP_
#include <boost/format.hpp>
#include <stdint.h>
#include <iostream>
#include <iomanip>

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef WIN32
#define  COLOR_CONSOLE 1
#endif
#include <fc/console_defines.h>
#include <fc/unique_lock.hpp>
#include <fc/exception.hpp>
#include <boost/thread/mutex.hpp>


namespace fc {
    const char* thread_name();

    boost::mutex& log_mutex();
    namespace detail {

    inline std::string short_name( const std::string& file_name ) { return file_name.substr( file_name.rfind( '/' ) + 1 ); }

    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const char* text ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<<text<< CONSOLE_DEFAULT << std::endl; 
    }
    template<typename P1>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< boost::format(format) %p1 %p2 << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4, typename P5>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4 %p5) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6 ) {
        fc::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4 %p5 %p6) << CONSOLE_DEFAULT << std::endl;
    }

  }
} 
/**
 *  @def dlog
 */
#define dlog(...) do {try { fc::detail::log( std::cerr, CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } \
                catch (...){ fc::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", fc::current_exception().diagnostic_information().c_str() ); }  \
                }while(false)

/**
 *  @def slog
 */
#define slog(...) do {try {fc::detail::log( std::cerr, CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (...){ fc::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", fc::current_exception().diagnostic_information().c_str() ); }  \
                }while(false)

/**
 *  @def elog
 */
#define elog(...) do {try {fc::detail::log( std::cerr, CONSOLE_RED,     __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (...){ fc::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", fc::current_exception().diagnostic_information().c_str() ); }  \
                }while(false)

/**
 *  @def wlog
 */
#define wlog(...) do {try {fc::detail::log( std::cerr, CONSOLE_BROWN,   __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (...){ fc::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", fc::current_exception().diagnostic_information().c_str() ); }  \
                }while(false)

#endif 
