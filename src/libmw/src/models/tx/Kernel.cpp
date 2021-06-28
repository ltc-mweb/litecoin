#include <mw/models/tx/Kernel.h>
#include <mw/crypto/Schnorr.h>

Kernel Kernel::Create(
    const BlindingFactor& blind,
    const boost::optional<CAmount>& fee,
    const boost::optional<CAmount>& pegin_amount,
    const boost::optional<PegOutCoin>& pegout,
    const boost::optional<int32_t>& lock_height)
{
    Commitment excess_commit = Commitment::Blinded(blind, 0);

    const uint8_t features_byte =
        (fee ? FEE_FEATURE_BIT : 0) |
        (pegin_amount ? PEGIN_FEATURE_BIT : 0) |
        (pegout ? PEGOUT_FEATURE_BIT : 0) |
        (lock_height ? HEIGHT_LOCK_FEATURE_BIT : 0);
    mw::Hash sig_message = Kernel::GetSignatureMessage(features_byte, fee, pegin_amount, pegout, lock_height, {});
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        features_byte,
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
    return Kernel::GetSignatureMessage(m_features, m_fee, m_pegin, m_pegout, m_lockHeight, m_extraData);
}

mw::Hash Kernel::GetSignatureMessage(
    const uint8_t features,
    const boost::optional<CAmount>& fee,
    const boost::optional<CAmount>& pegin_amount,
    const boost::optional<PegOutCoin>& pegout,
    const boost::optional<int32_t>& lock_height,
    const std::vector<uint8_t>& extra_data)
{
    Hasher s;
    s << features;

    if (fee) {
        ::WriteVarInt<Hasher, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s, fee.value());
    }

    if (pegin_amount) {
        ::WriteVarInt<Hasher, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s, pegin_amount.value());
    }

    if (pegout) {
        s << pegout.value();
    }

    if (lock_height) {
        ::WriteVarInt<Hasher, VarIntMode::NONNEGATIVE_SIGNED, int32_t>(s, lock_height.value());
    }

    if (!extra_data.empty()) {
        s << extra_data;
    }

    return s.hash();
}