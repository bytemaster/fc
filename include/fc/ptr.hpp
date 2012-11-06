#pragma once 
#include <fc/function.hpp>
#include <fc/future.hpp>

namespace fc {
  
  namespace detail {
    struct identity_member { 
        // TODO: enumerate all method patterns
        template<typename R, typename C, typename P>
        static fc::function<R> functor( P&& p, R (C::*mem_func)() ) {
          return [=](){ return (p->*mem_func)(); };
        }
        
        template<typename R, typename C, typename A1, typename P>
        static fc::function<R,A1> functor( P&& p, R (C::*mem_func)(A1) ) {
          return fc::function<R,A1>([=](A1 a1){ return (p->*mem_func)(a1); });
        }
    };

    template< typename Interface, typename Transform = detail::identity_member >
    struct vtable{};
    
    template<typename ThisPtr>
    struct vtable_visitor {
        template<typename U>
        vtable_visitor( U&& u ):_this( fc::forward<U>(u) ){}
    
        template<typename Function, typename MemberPtr>
        void operator()( const char* name, Function& memb, MemberPtr m )const {
          memb = identity_member::functor( _this, m );
        }
        ThisPtr _this;
    };
  } // namespace detail

  template<typename Interface, typename Transform = detail::identity_member >
  class ptr {
    public:
      typedef detail::vtable<Interface,Transform> vtable_type;

      ptr(){}

      template<typename InterfaceType>
      ptr( InterfaceType* p )
      :_vtable( new vtable_type() ) {
          _vtable->template visit_other<InterfaceType>( detail::vtable_visitor<InterfaceType*>(p) );
      }

      template<typename InterfaceType>
      ptr( const fc::shared_ptr<InterfaceType>& p )
      :_vtable( new vtable_type() ),_self(p){ 
          _vtable->template visit_other<InterfaceType>( detail::vtable_visitor<InterfaceType*>(p.get()) );
      }

      //vtable_type& operator*()            { return *_vtable; }
      vtable_type& operator*()const { return *_vtable; }

      //vtable_type* operator->()            { return _vtable.get(); }
      vtable_type* operator->()const { return _vtable.get(); }

    protected:
      fc::shared_ptr< vtable_type >    _vtable;
      fc::shared_ptr< fc::retainable > _self;
  };


}

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

#define FC_STUB_VTABLE_DEFINE_MEMBER( r, data, elem ) \
      decltype(Transform::functor( (data*)nullptr, &data::elem)) elem; 
#define FC_STUB_VTABLE_DEFINE_VISIT_OTHER( r, data, elem ) \
        v( BOOST_PP_STRINGIZE(elem), elem, &T::elem ); 
#define FC_STUB_VTABLE_DEFINE_VISIT( r, data, elem ) \
        v( BOOST_PP_STRINGIZE(elem), elem ); 

#define FC_STUB( CLASS, METHODS ) \
namespace fc { namespace detail { \
  template<typename Transform> \
  struct vtable<test,Transform> : public fc::retainable { \
     vtable(){} \
    BOOST_PP_SEQ_FOR_EACH( FC_STUB_VTABLE_DEFINE_MEMBER, CLASS, METHODS ) \
      template<typename T, typename Visitor> \
      void visit_other( Visitor&& v ){ \
        BOOST_PP_SEQ_FOR_EACH( FC_STUB_VTABLE_DEFINE_VISIT_OTHER, CLASS, METHODS ) \
      } \
      template<typename Visitor> \
      void visit( Visitor&& v ){ \
        BOOST_PP_SEQ_FOR_EACH( FC_STUB_VTABLE_DEFINE_VISIT, CLASS, METHODS ) \
      } \
  }; \
} } 
//#undef FC_STUB_VTABLE_DEFINE_MEMBER
//#undef FC_STUB_VTABLE_DEFINE_VISIT
