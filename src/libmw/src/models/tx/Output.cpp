#include <mw/models/tx/Output.h>
#include <mw/models/tx/OutputMask.h>
#include <mw/models/wallet/StealthAddress.h>
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

    // Construct one-time public key for receiver 'Ko' = H(T_outkey, t)*B
    PublicKey Ko = receiver_addr.B().Mul(Hashed(EHashTag::OUT_KEY, t));

    // Key exchange public key 'Ke' = s*B
    PublicKey Ke = receiver_addr.B().Mul(s);

    // Calc blinding factor and mask nonce and amount.
    OutputMask mask = OutputMask::FromShared(t);
    blind_out = mask.BlindSwitch(value);
    uint64_t mv = mask.MaskValue(value);
    BigInt<16> mn = mask.MaskNonce(n);

    // Commitment 'C' = r*G + v*H
    Commitment output_commit = Commitment::Blinded(blind_out, value);

    // Calculate the ephemeral send pubkey 'Ks' = ks*G
    PublicKey Ks = Keys::From(sender_privkey).PubKey();

    OutputMessage message{Ko, Ke, t[0], mv, mn, Ks};

    // Probably best to store sender_key so sender can identify all outputs they've sent?
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        value,
        SecretKey(blind_out.vec()),
        SecretKey::Random(),
        SecretKey::Random(),
        ProofMessage{},
        message.Serialized()
    );
    
    // Sign the output
    mw::Hash sig_message = Hasher()
        .Append(output_commit)
        .Append(message)
        .Append(pRangeProof->GetHash())
        .hash();
    Signature signature = Schnorr::Sign(sender_privkey.data(), sig_message);

    return Output{
        std::move(output_commit),
        std::move(message),
        pRangeProof,
        std::move(signature)
    };
}

SignedMessage Output::BuildSignedMsg() const noexcept
{
    mw::Hash hashed_msg = Hasher()
        .Append(m_commitment)
        .Append(m_message)
        .Append(m_pProof->GetHash())
        .hash();
    return SignedMessage{ std::move(hashed_msg), m_message.senderPubKey, m_signature };
}

ProofData Output::BuildProofData() const noexcept
{
    return ProofData{ m_commitment, m_pProof, m_message.Serialized() };
}