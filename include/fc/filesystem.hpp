#pragma once
#include <utility>
#include <fc/string.hpp>
#include <fc/reflect/typename.hpp>
#include <fc/optional.hpp>
#include <fc/fwd.hpp>

namespace boost {
  namespace filesystem {
    class path;
    class directory_iterator;
    class recursive_directory_iterator;
  }
}


namespace fc {
  /**
   *  @brief wraps boost::filesystem::path to provide platform independent path manipulation.
   *
   *  Most calls are simply a passthrough to boost::filesystem::path, however exceptions are
   *  wrapped in an fc::error_report and the additional helper method fc::path::windows_string(),
   *  can be used to calculate paths intended for systems different than the host.
   *
   *  @note Serializes to a fc::value() as the result of generic_string()
   */
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
      friend bool operator < ( const fc::path& p, const fc::path& );

      operator boost::filesystem::path& ();
      operator const boost::filesystem::path& ()const;

      void       replace_extension( const fc::path& e );
      fc::path   stem()const;
      fc::path   extension()const;
      fc::path   filename()const;
      fc::path   parent_path()const;
      fc::string string()const;
      fc::string generic_string()const;

      /**
       * @brief replaces '/' with '\' in the result of generic_string()
       *
       * @note not part of boost::filesystem::path
       */
      fc::string windows_string()const;

      bool       is_relative()const;
      bool       is_absolute()const;

      static char    separator_char;

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
  class recursive_directory_iterator {
    public:
      recursive_directory_iterator( const fc::path& p );
      recursive_directory_iterator();
      ~recursive_directory_iterator();

      fc::path            operator*()const;
      recursive_directory_iterator& operator++(int);
      recursive_directory_iterator& operator++();
      void pop();
      int  level();


      friend bool operator==( const recursive_directory_iterator&, const recursive_directory_iterator& );
      friend bool operator!=( const recursive_directory_iterator&, const recursive_directory_iterator& );
    private:
      fwd<boost::filesystem::recursive_directory_iterator,16> _p; 
  };

  bool     exists( const path& p );
  bool     is_directory( const path& p );
  bool     is_regular_file( const path& p );
  void     create_directories( const path& p );
  void     remove_all( const path& p );
  path     absolute( const path& p );
  path     make_relative(const path& from, const path& to);
  path     canonical( const path& p );
  uint64_t file_size( const path& p );
  bool     remove( const path& p );
  void     copy( const path& from, const path& to );
  void     rename( const path& from, const path& to );
  void     resize_file( const path& file, size_t s );

  void     create_hard_link( const path& from, const path& to );

  path     unique_path();
  path     temp_directory_path();

  class variant;
  void to_variant( const fc::path&,  fc::variant&  );
  void from_variant( const fc::variant& , fc::path& );

  template<> struct get_typename<path> { static const char* name()   { return "path";   } };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_file_base
  {
  public:
     inline ~temp_file_base() { remove(); }
     inline operator bool() const { return _path; }
     inline bool operator!() const { return !_path; }
     const fc::path& path() const;
     void remove();
     void release();
  protected:
     typedef fc::optional<fc::path> path_t;
     inline temp_file_base(const path_t& path) : _path(path) {}
     inline temp_file_base(path_t&& path) : _path(std::move(path)) {}
     path_t _path;
  };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_file : public temp_file_base
  {
  public:
     temp_file(temp_file&& other);
     temp_file& operator=(temp_file&& other);
     temp_file(const fc::path& tempFolder = fc::temp_directory_path(), bool create = false);
  };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_directory : public temp_file_base
  {
  public:
     temp_directory(temp_directory&& other);
     temp_directory& operator=(temp_directory&& other);
     temp_directory(const fc::path& tempFolder = fc::temp_directory_path());
  };

}

