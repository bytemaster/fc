#pragma once
#include <fc/variant.hpp>
#include <fc/optional.hpp>
#include <fc/api.hpp>
#include <fc/any.hpp>
#include <vector>
#include <functional>
#include <utility>

namespace fc {
   class api_server;
   namespace impl {
      using std::vector;

      class generic_api
      {
         public:
            template<typename Api>
            generic_api( const Api& a, api_server& s )
            :_api_server(s),_api(a)
            {
               boost::any_cast<const Api&>(a)->visit( api_visitor( *this, _api_server ) );
            }
            generic_api( const generic_api& cpy ) = delete;

            variant call( const string& name, const variants& args )
            {
               auto itr = _by_name.find(name);
               FC_ASSERT( itr != _by_name.end() );
               return call( itr->second, args );
            }

            variant call( uint32_t method_id, const variants& args )
            {
               FC_ASSERT( method_id < _methods.size() );
               return _methods[method_id](args);
            }

         private:
            friend struct api_visitor;

            struct api_visitor
            {
               api_visitor( generic_api& a, api_server& s ):api(a),server(s){
               }
               ~api_visitor()
               {
               }

               template<typename R, typename Arg0, typename ... Args>
               std::function<R(Args...)> bind_first_arg( const std::function<R(Arg0,Args...)>& f, Arg0 a0 )const
               {
                  return [=]( Args... args ) { return f( a0, args... ); };
               }
               template<typename R>
               R call_generic( const std::function<R()>& f, variants::const_iterator a0, variants::const_iterator e )const
               {
                  return f();
               }

               template<typename R, typename Arg0>
               R call_generic( const std::function<R(Arg0)>& f, variants::const_iterator a0, variants::const_iterator e )const
               {
                  FC_ASSERT( a0 != e );
                  return f(a0->as<Arg0>());
               }
               template<typename R, typename Arg0, typename Arg1>
               R call_generic( const std::function<R(Arg0,Arg1)>& f, variants::const_iterator a0, variants::const_iterator e )const
               {
                  FC_ASSERT( a0 != e && (a0+1) != e);
                  return f(a0->as<Arg0>(), (a0+1)->as<Arg1>());
               }

               template<typename R, typename Arg0, typename ... Args>
               R call_generic( const std::function<R(Arg0,Args...)>& f, variants::const_iterator a0, variants::const_iterator e )const
               {
                  FC_ASSERT( a0 != e );
                  return  call_generic<R,Args...>( bind_first_arg( f, a0->as<Arg0>() ), a0+1, e );
               }

               template<typename Interface, typename Adaptor, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<api<Interface,Adaptor>(Args...)>& f )const;
               template<typename Interface, typename Adaptor, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<fc::optional<api<Interface,Adaptor>>(Args...)>& f )const;

               template<typename R, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<R(Args...)>& f )const
               {
                  return [=]( const variants& args ) { 
                     return call_generic( f, args.begin(), args.end() ); 
                  };
               }


               template<typename Result, typename... Args>
               void operator()( const char* name, std::function<Result(Args...)>& memb )const {
                  api._methods.emplace_back( to_generic( memb ) ); 
                  api._by_name[name] = api._methods.size() - 1;
               }
               generic_api&  api;
               api_server&   server;
               api_server*   server_ptr = nullptr;
            };


            api_server&                                       _api_server;
            fc::any                                           _api;
            std::map< std::string, uint32_t >                 _by_name;
            vector< std::function<variant(const variants&)> > _methods;
      };



   } // namespace impl


   /**
    *  @brief exposes Object Oriented API via a flat RPC call interface
    *
    *  The wire API is simply:
    *
    *  variant call( obj_id, method_num|name, [params...] )
    *
    *  The default obj_id is 0.
    */
   class api_server
   {
      public:
         typedef uint32_t api_id_type;
         api_server()
         {
         }
         api_server( const api_server& cpy ) = delete;

         template<typename Interface>
         api_id_type register_api( const Interface& a )
         {
            _apis.push_back( std::unique_ptr<impl::generic_api>( new impl::generic_api(a, *this) ) );
            return _apis.size() - 1;
         }

         variant call( api_id_type api_id, const string& method_name, const variants& args = variants() )const
         {
            wdump( (api_id)(method_name)(args) );
            FC_ASSERT( _apis.size() > api_id );
            return _apis[api_id]->call( method_name, args );
         }

      private:
         std::vector< std::unique_ptr<impl::generic_api> > _apis;
        
   };

   template<typename Interface, typename Adaptor, typename ... Args>
   std::function<variant(const fc::variants&)> impl::generic_api::api_visitor::to_generic( 
                                               const std::function<fc::api<Interface,Adaptor>(Args...)>& f )const
   {
      auto tmp = *this;
      return [=]( const variants& args ) { 
         auto api_result = tmp.call_generic( f, args.begin(), args.end() ); 
         return tmp.server.register_api( api_result );
      };
   }
   template<typename Interface, typename Adaptor, typename ... Args>
   std::function<variant(const fc::variants&)> impl::generic_api::api_visitor::to_generic( 
                                               const std::function<fc::optional<fc::api<Interface,Adaptor>>(Args...)>& f )const
   {
      auto tmp = *this;
      return [=]( const variants& args )-> fc::variant { 
         auto api_result = tmp.call_generic( f, args.begin(), args.end() ); 
         if( api_result )
            return tmp.server.register_api( *api_result );
         return variant();
      };
   }

} // namesapce fc 

FC_API( fc::api_server, (call) )
