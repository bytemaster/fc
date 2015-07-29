#include <boost/endian/conversion.hpp>
#include <fc/exception/exception.hpp>
#include "_digest_common.hpp"

namespace fc { namespace detail {
    static void shift_l( const uint32_t* in, uint32_t* out, std::size_t n, int i) {
        if (i < n) {
            memcpy( out, in + i, 4*(n-i) );
        } else {
            i = n;
        }
        memset( out + (n-i), 0, 4 * i );
    }

    void shift_l( const char* in, char* out, std::size_t n, int i) {
        FC_ASSERT( (n & 3) == 0 ); // all hashes are a multiple of 32 bit
        n >>= 2;
        const uint32_t* in32 = (uint32_t*) in;
        uint32_t* out32 = (uint32_t*) out;

        if (i >= 32) {
            shift_l( in32, out32, n, i >> 5 );
            i &= 0x1f;
            in32 = out32;
        }

        std::size_t p;
        for( p = 0; p < n-1; ++p )
            out32[p] = boost::endian::native_to_big(boost::endian::big_to_native(in32[p]) << i
                                                    | (boost::endian::big_to_native(in32[p+1])>>(32-i)));
        out32[p] = boost::endian::native_to_big(boost::endian::big_to_native(in32[p]) << i);
    }
}}
