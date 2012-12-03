#include <fc/iostream.hpp>
#include <fc/sstream.hpp>
#include <fc/thread.hpp>
#include <iostream>
#include <string.h>
#include <fc/log.hpp>
#include <string>

namespace fc {
  ostream& operator<<( ostream& o, const char* v ) {
      o.write( v, strlen(v) );
      return o;
  }
  
  struct cin_buffer {
    cin_buffer():eof(false),write_pos(0),read_pos(0),cinthread("cin"){
      cinthread.async( [=](){read();} );
    }

    void     read() {
      char c;
      std::cin.read(&c,1);
      while( !std::cin.eof() ) {
        while( write_pos - read_pos > 0xfffff ) {
          fc::promise<void>::ptr wr( new fc::promise<void>() );
          write_ready = wr;
          if( write_pos - read_pos <= 0xfffff ) {
            wr->wait();
          }
          write_ready.reset();
        }
        buf[write_pos&0xfffff] = c;
        ++write_pos;

        auto tmp = read_ready; // copy read_ready because it is accessed from multiple threads
        if( tmp && !tmp->ready() ) { 
          tmp->set_value(); 
        }
        std::cin.read(&c,1);
      }
      eof = true;
      auto tmp = read_ready; // copy read_ready because it is accessed from multiple threads
      if( tmp && !tmp->ready() ) { 
        tmp->set_value(); 
      }
    }
    fc::promise<void>::ptr read_ready;
    fc::promise<void>::ptr write_ready;
    
    volatile bool     eof;
    
    volatile uint64_t write_pos;
             char     buf[0xfffff+1]; // 1 mb buffer
    volatile uint64_t read_pos;
    fc::thread        cinthread;
  };

  cin_buffer& get_cin_buffer() {
    static cin_buffer* b = new cin_buffer();
    return *b;
  }


  fc::thread& cin_thread() { static fc::thread i("cin"); return i; }

  fc::istream& getline( fc::istream& i, fc::string& s, char delim  ) {
    fc::stringstream ss; 
    char c;
    i.read( &c, 1 );
    while( !i.eof() ) {
      if( c == delim ) { s = ss.str();  return i; }
      ss.write(&c,1);
      i.read( &c, 1 );
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

  ostream& cerr_t::write( const fc::string& s ) { std::cerr<< static_cast<std::string>(s); return *this; }

  size_t cin_t::readsome( char* buf, size_t len ) {
    cin_buffer& b = get_cin_buffer();
    size_t avail = b.write_pos - b.read_pos;
    avail = (fc::min)(len,avail);
    size_t u = 0;
    while( avail && len ) {
      *buf = b.buf[b.read_pos&0xfffff]; 
      ++b.read_pos;
      ++buf;
      --avail;
      --len;
      ++u;
    }
    return u;
  }

  istream& cin_t::read( char* buf, size_t len ) {
    cin_buffer& b = get_cin_buffer();
    do {
        while( !b.eof &&  (b.write_pos - b.read_pos)==0 ){ 
           // wait for more... 
           fc::promise<void>::ptr rr( new fc::promise<void>() );
           b.read_ready = rr;
           if( b.write_pos - b.read_pos == 0 ) {
             rr->wait();
           }
           b.read_ready.reset();
        }
        if( b.eof ) return *this;
        size_t r = readsome( buf, len );
        buf += r;
        len -= r;

        auto tmp = b.write_ready; // copy write_writey because it is accessed from multiple thwrites
        if( tmp && !tmp->ready() ) { 
          tmp->set_value(); 
        }


    } while( len > 0 && !b.eof );
    return *this;
  }

  bool cin_t::eof()const { return get_cin_buffer().eof; }

  cout_t cout;
  cerr_t cerr;
  cin_t  cin;
}
