#ifndef _FC_VECTOR_HPP_
#define _FC_VECTOR_HPP_
#include <fc/abstract_types.hpp>

namespace fc {
    class vector_impl {
      public:
        size_t size()const;
        size_t capacity()const;
        void pop_back();                     
        void clear();
        void resize( size_t );
        void reserve( size_t );

      protected:
        vector_impl( abstract_value_type& v, size_t size );
        vector_impl( const vector_impl& );
        vector_impl( vector_impl&& );
        ~vector_impl();

        vector_impl& operator=( const vector_impl& v );
        vector_impl& operator=( vector_impl&& v );

        void        _push_back( const void* v );
        void        _push_back_m( void* v );
        
        void*       _back();
        const void* _back()const;

        void*       _at(size_t);
        const void* _at(size_t)const;

        void* _insert( void* pos, const void* t );
        void* _insert( void* pos, void* t );
        void* _erase( void* pos );
        void* _erase( void* first, void* last );

        struct vector_impl_d* my;
    };
    
    class vector_pod_impl {
      public:
        size_t size()const;
        size_t capacity()const;
        void pop_back();
        void clear();
        void resize( size_t );
        void reserve( size_t );

      protected:
        vector_pod_impl( unsigned int size_of, size_t size );
        vector_pod_impl( const vector_pod_impl& );
        vector_pod_impl( vector_pod_impl&& );
        ~vector_pod_impl();

        vector_pod_impl& operator=( const vector_pod_impl& v );
        vector_pod_impl& operator=( vector_pod_impl&& v );

        void        _push_back( const void* v );
        void        _push_back_m( void* v );
        
        void*       _back();
        const void* _back()const;

        void*       _at(size_t);
        const void* _at(size_t)const;

        void* _insert( void* pos, const void* t );
        void* _erase( void* pos );
        void* _erase( void* first, void* last );

        struct vector_pod_impl_d* my;
    };

    template<typename T, bool is_class = true > 
    class vector_base : public vector_impl {
      public:
        vector_base( size_t s ):vector_impl( value_type<T>::instance(), s ){};
        vector_base( const vector_base& c ):vector_impl( c ){};
        vector_base( vector_base&& c ):vector_impl( fc::move(c) ){};

        vector_base& operator=( const vector_base& v ){ vector_impl::operator=(v);           return *this; }
        vector_base& operator=( vector_base&& v )     { vector_impl::operator=(fc::move(v)); return *this; }
    };

    template<typename T>
    class vector_base<T,false> : public vector_pod_impl {
      public:
        vector_base( size_t s ):vector_pod_impl( sizeof(T), s ){};
        vector_base( const vector_base& c ):vector_pod_impl( c ){};
        vector_base( vector_base&& c ):vector_pod_impl( fc::move(c) ){};

        vector_base& operator=( const vector_base& v ){ vector_pod_impl::operator=(v);           return *this; }
        vector_base& operator=( vector_base&& v )     { vector_pod_impl::operator=(fc::move(v)); return *this; }
    };

    template<typename T>
    class vector : public vector_base<T, fc::is_class<T>::value > {
      public:
        vector( size_t size = 0 ):vector_base<T,fc::is_class<T>::value>( size ){}
        vector( const vector& v ):vector_base<T,fc::is_class<T>::value>(v){}
        vector( vector&& v ):vector_base<T,fc::is_class<T>::value>(fc::move(v)){}

        vector& operator=( const vector& v ){ vector_base<T,fc::is_class<T>::value>::operator=(v);           return *this; }
        vector& operator=( vector&& v )     { vector_base<T,fc::is_class<T>::value>::operator=(fc::move(v)); return *this; }

        typedef T*       iterator;
        typedef const T* const_iterator;

        T*       begin()                                      { return &front();                            }
        const T* begin()const                                 { return &front();                            }
        const T* end()const                                   { return &back() + 1;                         }
                                                                                                            
        void push_back( const T& t )                          { _push_back(&t);                             }
        void push_back( T&& t )                               { _push_back_m(&t);                           }
                                                                                                      
        T&       back()                                       { return *((T*)this->_back());                }
        const T& back()const                                  { return *((const T*)this->_back());          }
                                                                                                      
        T&       front()                                      { return *((T*)this->_at(0));                 }
        const T& front()const                                 { return *((const T*)this->_at(0));           }
                                                                                                      
        T&       operator[]( size_t p )                       { return *((T*)this->_at(p));                 }
        const T& operator[]( size_t p )const                  { return *((const T*)this->_at(p));           }
                                                              
        T&       at( size_t p )                               { return *((T*)this->_at(p));                 }
        const T& at( size_t p )const                          { return *((const T*)this->_at(p));           }
                                                                                                   
        iterator insert( iterator pos, const T& t )           { return (iterator*)this->_insert( pos, &t ); }
        iterator insert( iterator pos, T&& t )                { return (iterator*)this->_insert_m(pos, &t); }
        iterator erase( iterator pos )                        { return (iterator*)this->_erase(pos);        }
        iterator erase( iterator first, iterator last )       { return (iterator*)this->_erase(first,last); }
    };
  namespace reflect {
    template<typename T> class reflector;
    template<typename T> class reflector<vector<T>>;
  }
} // namespace fc

#endif // _FC_VECTOR_HPP_
