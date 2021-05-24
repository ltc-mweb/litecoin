// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <libmw/libmw.h>
#include <mw/file/ScopedFileRemover.h>
#include <mw/node/validation/BlockValidator.h>

#include <test_framework/DBWrapper.h>
#include <test_framework/Miner.h>
#include <test_framework/TestUtil.h>

BOOST_FIXTURE_TEST_SUITE(TestBlockBuilder, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(BlockBuilder)
{
    FilePath datadir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(datadir); // Removes the directory when this goes out of scope.

    {
        auto pDatabase = std::make_shared<TestDBWrapper>();
        libmw::ChainParams params{ datadir.u8string() };
        mw::ICoinsView::Ptr db_view = libmw::node::Initialize(params, mw::Header::CPtr{nullptr}, pDatabase, {});
        mw::ICoinsView::Ptr cached_view = std::make_shared<mw::CoinsViewCache>(db_view);

        test::Miner miner;

        ///////////////////////
        // Mine Block 1
        ///////////////////////
        test::Tx block1_tx1 = test::Tx::CreatePegIn(1000);
        auto block1 = miner.MineBlock(150, { block1_tx1 });
        libmw::node::ConnectBlock(block1.GetBlock(), cached_view);

        ///////////////////////
        // Mine Block 2
        ///////////////////////
        test::Tx block2_tx1 = test::Tx::CreatePegIn(500);
        auto block2 = miner.MineBlock(151, { block2_tx1 });
        libmw::node::ConnectBlock(block2.GetBlock(), cached_view);

        ///////////////////////
        // Flush View
        ///////////////////////
        auto pBatch = pDatabase->CreateBatch();
        libmw::node::FlushCache(cached_view, pBatch);
        pBatch->Commit();

        ///////////////////////
        // BlockBuilder
        ///////////////////////
        std::shared_ptr<mw::BlockBuilder> block_builder = libmw::miner::NewBuilder(152, cached_view);

        test::Tx builder_tx1 = test::Tx::CreatePegIn(150);
        PegInCoin builder_tx1_pegin{ 150, builder_tx1.GetKernels().front().GetCommitment() };
        bool tx1_status = libmw::miner::AddTransaction(
            block_builder,
            builder_tx1.GetTransaction(),
            { builder_tx1_pegin }
        );
        BOOST_REQUIRE(tx1_status);

        mw::Block::Ptr built_block = libmw::miner::BuildBlock(block_builder);
        BOOST_REQUIRE(built_block->GetKernels().front() == builder_tx1.GetKernels().front());
        BlockValidator().Validate(built_block, built_block->GetHash(), { builder_tx1.GetPegInCoin() }, {});

        libmw::node::Shutdown();
    }
}

BOOST_AUTO_TEST_SUITE_END()