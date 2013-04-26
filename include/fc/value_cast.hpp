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
#include <vector>

//#include <typeinfo>

namespace fc {

    namespace detail {

        void cast_value( const value& v, int8_t& );
        void cast_value( const value& v, int16_t& );
        void cast_value( const value& v, int32_t& );
        void cast_value( const value& v, int64_t& );
        void cast_value( const value& v, uint8_t& );
        void cast_value( const value& v, uint16_t& );
        void cast_value( const value& v, uint32_t& );
        void cast_value( const value& v, uint64_t& );
        void cast_value( const value& v, double& );
        void cast_value( const value& v, float& );
        void cast_value( const value& v, bool& );
        void cast_value( const value& v, fc::string& );
        void cast_value( const value& v, value& );

        template<typename T>
        void cast_value( const value& v, T& t) {
           unpack(v,t);
        }
        template<typename T>
        void cast_value( const value& v, std::vector<T>& out ) {
           if( v.type() != value::array_type ) {
              FC_THROW_REPORT( "Error casting ${type} to array", fc::value("type", fc::reflector<value::value_type>::to_string(v.type()) ) );
           }
           out.resize(v.size());
           slog( "out .size %d", out.size() );
           const fc::vector<value>& val = v.as_array();
           auto oitr = out.begin();
           int  idx = 0;
           for( auto itr = val.begin(); itr != val.end(); ++itr, ++oitr, ++idx ) {
              try {
                 *oitr = itr->cast<T>(); //value_cast<T>(*itr);
                // value_cast( *itr, *oitr );
              } catch ( fc::error_report& er ) {
                 throw FC_REPORT_PUSH( er, "Error casting value[${index}] to ${type}", 
                                       fc::value("index",idx)
                                                ("type", fc::get_typename<T>::name())
                                     );
              }
           }
        }
       
        template<typename T>
        void cast_value( const value& v, fc::vector<T>& out ) {
           if( v.type() != value::array_type ) {
              FC_THROW_REPORT( "Error casting ${type} to array", fc::value("type", fc::reflector<value::value_type>::to_string(v.type()) ) );
           }
           out.resize(v.size());
           slog( "out .size %d", out.size() );
           const fc::vector<value>& val = v.as_array();
           auto oitr = out.begin();
           int  idx = 0;
           for( auto itr = val.begin(); itr != val.end(); ++itr, ++oitr, ++idx ) {
              try {
                 *oitr = itr->cast<T>(); //value_cast<T>(*itr);
                // value_cast( *itr, *oitr );
              } catch ( fc::error_report& er ) {
                 throw FC_REPORT_PUSH( er, "Error casting value[${index}] to ${type}", 
                                       fc::value("index",idx)
                                                ("type", fc::get_typename<T>::name())
                                     );
              }
           }
        }

        template<typename IsTuple=fc::false_type>
        struct cast_if_tuple {
          template<typename T>
          static T cast(  const value& v ) {
             typename fc::deduce<T>::type out;
             cast_value(v,out);
           //  v.visit(cast_visitor<decltype(out)>(out));
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
                  m = value_cast<Member>(_val[idx]);
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
        return value_cast<T>(*this);
    }
}
