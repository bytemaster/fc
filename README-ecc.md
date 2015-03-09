ECC Support
===========

include/fc/crypto/elliptic.hpp defines an interface for some cryptographic
wrapper classes handling elliptic curve cryptography.

Two implementations of this interface exist. One is based on OpenSSL, the
other is based on libsecp256k1 (see https://github.com/bitcoin/secp256k1 ).
The implementation to be used is selected at compile time using the
cmake variable "ECC_IMPL". It can take two values, openssl or secp256k1 .
The default is "openssl". The alternative can be configured when invoking
cmake, for example

cmake -D ECC_IMPL=secp256k1 .

If secp256k1 is chosen, the secp256k1 library and its include file must
already be installed in the appropriate library / include directories on
your system.


Testing
-------

Type "make ecc_test" to build the ecc_test executable from tests/ecc_test.cpp
with the currently configured ECC implementation.

ecc_test expects two arguments:

ecc_test <pass> <interop-file>

<pass> is a somewhat arbitrary password used for testing.

<interop-file> is a data file containing intermediate test results.
If the file does not exist, it will be created and intermediate results from
the current ECC backend are written to it.
If the file does exist, intermediate results from the current ECC backend
are compared with the file contents.

For a full round of interoperability testing, you need to do this:

1. Build ecc_test with openssl backend.
2. Run "ecc_test test ecc.interop.openssl".
3. Run "ecc_test test ecc.interop.openssl" again, testing openssl against
   itself.
4. Build ecc_test with secp256k1 backend.
5. Run "ecc_test test ecc.interop.secp256k1".
6. Run "ecc_test test ecc.interop.secp256k1" again, testing secp256k1 against
   itself.
7. Run "ecc_test test ecc.interop.openssl", testing secp256k1 against openssl.
8. Build ecc_test with openssl backend.
9. Run "ecc_test test ecc.interop.secp256k1", testing openssl against secp256k1.

None of the test runs should produce any output. The above steps are scripted
in tests//ecc-interop.sh .

