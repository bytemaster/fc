#ifndef _FC_FILESYSTEM_HPP_
#define _FC_FILESYSTEM_HPP_
#include <fc/string.hpp>
#include <fc/fwd.hpp>

namespace boost {
  namespace filesystem {
    class path;
  }
}


namespace fc {
  class path {
    public:
      path();
      ~path();
      path( const boost::filesystem::path& );
      path( const fc::string& p );
      path( const char* );
      path( const path& p );
      path( path&& p );
      path& operator =( const path& );
      path& operator =( path&& );

      path& operator /=( const fc::path& );
      friend path operator /( const fc::path& p, const fc::path& );

      operator boost::filesystem::path& ();
      operator const boost::filesystem::path& ()const;

      fc::string string()const;
    private:
      fwd<boost::filesystem::path,8> _p; 
  };

  bool exists( const path& p );
  void create_directories( const path& p );
}

#endif // _FC_FILESYSTEM_HPP_
