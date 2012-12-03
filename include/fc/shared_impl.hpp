#pragma once
#include <fc/shared_ptr.hpp>

namespace fc {
  /**
   *  @class shared_impl<T>
   *  @brief used to create types with reference semantics 
   *
   *  A type with reference semantics is effectively a shared pointer except that
   *  it is used like a Java or C# type using '.' notation instead of -> notation.
   *
   *  It is ideal for use with classes that almost always get managed by a shared
   *  pointer (sockets and long-lived resources).  These types are rarely, if ever
   *  copied by value and often by-value copy or 'deep-copy' has no real meaning.
   *
   *  An additional feature of shared_impl<T> is that your classes implementation
   *  should be private and defined entirely in your source file.  There should be
   *  no member variables defined in your types header.
   *
   *  To make this design pattern work requires a lot of 'boiler-plate' code for
   *  handling assignments, moves, copies, etc that cannot be provided via tradtional
   *  means such as templates or base classes. 
   *
   *  To create a new type with reference semantics you place FC_REFERENCE_TYPE(type) 
   *  inside the private section of 'type'. Then in your source file you will
   *  need to define your 'private' data.
   *
   *  @code
   *  #include <your_type.hpp>
   *
   *  FC_START_SHARED_IMPL( your_namespace::your_type )
   *    impl( int x, int y ); // custom constructor
   *
   *    ...
   *    int member_variable;
   *    your private member variables / methods go here.
   *    ...
   *  FC_END_SHARED_IMPL
   *  #include <fc/shared_impl.cpp>
   *  @endcode
   *
   *
   *  Lastly, you will need to provide the implementation your class below.  This
   *  implementation will need to provide the matching implementations for the
   *  methods declared by FC_REFERENCE_TYPE(type) in your header.  To do this
   *  use the FC_REFERENCE_TYPE_IMPL(type)
   *
   *  @code
   *  namespace your_namespace {
   *   FC_REFERENCE_TYPE(your_type) 
   *   ... your methods here...
   *  } 
   *  @endcode
   * 
   *  Within the body of your methods you can access your private data and members
   *  via using 'my->member_variable' 
   *    
   *  If you want to define custom constructors for your reference type, you will
   *  need to implement them inside the FC_START_SHARED_IMPL block using the pattern:
   *
   *  @code
   *  FC_START_SHARED_IMPL( your_namespace::your_type )
   *    impl( int x, int y ){} // custom constructor
   *  FC_END_SHARED_IMPL
   *  @code
   *  
   *  A limited number (3) of arguments are supported for custom constructors.
   *
   *  Once you have defined your reference type you can use it like so:
   *
   *  @code
   *    your_type  val  = nullptr;  // create a null type
   *    your_type  val2 = new your_type(...); // construct a new instance, unnecessary temporary heap alloc
   *    your_type  val3(...);                 // constructs a new instance, more effecient
   *
   *    val2.your_method();
   *    val2 = nullptr; // reset val2 to a null object
   *
   *    val2 = val3;  // val2 and val3 now reference the same data
   *    if( !!val2 ){} // val2 is not null
   *    else{} // val2 is null
   *  @endcode
   *
   *  As you can see, when creating types with this method your code will
   *  look and act like a Java or C# garbage collected type.
   *
   *  Often times your private methods will need to call your public methods, to achieve
   *  this you can use the following techinque:
   *
   *  @code
   *  FC_START_SHARED_IMPL( your_namespace::your_type )
   *    void private_method( int x, int y ){
   *      auto s = self();
   *      s.public_method();
   *    } 
   *  FC_END_SHARED_IMPL
   *  @code
   *
   *  For performance reasons, it is best to only call 'self()' once and save the
   *  result to avoid unnecessary copies of shared pointers which require atomic
   *  operations.
   *
   */
  template<typename T>
  struct shared_impl {
    class impl;
    impl& operator*  ()const;
    impl* operator-> ()const;

    bool operator !()const;

    template<typename U>
    shared_impl( U&& u );

    shared_impl( const shared_impl& u );
    shared_impl( shared_impl&& u );
    shared_impl& operator=( shared_impl&& u );
    shared_impl& operator=( const shared_impl& u );

    ~shared_impl();

    fc::shared_ptr<typename shared_impl<T>::impl> _impl;
  };

}

#define FC_REFERENCE_TYPE( TYPE ) \
  public:\
      TYPE(); \
      TYPE( std::nullptr_t ); \
      TYPE( TYPE*  ); \
      TYPE( TYPE&&  ); \
      TYPE( const TYPE&  ); \
      template<typename A1> \
      TYPE( A1&&  ); \
      template<typename A1,typename A2> \
      TYPE( A1&&, A2&&  ); \
      template<typename A1,typename A2, typename A3> \
      TYPE( A1&&, A2&&, A3&&  ); \
      ~TYPE(); \
      bool operator !()const; \
      friend bool operator==( const TYPE& a, const TYPE& b ); \
      friend bool operator!=( const TYPE& a, const TYPE& b ); \
      TYPE& operator = ( const TYPE&  ); \
      TYPE& operator = ( TYPE&&  );\
      TYPE& operator = ( TYPE*  );\
      TYPE& operator = ( std::nullptr_t  );\
  private: \
      friend class shared_impl<TYPE>::impl; \
      TYPE( shared_impl<TYPE>::impl* m ); \
      shared_impl<TYPE> my; 

#define FC_START_SHARED_IMPL( SCOPED_TYPE )  \
namespace fc { \
template<> \
struct fc::shared_impl<SCOPED_TYPE>::impl : public fc::retainable { \
   SCOPED_TYPE self() { return SCOPED_TYPE(this); } \


#define FC_END_SHARED_IMPL }; }

