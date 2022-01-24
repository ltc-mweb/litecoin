// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/consensus/Weight.h>
#include <random.h>

#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestWeight, MWEBTestingSetup)

static Kernel CreateKernel(const bool with_stealth, const std::vector<PegOutCoin>& pegouts = {})
{
    return Kernel(
        0,
        boost::none,
        boost::none,
        pegouts,
        boost::none,
        with_stealth ? boost::make_optional(PublicKey()) : boost::none,
        std::vector<uint8_t>{},
        Commitment(),
        Signature()
    );
}

static Output CreateStandardOutput()
{
    OutputMessage message;
    message.features = (uint8_t)GetRand(UINT8_MAX) | OutputMessage::STANDARD_FIELDS_FEATURE_BIT;

    return Output(
        Commitment{},
        PublicKey{},
        PublicKey{},
        std::move(message),
        std::make_shared<RangeProof>(),
        Signature{}
    );
}

BOOST_AUTO_TEST_CASE(ExceedsMaximum)
{
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Kernel> kernels;

    for (int i = 0; i < 1000; i++) {
        inputs.push_back(Input());
        kernels.push_back(CreateKernel(true));
        outputs.push_back(CreateStandardOutput());
    }
    TxBody tx1(inputs, outputs, kernels);

    // 1,000 outputs - 1,000 kernels with stealth excesses = 21,000 Weight
    BOOST_REQUIRE(Weight::Calculate(tx1) == 21'000);
    BOOST_REQUIRE(!Weight::ExceedsMaximum(tx1));

    // 1,000 outputs - 1,001 kernels (1,000 with stealth excesses) = 21,002 Weight
    inputs.push_back(Input());
    kernels.push_back(CreateKernel(false));
    TxBody tx2(inputs, outputs, kernels);
    BOOST_REQUIRE(Weight::Calculate(tx2) == 21'002);
    BOOST_REQUIRE(Weight::ExceedsMaximum(tx2));
}

BOOST_AUTO_TEST_SUITE_END()