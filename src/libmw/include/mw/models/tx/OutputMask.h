#pragma once

#include <mw/crypto/Hasher.h>
#include <mw/crypto/Pedersen.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/SecretKey.h>

class OutputMask
{
public:
    OutputMask(const OutputMask&) = default;
    OutputMask(OutputMask&&) noexcept = default;

    // Feeds the shared secret 't' into a stream cipher (in our case, just a hash function)
    // to derive a blinding factor r and two encryption masks mv (masked value) and mn (masked nonce)
    static OutputMask FromShared(const SecretKey& shared_secret)
    {
        BigInt<64> hash = Hash512(shared_secret);
        VectorReader reader(SER_NETWORK, PROTOCOL_VERSION, hash.vec(), 0);

        OutputMask mask;
        reader >> mask.blind;
        reader >> mask.value_mask;
        reader >> mask.nonce_mask;
        return mask;
    }

    const BlindingFactor& GetRawBlind() const noexcept { return blind; }
    BlindingFactor BlindSwitch(const uint64_t value) const { return Pedersen::BlindSwitch(blind, value); }
    Commitment SwitchCommit(const uint64_t value) const { return Commitment::Switch(blind, value); }
    BigInt<16> MaskNonce(const BigInt<16>& nonce) const { return nonce ^ nonce_mask; }
    uint64_t MaskValue(const uint64_t value) const { return value ^ value_mask; }

private:
    OutputMask() = default;

    BlindingFactor blind; // r
    uint64_t value_mask; // mv
    BigInt<16> nonce_mask; // mn
};