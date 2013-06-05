#include <fc/io/fstream.hpp>
#include <fc/filesystem.hpp>
#include <fstream>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>


namespace fc {
   class ofstream::impl : public fc::retainable {
      public:
         std::ofstream ofs;
   };
   class ifstream::impl : public fc::retainable {
      public:
         std::ifstream ifs;
   };

   ofstream::ofstream()
   :my( new impl() ){}

   ofstream::ofstream( const fc::path& file, int m )
   :my( new impl() ) { this->open( file, m ); }
   ofstream::~ofstream(){}

   void ofstream::open( const fc::path& file, int m ) {
      my->ofs.open( file.string().c_str(), std::ios::binary );
   }
   size_t ofstream::writesome( const char* buf, size_t len ) {
        my->ofs.write(buf,len);
        return len;
   }
   void   ofstream::put( char c ) {
        my->ofs.put(c);
   }
   void   ofstream::close() {
        my->ofs.close();
   }
   void   ofstream::flush() {
        my->ofs.flush();
   }

   ifstream::ifstream()
   :my(new impl()){}
   ifstream::ifstream( const fc::path& file, int m )
   :my(new impl())
   {
      this->open( file, m );
   }
   ifstream::~ifstream(){}

   void ifstream::open( const fc::path& file, int m ) {
      my->ifs.open( file.string().c_str(), std::ios::binary );
   }
   size_t ifstream::readsome( char* buf, size_t len ) {
      auto s = size_t(my->ifs.readsome( buf, len ));
      if( s <= 0 ) {
         read( buf, 1 );
         s = 1;
      }
      return s;
   }
   ifstream& ifstream::read( char* buf, size_t len ) {
      if( eof() ) FC_THROW_EXCEPTION( eof_exception , "");
      my->ifs.read(buf,len);
      if (my->ifs.gcount() < int64_t(len))
        FC_THROW_EXCEPTION( eof_exception , "");
      return *this;
   }
   ifstream& ifstream::seekg( size_t p, seekdir d ) {
      switch( d ) {
        case beg: my->ifs.seekg( p, std::ios_base::beg ); return *this;
        case cur: my->ifs.seekg( p, std::ios_base::cur ); return *this;
        case end: my->ifs.seekg( p, std::ios_base::end ); return *this;
      }
      return *this;
   }
   void   ifstream::close() { return my->ifs.close(); }

   bool   ifstream::eof()const { return !my->ifs.good(); }

  
} // namespace fc 
