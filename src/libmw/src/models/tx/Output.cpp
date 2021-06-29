#include <mw/models/tx/Output.h>
#include <mw/models/tx/OutputMask.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Pedersen.h>
#include <mw/crypto/Schnorr.h>

Output Output::Create(
    BlindingFactor& blind_out,
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

    // Calc blinding factor and mask nonce and amount.
    OutputMask mask = OutputMask::FromShared(t);
    blind_out = mask.BlindSwitch(value);
    uint64_t mv = mask.MaskValue(value);
    BigInt<16> mn = mask.MaskNonce(n);

    // Commitment 'C' = r*G + v*H
    Commitment output_commit = Commitment::Blinded(blind_out, value);

    // Sign the malleable output data
    mw::Hash sig_message = Hasher()
        .Append(Ko)
        .Append(Ke)
        .Append(t[0])
        .Append(mv)
        .Append(mn)
        .hash();
    PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
    Signature signature = Schnorr::Sign(sender_privkey.data(), sig_message);

    OutputMessage message{Ko, Ke, t[0], mv, mn, sender_pubkey};
    std::vector<uint8_t> proof_data = message.Serialized();
    proof_data.insert(proof_data.end(), signature.vec().begin(), signature.vec().end());

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
        std::move(message),
        std::move(signature),
        pRangeProof
    };
}

SignedMessage Output::BuildSignedMsg() const noexcept
{
    mw::Hash hashed_msg = Hasher()
        .Append(m_message.receiverPubKey)
        .Append(m_message.keyExchangePubKey)
        .Append(m_message.viewTag)
        .Append(m_message.maskedValue)
        .Append(m_message.maskedNonce)
        .hash();
    return SignedMessage{ std::move(hashed_msg), m_message.senderPubKey, m_signature };
}

ProofData Output::BuildProofData() const noexcept
{
    std::vector<uint8_t> message = m_message.Serialized();
    message.insert(message.end(), m_signature.vec().begin(), m_signature.vec().end());

    return ProofData{ m_commitment, m_pProof, std::move(message) };
}