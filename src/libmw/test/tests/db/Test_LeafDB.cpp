// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/db/LeafDB.h>
#include <mw/file/ScopedFileRemover.h>

#include <test_framework/DBWrapper.h>
#include <test_framework/TestUtil.h>

BOOST_FIXTURE_TEST_SUITE(TestLeafDB, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(LeafDBTest)
{
    FilePath datadir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(datadir); // Removes the directory when this goes out of scope.

    TestDBWrapper test_wrapper(datadir.GetChild("db"));
    auto pDatabase = test_wrapper.Get();

    LeafDB ldb('L', pDatabase.get());

    auto leaf1 = mmr::Leaf::Create(mmr::LeafIndex::At(0), { 0, 1, 2 });
    auto leaf2 = mmr::Leaf::Create(mmr::LeafIndex::At(1), { 1, 2, 3 });
    auto leaf3 = mmr::Leaf::Create(mmr::LeafIndex::At(2), { 2, 3, 4 });

    ldb.Add({leaf1, leaf2, leaf3});

    auto pLeaf = ldb.Get(mmr::LeafIndex::At(0));
    BOOST_REQUIRE(pLeaf->GetHash() == leaf1.GetHash());
    BOOST_REQUIRE(pLeaf->vec() == leaf1.vec());

    pLeaf = ldb.Get(mmr::LeafIndex::At(1));
    BOOST_REQUIRE(pLeaf->GetHash() == leaf2.GetHash());
    BOOST_REQUIRE(pLeaf->vec() == leaf2.vec());

    pLeaf = ldb.Get(mmr::LeafIndex::At(2));
    BOOST_REQUIRE(pLeaf->GetHash() == leaf3.GetHash());
    BOOST_REQUIRE(pLeaf->vec() == leaf3.vec());

    std::vector<uint8_t> data;
    BOOST_REQUIRE(pDatabase->Read("L" + std::to_string(leaf1.GetLeafIndex().Get()), data));
    BOOST_REQUIRE(data == leaf1.vec());
    BOOST_REQUIRE(pDatabase->Read("L" + std::to_string(leaf2.GetLeafIndex().Get()), data));
    BOOST_REQUIRE(data == leaf2.vec());
    BOOST_REQUIRE(pDatabase->Read("L" + std::to_string(leaf3.GetLeafIndex().Get()), data));
    BOOST_REQUIRE(data == leaf3.vec());

    ldb.Remove({leaf2.GetLeafIndex()});
    BOOST_REQUIRE(ldb.Get(mmr::LeafIndex::At(1)) == nullptr);

    ldb.RemoveAll();
    BOOST_REQUIRE(ldb.Get(mmr::LeafIndex::At(0)) == nullptr);
    BOOST_REQUIRE(ldb.Get(mmr::LeafIndex::At(2)) == nullptr);

    ldb.Add({leaf2});
    pLeaf = ldb.Get(mmr::LeafIndex::At(1));
    BOOST_REQUIRE(pLeaf->GetHash() == leaf2.GetHash());
    BOOST_REQUIRE(pLeaf->vec() == leaf2.vec());
}

BOOST_AUTO_TEST_SUITE_END()