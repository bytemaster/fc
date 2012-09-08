#ifndef _FC_REFLECT_IMPL_HPP_
#define _FC_REFLECT_IMPL_HPP_
#include <fc/reflect.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>

/**
 *  @file reflect_impl.hpp
 *  @brief defines the FC_REFLECT() macro.
 *
 *  This header uses the boost preprocessor library and 
 *
 */

#define FC_REFLECT_FIELD( r, data, i, elem ) \
  v.visit( BOOST_PP_STRINGIZE(elem), i, data, e-> elem );

#define FC_REFLECT( NAME, MEMBERS ) \
namespace fc {  \
  const char* reflector<NAME>::name()const { return BOOST_PP_STRINGIZE(NAME); } \
  void reflector<NAME>::visit( void* s, const abstract_visitor& v )const {  \
    NAME* e = (NAME*)s; \
    BOOST_PP_SEQ_FOR_EACH_I( FC_REFLECT_FIELD, BOOST_PP_SEQ_SIZE(MEMBERS), MEMBERS ) \
  } \
  void reflector<NAME>::visit( const void* s, const abstract_const_visitor& v )const { \
    const NAME* e = (const NAME*)s; \
    BOOST_PP_SEQ_FOR_EACH_I( FC_REFLECT_FIELD, BOOST_PP_SEQ_SIZE(MEMBERS), MEMBERS ) \
  } \
  reflector<NAME>& reflector<NAME>::instance() { static reflector<NAME> inst; return inst; } \
}

#endif // _FC_REFLECT_IMPL_HPP_
