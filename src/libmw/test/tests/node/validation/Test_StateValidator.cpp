// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/file/ScopedFileRemover.h>
#include <mw/node/CoinsView.h>

#include <test_framework/models/Tx.h>
#include <test_framework/DBWrapper.h>
#include <test_framework/Miner.h>
#include <test_framework/TestUtil.h>
#include <test_framework/TxBuilder.h>

BOOST_FIXTURE_TEST_SUITE(TestStateValidator, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(ValidateState)
{
    FilePath datadir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(datadir); // Removes the directory when this goes out of scope.

    {
        auto pDatabase = std::make_shared<TestDBWrapper>();
        auto pDBView = mw::Node::Init(datadir, nullptr, pDatabase);
        BOOST_REQUIRE(pDBView != nullptr);

        auto pCachedView = std::make_shared<mw::CoinsViewCache>(pDBView);

        test::Miner miner;

        // Block containing peg-ins only
        test::Tx tx1 = test::TxBuilder()
            .AddPeginKernel(50)
            .AddOutput(50, EOutputFeatures::PEGGED_IN)
            .AddPeginKernel(30)
            .AddOutput(30, EOutputFeatures::PEGGED_IN)
            .Build();

        std::vector<PegInCoin> pegInCoins = tx1.GetTransaction()->GetPegIns();

        auto block1 = miner.MineBlock(150, { tx1 });
        BOOST_REQUIRE(mw::Node::ValidateBlock(block1.GetBlock(), block1.GetHash(), pegInCoins, {}));
        mw::Node::ConnectBlock(block1.GetBlock(), pCachedView);
        pCachedView->ValidateState();

        // Block containing peg-outs and regular sends only
        test::Tx tx2 = test::TxBuilder()
            .AddInput(50, EOutputFeatures::PEGGED_IN, tx1.GetOutputs().front().GetBlind())
            .AddPegoutKernel(15, 5)
            .AddPlainKernel(10)
            .AddOutput(20)
            .Build();

        std::vector<PegOutCoin> pegOutCoins = tx2.GetTransaction()->GetPegOuts();

        auto block2 = miner.MineBlock(151, { tx2 });
        BOOST_REQUIRE(mw::Node::ValidateBlock(block2.GetBlock(), block2.GetHash(), {}, pegOutCoins));
        mw::Node::ConnectBlock(block2.GetBlock(), pCachedView);
        pCachedView->ValidateState();

        pDBView.reset();
    }
}

BOOST_AUTO_TEST_SUITE_END()