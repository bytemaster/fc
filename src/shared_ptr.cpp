#include <fc/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/memory_order.hpp>

namespace fc {
  retainable::retainable()
  :_ref_count(1) { }

  void retainable::retain() {
    ((boost::atomic<int32_t>*)&_ref_count)->fetch_add(1, boost::memory_order_relaxed );
  }

  void retainable::release() {
    if( 1 == ((boost::atomic<int32_t>*)&_ref_count)->fetch_sub(1, boost::memory_order_release ) ) {
        boost::atomic_thread_fence(boost::memory_order_acquire);
        delete this;
    }
  }

  int32_t retainable::retain_count()const {
    return _ref_count;
  }
}
