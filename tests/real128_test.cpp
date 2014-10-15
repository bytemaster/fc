#include <fc/real128.hpp>
#define BOOST_TEST_MODULE Real128Test
#include <boost/test/unit_test.hpp>

using fc::real128;
using std::string;

BOOST_AUTO_TEST_CASE(real128_test)
{
   BOOST_CHECK_EQUAL(string(real128(0)), string("0."));
   BOOST_CHECK_EQUAL(real128(8).to_uint64(), 8);
   BOOST_CHECK_EQUAL(real128(6789).to_uint64(), 6789);
   BOOST_CHECK_EQUAL(real128(10000).to_uint64(), 10000);
   BOOST_CHECK_EQUAL(string(real128(1)), string("1."));
   BOOST_CHECK_EQUAL(string(real128(5)), string("5."));
   BOOST_CHECK_EQUAL(string(real128(12345)), string("12345."));
   BOOST_CHECK_EQUAL(string(real128(0)), string(real128("0")));

   BOOST_CHECK_EQUAL(real128("12345.6789").to_uint64(), 12345);
   BOOST_CHECK_EQUAL((real128("12345.6789")*10000).to_uint64(), 123456789);
   BOOST_CHECK_EQUAL(string(real128("12345.6789")), string("12345.6789"));
}
