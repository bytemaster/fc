#ifndef _FC_BUFFER_HPP_
#define _FC_BUFFER_HPP_
namespace fc {

  struct const_buffer {
    const_buffer( const char* const c = 0, size_t l = 0 )
    :data(c),size(l){}
    const char* const data;
    size_t            size;
  };

  struct mutable_buffer {
    mutable_buffer( char* c = 0, size_t l = 0 )
    :data(c),size(l){}
    char*     data;
    size_t    size;
  };

}

#endif // _FC_BUFFER_HPP_
