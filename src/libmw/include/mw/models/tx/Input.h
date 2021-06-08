#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/tx/Features.h>
#include <mw/crypto/Hasher.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>

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
    Input(Commitment commitment, PublicKey pubkey, Signature signature)
        : m_commitment(std::move(commitment)), m_pubkey(std::move(pubkey)), m_signature(std::move(signature))
    {
        m_hash = Hashed(*this);
    }
    Input(const Input& input) = default;
    Input(Input&& input) noexcept = default;
    Input() = default;

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
    const PublicKey& GetPubKey() const noexcept { return m_pubkey; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    SignedMessage BuildSignedMsg() const noexcept
    {
        return SignedMessage{InputMessage(), GetPubKey(), GetSignature()};
    }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_commitment);
        READWRITE(m_pubkey);
        READWRITE(m_signature);

        if (ser_action.ForRead()) {
            m_hash = Hashed(*this);
        }
    }

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_hash; }

private:
    // The commit referencing the output being spent.
    Commitment m_commitment;

    // The sender's public key
    PublicKey m_pubkey;

    Signature m_signature;

    mw::Hash m_hash;
};