#pragma once
#include <fc/utility.hpp>

namespace fc {

  /**
   *  Provides a fast-compiling tuple that doesn't use fancy meta-programming
   *  techniques.  It is limited to 4 parameters which is sufficient for most 
   *  methods argument lists which is the primary use case for this tuple. Methods
   *  that require more than 4 parameters are probably better served by defining
   *  a struct.
   *
   *  The members of the tuple are easily visited with a simple visitor functor 
   *  of the form:
   *  @code
   *  struct visitor {
   *    template<typename MemberType>
   *    void operator()( MemberType& m );
   *
   *    template<typename MemberType>
   *    void operator()( const MemberType& m );
   *  };
   *  @endcode
   */
  template<typename A=void, typename B=void,typename C=void, typename D=void>
  struct tuple { 
    tuple(){}
    enum size_enum { size = 4 };

    template<typename AA, typename BB, typename CC, typename DD>
    tuple( AA&& aa, BB&& bb, CC&& cc, DD&& dd )
    :a( fc::forward<AA>(aa) ),
     b( fc::forward<BB>(bb) ),
     c( fc::forward<CC>(cc) ),
     d( fc::forward<DD>(dd) )
     {}

    template<typename V>
    void visit( V&& v ) { v(a); v(b); v(c); v(d); }
    template<typename V>
    void visit( V&& v )const { v(a); v(b); v(c); v(d); }

    A a; 
    B b; 
    C c; 
    D d; 
  }; 

  template<>
  struct tuple<> {
    enum size_enum { size = 0 };
    template<typename V> 
    void visit( V&& v)const{};
  };

  template<typename A>
  struct tuple<A,void> { 
    enum size_enum { size = 1 };
    template<typename AA>
    tuple( AA&& aa ):a( fc::forward<AA>(aa) ){}
    tuple( const tuple& t ):a(t.a){}
    tuple( tuple&& t ):a(t.a){}
    tuple(){}

    template<typename V>
    void visit( V&& v ) { v(a); }
    template<typename V>
    void visit( V&& v )const { v(a); }

    A a; 
  };

  template<typename A, typename B>
  struct tuple<A,B> { 
    enum size_enum { size = 2 };

    template<typename AA, typename BB>
    tuple( AA&& aa, BB&& bb )
    :a( fc::forward<AA>(aa) ),
     b( fc::forward<BB>(bb) ){}

    tuple(){}

    template<typename V>
    void visit( V&& v ) { v(a); v(b); }
    template<typename V>
    void visit( V&& v )const { v(a); v(b); }

    A a; 
    B b; 
  };

  template<typename A, typename B, typename C>
  struct tuple<A, B, C > {
    enum size_enum { size = 3 };
    tuple(){}

    template<typename AA, typename BB, typename CC>
    tuple( AA&& aa, BB&& bb, CC&& cc )
    :a( fc::forward<AA>(aa) ),
     b( fc::forward<BB>(bb) ),
     c( fc::forward<CC>(cc) )
     {}

    template<typename V>
    void visit( V&& v ) { v(a); v(b); v(c); }
    template<typename V>
    void visit( V&& v )const { v(a); v(b); v(c); }

    A a; 
    B b; 
    C c; 
  };

  tuple<> make_tuple();

  template<typename A>
  tuple<A> make_tuple(A&& a){ return tuple<A>( fc::forward<A>(a) ); }

  template<typename A,typename B>
  tuple<A,B> make_tuple(A&& a, B&& b){ return tuple<A,B>( fc::forward<A>(a), fc::forward<B>(b) ); }

  template<typename A,typename B, typename C>
  tuple<A,B,C> make_tuple(A&& a, B&& b, C&& c){ return tuple<A,B,C>( fc::forward<A>(a), fc::forward<B>(b), fc::forward<C>(c) ); }

  template<typename A,typename B, typename C,typename D>
  tuple<A,B,C> make_tuple(A&& a, B&& b, C&& c, D&& d){ 
    return tuple<A,B,C,D>( fc::forward<A>(a), fc::forward<B>(b), fc::forward<C>(c), fc::forward<D>(d) ); 
  }
}
