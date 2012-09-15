#include <fc/string.hpp>
#include <fc/utility.hpp>

#include <string>
namespace detail {
  void destroy( void* t ) {
    using namespace std;
    reinterpret_cast<std::string*>(t)->~string();
  }
}

/**
 *  Implemented with std::string for now.
 */

namespace fc  {

  string::string(const char* s, int l) {
    static_assert( sizeof(*this) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string(s,l);
  }
  string::string() {
    static_assert( sizeof(*this) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string();
  }

  string::string( const string& c ) {
    static_assert( sizeof(my) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string(reinterpret_cast<const std::string&>(c));
  }
 

  string::string( string&& m )  {
    static_assert( sizeof(my) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string(reinterpret_cast<std::string&&>(m));
  }
  
  string::string( const char* c ){
    static_assert( sizeof(my) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string(c);
  }

  string::string( const_iterator b, const_iterator e ) {
    static_assert( sizeof(my) >= sizeof(std::string), "failed to reserve enough space" );
    new (this) std::string(b,e);
  }


  string::~string() {
    ::detail::destroy( this );
  }

  string::iterator string::begin()            { return &(*this)[0]; }
  string::iterator string::end()              { return &(*this)[size()]; }
  string::const_iterator string::begin()const { return reinterpret_cast<const std::string*>(this)->c_str(); }
  string::const_iterator string::end()const   { return reinterpret_cast<const std::string*>(this)->c_str() + reinterpret_cast<const std::string*>(this)->size(); }

  char&       string::operator[](uint64_t idx)      { return reinterpret_cast<std::string*>(this)->at(idx); }
  const char& string::operator[](uint64_t idx)const { return reinterpret_cast<const std::string*>(this)->at(idx); }

  void       string::reserve(uint64_t r)    { reinterpret_cast<std::string*>(this)->reserve(r); }
  uint64_t   string::size()const            { return reinterpret_cast<const std::string*>(this)->size(); }
  uint64_t   string::find(char c, uint64_t p)const { return reinterpret_cast<const std::string*>(this)->find(c,p); }
  void       string::clear()                { return reinterpret_cast<std::string*>(this)->clear(); }
  void       string::resize( uint64_t s )   { reinterpret_cast<std::string*>(this)->resize(s); }
                                            
  const char* string::c_str()const          { return reinterpret_cast<const std::string*>(this)->c_str(); }

  bool    string::operator == ( const char* s )const {
    return reinterpret_cast<const std::string&>(*this) == s;
  }
  bool    string::operator == ( const string& s )const {
    return reinterpret_cast<const std::string&>(*this) == reinterpret_cast<const std::string&>(s);
  }
  bool    string::operator != ( const string& s )const {
    return reinterpret_cast<const std::string&>(*this) != reinterpret_cast<const std::string&>(s);
  }

  string& string::operator =( const string& c ) {
    reinterpret_cast<std::string&>(*this) = reinterpret_cast<const std::string&>(c);
    return *this;
  }
  string& string::operator =( string&& c ) {
    reinterpret_cast<std::string&>(*this) = fc::move( reinterpret_cast<std::string&>(c) );
    return *this;
  }

  string& string::operator+=( const string& s ) {
    reinterpret_cast<std::string&>(*this) += reinterpret_cast<const std::string&>(s);
    return *this;
  }
  string& string::operator+=( char c ) {
    reinterpret_cast<std::string&>(*this) += c;
    return *this;
  }

  string operator + ( const string& s, const string& c ) {
    return string(s) += c;
  }
  string operator + ( const string& s, char c ) {
    return string(s) += c;
  }

} // namespace fc


