#ifndef _FC_REFLECT_VECTOR_HPP_
#define _FC_REFLECT_VECTOR_HPP_
#include <fc/reflect.hpp>
#include <fc/vector.hpp>
namespace fc {
  template<typename T> 
  class reflector<fc::vector<T>> : public detail::reflector_impl<vector<T>, reflector<vector<T>> >{
        public:
    virtual const char* name()const { 
        static fc::string s = fc::string("vector<") + reflector<T>::instance().name() + '>'; 
        return s.c_str();
    }
    virtual void visit( void* s, const abstract_visitor& v )const {
      vector<T>& vec = *((vector<T>*)s);
      size_t si = vec.size();
      for( size_t i = 0; i < si; ++i ) v.visit( i, si, vec.at(i) );
    }
    virtual void visit( const void* s, const abstract_const_visitor& v )const {
       const vector<T>& vec = *((const vector<T>*)s);
       size_t si = vec.size();
       v.array_size(si);
       for( size_t i = 0; i < si; ++i ) v.visit( i, si, vec.at(i) );
    }
    static reflector& instance() { static reflector inst; return inst; }
  };
} // namespace fc 
#endif // _FC_REFLECT_VECTOR_HPP_
