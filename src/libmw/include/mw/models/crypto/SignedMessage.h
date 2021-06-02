#pragma once

#include <mw/crypto/Hasher.h>
#include <mw/models/crypto/Hash.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <boost/functional/hash.hpp>

/// <summary>
/// Contains a hashed message, a signature of that message, and the public key it was signed for.
/// </summary>
class SignedMessage : public Traits::ISerializable, public Traits::IHashable
{
public:
    SignedMessage() = default;
    SignedMessage(const SignedMessage&) = default;
    SignedMessage(SignedMessage&&) = default;
    SignedMessage(const mw::Hash& msgHash, const PublicKey& publicKey, const Signature& signature)
        : m_messageHash(msgHash), m_publicKey(publicKey), m_signature(signature) { }
    SignedMessage(mw::Hash&& msgHash, PublicKey&& publicKey, Signature&& signature)
        : m_messageHash(std::move(msgHash)), m_publicKey(std::move(publicKey)), m_signature(std::move(signature)) { }

    //
    // Operators
    //
    SignedMessage& operator=(const SignedMessage& other) = default;
    SignedMessage& operator=(SignedMessage&& other) noexcept = default;
    bool operator==(const SignedMessage& rhs) const noexcept {
        return m_messageHash == rhs.m_messageHash
            && m_publicKey == rhs.m_publicKey
            && m_signature == rhs.m_signature;
    }
    bool operator!=(const SignedMessage& rhs) const noexcept { return !(*this == rhs); }

    //
    // Getters
    //
    const mw::Hash& GetMsgHash() const noexcept { return m_messageHash; }
    const PublicKey& GetPublicKey() const noexcept { return m_publicKey; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_messageHash);
        READWRITE(m_publicKey);
        READWRITE(m_signature);
    }

    mw::Hash GetHash() const noexcept final
    {
        return Hashed(Serialized());
    }

private:
    mw::Hash m_messageHash;
    PublicKey m_publicKey;
    Signature m_signature;
};

namespace std
{
    template<>
    struct hash<SignedMessage>
    {
        size_t operator()(const SignedMessage& hash) const
        {
            return boost::hash_value(hash.Serialized());
        }
    };
}