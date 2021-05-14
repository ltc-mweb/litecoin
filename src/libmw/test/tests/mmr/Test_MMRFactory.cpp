// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/crypto/Random.h>
#include <mw/mmr/Leaf.h>
#include <mw/mmr/MMRFactory.h>
#include <mw/mmr/MMRUtil.h>

using namespace mmr;

BOOST_FIXTURE_TEST_SUITE(TestMMRFactory, BasicTestingSetup)

static std::vector<Leaf> RandomLeaves(const BitSet& leafset)
{
    std::vector<Leaf> leaves;
    for (uint64_t leaf_idx = 0; leaf_idx < leafset.size(); leaf_idx++) {
        if (leafset.test(leaf_idx)) {
            leaves.push_back(Leaf::Create(LeafIndex::At(leaf_idx), Random::CSPRNG<20>().vec()));
        }
    }
    return leaves;
}

static std::vector<mw::Hash> RandomHashes(const uint64_t num_hashes)
{
    std::vector<mw::Hash> hashes(num_hashes);
    for (uint64_t i = 0; i < num_hashes; i++) {
        hashes[i] = Random::CSPRNG<32>().GetBigInt();
    }
    return hashes;
}

BOOST_AUTO_TEST_CASE(CalcHashes)
{
    size_t num_leaves = 50;
    BitSet unspent_leaf_indices(num_leaves);
    unspent_leaf_indices.set(2);
    unspent_leaf_indices.set(9);
    unspent_leaf_indices.set(26);
    unspent_leaf_indices.set(27);
    for (size_t i = 30; i < num_leaves; i++) {
        unspent_leaf_indices.set(i);
    }

    std::vector<Leaf> leaves = RandomLeaves(unspent_leaf_indices);

    BitSet pruned_parent_indices = MMRUtil::CalcPrunedParents(unspent_leaf_indices);
    BOOST_REQUIRE(pruned_parent_indices.str() == "0010100000000101000010000000100000000000000001001000000100000000000000000000000000000000000000000000");
    std::vector<mw::Hash> pruned_parent_hashes = RandomHashes(pruned_parent_indices.count());

    std::vector<mw::Hash> hashes = MMRFactory::CalcHashes(unspent_leaf_indices, leaves, pruned_parent_hashes);
    BOOST_REQUIRE(hashes.size() == 63);
    BOOST_REQUIRE(hashes[0] == pruned_parent_hashes[0]);
    BOOST_REQUIRE(hashes[61] == leaves[23].GetHash());

    // Finish these assertions
}

BOOST_AUTO_TEST_SUITE_END()