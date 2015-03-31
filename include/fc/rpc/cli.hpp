#pragma once
#include <fc/io/stdio.hpp>
#include <fc/io/json.hpp>
#include <fc/io/buffered_iostream.hpp>
#include <fc/io/sstream.hpp>
#include <fc/thread/thread.hpp>

namespace fc { namespace rpc {

   /**
    *  Provides a simple wrapper for RPC calls to a given interface.
    */
   class cli : public api_connection
   {
      public:
         virtual variant send_call( api_id_type api_id, string method_name, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }
         virtual variant send_callback( uint64_t callback_id, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }
         virtual void    send_notice( uint64_t callback_id, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }

         void start()
         {
            _run_complete = fc::async( [&](){ run(); } );
         }
         void stop()
         {
            _run_complete.cancel();
            _run_complete.wait();
         }
         void format_result( const string& method, std::function<string(variant,const variants&)> formatter)
         {
            _result_formatters[method] = formatter;
         }
      private:
         void run()
         {
            while( !_run_complete.canceled() )
            {
               std::cout << ">>> ";
               std::string line;
               fc::getline( fc::cin, line );
               auto line_stream = std::make_shared<fc::stringstream>( line );
               buffered_istream bstream(line_stream);
               fc::variant  method;
               fc::variants args;

               method = fc::json::from_stream( bstream );
               while( true )
               {
                  try {
                     args.push_back( fc::json::from_stream( bstream ) );
                  } catch ( const fc::eof_exception& eof ){}
               }
               auto result = receive_call( 0, method.get_string(), args );
               auto itr = _result_formatters.find( method.get_string() );
               if( itr == _result_formatters.end() )
               {
                  std::cout << fc::json::to_pretty_string( result ) << "\n";
               }
               else
                  std::cout << itr->second( result, args ) << "\n";
            }
         }
         std::map<string,std::function<string(variant,const variants&)> > _result_formatters;
         fc::future<void> _run_complete;
   };
} } 
