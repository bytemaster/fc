#include <fc/value_cast.hpp>
#include <fc/exception.hpp>
#include <fc/typename.hpp>
#include <string.h>

namespace fc {

  namespace detail {
      struct value_visitor {
        virtual void operator()( int8_t& v      ){};
        virtual void operator()( int16_t& v     ){};
        virtual void operator()( int32_t& v     ){};
        virtual void operator()( int64_t& v     ){};
        virtual void operator()( uint8_t& v     ){};
        virtual void operator()( uint16_t& v    ){};
        virtual void operator()( uint32_t& v    ){};
        virtual void operator()( uint64_t& v    ){};
        virtual void operator()( float& v       ){};
        virtual void operator()( double& v      ){};
        virtual void operator()( bool& v        ){};
        virtual void operator()( fc::string& v ){};
        virtual void operator()( value::object&  ){};
        virtual void operator()( value::array&  ){};
        virtual void operator()( ){};
      };
      
      
      // fundamental values...
      template<typename T>
      struct value_holder_impl : value_holder {
        static_assert( !fc::is_class<T>::value, "only fundamental types can be stored without specialization" );
        virtual const char* type()const             { return fc::get_typename<T>::name(); }
        virtual void visit( value::const_visitor&& v )const{ v(val); }
        virtual void visit( value_visitor&& v )           { v(val); }
        virtual void clear()                        { val = T(); }
        virtual size_t size()const                  { return 0;  }
      
        virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl( fc::move(val) ); }
        virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }
      
        template<typename V>
        value_holder_impl( V&& v ):val( fc::forward<V>(v) ){}
      
        T val;
      };
      
      template<>
      struct value_holder_impl<void> : value_holder {
        value_holder_impl(){};
       // typedef void_t T;
      /*
        virtual const char* type()const             { return "void"; }
        virtual void visit( value::const_visitor&& v )const{ v(); }
        virtual void visit( value_visitor&& v )           { v(); }
        virtual void clear()                        {  }
        virtual size_t size()const                  { return 0; }
      
        virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl(); }
        virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl();}
        */
      };
      
      
      
      template<>
      struct value_holder_impl<fc::string> : value_holder {
        template<typename V>
        value_holder_impl( V&& v ):val( fc::forward<V>(v) ){}
      
        virtual const char* type()const              { return "string"; }
        virtual void visit( value::const_visitor&& v )const { v(val); }
        virtual void visit( value_visitor&& v )            { v(val); }
      
        virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl( fc::move(val) ); }
        virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }
      
        virtual void clear()                        { val = fc::string(); }
        virtual size_t size()const                  { return 0;  }
      
      
        fc::string val;
      };
      
      template<>
      struct value_holder_impl<value::object> : value_holder {
        virtual const char* type()const              { return "object"; }
        virtual void visit( value::const_visitor&& v )const;
        virtual void visit( value_visitor&& v );
        virtual value_holder* move_helper( char* c );
        virtual value_holder* copy_helper( char* c )const;
        virtual void reserve( size_t s );
      
        virtual void clear();
        virtual size_t size()const;
      
        template<typename V>
        value_holder_impl( V&& v ):val( fc::forward<V>(v) ){}
      
        value::object val; 
      };
      
      template<>
      struct value_holder_impl<value::array> : value_holder {
        virtual const char* type()const              { return "array"; }
        virtual void visit( value::const_visitor&& v )const;
        virtual void visit( value_visitor&& v );
        virtual value_holder* move_helper( char* c );
        virtual value_holder* copy_helper( char* c )const;
      
        virtual void resize( size_t s );
        virtual void reserve( size_t s );
        virtual value& at( size_t i);
        virtual const value& at( size_t i)const;
        virtual void push_back( value&& v );
      
        template<typename V>
        value_holder_impl( V&& v ):val( fc::forward<V>(v) ){}
      
        virtual void clear();
        virtual size_t size()const;
        
        value::array val; 
      };

      value_holder::~value_holder(){}
      const char* value_holder::type()const  { return "void"; }
      value_holder* value_holder::move_helper( char* c )      { return new(c) value_holder(); }
      value_holder* value_holder::copy_helper( char* c )const { return new(c) value_holder(); }

      void value_holder::visit( value::const_visitor&& v )const     { v();           }
      void value_holder::visit( value_visitor&& v )                { v();           }

      void value_holder::clear()                             {}
      size_t value_holder::size()const                       { return 0; }
      void value_holder::resize( size_t )                    { FC_THROW_MSG("value type '%s' not an array", type()); }
      void value_holder::reserve( size_t )                   { FC_THROW_MSG("value type '%s' not an array or object", type()); }
      value& value_holder::at( size_t )                      { FC_THROW_MSG("value type '%s' not an array", type()); return *((value*)0); }
      const value& value_holder::at( size_t )const           { FC_THROW_MSG("value type '%s' not an array", type()); return *((const value*)0); }
      void value_holder::push_back( value&& v )              { FC_THROW_MSG("value type '%s' not an array", type());  }

     // value_holder* value_holder::move_helper( char* c )  = 0;
     // value_holder* value_holder::copy_helper( char* c )const = 0;

      void value_holder_impl<value::array>::resize( size_t s )               { val.fields.resize(s);  }
      void value_holder_impl<value::array>::reserve( size_t s )              { val.fields.reserve(s); }
      value& value_holder_impl<value::array>::at( size_t i)                  { return val.fields[i]; }
      const value& value_holder_impl<value::array>::at( size_t i)const       { return val.fields[i]; }
      value_holder* value_holder_impl<value::array>::move_helper( char* c ){ return new(c) value_holder_impl( fc::move(val) ); }
      value_holder* value_holder_impl<value::array>::copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      void value_holder_impl<value::array>::clear()                        { val.fields.clear();        }
      size_t value_holder_impl<value::array>::size()const                  { return val.fields.size();  }
      void value_holder_impl<value::array>::visit( value::const_visitor&& v )const { v(val); }
      void value_holder_impl<value::array>::visit( value_visitor&& v )            { v(val); }
      void value_holder_impl<value::array>::push_back( value&& v )          { val.fields.push_back( fc::move(v) ); }


      void value_holder_impl<value::object>::visit( value::const_visitor&& v )const { v(val); }
      void value_holder_impl<value::object>::visit( value_visitor&& v )            { v(val); }
      value_holder* value_holder_impl<value::object>::move_helper( char* c ) { return new(c) value_holder_impl( fc::move(val) ); }
      value_holder* value_holder_impl<value::object>::copy_helper( char* c )const { return new(c) value_holder_impl(val);              }
      void value_holder_impl<value::object>::reserve( size_t s )             { val.fields.reserve(s); }

      void value_holder_impl<value::object>::clear()                         { val = value::object(); }
      size_t value_holder_impl<value::object>::size()const                   { return val.fields.size();  }
  } // namespace detail

static detail::value_holder* gh( aligned<24>& h ) {
  return (detail::value_holder*)h._store._data;
}
static const detail::value_holder* gh( const aligned<24>& h ) {
  return (const detail::value_holder*)&h._store._data;
}
  
value::value() {
  new (holder) detail::value_holder_impl<void>(); 
}
value::value( value&& m ) {
  gh(m.holder)->move_helper(holder._store._data);
}
value::value( const value& m ){
  gh(m.holder)->copy_helper(holder._store._data);
}
value::value( char* c ) {
   new (holder) detail::value_holder_impl<fc::string>( c );
}
value::~value() {
  gh(holder)->~value_holder();
}
value::value( int8_t v){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<int8_t> ), "size check" );
  new (holder) detail::value_holder_impl<int8_t>(v);
}
value::value( int16_t v){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<int16_t> ), "size check" );
  new (holder) detail::value_holder_impl<int16_t>(v);
}
value::value( int32_t v){
  new (holder) detail::value_holder_impl<int32_t>(v);
}
value::value( int64_t v){
  new (holder) detail::value_holder_impl<int64_t>(v);
}
value::value( uint8_t v){
  new (holder) detail::value_holder_impl<uint8_t>(v);
}
value::value( uint16_t v){
  new (holder) detail::value_holder_impl<uint16_t>(v);
}
value::value( uint32_t v){
  new (holder) detail::value_holder_impl<uint32_t>(v);
}
value::value( uint64_t v){
  new (holder) detail::value_holder_impl<uint64_t>(v);
}
value::value( double v){
  new (holder) detail::value_holder_impl<double>(v);
}
value::value( float v){
  new (holder) detail::value_holder_impl<float>(v);
}
value::value( bool v){
  new (holder) detail::value_holder_impl<bool>(v);
}
value::value( fc::string&& v){
  new (holder) detail::value_holder_impl<fc::string>(fc::move(v));
}
value::value( fc::string& v){
  new (holder) detail::value_holder_impl<fc::string>(v);
}
value::value( const fc::string& v){
  new (holder) detail::value_holder_impl<fc::string>(v);
}
value::value( value::object&& o ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::object> ), "size check" );
  new (holder) detail::value_holder_impl<value::object>(fc::move(o));
}
value::value( value::array&& a ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::array> ), "size check" );
  new (holder) detail::value_holder_impl<value::array>(fc::move(a));
}
value::value( const value::array& a ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::array> ), "size check" );
  new (holder) detail::value_holder_impl<value::array>(a);
}
value::value( value::array& a ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::array> ), "size check" );
  new (holder) detail::value_holder_impl<value::array>(a);
}
value::value( const value::object& a ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::object> ), "size check" );
  new (holder) detail::value_holder_impl<value::object>(a);
}
value::value( value::object& a ){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::object> ), "size check" );
  new (holder) detail::value_holder_impl<value::object>(a);
}
bool operator == ( const value& v, std::nullptr_t ) {
  return v.is_null();
}
bool operator != ( const value& v, std::nullptr_t ) {
  return v.is_null();
}

value& value::operator=( value&& v ){
  decltype(holder) tmp;
  gh(holder)->move_helper(tmp);
  gh(v.holder)->move_helper(holder);
  gh(tmp)->move_helper(v.holder);
  return *this;
}
value& value::operator=( const value& v ){
  if( this == &v ) return *this;
  gh(holder)->~value_holder();
  gh(v.holder)->copy_helper(holder);
  return *this;
}
bool value::is_null()const {
    return strcmp(gh(holder)->type(), "void") == 0;
}


value::object::const_iterator value::find( const char* key )const {
  if( strcmp(gh(holder)->type(), "object") == 0) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i;
    }
    return o->val.fields.end();
  }
  FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return nullptr; 
}
value::object::const_iterator value::begin()const {
  if( strcmp(gh(holder)->type(), "object") == 0 ) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val.fields.begin();
  }
  FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return nullptr; 
}
value::object::const_iterator value::end()const {
  if( strcmp(gh(holder)->type(), "object" ) == 0 ) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val.fields.end();
  }
  FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return nullptr; 
}
value&       value::operator[]( const char* key ) {
  if( strcmp(gh(holder)->type(), "object") == 0) {
    detail::value_holder_impl<value::object>* o = static_cast<detail::value_holder_impl<value::object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i->val;
    }
    o->val.fields.push_back( key_val(key) );
    return o->val.fields.back().val;
  } else if (strcmp(gh(holder)->type(), "void" ) == 0 ) {
    new (holder) detail::value_holder_impl<value::object>(value::object());
    return (*this)[key];
  }
  FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return *((value*)0);
}
value&       value::operator[]( const fc::string& key )      { return (*this)[key.c_str()]; }
const value& value::operator[]( const fc::string& key )const { return (*this)[key.c_str()]; }
const value& value::operator[]( const char* key )const {
  auto i = find(key);
  if( i == end() ) {
    FC_THROW_MSG( "Key '%s' not found in object", key );
  }
  return i->val;
}

void    value::clear() {
  gh(holder)->clear();  
}
size_t  value::size()const {
  return gh(holder)->size();  
}
void         value::resize( size_t s ) {
  gh(holder)->resize(s);  
}
void         value::reserve( size_t s ) {
  gh(holder)->reserve(s);  
}
void         value::push_back( value&& v ) {
  gh(holder)->push_back(fc::move(v));  
}
value&       value::operator[]( int32_t idx ) {
  return gh(holder)->at(idx);  
}
const value& value::operator[]( int32_t idx )const {
  return gh(holder)->at(idx);  
}

const char* value::type()const { return gh(holder)->type(); }

void  value::visit( value::const_visitor&& v )const {
   auto h = ((detail::value_holder*)&holder[0]);
   h->visit( fc::move(v) );
}
/*  sets the subkey key with v and return *this */
value&  value::set( const char* key,       fc::value v ) {
    (*this)[key] = fc::move(v);
    return *this;
}
value&  value::set( const fc::string& key, fc::value v ) {
    (*this)[key.c_str()] = fc::move(v);
    return *this;
}

} // namepace fc
