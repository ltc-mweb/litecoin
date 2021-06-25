#pragma once

#include <mw/common/Traits.h>
#include <mw/util/StringUtil.h>
#include <script/script.h>
#include <util/strencodings.h>
#include <amount.h>

//
// Represents coins being pegged out, i.e. moved from the extension block to the canonical chain.
//
class PegOutCoin : public Traits::ISerializable, public Traits::IPrintable
{
public:
    PegOutCoin() = default;
    PegOutCoin(const CAmount amount, CScript scriptPubKey)
        : m_amount(amount), m_scriptPubKey(std::move(scriptPubKey)) { }

    bool operator==(const PegOutCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_scriptPubKey == rhs.m_scriptPubKey;
    }

    CAmount GetAmount() const noexcept { return m_amount; }
    const CScript& GetScriptPubKey() const noexcept { return m_scriptPubKey; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(PegOutCoin);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(m_amount, VarIntMode::NONNEGATIVE_SIGNED));
        READWRITE(m_scriptPubKey);
    }

    std::string Format() const noexcept final
    {
        return StringUtil::Format("PegOutCoin(pubkey:{}, amount:{})", HexStr(m_scriptPubKey), m_amount);
    }

private:
    CAmount m_amount;
    CScript m_scriptPubKey;
};