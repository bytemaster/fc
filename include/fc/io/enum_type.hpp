#pragma once
#include <fc/reflect/reflect.hpp>
#include <fc/io/raw_fwd.hpp>


namespace fc
{
  template<typename IntType, typename EnumType>
  class enum_type
  {
    public:
      enum_type( EnumType t )
      :value(t){}
      
      enum_type( IntType t )
      :value( (EnumType)t ){}
      
      enum_type(){}
      
      operator IntType()const  { return static_cast<IntType>(value); }
      operator EnumType()const { return value; }
      
      enum_type& operator=( IntType i )  { value = (EnumType)i; return *this;}
      enum_type& operator=( EnumType i ) { value = i; return *this;}
      
      EnumType value;
  };

  /** reflects like an enum */
  template<typename IntType, typename EnumType>
  struct reflector< enum_type<IntType,EnumType> >
  {
     typedef EnumType type;
     typedef fc::true_type is_defined;
     typedef fc::true_type is_enum;
     
     template<typename Visitor> 
     static inline void visit( const Visitor& v ) 
     { 
        reflector<EnumType>::visit(v);
     }
     static const char* to_string(int64_t i) 
     { 
        return reflector<EnumType>::to_string(i);
     }
     static EnumType from_string(const char* s)
     { 
        return reflector<EnumType>::from_string(s);
     }
  };

  /** serializes like an IntType */
  namespace raw 
  { 
    template<typename Stream, typename IntType, typename EnumType>
    inline void pack( Stream& s, const fc::enum_type<IntType,EnumType>& tp )
    {
       fc::raw::pack( s, static_cast<IntType>(tp) );
    }

    template<typename Stream, typename IntType, typename EnumType>
    inline void unpack( Stream& s, fc::enum_type<IntType,EnumType>& tp )
    {
       IntType t;
       fc::raw::unpack( s, t );
       tp = t;
    }
  }

}


