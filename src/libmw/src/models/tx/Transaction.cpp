#include <mw/models/tx/Transaction.h>
#include <mw/consensus/KernelSumValidator.h>
#include <mw/consensus/OwnerSumValidator.h>
#include <numeric>

void mw::Transaction::Validate() const
{
    m_body.Validate();

    KernelSumValidator::ValidateForTx(*this);
    OwnerSumValidator::Validate(m_ownerOffset, m_body);
}

std::string mw::Transaction::Print() const noexcept
{
    auto print_kernel = [](const Kernel& kernel) -> std::string {
        return StringUtil::Format(
            "kern(commit:{}, pegin: {}, pegout: {}, fee: {})",
            kernel.GetCommitment(),
            kernel.GetPegIn(),
            kernel.GetPegOut().has_value() ? kernel.GetPegOut().value().GetAmount() : 0,
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