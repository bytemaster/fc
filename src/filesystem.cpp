//#define BOOST_NO_SCOPED_ENUMS
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/utility.hpp>
#include <fc/io/fstream.hpp>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <fc/variant.hpp>

namespace fc {
  void to_variant( const fc::path& t, variant& v ) {
    v = t.generic_string();
  }
  void from_variant( const fc::variant& v, fc::path& t ) {
    t = fc::path(v.as_string());
  }

   // Note: we can do this cast because the separator should be an ASCII character
   char path::separator_char = static_cast<char>(boost::filesystem::path("/").make_preferred().native()[0]);

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
     for( size_t i =0 ; i < gs.size(); ++i ) {
       if( gs[i] == '/' ) gs[i] = '\\';
     }
     return gs;
   }

   fc::string path::string()const {
    return _p->string();
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
      FC_THROW( "Unable to create directories ${path}", ("path", p )("inner", fc::except_str() ) );
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
     	FC_THROW( "Copy from ${srcfile} to ${dstfile} failed because ${reason}",
	         ("srcfile",f)("dstfile",t)("reason",e.what() ) );
     } catch ( ... ) {
     	FC_THROW( "Copy from ${srcfile} to ${dstfile} failed",
	         ("srcfile",f)("dstfile",t)("inner", fc::except_str() ) );
     }
  }
  void resize_file( const path& f, size_t t ) 
  { 
    try {
      boost::filesystem::resize_file( f, t );
    } 
    catch ( boost::system::system_error& e )
    {
      FC_THROW( "Resize file '${f}' to size ${s} failed: ${reason}",
                ("f",f)("s",t)( "reason", e.what() ) );
    } 
    catch ( ... ) 
    {
      FC_THROW( "Resize file '${f}' to size ${s} failed: ${reason}",
                ("f",f)("s",t)( "reason", fc::except_str() ) );
    }
  }
  void rename( const path& f, const path& t ) { 
     try {
  	    boost::filesystem::rename( boost::filesystem::path(f), boost::filesystem::path(t) ); 
     } catch ( boost::system::system_error& e ) {
     	FC_THROW( "Rename from ${srcfile} to ${dstfile} failed because ${reason}",
	         ("srcfile",f)("dstfile",t)("reason",e.what() ) );
     } catch ( ... ) {
     	FC_THROW( "Rename from ${srcfile} to ${dstfile} failed",
	         ("srcfile",f)("dstfile",t)("inner", fc::except_str() ) );
     }
  }
  void create_hard_link( const path& f, const path& t ) { 
     try {
        boost::filesystem::create_hard_link( f, t ); 
     } catch ( ... ) {
         FC_THROW( "Unable to create hard link from '${from}' to '${to}'", 
                          ( "from", f )("to",t)("exception", fc::except_str() ) );
     }
  }
  bool remove( const path& f ) { 
     try {
        return boost::filesystem::remove( f ); 
     } catch ( ... ) {
         FC_THROW( "Unable to remove '${path}'", ( "path", f )("exception", fc::except_str() ) );
     }
  }
  fc::path canonical( const fc::path& p ) { 
     try {
        return boost::filesystem::canonical(p); 
     } catch ( ... ) {
         FC_THROW( "Unable to resolve path '${path}'", ( "path", p )("exception", fc::except_str() ) );
     }
  }
  fc::path absolute( const fc::path& p ) { return boost::filesystem::absolute(p); }
  path     unique_path() { return boost::filesystem::unique_path(); }
  path     temp_directory_path() { return boost::filesystem::temp_directory_path(); }

  // Return path when appended to a_From will resolve to same as a_To
  fc::path make_relative(const fc::path& from, const fc::path& to) {
    boost::filesystem::path a_From = boost::filesystem::absolute(from);
    boost::filesystem::path a_To = boost::filesystem::absolute(to);
    boost::filesystem::path ret;
    boost::filesystem::path::const_iterator itrFrom(a_From.begin()), itrTo(a_To.begin());
    // Find common base
    for( boost::filesystem::path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ; itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo );
    // Navigate backwards in directory to reach previously found base
    for( boost::filesystem::path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom ) {
      if( (*itrFrom) != "." )
         ret /= "..";
    }
    // Now navigate down the directory branch
    for (; itrTo != a_To.end(); ++itrTo)
      ret /= *itrTo;
    return ret;
  }

   temp_file::temp_file(const fc::path& p, bool create)
   : temp_file_base(p / fc::unique_path())
   {
      if (fc::exists(*_path))
      {
         FC_THROW( "Name collision: ${path}", ("path", _path->string()) );
      }
      if (create)
      {
         fc::ofstream ofs(*_path, fc::ofstream::out | fc::ofstream::binary);
         ofs.close();
      }
   }

   temp_file::temp_file(temp_file&& other)
      : temp_file_base(std::move(other._path))
   {
   }

   temp_file& temp_file::operator=(temp_file&& other)
   {
      if (this != &other)
      {
         remove();
         _path = std::move(other._path);
      }
      return *this;
   }

   temp_directory::temp_directory(const fc::path& p)
   : temp_file_base(p / fc::unique_path())
   {
      if (fc::exists(*_path))
      {
         FC_THROW( "Name collision: ${path}", ("path", _path->string()) );
      }
      fc::create_directories(*_path);
   }

   temp_directory::temp_directory(temp_directory&& other)
      : temp_file_base(std::move(other._path))
   {
   }

   temp_directory& temp_directory::operator=(temp_directory&& other)
   {
      if (this != &other)
      {
         remove();
         _path = std::move(other._path);
      }
      return *this;
   }

   const fc::path& temp_file_base::path() const
   {
      if (!_path)
      {
         FC_THROW( "Temporary directory has been released." );
      }
      return *_path;
   }

   void temp_file_base::remove()
   {
      if (_path)
      {
         try
         {
            fc::remove_all(*_path);
         }
         catch (...)
         {
            // eat errors on cleanup
         }
         release();
      }
   }

   void temp_file_base::release()
   {
      _path = fc::optional<fc::path>();
   }
}
