#pragma once
#include <vector>

namespace fc {

  std::vector<char> lzma_compress( const std::vector<char>& in );
  std::vector<char> lzma_decompress( const std::vector<char>& compressed );

} // namespace fc
