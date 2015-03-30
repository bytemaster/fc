#pragma once
#include <fc/variant.hpp>
#include <fc/optional.hpp>
#include <fc/api.hpp>
#include <fc/any.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <utility>
//#include <fc/rpc/json_connection.hpp>

namespace fc {
   class api_connection;
   
   typedef uint32_t api_id_type;

   namespace detail {
      class generic_api
      {
         public:
            template<typename Api>
            generic_api( const Api& a, const std::shared_ptr<fc::api_connection>& c );

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
               api_visitor( generic_api& a, const std::shared_ptr<fc::api_connection>& s ):api(a),_api_con(s){ }

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

               template<typename R, typename Arg0, typename ... Args>
               R call_generic( const std::function<R(Arg0,Args...)>& f, variants::const_iterator a0, variants::const_iterator e )const
               {
                  FC_ASSERT( a0 != e );
                  return  call_generic<R,Args...>( bind_first_arg<R,Arg0,Args...>( f, a0->as< typename std::decay<Arg0>::type >() ), a0+1, e );
               }

               template<typename Interface, typename Adaptor, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<api<Interface,Adaptor>(Args...)>& f )const;

               template<typename Interface, typename Adaptor, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<fc::optional<api<Interface,Adaptor>>(Args...)>& f )const;

               template<typename R, typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<R(Args...)>& f )const
               {
                  return [=]( const variants& args ) { 
                     return variant( call_generic( f, args.begin(), args.end() ) ); 
                  };
               }

               template<typename ... Args>
               std::function<variant(const fc::variants&)> to_generic( const std::function<void(Args...)>& f )const
               {
                  return [=]( const variants& args ) { 
                     call_generic( f, args.begin(), args.end() ); 
                     return variant();
                  };
               }

               template<typename Result, typename... Args>
               void operator()( const char* name, std::function<Result(Args...)>& memb )const {
                  api._methods.emplace_back( to_generic( memb ) ); 
                  api._by_name[name] = api._methods.size() - 1;
               }

               generic_api&  api;
               const std::shared_ptr<fc::api_connection>& _api_con;
            };


            std::shared_ptr<fc::api_connection>                     _api_connection;
            fc::any                                                 _api;
            std::map< std::string, uint32_t >                       _by_name;
            std::vector< std::function<variant(const variants&)> >  _methods;
      }; // class generic_api
   } // namespace detail



   class api_connection : public std::enable_shared_from_this<fc::api_connection>
   {
      public:
         api_connection(){}
         virtual ~api_connection(){};


         template<typename T>
         api<T> get_remote_api( api_id_type api_id = 0 )
         {
            api<T> result;
            result->visit( api_visitor(  api_id, this->shared_from_this() ) );
            return result;
         }
  
         /** makes calls to the remote server */
         virtual variant send_call( api_id_type api_id, const string& method_name, const variants& args = variants() ) = 0;

         variant receive_call( api_id_type api_id, const string& method_name, const variants& args = variants() )const
         {
            //wdump( (api_id)(method_name)(args) );
            FC_ASSERT( _local_apis.size() > api_id );
            return _local_apis[api_id]->call( method_name, args );
         }

         template<typename Interface>
         api_id_type register_api( const Interface& a )
         {
            _local_apis.push_back( std::unique_ptr<detail::generic_api>( new detail::generic_api(a, shared_from_this() ) ) );
            return _local_apis.size() - 1;
         }

      private:
         std::vector< std::unique_ptr<detail::generic_api> > _local_apis;


         struct api_visitor
         {
            uint32_t                            _api_id;
            std::shared_ptr<fc::api_connection> _connection;

            api_visitor( uint32_t api_id, std::shared_ptr<fc::api_connection> con )
            :_api_id(api_id),_connection(std::move(con))
            {
            }

            api_visitor() = delete;

            template<typename Result>
            static Result from_variant( const variant& v, Result*, const std::shared_ptr<fc::api_connection>&  )
            {
               return v.as<Result>();
            }

            template<typename ResultInterface>
            static fc::api<ResultInterface> from_variant( const variant& v, 
                                                                        fc::api<ResultInterface>* /*used for template deduction*/,
                                                                        const std::shared_ptr<fc::api_connection>&  con 
                                                                      )
            {
               return con->get_remote_api<ResultInterface>( v.as_uint64() );
            }


            template<typename Result, typename... Args>
            void operator()( const char* name, std::function<Result(Args...)>& memb )const 
            {
                auto con   = _connection;
                auto api_id = _api_id;
                memb = [con,api_id,name]( Args... args ) {
                    auto var_result = con->send_call( api_id, name, {args...} );
                    return from_variant( var_result, (Result*)nullptr, con );
                };
            }
         };
   };

   class local_api_connection : public api_connection
   {
      public:
         /** makes calls to the remote server */
         virtual variant send_call( api_id_type api_id, const string& method_name, const variants& args = variants() ) override
         {
            FC_ASSERT( _remote_connection );
            return _remote_connection->receive_call( api_id, method_name, args );
         }

         void  set_remote_connection( const std::shared_ptr<fc::api_connection>& rc )
         {
            FC_ASSERT( !_remote_connection );
            FC_ASSERT( rc != this->shared_from_this() );
            _remote_connection = rc;
         }
         const std::shared_ptr<fc::api_connection>& remote_connection()const  { return _remote_connection; }

         std::shared_ptr<fc::api_connection>    _remote_connection;
   };

   template<typename Api>
   detail::generic_api::generic_api( const Api& a, const std::shared_ptr<fc::api_connection>& c )
   :_api_connection(c),_api(a)
   {
      boost::any_cast<const Api&>(a)->visit( api_visitor( *this, _api_connection ) );
   }

   template<typename Interface, typename Adaptor, typename ... Args>
   std::function<variant(const fc::variants&)> detail::generic_api::api_visitor::to_generic( 
                                               const std::function<fc::api<Interface,Adaptor>(Args...)>& f )const
   {
      auto api_con = _api_con;
      return [=]( const variants& args ) { 
         auto api_result = call_generic( f, args.begin(), args.end() ); 
         return api_con->register_api( api_result );
      };
   }
   template<typename Interface, typename Adaptor, typename ... Args>
   std::function<variant(const fc::variants&)> detail::generic_api::api_visitor::to_generic( 
                                               const std::function<fc::optional<fc::api<Interface,Adaptor>>(Args...)>& f )const
   {
      auto api_con = _api_con;
      return [=]( const variants& args )-> fc::variant { 
         auto api_result = call_generic( f, args.begin(), args.end() ); 
         if( api_result )
            return api_con->register_api( *api_result );
         return variant();
      };
   }

   /*
   class json_api_connection : public api_connection
   {
      public:
         json_api_connection(){};
         void set_json_connection( const std::shared_ptr<rpc::json_connection>& json_con )
         {
            json_con->add_method( "call", [this]( const variants& args ) -> variant {
                      FC_ASSERT( args.size() == 3 && args[2].is_array() );
                      return this->receive_call( args[0].as_uint64(),
                                          args[1].as_string(),
                                          args[2].get_array() );

                                  });
         }

         json_api_connection( const std::shared_ptr<rpc::json_connection>& json_con )
         {
            set_json_connection( json_con );
         }

         virtual variant send_call( api_id_type api_id, 
                                    const string& method_name, 
                                    const variants& args = variants() )const override
         {
            return _json_con->async_call( "call", {api_id, method_name, args} ).wait();
         }

      private:
         std::shared_ptr<rpc::json_connection> _json_con;

   };
   */

} // fc
