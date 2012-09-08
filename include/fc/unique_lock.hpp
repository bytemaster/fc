#ifndef _FC_UNIQUE_LOCK_HPP_
#define _FC_UNIQUE_LOCK_HPP_

namespace fc {
  struct try_to_lock_t{};
  class time_point;
  
  /**
   *  Including Boost's unique lock drastically increases compile times
   *  for something that is this trivial!
   */
  template<typename T>
  class unique_lock  {
    public:
      unique_lock( T& l, const fc::time_point& abs ):_lock(l) { _locked = _lock.try_lock_until(abs); }
      unique_lock( T& l, try_to_lock_t ):_lock(l) { _locked = _lock.try_lock(); }
      unique_lock( T& l ):_lock(l)                { _lock.lock();   _locked = true;  }
      ~unique_lock()                              { _lock.unlock(); _locked = false; }
      operator bool()const { return _locked; }
    private:
      unique_lock( const unique_lock& );
      unique_lock& operator=( const unique_lock& );
      bool _locked;
      T&  _lock;
  };

}

/**
 *  Emulate java with the one quirk that the open bracket must come before
 *  the synchronized 'keyword'. 
 *  
 *  <code>
    { synchronized( lock_type ) 

    }
 *  </code>
 */
#define synchronized(X)  fc::unique_lock<decltype((X))> __lock(((X)));

#endif
