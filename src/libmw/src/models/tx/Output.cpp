#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/serialization/Serializer.h>
#include <mw/serialization/Deserializer.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Schnorr.h>

Output Output::Create(
    BlindingFactor& blind_out,
    const Features& features,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const uint64_t value)
{
    // Generate 128-bit secret nonce 'n' = Hash128(T_nonce, sender_privkey)
    BigInt<16> n(Hashed(EHashTag::NONCE, sender_privkey).data());

    // Calculate unique sending key 's' = H(T_send, A, B, v, n)
    SecretKey s = Hasher(EHashTag::SEND_KEY)
        .Append(receiver_addr.A())
        .Append(receiver_addr.B())
        .Append(value)
        .Append(n)
        .hash();

    // Derive shared secret 't' = H(T_derive, s*A)
    SecretKey t = Hasher(EHashTag::DERIVE)
        .Append(receiver_addr.A().Mul(s))
        .hash();

    // Construct one-time public key for receiver 'Ko' = H(T_outkey, t)*G + B
    PublicKey Ko = PublicKey::From(Hashed(EHashTag::OUT_KEY, t))
        .Add(receiver_addr.B());

    // Key exchange public key 'Ke' = s*B
    PublicKey Ke = receiver_addr.B().Mul(s);

    // Feed the shared secret 't' into a stream cipher (in our case, just a hash function)
    // to derive a blinding factor r and two encryption masks mv (masked value) and mn (masked nonce)
    Deserializer hash64(Hash512(t).vec());
    BlindingFactor r = hash64.Read<BlindingFactor>();
    uint64_t mv = hash64.Read<uint64_t>() ^ value;
    BigInt<16> mn = n ^ hash64.ReadVector(16);

    // Commitment 'C' = r*G + v*H
    blind_out = Crypto::BlindSwitch(r, value);
    Commitment output_commit = Crypto::CommitBlinded(value, blind_out);

    // Sign the malleable output data
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(features.Get())
        .Append(Ko)
        .Append(Ke)
        .Append(t[0])
        .Append(mv)
        .Append(mn)
        .hash();
    PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
    Signature signature = Schnorr::Sign(sender_privkey.data(), sig_message);

    std::vector<uint8_t> proof_data = Serializer()
        .Append<uint8_t>(features.Get())
        .Append(Ko)
        .Append(Ke)
        .Append(t[0])
        .Append(mv)
        .Append(mn)
        .Append(sender_pubkey)
        .Append(signature)
        .vec();

    // Probably best to store sender_key so sender can identify all outputs they've sent?
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        value,
        SecretKey(blind_out.vec()),
        Random::CSPRNG<32>(),
        Random::CSPRNG<32>(),
        ProofMessage{},
        proof_data
    );

    return Output{
        std::move(output_commit),
        features,
        std::move(Ko),
        std::move(Ke),
        t[0],
        mv,
        std::move(mn),
        std::move(sender_pubkey),
        std::move(signature),
        pRangeProof
    };
}

SignedMessage Output::BuildSignedMsg() const noexcept
{
    mw::Hash hashed_msg = Hasher()
        .Append<uint8_t>(m_features.Get())
        .Append(m_receiverPubKey)
        .Append(m_keyExchangePubKey)
        .Append(m_viewTag)
        .Append(m_maskedValue)
        .Append(m_maskedNonce)
        .hash();
    return SignedMessage{ std::move(hashed_msg), m_senderPubKey, m_signature };
}

ProofData Output::BuildProofData() const noexcept
{
    std::vector<uint8_t> message = Serializer()
        .Append<uint8_t>(m_features.Get())
        .Append(m_receiverPubKey)
        .Append(m_keyExchangePubKey)
        .Append(m_viewTag)
        .Append(m_maskedValue)
        .Append(m_maskedNonce)
        .Append(m_senderPubKey)
        .Append(m_signature)
        .vec();

    return ProofData{ m_commitment, m_pProof, std::move(message) };
}