#include <boost/asio.hpp>

namespace fc
{
  class tcp_socket_io_hooks
  {
  public:
    virtual ~tcp_socket_io_hooks() {}
    virtual size_t readsome(boost::asio::ip::tcp::socket& socket, char* buffer, size_t length) = 0;
    virtual size_t writesome(boost::asio::ip::tcp::socket& socket, const char* buffer, size_t length) = 0;
  };
} // namesapce fc
