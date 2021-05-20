#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/crypto/Crypto.h>

Commitment Commitment::Switch(const BlindingFactor& blind, const uint64_t value)
{
    return Crypto::CommitBlinded(value, Crypto::BlindSwitch(blind, value));
}

Commitment Commitment::Blinded(const BlindingFactor& blind, const uint64_t value)
{
    return Crypto::CommitBlinded(value, blind);
}

Commitment Commitment::Transparent(const uint64_t value)
{
    return Crypto::CommitTransparent(value);
}