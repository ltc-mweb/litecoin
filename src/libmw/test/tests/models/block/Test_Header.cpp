// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/models/block/Header.h>

BOOST_FIXTURE_TEST_SUITE(TestHeader, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(Header)
{
    const uint64_t height = 1;
    const uint64_t outputMMRSize = 2;
    const uint64_t kernelMMRSize = 3;

    mw::Hash outputRoot = mw::Hash::FromHex("000102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    mw::Hash kernelRoot = mw::Hash::FromHex("002102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    mw::Hash leafsetRoot = mw::Hash::FromHex("003102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    BlindingFactor kernelOffset = BigInt<32>::FromHex("004102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    BlindingFactor ownerOffset = BigInt<32>::FromHex("005102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");

    mw::Header header(
        height,
        mw::Hash(outputRoot),
        mw::Hash(kernelRoot),
        mw::Hash(leafsetRoot),
        BlindingFactor(kernelOffset),
        BlindingFactor(ownerOffset),
        outputMMRSize,
        kernelMMRSize
    );
    mw::Header header2(
        height + 1,
        mw::Hash(outputRoot),
        mw::Hash(kernelRoot),
        mw::Hash(leafsetRoot),
        BlindingFactor(kernelOffset),
        BlindingFactor(ownerOffset),
        outputMMRSize,
        kernelMMRSize
    );

    BOOST_REQUIRE(header != header2);
    BOOST_REQUIRE(header.GetHeight() == height);
    BOOST_REQUIRE(header.GetOutputRoot() == outputRoot);
    BOOST_REQUIRE(header.GetKernelRoot() == kernelRoot);
    BOOST_REQUIRE(header.GetLeafsetRoot() == leafsetRoot);
    BOOST_REQUIRE(header.GetKernelOffset() == kernelOffset);
    BOOST_REQUIRE(header.GetOwnerOffset() == ownerOffset);
    BOOST_REQUIRE(header.GetNumTXOs() == outputMMRSize);
    BOOST_REQUIRE(header.GetNumKernels() == kernelMMRSize);
    BOOST_REQUIRE_EQUAL(header.Format(), "3d3ebb08c2bca28216bf55730b7950b8641e3ec38c9c85a00a54d0c5a16bac69");

    std::vector<uint8_t> header_serialized = header.Serialized();
    mw::Header header3;
    CDataStream(header_serialized, SER_DISK, 0) >> header3;
    BOOST_REQUIRE(header == header3);
}

BOOST_AUTO_TEST_SUITE_END()