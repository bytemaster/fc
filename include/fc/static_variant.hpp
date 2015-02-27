/** This source adapted from https://github.com/kmicklas/variadic-static_variant 
 *
 * Copyright (C) 2013 Kenneth Micklas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/
#include <stdexcept>
#include <typeinfo>

namespace fc {

// Implementation details, the user should not import this:
namespace impl {

template<int N, typename... Ts>
struct storage_ops;

template<typename X, typename... Ts>
struct position;

template<typename... Ts>
struct type_info;

template<typename StaticVariant>
struct copy_construct
{
   StaticVariant& sv;
   copy_construct( StaticVariant& s ):sv(s){}
   template<typename T>
   void operator()( const T& v )const
   {
      sv.init(v);
   }
};

template<int N, typename T, typename... Ts>
struct storage_ops<N, T&, Ts...> {
    static void del(int n, void *data) {}
    static void con(int n, void *data) {}
    
    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, visitor& v) {}

    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, const visitor& v) {}

    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, visitor& v) {}

    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, const visitor& v) {}
};

template<int N, typename T, typename... Ts>
struct storage_ops<N, T, Ts...> {
    static void del(int n, void *data) {
        if(n == N) reinterpret_cast<T*>(data)->~T();
        else storage_ops<N + 1, Ts...>::del(n, data);
    }
    static void con(int n, void *data) {
        if(n == N) new(reinterpret_cast<T*>(data)) T();
        else storage_ops<N + 1, Ts...>::del(n, data);
    }

    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, visitor& v) {
        if(n == N) return v(*reinterpret_cast<T*>(data));
        else return storage_ops<N + 1, Ts...>::apply(n, data, v);
    }

    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, const visitor& v) {
        if(n == N) return v(*reinterpret_cast<T*>(data));
        else return storage_ops<N + 1, Ts...>::apply(n, data, v);
    }

    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, visitor& v) {
        if(n == N) return v(*reinterpret_cast<const T*>(data));
        else return storage_ops<N + 1, Ts...>::apply(n, data, v);
    }

    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, const visitor& v) {
        if(n == N) return v(*reinterpret_cast<const T*>(data));
        else return storage_ops<N + 1, Ts...>::apply(n, data, v);
    }
};

template<int N>
struct storage_ops<N> {
    static void del(int n, void *data) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }
    static void con(int n, void *data) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }

    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, visitor& v) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }
    template<typename visitor>
    static typename visitor::result_type apply(int n, void *data, const visitor& v) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }
    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, visitor& v) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }
    template<typename visitor>
    static typename visitor::result_type apply(int n, const void *data, const visitor& v) {
        throw std::runtime_error(
            "Internal error: static_variant tag is invalid."
        );
    }
};

template<typename X>
struct position<X> {
    static const int pos = -1;
};

template<typename X, typename... Ts>
struct position<X, X, Ts...> {
    static const int pos = 0;
};

template<typename X, typename T, typename... Ts>
struct position<X, T, Ts...> {
    static const int pos = position<X, Ts...>::pos != -1 ? position<X, Ts...>::pos + 1 : -1;
};

template<typename T, typename... Ts>
struct type_info<T&, Ts...> {
    static const bool no_reference_types = false;
    static const bool no_duplicates = position<T, Ts...>::pos == -1 && type_info<Ts...>::no_duplicates;
    static const size_t size = type_info<Ts...>::size > sizeof(T&) ? type_info<Ts...>::size : sizeof(T&);
};

template<typename T, typename... Ts>
struct type_info<T, Ts...> {
    static const bool no_reference_types = type_info<Ts...>::no_reference_types;
    static const bool no_duplicates = position<T, Ts...>::pos == -1 && type_info<Ts...>::no_duplicates;
    static const size_t size = type_info<Ts...>::size > sizeof(T) ? type_info<Ts...>::size : sizeof(T&);
};

template<>
struct type_info<> {
    static const bool no_reference_types = true;
    static const bool no_duplicates = true;
    static const size_t size = 0;
};

} // namespace impl

template<typename... Types>
class static_variant {
    static_assert(impl::type_info<Types...>::no_reference_types, "Reference types are not permitted in static_variant.");
    static_assert(impl::type_info<Types...>::no_duplicates, "static_variant type arguments contain duplicate types.");
    
    int _tag;
    char storage[impl::type_info<Types...>::size];
    
    template<typename X>
    void init(const X& x) {
        _tag = impl::position<X, Types...>::pos;
        new(storage) X(x);
    }

    template<typename StaticVariant>
    friend struct copy_construct;
public:
    template<typename X>
    struct tag
    {
       static_assert(
         impl::position<X, Types...>::pos != -1,
         "Type not in static_variant."
       );
       static const int value = impl::position<X, Types...>::pos;
    };
    static_variant()
    {
       _tag = 0;
       impl::storage_ops<0, Types...>::con(0, storage);
    }

    template<typename... Other>
    static_variant( const static_variant<Other...>& cpy )
    {
       cpy.apply( impl::copy_construct<static_variant<Types...>>(*this) );
    }

    template<typename X>
    static_variant(const X& v) {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        init(v);
    }
    ~static_variant() {
        impl::storage_ops<0, Types...>::del(_tag, storage);
    }
    template<typename X>
    void operator=(const X& v) {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        this->~static_variant();
        init(v);
    }
    template<typename X>
    X& get() {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        if(_tag == impl::position<X, Types...>::pos) {
            return *reinterpret_cast<X*>(storage);
        } else {
            throw std::runtime_error(
                std::string("static_variant does not contain value of type ") + typeid(X).name()
            );
        }
    }
    template<typename X>
    const X& get() const {
        static_assert(
            impl::position<X, Types...>::pos != -1,
            "Type not in static_variant."
        );
        if(_tag == impl::position<X, Types...>::pos) {
            return *reinterpret_cast<const X*>(storage);
        } else {
            throw std::runtime_error(
                std::string("static_variant does not contain value of type ") + typeid(X).name()
            );
        }
    }
    template<typename visitor>
    typename visitor::result_type visit(visitor& v) {
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    }

    template<typename visitor>
    typename visitor::result_type visit(const visitor& v) {
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    }

    template<typename visitor>
    typename visitor::result_type visit(visitor& v)const {
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    }

    template<typename visitor>
    typename visitor::result_type visit(const visitor& v)const {
        return impl::storage_ops<0, Types...>::apply(_tag, storage, v);
    }
    
     void set_which( int w ) { 
       this->~static_variant();
           impl::storage_ops<0, Types...>::con(_tag, storage);
     }

     int which() const {return _tag;}
};

template<typename Result>
struct visitor {
    typedef Result result_type;
};

} // namespace fc
