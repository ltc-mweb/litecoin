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

    MemMMR::Ptr pKernelMMR = std::make_shared<MemMMR>();
    std::for_each(
        GetKernels().cbegin(), GetKernels().cend(),
        [&pKernelMMR](const Kernel& kernel) { pKernelMMR->Add(kernel); }
    );
    if (m_pHeader->GetKernelRoot() != pKernelMMR->Root()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }
}