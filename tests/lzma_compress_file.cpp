#include <fc/compress/lzma.hpp>
#include <fc/filesystem.hpp>

#include <iostream>
#include <string>

using namespace fc;

int main( int argc, char** argv )
{
    if( argc != 2 && argc != 3 )
    {
        std::cout << "usage: " << argv[0] << " <src_path> [dst_path = src_path.lzma]\n";
        exit( -1 );
    }

    auto src = std::string( argv[1] );
    auto dst = (argc == 3) ? std::string( argv[2] ) : src + ".lzma";

    lzma_compress_file( src, dst );

    return 0;
}
