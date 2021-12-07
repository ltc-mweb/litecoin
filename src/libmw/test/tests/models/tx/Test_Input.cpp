// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Input.h>
#include <mw/models/crypto/SecretKey.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestInput, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(PlainTxInput)
{
    Commitment commit = Commitment::Random();
    PublicKey input_pubkey = PublicKey::Random();
    PublicKey output_pubkey = PublicKey::Random();
    Signature signature(SecretKey64::Random().GetBigInt());
    Input input(commit, input_pubkey, output_pubkey, signature);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = input.Serialized();

        CDataStream deserializer(serialized, SER_DISK, PROTOCOL_VERSION);
        Commitment commit2;
        deserializer >> commit2;
        BOOST_REQUIRE(commit2 == commit);

        PublicKey input_pubkey2;
        deserializer >> input_pubkey2;
        BOOST_REQUIRE(input_pubkey2 == input_pubkey);

        PublicKey output_pubkey2;
        deserializer >> output_pubkey2;
        BOOST_REQUIRE(output_pubkey2 == output_pubkey);

        Signature signature2;
        deserializer >> signature2;
        BOOST_REQUIRE(signature2 == signature);

        BOOST_REQUIRE(input == Input::Deserialize(serialized));

        BOOST_REQUIRE(input.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(input.GetCommitment() == commit);
        BOOST_REQUIRE(input.GetInputPubKey() == input_pubkey);
        BOOST_REQUIRE(input.GetOutputPubKey() == output_pubkey);
        BOOST_REQUIRE(input.GetSignature() == signature);
    }
}

BOOST_AUTO_TEST_SUITE_END()