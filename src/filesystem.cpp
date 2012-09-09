#include <fc/filesystem.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/utility.hpp>
#include <boost/filesystem.hpp>

namespace fc {

   path::path(){}
   path::~path(){};
   path::path( const boost::filesystem::path& p )
   :_p(p){}

   path::path( const char* p )
   :_p(p){}
   path::path( const fc::string& p )
   :_p(p.c_str()){}

   path::path( const path& p )
   :_p(p){}

   path::path( path&& p )
   :_p(std::move(p)){}

   path& path::operator =( const path& p ) {
    *_p = *p._p;
    return *this;
   }
   path& path::operator =( path&& p ) {
    *_p = fc::move( *p._p );
    return *this;
   }

   path& path::operator /=( const fc::path& p ) {
      *_p /= *p._p;
      return *this;
   }
   path   operator /( const fc::path& p, const fc::path& o ) {
      path tmp;
      tmp = *p._p / *o._p;
      return tmp;
   }

   path::operator boost::filesystem::path& () {
    return *_p;
   }
   path::operator const boost::filesystem::path& ()const {
    return *_p;
   }

   fc::string path::string()const {
    return _p->string().c_str();
   }


  bool exists( const path& p ) { return boost::filesystem::exists(p); }
  void create_directories( const path& p ) { boost::filesystem::create_directories(p); }
}
