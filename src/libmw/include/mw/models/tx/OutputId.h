#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/PublicKey.h>
#include <mw/models/tx/Features.h>
#include <mw/crypto/Hasher.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>

////////////////////////////////////////
// OUTPUT IDENTIFIER
////////////////////////////////////////
class OutputId final :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    OutputId(
        Commitment commitment,
        Features features,
        PublicKey receiverPubKey,
        PublicKey exchangePubKey,
        uint8_t viewTag,
        uint64_t maskedValue,
        BigInt<16> maskedNonce,
        PublicKey senderPubKey
    )
        : m_commitment(std::move(commitment)),
        m_features(features),
        m_receiverPubKey(std::move(receiverPubKey)),
        m_keyExchangePubKey(std::move(exchangePubKey)),
        m_viewTag(viewTag),
        m_maskedValue(maskedValue),
        m_maskedNonce(std::move(maskedNonce)),
        m_senderPubKey(std::move(senderPubKey))
    {
        m_hash = Hashed(*this);
    }
    OutputId(const OutputId& output) = default;
    OutputId(OutputId&& output) noexcept = default;
    OutputId() = default;

    //
    // Operators
    //
    OutputId& operator=(const OutputId& OutputId) = default;
    OutputId& operator=(OutputId&& OutputId) noexcept = default;
    bool operator<(const OutputId& OutputId) const noexcept { return m_hash < OutputId.m_hash; }
    bool operator==(const OutputId& OutputId) const noexcept { return m_hash == OutputId.m_hash; }

    //
    // Getters
    //
    //Features GetFeatures() const noexcept { return m_features; }
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }

    bool IsPeggedIn() const noexcept { return m_features.IsSet(EOutputFeatures::PEGGED_IN); }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_commitment);
        READWRITE(m_features);
        READWRITE(m_receiverPubKey);
        READWRITE(m_keyExchangePubKey);
        READWRITE(m_viewTag);
        READWRITE(m_maskedValue);
        READWRITE(m_maskedNonce);
        READWRITE(m_senderPubKey);

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_hash; }

private:
    Commitment m_commitment;
    Features m_features;
    PublicKey m_receiverPubKey;
    PublicKey m_keyExchangePubKey;
    uint8_t m_viewTag;
    uint64_t m_maskedValue;
    BigInt<16> m_maskedNonce;
    PublicKey m_senderPubKey;

    mw::Hash m_hash;
};