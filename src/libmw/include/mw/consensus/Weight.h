#pragma once

#include <mw/consensus/Params.h>
#include <mw/models/tx/TxBody.h>
#include <numeric>

class Weight
{
public:
    static constexpr size_t KERNEL_WEIGHT = 2;
    static constexpr size_t STEALTH_EXCESS_WEIGHT = 1;
    static constexpr size_t OUTPUT_WEIGHT = 18;

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

    static size_t Calculate(const size_t num_kernels, const size_t num_stealth_excesses, const size_t num_outputs)
    {
        return (num_kernels * KERNEL_WEIGHT)
            + (num_stealth_excesses * STEALTH_EXCESS_WEIGHT)
            + (num_outputs * OUTPUT_WEIGHT);
    }

    static size_t Calculate(const TxBody& tx_body)
    {
        size_t num_stealth_commits = std::accumulate(
            tx_body.GetKernels().begin(), tx_body.GetKernels().end(), (size_t)0,
            [](size_t sum, const Kernel& kern) {
                return kern.HasStealthExcess() ? sum + 1 : sum;
            }
        );

        return Calculate(
            tx_body.GetKernels().size(),
            num_stealth_commits,
            tx_body.GetOutputs().size()
        );
    }

    static bool ExceedsMaximum(const TxBody& tx_body)
    {
        return Calculate(tx_body) > mw::MAX_BLOCK_WEIGHT;
    }
};