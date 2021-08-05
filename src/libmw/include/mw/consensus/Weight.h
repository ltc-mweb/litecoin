#pragma once

#include <mw/consensus/Params.h>
#include <mw/models/tx/TxBody.h>

class Weight
{
    static constexpr size_t KERNEL_WEIGHT = 2;
    static constexpr size_t OWNER_SIG_WEIGHT = 1;
    static constexpr size_t OUTPUT_WEIGHT = 18;

public:
    struct Arguments
    {
        size_t num_kernels;
        size_t num_owner_sigs;
        size_t num_outputs;
    };

    static size_t Calculate(const Arguments& args)
    {
        return Calculate(args.num_kernels, args.num_owner_sigs, args.num_outputs);
    }

    static size_t Calculate(const size_t num_kernels, const size_t num_owner_sigs, const size_t num_outputs)
    {
        return (num_kernels * KERNEL_WEIGHT)
            + (num_owner_sigs * OWNER_SIG_WEIGHT)
            + (num_outputs * OUTPUT_WEIGHT);
    }

    static size_t Calculate(const TxBody& tx_body)
    {
        return Calculate(
            tx_body.GetKernels().size(),
            tx_body.GetOwnerSigs().size(),
            tx_body.GetOutputs().size()
        );
    }

    static bool ExceedsMaximum(const TxBody& tx_body)
    {
        return Calculate(tx_body) > mw::MAX_BLOCK_WEIGHT;
    }
};