// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/crypto/Crypto.h>
#include <mw/crypto/MuSig.h>
#include <mw/crypto/Schnorr.h>
#include <mw/crypto/Random.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestAggSig, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(AggSigInteraction)
{
    mw::Hash message = Random::CSPRNG<32>().GetBigInt();

    // Generate sender keypairs
    SecretKey secretKeySender = Random::CSPRNG<32>();
    PublicKey publicKeySender = Crypto::CalculatePublicKey(secretKeySender.GetBigInt());
    SecretKey secretNonceSender = MuSig::GenerateSecureNonce();
    PublicKey publicNonceSender = Crypto::CalculatePublicKey(secretNonceSender.GetBigInt());

    // Generate receiver keypairs
    SecretKey secretKeyReceiver = Random::CSPRNG<32>();
    PublicKey publicKeyReceiver = Crypto::CalculatePublicKey(secretKeyReceiver.GetBigInt());
    SecretKey secretNonceReceiver = MuSig::GenerateSecureNonce();
    PublicKey publicNonceReceiver = Crypto::CalculatePublicKey(secretNonceReceiver.GetBigInt());

    // Add pubKeys and pubNonces
    PublicKey sumPubKeys = Crypto::AddPublicKeys(
        std::vector<PublicKey>({ publicKeySender, publicKeyReceiver })
    );

    PublicKey sumPubNonces = Crypto::AddPublicKeys(
        std::vector<PublicKey>({ publicNonceSender, publicNonceReceiver })
    );

    // Generate partial signatures
    CompactSignature senderPartialSignature = MuSig::CalculatePartial(
        secretKeySender,
        secretNonceSender,
        sumPubKeys,
        sumPubNonces,
        message
    );
    CompactSignature receiverPartialSignature = MuSig::CalculatePartial(
        secretKeyReceiver,
        secretNonceReceiver,
        sumPubKeys,
        sumPubNonces,
        message
    );

    // Validate partial signatures
    const bool senderSigValid = MuSig::VerifyPartial(
        senderPartialSignature,
        publicKeySender,
        sumPubKeys,
        sumPubNonces,
        message
    );
    BOOST_REQUIRE(senderSigValid == true);

    const bool receiverSigValid = MuSig::VerifyPartial(
        receiverPartialSignature,
        publicKeyReceiver,
        sumPubKeys,
        sumPubNonces,
        message
    );
    BOOST_REQUIRE(receiverSigValid == true);

    // Aggregate signature and validate
    Signature aggregateSignature = MuSig::Aggregate(
        std::vector<CompactSignature>({ senderPartialSignature, receiverPartialSignature }),
        sumPubNonces
    );
    const bool aggSigValid = Schnorr::Verify(
        aggregateSignature,
        sumPubKeys,
        message
    );
    BOOST_REQUIRE(aggSigValid == true);
}

BOOST_AUTO_TEST_CASE(SingleSchnorrSig)
{
    mw::Hash message = Random::CSPRNG<32>().GetBigInt();
    SecretKey secret_key = Random::CSPRNG<32>();
    PublicKey public_key = Crypto::CalculatePublicKey(secret_key.GetBigInt());
    Signature signature = Schnorr::Sign(secret_key.data(), message);

    const bool valid = Schnorr::Verify(signature, public_key, message);
    BOOST_REQUIRE(valid == true);
}

BOOST_AUTO_TEST_SUITE_END()