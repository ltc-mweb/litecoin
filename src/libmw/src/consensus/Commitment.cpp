#include <mw/models/crypto/Commitment.h>

Serializer& Commitment::Serialize(Serializer& serializer) const noexcept
{
    return m_bytes.Serialize(serializer);
}

Commitment Commitment::Deserialize(Deserializer& deserializer)
{
    return Commitment(BigInt<SIZE>::Deserialize(deserializer));
}