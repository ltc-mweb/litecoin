// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/file/ScopedFileRemover.h>
#include <mw/mmr/PruneList.h>

#include <test_framework/TestUtil.h>

BOOST_FIXTURE_TEST_SUITE(TestPruneList, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(PruneList)
{
    FilePath tempDir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(tempDir);

    // Bitset: 00100000 01000000 00000000 00110011 11111111 100000000
    File(tempDir.GetChild("prun000009.dat"))
        .Write({ 0x20, 0x40, 0x00, 0x33, 0xff, 0x80 });

    mmr::PruneList::Ptr pPruneList = mmr::PruneList::Open(tempDir, 9);

    BOOST_REQUIRE(pPruneList->GetTotalShift() == 15);
    BOOST_REQUIRE(pPruneList->GetShift(mmr::Index::At(1)) == 0);
    BOOST_REQUIRE(pPruneList->GetShift(mmr::Index::At(3)) == 1);
    BOOST_REQUIRE(pPruneList->GetShift(mmr::Index::At(28)) == 4);
    BOOST_REQUIRE(pPruneList->GetShift(mmr::Index::At(60)) == 15);
}

BOOST_AUTO_TEST_SUITE_END()