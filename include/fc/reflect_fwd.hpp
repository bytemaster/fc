#ifndef _FC_REFLECT_FWD_HPP_
#define _FC_REFLECT_FWD_HPP_
/**
 *  @file reflect_fwd.hpp
 *  @brief forward declares types defined in reflect.hpp
 *
 *  You should include this file in your headers to accelerate your
 *  compile times over including reflect.hpp
 */

namespace fc { 
  class abstract_reflector;
  template<typename T> class reflector;
  class abstract_visitor;
  class abstract_const_visitor;
  class ref;
  class cref;
}

#define FC_REFLECTABLE( TYPE ) \
namespace fc{  \
  template<> class reflector<TYPE> : virtual public detail::reflector_impl<TYPE, reflector<TYPE> > { \
    public:\
      virtual const char* name()const; \
      virtual void visit( void* s, const abstract_visitor& v )const; \
      virtual void visit( const void* s, const abstract_const_visitor& v )const; \
      static reflector& instance(); \
  };\
}

#endif// _FC_REFLECT_FWD_HPP_
