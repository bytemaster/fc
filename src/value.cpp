#include <fc/value_cast.hpp>
#include <fc/exception.hpp>
#include <fc/typename.hpp>
#include <string.h>
#include <fc/error_report.hpp>


namespace fc {

  namespace detail {
      class value_visitor {
      	public:
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
       template<typename T>
       struct cast_visitor : value::const_visitor {
         cast_visitor( T& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int16_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int32_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const int64_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint8_t& v     )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint16_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint32_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const uint64_t& v    )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const float& v       )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const double& v      )  { m_out = fc::numeric_cast<T>(v); }
         virtual void operator()( const bool& v        )  { m_out = v; }
         virtual void operator()( const fc::string& v )   { m_out = fc::lexical_cast<T>(v); }
         virtual void operator()( const value::object&  ) { FC_THROW_REPORT("bad cast to ${type} from object", 
                                                                            fc::value().set("type",fc::get_typename<T>::name())); }
         virtual void operator()( const value::array&  )  { FC_THROW_REPORT("bad cast to ${type} from array",
                                                                            fc::value().set("type",fc::get_typename<T>::name())); }
         virtual void operator()( )                       { FC_THROW_REPORT("bad cast to ${type} from null",
                                                                            fc::value().set("type",fc::get_typename<T>::name())); } 
         private:
         T& m_out;
       };

       template<>
       struct cast_visitor<bool> : value::const_visitor {
         cast_visitor( bool& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ m_out = v != 0; }
         virtual void operator()( const int16_t& v     ){ m_out = v != 0; }
         virtual void operator()( const int32_t& v     ){ m_out = v != 0; }
         virtual void operator()( const int64_t& v     ){ m_out = v != 0; }
         virtual void operator()( const uint8_t& v     ){ m_out = v != 0; }
         virtual void operator()( const uint16_t& v    ){ m_out = v != 0; }
         virtual void operator()( const uint32_t& v    ){ m_out = v != 0; }
         virtual void operator()( const uint64_t& v    ){ m_out = v != 0; }
         virtual void operator()( const float& v       ){ m_out = v != 0; }
         virtual void operator()( const double& v      ){ m_out = v != 0; }
         virtual void operator()( const bool& v        ){ m_out = v; }
         virtual void operator()( const fc::string& v ) { m_out = !(v != "true"); }
         virtual void operator()( const value::object&  )      { FC_THROW_REPORT("bad cast to bool from object"); }
         virtual void operator()( const value::array&  )       { FC_THROW_REPORT("bad cast to bool from array");  }
         virtual void operator()( )                            { FC_THROW_REPORT("bad cast to bool from null");   }
         private:
         bool& m_out;
       };
     
       template<>
       struct cast_visitor<fc::string> : value::const_visitor {
         cast_visitor( fc::string& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int16_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int32_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const int64_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint8_t& v     ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint16_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint32_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const uint64_t& v    ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const float& v       ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const double& v      ) { m_out = fc::lexical_cast<fc::string>(v); }
         virtual void operator()( const bool& v        ) { m_out = v != 0 ? "true" : "false";       }
         virtual void operator()( const fc::string& v )  { m_out = v;                               }
         virtual void operator()( const value::object&  ){ FC_THROW_REPORT("bad cast to string from object"); }
         virtual void operator()( const value::array&  ) { FC_THROW_REPORT("bad cast to string from array"); }
         virtual void operator()( )                      { m_out = fc::string(); }
     
         private:
         fc::string& m_out;
       };
     
       template<>
       struct cast_visitor<value::array> : value::const_visitor {
         cast_visitor( value::array& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )   { FC_THROW_REPORT("bad cast to array from int8");}
         virtual void operator()( const int16_t& v     )   { FC_THROW_REPORT("bad cast to array from int16");}
         virtual void operator()( const int32_t& v     )   { FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const int64_t& v     )   { FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const uint8_t& v     )   { FC_THROW_REPORT("bad cast to array from uint8");}
         virtual void operator()( const uint16_t& v    )   { FC_THROW_REPORT("bad cast to array from uint16");}
         virtual void operator()( const uint32_t& v    )   { FC_THROW_REPORT("bad cast to array from uint32");}
         virtual void operator()( const uint64_t& v    )   { FC_THROW_REPORT("bad cast to array from uint64");}
         virtual void operator()( const float& v       )   { FC_THROW_REPORT("bad cast to array from float");}
         virtual void operator()( const double& v      )   { FC_THROW_REPORT("bad cast to array from double");}
         virtual void operator()( const bool& v        )   { FC_THROW_REPORT("bad cast to array from bool");}
         virtual void operator()( const fc::string& v )    { FC_THROW_REPORT("bad cast to array from string");}
         virtual void operator()( const value::object&  )  { FC_THROW_REPORT("bad cast to array from object");}
         virtual void operator()( const value::array& a )  { m_out = a;              }
         virtual void operator()( )                        { m_out = value::array(); }
     
         private:
         value::array& m_out;
       };
     
       template<>
       struct cast_visitor<value::object> : value::const_visitor {
         cast_visitor( value::object& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      ){ FC_THROW_REPORT("bad cast to array from int8");}
         virtual void operator()( const int16_t& v     ){ FC_THROW_REPORT("bad cast to array from int16");}
         virtual void operator()( const int32_t& v     ){ FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const int64_t& v     ){ FC_THROW_REPORT("bad cast to array from int32");}
         virtual void operator()( const uint8_t& v     ){ FC_THROW_REPORT("bad cast to array from uint8");}
         virtual void operator()( const uint16_t& v    ){ FC_THROW_REPORT("bad cast to array from uint16");}
         virtual void operator()( const uint32_t& v    ){ FC_THROW_REPORT("bad cast to array from uint32");}
         virtual void operator()( const uint64_t& v    ){ FC_THROW_REPORT("bad cast to array from uint64");}
         virtual void operator()( const float& v       ){ FC_THROW_REPORT("bad cast to array from float");}
         virtual void operator()( const double& v      ){ FC_THROW_REPORT("bad cast to array from double");}
         virtual void operator()( const bool& v        ){ FC_THROW_REPORT("bad cast to array from bool");}
         virtual void operator()( const fc::string& v ) { FC_THROW_REPORT("bad cast to array from string");}
         virtual void operator()( const value::object& a )  { m_out = a;                  }
         virtual void operator()( const value::array&  )    { FC_THROW_REPORT("bad cast");}
         virtual void operator()( )                         { m_out = value::object();      }
     
         private:
         value::object& m_out;
       };
       template<>
       struct cast_visitor<fc::value> : value::const_visitor {
         cast_visitor( value& out )
         :m_out(out){}
         virtual void operator()( const int8_t& v      )    { m_out = v; }
         virtual void operator()( const int16_t& v     )    { m_out = v; }
         virtual void operator()( const int32_t& v     )    { m_out = v; }
         virtual void operator()( const int64_t& v     )    { m_out = v; }
         virtual void operator()( const uint8_t& v     )    { m_out = v; }
         virtual void operator()( const uint16_t& v    )    { m_out = v; }
         virtual void operator()( const uint32_t& v    )    { m_out = v; }
         virtual void operator()( const uint64_t& v    )    { m_out = v; }
         virtual void operator()( const float& v       )    { m_out = v; }
         virtual void operator()( const double& v      )    { m_out = v; }
         virtual void operator()( const bool& v        )    { m_out = v; }
         virtual void operator()( const fc::string& v )     { m_out = v; }
         virtual void operator()( const value::object& a )  { m_out = a; }
         virtual void operator()( const value::array& a )   { m_out = a; }
         virtual void operator()( )                     { m_out = value(); }
     
         value& m_out;
       };

       template<>
       struct cast_visitor<void> : value::const_visitor {
         virtual void operator()( const int8_t& v      )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int16_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int32_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const int64_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint8_t& v     )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint16_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint32_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const uint64_t& v    )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const float& v       )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const double& v      )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const bool& v        )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const fc::string& v )        { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const value::object& a )     { FC_THROW_REPORT("bad cast");}
         virtual void operator()( const value::array&  )       { FC_THROW_REPORT("bad cast");}
         virtual void operator()( )                     { }
       };

       void cast_value( const value& v, int8_t& out ){
          v.visit( cast_visitor<int8_t>(out) );
       }

       void cast_value( const value& v, int16_t& out ){
          v.visit( cast_visitor<int16_t>(out) );
       }

       void cast_value( const value& v, int32_t& out ){
          v.visit( cast_visitor<int32_t>(out) );
       }

       void cast_value( const value& v, int64_t& out ){
          v.visit( cast_visitor<int64_t>(out) );
          slog( "cast_value( v, int64: %lld )", out );
       }

       void cast_value( const value& v, uint8_t& out ){
          v.visit( cast_visitor<uint8_t>(out) );
       }

       void cast_value( const value& v, uint16_t& out ){
          v.visit( cast_visitor<uint16_t>(out) );
       }

       void cast_value( const value& v, uint32_t& out ){
          v.visit( cast_visitor<uint32_t>(out) );
       }

       void cast_value( const value& v, uint64_t& out ){
          v.visit( cast_visitor<uint64_t>(out) );
       }

       void cast_value( const value& v, double& out ){
          v.visit( cast_visitor<double>(out) );
       }

       void cast_value( const value& v, float& out ){
          v.visit( cast_visitor<float>(out) );
       }

       void cast_value( const value& v, bool& out ){
          v.visit( cast_visitor<bool>(out) );
       }

       void cast_value( const value& v, fc::string& out ){
          v.visit( cast_visitor<fc::string>(out) );
       }

       void cast_value( const value& v, value& out ){
          out = v;
       }


       struct value_holder {
         virtual ~value_holder();
         virtual value::value_type type()const;
         const char* get_typename()const { return fc::reflector<value::value_type>::to_string(type()); }
         virtual void visit( value::const_visitor&& v )const;
         virtual void visit( value_visitor&& v );
       
         virtual void clear();
         virtual size_t size()const;
         virtual void resize( size_t );
         virtual void reserve( size_t );
         virtual value& at( size_t );
         virtual const value& at( size_t )const;
         virtual void push_back( value&& v );
       
         virtual value_holder* move_helper( char* c );
         virtual value_holder* copy_helper( char* c )const;
       };
      
      void value_holder::visit( value::const_visitor&& v )const {v(); }
      void value_holder::visit( value_visitor&& v ) 	     {v(); }

      template<typename T>
      struct get_value_type{};
      template<> struct get_value_type<void>   { static value::value_type type(){ return value::null_type; } };
      template<> struct get_value_type<int8_t> { static value::value_type type(){ return value::int8_type; } };
      template<> struct get_value_type<int16_t>{ static value::value_type type(){ return value::int16_type; } };
      template<> struct get_value_type<int32_t>{ static value::value_type type(){ return value::int32_type; } };
      template<> struct get_value_type<int64_t>{ static value::value_type type(){ return value::int64_type; } };
      template<> struct get_value_type<uint8_t>{ static value::value_type type(){ return value::uint8_type; } };
      template<> struct get_value_type<uint16_t>{ static value::value_type type(){ return value::uint16_type; } };
      template<> struct get_value_type<uint32_t>{ static value::value_type type(){ return value::uint32_type; } };
      template<> struct get_value_type<uint64_t>{ static value::value_type type(){ return value::uint64_type; } };
      template<> struct get_value_type<double>{ static value::value_type type(){ return value::double_type; } };
      template<> struct get_value_type<float>{ static value::value_type type(){ return value::float_type; } };
      template<> struct get_value_type<fc::string>{ static value::value_type type(){ return value::string_type; } };
      template<> struct get_value_type<bool>{ static value::value_type type(){ return value::bool_type; } };
      template<> struct get_value_type<fc::vector<value>>{ static value::value_type type(){ return value::array_type; } };
      template<> struct get_value_type<value::object>{ static value::value_type type(){ return value::object_type; } };
      
      // fundamental values...
      template<typename T>
      struct value_holder_impl : value_holder {
        static_assert( !fc::is_class<T>::value, "only fundamental types can be stored without specialization" );

        value_holder_impl(){
           static_assert( sizeof(value_holder_impl) <= 40, "Validate size" );
        }
        virtual value::value_type type()const             { return get_value_type<T>::type(); }
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
        virtual void visit( value::const_visitor&& v )const{ v(); }
        virtual void visit( value_visitor&& v )            { v(); }
        virtual value_holder* move_helper( char* c )     { return new(c) value_holder_impl<void>(); }
        virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl<void>(); }
      };
      
      
      template<>
      struct value_holder_impl<fc::string> : value_holder {
        template<typename V>
        value_holder_impl( V&& v ):val( fc::forward<V>(v) ){
        	static_assert( sizeof(value_holder_impl<fc::string>) <= 40, "Validate size" );
	      }
      
        virtual value::value_type type()const              { return value::string_type; }
        virtual void visit( value::const_visitor&& v )const { v(val); }
        virtual void visit( value_visitor&& v )            { v(val); }
      
        virtual value_holder* move_helper( char* c ){ return new(c) value_holder_impl( fc::move(val) ); }
        virtual value_holder* copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }
      
        virtual void clear()                        { val = fc::string(); }
        virtual size_t size()const                  { FC_THROW_REPORT( "Attempt to access string as array" ); }
      
        fc::string val;
      };
      
      template<>
      struct value_holder_impl<value::object> : value_holder {
        virtual value::value_type type()const              { return value::object_type; }
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
        virtual value::value_type type()const              { return value::array_type; }
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
      static_assert( sizeof( value_holder_impl<value::object> ) <= 40, "sanity check" );
      static_assert( sizeof( value_holder_impl<value::array> ) <= 40, "sanity check" );

      value_holder::~value_holder(){}
      value::value_type value_holder::type()const             { return value::null_type; }
      value_holder* value_holder::move_helper( char* c )      { return new(c) value_holder_impl<void>(); }
      value_holder* value_holder::copy_helper( char* c )const { return new(c) value_holder_impl<void>(); }

      void new_value_holder_void( value* v ) {
      	new (v) value_holder_impl<void>();
      }

      void value_holder::clear()                             {}
      size_t value_holder::size()const                       { return 0; }
      void value_holder::resize( size_t )                    { FC_THROW_MSG("value type '%s' not an array", get_typename()); }
      void value_holder::reserve( size_t )                   { FC_THROW_MSG("value type '%s' not an array or object", get_typename()); }
      value& value_holder::at( size_t )                      { FC_THROW_MSG("value type '%s' not an array", get_typename()); return *((value*)0); }
      const value& value_holder::at( size_t )const           { FC_THROW_MSG("value type '%s' not an array", get_typename()); return *((const value*)0); }
      void value_holder::push_back( value&& v )              { FC_THROW_MSG("value type '%s' not an array", get_typename());  }


      void value_holder_impl<value::array>::resize( size_t s )               { val.resize(s);  }
      void value_holder_impl<value::array>::reserve( size_t s )              { val.reserve(s); }
      value& value_holder_impl<value::array>::at( size_t i)                  { return val[i]; }
      const value& value_holder_impl<value::array>::at( size_t i)const       { return val[i]; }
      value_holder* value_holder_impl<value::array>::move_helper( char* c ){ return new(c) value_holder_impl( fc::move(val) ); }
      value_holder* value_holder_impl<value::array>::copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      void value_holder_impl<value::array>::clear()                        { val.clear();        }
      size_t value_holder_impl<value::array>::size()const                  { return static_cast<size_t>(val.size());  }
      void value_holder_impl<value::array>::visit( value::const_visitor&& v )const { v(val); }
      void value_holder_impl<value::array>::visit( value_visitor&& v )            { v(val); }
      void value_holder_impl<value::array>::push_back( value&& v )          { val.push_back( fc::move(v) ); }


      void value_holder_impl<value::object>::visit( value::const_visitor&& v )const { v(val); }
      void value_holder_impl<value::object>::visit( value_visitor&& v )            { v(val); }
      value_holder* value_holder_impl<value::object>::move_helper( char* c ) { return new(c) value_holder_impl<value::object>( fc::move(val) ); }
      value_holder* value_holder_impl<value::object>::copy_helper( char* c )const { return new(c) value_holder_impl<value::object>(val);              }
      void value_holder_impl<value::object>::reserve( size_t s )             { val.fields.reserve(s); }

      void value_holder_impl<value::object>::clear()                         { val = value::object(); }
      size_t value_holder_impl<value::object>::size()const                   { return val.fields.size();  }
  } // namespace detail

static detail::value_holder* gh( aligned<40>& h ) {
  return (detail::value_holder*)h._store._data;
}
static const detail::value_holder* gh( const aligned<40>& h ) {
  return (const detail::value_holder*)h._store._data;
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
value::value( const char* c ) {
   new (holder) detail::value_holder_impl<fc::string>( c );
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
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<fc::string> ), "size check" );
  new (holder) detail::value_holder_impl<fc::string>(fc::move(v));
}
value::value( fc::string& v){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<fc::string> ), "size check" );
  new (holder) detail::value_holder_impl<fc::string>(v);
}
value::value( const fc::string& v){
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<fc::string> ), "size check" );
  new (holder) detail::value_holder_impl<fc::string>(v);
}
value::value( const fc::string& v, const value& val ) {
  static_assert( sizeof(holder) >= sizeof( detail::value_holder_impl<value::object> ), "size check" );
  new (holder) detail::value_holder_impl<value::object>(value::object());
  set( v, val );
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
    return gh(holder)->type() == null_type;
}
bool value::is_object()const {
    return gh(holder)->type() == object_type;
}
bool value::is_array()const {
    return gh(holder)->type() == array_type;
}
bool value::is_string()const {
    return gh(holder)->type() ==  string_type;
}

const fc::vector<value>& value::as_array()const {
    if( gh(holder)->type() != array_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value array", value("type",gh(holder)->get_typename() ) );
    }
    const detail::value_holder_impl<value::array>* o = static_cast<const detail::value_holder_impl<value::array>*>(gh(holder));
    return o->val;
}
fc::vector<value>& value::as_array(){
    if( gh(holder)->type() != array_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value array", value("type",gh(holder)->get_typename() ) );
    }
    detail::value_holder_impl<value::array>* o = static_cast<detail::value_holder_impl<value::array>*>(gh(holder));
    return o->val;
}
const value::object& value::as_object()const {
    if( gh(holder)->type() != object_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value object", value("type",gh(holder)->get_typename() ) );
    }
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val;
}
value::object& value::as_object(){
    if( gh(holder)->type() != object_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value object", value("type",gh(holder)->get_typename() ) );
    }
    detail::value_holder_impl<value::object>* o = static_cast<detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val;
}
const fc::string& value::as_string()const {
    if( gh(holder)->type() != string_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value string", value("type",gh(holder)->get_typename() ) );
    }
    const detail::value_holder_impl<fc::string>* o = static_cast<const detail::value_holder_impl<fc::string>*>(gh(holder));
    return o->val;
}
fc::string& value::as_string(){
    if( gh(holder)->type() != string_type ) {
       FC_THROW_REPORT( "Attempt to dereference value of type ${type} as value string", value("type",gh(holder)->get_typename() ) );
    }
    detail::value_holder_impl<fc::string>* o = static_cast<detail::value_holder_impl<fc::string>*>(gh(holder));
    return o->val;
}


value::object::const_iterator value::find( const char* key )const {
  if( gh(holder)->type() == object_type ) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i;
    }
    return o->val.fields.end();
  }
  //FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return value::object::const_iterator();
}
value::object::const_iterator value::begin()const {
  if( gh(holder)->type() == object_type ) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val.fields.begin();
  }
 //// FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return value::object::const_iterator();
  //return nullptr; 
}
value::object::const_iterator value::end()const {
  if( gh(holder)->type()== object_type  ) {
    const detail::value_holder_impl<value::object>* o = static_cast<const detail::value_holder_impl<value::object>*>(gh(holder));
    return o->val.fields.end();
  }
  ////FC_THROW_MSG( "Bad cast of %s to object", gh(holder)->type() );
  return value::object::const_iterator();
  //return nullptr; 
}
/**
 *  If this value is an object, remove key from the object
 *
 *  @return *this;
 */
value&       value::clear( const fc::string& key ) {
  if( gh(holder)->type()== object_type ) {
    detail::value_holder_impl<value::object>* o = dynamic_cast<detail::value_holder_impl<value::object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key.c_str() ) == 0 ) {
         o->val.fields.erase(i);
         return *this;
      }
    }
  } 
  return *this;
}
value&       value::operator[]( const char* key ) {
  if( gh(holder)->type()== object_type ) {
    detail::value_holder_impl<value::object>* o = dynamic_cast<detail::value_holder_impl<value::object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i->val;
    }
    o->val.fields.reserve(o->val.fields.size()+1);
    o->val.fields.push_back( key_val(key) );
    return o->val.fields.back().val;
  } else if (gh(holder)->type() == null_type ) {
    new (gh(holder)) detail::value_holder_impl<value::object>(value::object());
    return (*this)[key];
  }
  FC_THROW_REPORT( "Bad cast of ${type} to object", fc::value().set("type", gh(holder)->get_typename()) );
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
value&         value::push_back( value&& v ) {
  if (gh(holder)->type() == null_type  ) {
    new (gh(holder)) detail::value_holder_impl<value::array>(value::array());
    return push_back( fc::move(v) );
  }
  gh(holder)->push_back(fc::move(v));  
  return *this;
}
value&         value::push_back( const value& v ) {
  if (gh(holder)->type() == null_type  ) {
    new (gh(holder)) detail::value_holder_impl<value::array>(value::array());
    return push_back( v );
  }
  gh(holder)->push_back(value(v));
  return *this;
}
value&       value::operator[]( int32_t idx ) {
  return gh(holder)->at(idx);  
}
const value& value::operator[]( int32_t idx )const {
  return gh(holder)->at(idx);  
}

value::value_type value::type()const { return gh(holder)->type(); }

void  value::visit( value::const_visitor&& v )const {
   auto h = ((detail::value_holder*)&holder[0]);
   h->visit( fc::move(v) );
}
/*  sets the subkey key with v and return *this */
value&  value::set( const char* key,       fc::value v ) {
    (*this)[key] = fc::move(v);
    return *this;
}
value&  value::operator()( const char* key,       fc::value v ) {
    (*this)[key] = fc::move(v);
    return *this;
}
value&  value::set( const fc::string& key, fc::value v ) {
    (*this)[key.c_str()] = fc::move(v);
    return *this;
}

} // namepace fc
