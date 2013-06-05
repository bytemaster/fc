#pragma once
#include <fc/vector.hpp>
#include <functional>
#include <fc/log/logger.hpp>
#ifdef emit
#undef emit
#endif

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
#ifdef WIN32
      template<typename Arg>
      void emit( Arg&& arg ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg) );
        }
      }
      template<typename Arg>
      void operator()( Arg&& arg ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg) );
        }
      }
      template<typename Arg,typename Arg2>
      void emit( Arg&& arg, Arg2&& arg2 ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg), fc::forward<Arg2>(arg2) );
        }
      }
      template<typename Arg, typename Arg2>
      void operator()( Arg&& arg, Arg2&& arg2 ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg), fc::forward<Arg2>(arg2) );
        }
      }
      template<typename Arg, typename Arg2, typename Arg3>
      void emit( Arg&& arg, Arg2&& arg2, Arg3&& arg3 ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg), fc::forward<Arg2>(arg2), fc::forward<Arg3>(arg3) );
        }
      }
      template<typename Arg, typename Arg2, typename Arg3>
      void operator()( Arg&& arg, Arg2&& arg2, Arg3&& arg3 ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Arg>(arg), fc::forward<Arg2>(arg2), fc::forward<Arg3>(arg3) );
        }
      }
#else
      template<typename... Args>
      void emit( Args&&... args ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Args>(args)... );
        }
      }
      template<typename... Args>
      void operator()( Args&&... args ) {
        for( size_t i = 0; i < _handlers.size(); ++i ) {
          (*_handlers[i])( fc::forward<Args>(args)... );
        }
      }
#endif

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
      signal()
      {
         _handlers.reserve(4);
      }
      ~signal()
      {
         for( auto itr = _handlers.begin(); itr != _handlers.end(); ++itr )
         {
            delete *itr;
         }
         _handlers.clear();
      }

    private:
      fc::vector< std::function<Signature>* > _handlers;
  };

}
