#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Traits.h>
#include <mw/crypto/Hasher.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/SignedMessage.h>

////////////////////////////////////////
// INPUT
////////////////////////////////////////
class Input :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    Input(Commitment commitment, PublicKey input_pubkey, PublicKey output_pubkey, Signature signature)
        : m_commitment(std::move(commitment)),
        m_inputPubKey(std::move(input_pubkey)),
        m_outputPubKey(std::move(output_pubkey)),
        m_signature(std::move(signature))
    {
        m_hash = Hashed(*this);
    }
    Input(const Input& input) = default;
    Input(Input&& input) noexcept = default;
    Input() = default;

    //
    // Factories
    //
    static Input Create(const Commitment& commitment, const SecretKey& input_key, const SecretKey& output_key);

    //
    // Destructor
    //
    virtual ~Input() = default;

    //
    // Operators
    //
    Input& operator=(const Input& input) = default;
    Input& operator=(Input&& input) noexcept = default;
    bool operator<(const Input& input) const noexcept { return m_hash < input.m_hash; }
    bool operator==(const Input& input) const noexcept { return m_hash == input.m_hash; }

    //
    // Getters
    //
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }
    const PublicKey& GetInputPubKey() const noexcept { return m_inputPubKey; }
    const PublicKey& GetOutputPubKey() const noexcept { return m_outputPubKey; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    SignedMessage BuildSignedMsg() const noexcept;

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(Input, obj)
    {
        READWRITE(obj.m_commitment);
        READWRITE(obj.m_inputPubKey);
        READWRITE(obj.m_outputPubKey);
        READWRITE(obj.m_signature);
        SER_READ(obj, obj.m_hash = Hashed(obj));
    }

    //
    // Traits
    //
    const mw::Hash& GetHash() const noexcept final { return m_hash; }

private:
    // The commit referencing the output being spent.
    Commitment m_commitment;

    // The input pubkey.
    PublicKey m_inputPubKey;

    // The public key of the output being spent.
    PublicKey m_outputPubKey;

    Signature m_signature;

    mw::Hash m_hash;
};