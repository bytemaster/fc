#include <fc/network/http/connection.hpp>
#include <fc/network/rate_limiting.hpp>
#include <fc/network/ip.hpp>
#include <fc/time.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>
fc::rate_limiting_group rate_limiter(1000000, 1000000);

void download_url(const std::string& ip_address, const std::string& url)
{
  fc::http::connection http_connection;
  rate_limiter.add_tcp_socket(&http_connection.get_socket());
  http_connection.connect_to(fc::ip::endpoint(fc::ip::address(ip_address.c_str()), 80));
  std::cout << "Starting download...\n";
  fc::time_point start_time(fc::time_point::now());
  fc::http::reply reply = http_connection.request("GET", "http://mirror.cs.vt.edu/pub/cygwin/glibc/releases/glibc-2.9.tar.gz");
  fc::time_point end_time(fc::time_point::now());

  std::cout << "HTTP return code: " << reply.status << "\n";
  std::cout << "Retreived " << reply.body.size() << " bytes in " << ((end_time - start_time).count() / fc::milliseconds(1).count()) << "ms\n";
  std::cout << "Average speed " << ((1000 * (uint64_t)reply.body.size()) / ((end_time - start_time).count() / fc::milliseconds(1).count())) << " bytes per second";
}

int main( int argc, char** argv )
{
  std::vector<fc::future<void> > download_futures;
  download_futures.push_back(fc::async([](){ download_url("198.82.184.145", "http://mirror.cs.vt.edu/pub/cygwin/glibc/releases/glibc-2.9.tar.gz"); }));
  download_futures.push_back(fc::async([](){ download_url("198.82.184.145", "http://mirror.cs.vt.edu/pub/cygwin/glibc/releases/glibc-2.7.tar.gz"); }));

  for (int i = 0; i < download_futures.size(); ++i)
    download_futures[i].wait();
  return 0;
}
