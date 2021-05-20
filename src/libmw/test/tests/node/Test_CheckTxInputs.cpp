// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <libmw/libmw.h>
#include <mw/crypto/Hasher.h>
#include <mw/file/ScopedFileRemover.h>
#include <mw/mmr/backends/FileBackend.h>
#include <mw/node/CoinsView.h>
#include <mw/node/INode.h>

#include <test_framework/DBWrapper.h>
#include <test_framework/Miner.h>
#include <test_framework/TestUtil.h>
#include <test_framework/TxBuilder.h>

BOOST_FIXTURE_TEST_SUITE(TestCheckTxInputs, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(CheckTxInputs)
{
    FilePath datadir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(datadir);

    auto pDatabase = std::make_shared<TestDBWrapper>();
    auto pNode = mw::InitializeNode(datadir, nullptr, pDatabase);
    BOOST_REQUIRE(pNode != nullptr);

    auto pDBView = pNode->GetDBView();
    auto pCachedView = libmw::CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pDBView) };

    test::Miner miner;
    uint64_t height = 100;

    // Mine pegin tx
    test::Tx tx1 = test::Tx::CreatePegIn(1000);
    auto block = miner.MineBlock(height, { tx1 });
    pNode->ValidateBlock(block.GetBlock(), block.GetHash(), { tx1.GetPegInCoin() }, {});
    pNode->ConnectBlock(block.GetBlock(), pCachedView.pCoinsView);
    height += libmw::PEGIN_MATURITY;

    // Try to spend the pegin output
    const auto& output1 = tx1.GetOutputs().front();
    test::Tx tx2 = test::TxBuilder().AddInput(output1).AddPlainKernel(0).AddOutput(1000).Build();
    auto transaction = tx2.GetTransaction();
    BOOST_REQUIRE(libmw::node::CheckTransaction(transaction));
    BOOST_REQUIRE(!libmw::node::CheckTxInputs(pCachedView, transaction, height - 1));
    BOOST_REQUIRE(libmw::node::CheckTxInputs(pCachedView, transaction, height));

    // Try to spend an unknown pegin output
    test::Tx tx3 = test::Tx::CreatePegIn(1000);
    const auto& output2 = tx3.GetOutputs().front();
    test::Tx tx4 = test::TxBuilder().AddInput(output2).AddPlainKernel(0).AddOutput(1000).Build();
    transaction = tx4.GetTransaction();
    BOOST_REQUIRE(libmw::node::CheckTransaction(transaction));
    BOOST_REQUIRE(!libmw::node::CheckTxInputs(pCachedView, transaction, height - 1));
    BOOST_REQUIRE(!libmw::node::CheckTxInputs(pCachedView, transaction, height));

    pNode.reset();
}

BOOST_AUTO_TEST_SUITE_END()