// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>

BOOST_FIXTURE_TEST_SUITE(TestRangeProofs, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(RangeProofs)
{
    const uint64_t value = 123;
    BlindingFactor blind = Random::CSPRNG<32>();
    Commitment commit = Crypto::CommitBlinded(value, blind);
    SecretKey nonce = Random::CSPRNG<32>();
    SecretKey nonce2 = Random::CSPRNG<32>();
    ProofMessage message = BigInt(Random::CSPRNG<20>().GetBigInt());
    std::vector<uint8_t> extraData = Random::CSPRNG<100>().vec();

    // Create a RangeProof via Crypto::GenerateRangeProof. Use the same value for privateNonce and rewindNonce.
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        value,
        SecretKey(blind.vec()),
        nonce,
        nonce,
        message,
        extraData
    );

    // Try rewinding it via Bulletproofs::Rewind using the *wrong* rewindNonce. Make sure it returns null.
    BOOST_REQUIRE(!Bulletproofs::Rewind(commit, *pRangeProof, extraData, nonce2));

    // Try rewinding it via Bulletproofs::Rewind using the *correct* rewindNonce. Make sure it returns a valid RewoundProof.
    std::unique_ptr<RewoundProof> pRewoundProof = Bulletproofs::Rewind(commit, *pRangeProof, extraData, nonce);
    BOOST_REQUIRE(pRewoundProof);

    // Make sure amount, blindingFactor (aka 'key'), and ProofMessage match the values passed to GenerateRangeProof
    BOOST_REQUIRE(*pRewoundProof == RewoundProof(
        value,
        std::make_unique<SecretKey>(SecretKey(blind.vec())),
        ProofMessage(message)
    ));

    // Make sure BatchVerify returns true. Use an empty vector for the third tuple value.
    std::vector<ProofData> rangeProofs;
    rangeProofs.push_back(ProofData{ commit, pRangeProof, extraData });
    BOOST_REQUIRE(Bulletproofs::BatchVerify(rangeProofs));
}

BOOST_AUTO_TEST_SUITE_END()