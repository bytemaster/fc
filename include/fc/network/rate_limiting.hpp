#pragma once
#include <stdint.h>

#include <memory>

namespace fc 
{
  namespace detail
  {
    class rate_limiting_group_impl;
  }

  class tcp_socket;

  class rate_limiting_group 
  {
  public:
    rate_limiting_group(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second);
    ~rate_limiting_group();

    void set_upload_limit(uint32_t upload_bytes_per_second);
    uint32_t get_upload_limit() const;

    void set_download_limit(uint32_t download_bytes_per_second);
    uint32_t get_download_limit() const;

    void add_tcp_socket(tcp_socket* tcp_socket_to_limit);
    void remove_tcp_socket(tcp_socket* tcp_socket_to_stop_limiting);
  private:
    std::unique_ptr<detail::rate_limiting_group_impl> my;
  };
  typedef std::shared_ptr<rate_limiting_group> rate_limiting_group_ptr;

} // namesapce fc

