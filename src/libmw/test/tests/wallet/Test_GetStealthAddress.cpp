// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/wallet/Keychain.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestGetStealthAddress, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(GetStealthAddress)
{
    SecretKey a = BigInt<32>::FromHex("6a94ffabf7cb6c56b7f1bda50158019a49d101e270ad202e04912b2f001ece79");
    SecretKey b = BigInt<32>::FromHex("da5b685cbcdad4aabcec8f58253f8f4aa89a116659b1e5a8e3c407cc09c19738");

    StealthAddress address_100 = mw::Keychain(a, b, 50).GetStealthAddress(100);
    BOOST_REQUIRE_EQUAL(address_100.A().ToHex(), "03186fefb89b5bdfe2a466058b02aa1cceff5a66225b34ac7c79893cbf2772ccfe");
    BOOST_REQUIRE_EQUAL(address_100.B().ToHex(), "0335be5bf363be2cc892fc868135b2f8f93a867695383cff45b89e9aeee6df5869");
}

BOOST_AUTO_TEST_SUITE_END()