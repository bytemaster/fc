#ifndef _TORNET_RPC_RAW_HPP_
#define _TORNET_RPC_RAW_HPP_
#include <fc/static_reflect.hpp>
#include <fc/datastream.hpp>
#include <fc/varint.hpp>
#include <fc/optional.hpp>
#include <fc/vector.hpp>
#include <fc/fwd.hpp>
#include <fc/array.hpp>
//#include <fc/value.hpp>

namespace fc { 
    class value; 
    namespace raw {

    template<typename Stream, typename T>
    void unpack( Stream& s, fc::optional<T>& v );
    template<typename Stream, typename T>
    void pack( Stream& s, const fc::optional<T>& v );

    template<typename Stream>
    void unpack( Stream& s, fc::value& );
    template<typename Stream>
    void pack( Stream& s, const fc::value& );

    template<typename Stream>
    void unpack( Stream& s, fc::string& );

    template<typename Stream>
    void pack( Stream& s, const fc::string& );

    template<typename Stream, typename T>
    inline void pack( Stream& s, const T& v );

    template<typename Stream, typename T>
    inline void unpack( Stream& s, T& v );

    template<typename Stream, typename T>
    inline void pack( Stream& s, const fc::vector<T>& v );
    template<typename Stream, typename T>
    inline void unpack( Stream& s, fc::vector<T>& v );


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
        s.put(b);
      } while( val );
    }

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.put(b);
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

    // fc::vector
    template<typename Stream> inline void pack( Stream& s, const fc::vector<char>& value ) { 
      pack( s, unsigned_int(value.size()) );
      if( value.size() )
        s.write( &value.front(), value.size() );
    }
    template<typename Stream> inline void unpack( Stream& s, fc::vector<char>& value ) { 
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
      fc::vector<char> tmp; unpack(s,tmp);
      v = fc::string(tmp.begin(),tmp.end());
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

      template<typename Stream>
      struct pack_sequence {
         pack_sequence( Stream& _s ):s(_s){}
         template<typename T>
         void operator() ( const T& v )const { raw::pack(s,v); }
         Stream&    s;
      };

      template<typename Stream>
      struct unpack_sequence {
         unpack_sequence( Stream& _s ):s(_s){}
         template<typename T>
         void operator() ( T& v )const { raw::unpack(s,v); }
         Stream&  s;
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
          fc::static_reflector<T>::visit( pack_object_visitor<Stream,T>( v, s ) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          fc::static_reflector<T>::visit( unpack_object_visitor<Stream,T>( v, s ) );
        }
      };

    } // namesapce detail


    template<typename Stream, typename T>
    inline void pack( Stream& s, const fc::vector<T>& value ) {
      pack( s, unsigned_int(value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, fc::vector<T>& value ) {
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
    inline void pack( Stream& s, const T& v ) {
      fc::raw::detail::if_reflected< typename fc::static_reflector<T>::is_defined >::pack(s,v);
    }
    template<typename Stream, typename T> 
    inline void unpack( Stream& s, T& v ) {
      fc::raw::detail::if_reflected< typename fc::static_reflector<T>::is_defined >::unpack(s,v);
    }


    template<typename T>
    inline fc::vector<char> pack(  const T& v ) {
      datastream<size_t> ps; 
      raw::pack(ps,v );
      fc::vector<char> vec(ps.tellp());
      if( vec.size() ) {
        datastream<char*>  ds( vec.data(), vec.size() ); 
        raw::pack(ds,v);
      }
      return vec;
    }

    template<typename T>
    inline T unpack( const fc::vector<char>& s ) {
      T tmp;
      if( s.size() ) {
        datastream<const char*>  ds( s.data(), s.size() ); 
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
    
} } // namespace fc::raw

#endif // BOOST_RPC_RAW_HPP
