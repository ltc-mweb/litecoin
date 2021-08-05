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
    BOOST_CHECK_EQUAL(address_100.A().ToHex(), "0378091104a43ec19c2196b8fc128a084a395c62a3d591166c8e8fefc3b04a0f0d");
    BOOST_CHECK_EQUAL(address_100.B().ToHex(), "0266a008e6831e31822d3786e54c89174b3adfda827a93da0123eab93cd99fd2a6");
}

BOOST_AUTO_TEST_SUITE_END()