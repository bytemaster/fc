#include <fc/fstream.hpp>
#include <fc/filesystem.hpp>
#include <fstream>

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
   ofstream& ofstream::write( const char* buf, size_t len ) {
        my->ofs.write(buf,len);
        return *this;
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
      return my->ifs.readsome( buf, len );
   }
   ifstream& ifstream::read( char* buf, size_t len ) {
        my->ifs.read(buf,len);
        return *this;
   }
   void   ifstream::close() { return my->ifs.close(); }

   bool   ifstream::eof()const { return my->ifs.eof(); }

  
} // namespace fc 
