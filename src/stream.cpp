#include <fc/stream.hpp>
#include <iostream>
#include <boost/iostreams/stream.hpp>
#include <fc/thread.hpp>

namespace fc {
  namespace detail {
    namespace io = boost::iostreams;
    class cin_source : public io::source {
      public:
        typedef char      type;

        template<typename T>
        cin_source(T):_cin_thread(NULL){}
        cin_source():_cin_thread(NULL){}

        cin_source( const cin_source& s ):_cin_thread(s._cin_thread){}

        std::streamsize read( char* s, std::streamsize n ) {
          int r =  std::cin.readsome(s,n);
          if( std::cin && r <= 0 ) {
            std::cin.read( s, 1 );
            if( std::cin.eof() ) return -1;
            return 1;
          }
          return r;
        }
      private:
        fc::thread* _cin_thread;
    };
    std::istream& get_cin_stream() {
      static io::stream<cin_source> cin_stream;// =   cin_source(); 
      cin_stream.open(0);
      return cin_stream;
    }

    class abstract_source : public io::source {
      public:
       typedef char type;
        abstract_source( abstract_istream& ais ):_ais(ais){}

        std::streamsize read( char* s, std::streamsize n ) {
          return _ais.readsome_impl( s, n );
        }
        abstract_istream& _ais;
    };
    abstract_istream::abstract_istream() {
      static_assert( sizeof(_store) >= sizeof( io::stream<abstract_source> ), "Failed to allocate enough space" );
      (new (&_store[0]) io::stream<abstract_source>( *this ));
    }
    size_t abstract_istream::readsome( char* buf, size_t len ) {
      auto iost = (io::stream<abstract_source>*)(&_store[0]); 
      iost->read(buf,len);
      return len;
    }

    class abstract_sink : public io::sink {
      public:
        struct category : io::sink::category, io::flushable_tag {};
        typedef char      type;
        abstract_sink( abstract_ostream& aos ):_aos(aos){}

        std::streamsize write( const char* s, std::streamsize n ) {
          return _aos.write_impl( s, n );
        }
        void close() { _aos.close_impl(); }
        bool flush() { _aos.flush_impl(); return true; }

        abstract_ostream& _aos;
    };

    abstract_ostream::abstract_ostream() {
      static_assert( sizeof(_store) >= sizeof( io::stream<abstract_sink> ), "Failed to allocate enough space" );
      (new (&_store[0]) io::stream<abstract_sink>( *this ));
    }
    size_t abstract_ostream::write( const char* buf, size_t len ) {
      auto iost = (io::stream<abstract_sink>*)(&_store[0]); 
      iost->write(buf,len);
      return len;
    }
    void abstract_ostream::flush() {
      auto iost = (io::stream<abstract_sink>*)(&_store[0]); 
      iost->flush();
    }
    void abstract_ostream::close() {
      auto iost = (io::stream<abstract_sink>*)(&_store[0]); 
      iost->close();
    }


    abstract_istream::~abstract_istream() {
    }
    abstract_ostream::~abstract_ostream() {
      auto iost = (io::stream<abstract_sink>*)(&_store[0]); 
      iost->~stream<abstract_sink>();
    }
  }
  
  istream::~istream(){
    detail::abstract_istream* i = (detail::abstract_istream*)&_store[0];
    i->~abstract_istream();
  }
 
  size_t istream::readsome( char* buf, size_t len ){
    detail::abstract_istream* i = (detail::abstract_istream*)&_store[0];
    return i->readsome(buf,len);
  }
  #define read_help  \
    detail::abstract_istream* aos = (detail::abstract_istream*)&i._store[0];\
    auto iist = (detail::io::stream<detail::abstract_source>*)(&aos->_store[0]); \
    (*iist) >> s; \
    return i;
 
  istream& operator>>( istream& i, int64_t&    s){ read_help }
  istream& operator>>( istream& i, uint64_t&   s){ read_help }
  istream& operator>>( istream& i, int32_t&    s){ read_help }
  istream& operator>>( istream& i, uint32_t&   s){ read_help }
  istream& operator>>( istream& i, int16_t&    s){ read_help }
  istream& operator>>( istream& i, uint16_t&   s){ read_help }
  istream& operator>>( istream& i, int8_t&     s){ read_help }
  istream& operator>>( istream& i, uint8_t&    s){ read_help }
  istream& operator>>( istream& i, float&      s){ read_help }
  istream& operator>>( istream& i, double&     s){ read_help }
  istream& operator>>( istream& i, bool&       s){ read_help }
  istream& operator>>( istream& i, char&       s){ read_help }
  istream& operator>>( istream& i, fc::string& s){
    std::string ss; 
    detail::abstract_istream* aos = (detail::abstract_istream*)&i._store[0];
    auto iist = (detail::io::stream<detail::abstract_source>*)(&aos->_store[0]); 
    (*iist) >> ss;
    s = ss.c_str();
    return i;
  }

  #undef read_help

 
  ostream::~ostream(){
    detail::abstract_ostream* o = (detail::abstract_ostream*)&_store[0];
    close();
    o->~abstract_ostream();
  }
 
  size_t ostream::write( const char* buf, size_t len ){
    detail::abstract_ostream* o = (detail::abstract_ostream*)&_store[0];
    return o->write(buf,len);
  }
  void   ostream::close(){
    detail::abstract_ostream* o = (detail::abstract_ostream*)&_store[0];
    o->close();
  }
  void   ostream::flush(){
    detail::abstract_ostream* o = (detail::abstract_ostream*)&_store[0];
    o->flush();
  }
  #define print_help  \
    detail::abstract_ostream* aos = (detail::abstract_ostream*)&o._store[0];\
    auto iost = (detail::io::stream<detail::abstract_sink>*)(&aos->_store[0]); \
    (*iost) << s; \
    return o;
 
  ostream& operator<<( ostream& o, int64_t  s ){ print_help }
  ostream& operator<<( ostream& o, uint64_t s ){ print_help }
  ostream& operator<<( ostream& o, int32_t  s ){ print_help }
  ostream& operator<<( ostream& o, uint32_t s ){ print_help }
  ostream& operator<<( ostream& o, int16_t  s ){ print_help }
  ostream& operator<<( ostream& o, uint16_t s ){ print_help }
  ostream& operator<<( ostream& o, int8_t   s ){ print_help }
  ostream& operator<<( ostream& o, uint8_t  s ){ print_help }
  ostream& operator<<( ostream& o, float    s ){ print_help }
  ostream& operator<<( ostream& o, double   s ){ print_help }
  ostream& operator<<( ostream& o, bool     s ){ print_help }
  ostream& operator<<( ostream& o, char     s ){ print_help }
  ostream& operator<<( ostream& o, const char* s ){ print_help }
  ostream& operator<<( ostream& o, const fc::string& s ){ return o << s.c_str(); }
 
  #undef print_help

  ostream cout( std::cout );
  ostream cerr( std::cerr );
  istream cin(  detail::get_cin_stream() );
} //namespace fc
