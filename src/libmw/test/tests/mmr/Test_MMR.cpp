// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/VectorBackend.h>

#include <test_framework/TestMWEB.h>

using namespace mmr;

BOOST_FIXTURE_TEST_SUITE(TestMMR, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(MMRTest)
{
    auto pBackend = std::make_shared<VectorBackend>();
    MMR mmr(pBackend);

    std::vector<uint8_t> leaf0({ 0, 1, 2 });
    std::vector<uint8_t> leaf1({ 1, 2, 3 });
    std::vector<uint8_t> leaf2({ 2, 3, 4 });
    std::vector<uint8_t> leaf3({ 3, 4, 5 });
    std::vector<uint8_t> leaf4({ 4, 5, 6 });

    mmr.Add(leaf0);
    mmr.Add(leaf1);
    mmr.Add(leaf2);
    mmr.Add(leaf3);

    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(0)) == Leaf::Create(LeafIndex::At(0), leaf0));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(1)) == Leaf::Create(LeafIndex::At(1), leaf1));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(2)) == Leaf::Create(LeafIndex::At(2), leaf2));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(3)) == Leaf::Create(LeafIndex::At(3), leaf3));

    BOOST_REQUIRE(mmr.GetNumLeaves() == 4);
    BOOST_REQUIRE(mmr.GetNumNodes() == 7);
    BOOST_REQUIRE_EQUAL(mmr.Root().ToHex(), "56a97c32569242afde58b078bf37aa9f4f0bc6c90f466ff668295b281dac88bf");

    mmr.Add(leaf4);
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(4)) == Leaf::Create(LeafIndex::At(4), leaf4));
    BOOST_REQUIRE(mmr.GetNumLeaves() == 5);
    BOOST_REQUIRE(mmr.GetNumNodes() == 8);
    BOOST_REQUIRE_EQUAL(mmr.Root().ToHex(), "3b9b05f1c8fd31811131684ead63ca7de8dca35e92ae97ba1ace850a804b1f40");

    mmr.Rewind(4);
    BOOST_REQUIRE(mmr.GetNumLeaves() == 4);
    BOOST_REQUIRE(mmr.GetNumNodes() == 7);
    BOOST_REQUIRE_EQUAL(mmr.Root().ToHex(), "56a97c32569242afde58b078bf37aa9f4f0bc6c90f466ff668295b281dac88bf");
}

BOOST_AUTO_TEST_CASE(MMRCacheTest)
{
    auto pBackend = std::make_shared<VectorBackend>();
    std::shared_ptr<MMR> mmr = std::make_shared<MMR>(pBackend);

    MMRCache cache(mmr);
    std::vector<uint8_t> leaf0({ 0, 1, 2 });
    std::vector<uint8_t> leaf1({ 1, 2, 3 });
    std::vector<uint8_t> leaf2({ 2, 3, 4 });
    std::vector<uint8_t> leaf3({ 3, 4, 5 });
    std::vector<uint8_t> leaf4({ 4, 5, 6 });

    cache.Add(leaf0);
    cache.Add(leaf1);
    cache.Add(leaf2);
    cache.Add(leaf3);

    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(0)) == Leaf::Create(LeafIndex::At(0), leaf0));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(1)) == Leaf::Create(LeafIndex::At(1), leaf1));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(2)) == Leaf::Create(LeafIndex::At(2), leaf2));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(3)) == Leaf::Create(LeafIndex::At(3), leaf3));

    BOOST_REQUIRE(cache.GetNumLeaves() == 4);
    BOOST_REQUIRE_EQUAL(cache.Root().ToHex(), "56a97c32569242afde58b078bf37aa9f4f0bc6c90f466ff668295b281dac88bf");

    cache.Add(leaf4);
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(4)) == Leaf::Create(LeafIndex::At(4), leaf4));
    BOOST_REQUIRE(cache.GetNumLeaves() == 5);
    BOOST_REQUIRE_EQUAL(cache.Root().ToHex(), "3b9b05f1c8fd31811131684ead63ca7de8dca35e92ae97ba1ace850a804b1f40");

    cache.Rewind(4);
    BOOST_REQUIRE(cache.GetNumLeaves() == 4);
    BOOST_REQUIRE_EQUAL(cache.Root().ToHex(), "56a97c32569242afde58b078bf37aa9f4f0bc6c90f466ff668295b281dac88bf");

    cache.Flush(0, nullptr);
}

BOOST_AUTO_TEST_SUITE_END()