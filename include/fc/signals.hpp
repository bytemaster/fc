#ifndef _FC_SIGNALS_HPP
#define _FC_SIGNALS_HPP
#include <boost/signal.hpp>
#include <fc/future.hpp>
#include <fc/thread.hpp>

namespace fc {
   template<typename T>
   using signal = boost::signal<T>;

   template<typename T>
   inline T wait( boost::signal<void(T)>& sig, const microseconds& timeout_us=microseconds::max() ) {
     typename promise<T>::ptr p(new promise<T>());
     boost::signals::scoped_connection c = sig.connect( [=]( T t ) { p->set_value(t); } ); 
     return p->wait( timeout_us ); 
   }

   inline void wait( boost::signal<void()>& sig, const microseconds& timeout_us=microseconds::max() ) {
     promise<void>::ptr p(new promise<void>());
     boost::signals::scoped_connection c = sig.connect( [=]() { p->set_value(); } ); 
     p->wait( timeout_us ); 
   }
} 

#endif
