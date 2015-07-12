#define BOOST_TEST_MODULE BlindingTest
#include <boost/test/unit_test.hpp>
#include <fc/array.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/exception/exception.hpp>

// See https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#Test_Vectors

static fc::string TEST1_SEED = "000102030405060708090a0b0c0d0e0f";
static fc::string TEST1_M_PUB = "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8";
static fc::string TEST1_M_PRIV = "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi";
static fc::string TEST1_M_0H_PUB = "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw";
static fc::string TEST1_M_0H_PRIV = "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7";
static fc::string TEST1_M_0H_1_PUB = "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ";
static fc::string TEST1_M_0H_1_PRIV = "xprv9wTYmMFdV23N2TdNG573QoEsfRrWKQgWeibmLntzniatZvR9BmLnvSxqu53Kw1UmYPxLgboyZQaXwTCg8MSY3H2EU4pWcQDnRnrVA1xe8fs";
static fc::string TEST1_M_0H_1_2H_PUB = "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5";
static fc::string TEST1_M_0H_1_2H_PRIV = "xprv9z4pot5VBttmtdRTWfWQmoH1taj2axGVzFqSb8C9xaxKymcFzXBDptWmT7FwuEzG3ryjH4ktypQSAewRiNMjANTtpgP4mLTj34bhnZX7UiM";
static fc::string TEST1_M_0H_1_2H_2_PUB = "xpub6FHa3pjLCk84BayeJxFW2SP4XRrFd1JYnxeLeU8EqN3vDfZmbqBqaGJAyiLjTAwm6ZLRQUMv1ZACTj37sR62cfN7fe5JnJ7dh8zL4fiyLHV";
static fc::string TEST1_M_0H_1_2H_2_PRIV = "xprvA2JDeKCSNNZky6uBCviVfJSKyQ1mDYahRjijr5idH2WwLsEd4Hsb2Tyh8RfQMuPh7f7RtyzTtdrbdqqsunu5Mm3wDvUAKRHSC34sJ7in334";
static fc::string TEST1_M_0H_1_2H_2_1g_PUB = "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy";
static fc::string TEST1_M_0H_1_2H_2_1g_PRIV = "xprvA41z7zogVVwxVSgdKUHDy1SKmdb533PjDz7J6N6mV6uS3ze1ai8FHa8kmHScGpWmj4WggLyQjgPie1rFSruoUihUZREPSL39UNdE3BBDu76";

static fc::string TEST2_SEED = "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542";
static fc::string TEST2_M_PUB = "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB";
static fc::string TEST2_M_PRIV = "xprv9s21ZrQH143K31xYSDQpPDxsXRTUcvj2iNHm5NUtrGiGG5e2DtALGdso3pGz6ssrdK4PFmM8NSpSBHNqPqm55Qn3LqFtT2emdEXVYsCzC2U";
static fc::string TEST2_M_0_PUB = "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH";
static fc::string TEST2_M_0_PRIV = "xprv9vHkqa6EV4sPZHYqZznhT2NPtPCjKuDKGY38FBWLvgaDx45zo9WQRUT3dKYnjwih2yJD9mkrocEZXo1ex8G81dwSM1fwqWpWkeS3v86pgKt";
static fc::string TEST2_M_0_m1_PUB = "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySDhKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a";
static fc::string TEST2_M_0_m1_PRIV = "xprv9wSp6B7kry3Vj9m1zSnLvN3xH8RdsPP1Mh7fAaR7aRLcQMKTR2vidYEeEg2mUCTAwCd6vnxVrcjfy2kRgVsFawNzmjuHc2YmYRmagcEPdU9";
static fc::string TEST2_M_0_m1_1_PUB = "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdMVCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon";
static fc::string TEST2_M_0_m1_1_PRIV = "xprv9zFnWC6h2cLgpmSA46vutJzBcfJ8yaJGg8cX1e5StJh45BBciYTRXSd25UEPVuesF9yog62tGAQtHjXajPPdbRCHuWS6T8XA2ECKADdw4Ef";
static fc::string TEST2_M_0_m1_1_m2_PUB = "xpub6ERApfZwUNrhLCkDtcHTcxd75RbzS1ed54G1LkBUHQVHQKqhMkhgbmJbZRkrgZw4koxb5JaHWkY4ALHY2grBGRjaDMzQLcgJvLJuZZvRcEL";
static fc::string TEST2_M_0_m1_1_m2_PRIV = "xprvA1RpRA33e1JQ7ifknakTFpgNXPmW2YvmhqLQYMmrj4xJXXWYpDPS3xz7iAxn8L39njGVyuoseXzU6rcxFLJ8HFsTjSyQbLYnMpCqE2VbFWc";
static fc::string TEST2_M_0_m1_1_m2_2_PUB = "xpub6FnCn6nSzZAw5Tw7cgR9bi15UV96gLZhjDstkXXxvCLsUXBGXPdSnLFbdpq8p9HmGsApME5hQTZ3emM2rnY5agb9rXpVGyy3bdW6EEgAtqt";
static fc::string TEST2_M_0_m1_1_m2_2_PRIV = "xprvA2nrNbFZABcdryreWet9Ea4LvTJcGsqrMzxHx98MMrotbir7yrKCEXw7nadnHM8Dq38EGfSh6dqA9QWTyefMLEcBYJUuekgW4BYPJcr9E7j";

BOOST_AUTO_TEST_CASE(test_extended_keys_1)
{
    char seed[16];
    fc::from_hex( TEST1_SEED, seed, sizeof(seed) );
    fc::ecc::extended_private_key master = fc::ecc::extended_private_key::generate_master( seed, sizeof(seed) );
    BOOST_CHECK_EQUAL( master.str(), TEST1_M_PRIV );
    BOOST_CHECK_EQUAL( master.get_extended_public_key().str(), TEST1_M_PUB );

    BOOST_CHECK_EQUAL( fc::ecc::extended_private_key::from_base58(TEST1_M_PRIV).str(), TEST1_M_PRIV );
    BOOST_CHECK_EQUAL( fc::ecc::extended_public_key::from_base58(TEST1_M_PUB).str(), TEST1_M_PUB );
    BOOST_CHECK_EQUAL( fc::ecc::extended_private_key::from_base58(TEST1_M_0H_PRIV).str(), TEST1_M_0H_PRIV );
    BOOST_CHECK_EQUAL( fc::ecc::extended_public_key::from_base58(TEST1_M_0H_PUB).str(), TEST1_M_0H_PUB );

    fc::ecc::extended_private_key m_0 = master.derive_child(0x80000000);
    BOOST_CHECK_EQUAL( m_0.str(), TEST1_M_0H_PRIV );
    BOOST_CHECK_EQUAL( m_0.get_extended_public_key().str(), TEST1_M_0H_PUB );

    fc::ecc::extended_private_key m_0_1 = m_0.derive_child(1);
    BOOST_CHECK_EQUAL( m_0_1.str(), TEST1_M_0H_1_PRIV );
    BOOST_CHECK_EQUAL( m_0_1.get_extended_public_key().str(), TEST1_M_0H_1_PUB );
    BOOST_CHECK_EQUAL( m_0.get_extended_public_key().derive_child(1).str(), TEST1_M_0H_1_PUB );

    fc::ecc::extended_private_key m_0_1_2 = m_0_1.derive_child(0x80000002);
    BOOST_CHECK_EQUAL( m_0_1_2.str(), TEST1_M_0H_1_2H_PRIV );
    BOOST_CHECK_EQUAL( m_0_1_2.get_extended_public_key().str(), TEST1_M_0H_1_2H_PUB );

    fc::ecc::extended_private_key m_0_1_2_2 = m_0_1_2.derive_child(2);
    BOOST_CHECK_EQUAL( m_0_1_2_2.str(), TEST1_M_0H_1_2H_2_PRIV );
    BOOST_CHECK_EQUAL( m_0_1_2_2.get_extended_public_key().str(), TEST1_M_0H_1_2H_2_PUB );
    BOOST_CHECK_EQUAL( m_0_1_2.get_extended_public_key().derive_child(2).str(), TEST1_M_0H_1_2H_2_PUB );

    fc::ecc::extended_private_key m_0_1_2_2_1g = m_0_1_2_2.derive_child(1000000000);
    BOOST_CHECK_EQUAL( m_0_1_2_2_1g.str(), TEST1_M_0H_1_2H_2_1g_PRIV );
    BOOST_CHECK_EQUAL( m_0_1_2_2_1g.get_extended_public_key().str(), TEST1_M_0H_1_2H_2_1g_PUB );
    BOOST_CHECK_EQUAL( m_0_1_2_2.get_extended_public_key().derive_child(1000000000).str(), TEST1_M_0H_1_2H_2_1g_PUB );
}

BOOST_AUTO_TEST_CASE(test_extended_keys_2)
{
    char seed[64];
    fc::from_hex( TEST2_SEED, seed, sizeof(seed) );
    fc::ecc::extended_private_key master = fc::ecc::extended_private_key::generate_master( seed, sizeof(seed) );
    BOOST_CHECK_EQUAL( master.str(), TEST2_M_PRIV );
    BOOST_CHECK_EQUAL( master.get_extended_public_key().str(), TEST2_M_PUB );

    fc::ecc::extended_private_key m_0 = master.derive_child(0);
    BOOST_CHECK_EQUAL( m_0.str(), TEST2_M_0_PRIV );
    BOOST_CHECK_EQUAL( m_0.get_extended_public_key().str(), TEST2_M_0_PUB );
    BOOST_CHECK_EQUAL( master.get_extended_public_key().derive_child(0).str(), TEST2_M_0_PUB );

    fc::ecc::extended_private_key m_0_m1 = m_0.derive_child(-1);
    BOOST_CHECK_EQUAL( m_0_m1.str(), TEST2_M_0_m1_PRIV );
    BOOST_CHECK_EQUAL( m_0_m1.get_extended_public_key().str(), TEST2_M_0_m1_PUB );

    fc::ecc::extended_private_key m_0_m1_1 = m_0_m1.derive_child(1);
    BOOST_CHECK_EQUAL( m_0_m1_1.str(), TEST2_M_0_m1_1_PRIV );
    BOOST_CHECK_EQUAL( m_0_m1_1.get_extended_public_key().str(), TEST2_M_0_m1_1_PUB );
    BOOST_CHECK_EQUAL( m_0_m1.get_extended_public_key().derive_child(1).str(), TEST2_M_0_m1_1_PUB );

    fc::ecc::extended_private_key m_0_m1_1_m2 = m_0_m1_1.derive_child(-2);
    BOOST_CHECK_EQUAL( m_0_m1_1_m2.str(), TEST2_M_0_m1_1_m2_PRIV );
    BOOST_CHECK_EQUAL( m_0_m1_1_m2.get_extended_public_key().str(), TEST2_M_0_m1_1_m2_PUB );

    fc::ecc::extended_private_key m_0_m1_1_m2_2 = m_0_m1_1_m2.derive_child(2);
    BOOST_CHECK_EQUAL( m_0_m1_1_m2_2.str(), TEST2_M_0_m1_1_m2_2_PRIV );
    BOOST_CHECK_EQUAL( m_0_m1_1_m2_2.get_extended_public_key().str(), TEST2_M_0_m1_1_m2_2_PUB );
    BOOST_CHECK_EQUAL( m_0_m1_1_m2.get_extended_public_key().derive_child(2).str(), TEST2_M_0_m1_1_m2_2_PUB );
}

BOOST_AUTO_TEST_CASE(test_blinding)
{
    char buffer[7] = "test_";
    fc::ecc::extended_private_key alice = fc::ecc::extended_private_key::generate_master( "master" );
    fc::ecc::extended_private_key bob = fc::ecc::extended_private_key::generate_master( "puppet" );

    for ( int i = 0; i < 10; i++ )
    {
        alice = alice.derive_child( i );
        bob = bob.derive_child( i | 0x80000000 );
        buffer[6] = '0' + i;
        fc::ecc::extended_public_key bob_pub = bob.get_extended_public_key();
        fc::sha256 hash = fc::sha256::hash( buffer, sizeof(buffer) );
        fc::ecc::public_key t = alice.blind_public_key( bob_pub, i );
        fc::ecc::blinded_hash blinded = alice.blind_hash( hash, i );
        fc::ecc::blind_signature blind_sig = bob.blind_sign( blinded, i );
        fc::ecc::compact_signature sig = alice.unblind_signature( bob_pub, blind_sig, i );
        try {
        fc::ecc::public_key validate( sig, hash );
//        BOOST_CHECK_EQUAL( validate.serialize(), t.serialize() );
        } catch (const fc::exception& e) {
            printf( "Test %d: %s\n", i, e.to_string().c_str() );
        }
    }
}
