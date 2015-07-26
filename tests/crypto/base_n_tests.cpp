#include <boost/test/unit_test.hpp>

#include <fc/crypto/base32.hpp>
#include <fc/crypto/base36.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/base64.hpp>

#include <iostream>

static const std::string TEST1("");
static const std::string TEST2("\0\00101", 4);
static const std::string TEST3("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

static void test_32( const std::string& test, const std::string& expected )
{
    std::vector<char> vec( test.begin(), test.end() );
    fc::string enc1 = fc::to_base32( vec );
    fc::string enc2 = fc::to_base32( test.c_str(), test.size() );
    BOOST_CHECK_EQUAL( enc1, enc2 );
    BOOST_CHECK_EQUAL( expected, enc2 );

    std::vector<char> dec = fc::from_base32( enc1 );
    BOOST_CHECK_EQUAL( vec.size(), dec.size() );
    BOOST_CHECK( !memcmp( vec.data(), dec.data(), vec.size() ) );
}

BOOST_AUTO_TEST_SUITE(fc_crypto)

BOOST_AUTO_TEST_CASE(base32_test)
{
    test_32( TEST1, "" );
    test_32( TEST2, "AAATAMI=" );
    test_32( TEST3, "IFBEGRCFIZDUQSKKJNGE2TSPKBIVEU2UKVLFOWCZLI======" );
}


static void test_36( const std::string& test, const std::string& expected )
{
    std::vector<char> vec( test.begin(), test.end() );
    fc::string enc1 = fc::to_base36( vec );
    fc::string enc2 = fc::to_base36( test.c_str(), test.size() );
    BOOST_CHECK_EQUAL( enc1, enc2 );
    BOOST_CHECK_EQUAL( expected, enc2 );

    std::vector<char> dec = fc::from_base36( enc1 );
    BOOST_CHECK_EQUAL( vec.size(), dec.size() );
    BOOST_CHECK( !memcmp( vec.data(), dec.data(), vec.size() ) );
}

BOOST_AUTO_TEST_CASE(base36_test)
{
    test_36( TEST1, "" );
    test_36( TEST2, "01o35" );
    test_36( TEST3, "l4ksdleyi5pnl0un5raue268ptj43dwjwmz15ie2" );
}


static void test_58( const std::string& test, const std::string& expected )
{
    std::vector<char> vec( test.begin(), test.end() );
    fc::string enc1 = fc::to_base58( vec );
    fc::string enc2 = fc::to_base58( test.c_str(), test.size() );
    BOOST_CHECK_EQUAL( enc1, enc2 );
    BOOST_CHECK_EQUAL( expected, enc2 );

    std::vector<char> dec = fc::from_base58( enc1 );
    BOOST_CHECK_EQUAL( vec.size(), dec.size() );
    BOOST_CHECK( !memcmp( vec.data(), dec.data(), vec.size() ) );

    char buffer[64];
    size_t len = fc::from_base58( enc1, buffer, 16 );
    BOOST_CHECK( len <= 16 );
    BOOST_CHECK( !memcmp( vec.data(), buffer, len ) );

}

BOOST_AUTO_TEST_CASE(base58_test)
{
    test_58( TEST1, "" );
    test_58( TEST2, "1Q9e" );
    test_58( TEST3, "2zuFXTJSTRK6ESktqhM2QDBkCnH1U46CnxaD" );
}


static void test_64( const std::string& test, const std::string& expected )
{
    fc::string enc1 = fc::base64_encode( test );
    fc::string enc2 = fc::base64_encode( test.c_str(), test.size() );
    BOOST_CHECK_EQUAL( enc1, enc2 );
    BOOST_CHECK_EQUAL( expected, enc2 );

    std::string dec = fc::base64_decode( enc1 );
    BOOST_CHECK_EQUAL( test.size(), dec.size() );
    BOOST_CHECK_EQUAL( test, dec );
}

BOOST_AUTO_TEST_CASE(base64_test)
{
    test_64( TEST1, "" );
    test_64( TEST2, "AAEwMQ==" );
    test_64( TEST3, "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=" );
}


BOOST_AUTO_TEST_SUITE_END()
