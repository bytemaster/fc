#include <boost/signals2/signal.hpp>
#include <fc/thread/future.hpp>
#include <fc/thread/thread.hpp>


namespace fc {
#if !defined(BOOST_NO_TEMPLATE_ALIASES) 
   template<typename T>
   using signal = boost::signals2::signal<T>;
#else
#endif

   template<typename T>
   inline T wait( boost::signals2::signal<void(T)>& sig, const microseconds& timeout_us=microseconds::maximum() ) {
     typename promise<T>::ptr p(new promise<T>());
     boost::signals2::scoped_connection c( sig.connect( [=]( T t ) { p->set_value(t); } )); 
     return p->wait( timeout_us ); 
   }

   inline void wait( boost::signals2::signal<void()>& sig, const microseconds& timeout_us=microseconds::maximum() ) {
     promise<void>::ptr p(new promise<void>());
     boost::signals2::scoped_connection c( sig.connect( [=]() { p->set_value(); } )); 
     p->wait( timeout_us ); 
   }
} 

