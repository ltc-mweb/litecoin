#include <mw/crypto/Hasher.h>

mw::Hash Hashed(const std::vector<uint8_t>& serialized)
{
    return mw::Hash(SerializeHash(serialized).begin());
}

mw::Hash Hashed(const Traits::ISerializable& serializable)
{
    Serializer serializer;
    serializable.Serialize(serializer);
    return mw::Hash(SerializeHash(serializer.vec()).begin());
}

mw::Hash Hashed(const EHashTag tag, const Traits::ISerializable& serializable)
{
    return Hasher(tag).Append(serializable).hash();
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