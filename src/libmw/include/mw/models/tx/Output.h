#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Features.h>
#include <mw/models/tx/OutputId.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/ProofData.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/SignedMessage.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Serializable.h>

// Forward Declarations
class StealthAddress;

////////////////////////////////////////
// OUTPUT
////////////////////////////////////////
class Output :
    public Traits::ICommitted,
    public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    Output(
        Commitment&& commitment,
        Features features,
        PublicKey&& receiver_pubkey,
        PublicKey&& key_exchange_pubkey,
        uint8_t view_tag,
        uint64_t masked_value,
        BigInt<16>&& masked_nonce,
        PublicKey&& sender_pubkey,
        Signature&& signature,
        const RangeProof::CPtr& pProof
    ) :
        m_commitment(std::move(commitment)),
        m_features(features),
        m_receiverPubKey(std::move(receiver_pubkey)),
        m_keyExchangePubKey(std::move(key_exchange_pubkey)),
        m_viewTag(view_tag),
        m_maskedValue(masked_value),
        m_maskedNonce(std::move(masked_nonce)),
        m_senderPubKey(std::move(sender_pubkey)),
        m_signature(std::move(signature)),
        m_pProof(pProof)
    {
        m_hash = Hashed(*this);
    }

    Output(const Output& Output) = default;
    Output(Output&& Output) noexcept = default;
    Output() = default;

    //
    // Factory
    //
    static Output Create(
        BlindingFactor& blind_out,
        const Features& features,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const uint64_t value
    );

    //
    // Destructor
    //
    virtual ~Output() = default;

    //
    // Operators
    //
    Output& operator=(const Output& Output) = default;
    Output& operator=(Output&& Output) noexcept = default;
    bool operator<(const Output& Output) const noexcept { return m_hash < Output.m_hash; }
    bool operator==(const Output& Output) const noexcept { return m_hash == Output.m_hash; }

    //
    // Getters
    //
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }
    const RangeProof::CPtr& GetRangeProof() const noexcept { return m_pProof; }

    Features GetFeatures() const noexcept { return m_features; }
    const PublicKey& GetReceiverPubKey() const noexcept { return m_receiverPubKey; }
    const PublicKey& GetKeyExchangePubKey() const noexcept { return m_keyExchangePubKey; }
    uint8_t GetViewTag() const noexcept { return m_viewTag; }
    uint64_t GetMaskedValue() const noexcept { return m_maskedValue; }
    const BigInt<16>& GetMaskedNonce() const noexcept { return m_maskedNonce; }
    const PublicKey& GetSenderPubKey() const noexcept { return m_senderPubKey; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    const PublicKey& Ko() const noexcept { return m_receiverPubKey; }
    const PublicKey& Ke() const noexcept { return m_keyExchangePubKey; }

    SignedMessage BuildSignedMsg() const noexcept;
    ProofData BuildProofData() const noexcept;

    bool IsPeggedIn() const noexcept { return GetFeatures().IsSet(EOutputFeatures::PEGGED_IN); }

    OutputId ToOutputId() const noexcept {
        return OutputId(
            m_commitment,
            m_features,
            m_receiverPubKey,
            m_keyExchangePubKey,
            m_viewTag,
            m_maskedValue,
            m_maskedNonce,
            m_senderPubKey
        );
    }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append(m_commitment)
            .Append<uint8_t>(m_features.Get())
            .Append(m_receiverPubKey)
            .Append(m_keyExchangePubKey)
            .Append(m_viewTag)
            .Append(m_maskedValue)
            .Append(m_maskedNonce)
            .Append(m_senderPubKey)
            .Append(m_signature)
            .Append(m_pProof);
    }

    static Output Deserialize(Deserializer& deserializer)
    {
        Commitment commitment = Commitment::Deserialize(deserializer);

        EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
        PublicKey receiver_pubkey = PublicKey::Deserialize(deserializer);
        PublicKey key_exchange_pubkey = PublicKey::Deserialize(deserializer);
        uint8_t view_tag = deserializer.Read<uint8_t>();
        uint64_t masked_value = deserializer.Read<uint64_t>();
        BigInt<16> masked_nonce = BigInt<16>::Deserialize(deserializer);
        PublicKey sender_pubkey = PublicKey::Deserialize(deserializer);
        Signature signature = Signature::Deserialize(deserializer);

        RangeProof::CPtr pProof = std::make_shared<const RangeProof>(RangeProof::Deserialize(deserializer));
        return Output(
            std::move(commitment),
            features,
            std::move(receiver_pubkey),
            std::move(key_exchange_pubkey),
            view_tag,
            masked_value,
            std::move(masked_nonce),
            std::move(sender_pubkey),
            std::move(signature),
            pProof
        );
    }

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
        READWRITE(m_signature);
        READWRITE(m_pProof);
    }

private:
    // The homomorphic commitment representing the output amount
    Commitment m_commitment;

    Features m_features;
    PublicKey m_receiverPubKey;
    PublicKey m_keyExchangePubKey;
    uint8_t m_viewTag;
    uint64_t m_maskedValue;
    BigInt<16> m_maskedNonce;
    PublicKey m_senderPubKey;
    Signature m_signature;

    // A proof that the commitment is in the right range
    RangeProof::CPtr m_pProof;

    mw::Hash m_hash;
};