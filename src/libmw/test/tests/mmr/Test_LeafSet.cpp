// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/mmr/LeafSet.h>
#include <mw/crypto/Hasher.h>
#include <mw/file/ScopedFileRemover.h>

#include <test_framework/TestUtil.h>

BOOST_FIXTURE_TEST_SUITE(TestMMRLeafSet, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(LeafSet)
{
    FilePath temp_dir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(temp_dir); // Removes the directory when this goes out of scope.

    {
        mmr::LeafSet::Ptr pLeafset = mmr::LeafSet::Open(temp_dir, 0);

        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 0);
        BOOST_REQUIRE(!pLeafset->Contains(mmr::LeafIndex::At(0)));
        BOOST_REQUIRE(!pLeafset->Contains(mmr::LeafIndex::At(1)));
        BOOST_REQUIRE(!pLeafset->Contains(mmr::LeafIndex::At(2)));
        BOOST_REQUIRE(pLeafset->Root() == Hashed(std::vector<uint8_t>{ }));

        pLeafset->Add(mmr::LeafIndex::At(0));
        BOOST_REQUIRE(pLeafset->Contains(mmr::LeafIndex::At(0)));
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 1);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b10000000 }));

        pLeafset->Add(mmr::LeafIndex::At(1));
        BOOST_REQUIRE(pLeafset->Contains(mmr::LeafIndex::At(1)));
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 2);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b11000000 }));

        pLeafset->Add(mmr::LeafIndex::At(2));
        BOOST_REQUIRE(pLeafset->Contains(mmr::LeafIndex::At(2)));
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 3);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b11100000 }));

        pLeafset->Remove(mmr::LeafIndex::At(1));
        BOOST_REQUIRE(!pLeafset->Contains(mmr::LeafIndex::At(1)));
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 3);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b10100000 }));

        pLeafset->Rewind(2, { mmr::LeafIndex::At(1) });
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 2);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b11000000 }));

        // Flush to disk and validate
        pLeafset->Flush(1);
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 2);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b11000000 }));
    }

    {
        // Reload from disk and validate
        mmr::LeafSet::Ptr pLeafset = mmr::LeafSet::Open(temp_dir, 1);
        BOOST_REQUIRE(pLeafset->GetNextLeafIdx().Get() == 2);
        BOOST_REQUIRE(pLeafset->Root() == Hashed({ 0b11000000 }));
    }
}

BOOST_AUTO_TEST_SUITE_END()