#include <mw/models/tx/TxBody.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/consensus/Params.h>
#include <mw/consensus/Weight.h>

#include <unordered_set>
#include <numeric>

std::vector<PegInCoin> TxBody::GetPegIns() const noexcept
{
    std::vector<PegInCoin> pegins;
    for (const Kernel& kernel : m_kernels) {
        if (kernel.HasPegIn()) {
            pegins.push_back(PegInCoin(kernel.GetPegIn(), kernel.GetHash()));
        }
    }

    return pegins;
}

CAmount TxBody::GetPegInAmount() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (CAmount)0,
        [](const CAmount sum, const auto& kernel) noexcept { return sum + kernel.GetPegIn(); }
    );
}

std::vector<PegOutCoin> TxBody::GetPegOuts() const noexcept
{
    std::vector<PegOutCoin> pegouts;
    for (const Kernel& kernel : m_kernels) {
        if (kernel.HasPegOut()) {
            pegouts.push_back(kernel.GetPegOut().value());
        }
    }
    return pegouts;
}

CAmount TxBody::GetTotalFee() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (CAmount)0,
        [](const CAmount sum, const auto& kernel) noexcept { return sum + kernel.GetFee(); }
    );
}

CAmount TxBody::GetSupplyChange() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (CAmount)0,
        [](const CAmount supply_change, const auto& kernel) noexcept { return supply_change + kernel.GetSupplyChange(); }
    );
}

int32_t TxBody::GetLockHeight() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (int32_t)0,
        [](const int32_t lock_height, const auto& kernel) noexcept { return std::max(lock_height, kernel.GetLockHeight()); }
    );
}

void TxBody::Validate() const
{
    // Verify weight
    if (Weight::ExceedsMaximum(*this)) {
        ThrowValidation(EConsensusError::BLOCK_WEIGHT);
    }

    // Verify no kernel's extra_data or pegout scriptPubKey exceeds max size
    bool extra_data_exceeds_max = std::any_of(
        m_kernels.cbegin(), m_kernels.cend(),
        [](const Kernel& kernel) {
            size_t pubkeySize = kernel.GetPegOut() ? kernel.GetPegOut().value().GetScriptPubKey().size() : 4;
            return pubkeySize > 42 || pubkeySize < 4 || kernel.GetExtraData().size() > mw::MAX_KERNEL_EXTRADATA_SIZE;
        }
    );
    if (extra_data_exceeds_max) {
        ThrowValidation(EConsensusError::BLOCK_WEIGHT);
    }

    // Verify inputs, outputs, kernels, and owner signatures are sorted
    if (!std::is_sorted(m_inputs.cbegin(), m_inputs.cend(), InputSort)
        || !std::is_sorted(m_outputs.cbegin(), m_outputs.cend(), OutputSort)
        || !std::is_sorted(m_kernels.cbegin(), m_kernels.cend(), KernelSort))
    {
        ThrowValidation(EConsensusError::NOT_SORTED);
    }

    auto contains_duplicates = [](const std::vector<mw::Hash>& hashes) -> bool {
        std::unordered_set<mw::Hash> set_hashes;
        std::copy(hashes.begin(), hashes.end(), std::inserter(set_hashes, set_hashes.end()));
        return set_hashes.size() != hashes.size();
    };

    // Verify no duplicate spends
    if (contains_duplicates(GetSpentHashes())) {
        ThrowValidation(EConsensusError::DUPLICATES);
    }

    // Verify no duplicate output hashes
    if (contains_duplicates(GetOutputHashes())) {
        ThrowValidation(EConsensusError::DUPLICATES);
    }

    // Verify no duplicate kernel hashes
    if (contains_duplicates(GetKernelHashes())) {
        ThrowValidation(EConsensusError::DUPLICATES);
    }

    //
    // Verify all signatures
    //
    std::vector<SignedMessage> signatures;
    std::transform(
        m_kernels.cbegin(), m_kernels.cend(), std::back_inserter(signatures),
        [](const Kernel& kernel) { return kernel.BuildSignedMsg(); }
    );

    std::transform(
        m_inputs.cbegin(), m_inputs.cend(), std::back_inserter(signatures),
        [](const Input& input) { return input.BuildSignedMsg(); }
    );

    std::transform(
        m_outputs.cbegin(), m_outputs.cend(), std::back_inserter(signatures),
        [](const Output& output) { return output.BuildSignedMsg(); }
    );

    if (!Schnorr::BatchVerify(signatures)) {
        ThrowValidation(EConsensusError::INVALID_SIG);
    }

    //
    // Verify RangeProofs
    //
    std::vector<ProofData> rangeProofs;
    std::transform(
        m_outputs.cbegin(), m_outputs.cend(), std::back_inserter(rangeProofs),
        [](const Output& output) { return output.BuildProofData(); }
    );
    if (!Bulletproofs::BatchVerify(rangeProofs)) {
        ThrowValidation(EConsensusError::BULLETPROOF);
    }
}