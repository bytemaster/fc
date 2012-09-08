#ifndef _REFLECT_CAST_HPP_
#define _REFLECT_CAST_HPP_
#include <fc/reflect.hpp>

namespace fc { 
  /**
   *  This is specialized for each type to implement a cast
   *  from a reflected value.
   *
   *  By default the cast will only work for 'exact' matches of
   *  type.   Use duck_cast<T> for a more flexible field-by-field
   *  cast.
   */
  template<typename T>
  class const_cast_visitor : public abstract_const_visitor {
    public:
      const_cast_visitor( T& s ):_s(s){}

      virtual void visit()=0;
      virtual void visit( const char& c )const=0;
      virtual void visit( const uint8_t& c )const=0;
      virtual void visit( const uint16_t& c )const=0;
      virtual void visit( const uint32_t& c )const=0;
      virtual void visit( const uint64_t& c )const=0;
      virtual void visit( const int8_t& c )const=0;
      virtual void visit( const int16_t& c )const=0;
      virtual void visit( const int32_t& c )const=0;
      virtual void visit( const int64_t& c )const=0;
      virtual void visit( const double& c )const=0;
      virtual void visit( const float& c )const=0;
      virtual void visit( const bool& c )const=0;
      virtual void visit( const fc::string& c )const=0;
      virtual void visit( const char* member, int idx, int size, const cref& v)const=0;
      virtual void visit( int idx, int size, const cref& v)const=0;
  };



} // namespace fc
#endif // _REFLECT_CAST_HPP_
