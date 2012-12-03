#include <fc/string.hpp>
#include <fc/utility.hpp>
#include <fc/fwd_impl.hpp>

#include <string>

/**
 *  Implemented with std::string for now.
 */

namespace fc  {

  string::string(const char* s, int l) :my(s,l){ }
  string::string(){}
  string::string( const string& c ):my(c.my) { }
  string::string( string&& m ):my(fc::move(m.my)) {}
  string::string( const char* c ):my(c){}
  string::string( const_iterator b, const_iterator e ):my(b,e){}
  string::string( const std::string& s ):my(s) {}
  string::string( std::string&& s ):my(fc::move(s)) {}
  string::~string() { }
  string::operator std::string&() { return *my; }
  string::operator const std::string&()const { return *my; }

  string::iterator string::begin()            { return &(*this)[0]; }
  string::iterator string::end()              { return &(*this)[size()]; }
  string::const_iterator string::begin()const { return my->c_str(); }
  string::const_iterator string::end()const   { return my->c_str() + my->size(); }

  char&       string::operator[](uint64_t idx)      { return (*my)[idx]; }
  const char& string::operator[](uint64_t idx)const { return (*my)[idx]; }

  void       string::reserve(uint64_t r)           { my->reserve(r); }
  uint64_t   string::size()const                   { return my->size(); }
  uint64_t   string::find(char c, uint64_t p)const { return my->find(c,p); }
  void       string::clear()                       { my->clear(); }
  void       string::resize( uint64_t s )          { my->resize(s); }
                                            
  fc::string string::substr( int32_t start, int32_t len ) { return my->substr(start,len); }
  const char* string::c_str()const                        { return my->c_str(); }

  bool    string::operator == ( const char* s )const   { return *my == s; }
  bool    string::operator == ( const string& s )const { return *my == *s.my; }
  bool    string::operator != ( const string& s )const { return *my != *s.my; }

  string& string::operator =( const string& c )          { *my = *c.my; return *this; }
  string& string::operator =( string&& c )               { *my = fc::move( *c.my ); return *this; }

  string& string::operator+=( const string& s )          { *my += *s.my; return *this; }
  string& string::operator+=( char c )                   { *my += c; return *this; }

  bool operator < ( const string& a, const string& b )   { return *a.my < *b.my; } 
  string operator + ( const string& s, const string& c ) { return string(s) += c; }
  string operator + ( const string& s, char c ) 	 { return string(s) += c; }

} // namespace fc


