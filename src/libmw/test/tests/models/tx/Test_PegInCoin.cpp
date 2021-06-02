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

        CDataStream deserializer(serialized, SER_DISK, PROTOCOL_VERSION);

        uint64_t amount2;
        deserializer >> amount2;
        BOOST_REQUIRE(amount2 == amount);

        Commitment commit2;
        deserializer >> commit2;
        BOOST_REQUIRE(commit2 == commit);

        PegInCoin pegInCoin2;
        CDataStream(serialized, SER_DISK, PROTOCOL_VERSION) >> pegInCoin2;
        BOOST_REQUIRE(pegInCoin == pegInCoin2);
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