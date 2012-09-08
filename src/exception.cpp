#include <fc/exception.hpp>
#include <boost/exception/all.hpp>

namespace fc {
  #define bexcept  void* e = &my[0]; (*((boost::exception_ptr*)e))
  #define cbexcept const void* e = &my[0]; (*((const boost::exception_ptr*)e))

  exception_ptr::exception_ptr() {
    new (&my[0]) boost::exception_ptr();
  }
  exception_ptr::exception_ptr( const boost::exception_ptr& c ){
    static_assert( sizeof(my) >= sizeof(c), "boost::exception_ptr is larger than space reserved for it" );
    new (&my[0]) boost::exception_ptr(c);
  }
  exception_ptr::exception_ptr( boost::exception_ptr&& c ){
    new (&my[0]) boost::exception_ptr(fc::move(c));
  }
  exception_ptr::exception_ptr( const exception_ptr& c ){
    new (&my[0]) boost::exception_ptr(c);
  }
  exception_ptr::exception_ptr( exception_ptr&& c ){
    new (&my[0]) boost::exception_ptr(fc::move(c));
  }
  exception_ptr::~exception_ptr(){
    bexcept.~exception_ptr();
  }
  exception_ptr& exception_ptr::operator=(const boost::exception_ptr& c){
    bexcept = c;
    return *this;
  }
  exception_ptr& exception_ptr::operator=(boost::exception_ptr&& c){
    bexcept = fc::move(c);
    return *this;
  }

  exception_ptr& exception_ptr::operator=(const exception_ptr& c){
    bexcept = c;
    return *this;
  }
  exception_ptr& exception_ptr::operator=(exception_ptr&& c){
    bexcept = fc::move(c);
    return *this;
  }

  fc::string exception_ptr::diagnostic_information()const{
    const void* e = &my[0]; 
    return boost::diagnostic_information( *((const boost::exception_ptr*)e) ).c_str();
  }

  exception_ptr::operator const boost::exception_ptr& ()const{
    const void* e = &my[0]; 
    return (*((const boost::exception_ptr*)e));
  }
  exception_ptr::operator boost::exception_ptr& (){
    void* e = &my[0]; 
    return (*((boost::exception_ptr*)e));
  }

  exception_ptr current_exception() {
    return boost::current_exception();
  }
  void          rethrow_exception( const exception_ptr& e ) {
    boost::rethrow_exception( static_cast<boost::exception_ptr>(e) );
  }
  exception_ptr::operator bool()const {
    const void* e = &my[0]; 
    return (*((boost::exception_ptr*)e));
  }
}
