// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Output.h>

#include <test_framework/Deserializer.h>
#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestOutputId, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(TxOutputIdentifier)
{
    Commitment commit = Commitment::Random();
    PublicKey receiverPubKey = PublicKey::Random();
    PublicKey exchangePubKey = PublicKey::Random();
    uint8_t viewTag = 100;
    uint64_t maskedValue = 123456789;
    BigInt<16> maskedNonce = secret_key_t<16>::Random().GetBigInt();
    PublicKey senderPubKey = PublicKey::Random();

    OutputMessage message(
        receiverPubKey,
        exchangePubKey,
        viewTag,
        maskedValue,
        maskedNonce,
        senderPubKey
    );
    OutputId outputId(commit, message);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = outputId.Serialized();

        Deserializer deserializer(serialized);
        BOOST_REQUIRE(deserializer.Read<Commitment>() == commit);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == receiverPubKey);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == exchangePubKey);
        BOOST_REQUIRE(deserializer.Read<uint8_t>() == viewTag);
        BOOST_REQUIRE(deserializer.Read<uint64_t>() == maskedValue);
        BOOST_REQUIRE(deserializer.Read<BigInt<16>>() == maskedNonce);
        BOOST_REQUIRE(deserializer.Read<PublicKey>() == senderPubKey);

        BOOST_REQUIRE(outputId == OutputId::Deserialize(serialized));

        BOOST_REQUIRE(outputId.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(outputId.GetCommitment() == commit);
    }
}

BOOST_AUTO_TEST_SUITE_END()