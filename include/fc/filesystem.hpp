#ifndef _FC_FILESYSTEM_HPP_
#define _FC_FILESYSTEM_HPP_
#include <fc/string.hpp>
#include <fc/fwd.hpp>

namespace boost {
  namespace filesystem {
    class path;
    class directory_iterator;
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
      friend bool operator ==( const fc::path& p, const fc::path& );
      friend bool operator !=( const fc::path& p, const fc::path& );

      operator boost::filesystem::path& ();
      operator const boost::filesystem::path& ()const;

      fc::path   stem()const;
      fc::path   extension()const;
      fc::path   filename()const;
      fc::path   parent_path()const;
      fc::string string()const;
      fc::string generic_string()const;
    private:
      fwd<boost::filesystem::path,32> _p; 
  };

  class directory_iterator {
    public:
      directory_iterator( const fc::path& p );
      directory_iterator();
      ~directory_iterator();

      fc::path            operator*()const;
      directory_iterator& operator++(int);
      directory_iterator& operator++();

      friend bool operator==( const directory_iterator&, const directory_iterator& );
      friend bool operator!=( const directory_iterator&, const directory_iterator& );
    private:
      fwd<boost::filesystem::directory_iterator,16> _p; 
  };

  bool     exists( const path& p );
  bool     is_directory( const path& p );
  bool     is_regular_file( const path& p );
  void     create_directories( const path& p );
  path     canonical( const path& p );
  uint64_t file_size( const path& p );
  bool     remove( const path& p );
  void     copy( const path& from, const path& to );

  path     unique_path();
  path     temp_directory_path();
}

#endif // _FC_FILESYSTEM_HPP_
