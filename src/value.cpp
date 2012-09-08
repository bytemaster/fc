#include <fc/fwd_reflect.hpp>
#include <fc/value.hpp>
#include <fc/reflect.hpp>
#include <fc/string.hpp>
#include <fc/reflect_vector.hpp>
#include <fc/reflect_value.hpp>
#include <fc/reflect_impl.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/error.hpp>
#include <fc/exception.hpp>

FC_REFLECT( fc::value::member, (_val) )


namespace fc {
  value::member::member(){}
  value::member::member( const char* c )
  :_key(c){ }
  value::member::member( string&& c )
  :_key(fc::move(c)){ }

  const string& value::member::key()const { return *_key; }
  value&        value::member::val()      { return *_val; }
  const value&  value::member::val()const { return *_val; }

  value::value()
  :_obj(nullptr),_obj_type(nullptr){ slog( "%p", this ); }

  value::value( value&& v ) 
  :_obj(v._obj),_obj_type(v._obj_type)
  {
    slog( "move construct value" );
    v._obj_type = nullptr;
    v._obj = nullptr;
  }

  value::~value() {
    slog( "~%p", this );
    if( nullptr != _obj_type ) {
      if( _obj != nullptr ) {
        slog( "obj_type %p", _obj_type );
        slog( "obj_type %s", _obj_type->name() );
        slog(".. obj type %p  %s", _obj, _obj_type->name() );
        size_t s = _obj_type->size_of();
        if( s > sizeof(_obj) ) {
          slog( "destroy! %p", _obj );
          _obj_type->destroy( _obj );
        } else {
          slog( "destructor! %p", &_obj );
          _obj_type->destructor( &_obj );
        }
      }
    }
  }

  value::value( const cref& v ) {
    slog( "this:%p %s from cref" , this, v._reflector.name());
    _obj_type = &v._reflector;
    size_t s = _obj_type->size_of();
    if( s > sizeof(_obj) ) {
        slog( "construct %s heap of size %d",_obj_type->name(),_obj_type->size_of() );
        _obj      = new char[_obj_type->size_of()];
        slog( "v._obj %p", v._obj );
        _obj_type->copy_construct( _obj, v._obj );
    } else {
        slog( "construct %s in place %p type p %p", _obj_type->name(), _obj,_obj_type );
        _obj_type->copy_construct( &_obj, v._obj );
    }
  }

  value::value( const value& v ) {
    slog( "%p", this );
   // slog( "copy v %s", v.type()->name() );
    _obj_type = v._obj_type;
    if( nullptr != _obj_type ) {
        size_t s = _obj_type->size_of();
        if( s > sizeof(_obj) ) {
            _obj      = new char[_obj_type->size_of()];
            _obj_type->copy_construct( _obj, v._obj );
        } else {
            _obj_type->copy_construct( &_obj, &v._obj );
        }
    }
  }
  value& value::operator=( value&&      v ) {
    swap( v._obj, _obj);
    swap( v._obj_type, _obj_type);
    return *this;
    if( v.type() == nullptr ) {
      return *this;
    }
    slog( "move assign v %s", v.type()->name() );
    size_t s = _obj_type->size_of();
    if( s > sizeof(_obj) ) {
        slog( "swap pointers to heap.." );
        fc::swap( _obj, v._obj );
        fc::swap( _obj_type, v._obj_type );
    } else {
       slog( "move construct in place %p  %s", this, v._obj_type->name() );
       int64_t tmp;
       if( nullptr != _obj_type && nullptr != v._obj_type ) {
        slog( "swaping objs %s and %s", _obj_type->name(), v._obj_type->name() );
        slog( "swaping objs %p and %p", &_obj, &v._obj );
        slog( "&tmp = %p", &tmp );
         _obj_type->move_construct( &tmp, &_obj );
         slog( "move to tmp" );
         v._obj_type->move_construct( &_obj, &v._obj );
         slog( "move to dest" );
         _obj_type->move_construct( &v._obj, &tmp );
         slog( "move to src" );
       } else {
         fc::swap( _obj, v._obj );
         fc::swap( _obj_type, v._obj_type );
       }
    }
    
    /*
    value tmp(std::move(v));
    return *this;
    size_t s = _obj_type->size_of();
    if( s > sizeof(_obj) ) {
        slog( "" ); 
        fc::swap( _obj, v._obj );
    } else {
        slog( "swap..." ); 
      void* tmp;
      _obj_type->move( &tmp, &_obj );
      _obj_type->move( &_obj, &v._obj );
      _obj_type->move( &v._obj, &tmp );
    }
    fc::swap( _obj_type, v._obj_type );
    */
    return *this;
  }
  value& value::operator=( const value& v ) {
    slog( "assign copy" );
    value t(v); fc::swap(t,*this);
    return *this;
  }
  value& value::operator=( const cref&  v ) {
    //slog( "assign copy this %p %p %s  obj_type %s",_obj,_obj_type, v._reflector.name(),_obj_type->name() );
    //if( _obj_type != null_ptr ) {
    //}
    wlog( ".." );
    value t(v); 

    wlog( "swap" );
    //swap( t._obj, _obj );
    //swap( t._obj_type, _obj_type );
    fc::swap(t,*this);
    slog( "done swap" );
    return *this; 
  }
  /**
   *  @pre value is null or an object
   */
  value& value::operator[]( const string& key ) {
    return (*this)[key.c_str()];
  }

  value& value::operator[]( string&& key ) {
      if( is_null() ) {
          *this = vector<member>(1);
      }
     if( _obj_type == &reflector< vector<member> >::instance() ) {
        vector<member>& vec = *static_cast<vector<member>*>(ptr());
        for( uint32_t i = 0; i < vec.size(); ++i ) {
          if( vec[i].key() == key ) { return vec[i].val(); }
        }
        vec.push_back(member(fc::move(key)));
        return vec.back().val();
     }
     FC_THROW( bad_cast() );
  }


  const value&  value::operator[]( const string& key )const {
    return (*this)[key.c_str()];
  }

  value& value::operator[]( const char* key ) {
    if( is_null() ) { 
      *this = vector<member>();
    } 
    if( _obj_type == &reflector<vector<member> >::instance() ) {
        slog( "sizeof vector<member*>: %d", sizeof( vector<member*> ) );
       vector<member>& v = *static_cast<vector<member>*>((void*)&_obj);
       vector<member>::iterator i = v.begin();
       while( i != v.end() ) {
        // todo convert to string cmp to prevent temporary string??
         if( i->key() == key ) {
           return i->val();
         }
         ++i;
       }
       v.push_back( member( key ) );
       return v.back().val();
    }
    // visit the native struct looking for key and return a ref to the value
    //
    // if not found, then convert native struct into vector<member> and recurse
  }
  const value& value::operator[]( const char* key )const {
    if( is_null() ) { 
      // TODO: throw!
    } 
    if( _obj_type == &reflector<vector<member> >::instance() ) {
       const vector<member>& v = *static_cast<const vector<member>*>((void*)&_obj);
       vector<member>::const_iterator i = v.begin();
       while( i != v.end() ) {
         if( i->key() == key ) {
           return i->val();
         }
         ++i;
       }
       FC_THROW( range_error() );
    }
    FC_THROW( bad_cast() );
  }

  value& value::operator[]( int index ) {
    return (*this)[uint64_t(index)];
  }
  value& value::operator[]( uint64_t index ) {
    if( is_null() ) { 
      slog( "init from vector<value> of size %d", index+1 );
      //static_assert( sizeof(_obj) >= sizeof(vector<value>), "sanity check" );
      *this = vector<value>(index+1);
      //new (&_obj) vector<value>(index+1);
      //_obj_type = &reflector<vector<value> >::instance();
    } 
    if( _obj_type == &reflector<vector<value> >::instance() ) {
       slog( "return ref to index..." );
       vector<value>& v = *static_cast<vector<value>*>(ptr());
       if( v.size() <= index ) { v.resize(index+1); }
       slog( "index %d vs size %d", index, v.size() );
       return v.at(index);
    }
    // visit the native struct looking for index... 
    //
    //
  }
  const value& value::operator[]( uint64_t index )const {
    if( is_null() ) { 
      // THROW 
      while(1) ;
    } 
    if( _obj_type == &reflector<vector<value> >::instance() ) {
       const vector<value>& v = *static_cast<const vector<value>*>(ptr());
       return v[index];
    }
    // visit the native struct looking for index... throw if not found.
  }

  bool value::key_exists( const string& key ) {
    return key_exists(key.c_str());
  }
  bool value::key_exists( const char* key ) {
    return false;
  }
  bool value::is_array()const {
    return  _obj_type == &reflector<vector<value> >::instance();
  }
  bool value::is_object()const {
    return  _obj_type == &reflector<vector<member> >::instance();
  }
  bool value::is_null()const {
    return _obj_type == nullptr;
  }
  bool value::is_string()const {
    return _obj_type == &reflector<string>::instance();
  }
  bool value::is_float()const {
    return _obj_type == &reflector<float>::instance();
  }
  bool value::is_double()const {
    return _obj_type == &reflector<double>::instance();
  }
  bool value::is_real()const {
    return is_float() || is_double();
  }
  bool value::is_integer()const {
    return false;
  }
  bool value::is_boolean()const {
    return _obj_type == &reflector<bool>::instance();
  }
  fwd<vector<string>,8> value::get_keys()const {
    fwd<vector<string>,8> s;
    return s;
  }

  value& value::push_back( const value& v ) {
    slog("here I go again... type %p %s", _obj_type, _obj_type ? _obj_type->name(): "null" );
    if( is_null() ) { 
      wlog( "converting this to vector..." );
      *this = vector<value>();
    } 
    if( _obj_type == &reflector<vector<value> >::instance() ) {
       vector<value>& vec = *static_cast<vector<value>*>(ptr());
       vec.push_back(v);
    } else {
      FC_THROW( bad_cast() );
    }
    return *this;
  }
  value& value::push_back( value&& v ) {
    slog("here I go again... type %p %s", _obj_type, _obj_type ? _obj_type->name(): "null" );
    if( is_null() ) { 
      *this = vector<value>();
    } 
    if( _obj_type == &reflector<vector<value> >::instance() ) {
       vector<value>& vec = *static_cast<vector<value>*>(ptr());
       vec.push_back(fc::move(v));
    } else {
       FC_THROW( bad_cast() );
    }
    return *this;
  }

  void*                        value::ptr(){  
    if( nullptr != _obj_type ) {
      if( _obj_type->size_of() > sizeof(_obj) )
        return _obj;
      return &_obj;
    }
    return nullptr;
  }
  const void*                  value::ptr()const {  
    if( _obj_type ) {
      if( _obj_type->size_of() > sizeof(_obj) )
        return _obj;
      return &_obj;
    }
    return nullptr;
  }

  abstract_reflector* value::type()const     { return _obj_type; }
} // namespace fc


namespace fc {
   const char* reflector<value>::name()const {  return "value"; }
   void reflector<value>::visit( void* s, const abstract_visitor& v )const {
   }
   void reflector<value>::visit( const void* s, const abstract_const_visitor& v )const {
     const value& val = *((const value*)s);
     if( val.is_null() ) { v.visit(); }
     else if( val.is_array() ) {
       const vector<value>& vec = *static_cast<const vector<value>*>(val.ptr());
       auto s = vec.size();
       auto e = vec.end();
       int idx = 0;
       for( auto i = vec.begin(); i != e; ++i ) {
         v.visit( idx, s, *i ); 
         ++idx;
       }
     } else if( val.is_object() ) {
       const vector<value::member>& vec = *static_cast<const vector<value::member>*>(val.ptr());
       auto s = vec.size();
       auto e = vec.end();
       int idx = 0;
       for( auto i = vec.begin(); i != e; ++i ) {
         v.visit( i->key().c_str(), idx, s, i->val() ); 
         ++idx;
       }
        
     } else {
       slog( "val type %s", val.type()->name() );
       val.type()->visit(val.ptr(), v );
     }
   }
   reflector<value>& reflector<value>::instance() { static reflector<value> inst; return inst; }
} // namespace fc 

