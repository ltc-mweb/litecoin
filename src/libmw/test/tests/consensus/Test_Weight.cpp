// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/consensus/Weight.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestWeight, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(ExceedsMaximum)
{
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Kernel> kernels;
    std::vector<SignedMessage> sigs;

    for (int i = 0; i < 1000; i++) {
        inputs.push_back(Input());
        kernels.push_back(Kernel());
        outputs.push_back(Output());
        sigs.push_back(SignedMessage());
    }
    TxBody tx1(inputs, outputs, kernels, sigs);

    // 1,000 outputs - 1,000 kernels - 1,000 owner_sigs = 21,000 Weight
    BOOST_REQUIRE(Weight::Calculate(tx1) == 21'000);
    BOOST_REQUIRE(!Weight::ExceedsMaximum(tx1));

    // 1,000 outputs - 1,001 kernels - 1,000 owner_sigs = 21,002 Weight
    inputs.push_back(Input());
    kernels.push_back(Kernel());
    TxBody tx2(inputs, outputs, kernels, sigs);
    BOOST_REQUIRE(Weight::Calculate(tx2) == 21'002);
    BOOST_REQUIRE(Weight::ExceedsMaximum(tx2));
}

BOOST_AUTO_TEST_SUITE_END()