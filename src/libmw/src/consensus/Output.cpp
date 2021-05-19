#include <mw/models/tx/Output.h>

Output::Output(
    Commitment&& commitment,
    Features features,
    PublicKey&& receiver_pubkey,
    PublicKey&& key_exchange_pubkey,
    uint8_t view_tag,
    uint64_t masked_value,
    BigInt<16>&& masked_nonce,
    PublicKey&& sender_pubkey,
    Signature&& signature,
    const RangeProof::CPtr& pProof
)
    : m_commitment(std::move(commitment)),
    m_features(features),
    m_receiverPubKey(std::move(receiver_pubkey)),
    m_keyExchangePubKey(std::move(key_exchange_pubkey)),
    m_viewTag(view_tag),
    m_maskedValue(masked_value),
    m_maskedNonce(std::move(masked_nonce)),
    m_senderPubKey(std::move(sender_pubkey)),
    m_signature(std::move(signature)),
    m_pProof(pProof)
{
    m_hash = Hashed(*this);
}

Serializer& Output::Serialize(Serializer& serializer) const noexcept
{
    return serializer
        .Append(m_commitment)
        .Append<uint8_t>(m_features.Get())
        .Append(m_receiverPubKey)
        .Append(m_keyExchangePubKey)
        .Append(m_viewTag)
        .Append(m_maskedValue)
        .Append(m_maskedNonce)
        .Append(m_senderPubKey)
        .Append(m_signature)
        .Append(m_pProof);
}

Output Output::Deserialize(Deserializer& deserializer)
{
    Commitment commitment = Commitment::Deserialize(deserializer);

    EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
    PublicKey receiver_pubkey = PublicKey::Deserialize(deserializer);
    PublicKey key_exchange_pubkey = PublicKey::Deserialize(deserializer);
    uint8_t view_tag = deserializer.Read<uint8_t>();
    uint64_t masked_value = deserializer.Read<uint64_t>();
    BigInt<16> masked_nonce = BigInt<16>::Deserialize(deserializer);
    PublicKey sender_pubkey = PublicKey::Deserialize(deserializer);
    Signature signature = Signature::Deserialize(deserializer);

    RangeProof::CPtr pProof = std::make_shared<const RangeProof>(RangeProof::Deserialize(deserializer));
    return Output(
        std::move(commitment),
        features,
        std::move(receiver_pubkey),
        std::move(key_exchange_pubkey),
        view_tag,
        masked_value,
        std::move(masked_nonce),
        std::move(sender_pubkey),
        std::move(signature),
        pProof
    );
}