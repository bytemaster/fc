#include <fc/compress/lzma.hpp>
#include <fc/exception/exception.hpp>

#include <lzma_c.h>

namespace fc {

std::vector<char> lzma_compress(const std::vector<char>& in)
{
  FC_ASSERT(!in.empty());
  
  const unsigned char* in_data = reinterpret_cast<const unsigned char*> (&in[0]);;
  unsigned char* out_data;
  size_t out_len = 0;

  int ret = simpleCompress(elzma_file_format::ELZMA_lzma, in_data, in.size(),
                            &out_data, &out_len);

  if(ret != 0)
  {
    FC_ASSERT(0);
    return std::vector<char>();
  }
  
  std::vector<char> out(out_data, out_data+out_len);
  
  return out;
}

std::vector<char> lzma_decompress( const std::vector<char>& compressed )
{
  FC_ASSERT(!compressed.empty());

  const unsigned char* in_data = reinterpret_cast<const unsigned char*> (&compressed[0]);;
  unsigned char* out_data;
  size_t out_len = 0;


  int ret = simpleDecompress(elzma_file_format::ELZMA_lzma, in_data, compressed.size(),
                              &out_data, &out_len);

  if(ret != 0)
  {
    FC_ASSERT(0);
    return std::vector<char>();
  }
  
  std::vector<char> out(out_data, out_data+out_len);

  return out;
}

} // namespace fc
