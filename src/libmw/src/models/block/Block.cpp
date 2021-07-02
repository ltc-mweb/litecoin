#include <mw/models/block/Block.h>

#include <mw/consensus/OwnerSumValidator.h>
#include <mw/mmr/MMR.h>

void mw::Block::Validate() const
{
    if (m_pHeader->GetNumKernels() != m_body.GetKernels().size()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }

    m_body.Validate();

    OwnerSumValidator::Validate(m_pHeader->GetOwnerOffset(), m_body);

    MemMMR kernel_mmr;
    std::for_each(
        GetKernels().cbegin(), GetKernels().cend(),
        [&kernel_mmr](const Kernel& kernel) { kernel_mmr.Add(kernel); }
    );
    if (m_pHeader->GetKernelRoot() != kernel_mmr.Root()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }
}