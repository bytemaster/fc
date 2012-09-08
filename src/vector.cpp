#include <fc/vector.hpp>

namespace fc {
    struct vector_impl_d {
      abstract_value_type& _vtbl;
      char*                _data;
      size_t             _size;
      size_t             _capacity;

      vector_impl_d( abstract_value_type& r, size_t size, size_t cap= 0 )
      :_vtbl(r),_data(0),_size(size),_capacity(cap) {
        if( _size > _capacity ) _capacity = _size;
        if( _capacity ) {
          const unsigned int so = _vtbl.size_of();
          _data = new char[_capacity*so];
          char* end = _data + _size *so;
          for( char* idx = _data; idx < end; idx += so ) {
            _vtbl.construct( idx );  
          }
        }
      }

      vector_impl_d( const vector_impl_d& cpy )
      :_vtbl(cpy._vtbl),_data(0),_size(cpy._size),_capacity(cpy._size) {
          if( _size ) {
            _data = new char[_size*_vtbl.size_of()];
            copy_from( cpy._data, cpy._size );
          }
      }

      void copy_from( char* src, int cnt ) {
         const unsigned int so = _vtbl.size_of();
         char* end = _data + cnt * so;
         char* cpy_idx = src;
         for( char* idx = _data; idx < end; idx += so ) {
           _vtbl.copy_construct( idx, cpy_idx );  
           cpy_idx += so;
         }
      }
      void move_from( char* src, int cnt ) {
         const unsigned int so = _vtbl.size_of();
         char* end = _data + cnt * so;
         char* cpy_idx = src;
         for( char* idx = _data; idx < end; idx += so ) {
           _vtbl.move_construct( idx, cpy_idx );  
           cpy_idx += so;
         }
      }
      void destruct( char* src, int cnt ) {
         const unsigned int so = _vtbl.size_of();
         char* end = src + cnt * so;
         for( char* idx = src; idx < end; idx += so ) {
           _vtbl.destructor( idx );
         }
      }

      ~vector_impl_d() {
        clear();
        delete[] _data;
      }

      void clear() {
        const unsigned int so = _vtbl.size_of();
        char* end = _data + _size * so;
        for( char* idx = _data; idx < end; idx += so ) {
          _vtbl.destructor( idx );  
        }
        _size = 0;
      }
    };

    vector_impl::vector_impl( abstract_value_type& r, size_t s ) {
      my = new vector_impl_d(r,s);
    }

    vector_impl::vector_impl( const vector_impl& cpy ) {
      my = new vector_impl_d(*cpy.my);
    }

    vector_impl::vector_impl( vector_impl&& cpy ){
      my = cpy.my;
      cpy.my = 0;
    }

    vector_impl::~vector_impl() {
      delete my;
    }

    vector_impl& vector_impl::operator=( const vector_impl& v ) {
      clear();
      reserve(v.size());
      size_t s = v.size();
      for( size_t i = 0; i < s; ++i ) {
        _push_back( v._at(i) );
      }
      return *this;
    }
    vector_impl& vector_impl::operator=( vector_impl&& v ) {
      fc::swap(my,v.my);
      return *this;
    }

    void        vector_impl::_push_back( const void* v ) {
      reserve( my->_size + 1 );
      my->_vtbl.copy_construct( _back(), v );
      my->_size++;
    }
    void        vector_impl::_push_back_m( void* v ) {
      reserve( my->_size + 1 );
      my->_vtbl.move_construct( _back(), v );
      my->_size++;
    }
    
    void*       vector_impl::_back() {
      return my->_data + my->_vtbl.size_of() * (my->_size-1);
    }
    const void* vector_impl::_back()const {
      return my->_data + my->_vtbl.size_of() * (my->_size-1);
    }

    void*       vector_impl::_at(size_t p) {
      return my->_data + my->_vtbl.size_of() * p;
    }
    const void* vector_impl::_at(size_t p)const {
      return my->_data + my->_vtbl.size_of() * p;
    }
    
    void vector_impl::pop_back() {
      my->_vtbl.destructor( _back() );
      my->_size--;
    }
    void vector_impl::clear() {
      my->clear(); 
    }

    size_t vector_impl::size()const {
      return my->_size;
    }

    void     vector_impl::reserve( size_t s ) {
      if( s < my->_capacity ) {
        return;
      }
      char* new_data = new char[s*my->_vtbl.size_of()];
      fc::swap(new_data,my->_data);
      my->move_from(new_data,my->_size);
      my->destruct(new_data,my->_size);
      delete[] new_data;
    }
    void     vector_impl::resize( size_t s ) {
      if( s <= my->_size ) {
         for( size_t i = s; i < my->_size; ++i ) {
             my->_vtbl.destructor( _at(i) );
         }
         my->_size = s;
         return;
      }
      if( s <= my->_capacity ) {

        return;
      }
      const unsigned int so = my->_vtbl.size_of();
      char* new_data = new char[s*so];
      fc::swap(new_data,my->_data);
      my->_capacity = s;
      // move from old to new location
      my->move_from(new_data,my->_size);
      // destroy old location
      my->destruct(new_data,my->_size);
      delete[] new_data;

      // default construct any left overs.
      char* cur = (char*)_back();
      char* end = (char*)my->_data + s * so;
      while( cur < end ) {
        my->_vtbl.construct(cur);
        cur += so;
        my->_size++;
      }
    }

    size_t vector_impl::capacity()const {
      return my->_capacity;
    }

} // namespace fc
