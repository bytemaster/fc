#ifndef _FC_BUFFER_HPP_
#define _FC_BUFFER_HPP_
namespace fc {

  struct const_buffer {
    const_buffer( const char* const c = 0, size_t l = 0 )
    :data(c),len(l){}
    const char* const data;
    size_t            len;
  };

  struct mutable_buffer {
    mutable_buffer( char* c = 0, size_t l = 0 )
    :data(c),len(l){}
    char*     data;
    size_t    len;
  };

}

#endif // _FC_BUFFER_HPP_
