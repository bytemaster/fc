#include <fc/network/http/connection.hpp>
#include <fc/network/rate_limiting.hpp>
#include <fc/network/ip.hpp>
#include <fc/time.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>

int main( int argc, char** argv )
{
  fc::rate_limiting_group rate_limiter(1000000,1000000);
  fc::http::connection http_connection;
  rate_limiter.add_tcp_socket(&http_connection.get_socket());
  http_connection.connect_to(fc::ip::endpoint(fc::ip::address("162.243.115.24"),80));
  std::cout << "Starting download...\n";
  fc::time_point start_time(fc::time_point::now());
  fc::http::reply reply = http_connection.request("GET", "http://invictus.io/bin/Keyhotee-0.7.0.dmg");
  fc::time_point end_time(fc::time_point::now());

  std::cout << "HTTP return code: " << reply.status << "\n";
  std::cout << "Retreived " << reply.body.size() << " bytes in " << ((end_time - start_time).count() / fc::milliseconds(1).count()) << "ms\n";
  std::cout << "Average speed " << ((1000 * (uint64_t)reply.body.size()) / ((end_time - start_time).count() / fc::milliseconds(1).count())) << " bytes per second";
  return 0;
}
