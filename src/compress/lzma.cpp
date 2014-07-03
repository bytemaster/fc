#include <fc/compress/lzma.hpp>
#include <fc/exception/exception.hpp>

#include <lzma_c.h>

#include <boost/iostreams/device/mapped_file.hpp>

#include <fstream>



#include <iostream>

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

struct lzma_file_ctx
{
    const unsigned char* src_buf;
    size_t src_len;

    path dst_path;
};

static int input_callback( void* input_ctx, void* input_buf, size_t* input_len )
{
    FC_ASSERT( input_ctx != NULL );
    FC_ASSERT( input_buf != NULL );

    const auto ctx = ( struct lzma_file_ctx* )input_ctx;
    const auto size = ( ctx->src_len < *input_len ) ? ctx->src_len : *input_len;

    if( size > 0 )
    {
        memcpy( input_buf, ( void * )ctx->src_buf, size );
        ctx->src_buf += size;
        ctx->src_len -= size;
    }

    *input_len = size;

    return 0;
}

static size_t output_callback( void* output_ctx, const void* output_buf, size_t output_len )
{
    FC_ASSERT( output_ctx != NULL );
    FC_ASSERT( output_buf != NULL );

    const auto ctx = ( struct lzma_file_ctx* )output_ctx;

    if( output_len > 0 )
    {
        size_t dst_len = 0;
        if( !exists( ctx->dst_path ) )
        {
            auto fs = std::ofstream( ctx->dst_path.string() );
            fs.close();
        }
        else
        {
            dst_len = file_size( ctx->dst_path );
        }

        resize_file( ctx->dst_path, dst_len + output_len );

        boost::iostreams::mapped_file_sink dst_file;
        dst_file.open( ctx->dst_path.string() );
        FC_ASSERT( dst_file.is_open() );

        memcpy( ( void* )(dst_file.data() + dst_len), output_buf, output_len);

        dst_file.close();
    }

    return output_len;
}

void lzma_compress_file( path src_path,
                         path dst_path,
                         unsigned char level,
                         unsigned int dict_size )
{
    FC_ASSERT( exists( src_path ) );
    FC_ASSERT( !exists( dst_path ) );

    boost::iostreams::mapped_file_source src_file;
    src_file.open( src_path.string() );
    FC_ASSERT( src_file.is_open() );

    elzma_compress_handle handle = NULL;
    handle = elzma_compress_alloc();
    FC_ASSERT( handle != NULL );

    struct lzma_file_ctx ctx;
    ctx.src_buf = ( const unsigned char* )src_file.data();
    ctx.src_len = src_file.size();
    ctx.dst_path = dst_path;

    auto rc = elzma_compress_config( handle,
                                     ELZMA_LC_DEFAULT,
                                     ELZMA_LP_DEFAULT,
                                     ELZMA_PB_DEFAULT,
                                     level,
                                     dict_size,
                                     elzma_file_format::ELZMA_lzma,
                                     ctx.src_len );

    try
    {
        FC_ASSERT( rc == ELZMA_E_OK );
    }
    catch( ... )
    {
        elzma_compress_free( &handle );
        throw;
    }

    rc = elzma_compress_run( handle,
                             input_callback,
                             ( void * )&ctx,
                             output_callback,
                             ( void * )&ctx,
                             NULL,
                             NULL );

    elzma_compress_free( &handle );
    FC_ASSERT( rc == ELZMA_E_OK );


    /* TEST */
    FC_ASSERT( exists( dst_path ) );

    boost::iostreams::mapped_file_source dst_file;
    dst_file.open( dst_path.string() );
    FC_ASSERT( dst_file.is_open() );

    std::vector<char> result( dst_file.data(), dst_file.data() + dst_file.size() );
    dst_file.close();

    for( const auto& c : result )
    {
        std::cout << c;
    }
    std::cout << "\n";

    result = lzma_decompress( result );
    for( const auto& c : result )
    {
        std::cout << c;
    }
}

} // namespace fc
