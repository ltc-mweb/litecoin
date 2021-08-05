// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/consensus/Aggregation.h>

#include <test_framework/TestMWEB.h>
#include <test_framework/TxBuilder.h>

BOOST_FIXTURE_TEST_SUITE(TestAggregation, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(Aggregate)
{
    mw::Transaction::CPtr tx1 = test::TxBuilder()
        .AddInput(10).AddInput(20)
        .AddOutput(25).AddPlainKernel(5, true)
        .Build().GetTransaction();
    tx1->Validate();

    mw::Transaction::CPtr tx2 = test::TxBuilder()
        .AddInput(20)
        .AddOutput(15).AddPlainKernel(5, true)
        .Build().GetTransaction();
    tx2->Validate();

    mw::Transaction::CPtr pAggregated = Aggregation::Aggregate({ tx1, tx2 });
    pAggregated->Validate();

    std::vector<Input> inputs = tx1->GetInputs();
    inputs.insert(inputs.end(), tx2->GetInputs().begin(), tx2->GetInputs().end());
    std::sort(inputs.begin(), inputs.end(), SortByCommitment);
    BOOST_REQUIRE(pAggregated->GetInputs().size() == 3);
    BOOST_REQUIRE(pAggregated->GetInputs() == inputs);

    std::vector<Output> outputs = tx1->GetOutputs();
    outputs.insert(outputs.end(), tx2->GetOutputs().begin(), tx2->GetOutputs().end());
    std::sort(outputs.begin(), outputs.end(), SortByCommitment);
    BOOST_REQUIRE(pAggregated->GetOutputs().size() == 2);
    BOOST_REQUIRE(pAggregated->GetOutputs() == outputs);

    std::vector<Kernel> kernels = tx1->GetKernels();
    kernels.insert(kernels.end(), tx2->GetKernels().begin(), tx2->GetKernels().end());
    std::sort(kernels.begin(), kernels.end(), KernelSort);
    BOOST_REQUIRE(pAggregated->GetKernels().size() == 2);
    BOOST_REQUIRE(pAggregated->GetKernels() == kernels);

    std::vector<SignedMessage> owner_sigs = tx1->GetOwnerSigs();
    owner_sigs.insert(owner_sigs.end(), tx2->GetOwnerSigs().begin(), tx2->GetOwnerSigs().end());
    std::sort(owner_sigs.begin(), owner_sigs.end(), SortByHash);
    BOOST_REQUIRE(pAggregated->GetOwnerSigs().size() == 2);
    BOOST_REQUIRE(pAggregated->GetOwnerSigs() == owner_sigs);

    BlindingFactor kernel_offset = Blinds()
        .Add(tx1->GetKernelOffset())
        .Add(tx2->GetKernelOffset())
        .Total();
    BOOST_REQUIRE(pAggregated->GetKernelOffset() == kernel_offset);

    BlindingFactor owner_offset = Blinds()
        .Add(tx1->GetOwnerOffset())
        .Add(tx2->GetOwnerOffset())
        .Total();
    BOOST_REQUIRE(pAggregated->GetOwnerOffset() == owner_offset);
}

BOOST_AUTO_TEST_SUITE_END()