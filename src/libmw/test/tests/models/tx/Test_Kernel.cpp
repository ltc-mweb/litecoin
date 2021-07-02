// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/crypto/Schnorr.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Kernel.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestKernel, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(PlainKernel_Test)
{
    CAmount fee = 1000;
    BlindingFactor excess_blind(Random::CSPRNG<32>());
    Kernel kernel = Kernel::Create(excess_blind, fee, boost::none, boost::none, boost::none);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();

        Kernel kernel2;
        CDataStream(serialized, SER_DISK, 0) >> kernel2;
        BOOST_REQUIRE(kernel == kernel2);
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        mw::Hash message_hash = Hasher()
            .Append<uint8_t>(1)
            .Append<CVarInt<VarIntMode::NONNEGATIVE_SIGNED, CAmount>>(VARINT(fee, VarIntMode::NONNEGATIVE_SIGNED))
            .hash();
        BOOST_REQUIRE(hashed == message_hash);
    }

    //
    // Getters
    //
    {
        BOOST_REQUIRE(!kernel.HasPegIn());
        BOOST_REQUIRE(!kernel.HasPegOut());
        BOOST_REQUIRE(kernel.GetPegIn() == 0);
        BOOST_REQUIRE(kernel.GetPegOut() == boost::none);
        BOOST_REQUIRE(kernel.GetLockHeight() == 0);
        BOOST_REQUIRE(kernel.GetFee() == fee);
        BOOST_REQUIRE(kernel.GetCommitment() == Commitment::Blinded(excess_blind, 0));
        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
    }
}

BOOST_AUTO_TEST_CASE(NonStandardKernel_Test)
{
    CAmount fee = 1000;
    CAmount pegin = 10000;
    PegOutCoin pegout(2000, CScript(Random::CSPRNG<30>().vec()));
    int32_t lock_height = 123456;
    Kernel standard_kernel(
        Kernel::FEE_FEATURE_BIT | Kernel::PEGIN_FEATURE_BIT | Kernel::PEGOUT_FEATURE_BIT | Kernel::HEIGHT_LOCK_FEATURE_BIT,
        fee,
        pegin,
        boost::make_optional(PegOutCoin(2000, CScript(Random::CSPRNG<30>().vec()))),
        lock_height,
        std::vector<uint8_t>{},
        Commitment::Random(),
        Signature(Random::CSPRNG<64>().GetBigInt())
    );
    Kernel nonstandard_kernel1(
        Kernel::ALL_FEATURE_BITS,
        fee,
        pegin,
        boost::make_optional(PegOutCoin(2000, CScript(Random::CSPRNG<30>().vec()))),
        lock_height,
        Random::CSPRNG<20>().vec(),
        standard_kernel.GetCommitment(),
        standard_kernel.GetSignature()
    );

    Kernel nonstandard_kernel2(
        0x20,
        boost::none,
        boost::none,
        boost::none,
        boost::none,
        std::vector<uint8_t>{},
        standard_kernel.GetCommitment(),
        standard_kernel.GetSignature()
    );

    BOOST_REQUIRE(standard_kernel.IsStandard());
    BOOST_REQUIRE(!nonstandard_kernel1.IsStandard());
    BOOST_REQUIRE(!nonstandard_kernel2.IsStandard());
}

BOOST_AUTO_TEST_SUITE_END()