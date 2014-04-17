#pragma once
#include <fc/utility.hpp>
#include <fc/fwd.hpp>
#include <fc/io/iostream.hpp>
#include <fc/time.hpp>

namespace fc 
{
  namespace detail
  {
    class rate_limiting_group_impl;
  }

  class rate_limiting_group 
  {
  public:
    rate_limiting_group(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second);
    ~rate_limiting_group();

  private:
    std::unique_ptr<detail::rate_limiting_group_impl> my;
  };
  typedef std::shared_ptr<rate_limiting_group> rate_limiting_group_ptr;

} // namesapce fc

