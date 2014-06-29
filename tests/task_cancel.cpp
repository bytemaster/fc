#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>
#include <fc/exception/exception.hpp>

int main( int argc, char** argv )
{
   try {
       auto result = fc::schedule( [=]() { ilog( "hello world" ); }, fc::time_point::now() + fc::seconds(3) );
       result.cancel();
       result.wait();
   } 
   catch ( const fc::exception& e )
   {
      wlog( "${e}", ("e",e.to_detail_string() ) );
   }
}
