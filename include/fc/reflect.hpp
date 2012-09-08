#ifndef _FC_REFLECT_HPP_
#define _FC_REFLECT_HPP_
#include <stdint.h>
#include <fc/abstract_types.hpp>
#include <fc/fwd.hpp>
#include <fc/reflect_fwd.hpp>

namespace fc { 

  class string;

  class abstract_visitor;
  class abstract_const_visitor;
  class abstract_reflector;

  // provides reference semantics
  class ref {
    public:
      template<typename T>
      ref( T& v );
      
      ref( const ref& v )
      :_obj(v._obj),_reflector(v._reflector){}

      ref( void* o, abstract_reflector& r )
      :_obj(o),_reflector(r){}
      
      void* _obj;
      abstract_reflector& _reflector;

    private: 
      ref& operator=(const ref& o);
  };

  class cref {
    public:
      template<typename T>
      cref( const T& v );
      
      cref( const cref& v )
      :_obj(v._obj),_reflector(v._reflector){}
      
      cref( const ref& v )
      :_obj(v._obj),_reflector(v._reflector){}

      cref( const void* o, abstract_reflector& r )
      :_obj(o),_reflector(r){}
      
      const void* _obj;
      abstract_reflector& _reflector;

    private: 
      cref& operator=(const cref& o);
  };


  class abstract_reflector : virtual public abstract_value_type {
    public:
     virtual ~abstract_reflector(){}
     virtual const char* name()const = 0;
     virtual void        visit( void* s,       const abstract_visitor& v )const = 0; 
     virtual void        visit( const void* s, const abstract_const_visitor& v )const = 0; 
     virtual ref         get_member(void*, uint64_t) = 0;
     virtual cref        get_member(const void*, uint64_t) = 0;
     virtual ref         get_member(void*, const char*) = 0;
     virtual cref        get_member(const void*, const char*) = 0;
     virtual size_t      member_count(const void*) = 0;

  };
  
  class abstract_visitor {
    public:
      virtual ~abstract_visitor(){}
      virtual void visit()const=0;
      virtual void visit( char& c )const=0;
      virtual void visit( uint8_t& c )const=0;
      virtual void visit( uint16_t& c )const=0;
      virtual void visit( uint32_t& c )const=0;
      virtual void visit( uint64_t& c )const=0;
      virtual void visit( int8_t& c )const=0;
      virtual void visit( int16_t& c )const=0;
      virtual void visit( int32_t& c )const=0;
      virtual void visit( int64_t& c )const=0;
      virtual void visit( double& c )const=0;
      virtual void visit( float& c )const=0;
      virtual void visit( bool& c )const=0;
      virtual void visit( fc::string& c )const=0;
      virtual void visit( const char* member, int idx, int size, const ref& v)const=0;
      virtual void visit( int idx, int size, const ref& v)const=0;
      virtual void array_size( int size )const=0;
      virtual void object_size( int size )const=0;
  };

  class abstract_const_visitor {
    public:
      virtual ~abstract_const_visitor(){}
      virtual void visit()const=0;
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
      virtual void array_size( int size )const=0;
      virtual void object_size( int size )const=0;
  };

  namespace detail {
      template<typename T, typename Derived>
      class reflector_impl : virtual public value_type<T>, virtual public abstract_reflector {
         virtual ref         get_member(void*, uint64_t) {
          int x = 0;
          return x;
         }
         virtual cref        get_member(const void*, uint64_t) {
          int x = 0;
          return x;
         }
         // throw if field is not found
         virtual ref         get_member(void*, const char*) {
          int x = 0;
          return x;
            // init static hash map the first time it is called...
            // lookup field in hash map, return ref
            //return ref();
         }
         // throw if field is not found
         virtual cref        get_member(const void*, const char*) {
          int x = 0;
          return x;
            // init static hash map the first time it is called...
            // lookup field in hash map, return ref
            //return cref();
         }
         // throw if field is not found
         virtual size_t      member_count(const void*) {
            // init static hash map the first time it is called...
            // lookup field in hash map, return ref
            return 0;
         }
      };
  }


  template<typename T>
  struct get_typename {};
  template<> struct get_typename<int32_t>  { static const char* name()  { return "int32_t";  } };
  template<> struct get_typename<int64_t>  { static const char* name()  { return "int64_t";  } };
  template<> struct get_typename<int16_t>  { static const char* name()  { return "int16_t";  } };
  template<> struct get_typename<int8_t>   { static const char* name()  { return "int8_t";   } };
  template<> struct get_typename<uint32_t> { static const char* name()  { return "uint32_t"; } };
  template<> struct get_typename<uint64_t> { static const char* name()  { return "uint64_t"; } };
  template<> struct get_typename<uint16_t> { static const char* name()  { return "uint16_t"; } };
  template<> struct get_typename<uint8_t>  { static const char* name()  { return "uint8_t";  } };
  template<> struct get_typename<double>   { static const char* name()  { return "double";   } };
  template<> struct get_typename<float>    { static const char* name()  { return "float";    } };
  template<> struct get_typename<bool>     { static const char* name()  { return "bool";     } };
  template<> struct get_typename<string>   { static const char* name()  { return "string";   } };

  template<typename T>
  class reflector : public detail::reflector_impl<T, reflector<T> >{
    public:
      virtual const char* name()const { return get_typename<T>::name(); }
      virtual void visit( void* s, const abstract_visitor& v )const {
         v.visit( *((T*)s) );
      }
      virtual void visit( const void* s, const abstract_const_visitor& v )const {
         v.visit( *((const T*)s) );
      }

      static reflector& instance() { static reflector inst; return inst; }
  };

  template<typename T> reflector<T>& reflect( const T& ) { return reflector<T>::instance(); }

  template<typename T>
  ref::ref( T& v ) :_obj(&v),_reflector(reflector<T>::instance()){}

  template<typename T>
  cref::cref( const T& v ) :_obj(&v),_reflector(reflector<T>::instance()){}

  template<typename T,unsigned int S>
  class reflector<fwd<T,S>>;

} // namespace fc


#endif // _REFLECT_HPP_
