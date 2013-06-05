#include <fc/crypto/base32.hpp>
#include <CyoDecode.h>
#include <CyoEncode.h>
namespace fc
{
    fc::vector<char> from_base32( const fc::string& b32 )
    {
       auto len = cyoBase32DecodeGetLength( b32.size() );
       fc::vector<char> v(len);
       cyoBase32Decode( v.data(), b32.c_str(), b32.size() );
       return v;
    }

    fc::string to_base32( const char* data, size_t len )
    { 
       auto s = cyoBase16EncodeGetLength(len);
       fc::string b32;
       b32.resize(s);
       cyoBase16Encode( b32.data(), data, len );
       b32.resize( b32.size()-1); // strip the nullterm
       return b32;
    }

    fc::string to_base32( const fc::vector<char>& vec )
    {
       return to_base32( vec.data(), vec.size() );
    }
}
