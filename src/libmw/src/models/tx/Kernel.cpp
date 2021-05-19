#include <mw/models/tx/Kernel.h>
#include <mw/models/tx/KernelBits.h>
#include <mw/crypto/Schnorr.h>

Kernel Kernel::Create(
    const BlindingFactor& blind,
    const boost::optional<uint64_t>& fee,
    const boost::optional<uint64_t>& pegin_amount,
    const boost::optional<PegOutCoin>& pegout,
    const boost::optional<uint64_t>& lock_height)
{
    Commitment excess_commit = Commitment::Blinded(blind, 0);

    mw::Hash sig_message = Kernel::GetSignatureMessage(fee, pegin_amount, pegout, lock_height, {});
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        fee,
        pegin_amount,
        pegout,
        lock_height,
        std::vector<uint8_t>{},
        std::move(excess_commit),
        std::move(sig)
    );
}

mw::Hash Kernel::GetSignatureMessage() const
{
    return Kernel::GetSignatureMessage(m_fee, m_pegin, m_pegout, m_lockHeight, m_extraData);
}

mw::Hash Kernel::GetSignatureMessage(
    const boost::optional<uint64_t>& fee,
    const boost::optional<uint64_t>& pegin_amount,
    const boost::optional<PegOutCoin>& pegout,
    const boost::optional<uint64_t>& lock_height,
    const std::vector<uint8_t>& extra_data)
{
    uint8_t features_byte =
        (fee ? FEE_FEATURE_BIT : 0) |
        (pegin_amount ? PEGIN_FEATURE_BIT : 0) |
        (pegout ? PEGOUT_FEATURE_BIT : 0) |
        (lock_height ? HEIGHT_LOCK_FEATURE_BIT : 0) |
        (extra_data.size() > 0 ? EXTRA_DATA_FEATURE_BIT : 0);

    Hasher sig_message_hasher;
    sig_message_hasher.Append<uint8_t>(features_byte);

    if (fee) {
        sig_message_hasher.Append<uint64_t>(fee.value());
    }

    if (pegin_amount) {
        sig_message_hasher.Append<uint64_t>(pegin_amount.value());
    }

    if (pegout) {
        sig_message_hasher
            .Append<uint64_t>(pegout.value().GetAmount())
            .Append(pegout.value().GetScriptPubKey());
    }

    if (lock_height) {
        sig_message_hasher.Append<uint64_t>(lock_height.value());
    }

    if (!extra_data.empty()) {
        sig_message_hasher.Append(extra_data);
    }

    return sig_message_hasher.hash();
}