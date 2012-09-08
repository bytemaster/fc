#ifndef _FC_REFLECT_PTR_HPP_
#define _FC_REFLECT_PTR_HPP_
#include <fc/reflect.hpp>

namespace fc { 

  struct ptr {
      ptr():_obj(0),_reflector(0){}

      template<typename T>
      ptr( T* v )
      :_obj(v),_reflector(&reflect(*v)){}

      ptr( const ptr& v )
      :_obj(v._obj),_reflector(v._reflector){}

      ref operator*()const { return ref( _obj, *_reflector); }

      private:
        friend struct cptr;
        void*               _obj;
        abstract_reflector* _reflector;
  };

  // provides pointer semantics
  struct cptr {
      cptr():_obj(0),_reflector(0){}
      template<typename T>

      cptr( const T* v )
      :_obj(v),_reflector(&reflect(*v)){}

      cptr( const cptr& v )
      :_obj(v._obj),_reflector(v._reflector){}

      cptr( const ptr& v )
      :_obj(v._obj),_reflector(v._reflector){}

      cref operator*()const { return cref( _obj, *_reflector); }
      
      private:
        const void*         _obj;
        abstract_reflector* _reflector;
  };

}

#endif // _FC_REFLECT_PTR_HPP
