#ifndef _FC_SHARED_PTR_HPP_
#define _FC_SHARED_PTR_HPP_
#include <fc/utility.hpp>

namespace fc {

  /**
   *  @brief used to create reference counted types.
   *
   *  Must be a virtual base class that is initialized with the
   *
   */
  class retainable {
    public:
      retainable();
      void    retain();
      void    release();
      int32_t retain_count()const;

    protected:
      virtual ~retainable(){};

    private:
      volatile int32_t _ref_count;
  };

  template<typename T>
  class shared_ptr {
    public:
      shared_ptr( T* t, bool inc = false )
      :_ptr(t) { if( inc ) t->retain(); }

      shared_ptr():_ptr(0){}
      shared_ptr( const shared_ptr& p ) {
        _ptr = p._ptr;
        if( _ptr ) _ptr->retain();
      }
      shared_ptr( shared_ptr&& p ) {
        _ptr = p._ptr;
        p._ptr = 0;
      }
      ~shared_ptr() {
        if( _ptr ) _ptr->release();
      }
      shared_ptr& reset( T* v = 0 )  {
        if( v == _ptr ) return *this;
        if( _ptr ) _ptr->release();
        _ptr = v;
        if( _ptr ) _ptr->retain();
        return *this;
      }

      shared_ptr& operator=(const shared_ptr& p ) {
        shared_ptr tmp(p);
        fc::swap(tmp._ptr,_ptr);
        return *this;
      }
      shared_ptr& operator=(shared_ptr&& p ) {
        fc::swap(_ptr,p._ptr); 
        return *this;
      }
      T& operator*  ()const  { return *_ptr; }
      T* operator-> ()const  { return _ptr; }

      bool operator==( const shared_ptr& p )const { return get() == p.get(); }
      bool operator<( const shared_ptr& p )const  { return get() < p.get();  }
      T * get() const { return _ptr; }

      bool operator!()const { return _ptr == 0; }
      operator bool()const  { return _ptr != 0; }
    private:
      T* _ptr;
  };
}

#endif
