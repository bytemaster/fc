#ifndef _FC_EXCEPTION_HPP_
#define _FC_EXCEPTION_HPP_
#include <fc/shared_ptr.hpp>
#include <fc/string.hpp>
#include <boost/current_function.hpp>

// TODO: Remove boost exception dependency here!!
// TODO: Remove boost format dependency here!!

// provided for easy integration with boost.
namespace boost { class exception_ptr; }

namespace fc {
  /**
   *  Simply including boost/exception_ptr.hpp is enough to significantly
   *  lengthen compile times.  This header defines an 'opaque' exception
   *  type that provides the most 'general' exception handling needs without
   *  requiring a significant amount of code to be included.
   */
  class exception_ptr {
    public:
      exception_ptr();
      exception_ptr( const boost::exception_ptr& c );
      exception_ptr( boost::exception_ptr&& c );
      exception_ptr( const exception_ptr& c );
      exception_ptr( exception_ptr&& c );
      ~exception_ptr();

      exception_ptr& operator=(const boost::exception_ptr& c);
      exception_ptr& operator=(boost::exception_ptr&& c);

      exception_ptr& operator=(const exception_ptr& c);
      exception_ptr& operator=(exception_ptr&& c);

      fc::string diagnostic_information()const;

      operator bool()const;

      operator const boost::exception_ptr& ()const;
      operator boost::exception_ptr& ();
    private:
      char my[sizeof(void*)*2];
  };

  exception_ptr current_exception();
  template<typename T>
  inline exception_ptr copy_exception( T&& e ) {
    try { throw e; } catch (...) { return current_exception(); }  
    return exception_ptr();
  }
  void          rethrow_exception( const exception_ptr& e );

  void throw_exception( const char* func, const char* file, int line, const char* msg );
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1 );
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2 );
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2, const fc::string& a3 );
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2, const fc::string& a3, const fc::string& a4 );

  template<typename T>
  fc::string to_string( T&& v ) { return fc::string(fc::forward<T>(v)); }
  fc::string to_string( char v ); // { return fc::string(&v,1); }
  fc::string to_string( uint64_t v );
  fc::string to_string( int64_t v );
  fc::string to_string( double v );
  fc::string to_string( float v );
  fc::string to_string( int32_t v );
  fc::string to_string( uint32_t v );
  fc::string to_string( int16_t v );
  fc::string to_string( uint16_t v );
  fc::string to_string( size_t v );
  fc::string to_string( long int v );

  template<typename T>
  void throw_exception( const char* func, const char* file, int line, const char* msg, T&& a1 ) {
    throw_exception( func, file, line, msg, to_string(fc::forward<T>(a1) ) );
  }

  template<typename T1, typename T2>
  void throw_exception( const char* func, const char* file, int line, const char* msg, T1&& a1, T2&& a2 ) {
    throw_exception( func, file, line, msg, to_string(fc::forward<T1>(a1) ), to_string( fc::forward<T2>(a2) ) );
  }


} // namespace fc 
#define FC_THROW(X) throw X
#define FC_THROW_MSG( ... ) \
  do { \
    fc::throw_exception( BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, __VA_ARGS__ ); \
  } while(0)


#endif // _FC_EXCEPTION_HPP_
