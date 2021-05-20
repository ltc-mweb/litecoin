// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Input.h>

BOOST_FIXTURE_TEST_SUITE(TestInput, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(PlainTxInput)
{
    Commitment commit(Random::CSPRNG<33>().GetBigInt());
    PublicKey pubkey(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Input input(commit, pubkey, signature);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = input.Serialized();

        Deserializer deserializer(serialized);
        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == commit);
        BOOST_REQUIRE(PublicKey::Deserialize(deserializer) == pubkey);
        BOOST_REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        BOOST_REQUIRE(input == Input::Deserialize(deserializer2));

        BOOST_REQUIRE(input.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(input.GetCommitment() == commit);
        BOOST_REQUIRE(input.GetSignature() == signature);
    }
}

BOOST_AUTO_TEST_SUITE_END()