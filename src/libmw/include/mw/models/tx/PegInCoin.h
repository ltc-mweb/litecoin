#pragma once

#include <mw/common/Traits.h>
#include <mw/models/crypto/Hash.h>
#include <mw/util/StringUtil.h>
#include <amount.h>

//
// Represents coins being pegged in, i.e. moved from canonical chain to the extension block.
//
class PegInCoin : public Traits::ISerializable, public Traits::IPrintable
{
public:
    PegInCoin() = default;
    PegInCoin(const CAmount amount, mw::Hash kernel_hash)
        : m_amount(amount), m_kernelHash(std::move(kernel_hash)) { }

    bool operator==(const PegInCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_kernelHash == rhs.m_kernelHash;
    }

    CAmount GetAmount() const noexcept { return m_amount; }
    const mw::Hash& GetKernelHash() const noexcept { return m_kernelHash; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(PegInCoin, obj)
    {
        READWRITE(VARINT_MODE(obj.m_amount, VarIntMode::NONNEGATIVE_SIGNED));
        READWRITE(obj.m_kernelHash);
    }

    std::string Format() const noexcept final
    {
        return StringUtil::Format("PegInCoin(kernel_hash: {}, amount: {})", m_kernelHash, m_amount);
    }

private:
    CAmount m_amount;
    mw::Hash m_kernelHash;
};