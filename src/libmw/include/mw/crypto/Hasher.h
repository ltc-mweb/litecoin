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
    Hasher() : m_writer(SER_GETHASH, 0) { }
    Hasher(const EHashTag tag)
        : m_writer(SER_GETHASH, 0)
    {
        m_writer << static_cast<char>(tag);
    }

    mw::Hash hash() { return mw::Hash(m_writer.GetHash().begin()); }

    template <class T>
    Hasher& Append(const T& t)
    {
        m_writer << t;
        return *this;
    }

private:
    CHashWriter m_writer;
};

extern mw::Hash Hashed(const std::vector<uint8_t>& serialized);
extern mw::Hash Hashed(const Traits::ISerializable& serializable);

template<class T>
mw::Hash Hashed(const EHashTag tag, const T& serializable)
{
    return Hasher(tag).Append(serializable).hash();
}
extern const mw::Hash& InputMessage();
extern BigInt<64> Hash512(const Traits::ISerializable& serializable);