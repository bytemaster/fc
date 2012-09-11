#ifndef _FC_DH_HPP_
#define _FC_DH_HPP_
#include <fc/vector.hpp>
#include <stdint.h>

namespace fc {

    struct diffie_hellman {
        diffie_hellman():valid(0),g(5){}
        bool generate_params( int s, uint8_t g );
        bool generate_pub_key();
        bool compute_shared_key( const char* buf, uint32_t s );
        bool compute_shared_key( const fc::vector<char>& pubk);
        bool validate();

        fc::vector<char> p;
        fc::vector<char> pub_key;
        fc::vector<char> priv_key;
        fc::vector<char> shared_key;
        bool             valid;
        uint8_t          g; 
    };

} // namespace fc


#endif
