#pragma once

#include <hash.h>
#include <crypto/sha512.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Serializable.h>

enum class EHashTag : char
{
    ADDRESS = 'A',
    SEND_KEY = 'S',
    DERIVE = 'D',
    OUT_KEY = 'O',
    NONCE = 'N'
};

// FUTURE: Incrementally update hash on each Append using CSHA256.write()
class Hasher
{
public:
    Hasher() = default;
    Hasher(const EHashTag tag)
    {
        m_serializer.Append<char>(static_cast<char>(tag));
    }

    mw::Hash hash() const { return mw::Hash(SerializeHash(m_serializer.vec()).begin()); }

    template <class T>
    Hasher& Append(const T& t)
    {
        m_serializer.Append(t);
        return *this;
    }

private:
    Serializer m_serializer;
};

extern mw::Hash Hashed(const std::vector<uint8_t>& serialized);
extern mw::Hash Hashed(const Traits::ISerializable& serializable);
extern mw::Hash Hashed(const EHashTag tag, const Traits::ISerializable& serializable);
extern const mw::Hash& InputMessage();
extern BigInt<64> Hash512(const Traits::ISerializable& serializable);