// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/mmr/backends/FileBackend.h>
#include <mw/models/tx/Kernel.h>
#include <mw/crypto/Random.h>

#include <test_framework/TestMWEB.h>

using namespace mmr;

BOOST_FIXTURE_TEST_SUITE(TestFileBackend, MWEBTestingSetup)

// MW: TODO - Test with pruning
BOOST_AUTO_TEST_CASE(FileBackendTest)
{
    auto pDatabase = GetDB();

    {
        auto pBackend = FileBackend::Open('T', GetDataDir(), 0, pDatabase, nullptr);
        pBackend->AddLeaf(mmr::Leaf::Create(mmr::LeafIndex::At(0), { 0x05, 0x03, 0x07 }));
        pBackend->Commit(1, nullptr);
    }
    {
        auto pBackend = FileBackend::Open('T', GetDataDir(), 1, pDatabase, nullptr);
        BOOST_REQUIRE(pBackend->GetNumLeaves() == 1);
        auto leaf = pBackend->GetLeaf(mmr::LeafIndex::At(0));
        BOOST_REQUIRE((leaf.vec() == std::vector<uint8_t>{ 0x05, 0x03, 0x07 }));
    }
}

BOOST_AUTO_TEST_SUITE_END()