#include <fc/exception.hpp>
#include <fc/error.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

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


  fc::string to_string( char v ) { return fc::string(&v,1); }
  fc::string to_string( uint64_t v ) { return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( int64_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( double v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( float v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( int32_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( uint32_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( int16_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( uint16_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( size_t v ){ return boost::lexical_cast<std::string>(v).c_str(); }
  fc::string to_string( long int v ){ return boost::lexical_cast<std::string>(v).c_str(); }

  void throw_exception( const char* func, const char* file, int line, const char* msg ) {
    ::boost::exception_detail::throw_exception_(fc::generic_exception(msg),func, file, line );
  }
  void throw_exception_( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1 ) {
    ::boost::exception_detail::throw_exception_(fc::generic_exception( (boost::format(msg) % a1.c_str() ).str().c_str()) ,func, file, line );
  }
  void throw_exception_( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2 ) {
    ::boost::exception_detail::throw_exception_(fc::generic_exception( (boost::format(msg) % a1.c_str() %a2.c_str()).str().c_str()),func, file, line );
  }
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2, const fc::string& a3 ) {
    ::boost::exception_detail::throw_exception_(fc::generic_exception(msg),func, file, line );
  }
  void throw_exception( const char* func, const char* file, int line, const char* msg, 
                        const fc::string& a1, const fc::string& a2, const fc::string& a3, const fc::string& a4 ) {
    ::boost::exception_detail::throw_exception_(fc::generic_exception(msg),func, file, line );
  }

}
