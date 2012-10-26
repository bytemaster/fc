#pragma once

#include <fc/reflect.hpp>
#include <fc/optional.hpp>
#include <fc/value.hpp>
#include <fc/value_cast.hpp>
#include <fc/tuple.hpp>

namespace fc {
  struct void_t{};

  template<typename T>
  void pack(fc::value& jsv, const T& v ); 

  template<typename T> 
  void unpack( const fc::value& jsv, T& v ); 

  template<typename A, typename B, typename C, typename D>
  void pack( fc::value& v, const tuple<A,B,C,D>& t );
  template<typename A, typename B, typename C, typename D>
  void unpack( const fc::value& val, tuple<A,B,C,D>& t ); 

  template<typename T> 
  void pack( fc::value& jsv, const fc::optional<T>& v );

  template<typename T> 
  void unpack( const fc::value& jsv, fc::optional<T>& v );

  inline void pack( fc::value& jsv, const char& v )         { jsv = fc::string(&v,1); }
  inline void pack( fc::value& jsv, const fc::value& v )    { jsv = v; }
  inline void pack( fc::value& jsv, fc::value& v )          { jsv = v; }
  inline void pack( fc::value& jsv, fc::value&& v )         { jsv = fc::move(v); }
  inline void pack( fc::value& jsv, const void_t& v )       { jsv = fc::value(); }
  inline void pack( fc::value& jsv, const bool& v )         { jsv = v; }
  inline void pack( fc::value& jsv, const float& v )        { jsv = v; }
  inline void pack( fc::value& jsv, const double& v )       { jsv = v; }
  inline void pack( fc::value& jsv, const uint8_t& v )      { jsv = v; }
  inline void pack( fc::value& jsv, const uint16_t& v )     { jsv = v; }
  inline void pack( fc::value& jsv, const uint32_t& v )     { jsv = v; }
  inline void pack( fc::value& jsv, const uint64_t& v )     { jsv = v; }
  inline void pack( fc::value& jsv, const int8_t& v )       { jsv = v; }
  inline void pack( fc::value& jsv, const int16_t& v )      { jsv = v; }
  inline void pack( fc::value& jsv, const int32_t& v )      { jsv = v; }
  inline void pack( fc::value& jsv, const int64_t& v )      { jsv = v; }
  inline void pack( fc::value& jsv, const fc::string& v )   { jsv = v; }
  inline void pack( fc::value& jsv, fc::string& v )         { jsv = v; }
  inline void pack( fc::value& jsv, fc::string&& v )        { jsv = fc::move(v); }
  inline void pack( fc::value& jsv, const char* v )         { jsv = fc::string(v); }

  void pack( fc::value& jsv, const fc::vector<char>& value );
  template<typename T>
  void pack( fc::value& jsv, const fc::vector<T>& value );


  inline void unpack( const fc::value& jsv, fc::value& v ) { v = jsv; }
  template<typename T>
  void unpack( const fc::value& jsv, const T& v ); 
  template<typename T> 
  void unpack( const fc::value& jsv, T& v ); 
  void unpack( const fc::value& jsv, bool& v );

  inline void unpack( const fc::value& jsv, void_t& v ){ };

  void unpack( const fc::value& jsv, float& v );
  void unpack( const fc::value& jsv, double& v );
  void unpack( const fc::value& jsv, uint8_t& v );
  void unpack( const fc::value& jsv, uint16_t& v );
  void unpack( const fc::value& jsv, uint32_t& v );
  void unpack( const fc::value& jsv, uint64_t& v );
  void unpack( const fc::value& jsv, int8_t& v );
  void unpack( const fc::value& jsv, int16_t& v );
  void unpack( const fc::value& jsv, int32_t& v );
  void unpack( const fc::value& jsv, int64_t& v );
  void unpack( const fc::value& jsv, fc::string& v );

  void unpack( const fc::value& jsv, fc::vector<double>& value );

  void unpack( const fc::value& jsv, fc::vector<char>& value );
  template<typename T>
  void unpack( const fc::value& jsv, fc::vector<T>& value );

  namespace detail {
    template<typename Class>
    struct pack_object_visitor {
      pack_object_visitor(const Class& _c, fc::value& _val)
      :c(_c),obj(_val){}

      /**
      VC++ does not understand the difference of return types, so an extra layer is needed.
      */
      template<typename T>
      inline void pack_helper( const T& v, const char* name )const {
        fc::pack( obj[name], v ); 
      }
      template<typename T>
      inline void pack_helper( const fc::optional<T>& v, const char* name )const {
        if( !!v ) {
          fc::pack( obj[name], *v ); 
        }
      }
      template<typename T, typename C, T (C::*p)>
      inline void operator()( const char* name )const {
        pack_helper( c.*p, name );
      }

      private:            
        const Class&       c;
        fc::value& obj;
    };

    template<typename T>
    struct is_optional {
      typedef fc::false_type type;
    };
    template<typename T>
    struct is_optional<fc::optional<T> > {
      typedef fc::true_type type;
    };

    template<typename Class>
    struct unpack_object_visitor  {
      unpack_object_visitor(Class& _c, const fc::value& _val)
      :c(_c),obj(_val){}

      template<typename T, typename C, T (C::*p)>
      void operator()( const char* name )const {
         if( obj.find(name) != obj.end()) {
             fc::unpack( obj[name], c.*p );
         }
         else {
            if( !is_optional< typename fc::remove_reference<decltype(c.*p)>::type >::type::value ) {
                wlog( "unable to find name: '%1%'",name);
            }
         }
      }
      Class&                  c;
      const fc::value& obj;
    };

    template<typename IsReflected=fc::false_type>
    struct if_enum {
      template<typename T>
      static inline void pack( fc::value& jsv, const T& v ) { 
         jsv = fc::value::object();
         detail::pack_object_visitor<T> pov(v,jsv);
         fc::reflector<T>::visit(pov);
      }
      template<typename T>
      static inline void unpack( const fc::value& jsv, T& v ) { 
         detail::unpack_object_visitor<T> pov(v,jsv );
         fc::reflector<T>::visit(pov);
      }
    };

    template<> struct if_enum<fc::true_type> {
      template<typename T>
      static inline void pack( fc::value& jsv, const T& v ) { 
         fc::pack( jsv, fc::reflector<T>::to_string(v) );
      }
      template<typename T>
      static inline void unpack( const fc::value& jsv, T& v ) { 
         if( strcmp( jsv.type(), "string" ) == 0 ) {
            v = fc::reflector<T>::from_string( value_cast<fc::string>(jsv).c_str() );
         } else {
            // throw if invalid int, by attempting to convert to string
            fc::reflector<T>::to_string( v = value_cast<int64_t>(jsv) );
         }
      }
    };


    template<typename IsReflected=fc::false_type>
    struct if_reflected {
      template<typename T>
      static inline void pack(fc::value& s, const T& v ) { 
        v.did_not_implement_reflect_macro();
      }
      template<typename T>
      static inline void unpack( const fc::value& s, T& v ) { 
        v.did_not_implement_reflect_macro();
        //wlog( "warning, ignoring unknown type" );
      }
    };

    template<>
    struct if_reflected<fc::true_type> {
      template<typename T>
      static inline void pack( fc::value& jsv, const T& v ) { 
         if_enum<typename fc::reflector<T>::is_enum>::pack( jsv,v );
      }
      template<typename T>
      static inline void unpack( const fc::value& jsv, T& v ) { 
         if_enum<typename fc::reflector<T>::is_enum>::unpack( jsv,v );
      }
    };

  } // namesapce detail

  inline void unpack( const fc::value& jsv, char& v )         { 
    auto s = value_cast<fc::string>(jsv); 
    if( s.size() ) v = s[0];
  }
  inline void unpack( const fc::value& jsv, bool& v )         { v = value_cast<bool>(jsv);          }
  inline void unpack( const fc::value& jsv, float& v )        { v = value_cast<float>(jsv);         }
  inline void unpack( const fc::value& jsv, double& v )       { v = value_cast<double>(jsv);        }
  inline void unpack( const fc::value& jsv, uint8_t& v )      { v = value_cast<uint8_t>(jsv);       }
  inline void unpack( const fc::value& jsv, uint16_t& v )     { v = value_cast<uint16_t>(jsv);      }
  inline void unpack( const fc::value& jsv, uint32_t& v )     { v = value_cast<uint32_t>(jsv);      }
  inline void unpack( const fc::value& jsv, uint64_t& v )     { v = value_cast<uint64_t>(jsv);      }
  inline void unpack( const fc::value& jsv, int8_t& v )       { v = value_cast<int8_t>(jsv);        }
  inline void unpack( const fc::value& jsv, int16_t& v )      { v = value_cast<int16_t>(jsv);       }
  inline void unpack( const fc::value& jsv, int32_t& v )      { v = value_cast<int32_t>(jsv);       }
  inline void unpack( const fc::value& jsv, int64_t& v )      { v = value_cast<int64_t>(jsv);       }
  inline void unpack( const fc::value& jsv, fc::string& v )   { v = value_cast<fc::string>(jsv);    }

  template<typename T> 
  void pack( fc::value& jsv, const fc::optional<T>& v ) {
    if( v ) pack(  jsv, *v );
    else jsv = fc::value();
  }
  template<typename T> 
  void unpack( const fc::value& jsv, fc::optional<T>& v ) {
    if( strcmp( jsv.type(), "void" ) != 0 ) {
      T tmp;
      unpack(  jsv, tmp );
      v = fc::move(tmp);
    }
  }


  template<typename T>
  inline void pack( fc::value& jsv, const fc::vector<T>& value ) {
      jsv = fc::value::array();
      jsv.resize(value.size());
      typename fc::vector<T>::const_iterator itr = value.begin();
      typename fc::vector<T>::const_iterator end = value.end();
      uint32_t i = 0;
      while( itr != end ) {
          fc::pack( jsv[i], *itr );
          ++itr;
          ++i;
      }
  }
  struct tuple_to_value_visitor {
    tuple_to_value_visitor( value& v ):_val(v),_count(0) { }
    template<typename T>
    void operator()( T&& t ) {
      _val[_count] = value(fc::forward<T>(t) );
      ++_count;
    }
    value& _val;
    int    _count;
  };
  struct tuple_from_value_visitor {
    tuple_from_value_visitor( const value& v ):_val(v),_count(0) { }
    template<typename T>
    void operator()( T&& t ) {
      if( _count < _val.size() ) unpack( _val[_count], t );
      ++_count;
    }
    const value& _val;
    int    _count;
  };

  template<typename A, typename B, typename C, typename D>
  inline void pack( fc::value& val, const tuple<A,B,C,D>& t ) {
    val = fc::value::array( tuple<A,B,C,D>::size );
    t.visit( tuple_to_value_visitor(val) );
  }
  template<typename A, typename B, typename C, typename D>
  inline void unpack( const fc::value& val, tuple<A,B,C,D>& t ) {
    val = fc::value::array( tuple<A,B,C,D>::size );
    t.visit( tuple_from_value_visitor(val) );
  }

  template<typename T>
  inline void unpack( const fc::value& jsv, fc::vector<T>& val ) {
      val.resize( jsv.size() );
      uint32_t s = jsv.size();
      for( uint32_t i = 0; i < s; ++i ) {
          unpack( jsv[i], val[i] );
      }
  }

  // default case
  template<typename T> 
  inline void pack( fc::value& jsv, const T& v ) {
      detail::if_reflected< typename fc::reflector<T>::is_defined >::pack(jsv,v);
  }

  template<typename T> 
  inline void unpack( const fc::value& jsv, T& v ) {
      detail::if_reflected< typename fc::reflector<T>::is_defined >::unpack(jsv,v);
  }

} // namespace fc 

