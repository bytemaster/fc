#ifndef _FC_EXCEPTION_HPP_
#define _FC_EXCEPTION_HPP_
#include <fc/shared_ptr.hpp>
#include <fc/string.hpp>

// TODO: Remove boost exception dependency here!!
// TODO: Remove boost format dependency here!!
#include <boost/format.hpp>
#include <boost/exception/all.hpp>

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


  typedef boost::error_info<struct err_msg_,std::string> err_msg;
  struct exception : public virtual boost::exception, public virtual std::exception {
      const char* what()const throw()     { return "exception";                     }
      virtual void       rethrow()const   { BOOST_THROW_EXCEPTION(*this);                  } 
      const std::string& message()const   { return *boost::get_error_info<fc::err_msg>(*this); }
  };
} // namespace fc 
#define FC_THROW(X,...) throw X
#define FC_THROW_MSG( MSG, ... ) \
  do { \
    BOOST_THROW_EXCEPTION( fc::exception() << fc::err_msg( (boost::format( MSG ) __VA_ARGS__ ).str() ) );\
  } while(0)


#endif // _FC_EXCEPTION_HPP_
