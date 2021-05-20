// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/PegOutCoin.h>

BOOST_FIXTURE_TEST_SUITE(TestPegOutCoin, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(TxPegOutCoin)
{
    uint64_t amount = 123;
    std::vector<uint8_t> scriptPubKey = Random::CSPRNG<32>().vec();
    PegOutCoin pegOutCoin(amount, scriptPubKey);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = pegOutCoin.Serialized();

        Deserializer deserializer(serialized);
        BOOST_REQUIRE(deserializer.Read<uint64_t>() == amount);
        BOOST_REQUIRE(deserializer.Read<uint8_t>() == 32);
        BOOST_REQUIRE(deserializer.ReadVector(32) == scriptPubKey);

        Deserializer deserializer2(serialized);
        BOOST_REQUIRE(pegOutCoin == PegOutCoin::Deserialize(deserializer2));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(pegOutCoin.GetAmount() == amount);
        BOOST_REQUIRE(pegOutCoin.GetScriptPubKey() == scriptPubKey);
    }
}

BOOST_AUTO_TEST_SUITE_END()