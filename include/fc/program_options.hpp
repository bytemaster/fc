#ifndef _FC_PROGRAM_OPTIONS_HPP_
#define _FC_PROGRAM_OPTIONS_HPP_
#include <fc/optional.hpp>
#include <fc/string.hpp>
#include <vector>
#include <string>
#include <fc/fwd.hpp>

namespace boost {
  namespace program_options {
    class variables_map;
  }
}

namespace fc {
  class ostream;

  namespace program_options {
    template<typename T>
    class value {
      public:
        value( T* v ):_v(v){}
        value& default_value( const T& d ) { _default = d; }

        T* get()const { return _v; }
      private:
        fc::optional<T>  _default;
        T* _v;
    };

    class options_description {
      public:
        options_description( const char* c );
        ~options_description();
  
        options_description& add_options();
        options_description& operator()( const char* o, const char* desc );
        options_description& operator()( const char* o, const value<fc::string>&,               const char* desc );
        options_description& operator()( const char* o, const value<uint16_t>&,                 const char* desc );
        options_description& operator()( const char* o, const value<std::vector<std::string> >&, const char* desc );

      private:
        class impl;
        fwd<impl,104> my;

        friend class variables_map;
        friend fc::ostream& operator<<( fc::ostream& o, const options_description& );
    };

    class variables_map {
      public:
        variables_map();
        ~variables_map();

        void parse_command_line( int argc, char** argv, const options_description& d );
        int count( const char* opt );
      private:
        class impl;
        fwd<impl,160> my;
    };

 } 
}
#endif // _FC_PROGRAM_OPTIONS_HPP_
