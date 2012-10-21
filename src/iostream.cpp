#include <fc/iostream.hpp>
#include <fc/sstream.hpp>
#include <fc/thread.hpp>
#include <iostream>

#include <fc/log.hpp>

namespace fc {
  fc::thread& cin_thread() { static fc::thread i("cin"); return i; }

  fc::istream& getline( fc::istream& i, fc::string& s, char delim  ) {
    fc::stringstream ss; 
    char c;
    if( i.readsome( &c, 1 ) != 1 ) {
      cin_thread().async([&](){ i.read(&c,1); } ).wait();
    }
    while( !i.eof() ) {
      if( c == delim ) { s = ss.str();  return i; }
      ss.write(&c,1);

      if( i.readsome( &c, 1 ) != 1 ) {
        cin_thread().async([&](){ i.read(&c,1); } ).wait();
      }
    }
    s = ss.str();
    return i;
  }

  ostream& cout_t::write( const char* buf, size_t len ) { std::cout.write(buf,len); return *this; }
  void   cout_t::close() {}
  void   cout_t::flush() { std::cout.flush(); }

  ostream& cout_t::write( const fc::string& s ) { std::cout.write(s.c_str(),s.size()); return *this; }

  ostream& cerr_t::write( const char* buf, size_t len ) { std::cerr.write(buf,len); return *this; }
  void   cerr_t::close() {};
  void   cerr_t::flush() { std::cerr.flush(); }

  ostream& cerr_t::write( const fc::string& s ) { std::cerr<< *reinterpret_cast<const std::string*>(&s); return *this; }

  size_t cin_t::readsome( char* buf, size_t len ) {
    return std::cin.readsome(buf,len);
  }
  istream& cin_t::read( char* buf, size_t len ) {
    std::cin.read(buf,len);
    return *this;
  }
  bool cin_t::eof()const { return std::cin.eof(); }

  istream& cin_t::read( int64_t&     v) { std::cin >> v; return *this; }
  istream& cin_t::read( uint64_t&    v) { std::cin >> v; return *this; }
  istream& cin_t::read( int32_t&     v) { std::cin >> v; return *this; }
  istream& cin_t::read( uint32_t&    v) { std::cin >> v; return *this; }
  istream& cin_t::read( int16_t&     v) { std::cin >> v; return *this; }
  istream& cin_t::read( uint16_t&    v) { std::cin >> v; return *this; }
  istream& cin_t::read( int8_t&      v) { std::cin >> v; return *this; }
  istream& cin_t::read( uint8_t&     v) { std::cin >> v; return *this; }
  istream& cin_t::read( float&       v) { std::cin >> v; return *this; }
  istream& cin_t::read( double&      v) { std::cin >> v; return *this; }
  istream& cin_t::read( bool&        v) { std::cin >> v; return *this; }
  istream& cin_t::read( char&        v) { std::cin >> v; return *this; }
  istream& cin_t::read( fc::string&  v) { std::cin >> *reinterpret_cast<std::string*>(&v); return *this; }


  cout_t cout;
  cerr_t cerr;
  cin_t  cin;
}
