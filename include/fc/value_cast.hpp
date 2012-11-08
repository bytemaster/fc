#pragma once
#include <fc/reflect.hpp>
#include <fc/value.hpp>
#include <fc/exception.hpp>
#include <fc/lexical_cast.hpp>
#include <fc/numeric_cast.hpp>
#include <fc/value_io.hpp>
#include <fc/tuple.hpp>

#include <typeinfo>

namespace fc {

    namespace detail {

       template<typename T>
       struct cast_visitor : value::const_visitor {
         cast_visitor( T& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int16_t& v     ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int32_t& v     ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int64_t& v     ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint8_t& v     ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint16_t& v    ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint32_t& v    ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint64_t& v    ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const float& v       ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const double& v      ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const bool& v        ){ m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const fc::string& v ) { m_out = fc::lexical_cast<T>(v); }
         virtual void operator()( const value::object&  )      { FC_THROW_MSG("bad cast"); }
         virtual void operator()( const value::array&  )       { FC_THROW_MSG("bad cast"); }
         virtual void operator()( )                     { FC_THROW_MSG("bad cast");        }
         private:
         T& m_out;
       };
     
       template<>
       struct cast_visitor<fc::string> : value::const_visitor {
         cast_visitor( fc::string& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int16_t& v     ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int32_t& v     ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int64_t& v     ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint8_t& v     ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint16_t& v    ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint32_t& v    ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint64_t& v    ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const float& v       ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const double& v      ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const bool& v        ){ m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const fc::string& v ){ m_out = v;                                }
         virtual void operator()( const value::object&  )      { FC_THROW_MSG("bad cast"); }
         virtual void operator()( const value::array&  )       { FC_THROW_MSG("bad cast"); }
         virtual void operator()( )                     { FC_THROW_MSG("bad cast");        }
     
         private:
         fc::string& m_out;
       };
     
       template<>
       struct cast_visitor<value::array> : value::const_visitor {
         cast_visitor( value::array& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int16_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int32_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int64_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint8_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint16_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint32_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint64_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const float& v       ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const double& v      ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const bool& v        ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const fc::string& v ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const value::object&  )      { FC_THROW_MSG("bad cast");}
         virtual void operator()( const value::array& a )      { m_out = a;            }
         virtual void operator()( )                     { FC_THROW_MSG("bad cast");}
     
         private:
         value::array& m_out;
       };
     
       template<>
       struct cast_visitor<value::object> : value::const_visitor {
         cast_visitor( value::object& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int16_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int32_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const int64_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint8_t& v     ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint16_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint32_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint64_t& v    ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const float& v       ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const double& v      ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const bool& v        ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const fc::string& v ){ FC_THROW_MSG("bad cast");}
         virtual void operator()( const value::object& a )     { m_out = a;            }
         virtual void operator()( const value::array&  )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( )                     { FC_THROW_MSG("bad cast");}
     
         private:
         value::object& m_out;
       };
       template<>
       struct cast_visitor<void> : value::value::const_visitor {
         virtual void operator()( const int8_t& v      )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const int16_t& v     )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const int32_t& v     )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const int64_t& v     )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint8_t& v     )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint16_t& v    )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint32_t& v    )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const uint64_t& v    )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const float& v       )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const double& v      )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const bool& v        )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( const fc::string& v )        { FC_THROW_MSG("bad cast");}
         virtual void operator()( const value::object& a )     { FC_THROW_MSG("bad cast");}
         virtual void operator()( const value::array&  )       { FC_THROW_MSG("bad cast");}
         virtual void operator()( )                     { }
       };
        template<typename IsTuple=fc::false_type>
        struct cast_if_tuple {
          template<typename T>
          static T cast(  const value& v ) {
             slog( "cast non tuple %s", typeid(T).name() );
             T out;
             v.visit(cast_visitor<T>(out));
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
               m = value_cast<Member>(_val[idx]);
               ++idx;
             }
             const value& _val;
             int    idx;
          };

          template<typename T>
          static T cast(  const value& v ) {
             T out;
             out.visit( member_visitor(v) );
             slog( "cast tuple" );
            // v.visit(cast_visitor<T>(out));
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
      return detail::cast_if_reflected<typename fc::reflector<T>::is_defined>::template cast<T>(v);
    }

    template<typename T>
    value::value( T&& v ) {
      new (holder) detail::value_holder(); 
      fc::pack( *this, fc::forward<T>(v) );
    }

}
