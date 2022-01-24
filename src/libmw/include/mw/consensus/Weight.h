#pragma once

#include <mw/consensus/Params.h>
#include <mw/models/tx/TxBody.h>
#include <numeric>
#include <utility>

class Weight
{
public:
    static constexpr size_t BYTES_PER_WEIGHT = 42;

    static constexpr size_t BASE_KERNEL_WEIGHT = 2;
    static constexpr size_t STEALTH_EXCESS_WEIGHT = 1;
    static constexpr size_t KERNEL_WITH_STEALTH_WEIGHT = BASE_KERNEL_WEIGHT + STEALTH_EXCESS_WEIGHT;

    static constexpr size_t BASE_OUTPUT_WEIGHT = 16;
    static constexpr size_t STANDARD_OUTPUT_FIELDS_WEIGHT = 2;
    static constexpr size_t STANDARD_OUTPUT_WEIGHT = BASE_OUTPUT_WEIGHT + STANDARD_OUTPUT_FIELDS_WEIGHT;

    static constexpr size_t MAX_NUM_INPUTS = 50'000;
    static constexpr size_t INPUT_BYTES = 195;
    static constexpr size_t MAX_MWEB_BYTES = 0; // MW: TODO - Calculate this

    static size_t Calculate(const TxBody& tx_body)
    {
        size_t kernel_weight = std::accumulate(
            tx_body.GetKernels().begin(), tx_body.GetKernels().end(), (size_t)0,
            [](size_t sum, const Kernel& kernel) {
                size_t kern_weight = CalcKernelWeight(
                    kernel.HasStealthExcess(),
                    kernel.HasPegOut() ? kernel.GetPegOut().value().GetScriptPubKey() : CScript(),
                    kernel.GetExtraData()
                );
                return sum + kern_weight;
            }
        );

        size_t output_weight = std::accumulate(
            tx_body.GetOutputs().begin(), tx_body.GetOutputs().end(), (size_t)0,
            [](size_t sum, const Output& output) {
                return sum + CalcOutputWeight(output.HasStandardFields(), output.GetExtraData());
            }
        );

        return kernel_weight + output_weight;
    }

    static bool ExceedsMaximum(const TxBody& tx_body)
    {
        return tx_body.GetInputs().size() > MAX_NUM_INPUTS || Calculate(tx_body) > mw::MAX_BLOCK_WEIGHT;
    }

    static size_t CalcKernelWeight(
        const bool stealth_excess,
        const CScript& scriptPubKey = CScript{},
        const std::vector<uint8_t>& extra_data = {})
    {
        // Kernels with a stealth excess cost extra.
        size_t base_weight = stealth_excess ? KERNEL_WITH_STEALTH_WEIGHT : BASE_KERNEL_WEIGHT;

        return base_weight + ExtraBytesToWeight(extra_data.size() + scriptPubKey.size());
    }

    // Outputs have a weight of 'BASE_OUTPUT_WEIGHT' plus 2 weight for standard fields, and 1 weight for every 'BYTES_PER_WEIGHT' extra_data bytes
    static size_t CalcOutputWeight(const bool standard_fields, const std::vector<uint8_t>& extra_data = {})
    {
        size_t base_weight = standard_fields ? STANDARD_OUTPUT_WEIGHT : BASE_OUTPUT_WEIGHT;
        return base_weight + ExtraBytesToWeight(extra_data.size());
    }

private:
    static size_t ExtraBytesToWeight(const size_t extra_bytes)
    {
        return (extra_bytes + (BYTES_PER_WEIGHT - 1)) / BYTES_PER_WEIGHT;
    }
};