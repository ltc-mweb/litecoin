#pragma once

#include <mw/common/Macros.h>

#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Commitment.h>

#include <boost/optional.hpp>

MW_NAMESPACE

/// <summary>
/// Change outputs will use the stealth address generated using index 2,000,000.
/// </summary>
static constexpr uint32_t CHANGE_INDEX{0};

/// <summary>
/// Peg-in outputs will use the stealth address generated using index 2,000,000.
/// </summary>
static constexpr uint32_t PEGIN_INDEX{1};

/// <summary>
/// Represents an output owned by the wallet, and the keys necessary to spend it.
/// </summary>
struct Coin : public Traits::ISerializable {
    // 0 for typical outputs or 1 for pegged-in outputs
    // This is used to determine the required number of confirmations before spending.
    uint8_t features;

    // Index of the subaddress this coin was received at.
    uint32_t address_index;

    // The private key needed in order to spend the coin.
    // May be empty for watch-only wallets.
    boost::optional<BlindingFactor> key;

    // The blinding factor needed in order to spend the coin.
    // May be empty for watch-only wallets.
    boost::optional<BlindingFactor> blind;

    // The output amount in litoshis.
    // Typically positive, but could be 0 in the future when we start using decoys to improve privacy.
    uint64_t amount;

    // The output commitment (v*H + r*G).
    Commitment commitment;

    bool IsChange() const noexcept { return address_index == CHANGE_INDEX; }
    bool IsPegIn() const noexcept { return address_index == PEGIN_INDEX; }

    IMPL_SERIALIZABLE;
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(features);
        READWRITE(address_index);
        READWRITE(key);
        READWRITE(blind);
        READWRITE(amount);
        READWRITE(commitment);
    }
};

END_NAMESPACE