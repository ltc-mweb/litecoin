// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/node/BlockValidator.h>

#include <test_framework/Miner.h>
#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestBlockValidator, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(BlockValidator_Test)
{
    test::Miner miner(GetDataDir());

    // Block 10
    std::vector<test::Tx> block_10_txs{
        test::Tx::CreatePegIn(5'000'000)
    };

    std::vector<PegInCoin> pegInCoins{
        PegInCoin(5'000'000, block_10_txs[0].GetTransaction()->GetKernels()[0].GetCommitment())
    };
    std::vector<PegOutCoin> pegOutCoins;
    test::MinedBlock block_10 = miner.MineBlock(10, block_10_txs);

    BOOST_REQUIRE(BlockValidator::ValidateBlock(block_10.GetBlock(), block_10.GetHash(), pegInCoins, pegOutCoins));
}

BOOST_AUTO_TEST_SUITE_END()