#!/bin/sh

#TIME=time

cd "`dirname $0`"/..

echo Building ecc_test with openssl...
(
cmake -D ECC_IMPL=openssl .
make ecc_test
mv ecc_test ecc_test.openssl
) >/dev/null 2>&1

echo Building ecc_test with secp256k1...
(
cmake -D ECC_IMPL=secp256k1 .
make ecc_test
mv ecc_test ecc_test.secp256k1
) >/dev/null 2>&1

run () {
    echo "Running ecc_test.$1 test ecc.interop.$1 ..."
    $TIME "./ecc_test.$1" test "ecc.interop.$1"
}

run openssl
run openssl
run secp256k1
run secp256k1
run secp256k1
run openssl

echo Done.

rm -f ecc_test.openssl ecc_test.secp256k1 ecc.interop.openssl ecc.interop.secp256k1
