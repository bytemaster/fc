#ifndef _FC_OPTIONAL_HPP_
#define _FC_OPTIONAL_HPP_
#include <fc/utility.hpp>

namespace fc {
  /**
   *  @brief provides stack-based nullable value similar to boost::optional
   *
   *  Simply including boost::optional adds 35,000 lines to each object file, using
   *  fc::optional adds less than 400.
   */
  template<typename T>
  class optional {
    public:
      optional():_valid(0){}
      ~optional(){ if( _valid ) (**this).~T(); }

      optional( const optional& o )
      :_valid(false) {
        if( o._valid ) new (&**this) T( *o );
        _valid = o._valid;
      }

      optional( optional&& o )
      :_valid(false) {
        if( o._valid ) new (&**this) T( fc::move(*o) );
        _valid = o._valid;
      }

      template<typename U>
      optional( U&& u )
      :_valid(false) {
        new (&**this) T( fc::forward<U>(u) );
        _valid = true;
      }

      template<typename U>
      optional& operator=( U&& u ) {
        if( &u == &**this ) return *this;
        if( !_valid ) {
          new (&**this) T( fc::forward<U>(u) );
          _valid = true;
        } else {
          **this = u;
        }
        return *this;
      }

      optional& operator=( const optional& o ) {
        if( _valid && o.valid ) { **this = *o; }
        else if( !_valid && o._valid ) {
          *this = **o;
        } // else !_valid && !o._valid == same!
        return *this;
      }

      bool operator!()const { return !_valid; }

      T&       operator*()      { void* v = &_value[0]; return *static_cast<T*>(v); }
      const T& operator*()const { const void* v = &_value[0]; return *static_cast<const T*>(v); }

      T&       operator->()      { void* v = &_value[0]; return *static_cast<T*>(v); }
      const T& operator->()const { const void* v = &_value[0]; return *static_cast<const T*>(v); }

    private:
      // force alignment... to 8 byte boundaries 
      double _value[8 * ((sizeof(T)+7)/8)];
      bool   _valid;
  };

  template<typename T>
  bool operator == ( const optional<T>& left, const optional<T>& right ) {
    return (!left == !right) || (!!left && *left == *right);
  }
  template<typename T, typename U>
  bool operator == ( const optional<T>& left, const U& u ) {
    return !!left && *left == u;
  }
  template<typename T>
  bool operator != ( const optional<T>& left, const optional<T>& right ) {
    return (!left != !right) || (!!left && *left != *right);
  }
  template<typename T, typename U>
  bool operator != ( const optional<T>& left, const U& u ) {
    return !left || *left != u;
  }

} // namespace fc

#endif
