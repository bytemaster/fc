struct s {
  int a;
  int b;
};

class visitor {
  void operator()( const char*, void*, abstract_reflector& r );
};

class abstract_reflector {
    virtual void visit( void* s, visitor  v ) = 0;
};



template<typename S, typename Derived>
class reflector_impl : public abstract_reflector {
  public:
    virtual void visit( void* s, visitor  v ) {
      visit( *((S*)s), v );
    }
    static Derived& instance() {
      static Derived i;
      return i;
    }
};

template<>
class reflector<s> : public reflector_impl< s, reflector<s> > {
    void visit( s&, visitor  v ) {
      v( "a", &s.a, reflector<decltype(s.a)>::instance() )
    }
    const char* name() { return "s"; }
}

class abstract_visitor {
    // fundamental types called directly
    virtual void operator()( double& d ); 
    virtual void operator()( float& d );
    virtual void operator()( int& d );

    // objects call this operator for each named member..
    virtual void operator()( const char* member, int idx, void* d, abstract_reflector& v);

    // called for each item in a collection
    virtual void operator()( int idx, void* d, abstract_reflector& v);
};

class json_visitor : public visitor{
    virtual void operator()( double& d ) {

    }
    virtual void operator()( float& d ) {
      
    }
    virtual void operator()( int& d ) {
    }
    virtual void operator()( void* d, abstract_reflector& v) {
      to_json( d, v );
    }
};

namespace detail {
    string to_json( const void*, abstract_reflector& r ) {
       r.visit( v, to_json_visitor( v ) );
    }
    void from_json( void*, abstract_reflector& r );
}

template<typename T>
string to_json( const T& v) {
  return detail::to_json( &v, reflect(v) );
}

struct param {
  void*      arg;
  reflector* ref;
};

class invoker_impl {
};

class my_interface {
  my_interface( invoker::ptr inv );
  
  virtual int some_func( Arg a ) {
    // this can go in cpp...
    return inv->invoke( "some_func", a );
  }
};

/**
  
*/
class invoker {
    /**
     *  If variadic templates are supported... use them here.
     */
    template<typename T> 
    future<R> invoke( string name ) {
      auto p = new promise<R>(...)
      invoke( p, name, 0 );
      return p;
    }

    template<typename T, typename P1> 
    future<R> invoke( const string& name, P1&& p ) {
      auto p = new promise<R>(...)
      pair<void*,reflector*>    params[1];
      params[0].first = &p;
      params[0].second = reflector<P1>::instance() );
      inv->invoke( p, name, 1, params );
      return p;
    }
    virtual void invoke( promise::ptr p, const string& s, int num_params = 0, param* params = NULL) = 0;

    /// up to max params... 
};

class json_rpc_client : public invoker {

}



