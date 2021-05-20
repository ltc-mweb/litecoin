// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/VectorBackend.h>

using namespace mmr;

BOOST_FIXTURE_TEST_SUITE(TestMMR, BasicTestingSetup)

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

    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(0)) == Leaf::Create(LeafIndex::At(0), std::vector<uint8_t>(leaf0)));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(1)) == Leaf::Create(LeafIndex::At(1), std::vector<uint8_t>(leaf1)));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(2)) == Leaf::Create(LeafIndex::At(2), std::vector<uint8_t>(leaf2)));
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(3)) == Leaf::Create(LeafIndex::At(3), std::vector<uint8_t>(leaf3)));

    BOOST_REQUIRE(mmr.GetNumLeaves() == 4);
    BOOST_REQUIRE(mmr.GetNumNodes() == 7);
    BOOST_REQUIRE(mmr.Root() == mw::Hash::FromHex("675996c8bbfce6319dd00588ebd289d555eedfa60aa17a9a83cc7da80888a97e"));

    mmr.Add(leaf4);
    BOOST_REQUIRE(mmr.GetLeaf(LeafIndex::At(4)) == Leaf::Create(LeafIndex::At(4), std::vector<uint8_t>(leaf4)));
    BOOST_REQUIRE(mmr.GetNumLeaves() == 5);
    BOOST_REQUIRE(mmr.GetNumNodes() == 8);
    BOOST_REQUIRE(mmr.Root() == mw::Hash::FromHex("2657bf9ad4d4269ba1cfa872b325497fcff937c6b96df336e4ba29842a80232d"));

    mmr.Rewind(4);
    BOOST_REQUIRE(mmr.GetNumLeaves() == 4);
    BOOST_REQUIRE(mmr.GetNumNodes() == 7);
    BOOST_REQUIRE(mmr.Root() == mw::Hash::FromHex("675996c8bbfce6319dd00588ebd289d555eedfa60aa17a9a83cc7da80888a97e"));
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

    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(0)) == Leaf::Create(LeafIndex::At(0), std::vector<uint8_t>(leaf0)));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(1)) == Leaf::Create(LeafIndex::At(1), std::vector<uint8_t>(leaf1)));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(2)) == Leaf::Create(LeafIndex::At(2), std::vector<uint8_t>(leaf2)));
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(3)) == Leaf::Create(LeafIndex::At(3), std::vector<uint8_t>(leaf3)));

    BOOST_REQUIRE(cache.GetNumLeaves() == 4);
    BOOST_REQUIRE(cache.Root() == mw::Hash::FromHex("675996c8bbfce6319dd00588ebd289d555eedfa60aa17a9a83cc7da80888a97e"));

    cache.Add(leaf4);
    BOOST_REQUIRE(cache.GetLeaf(LeafIndex::At(4)) == Leaf::Create(LeafIndex::At(4), std::vector<uint8_t>(leaf4)));
    BOOST_REQUIRE(cache.GetNumLeaves() == 5);
    BOOST_REQUIRE(cache.Root() == mw::Hash::FromHex("2657bf9ad4d4269ba1cfa872b325497fcff937c6b96df336e4ba29842a80232d"));

    cache.Rewind(4);
    BOOST_REQUIRE(cache.GetNumLeaves() == 4);
    BOOST_REQUIRE(cache.Root() == mw::Hash::FromHex("675996c8bbfce6319dd00588ebd289d555eedfa60aa17a9a83cc7da80888a97e"));

    cache.Flush(0, nullptr);
}

BOOST_AUTO_TEST_SUITE_END()