#pragma once

#include <mw/traits/Serializable.h>
#include <mw/traits/Printable.h>
#include <mw/util/StringUtil.h>

//
// Represents coins being pegged out, i.e. moved from the extension block to the canonical chain.
//
class PegOutCoin : public Traits::ISerializable, public Traits::IPrintable
{
public:
    PegOutCoin() = default;
    PegOutCoin(const uint64_t amount, const std::vector<uint8_t>& scriptPubKey)
        : m_amount(amount), m_scriptPubKey(scriptPubKey) { }
    PegOutCoin(const uint64_t amount, std::vector<uint8_t>&& scriptPubKey)
        : m_amount(amount), m_scriptPubKey(std::move(scriptPubKey)) { }

    bool operator==(const PegOutCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_scriptPubKey == rhs.m_scriptPubKey;
    }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const std::vector<uint8_t>& GetScriptPubKey() const noexcept { return m_scriptPubKey; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_amount);
        READWRITE(m_scriptPubKey);
    }

    std::string Format() const noexcept final
    {
        return StringUtil::Format("PegOutCoin(pubkey:{}, amount:{})", HexUtil::ToHex(m_scriptPubKey), m_amount);
    }

private:
    uint64_t m_amount;
    std::vector<uint8_t> m_scriptPubKey;
};