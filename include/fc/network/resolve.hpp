#pragma once
#include <fc/vector.hpp>
#include <fc/network/ip.hpp>

namespace fc
{
  fc::vector<fc::ip::endpoint> resolve( const fc::string& host, uint16_t port );
}
