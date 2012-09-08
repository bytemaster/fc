#ifndef _FC_VALUE_CAST_HPP_
#define _FC_VALUE_CAST_HPP_
#include <fc/value.hpp>
#include <fc/reflect.hpp>
#include <fc/error.hpp>
#include <fc/exception.hpp>

namespace fc {

  template<typename T>
  const T& value_cast( const value& v ) {
    if( &reflector<T>::instance() == v._obj_type ) {
      if( v._obj_type->size_of() <= 8 ) {
        slog( "stack..." );
         return *((const T*)&v._obj);
      }
        slog( "heap..." );
      return *((const T*)v._obj);
    }
    FC_THROW( bad_cast() );
  }
  template<typename T>
  T& value_cast( value& v ) {
    if( &reflector<T>::instance() == v._obj_type ) {
      if( v._obj_type->size_of() <= 8 ) {
        slog( "stack..." );
         return *((T*)&v._obj);
      }
        slog( "heap..." );
      return *((T*)v._obj);
    }
    FC_THROW( bad_cast() );
  }

  template<typename T>
  T* value_cast( value* v ) {
  }

  template<typename T>
  const T* value_cast( const value* v ) {
  }


  template<typename T> class reinterpret_value_visitor;

  #define CAST_VISITOR_DECL(X)  \
  template<> class reinterpret_value_visitor<X> : public abstract_const_visitor {  \
    private: X& _s; \
    public: \
      reinterpret_value_visitor( X& s ):_s(s){} \
      virtual void visit()const; \
      virtual void visit( const char& c )const; \
      virtual void visit( const uint8_t& c )const; \
      virtual void visit( const uint16_t& c )const; \
      virtual void visit( const uint32_t& c )const; \
      virtual void visit( const uint64_t& c )const; \
      virtual void visit( const int8_t& c )const; \
      virtual void visit( const int16_t& c )const; \
      virtual void visit( const int32_t& c )const; \
      virtual void visit( const int64_t& c )const; \
      virtual void visit( const double& c )const; \
      virtual void visit( const float& c )const; \
      virtual void visit( const bool& c )const; \
      virtual void visit( const string& c )const; \
      virtual void visit( const char* member, int idx, int size, const cref& v)const;\
      virtual void visit( int idx, int size, const cref& v)const; \
      virtual void array_size( int size )const{} \
      virtual void object_size( int size )const{} \
  }

  CAST_VISITOR_DECL(int64_t);
  CAST_VISITOR_DECL(int32_t);
  CAST_VISITOR_DECL(int16_t);
  CAST_VISITOR_DECL(int8_t);
  CAST_VISITOR_DECL(uint64_t);
  CAST_VISITOR_DECL(uint32_t);
  CAST_VISITOR_DECL(uint16_t);
  CAST_VISITOR_DECL(uint8_t);
  CAST_VISITOR_DECL(double);
  CAST_VISITOR_DECL(float);
  CAST_VISITOR_DECL(bool);
  CAST_VISITOR_DECL(string);


  template<typename T>
  T reinterpret_value_cast( const value& v ) {
    if( v.is_null() ) FC_THROW( bad_cast() );
    T r;
    reinterpret_value_visitor<T> vis(r);
    if( v._obj_type->size_of() > sizeof(v._obj) )
        v._obj_type->visit( v._obj, vis );
    else 
        v._obj_type->visit( &v._obj, vis );
    return r;
  }

}


#endif // _FC_VALUE_CAST_HPP_
