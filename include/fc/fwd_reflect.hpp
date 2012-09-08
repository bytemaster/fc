#ifndef _FC_FWD_REFLECT_HPP_
#define _FC_FWD_REFLECT_HPP_
#include <fc/reflect.hpp>
#include <fc/fwd.hpp>

namespace fc {
  template<typename T,unsigned int S>
  class reflector<fwd<T,S>> : public detail::reflector_impl<fwd<T,S>, reflector<fwd<T,S>> >{
    public:
      virtual const char* name()const { return instance().name(); }
      virtual void visit( void* s, const abstract_visitor& v )const {
         instance().visit(s,v);
      }
      virtual void visit( const void* s, const abstract_const_visitor& v )const {
         instance().visit(s,v);
      }

      static reflector<T>& instance() { return reflector<T>::instance(); }
  };
} // namespace fc
#endif //_FC_FWD_REFLECT_HPP_
