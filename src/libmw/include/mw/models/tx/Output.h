#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Traits.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/ProofData.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/SignedMessage.h>

// Forward Declarations
class StealthAddress;

////////////////////////////////////////
// OUTPUT MESSAGE
////////////////////////////////////////
class OutputMessage : public Traits::ISerializable, public Traits::IHashable
{
public:
    //
    // Constructors
    //
    OutputMessage(
        PublicKey receiverPubKey_,
        PublicKey keyExchangePubKey_,
        uint8_t viewTag_,
        uint64_t maskedValue_,
        BigInt<16> maskedNonce_,
        PublicKey senderPubKey_
    ) : 
        receiverPubKey(std::move(receiverPubKey_)),
        keyExchangePubKey(std::move(keyExchangePubKey_)),
        viewTag(viewTag_),
        maskedValue(maskedValue_),
        maskedNonce(std::move(maskedNonce_)),
        senderPubKey(std::move(senderPubKey_))
    {
        m_hash = Hashed(*this);
    }
    OutputMessage(const OutputMessage& output_message) = default;
    OutputMessage(OutputMessage&& output_message) noexcept = default;
    OutputMessage() = default;

    //
    // Operators
    //
    OutputMessage& operator=(const OutputMessage& output_message) = default;
    OutputMessage& operator=(OutputMessage&& output_message) noexcept = default;
    bool operator<(const OutputMessage& output_message) const noexcept { return m_hash < output_message.m_hash; }
    bool operator==(const OutputMessage& output_message) const noexcept { return m_hash == output_message.m_hash; }

    PublicKey receiverPubKey;
    PublicKey keyExchangePubKey;
    uint8_t viewTag;
    uint64_t maskedValue;
    BigInt<16> maskedNonce;
    PublicKey senderPubKey;

    IMPL_SERIALIZABLE(OutputMessage);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(receiverPubKey);
        READWRITE(keyExchangePubKey);
        READWRITE(viewTag);
        READWRITE(maskedValue);
        READWRITE(maskedNonce);
        READWRITE(senderPubKey);

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

private:
    mw::Hash m_hash;
};

////////////////////////////////////////
// OUTPUT IDENTIFIER
////////////////////////////////////////
class OutputId final : public Traits::ICommitted,
                       public Traits::IHashable,
                       public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    OutputId(Commitment commitment, OutputMessage message)
        : m_commitment(std::move(commitment)), m_message(std::move(message))
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
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(OutputId);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_commitment);
        READWRITE(m_message);

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

private:
    Commitment m_commitment;
    OutputMessage m_message;

    mw::Hash m_hash;
};

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
        Commitment commitment,
        OutputMessage message,
        Signature signature,
        const RangeProof::CPtr& pProof
    ) :
        m_commitment(std::move(commitment)),
        m_message(std::move(message)),
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
    Output& operator=(const Output& output) = default;
    Output& operator=(Output&& output) noexcept = default;
    bool operator<(const Output& output) const noexcept { return m_hash < output.m_hash; }
    bool operator==(const Output& output) const noexcept { return m_hash == output.m_hash; }

    //
    // Getters
    //
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }
    const RangeProof::CPtr& GetRangeProof() const noexcept { return m_pProof; }
    const OutputMessage& GetOutputMessage() const noexcept { return m_message; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    const PublicKey& GetReceiverPubKey() const noexcept { return m_message.receiverPubKey; }
    const PublicKey& GetKeyExchangePubKey() const noexcept { return m_message.keyExchangePubKey; }
    uint8_t GetViewTag() const noexcept { return m_message.viewTag; }
    uint64_t GetMaskedValue() const noexcept { return m_message.maskedValue; }
    const BigInt<16>& GetMaskedNonce() const noexcept { return m_message.maskedNonce; }
    const PublicKey& GetSenderPubKey() const noexcept { return m_message.senderPubKey; }

    const PublicKey& Ko() const noexcept { return m_message.receiverPubKey; }
    const PublicKey& Ke() const noexcept { return m_message.keyExchangePubKey; }

    SignedMessage BuildSignedMsg() const noexcept;
    ProofData BuildProofData() const noexcept;

    OutputId ToOutputId() const noexcept {
        return OutputId(m_commitment, m_message);
    }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(Output);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_commitment);
        READWRITE(m_message);
        READWRITE(m_signature);
        READWRITE(m_pProof);

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

private:
    // The homomorphic commitment representing the output amount
    Commitment m_commitment;
    OutputMessage m_message;
    Signature m_signature;

    // A proof that the commitment is in the right range
    RangeProof::CPtr m_pProof;

    mw::Hash m_hash;
};