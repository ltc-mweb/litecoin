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
#include <mw/node/Node.h>
#include <mw/node/State.h>
#include <mw/wallet/Keychain.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#define LIBMW_NAMESPACE namespace libmw {
#define NODE_NAMESPACE namespace node {
#define WALLET_NAMESPACE namespace wallet {
#define END_NAMESPACE }

LIBMW_NAMESPACE

NODE_NAMESPACE

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
    const uint64_t fee
);

END_NAMESPACE     // wallet
END_NAMESPACE // libmw