#ifndef _FC_ABSTRACT_TYPES_HPP_
#define _FC_ABSTRACT_TYPES_HPP_
#include <fc/utility.hpp>
#include <fc/log.hpp>

namespace fc {

  struct abstract_type {
    virtual ~abstract_type(){}
    virtual size_t size_of()const = 0;
    /*
     *  @brief Inplace destructor (does not free memory) ((T*)dst)->~T();
     */
    virtual void   destructor( void* dst )const = 0;

    /** @brief 'delete T' */
    virtual void   destroy( void* dst )const = 0;
  };

  template<typename T>
  struct type : virtual abstract_type {
    virtual  size_t  size_of()const                { return sizeof(T);  }
    virtual  void    destructor( void* dst )const  { ((T*)dst)->~T();   }
    virtual  void    destroy( void* dst )const     { delete ((T*)dst);  }
  };

  struct abstract_moveable_type : virtual abstract_type {
    virtual  void      move_construct( void* dst, void* src )const       = 0;
    virtual  void      move( void* dst, void* src )const                 = 0;
  };

  template<typename T>
  struct moveable_type : virtual type<T>, virtual abstract_moveable_type {
    static abstract_moveable_type& instance() { static moveable_type<T> inst; return inst; }
    virtual  void        destruct( void* dst )const                        { ((T*)dst)->~T(); }
    virtual  void        move_construct( void* dst, void* src )const       { slog( "move construct" ); new ((char*)dst) T( fc::move(*((T*)src)) ); }
    virtual  void        move( void* dst, void* src )const                 { *((T*)dst) = fc::move(*((T*)src));  }
  };

  struct abstract_value_type : virtual abstract_moveable_type {
    virtual  void      construct( void* dst )const                       = 0;
    virtual  void      copy_construct( void* dst, const void* src )const = 0;
    virtual  void      assign( void* dst, const void* src )const         = 0;
  };

  /**
   *  Default constructable, moveable, copyable, assignable. 
   */
  template<typename T>
  struct value_type : virtual moveable_type<T>, virtual abstract_value_type {
    static abstract_value_type& instance() { static value_type<T> inst; return inst; }

    virtual  void      construct( void* dst )const                       { new ((char*)dst) T();                       }
    virtual  void      copy_construct( void* dst, const void* src )const { new ((char*)dst) T( *((const T*)src) );     }
    virtual  void      assign( void* dst, const void* src )const         { *((T*)dst) = *((const T*)src);      } 
  };

  struct abstract_less_than_comparable_type {
    virtual bool less_than( const void* left, const void* right )const = 0;
  };


  template<typename T>
  struct less_than_comparable_type : abstract_less_than_comparable_type {
    virtual bool less_than( const void* left, const void* right )const {
      return  *((const T*)left) < *((const T*)right);
    }
  };

  struct abstract_equal_comparable_type {
    virtual bool equal( const void* left, const void* right )const = 0;
  };

  template<typename T>
  struct equal_comparable_type : abstract_equal_comparable_type {
    virtual bool equal( const void* left, const void* right )const {
      return  *((const T*)left) == *((const T*)right);
    }
  };

  struct abstract_callable_type {
    virtual void call( const void* self )const = 0;
  };

  template<typename T>
  struct callable_type : virtual abstract_callable_type, virtual value_type<T>  {
    virtual void call( const void* self )const { (*((const T*)self))(); }
  };

} // namespace fc

#endif
