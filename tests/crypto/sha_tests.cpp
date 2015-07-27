#include <boost/test/unit_test.hpp>

#include <fc/crypto/sha1.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/exception/exception.hpp>

#include <iostream>

// SHA test vectors taken from http://www.di-mgt.com.au/sha_testvectors.html
static const std::string TEST1("abc");
static const std::string TEST2("");
static const std::string TEST3("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
static const std::string TEST4("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
static char TEST5[1000001];

static void init_5() {
    memset( TEST5, 'a', sizeof(TEST5) - 1 );
    TEST5[1000000] = 0;
}

template<typename H>
void test( const char* to_hash, const std::string& expected ) {
    H hash = H::hash( to_hash, strlen( to_hash ) );
    BOOST_CHECK_EQUAL( expected, (std::string) hash );
    H hash2( expected );
    BOOST_CHECK( hash == hash2 );
}

template<typename H>
void test( const std::string& to_hash, const std::string& expected ) {
    test<H>( to_hash.c_str(), expected );
}

template void test<fc::sha1>( const std::string& test, const std::string& expected );
template void test<fc::sha224>( const std::string& test, const std::string& expected );
template void test<fc::sha256>( const std::string& test, const std::string& expected );
template void test<fc::sha512>( const std::string& test, const std::string& expected );

BOOST_AUTO_TEST_SUITE(fc_crypto)

BOOST_AUTO_TEST_CASE(sha1_test)
{
    init_5();
    test<fc::sha1>( TEST1, "a9993e364706816aba3e25717850c26c9cd0d89d" );
    test<fc::sha1>( TEST2, "da39a3ee5e6b4b0d3255bfef95601890afd80709" );
    test<fc::sha1>( TEST3, "84983e441c3bd26ebaae4aa1f95129e5e54670f1" );
    test<fc::sha1>( TEST4, "a49b2446a02c645bf419f995b67091253a04a259" );
    test<fc::sha1>( TEST5, "34aa973cd4c4daa4f61eeb2bdbad27316534016f" );
}

BOOST_AUTO_TEST_CASE(sha224_test)
{
    init_5();
    test<fc::sha224>( TEST1, "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7" );
    test<fc::sha224>( TEST2, "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f" );
    test<fc::sha224>( TEST3, "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525" );
    test<fc::sha224>( TEST4, "c97ca9a559850ce97a04a96def6d99a9e0e0e2ab14e6b8df265fc0b3" );
    test<fc::sha224>( TEST5, "20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67" );
}

BOOST_AUTO_TEST_CASE(sha256_test)
{
    init_5();
    test<fc::sha256>( TEST1, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
    test<fc::sha256>( TEST2, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );
    test<fc::sha256>( TEST3, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" );
    test<fc::sha256>( TEST4, "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1" );
    test<fc::sha256>( TEST5, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0" );
}

BOOST_AUTO_TEST_CASE(sha512_test)
{
    init_5();
    test<fc::sha512>( TEST1, "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
                             "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f" );
    test<fc::sha512>( TEST2, "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce"
                             "47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e" );
    test<fc::sha512>( TEST3, "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c335"
                             "96fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445" );
    test<fc::sha512>( TEST4, "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
                             "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909" );
    test<fc::sha512>( TEST5, "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
                             "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b" );
}

BOOST_AUTO_TEST_SUITE_END()
