#pragma once
#include <fc/exception/exception.hpp>

namespace fc {

   /**
    *  This type is designed to provide automatic checks for
    *  integer overflow and default initialization. It will
    *  throw an exception on overflow conditions.
    */
   template<typename T>
   struct safe 
   {
      template<typename O>
      safe( O o ):value(o){}
      safe(){}
      safe( const safe& o ):value(o.value){}


      safe& operator += (  const safe& b )
      {
         if( b.value > 0 && value > (std::numeric_limits<T>::max() - b.value) ) FC_CAPTURE_AND_THROW( overflow_exception, (*this)(b) );
         if( b.value < 0 && value < (std::numeric_limits<T>::min() - b.value) ) FC_CAPTURE_AND_THROW( underflow_exception, (*this)(b) );
         value += b.value;
         return *this;
      }
      friend safe operator + ( const safe& a, const safe& b )
      {
         if( b.value > 0 && a.value > std::numeric_limits<T>::max() - b.value ) FC_CAPTURE_AND_THROW( overflow_exception, (a)(b) );
         if( b.value < 0 && a.value < std::numeric_limits<T>::min() - b.value ) FC_CAPTURE_AND_THROW( underflow_exception, (a)(b) );
         return safe(a.value+b.value);
      }
      safe& operator -= (  const safe& b ) { return *this += safe(-b.value); }
      safe operator -()const{ return safe(-value); }

      friend safe operator - ( const safe& a, const safe& b )
      {
         safe tmp(a); tmp -= b; return tmp;
      }

      friend bool operator == ( const safe& a, const safe& b )
      {
         return a.value == b.value;
      }
      friend bool operator < ( const safe& a, const safe& b )
      {
         return a.value < b.value;
      }
      friend bool operator > ( const safe& a, const safe& b )
      {
         return a.value > b.value;
      }
      friend bool operator >= ( const safe& a, const safe& b )
      {
         return a.value >= b.value;
      }
      friend bool operator <= ( const safe& a, const safe& b )
      {
         return a.value <= b.value;
      }
      T value = 0;
   };

} 
