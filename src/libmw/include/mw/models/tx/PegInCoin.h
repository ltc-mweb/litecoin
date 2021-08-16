#pragma once

#include <mw/common/Traits.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/util/StringUtil.h>
#include <amount.h>

//
// Represents coins being pegged in, i.e. moved from canonical chain to the extension block.
//
class PegInCoin : public Traits::ISerializable, public Traits::IPrintable
{
public:
    PegInCoin() = default;
    PegInCoin(const CAmount amount, Commitment commitment)
        : m_amount(amount), m_commitment(std::move(commitment)) { }

    bool operator==(const PegInCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_commitment == rhs.m_commitment;
    }

    CAmount GetAmount() const noexcept { return m_amount; }
    const Commitment& GetCommitment() const noexcept { return m_commitment; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(PegInCoin, obj)
    {
        READWRITE(VARINT_MODE(obj.m_amount, VarIntMode::NONNEGATIVE_SIGNED));
        READWRITE(obj.m_commitment);
    }

    std::string Format() const noexcept final
    {
        return StringUtil::Format("PegInCoin(commitment: {}, amount: {})", m_commitment, m_amount);
    }

private:
    CAmount m_amount;
    Commitment m_commitment;
};