#ifndef _FC_VALUE_HPP_
#define _FC_VALUE_HPP_
#include <stdint.h>
#include <fc/utility.hpp>
#include <fc/reflect_fwd.hpp>
#include <fc/vector_fwd.hpp>
#include <fc/fwd.hpp>

namespace fc {
  class string;
   
  /**
   * @brief dynamic type that will store any reflected type.
   *
   * A struct can be stored directly or 'exploded' to be stored
   * as individual elements.  Direct storage is more effecient (no
   * need to allocate/manage keys), but does not support adding / removing
   * keys.  
   *
   */
  class value {
    public:
      struct member {
        member();
        member(const char* key);
        member(string&& key );

        const string& key()const; 
        value&        val();
        const value&  val()const;

        private:
          friend class value;
          friend class reflector<member>;
          fwd<string,8>  _key;
          fwd<value,16>  _val;
      };
      typedef member* iterator;
      typedef const member* const_iterator;

      value();

      template<typename T>
      explicit value( T&& t ):_obj(nullptr),_obj_type(nullptr) {
          *this = cref(fc::forward<T>(t));
      }

      value( value&& v );
      value( const value& v );
      value( const cref& v  );
      ~value();

      value& operator=( value&&      v );
      value& operator=( const value& v );
      value& operator=( const cref&  v );

      template<typename T>
      value& operator=( T&& t ) {
          value temp(fc::forward<T>(t));
          swap(temp,*this);
          return *this;
      }

      template<typename T>
      value& push_back( T&&  v ) { return push_back( value( forward<T>(v) ) ); }
      value& push_back( value&&  v );
      value& push_back( const value&  v );

      /**
       *  These methods will create the key if it
       *  does not exist.
       * @{
       */
      /**
       *  @pre value is null or an object
       */
      value& operator[]( const string& key );
      /**
       *  @pre value is null or an object
       */
      value& operator[]( const char* key );
      /**
       *  @pre value is null or an array or index is 0
       */
      value& operator[]( uint64_t index );
      value& operator[]( int index );
      /** @}  */

      value& operator[]( string&& key );

      const value& operator[]( const string& key )const;
      const value& operator[]( const char* key )const;
      const value& operator[]( uint64_t )const;

      bool key_exists( const string& key );
      bool key_exists( const char* key );
      bool is_array()const;
      bool is_object()const;
      bool is_null()const;
      bool is_string()const;
      bool is_real()const;
      bool is_float()const;
      bool is_double()const;
      bool is_integer()const;
      bool is_int64()const;
      bool is_int32()const;
      bool is_int16()const;
      bool is_int8()const;
      bool is_boolean()const;

      template<typename T>
      bool is()const {
        return _obj_type == reflector<T>::instance();
      }

      fwd<vector<string>,24> get_keys()const; 

      iterator              find( const char* key );
      const_iterator        find( const char* key )const;
      iterator              begin();
      const_iterator        begin()const;
      const_iterator        end()const;

      void*                 ptr();
      const void*           ptr()const;
      abstract_reflector*   type()const;
    private:
      template<typename T> friend const T& value_cast( const value& v ); 
      template<typename T> friend T& value_cast( value& v ); 
      template<typename T> friend T* value_cast( value* v ); 
      template<typename T> friend const T* value_cast( const value* v ); 
      template<typename T> friend T reinterpret_value_cast( const value& v );

      void*                _obj;
      abstract_reflector*  _obj_type;
  };

};


#endif // _MACE_VALUE_HPP_
