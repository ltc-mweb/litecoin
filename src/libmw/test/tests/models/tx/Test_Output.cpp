// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/crypto/Blinds.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Schnorr.h>
#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>

#include <test_framework/Deserializer.h>
#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestOutput, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(Create)
{
    // Generate receiver addr
    SecretKey a = Random::CSPRNG<32>();
    SecretKey b = Random::CSPRNG<32>();
    StealthAddress receiver_addr = StealthAddress(
        PublicKey::From(a).Mul(b),
        PublicKey::From(b)
    );

    // Build output
    uint64_t amount = 1'234'567;
    BlindingFactor blind;
    SecretKey sender_key = Random::CSPRNG<32>();
    Output output = Output::Create(
        blind,
        sender_key,
        receiver_addr,
        amount
    );
    Commitment expected_commit = Commitment::Blinded(blind, amount);

    // Verify bulletproof
    ProofData proof_data = output.BuildProofData();
    BOOST_REQUIRE(proof_data.commitment == expected_commit);
    BOOST_REQUIRE(proof_data.pRangeProof == output.GetRangeProof());
    BOOST_REQUIRE(Bulletproofs::BatchVerify({ output.BuildProofData() }));

    // Verify sender signature
    SignedMessage signed_msg = output.BuildSignedMsg();
    BOOST_REQUIRE(signed_msg.GetPublicKey() == PublicKey::From(sender_key));
    BOOST_REQUIRE(Schnorr::BatchVerify({ signed_msg }));

    // Getters
    BOOST_REQUIRE(output.GetCommitment() == expected_commit);
    BOOST_REQUIRE(output.ToOutputId() == OutputId(expected_commit, output.GetOutputMessage()));

    //
    // Test Restoring Output
    //
    {
        // Check view tag
        SecretKey t = Hashed(EHashTag::DERIVE, output.Ke().Mul(a));
        BOOST_REQUIRE(t[0] == output.GetViewTag());

        // Make sure B belongs to wallet
        BOOST_REQUIRE(receiver_addr.B() == output.Ko().Sub(Hashed(EHashTag::OUT_KEY, t)));

        Deserializer hash64(Hash512(t).vec());
        SecretKey r = hash64.Read<SecretKey>();
        uint64_t value = output.GetMaskedValue() ^ hash64.Read<uint64_t>();
        BigInt<16> n = output.GetMaskedNonce() ^ hash64.ReadVector(16);

        BOOST_REQUIRE(Commitment::Switch(r, value) == output.GetCommitment());

        // Calculate Carol's sending key 's' and check that s*B ?= Ke
        SecretKey s = Hasher(EHashTag::SEND_KEY)
            .Append(receiver_addr.A())
            .Append(receiver_addr.B())
            .Append(value)
            .Append(n)
            .hash();
        BOOST_REQUIRE(output.Ke() == receiver_addr.B().Mul(s));

        // Make sure receiver can generate the spend key
        SecretKey spend_key = Blinds()
            .Add(b)
            .Add(Hashed(EHashTag::OUT_KEY, t))
            .ToKey();
        BOOST_REQUIRE(output.GetReceiverPubKey() == PublicKey::From(spend_key));
    }
}

BOOST_AUTO_TEST_SUITE_END()