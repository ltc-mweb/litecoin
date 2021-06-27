#include <mw/models/tx/Transaction.h>
#include <mw/consensus/KernelSumValidator.h>
#include <mw/consensus/OwnerSumValidator.h>

using namespace mw;

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
    OwnerSumValidator::Validate(m_ownerOffset, m_body);
}