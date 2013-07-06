#pragma once
#include <fc/reflect/reflect.hpp>
#include <fc/io/datastream.hpp>
#include <fc/io/varint.hpp>
#include <fc/optional.hpp>
#include <fc/vector.hpp>
#include <fc/fwd.hpp>
#include <fc/array.hpp>
#include <fc/time.hpp>
#include <fc/io/raw_fwd.hpp>
#include <fc/variant_object.hpp>
#include <fc/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <set>

namespace fc { 
    namespace raw {

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::set<T>& value );
    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::set<T>& value );

    template<typename Stream> 
    inline void pack( Stream& s, const variant_object& v );
    template<typename Stream> 
    inline void unpack( Stream& s, variant_object& v );

    template<typename Stream> 
    inline void pack( Stream& s, const variant& v );
    template<typename Stream> 
    inline void unpack( Stream& s, variant& v );

    template<typename Stream>
    inline void pack( Stream& s, const fc::time_point& tp )
    {
       uint64_t usec = tp.time_since_epoch().count();
       s.write( (const char*)&usec, sizeof(usec) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fc::time_point& tp )
    {
       uint64_t usec;
       s.read( (char*)&usec, sizeof(usec) );
       tp = fc::time_point() + fc::microseconds(usec);
    }

    template<typename Stream, typename T, size_t N> 
    inline void pack( Stream& s, const fc::array<T,N>& v) {
      s.write((const char*)&v.data[0],N*sizeof(T));
    }

    template<typename Stream, typename T, size_t N> 
    inline void unpack( Stream& s, fc::array<T,N>& v) {
      s.read((char*)&v.data[0],N*sizeof(T));
    }

    template<typename Stream> inline void pack( Stream& s, const signed_int& v ) {
      uint32_t val = (v.value<<1) ^ (v.value>>31);
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      } while( val );
    }

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      }while( val );
    }

    template<typename Stream> inline void unpack( Stream& s, signed_int& vi ) {
      uint32_t v = 0; char b = 0; int by = 0;
      do {
        s.get(b);
        v |= uint32_t(uint8_t(b) & 0x7f) << by;
        by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
      vi.value = v&0x01 ? vi.value : -vi.value;
      vi.value = -vi.value;
    }
    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi ) {
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
          s.get(b);
          v |= uint32_t(uint8_t(b) & 0x7f) << by;
          by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = v;
    }

    template<typename Stream> inline void pack( Stream& s, const char* v ) { pack( s, fc::string(v) ); }

    // optional
    template<typename Stream, typename T> 
    inline void pack( Stream& s, const fc::optional<T>& v ) {
      pack( s, bool(!!v) );
      if( !!v ) pack( s, *v );
    }

    template<typename Stream, typename T>
    void unpack( Stream& s, fc::optional<T>& v ) {
      bool b; unpack( s, b ); 
      if( b ) { v = T(); unpack( s, *v ); }
    }

    // std::vector<char>
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value ) { 
      pack( s, unsigned_int(value.size()) );
      if( value.size() )
        s.write( &value.front(), value.size() );
    }
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value ) { 
      unsigned_int size; unpack( s, size );
      value.resize(size.value);
      if( value.size() )
        s.read( value.data(), value.size() );
    }

    // fc::string
    template<typename Stream> inline void pack( Stream& s, const fc::string& v )  {
      pack( s, unsigned_int(v.size()) );     
      if( v.size() ) s.write( v.c_str(), v.size() );
    }

    template<typename Stream> inline void unpack( Stream& s, fc::string& v )  {
      std::vector<char> tmp; unpack(s,tmp);
      if( tmp.size() )
         v = fc::string(tmp.data(),tmp.data()+tmp.size());
      else v = fc::string();
    }

    // bool
    template<typename Stream> inline void pack( Stream& s, const bool& v ) { pack( s, uint8_t(v) );             }
    template<typename Stream> inline void unpack( Stream& s, bool& v )     { uint8_t b; unpack( s, b ); v=b;    }

    namespace detail {
    
      template<typename Stream, typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        void operator()( const char* name )const {
          raw::pack( s, c.*p );
        }
        private:            
          const Class& c;
          Stream&      s;
      };

      template<typename Stream, typename Class>
      struct unpack_object_visitor {
        unpack_object_visitor(Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        inline void operator()( const char* name )const {
          raw::unpack( s, c.*p );
        }
        private:            
          Class&  c;
          Stream& s;
      };

      template<typename IsClass=fc::true_type>
      struct if_class{
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { s << v; }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { s >> v; }
      };

      template<>
      struct if_class<fc::false_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { 
          s.write( (char*)&v, sizeof(v) );   
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          s.read( (char*)&v, sizeof(v) );   
        }
      };

      template<typename IsReflected=fc::false_type>
      struct if_reflected {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { 
          if_class<typename fc::is_class<T>::type>::pack(s,v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          if_class<typename fc::is_class<T>::type>::unpack(s,v);
        }
      };
      template<>
      struct if_reflected<fc::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { 
          fc::reflector<T>::visit( pack_object_visitor<Stream,T>( v, s ) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          fc::reflector<T>::visit( unpack_object_visitor<Stream,T>( v, s ) );
        }
      };

    } // namesapce detail


    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::vector<T>& value ) {
      pack( s, unsigned_int(value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::vector<T>& value ) {
      unsigned_int size; unpack( s, size );
      value.resize(size.value);
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::unpack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::set<T>& value ) {
      pack( s, unsigned_int(value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::set<T>& value ) {
      unsigned_int size; unpack( s, size );
      for( uint64_t i = 0; i < size.value; ++i )
      {
        T tmp;
        unpack( s, tmp );
        value.insert( std::move(tmp) );
      }
    }



    template<typename Stream, typename T> 
    inline void pack( Stream& s, const T& v ) {
      fc::raw::detail::if_reflected< typename fc::reflector<T>::is_defined >::pack(s,v);
    }
    template<typename Stream, typename T> 
    inline void unpack( Stream& s, T& v ) {
      fc::raw::detail::if_reflected< typename fc::reflector<T>::is_defined >::unpack(s,v);
    }

    template<typename Stream>
    class variant_packer : public variant::visitor
    {
       public:
         variant_packer( Stream& _s ):s(_s){}
         virtual void handle()const { }
         virtual void handle( const int64_t& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const uint64_t& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const double& v )const 
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const bool& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const string& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const variant_object& v)const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const variants& v)const
         {
            fc::raw::pack( s, v );
         }
        
         Stream& s;
        
    };


    template<typename Stream> 
    inline void pack( Stream& s, const variant& v )
    {
       pack( s, uint8_t(v.get_type()) );
       v.visit( variant_packer<Stream>(s) );
    }
    template<typename Stream> 
    inline void unpack( Stream& s, variant& v )
    {
      uint8_t t;
      unpack( s, t );
      switch( t )
      {
         case variant::null_type:
            return;
         case variant::int64_type:
         {
            int64_t val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::uint64_type:
         {
            uint64_t val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::double_type:
         {
            double val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::bool_type:
         {
            bool val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::string_type:
         {
            fc::string val;
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         case variant::array_type:
         {
            variants val;
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         case variant::object_type:
         {
            variant_object val; 
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         default:
            FC_THROW_EXCEPTION( parse_error_exception, "Unknown Variant Type ${t}", ("t", t) );
      }
    }

    template<typename Stream> 
    inline void pack( Stream& s, const variant_object& v ) 
    {
       unsigned_int vs = v.size();
       pack( s, vs );
       for( auto itr = v.begin(); itr != v.end(); ++itr )
       {
          pack( s, itr->key() );
          pack( s, itr->value() );
       }
   wlog( "------------ done pack -------------" );
    }
    template<typename Stream> 
    inline void unpack( Stream& s, variant_object& v ) 
    {
       unsigned_int vs;
       unpack( s, vs );

       mutable_variant_object mvo;
       mvo.reserve(vs.value);
       for( auto i = 0; i < vs.value; ++i )
       {
          fc::string key;
          fc::variant value;
          fc::raw::unpack(s,key);
          fc::raw::unpack(s,value);
          mvo.set( fc::move(key), fc::move(value) );
       }
       v = fc::move(mvo);
    }

    template<typename T>
    inline std::vector<char> pack(  const T& v ) {
      datastream<size_t> ps; 
      raw::pack(ps,v );
      std::vector<char> vec(ps.tellp());

      if( vec.size() ) {
        datastream<char*>  ds( vec.data(), size_t(vec.size()) ); 
        raw::pack(ds,v);
      }
      return vec;
    }

    template<typename T>
    inline T unpack( const std::vector<char>& s ) {
      T tmp;
      if( s.size() ) {
        datastream<const char*>  ds( s.data(), size_t(s.size()) ); 
        raw::unpack(ds,tmp);
      }
      return tmp;
    }

    template<typename T>
    inline void pack( char* d, uint32_t s, const T& v ) {
      datastream<char*> ds(d,s); 
      raw::pack(ds,v );
    }

    template<typename T>
    inline T unpack( const char* d, uint32_t s ) {
      T v;
      datastream<const char*>  ds( d, s );
      raw::unpack(ds,v);
      return v;
    }
    template<typename T>
    inline void unpack( const char* d, uint32_t s, T& v ) {
      datastream<const char*>  ds( d, s );
      raw::unpack(ds,v);
      return v;
    }
    
} } // namespace fc::raw

