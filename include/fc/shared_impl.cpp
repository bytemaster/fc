#pragma once
#include <fc/shared_impl.hpp>

namespace fc {
template<typename T>
typename shared_impl<T>::impl& shared_impl<T>::operator*  ()const { return *_impl; }

template<typename T>
typename shared_impl<T>::impl* shared_impl<T>::operator-> ()const { return _impl.get(); }

template<typename T>
template<typename U>
shared_impl<T>::shared_impl( U&& u ):_impl(fc::forward<U>(u)){}

template<typename T>
shared_impl<T>::shared_impl( const shared_impl<T>& u ):_impl(u._impl){}
template<typename T>
shared_impl<T>::shared_impl( shared_impl<T>& u ):_impl(u._impl){}

template<typename T>
shared_impl<T>::shared_impl( shared_impl<T>&& u ):_impl(fc::move(u._impl)){}

template<typename T>
shared_impl<T>& shared_impl<T>::operator=( shared_impl<T>&& u ) {
  fc_swap(_impl,u._impl);
  return *this;
}
template<typename T>
shared_impl<T>& shared_impl<T>::operator=( const shared_impl<T>& u ) {
  _impl = u._impl;
  return *this;
}

template<typename T>
bool shared_impl<T>::operator !()const { return !_impl; }

template<typename T>
shared_impl<T>::~shared_impl(){}

}

#define FC_REFERENCE_TYPE_IMPL( TYPE ) \
template<typename A1> \
TYPE::TYPE( A1&& a1 ) \
:my( new typename fc::shared_impl<TYPE>::impl( fc::forward<A1>(a1) ) ){}\
template<typename A1,typename A2> \
TYPE::TYPE( A1&& a1, A2&& a2 ) \
:my( new typename fc::shared_impl<TYPE>::impl( fc::forward<A1>(a1), fc::forward<A2>(a2) ) ){}\
template<typename A1,typename A2, typename A3> \
TYPE::TYPE( A1&& a1, A2&& a2, A3&& a3 ) \
:my( new fc::shared_impl<TYPE>::impl( fc::forward<A1>(a1), fc::forward<A2>(a2), fc::forward<A3>(a3) ) ){}\
TYPE::TYPE( shared_impl<TYPE>::impl* m ) \
:my(m){}\
TYPE::TYPE( TYPE* c )\
:my(fc::move(c->my)){ delete c; }\
TYPE::TYPE( TYPE&& c )\
:my(fc::move(c.my)){}\
TYPE::TYPE( const TYPE& c )\
:my(c.my){}\
TYPE::TYPE( TYPE& c )\
:my(c.my){}\
TYPE::TYPE() \
:my( new fc::shared_impl<TYPE>::impl( ) ){}\
TYPE::~TYPE(){}\
bool TYPE::operator !()const { return !my; }\
TYPE& TYPE::operator = ( const TYPE& c ) {\
  my = c.my;\
  return *this;\
}\
TYPE& TYPE::operator = ( TYPE&& c ) {\
  fc_swap( my._impl, c.my._impl );\
  return *this;\
}\
TYPE& TYPE::operator = ( TYPE* c ) {\
  fc_swap( my._impl, c->my._impl );\
  delete c;\
  return *this;\
} \
bool operator==( const TYPE& a, const TYPE& b ) {\
  return a.my._impl.get() == b.my._impl.get(); \
} \
bool operator!=( const TYPE& a, const TYPE& b ) {\
  return a.my._impl.get() != b.my._impl.get(); \
}


