#pragma once
#include <fc/reflect.hpp>
#include <fc/value.hpp>
#include <fc/exception.hpp>
#include <fc/lexical_cast.hpp>
#include <fc/numeric_cast.hpp>
#include <fc/value_io.hpp>
#include <fc/tuple.hpp>
#include <fc/vector.hpp>
#include <fc/typename.hpp>
#include <fc/error_report.hpp>

//#include <typeinfo>

namespace fc {

    namespace detail {

       template<typename T>
       struct cast_visitor : value::const_visitor {
         cast_visitor( T& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int16_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int32_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int64_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint8_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint16_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint32_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint64_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const float& v       )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const double& v      )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const bool& v        )  { m_out = v; }
         virtual void operator()( const fc::string& v )   { m_out = fc::lexical_cast<T>(v); }
         virtual void operator()( const value::object&  ) { FC_THROW_REPORT("bad cast to ${type} from object", 
                                                                            fc::value().set("type",fc::get_typename<T>::name())); }
         virtual void operator()( const value::array&  )  { FC_THROW_REPORT("bad cast to ${type} from array",
                                                                            fc::value().set("type",fc::get_typename<T>::name())); }
         virtual void operator()( )                       { FC_THROW_REPORT("bad cast to ${type} from null",
                                                                            fc::value().set("type",fc::get_typename<T>::name())); } 
         private:
         T& m_out;
       };
       template<>
       struct cast_visitor<bool> : value::const_visitor {
         cast_visitor( bool& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ m_out = v != 0; }
         virtual void operator()( const int16_t& v     ){ m_out = v != 0; }
         virtual void operator()( const int32_t& v     ){ m_out = v != 0; }
         virtual void operator()( const int64_t& v     ){ m_out = v != 0; }
         virtual void operator()( const uint8_t& v     ){ m_out = v != 0; }
         virtual void operator()( const uint16_t& v    ){ m_out = v != 0; }
         virtual void operator()( const uint32_t& v    ){ m_out = v != 0; }
         virtual void operator()( const uint64_t& v    ){ m_out = v != 0; }
         virtual void operator()( const float& v       ){ m_out = v != 0; }
         virtual void operator()( const double& v      ){ m_out = v != 0; }
         virtual void operator()( const bool& v        ){ m_out = v; }
         virtual void operator()( const fc::string& v ) { m_out = !(v != "true"); }
         virtual void operator()( const value::object&  )      { FC_THROW_REPORT("bad cast to bool from object"); }
         virtual void operator()( const value::array&  )       { FC_THROW_REPORT("bad cast to bool from array");  }
         virtual void operator()( )                            { FC_THROW_REPORT("bad cast to bool from null");   }
         private:
         bool& m_out;
       };
     
       template<>
       struct cast_visitor<fc::string> : value::const_visitor {
         cast_visitor( fc::string& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int16_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int32_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int64_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint8_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint16_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint32_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint64_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const float& v       ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const double& v      ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const bool& v        ) { m_out = v != 0 ? "true" : "false";       }
         virtual void operator()( const fc::string& v )  { m_out = v;                               }
         virtual void operator()( const value::object&  ){ FC_THROW_REPORT("bad cast to string from object"); }
         virtual void operator()( const value::array&  ) { FC_THROW_REPORT("bad cast to string from array"); }
         virtual void operator()( )                      { m_out = fc::string(); }
     
         private:
         fc::string& m_out;
       };
     
       template<>
       struct cast_visitor<value::array> : value::const_visitor {
         cast_visitor( value::array& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )   { FC_THROW_REPORT("bad cast to array from int8");}
         virtual void operator()( const int16_t& v     )   { FC_THROW_REPORT("bad cast to array from int16");}
         virtual void operator()( const int32_t& v     )   { FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const int64_t& v     )   { FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const uint8_t& v     )   { FC_THROW_REPORT("bad cast to array from uint8");}
         virtual void operator()( const uint16_t& v    )   { FC_THROW_REPORT("bad cast to array from uint16");}
         virtual void operator()( const uint32_t& v    )   { FC_THROW_REPORT("bad cast to array from uint32");}
         virtual void operator()( const uint64_t& v    )   { FC_THROW_REPORT("bad cast to array from uint64");}
         virtual void operator()( const float& v       )   { FC_THROW_REPORT("bad cast to array from float");}
         virtual void operator()( const double& v      )   { FC_THROW_REPORT("bad cast to array from double");}
         virtual void operator()( const bool& v        )   { FC_THROW_REPORT("bad cast to array from bool");}
         virtual void operator()( const fc::string& v )    { FC_THROW_REPORT("bad cast to array from string");}
         virtual void operator()( const value::object&  )  { FC_THROW_REPORT("bad cast to array from object");}
         virtual void operator()( const value::array& a )  { m_out = a;              }
         virtual void operator()( )                        { m_out = value::array(); }
     
         private:
         value::array& m_out;
       };
     
       template<>
       struct cast_visitor<value::object> : value::const_visitor {
         cast_visitor( value::object& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ FC_THROW_REPORT("bad cast to array from int8");}
         virtual void operator()( const int16_t& v     ){ FC_THROW_REPORT("bad cast to array from int16");}
         virtual void operator()( const int32_t& v     ){ FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const int64_t& v     ){ FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const uint8_t& v     ){ FC_THROW_REPORT("bad cast to array from uint8");}
         virtual void operator()( const uint16_t& v    ){ FC_THROW_REPORT("bad cast to array from uint16");}
         virtual void operator()( const uint32_t& v    ){ FC_THROW_REPORT("bad cast to array from uint32");}
         virtual void operator()( const uint64_t& v    ){ FC_THROW_REPORT("bad cast to array from uint64");}
         virtual void operator()( const float& v       ){ FC_THROW_REPORT("bad cast to array from float");}
         virtual void operator()( const double& v      ){ FC_THROW_REPORT("bad cast to array from double");}
         virtual void operator()( const bool& v        ){ FC_THROW_REPORT("bad cast to array from bool");}
         virtual void operator()( const fc::string& v ) { FC_THROW_REPORT("bad cast to array from string");}
         virtual void operator()( const value::object& a )  { m_out = a;                  }
         virtual void operator()( const value::array&  )    { FC_THROW_REPORT("bad cast");}
         virtual void operator()( )                         { m_out = value::object();      }
     
         private:
         value::object& m_out;
       };
       template<>
       struct cast_visitor<fc::value> : value::const_visitor {
         cast_visitor( value& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )    { m_out = v; }
         virtual void operator()( const int16_t& v     )    { m_out = v; }
         virtual void operator()( const int32_t& v     )    { m_out = v; }
         virtual void operator()( const int64_t& v     )    { m_out = v; }
         virtual void operator()( const uint8_t& v     )    { m_out = v; }
         virtual void operator()( const uint16_t& v    )    { m_out = v; }
         virtual void operator()( const uint32_t& v    )    { m_out = v; }
         virtual void operator()( const uint64_t& v    )    { m_out = v; }
         virtual void operator()( const float& v       )    { m_out = v; }
         virtual void operator()( const double& v      )    { m_out = v; }
         virtual void operator()( const bool& v        )    { m_out = v; }
         virtual void operator()( const fc::string& v )     { m_out = v; }
         virtual void operator()( const value::object& a )  { m_out = a; }
         virtual void operator()( const value::array& a )   { m_out = a; }
         virtual void operator()( )                     { m_out = value(); }
     
         value& m_out;
       };

       template<typename T>
       struct cast_visitor<fc::vector<T>> : value::const_visitor {
         cast_visitor( fc::vector<T>& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )   { FC_THROW_REPORT("bad cast to vector<T> from int8");}
         virtual void operator()( const int16_t& v     )   { FC_THROW_REPORT("bad cast to vector<T> from int16");}
         virtual void operator()( const int32_t& v     )   { FC_THROW_REPORT("bad cast to vector<T> from int32");}
         virtual void operator()( const int64_t& v     )   { FC_THROW_REPORT("bad cast to vector<T> from int64");}
         virtual void operator()( const uint8_t& v     )   { FC_THROW_REPORT("bad cast to vector<T> from uint8");}
         virtual void operator()( const uint16_t& v    )   { FC_THROW_REPORT("bad cast to vector<T> from uint16");}
         virtual void operator()( const uint32_t& v    )   { FC_THROW_REPORT("bad cast to vector<T> from uint32");}
         virtual void operator()( const uint64_t& v    )   { FC_THROW_REPORT("bad cast to vector<T> from uint64");}
         virtual void operator()( const float& v       )   { FC_THROW_REPORT("bad cast to vector<T> from float");}
         virtual void operator()( const double& v      )   { FC_THROW_REPORT("bad cast to vector<T> from double");}
         virtual void operator()( const bool& v        )   { FC_THROW_REPORT("bad cast to vector<T> from bool");}
         virtual void operator()( const fc::string& v )    { FC_THROW_REPORT("bad cast to vector<T> from string");}
         virtual void operator()( const value::object& a ) { FC_THROW_REPORT("bad cast to vector<T> from object");} 
         virtual void operator()( const value::array& a )  {
            m_out.resize(0);
            m_out.reserve( a.fields.size() );
            int idx = 0;
            for( auto i = a.fields.begin(); i != a.fields.end(); ++i ) {
              try {
                m_out.push_back(T());
                unpack( *i, m_out.back()  );
              } catch( fc::error_report& er ) {
                 throw FC_REPORT_PUSH( er, "Error parsing array index ${index} to ${type}", 
                                           fc::value().set("index", idx).set("type",fc::get_typename<T>::name()) );
              }
              ++idx;
            }
         }

         virtual void operator()( )                     { FC_THROW_REPORT("bad cast");}
     
         private:
         fc::vector<T>& m_out;
       };



       template<>
       struct cast_visitor<void> : value::const_visitor {
         virtual void operator()( const int8_t& v      )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int16_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int32_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int64_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint8_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint16_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint32_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint64_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const float& v       )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const double& v      )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const bool& v        )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const fc::string& v )        { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const value::object& a )     { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const value::array&  )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( )                     { }
       };
        template<typename IsTuple=fc::false_type>
        struct cast_if_tuple {
          template<typename T>
          static T cast(  const value& v ) {
             typename fc::deduce<T>::type out;
             v.visit(cast_visitor<decltype(out)>(out));
             return out;
          }
        };
        template<>
        struct cast_if_tuple<fc::true_type> {
          struct member_visitor {
             member_visitor( const value& v )
             :_val(v),idx(0){}
             template<typename Member>
             void operator()( Member& m ) {
               try {
                  //m = value_cast<Member>(_val[idx]);
                  unpack( _val[idx], m );
                //  m = value_cast<Member>(_val[idx]);
               } catch ( fc::error_report& er ) {
                  throw FC_REPORT_PUSH( er, "Error parsing tuple element ${index}", fc::value().set("index",idx) );
               }
               ++idx;
             }
             const value& _val;
             int    idx;
          };

          template<typename Tuple>
          static Tuple cast(  const value& v ) {
             typename fc::deduce<Tuple>::type out;
             out.visit( member_visitor(v) );
             return out;
          }
        };

        template<typename IsReflected=fc::false_type>
        struct cast_if_reflected {
          template<typename T>
          static T cast(  const value& v ) {
            return cast_if_tuple<typename is_tuple<T>::type>::template cast<T>(v);
          }
        };

        template<>
        struct cast_if_reflected<fc::true_type> {
          template<typename T>
          static T cast(  const value& v ) {
              T tmp;
             unpack(v,tmp);
             return tmp;
          }
        };

       class value_visitor;

       struct value_holder {
         virtual ~value_holder();
         virtual const char* type()const;
         virtual void visit( value::const_visitor&& v )const;
         virtual void visit( value_visitor&& v );
       
         virtual void clear();
         virtual size_t size()const;
         virtual void resize( size_t );
         virtual void reserve( size_t );
         virtual value& at( size_t );
         virtual const value& at( size_t )const;
         virtual void push_back( value&& v );
       
         virtual value_holder* move_helper( char* c );
         virtual value_holder* copy_helper( char* c )const;
       };

       void new_value_holder_void( value* v );
    } // namespace detail


    /**
     *  Convert from value v to T
     *
     *  Performs the following conversions
     *  true -> 1.0, 1, "true"
     *
     *  Not all casts are 'clean', the following conversions
     *  could cause errors:
     *
     *  signed int -> unsigned 
     *  large int -> smaller int
     *  real -> int
     *  non-numeric string -> number
     *  object -> string or number
     *  array -> string or number
     *  number,string,array -> object
     */
    template<typename T>
    T value_cast( const value& v ) {
      return detail::cast_if_reflected<typename fc::reflector<typename fc::deduce<T>::type>::is_defined>::template cast< typename fc::deduce<T>::type >(v);
    }

    template<typename T>
    value::value( const T& v ) {
      detail::new_value_holder_void(this);
      fc::pack( *this, v);
    }
    template<typename T>
    value& value::operator=(  const T& v ) {
       this->~value();
       value tmp(v);
       *this = fc::move(tmp);
       return *this;
    }
    template<typename T>
    T value::cast()const {
         T tmp;
         unpack(*this,tmp);
         return tmp;
//        return unpackvalue_cast<T>(*this);
    }
}
