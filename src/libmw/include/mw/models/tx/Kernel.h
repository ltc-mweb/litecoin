#pragma once

#include <mw/common/Macros.h>
#include <mw/common/Traits.h>
#include <mw/crypto/Hasher.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/tx/KernelType.h>
#include <amount.h>
#include <boost/optional.hpp>

class Kernel :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
    enum FeatureBit {
        FEE_FEATURE_BIT         = 0x01,
        PEGIN_FEATURE_BIT       = 0x02,
        PEGOUT_FEATURE_BIT      = 0x04,
        HEIGHT_LOCK_FEATURE_BIT = 0x08,
        EXTRA_DATA_FEATURE_BIT  = 0x10
    };
public:
    Kernel() = default;
    Kernel(
        boost::optional<CAmount> fee,
        boost::optional<CAmount> pegin,
        boost::optional<PegOutCoin> pegout,
        boost::optional<int32_t> lockHeight,
        std::vector<uint8_t> extraData,
        Commitment&& excess,
        Signature&& signature
    ) : m_fee(fee),
        m_pegin(pegin),
        m_pegout(std::move(pegout)),
        m_lockHeight(lockHeight),
        m_extraData(std::move(extraData)),
        m_excess(std::move(excess)),
        m_signature(std::move(signature))
    {
        m_hash = Hashed(*this);
    }

    //
    // Factories
    //
    static Kernel Create(
        const BlindingFactor& blind,
        const boost::optional<CAmount>& fee,
        const boost::optional<CAmount>& pegin_amount,
        const boost::optional<PegOutCoin>& pegout,
        const boost::optional<int32_t>& lock_height
    );

    //
    // Operators
    //
    bool operator<(const Kernel& rhs) const { return GetHash() < rhs.GetHash(); }
    bool operator==(const Kernel& rhs) const { return GetHash() == rhs.GetHash(); }
    bool operator!=(const Kernel& rhs) const { return GetHash() != rhs.GetHash(); }

    //
    // Getters
    //
    CAmount GetFee() const noexcept { return m_fee.value_or(0); }
    int32_t GetLockHeight() const noexcept { return m_lockHeight.value_or(0); }
    const Commitment& GetExcess() const noexcept { return m_excess; }
    const Signature& GetSignature() const noexcept { return m_signature; }
    const std::vector<uint8_t>& GetExtraData() const noexcept { return m_extraData; }

    mw::Hash GetSignatureMessage() const;
    static mw::Hash GetSignatureMessage(
        const boost::optional<CAmount>& fee,
        const boost::optional<CAmount>& pegin_amount,
        const boost::optional<PegOutCoin>& pegout,
        const boost::optional<int32_t>& lock_height,
        const std::vector<uint8_t>& extra_data
    );

    bool HasPegIn() const noexcept { return !!m_pegin; }
    bool HasPegOut() const noexcept { return !!m_pegout; }

    CAmount GetPegIn() const noexcept { return m_pegin.value_or(0); }
    const boost::optional<PegOutCoin>& GetPegOut() const noexcept { return m_pegout; }

    CAmount GetSupplyChange() const noexcept
    {
        return (m_pegin.value_or(0) - m_fee.value_or(0)) -
            (m_pegout ? m_pegout.value().GetAmount() : 0);
    }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(Kernel);

    template <typename Stream>
    void Serialize(Stream& s) const
    {
        uint8_t features_byte =
            (m_fee ? FEE_FEATURE_BIT : 0) |
            (m_pegin ? PEGIN_FEATURE_BIT : 0) |
            (m_pegout ? PEGOUT_FEATURE_BIT : 0) |
            (m_lockHeight ? HEIGHT_LOCK_FEATURE_BIT : 0) |
            (m_extraData.size() > 0 ? EXTRA_DATA_FEATURE_BIT : 0);
        s << features_byte;

        if (m_fee) {
            ::WriteVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s, m_fee.value());
        }

        if (m_pegin) {
            ::WriteVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s, m_pegin.value());
        }

        if (m_pegout) {
            s << m_pegout.value();
        }

        if (m_lockHeight) {
            ::WriteVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, int32_t>(s, m_lockHeight.value());
        }

        if (!m_extraData.empty()) {
            s << m_extraData;
        }

        s << m_excess << m_signature;
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        uint8_t features_byte;
        s >> features_byte; // MW: TODO - Verify it's less than 32? Maybe only check in IsStandardTx?

        if (features_byte & FEE_FEATURE_BIT) {
            m_fee = ::ReadVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s);
        }

        if (features_byte & PEGIN_FEATURE_BIT) {
            m_pegin = ::ReadVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, CAmount>(s);
        }

        if (features_byte & PEGOUT_FEATURE_BIT) {
            PegOutCoin pegout;
            s >> pegout;
            m_pegout = boost::make_optional(std::move(pegout));
        }

        if (features_byte & HEIGHT_LOCK_FEATURE_BIT) {
            m_lockHeight = ::ReadVarInt<Stream, VarIntMode::NONNEGATIVE_SIGNED, int32_t>(s);
        }

        if (features_byte & EXTRA_DATA_FEATURE_BIT) {
            s >> m_extraData;
        }

        s >> m_excess >> m_signature;

        m_hash = Hashed(*this);
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

    const Commitment& GetCommitment() const noexcept final { return m_excess; }

private:
    boost::optional<CAmount> m_fee;
    boost::optional<CAmount> m_pegin;
    boost::optional<PegOutCoin> m_pegout;
    boost::optional<int32_t> m_lockHeight;
    std::vector<uint8_t> m_extraData;

    // Remainder of the sum of all transaction commitments. 
    // If the transaction is well formed, amounts components should sum to zero and the excess is hence a valid public key.
    Commitment m_excess;

    // The signature proving the excess is a valid public key, which signs the transaction fee.
    Signature m_signature;

    mw::Hash m_hash;
};

// Sorts by net supply increase [pegin - (fee + pegout)] with highest increase first, then sorts by hash.
static const struct
{
    bool operator()(const Kernel& a, const Kernel& b) const
    {
        CAmount a_pegin = a.GetSupplyChange();
        CAmount b_pegin = b.GetSupplyChange();
        return (a_pegin > b_pegin) || (a_pegin == b_pegin && a.GetHash() < b.GetHash());
    }
} KernelSort;