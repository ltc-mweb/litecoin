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
class OutputMessage : public Traits::ISerializable, public Traits::IHashable // MW: TODO - Should we version these?
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

    IMPL_SERIALIZABLE(OutputMessage, obj)
    {
        READWRITE(obj.receiverPubKey);
        READWRITE(obj.keyExchangePubKey);
        READWRITE(obj.viewTag);
        READWRITE(obj.maskedValue);
        READWRITE(obj.maskedNonce);
        READWRITE(obj.senderPubKey);
        SER_READ(obj, obj.m_hash = Hashed(obj));
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

private:
    mw::Hash m_hash;
};

////////////////////////////////////////
// OUTPUT
////////////////////////////////////////
class Output :
    public Traits::ICommitted,
    public Traits::ISerializable,
    public Traits::IHashable
{
public:
    //
    // Constructors
    //
    Output(
        Commitment commitment,
        OutputMessage message,
        const RangeProof::CPtr& pProof,
        Signature signature
    ) :
        m_commitment(std::move(commitment)),
        m_message(std::move(message)),
        m_pProof(pProof),
        m_signature(std::move(signature))
    {
        m_hash = ComputeHash();
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

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(Output, obj)
    {
        READWRITE(obj.m_commitment);
        READWRITE(obj.m_message);
        READWRITE(obj.m_pProof);
        READWRITE(obj.m_signature);
        SER_READ(obj, obj.m_hash = obj.ComputeHash());
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

private:
    //
    // Outputs use a special serialization when hashing that only includes
    // the hash of the rangeproof, instead of the full 675 byte rangeproof.
    // 
    // This will make some light client use cases more efficient.
    //
    mw::Hash ComputeHash() const noexcept
    {
        return Hasher()
            .Append(m_commitment)
            .Append(m_message.GetHash())
            .Append(m_pProof->GetHash())
            .Append(m_signature)
            .hash();
    }

    Commitment m_commitment;
    OutputMessage m_message;
    RangeProof::CPtr m_pProof;
    Signature m_signature;

    mw::Hash m_hash;
};