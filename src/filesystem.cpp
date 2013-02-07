//#define BOOST_NO_SCOPED_ENUMS
#include <fc/filesystem.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/utility.hpp>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <fc/value_cast.hpp>
#include <fc/error_report.hpp>

namespace fc {
  void pack( fc::value& v, const fc::path& s ) {
      v = s.generic_string();
  }
  void unpack( const fc::value& v, fc::path& s ) {
      s = path(fc::value_cast<fc::string>(v));
  }

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

   bool operator <( const fc::path& l, const fc::path& r ) { return *l._p < *r._p; }
   bool operator ==( const fc::path& l, const fc::path& r ) { return *l._p == *r._p; }
   bool operator !=( const fc::path& l, const fc::path& r ) { return *l._p != *r._p; }

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
   fc::string path::generic_string()const {
    return _p->generic_string();
   }
   /**
    *  @todo use iterators instead of indexes for 
    *  faster performance
    */
   fc::string path::windows_string()const {
     auto gs = _p->generic_string();
     for( int i =0 ; i < gs.size(); ++i ) {
       if( gs[i] == '/' ) gs[i] = '\\';
     }
     return gs;
   }

   fc::string path::string()const {
    return _p->string().c_str();
   }
   fc::path path::filename()const {
    return _p->filename();
   }
   void     path::replace_extension( const fc::path& e ) {
        _p->replace_extension(e);
   }
   fc::path path::extension()const {
    return _p->extension();
   }
   fc::path path::stem()const {
    return _p->stem();
   }
   fc::path path::parent_path()const {
    return _p->parent_path();
   }
  bool path::is_relative()const { return _p->is_relative(); }
  bool path::is_absolute()const { return _p->is_absolute(); }

      directory_iterator::directory_iterator( const fc::path& p )
      :_p(p){}

      directory_iterator::directory_iterator(){}
      directory_iterator::~directory_iterator(){}

      fc::path            directory_iterator::operator*()const { return boost::filesystem::path(*(*_p)); }
      directory_iterator& directory_iterator::operator++(int)  { (*_p)++; return *this; }
      directory_iterator& directory_iterator::operator++()     { (*_p)++; return *this; }

      bool operator==( const directory_iterator& r, const directory_iterator& l) {
        return *r._p == *l._p;
      }
      bool operator!=( const directory_iterator& r, const directory_iterator& l) {
        return *r._p != *l._p;
      }


      recursive_directory_iterator::recursive_directory_iterator( const fc::path& p )
      :_p(p){}

      recursive_directory_iterator::recursive_directory_iterator(){}
      recursive_directory_iterator::~recursive_directory_iterator(){}

      fc::path            recursive_directory_iterator::operator*()const { return boost::filesystem::path(*(*_p)); }
      recursive_directory_iterator& recursive_directory_iterator::operator++(int)  { (*_p)++; return *this; }
      recursive_directory_iterator& recursive_directory_iterator::operator++()     { (*_p)++; return *this; }

      void recursive_directory_iterator::pop() { (*_p).pop(); }
      int recursive_directory_iterator::level() { return _p->level(); }

      bool operator==( const recursive_directory_iterator& r, const recursive_directory_iterator& l) {
        return *r._p == *l._p;
      }
      bool operator!=( const recursive_directory_iterator& r, const recursive_directory_iterator& l) {
        return *r._p != *l._p;
      }


  bool exists( const path& p ) { return boost::filesystem::exists(p); }
  void create_directories( const path& p ) { 
    try {
      boost::filesystem::create_directories(p); 
    } catch ( ... ) {
      FC_THROW_REPORT( "Unable to create directories ${path}", fc::value().set("path", p ).set("inner", fc::except_str() ) );
    }
  }
  bool is_directory( const path& p ) { return boost::filesystem::is_directory(p); }
  bool is_regular_file( const path& p ) { return boost::filesystem::is_regular_file(p); }
  uint64_t file_size( const path& p ) { return boost::filesystem::file_size(p); }
  void remove_all( const path& p ) { boost::filesystem::remove_all(p); }
  void copy( const path& f, const path& t ) { 
     try {
  	    boost::filesystem::copy( boost::filesystem::path(f), boost::filesystem::path(t) ); 
     } catch ( boost::system::system_error& e ) {
     	FC_THROW_REPORT( "Copy from ${srcfile} to ${dstfile} failed because ${reason}",
	         fc::value().set("srcfile",f).set("dstfile",t).set("reason",e.what() ) );
     } catch ( ... ) {
     	FC_THROW_REPORT( "Copy from ${srcfile} to ${dstfile} failed",
	         fc::value().set("srcfile",f).set("dstfile",t).set("inner", fc::except_str() ) );
     }
  }
  void create_hard_link( const path& f, const path& t ) { 
     try {
        boost::filesystem::create_hard_link( f, t ); 
     } catch ( ... ) {
         FC_THROW_REPORT( "Unable to create hard link from '${from}' to '${to}'", 
                          fc::value().set( "from", f )
                          .set("to",t).set("exception", fc::except_str() ) );
     }
  }
  bool remove( const path& f ) { 
     try {
        return boost::filesystem::remove( f ); 
     } catch ( ... ) {
         FC_THROW_REPORT( "Unable to remove '${path}'", fc::value().set( "path", f ).set("exception", fc::except_str() ) );
     }
  }
  fc::path canonical( const fc::path& p ) { 
     try {
        return boost::filesystem::canonical(p); 
     } catch ( ... ) {
         FC_THROW_REPORT( "Unable to resolve path '${path}'", fc::value().set( "path", p ).set("exception", fc::except_str() ) );
     }
  }
  fc::path absolute( const fc::path& p ) { return boost::filesystem::absolute(p); }
  path     unique_path() { return boost::filesystem::unique_path(); }
  path     temp_directory_path() { return boost::filesystem::temp_directory_path(); }
}
