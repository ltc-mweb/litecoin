#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "interfaces/chain_interface.h"
#include "interfaces/db_interface.h"

#include <mw/consensus/Params.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/Coin.h>
#include <mw/models/wallet/Recipient.h>
#include <mw/node/BlockBuilder.h>
#include <mw/node/CoinsView.h>
#include <mw/node/State.h>
#include <mw/wallet/Keychain.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#define LIBMW_NAMESPACE namespace libmw {
#define NODE_NAMESPACE namespace node {
#define MINER_NAMESPACE namespace miner {
#define DB_NAMESPACE namespace db {
#define WALLET_NAMESPACE namespace wallet {
#define END_NAMESPACE }

LIBMW_NAMESPACE

MINER_NAMESPACE

/// <summary>
/// Creates a new BlockBuilder for assembling an MW ext block incrementally (tx by tx).
/// </summary>
/// <param name="height">The height of the block being built.</param>
/// <param name="view">The CoinsView representing the latest state of the active chain. Must not be null.</param>
/// <returns>A non-null BlockBuilder</returns>
std::shared_ptr<mw::BlockBuilder> NewBuilder(const uint64_t height, const mw::ICoinsView::Ptr& view);

bool AddTransaction(
    const std::shared_ptr<mw::BlockBuilder>& builder,
    const mw::Transaction::CPtr& transaction,
    const std::vector<PegInCoin>& pegins);

END_NAMESPACE // miner

NODE_NAMESPACE

/// <summary>
/// Loads the state (MMRs mostly) into memory, and validates the current UTXO set.
/// </summary>
/// <param name="data_dir">The data directory.</param>
/// <param name="header">The possibly-null chain tip.</param>
/// <param name="pDBWrapper">A wrapper around the node database. Must not be null.</param>
/// <param name="log_callback">A callback to the logger print function.</param>
/// <returns>The CoinsViewDB which represents the state of the flushed chain.</returns>
mw::ICoinsView::Ptr Initialize(
    const boost::filesystem::path& data_dir,
    const mw::Header::CPtr& header,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
    const std::function<void(const std::string&)>& log_callback);

/// <summary>
/// Shuts down node and cleans up memory.
/// </summary>
void Shutdown();

/// <summary>
/// Validates the chainstate and replaces the existing state if valid.
/// This should be used during initial sync, or when syncing from beyond the horizon.
/// </summary>
/// <param name="pChain">Provides access to MWEB headers and blocks. Must not be null.</param>
/// <param name="pCoinsDB">A wrapper around the node database. Must not be null.</param>
/// <param name="stateHeader">The MWEB header at the chain tip.</param>
/// <param name="state">The chainstate to validate and apply. Must not be null.</param>
/// <returns>The CoinsViewDB which represents the state of the flushed chain.</returns>
mw::ICoinsView::Ptr ApplyState(
    const libmw::IChain::Ptr& pChain,
    const mw::Header::CPtr& stateHeader,
    const libmw::IDBWrapper::Ptr& pCoinsDB,
    const mw::State& state);

/// <summary>
/// Context-free validation of the MW ext block.
/// This performs all consensus checks that don't require knowledge of the state.
/// </summary>
/// <param name="block">The MW ext block to validate. Must not be null.</param>
/// <param name="hash">The expected MWEB header hash.</param>
/// <param name="pegInCoins">The peg-in coins that are expected to be part of the MWEB.</param>
/// <param name="pegOutCoins">The peg-out coins that are expected to be part of the MWEB.</param>
/// <returns>True if the inputs are unspent.</returns>
bool CheckBlock(
    const mw::Block::CPtr& block,
    const mw::Hash& hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins);

/// <summary>
/// Validates and connects the MW ext block to the end of the chain in the given CoinsView.
/// </summary>
/// <pre>Block must be validated via CheckBlock before connecting it to the chain.</pre>
/// <param name="block">The block to connect. Must not be null.</param>
/// <param name="view">The CoinsView to connect the block to. Must not be null.</param>
/// <throws>ValidationException if consensus rules are not met.</throws>
mw::BlockUndo::CPtr ConnectBlock(const mw::Block::CPtr& block, const mw::ICoinsView::Ptr& view);

/// <summary>
/// Removes a MW ext block from the end of the chain in the given CoinsView.
/// </summary>
/// <param name="undoData">The MW ext block undo data to apply. Must not be null.</param>
/// <param name="view">The CoinsView to disconnect the block from. Must not be null.</param>
void DisconnectBlock(const mw::BlockUndo::CPtr& undoData, const mw::ICoinsView::Ptr& view);

/// <summary>
/// Commits the changes from the cached CoinsView to the base CoinsView.
/// Adds the cached updates to the database if the base CoinsView is a DB view.
/// </summary>
/// <param name="view">The CoinsView cache whose changes will be committed. Must not be null.</param>
/// <param name="pBatch">The optional DB batch. This must be non-null when the base CoinsView is a DB view.</param>
void FlushCache(
    const mw::ICoinsView::Ptr& view,
    const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr);

/// <summary>
/// Creates an in-memory snapshot of the chainstate in the given CoinsView.
/// </summary>
/// <param name="view">The CoinsView containing the chainstate to snapshot. Must not be null.</param>
/// <returns>A non-null snapshot of the chainstate.</returns>
std::unique_ptr<mw::State> SnapshotState(const mw::ICoinsView::Ptr& view);

/// <summary>
/// Context-free validation of the MWEB transaction.
/// This validates that the transaction is valid without checking for double-spends.
/// </summary>
/// <param name="transaction">The MWEB transaction to validate. Must not be null.</param>
/// <returns>True if transaction is valid.</returns>
bool CheckTransaction(const mw::Transaction::CPtr& transaction);

/// <summary>
/// Validation of the MWEB transaction inputs against the given CoinsView.
/// This validates that the inputs are unspent, and their maturity is reached.
/// </summary>
/// <param name="view">The CoinsView to validate against. Must not be null.</param>
/// <param name="transaction">The MWEB transaction to validate. Must not be null.</param>
/// <param name="nSpendHeight">The height at which the transaction is included.</param>
/// <returns>True if the inputs are unspent.</returns>
bool CheckTxInputs(const mw::ICoinsView::Ptr& view, const mw::Transaction::CPtr& transaction, uint64_t nSpendHeight);

/// <summary>
/// Checks if there's a unspent coin in the view with a matching commitment.
/// </summary>
/// <param name="view">The coins view to check.</param>
/// <param name="commitment">The commitment to look for.</param>
/// <returns>True if there's a matching unspent coin. Otherwise, false.</returns>
bool HasCoin(const mw::ICoinsView::Ptr& view, const Commitment& commitment);

/// <summary>
/// Checks if there's a unspent coin with a matching commitment in the view that has not been flushed to the parent.
/// This is useful for checking if a coin is in the mempool but not yet on chain.
/// </summary>
/// <param name="view">The coins view to check.</param>
/// <param name="commitment">The commitment to look for.</param>
/// <returns>True if there's a matching unspent coin. Otherwise, false.</returns>
bool HasCoinInCache(const mw::ICoinsView::Ptr& view, const Commitment& commitment);

END_NAMESPACE // node

WALLET_NAMESPACE

mw::Transaction::CPtr CreateTx(
    const std::vector<mw::Coin>& input_coins,
    const std::vector<mw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee);

bool RewindBlockOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Block::CPtr& block,
    const Commitment& output_commit,
    mw::Coin& coin_out);

bool RewindTxOutput(
    const mw::Keychain::Ptr& keychain,
    const mw::Transaction::CPtr& tx,
    const Commitment& output_commit,
    mw::Coin& coin_out);

END_NAMESPACE     // wallet
END_NAMESPACE // libmw