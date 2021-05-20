#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020-2021 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <array>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Commitment.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#define LIBMW_NAMESPACE namespace libmw {
#define NODE_NAMESPACE namespace node {
#define MINER_NAMESPACE namespace miner {
#define DB_NAMESPACE namespace db {
#define WALLET_NAMESPACE namespace wallet {
#define END_NAMESPACE }

// Forward Declarations
namespace mw
{
    class Header;
    class Block;
    class BlockUndo;
    class Transaction;
    class ICoinsView;
    class BlockBuilder;
    struct State;
    class Keychain;
}

LIBMW_NAMESPACE

typedef std::array<uint8_t, 32> Offset;
typedef std::array<uint8_t, 32> PrivateKey;
typedef std::array<uint8_t, 33> PubKey;
typedef std::pair<PubKey, PubKey> MWEBAddress;

static const uint8_t NORMAL_OUTPUT = 0;
static const uint8_t PEGIN_OUTPUT = 1;

/// <summary>
/// Consensus parameters
/// Any change to these will cause a hardfork!
/// </summary>
static constexpr size_t MAX_BLOCK_WEIGHT = 21'000;
static constexpr uint16_t PEGIN_MATURITY = 20;
static constexpr uint8_t MAX_KERNEL_EXTRADATA_SIZE = 33;

/// <summary>
/// Change outputs will use the stealth address generated using index 2,000,000.
/// </summary>
static constexpr uint32_t CHANGE_INDEX{ 0 };

/// <summary>
/// Peg-in outputs will use the stealth address generated using index 2,000,000.
/// </summary>
static constexpr uint32_t PEGIN_INDEX{ 1 };

struct PegIn
{
    uint64_t amount;
    Commitment commitment;

    bool operator==(const PegIn& rhs) const {
        return amount == rhs.amount && commitment == rhs.commitment;
    }

    bool operator!=(const PegIn& rhs) const {
        return !(*this == rhs);
    }
};

struct PegOut
{
    uint64_t amount;
    std::vector<uint8_t> scriptPubKey;
};

struct HeaderRef
{
    std::shared_ptr<const mw::Header> pHeader;
};

/// <summary>
/// A wrapper around an internal pointer to a BlockUndo object.
/// </summary>
struct BlockUndoRef
{
    std::shared_ptr<const mw::BlockUndo> pUndo;
};

/// <summary>
/// A wrapper around an internal pointer to a coin view.
/// This can either be a CoinsViewDB, which represents the flushed view, or a CoinsViewCache.
/// When the node is instantiated, a single CoinsViewDB is created.
/// Every other CoinView is a cache built directly on top of that, or on top of other caches.
/// </summary>
struct CoinsViewRef
{
    /// <summary>
    /// Creates a new CoinsViewCache on top of this CoinsView.
    /// </summary>
    /// <returns>A wrapper around the newly created CoinsViewCache, or null if the current view is null.</returns>
    CoinsViewRef CreateCache() const;

    std::shared_ptr<mw::ICoinsView> pCoinsView;
};

struct ChainParams
{
    boost::filesystem::path dataDirectory;
};

/// <summary>
/// Represents an output owned by the wallet, and the keys necessary to spend it.
/// </summary>
struct Coin : public Traits::ISerializable
{
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

    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint8_t>(features)
            .Append<uint32_t>(address_index)
            .Append(key)
            .Append(blind)
            .Append<uint64_t>(amount)
            .Append(commitment);
    }

    static Coin Deserialize(Deserializer& deserializer)
    {
        Coin coin;
        coin.features = deserializer.Read<uint8_t>();
        coin.address_index = deserializer.Read<uint32_t>();
        coin.key = deserializer.ReadOpt<BlindingFactor>();
        coin.blind = deserializer.ReadOpt<BlindingFactor>();
        coin.amount = deserializer.Read<uint64_t>();
        coin.commitment = deserializer.Read<Commitment>();
        return coin;
    }
};

END_NAMESPACE