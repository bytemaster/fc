#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fc/utility.hpp>
#include <fc/log.hpp>


namespace fc {
  namespace detail {
    template<typename T>
    struct data {
      uint64_t size;
      uint64_t capacity;
      T        first;

      static data* allocate( uint64_t cap ) {
        data* d = nullptr;
        if( cap ){
          d = (data*)malloc(sizeof(data) + sizeof(T)*(static_cast<size_t>(cap)-1));
          d->capacity = static_cast<size_t>(cap);
        } else {
          d = (data*)malloc(sizeof(data));
          d->capacity = 1;
        }
        d->size = 0;
        return d;
      }
      static data* reallocate( data* d, uint64_t cap ) {
        if( cap ){
          d = (data*)realloc(d,sizeof(data) + sizeof(T)*(static_cast<size_t>(cap)-1));
          d->capacity = static_cast<size_t>(cap);
        } else {
          d = (data*)realloc(d,sizeof(data));
          d->capacity = 1;
        }
        if( d->size > d->capacity ) 
           d->size = d->capacity;
        return d;
      }
      private:
        data(){};
    };

    template<typename T, typename IsClass=fc::false_type>
    struct vector_impl {
      public:
          typedef T*       iterator;
          typedef const T* const_iterator;
          vector_impl():_data(nullptr){}
          vector_impl( vector_impl&& c):_data(c._data){c._data =nullptr; }
          vector_impl( const vector_impl& c):_data(nullptr) {
            //slog( "copy: c.size %d", c.size() );
            if( c.size() ) {
              _data  = detail::data<T>::allocate( c.size() );
              _data->size = c.size();
              memcpy(begin(),c.begin(), static_cast<size_t>(c.size()) );
            }
            //slog( "copy: this.size %d", size() );
          }
          vector_impl(const_iterator b, const_iterator e ):_data(nullptr) {
            resize(e-b);
            if( size() ) memcpy( data(), b, static_cast<size_t>(size()) );
          }
          vector_impl(uint64_t s):_data(nullptr){
            resize(s);
          }
          ~vector_impl() {
            clear();
          }


          uint64_t size()const     { return _data ? _data->size : 0;     }
          uint64_t capacity()const { return _data ? _data->capacity : 0; }

          T&       back()       { return (&_data->first)[-1+_data->size]; }
          const T& back()const  { return (&_data->first)[-1+_data->size]; }
          T&       front()      { return (&_data->first)[0]; }
          const T& front()const { return (&_data->first)[0]; } 
          const T* data()const  { return (&_data->first);    }
          T*       data()       { return (&_data->first);    }

          iterator       begin()            { return _data ? &front()   : 0;}
          const_iterator begin()const { return _data ? &front()   : 0;}
          iterator       end()        { return _data ? (&back())+1: 0;}
          const_iterator end()const   { return _data ? (&back())+1: 0;}

          T&       operator[]( uint64_t i )      { return (&_data->first)[i]; }
          const T& operator[]( uint64_t i )const { return (&_data->first)[i]; }

          T&       at( uint64_t i )      { return (&_data->first)[i]; }
          const T& at( uint64_t i )const { return (&_data->first)[i]; }

          void pop_back() { erase( &back() ); }

          void clear() {
            if( _data != nullptr )  {
              free(_data); 
            }
            _data = nullptr;
          }

          void     reserve( uint64_t i ) {
            _data = detail::data<T>::reallocate( _data, i );
          }

          void     resize( uint64_t i ) {
            if( capacity() < i ) {
               if( _data )
                   _data = detail::data<T>::reallocate( _data, i );
               else
                   _data = detail::data<T>::allocate( i );
            }
            if( _data ) _data->size = i;
          }

          template<typename U>
          void push_back( U&& v ) {
            resize( size()+1 );
            back() = fc::forward<U>(v);
          }

          template<typename U>
          iterator insert( const_iterator loc, U&& t ) {
            uint64_t pos = loc - begin();
            resize( size()+1 );
            char* src = &at(pos);
            if( src != &back() ) 
                memmove( src+1, src, (&back() - src) );
            &back = fc::forward<U>(t);
            return &at(pos);
          }

          iterator insert( iterator pos, const_iterator first, const_iterator last ) {
            if( first >= last ) return pos;

            uint64_t loc = pos - begin();
            uint64_t right_size = size() - loc;
            resize( size() + (last-first) );
            char* src = &at(loc);
            uint64_t s = last-first;
            memmove( src + s, src, right_size );
            memcpy( src, first, s );
            _data->size += (last-first);
            return src;
          }

          iterator erase( iterator pos )  {
             memmove( pos, pos+1, (&back() - pos) );
             _data->size--;
             return pos;
          }

          iterator erase( iterator first, iterator last ) {
            if( first != last ) {
               memmove( first, first + (last-first), (&back() - last) );
               _data->size -= last-first;
             }
             return first;
          }

          vector_impl& operator=( vector_impl&& v ) {
             fc_swap(_data,v._data);
             return *this;
          }
          vector_impl& operator=( const vector_impl& v ) {
             vector_impl tmp(v);
             fc_swap(tmp._data,_data);
             return *this;
          }
      protected:
          detail::data<T>*  _data;
    };
    
    template<typename T>
    struct vector_impl<T,fc::true_type>  {
      public: 
          typedef T*       iterator;
          typedef const T* const_iterator;
          vector_impl():_data(nullptr){}
          vector_impl( vector_impl&& c):_data(c._data){c._data =nullptr; }
          vector_impl( const vector_impl& c):_data(nullptr) {
            if( c.size() ) {
              _data  = detail::data<T>::allocate( c.size() );
              auto i = begin();
              auto ci = c.begin();
              auto ce = c.end();
              while( ci != ce ) {
                new (i) T(*ci);
                ++i;
                ++_data->size;
                ++ci;
              }
            }
          }
          vector_impl(const_iterator b, const_iterator e ):_data(nullptr) {
            resize(e-b);
            for( auto i = begin(); i != end(); ++i ) {
              *i = *b;
              ++b;
            }
          }

          vector_impl(uint64_t s):_data(nullptr){
            resize(s);
          }
          ~vector_impl() {
            clear();
          }


          uint64_t size()const     { return _data ? _data->size : 0;     }
          uint64_t capacity()const { return _data ? _data->capacity : 0; }

          T&       back()       { return (&_data->first)[-1+_data->size]; }
          const T& back()const  { return (&_data->first)[-1+_data->size]; }
          T&       front()      { return (&_data->first)[0]; }
          const T& front()const { return (&_data->first)[0]; }
          const T* data()const  { return (&_data->first);    }
          T*       data()       { return (&_data->first);    }

          iterator begin()            { return _data ? &front()   : 0;}
          const_iterator begin()const { return _data ? &front()   : 0;}
          const_iterator end()const   { return _data ? (&back())+1: 0;}

          T&       operator[]( uint64_t i )      { return (&_data->first)[i]; }
          const T& operator[]( uint64_t i )const { return (&_data->first)[i]; }

          T&       at( uint64_t i )      { return (&_data->first)[i]; }
          const T& at( uint64_t i )const { return (&_data->first)[i]; }

          void pop_back() { erase( &back() ); }


          void clear() {
            if( this->_data != nullptr ) {
              auto c = this->begin();
              auto e = this->end();
              while( c != e ) {
                (*c).~T();
                ++c;
              }
              free(this->_data); 
            }
            this->_data = nullptr;
          }

          void     reserve( uint64_t i ) {
            if( nullptr != this->_data && i <= this->_data->capacity ) 
               return;
            
            auto _ndata = detail::data<T>::allocate( i );
            auto nc = &_ndata->first;
            auto c = this->begin();
            auto e = this->end();
            while( c != e ) {
              new (nc) T(fc::move( *c )); 
              (*c).~T();
              ++_ndata->size;
              ++c;
              ++nc;
            }
            fc_swap( _ndata, this->_data );
            if( _ndata ) free(_ndata);
          }

          void     resize( uint64_t i ) {
            this->reserve(i);
            while( i < this->_data->size ) {
              this->back().~T();
              --this->_data->size;
            }
            while( this->_data->size < i ) {
              new (&this->back()+1) T();
              ++this->_data->size;
            }
          }

          template<typename U>
          void push_back( U&& v ) {
            this->reserve( this->size()+1 );
            new (&back()+1) T(fc::forward<U>(v));
            ++this->_data->size;
          }

          template<typename U>
          iterator insert( const_iterator loc, U&& t ) {
            uint64_t pos = loc - this->begin();
            this->reserve( this->size()+1 );
            loc = this->begin() + pos;
            if( this->size() != 0 ) {
                new ((void*)this->end()) T( fc::move(this->back()) );
                auto cur = this->back();
                ++this->_data->size;
                while( cur != loc ) {
                  *cur = fc::move( *(cur-1) );
                }
                *cur = fc::forward<U>(t);
            } else {
                new (this->end()) T( fc::forward<U>(t) );
                ++this->_data->size;
            }
            return &this->at(pos);
          }

          iterator insert( iterator pos, const_iterator first, const_iterator last ) {
            //static_assert( false, "Not Implemented" );
            return 0;
          }

          iterator erase( iterator pos )  {
             if( pos == this->end() ) { return pos; }
             auto next = pos + 1;
             while( next != this->end() ) {
                *pos = fc::move(*next);
                ++pos; ++next;
             }
             pos->~T();
             this->_data->size--;
             return pos;
          }

          iterator erase( iterator first, iterator last ) {
            iterator c = first;
            iterator m = last;
            iterator e = this->end();
            while( c != e ) {
              if( m != e ) *c = fc::move( *m );
              else c->~T();
              ++c;
              ++m;
            }
            this->_data->size -= last-first;
            return last;
          }
          vector_impl& operator=( vector_impl&& v ) {
             fc_swap(_data,v._data);
             return *this;
          }
          vector_impl& operator=( const vector_impl& v ) {
             vector_impl tmp(v);
             fc_swap(tmp._data,_data);
             return *this;
          }
       private:
          detail::data<T>* _data;
    };
  }

  template<typename T>
  class vector : public detail::vector_impl<T, typename fc::is_class<T>::type> {
    public:
      vector(){}
      vector( uint64_t s ):detail::vector_impl<T, typename fc::is_class<T>::type>(s){}
      vector( const vector& v ):detail::vector_impl<T, typename fc::is_class<T>::type>(v){}
      vector( vector&& v ):detail::vector_impl<T, typename fc::is_class<T>::type>(fc::move(v)){}

      vector( const T* b, const T* e ):detail::vector_impl<T, typename fc::is_class<T>::type>(b,e){}

      vector& operator=( vector&& v ) {
         *((base*)this) = fc::move(v);
         return *this;
      }
      vector& operator=( const vector& v ) {
         *((base*)this) = v;
         return *this;
      }
    private:
      typedef detail::vector_impl<T, typename fc::is_class<T>::type>  base;
  };

};

