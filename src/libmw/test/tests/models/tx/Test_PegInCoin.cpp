// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/PegInCoin.h>

BOOST_FIXTURE_TEST_SUITE(TestPegInCoin, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(TxPegInCoin)
{
    uint64_t amount = 123;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    PegInCoin pegInCoin(amount, commit);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = pegInCoin.Serialized();
        BOOST_REQUIRE(serialized.size() == 41);

        Deserializer deserializer(serialized);
        BOOST_REQUIRE(deserializer.Read<uint64_t>() == amount);
        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == commit);

        Deserializer deserializer2(serialized);
        BOOST_REQUIRE(pegInCoin == PegInCoin::Deserialize(deserializer2));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(pegInCoin.GetAmount() == amount);
        BOOST_REQUIRE(pegInCoin.GetCommitment() == commit);
    }
}

BOOST_AUTO_TEST_SUITE_END()