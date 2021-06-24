// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/node/CoinsView.h>
#include <mw/node/Node.h>

#include <test_framework/models/Tx.h>
#include <test_framework/Miner.h>
#include <test_framework/TestMWEB.h>
#include <test_framework/TxBuilder.h>

BOOST_FIXTURE_TEST_SUITE(TestStateValidator, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(ValidateState)
{
    auto pDatabase = GetDB();

    auto pDBView = mw::Node::Init(GetDataDir(), nullptr, pDatabase);
    BOOST_REQUIRE(pDBView != nullptr);

    auto pCachedView = std::make_shared<mw::CoinsViewCache>(pDBView);

    test::Miner miner(GetDataDir());

    // Block containing peg-ins only
    test::Tx tx1 = test::TxBuilder()
        .AddPeginKernel(50)
        .AddOutput(50)
        .AddPeginKernel(30)
        .AddOutput(30)
        .Build();

    std::vector<PegInCoin> pegInCoins = tx1.GetTransaction()->GetPegIns();

    auto block1 = miner.MineBlock(150, { tx1 });
    BOOST_REQUIRE(mw::Node::ValidateBlock(block1.GetBlock(), block1.GetHash(), pegInCoins, {}));
    pCachedView->ApplyBlock(block1.GetBlock());
    pCachedView->ValidateState();

    // Block containing peg-outs and regular sends only
    test::Tx tx2 = test::TxBuilder()
        .AddInput(tx1.GetOutputs().front())
        .AddPegoutKernel(15, 5)
        .AddPlainKernel(10)
        .AddOutput(20)
        .Build();

    std::vector<PegOutCoin> pegOutCoins = tx2.GetTransaction()->GetPegOuts();

    auto block2 = miner.MineBlock(151, { tx2 });
    BOOST_REQUIRE(mw::Node::ValidateBlock(block2.GetBlock(), block2.GetHash(), {}, pegOutCoins));
    pCachedView->ApplyBlock(block2.GetBlock());
    pCachedView->ValidateState();
}

BOOST_AUTO_TEST_SUITE_END()