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
    CAmount amount = 123;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    PegInCoin pegInCoin(amount, commit);

    //
    // Serialization
    //
    {
        PegInCoin pegInCoin2;
        CDataStream(pegInCoin.Serialized(), SER_DISK, PROTOCOL_VERSION) >> pegInCoin2;
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