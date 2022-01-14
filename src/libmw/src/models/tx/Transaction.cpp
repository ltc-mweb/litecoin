#include <mw/models/tx/Transaction.h>
#include <mw/consensus/KernelSumValidator.h>
#include <mw/consensus/StealthSumValidator.h>

using namespace mw;

Transaction::CPtr Transaction::Create(
    BlindingFactor kernel_offset,
    BlindingFactor stealth_offset,
    std::vector<Input> inputs,
    std::vector<Output> outputs,
    std::vector<Kernel> kernels)
{
    std::sort(inputs.begin(), inputs.end(), InputSort);
    std::sort(outputs.begin(), outputs.end(), OutputSort);
    std::sort(kernels.begin(), kernels.end(), KernelSort);

    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(stealth_offset),
        TxBody{
            std::move(inputs),
            std::move(outputs),
            std::move(kernels)
        }
    );
}

bool Transaction::IsStandard() const noexcept
{
    for (const Kernel& kernel : GetKernels()) {
        if (!kernel.IsStandard()) {
            return false;
        }
    }

    return true;
}

void Transaction::Validate() const
{
    m_body.Validate();

    KernelSumValidator::ValidateForTx(*this);
    StealthSumValidator::Validate(m_stealthOffset, m_body);
}

std::string Transaction::Print() const noexcept
{
    auto print_kernel = [](const Kernel& kernel) -> std::string {
        return StringUtil::Format(
            "kern(kernel_id:{}, commit:{}, pegin: {}, pegout: {}, fee: {})",
            kernel.GetKernelID(),
            kernel.GetCommitment(),
            kernel.GetPegIn(),
            kernel.GetPegOut() ? kernel.GetPegOut().value().GetAmount() : 0,
            kernel.GetFee()
        );
    };
    std::string kernels_str = std::accumulate(
        GetKernels().begin(), GetKernels().end(), std::string{},
        [&print_kernel](std::string str, const Kernel& kern) {
            return str.empty() ? print_kernel(kern) : std::move(str) + ", " + print_kernel(kern);
        }
    );

    return StringUtil::Format(
        "tx(hash:{}, offset:{}, kernels:[{}], inputs:{}, outputs:{})",
        GetHash(),
        GetKernelOffset().ToHex(),
        kernels_str,
        GetInputCommits(),
        GetOutputCommits()
    );
}