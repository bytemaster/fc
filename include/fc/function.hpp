#ifndef _FC_FUNCTION_HPP_
#define _FC_FUNCTION_HPP_

namespace fc {

   namespace detail {
     template<typename Functor>
     void call( void* functor ) {
        (*static_cast<Functor*>(functor*))();
     }
   }

   class function {
      public:
        template<typename Functor>
        function( Functor&& f ) {
          static_assert( sizeof(f) <= sizeof(store) );
          new ((void*)&store[0]) Functor( fc::forward<Functor>(f) );
          call = &detail::call<Functor>;
          copy = &detail::copy<Functor>;
          move = &detail::move<Functor>;
        }

        function( const function& f )
        :call(f.call),move(f.move),copy(f.copy){
          copy( &f.store[0], &store[0] );
        }

        function( function&& f )
        :call(f.call),move(f.move),copy(f.copy){
          move( &f.store[0], &store[0] );
        }

        function& operator = ( function&& f ) {
            
        }

        void operator()()const { call(&store[0]); }

      private:
        uint64_t store[8];
        void     (*call)(void*);
        void     (*move)(void* src, void* dst);
        void     (*copy)(const void*, void* dst);
        void     (*destroy)(void*);
   };

}

#endif // _FC_FUNCTION_HPP_
