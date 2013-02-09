#pragma once

#include <fc/string.hpp>
#include <fc/vector.hpp>
#include <fc/aligned.hpp>
#include <fc/typename.hpp>
#include <fc/optional.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>

namespace fc {
    template<BOOST_PP_ENUM_PARAMS(9, typename A)> struct tuple;

    /**
     *  @brief a dynamic container that can hold
     *  integers, reals, strings, booleans, arrays, and
     *  or null.   
     *
     *  This type serves as an intermediate representation between
     *  C++ type and serialized type (JSON,XML,etc).  
     *
     *  As much as possible value attempts to preserve 'type' information, but
     *  type information is not always provided equally by all serialization formats.
     *
     *  value is move aware, so move it when you can to avoid expensive copies
     */
    class value {
      public:
        enum value_type {
           null_type, string_type, bool_type,
           int8_type, int16_type, int32_type, int64_type,
           uint8_type, uint16_type, uint32_type, uint64_type,
           double_type, float_type, 
           array_type, object_type
        };

        class key_val;
        class object {
          public:
            typedef fc::vector<key_val>::const_iterator const_iterator;
            fc::vector<key_val>  fields;
        };
        typedef fc::vector<value> array;

        struct const_visitor {
          virtual ~const_visitor(){}
          virtual void operator()(const int8_t& v      )=0;
          virtual void operator()(const int16_t& v     )=0;
          virtual void operator()(const int32_t& v     )=0;
          virtual void operator()(const int64_t& v     )=0;
          virtual void operator()(const uint8_t& v     )=0;
          virtual void operator()(const uint16_t& v    )=0;
          virtual void operator()(const uint32_t& v    )=0;
          virtual void operator()(const uint64_t& v    )=0;
          virtual void operator()(const float& v       )=0;
          virtual void operator()(const double& v      )=0;
          virtual void operator()(const bool& v        )=0;
          virtual void operator()(const fc::string& v  )=0;
          virtual void operator()(const object&  )=0;
          virtual void operator()(const fc::vector<value>&  )=0;
          virtual void operator()()=0;
        };

        value();
        value(value&& m );
        value(const value& m );

        value(const char* c );
        value(char* c );
        value(int8_t );
        value(int16_t );
        value(int32_t );
        value(int64_t );
        value(uint8_t );
        value(uint16_t );
        value(uint32_t );
        value(uint64_t );
        value(double );
        value(float );
        value(bool );
        value(fc::string&& );
        value(fc::string& );
        value(const fc::string& );

        value(object&& o );
        value(const object& o );
        value(object& o );

        /// initialize an object with a single key/value pair
        value(const fc::string&, const value& v );
        template<typename T>
        value(const fc::string& s, const T& v ) {
           set( s, v );
        }

        value(fc::vector<value>&& a );
        value(fc::vector<value>& a );
        value(const fc::vector<value>& a );

        ~value();

        value& operator=(value&& v );
        value& operator=(const value& v );


        /**
         *  Include fc/value_cast.hpp for implementation
         */
        template<typename T>
        explicit value(const T& v );

        template<typename T>
        value& operator=( const T& v );

        template<typename T>
        T cast()const;

        /** used to iterate over object fields, use array index + size to iterate over array */
        object::const_iterator find(const char* key )const;
        object::const_iterator begin()const;
        object::const_iterator end()const;

        /** avoid creating temporary string just for comparisons! **/
        value&       operator[](const char* key );
        const value& operator[](const char* key )const;
        value&       operator[](const fc::string& key );
        const value& operator[](const fc::string& key )const;

        /** array & object interface **/
        void         clear();
        size_t       size()const;

        /** array interface **/
        void                      resize(size_t s );
        void                      reserve(size_t s );
        value&                    push_back(value&& v );
        value&                    push_back(const value& v );
        const fc::vector<value>&  as_array()const;
        fc::vector<value>&        as_array();
        const fc::string&         as_string()const;
        fc::string&               as_string();
        const value::object&      as_object()const;
        value::object&            as_object();

        /** same as push_back(), used for short-hand construction of value()(1)(2)(3)(4)  */
        value&                    operator()(fc::value v );
        value&                    operator[](int32_t idx );
        const value&              operator[](int32_t idx )const;

        /** gets the stored type **/
        value_type   type()const;
        bool         is_null()const;
        bool         is_string()const;
        bool         is_object()const;
        bool         is_array()const;

        void         visit(const_visitor&& v )const;

        /**  same as set(key, v ), used for short-hand construction of value()("key",1)("key2",2) */
        value&       operator()(const char* key, fc::value v );
        value&       set(const char* key,       fc::value v );
        value&       set(const fc::string& key, fc::value v );
        value&       clear(const fc::string& key );

        template<typename S, typename T>
        value&       set(S&& key, T&& v ) { return set(fc::forward<S>(key), fc::value(fc::forward<T>(v) ) ); }

      private:
        /** throws exceptions on errors 
         *
         *  Defined in fc/value_cast.hpp because it depends upon
         *  reflection
         */
        template<typename T>
        friend T value_cast(const value& v );

        aligned<40> holder;
    };
    typedef fc::optional<value> ovalue;
    bool operator == (const value& v, std::nullptr_t );
    bool operator != (const value& v, std::nullptr_t );

    class value::key_val {
       public:
       key_val(){};
       
       key_val(fc::string k )
       :key(fc::move(k)){}

       key_val(const fc::string& k, const value& v )
       :key(k),val(v){}
       
       key_val(key_val&& m )
       :key(fc::move(m.key)),val(fc::move(m.val)){ }
       
       key_val(const key_val& m )
       :key(m.key),val(m.val){}

       ~key_val(){ }
       
       key_val& operator=(key_val&& k ) {
         fc_swap(key, k.key );
	       fc_swap(val, k.val );
         return *this;
       }

       key_val& operator=(const key_val& k ) {
         key = k.key;
         val = k.val;
         return *this;
       }
       
       fc::string key;
       value      val;
    };

} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT_ENUM(fc::value::value_type,
   (null_type)
   (string_type)
   (bool_type)
   (int8_type)
   (int16_type)
   (int32_type)
   (int64_type)
   (uint8_type)
   (uint16_type)
   (uint32_type)
   (uint64_type)
   (double_type)
   (float_type)
   (array_type)
   (object_type)
) 
