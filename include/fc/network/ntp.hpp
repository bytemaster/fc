#pragma once
#include <string>
#include <fc/time.hpp>


namespace fc {

   class ntp 
   {
      public:
         static void set_server( const std::string& hostname, uint16_t port = 123 );
         static fc::time_point get_time();
   };

} // namespace fc
