#include <mw/crypto/Hasher.h>
#include <crypto/sha512.h>

#define BLAKE3_NO_AVX512 1
#define BLAKE3_NO_AVX2 1
#define BLAKE3_NO_SSE41 1
#define BLAKE3_NO_SSE2 1
extern "C" {
#include <crypto/blake3/blake3.c>
#include <crypto/blake3/blake3_dispatch.c>
#include <crypto/blake3/blake3_portable.c>
}

mw::Hash Hashed(const std::vector<uint8_t>& serialized)
{
    Hasher hasher;
    hasher.write((char*)serialized.data(), serialized.size());
    return hasher.hash();
}

mw::Hash Hashed(const Traits::ISerializable& serializable)
{
    return Hashed(serializable.Serialized());
}

const mw::Hash& InputMessage()
{
    static const mw::Hash mweb_hash = Hashed({'M', 'W', 'E', 'B'});
    return mweb_hash;
}

BigInt<64> Hash512(const Traits::ISerializable& serializable)
{
    BigInt<64> ret;

    std::vector<uint8_t> serialized = serializable.Serialized();
    CSHA512()
        .Write(serialized.data(), serialized.size())
        .Finalize(ret.data());
    return ret;
}