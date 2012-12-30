#pragma once
#include <fc/vector.hpp>
#include <functional>


namespace fc {
  
  template<typename Signature>
  class signal  {
    public:
      typedef int64_t connection_id_type;

      template<typename Functor>
      connection_id_type connect( Functor&& f ) {
         auto c = new std::function<Signature>( fc::forward<Functor>(f) ); 
         _handlers.push_back( c );
         return reinterpret_cast<connection_id_type>(c);
      }

      template<typename... Args>
      void emit( Args&&... args ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( args... );
        }
      }
      template<typename... Args>
      void operator()( Args&&... args ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( args... );
        }
      }

      void disconnect( connection_id_type cid ) {
        auto itr = _handlers.begin();
        while( itr != _handlers.end() ) {
          if( reinterpret_cast<connection_id_type>(*itr) == cid ) {
            delete *itr;
            _handlers.erase(itr);
          }
          ++itr;
        }
      }

    private:
      fc::vector< std::function<Signature>* > _handlers;
  };

}
