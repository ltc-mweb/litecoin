// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Schnorr.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Kernel.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestKernel, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(PlainKernel)
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
        BOOST_REQUIRE(kernel.GetCommitment() == Crypto::CommitBlinded(0, excess_blind));
        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
    }
}

// MW: TODO - Uncomment these

//BOOST_AUTO_TEST_CASE(PegInKernel)
//{
//    CAmount amount = 50;
//    BlindingFactor excess_blind(Random::CSPRNG<32>());
//    Kernel kernel = Kernel::CreatePegIn(excess_blind, amount);
//
//    //
//    // Serialization
//    //
//    {
//        std::vector<uint8_t> serialized = kernel.Serialized();
//
//        Deserializer deserializer(serialized);
//        BOOST_REQUIRE(deserializer.Read<uint8_t>() == 1);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == amount);
//        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == kernel.GetExcess());
//        BOOST_REQUIRE(Signature::Deserialize(deserializer) == kernel.GetSignature());
//
//        Deserializer deserializer2(serialized);
//        BOOST_REQUIRE(kernel == Kernel::Deserialize(deserializer2));
//    }
//
//    //
//    // Signature Message
//    //
//    {
//        mw::Hash hashed = kernel.GetSignatureMessage();
//        mw::Hash message_hash = Hasher()
//            .Append<uint8_t>(1)
//            .Append<uint64_t>(amount)
//            .hash();
//        BOOST_REQUIRE(hashed == message_hash);
//    }
//
//    //
//    // Getters
//    //
//    {
//        BOOST_REQUIRE(kernel.IsPegIn());
//        BOOST_REQUIRE(!kernel.IsPegOut());
//        BOOST_REQUIRE(kernel.GetPeggedIn() == amount);
//        BOOST_REQUIRE(kernel.GetPeggedOut() == 0);
//        BOOST_REQUIRE(kernel.GetLockHeight() == 0);
//        BOOST_REQUIRE(kernel.GetFee() == 0);
//        BOOST_REQUIRE(kernel.GetCommitment() == Crypto::CommitBlinded(0, excess_blind));
//        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
//    }
//}
//
//BOOST_AUTO_TEST_CASE(PegOutKernel)
//{
//    CAmount amount = 50;
//    CAmount fee = 1000;
//    BlindingFactor excess_blind(Random::CSPRNG<32>());
//    Bech32Address address = Bech32Address::FromString("bc1qc7slrfxkknqcq2jevvvkdgvrt8080852dfjewde450xdlk4ugp7szw5tk9");
//    Kernel kernel = Kernel::CreatePegOut(excess_blind, amount, fee, address);
//
//    //
//    // Serialization
//    //
//    {
//        std::vector<uint8_t> serialized = kernel.Serialized();
//
//        Deserializer deserializer(serialized);
//        BOOST_REQUIRE(deserializer.Read<uint8_t>() == 2);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == fee);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == amount);
//        BOOST_REQUIRE(Bech32Address::Deserialize(deserializer) == address);
//        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == kernel.GetExcess());
//        BOOST_REQUIRE(Signature::Deserialize(deserializer) == kernel.GetSignature());
//
//        Deserializer deserializer2(serialized);
//        BOOST_REQUIRE(kernel == Kernel::Deserialize(deserializer2));
//    }
//
//    //
//    // Signature Message
//    //
//    {
//        mw::Hash hashed = kernel.GetSignatureMessage();
//        mw::Hash message_hash = Hasher()
//            .Append<uint8_t>(2)
//            .Append<uint64_t>(fee)
//            .Append<uint64_t>(amount)
//            .Append(address)
//            .hash();
//        BOOST_REQUIRE(hashed == message_hash);
//    }
//
//    //
//    // Getters
//    //
//    {
//        BOOST_REQUIRE(!kernel.IsPegIn());
//        BOOST_REQUIRE(kernel.IsPegOut());
//        BOOST_REQUIRE(kernel.GetPeggedIn() == 0);
//        BOOST_REQUIRE(kernel.GetPeggedOut() == amount);
//        BOOST_REQUIRE(kernel.GetLockHeight() == 0);
//        BOOST_REQUIRE(kernel.GetFee() == fee);
//        BOOST_REQUIRE(kernel.GetCommitment() == Crypto::CommitBlinded(0, excess_blind));
//        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
//    }
//}
//
//BOOST_AUTO_TEST_CASE(HeightLocked)
//{
//    CAmount fee = 1000;
//    int32_t lockHeight = 2500;
//    BlindingFactor excess_blind(Random::CSPRNG<32>());
//    Kernel kernel = Kernel::CreateHeightLocked(excess_blind, fee, lockHeight);
//
//    //
//    // Serialization
//    //
//    {
//        std::vector<uint8_t> serialized = kernel.Serialized();
//
//        Deserializer deserializer(serialized);
//        BOOST_REQUIRE(deserializer.Read<uint8_t>() == 3);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == fee);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == lockHeight);
//        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == kernel.GetExcess());
//        BOOST_REQUIRE(Signature::Deserialize(deserializer) == kernel.GetSignature());
//
//        Deserializer deserializer2(serialized);
//        BOOST_REQUIRE(kernel == Kernel::Deserialize(deserializer2));
//    }
//
//    //
//    // Signature Message
//    //
//    {
//        mw::Hash hashed = kernel.GetSignatureMessage();
//        mw::Hash message_hash = Hasher()
//            .Append<uint8_t>(3)
//            .Append<uint64_t>(fee)
//            .Append<uint64_t>(lockHeight)
//            .hash();
//        BOOST_REQUIRE(hashed == message_hash);
//    }
//
//    //
//    // Getters
//    //
//    {
//        BOOST_REQUIRE(!kernel.HasPegIn());
//        BOOST_REQUIRE(!kernel.HasPegOut());
//        BOOST_REQUIRE(kernel.GetPegIn() == 0);
//        BOOST_REQUIRE(!kernel.GetPegOut().has_value());
//        BOOST_REQUIRE(kernel.GetLockHeight() == lockHeight);
//        BOOST_REQUIRE(kernel.GetFee() == fee);
//        BOOST_REQUIRE(kernel.GetCommitment() == Crypto::CommitBlinded(0, excess_blind));
//        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
//    }
//}
//
//BOOST_AUTO_TEST_CASE(UnknownKernel)
//{
//    uint8_t features = 99;
//    uint64_t fee = 1000;
//    BlindingFactor excess_blind(Random::CSPRNG<32>());
//    std::vector<uint8_t> extraData = { 1, 2, 3 };
//
//    Commitment excess_commit = Crypto::CommitBlinded(0, excess_blind);
//    mw::Hash message_hash = Hasher()
//        .Append<uint8_t>(features)
//        .Append<uint64_t>(fee)
//        .Append(extraData)
//        .hash();
//    Signature sig = Schnorr::Sign(excess_blind.data(), message_hash);
//
//    Kernel kernel(
//        features,     // Unknown type
//        fee,
//        0,
//        0,
//        boost::none,
//        std::vector<uint8_t>(extraData),
//        std::move(excess_commit),
//        std::move(sig)
//    );
//
//    //
//    // Serialization
//    //
//    {
//        std::vector<uint8_t> serialized = kernel.Serialized();
//
//        Deserializer deserializer(serialized);
//        BOOST_REQUIRE(deserializer.Read<uint8_t>() == features);
//        BOOST_REQUIRE(deserializer.Read<uint64_t>() == fee);
//        BOOST_REQUIRE(deserializer.Read<uint8_t>() == extraData.size());
//        BOOST_REQUIRE(deserializer.ReadVector(extraData.size()) == extraData);
//        BOOST_REQUIRE(Commitment::Deserialize(deserializer) == kernel.GetExcess());
//        BOOST_REQUIRE(Signature::Deserialize(deserializer) == kernel.GetSignature());
//
//        Deserializer deserializer2(serialized);
//        BOOST_REQUIRE(kernel == Kernel::Deserialize(deserializer2));
//    }
//
//    //
//    // Getters
//    //
//    {
//        BOOST_REQUIRE(!kernel.IsPegIn());
//        BOOST_REQUIRE(!kernel.IsPegOut());
//        BOOST_REQUIRE(kernel.GetPeggedIn() == 0);
//        BOOST_REQUIRE(kernel.GetPeggedOut() == 0);
//        BOOST_REQUIRE(kernel.GetLockHeight() == 0);
//        BOOST_REQUIRE(kernel.GetFee() == fee);
//        BOOST_REQUIRE(kernel.GetCommitment() == Crypto::CommitBlinded(0, excess_blind));
//        BOOST_REQUIRE(kernel.GetSignature() == Schnorr::Sign(excess_blind.data(), kernel.GetSignatureMessage()));
//        BOOST_REQUIRE(kernel.GetExtraData() == extraData);
//        BOOST_REQUIRE(kernel.GetSignatureMessage() == message_hash);
//    }
//}

BOOST_AUTO_TEST_SUITE_END()