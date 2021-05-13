// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/wallet/Keychain.h>
#include <libmw/libmw.h>

BOOST_FIXTURE_TEST_SUITE(TestGetStealthAddress, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(GetStealthAddress)
{
    SecretKey a = BigInt<32>::FromHex("6a94ffabf7cb6c56b7f1bda50158019a49d101e270ad202e04912b2f001ece79");
    SecretKey b = BigInt<32>::FromHex("da5b685cbcdad4aabcec8f58253f8f4aa89a116659b1e5a8e3c407cc09c19738");

    StealthAddress address_100 = mw::Keychain(a, b, 50).GetStealthAddress(100);
    BOOST_REQUIRE(address_100.A().Format() == "02676d6f55a58297072c602b9e3c98bdcf7f2632c4c88686bd8e8cf3f12d394111");
    BOOST_REQUIRE(address_100.B().Format() == "03e7421b2c09154a4201072c733929dfc25262dd79146f1164b4b000c4f5533e01");
}

BOOST_AUTO_TEST_SUITE_END()