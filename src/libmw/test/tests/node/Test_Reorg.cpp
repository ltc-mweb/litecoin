// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/node/CoinsView.h>
#include <mw/node/Node.h>

#include <test_framework/Miner.h>
#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestReorg, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(ReorgChain)
{
    auto pDatabase = GetDB();

    auto pDBView = mw::Node::Init(GetDataDir(), nullptr, pDatabase);
    BOOST_REQUIRE(pDBView != nullptr);

    auto pCachedView = std::make_shared<mw::CoinsViewCache>(pDBView);

    test::Miner miner(GetDataDir());

    ///////////////////////
    // Mine Block 1
    ///////////////////////
    test::Tx block1_tx1 = test::Tx::CreatePegIn(1000);
    auto block1 = miner.MineBlock(160, { block1_tx1 });
    BOOST_REQUIRE(mw::Node::ValidateBlock(block1.GetBlock(), block1.GetHash(), { block1_tx1.GetPegInCoin() }, {}));
    mw::Node::ConnectBlock(block1.GetBlock(), pCachedView);

    const auto& block1_tx1_output1 = block1_tx1.GetOutputs()[0];
    BOOST_REQUIRE(pDBView->GetUTXOs(block1_tx1_output1.GetCommitment()).empty());
    BOOST_REQUIRE(pCachedView->GetUTXOs(block1_tx1_output1.GetCommitment()).size() == 1);

    ///////////////////////
    // Mine Block 2
    ///////////////////////
    test::Tx block2_tx1 = test::Tx::CreatePegIn(500);
    auto block2 = miner.MineBlock(161, { block2_tx1 });
    BOOST_REQUIRE(mw::Node::ValidateBlock(block2.GetBlock(), block2.GetHash(), { block2_tx1.GetPegInCoin() }, {}));
    mw::BlockUndo::CPtr undoBlock2 = mw::Node::ConnectBlock(block2.GetBlock(), pCachedView);

    const auto& block2_tx1_output1 = block2_tx1.GetOutputs()[0];
    BOOST_REQUIRE(pDBView->GetUTXOs(block2_tx1_output1.GetCommitment()).empty());
    BOOST_REQUIRE(pCachedView->GetUTXOs(block2_tx1_output1.GetCommitment()).size() == 1);

    ///////////////////////
    // Disconnect Block 2
    ///////////////////////
    mw::Node::DisconnectBlock(undoBlock2, pCachedView);
    BOOST_REQUIRE(pCachedView->GetUTXOs(block1_tx1_output1.GetCommitment()).size() == 1);
    BOOST_REQUIRE(pCachedView->GetUTXOs(block2_tx1_output1.GetCommitment()).empty());
    miner.Rewind(1);

    ///////////////////////
    // Mine Block 3
    ///////////////////////
    test::Tx block3_tx1 = test::Tx::CreatePegIn(1500);
    auto block3 = miner.MineBlock(161, { block3_tx1 });
    BOOST_REQUIRE(mw::Node::ValidateBlock(block3.GetBlock(), block3.GetHash(), {block3_tx1.GetPegInCoin()}, {}));
    mw::Node::ConnectBlock(block3.GetBlock(), pCachedView);

    const auto& block3_tx1_output1 = block3_tx1.GetOutputs()[0];
    BOOST_REQUIRE(pDBView->GetUTXOs(block3_tx1_output1.GetCommitment()).empty());
    BOOST_REQUIRE(pCachedView->GetUTXOs(block3_tx1_output1.GetCommitment()).size() == 1);

    ///////////////////////
    // Flush View
    ///////////////////////
    auto pBatch = pDatabase->CreateBatch();
    pCachedView->Flush(pBatch);
    pBatch->Commit();

    BOOST_REQUIRE(pDBView->GetUTXOs(block1_tx1_output1.GetCommitment()).size() == 1);
    BOOST_REQUIRE(pCachedView->GetUTXOs(block1_tx1_output1.GetCommitment()).size() == 1);
    BOOST_REQUIRE(pDBView->GetUTXOs(block2_tx1_output1.GetCommitment()).empty());
    BOOST_REQUIRE(pCachedView->GetUTXOs(block2_tx1_output1.GetCommitment()).empty());
    BOOST_REQUIRE(pDBView->GetUTXOs(block3_tx1_output1.GetCommitment()).size() == 1);
    BOOST_REQUIRE(pCachedView->GetUTXOs(block3_tx1_output1.GetCommitment()).size() == 1);
}

BOOST_AUTO_TEST_SUITE_END()