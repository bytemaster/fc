#include <fc/network/ntp.hpp>
#include <fc/log/logger.hpp>

int main( int argc, char** argv )
{
   fc::time_point ntp_time = fc::ntp::get_time();
   auto delta = ntp_time - fc::time_point::now();
   auto minutes = delta.count() / 1000000 / 60;
   auto hours = delta.count() / 1000000 / 60 / 60;
   auto seconds = delta.count() / 1000000;
   auto msec= delta.count() / 1000;

   idump( (ntp_time)(delta)(msec)(seconds)(minutes)(hours) );
   return 0;
}
