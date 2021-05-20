// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/OutputId.h>

BOOST_FIXTURE_TEST_SUITE(TestOutputId, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(TxOutputIdentifier)
{
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT;
    PublicKey receiverPubKey = Random::CSPRNG<33>().GetBigInt();
    PublicKey exchangePubKey = Random::CSPRNG<33>().GetBigInt();
    uint8_t viewTag = 100;
    uint64_t maskedValue = 123456789;
    BigInt<16> maskedNonce = Random::CSPRNG<16>().GetBigInt();
    PublicKey senderPubKey = Random::CSPRNG<33>().GetBigInt();
    OutputId outputId(
        commit,
        features,
        receiverPubKey,
        exchangePubKey,
        viewTag,
        maskedValue,
        maskedNonce,
        senderPubKey
    );

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = outputId.Serialized();
        BOOST_REQUIRE(serialized.size() == 158);

        Deserializer deserializer(serialized);
        BOOST_REQUIRE(deserializer.Read<Commitment>() == commit);
        BOOST_REQUIRE(deserializer.Read<uint8_t>() == features);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == receiverPubKey);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == exchangePubKey);
        BOOST_REQUIRE(deserializer.Read<uint8_t>() == viewTag);
        BOOST_REQUIRE(deserializer.Read<uint64_t>() == maskedValue);
        BOOST_REQUIRE(deserializer.Read<BigInt<16>>() == maskedNonce);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == senderPubKey);

        Deserializer deserializer2(serialized);
        BOOST_REQUIRE(outputId == OutputId::Deserialize(deserializer2));

        BOOST_REQUIRE(outputId.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(!outputId.IsPeggedIn());
        //REQUIRE(outputId.GetFeatures() == features);
        BOOST_REQUIRE(outputId.GetCommitment() == commit);
    }
}

BOOST_AUTO_TEST_SUITE_END()