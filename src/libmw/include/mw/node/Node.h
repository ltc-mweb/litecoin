#pragma once

#include <mw/common/Macros.h>
#include <mw/common/BitSet.h>
#include <mw/node/CoinsView.h>
#include <mw/node/State.h>
#include <mw/models/block/Header.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/tx/UTXO.h>
#include <mw/interfaces/chain_interface.h>
#include <mw/interfaces/db_interface.h>
#include <functional>
#include <vector>

MW_NAMESPACE

class Node
{
public:
    /// <summary>
    /// Loads the state (MMRs mostly) into memory, and validates the current UTXO set.
    /// </summary>
    /// <param name="data_dir">The data directory.</param>
    /// <param name="header">The possibly-null chain tip.</param>
    /// <param name="pDBWrapper">A wrapper around the node database. Must not be null.</param>
    /// <returns>The CoinsViewDB which represents the state of the flushed chain.</returns>
    static mw::CoinsViewDB::Ptr Init(
        const FilePath& datadir,
        const mw::Header::CPtr& pBestHeader,
        const mw::IDBWrapper::Ptr& pDBWrapper
    );

    /// <summary>
    /// Context-free validation of the MW ext block.
    /// This performs all consensus checks that don't require knowledge of the state.
    /// </summary>
    /// <param name="pBlock">The MW ext block to validate. Must not be null.</param>
    /// <param name="mweb_hash">The expected MWEB header hash.</param>
    /// <param name="pegInCoins">The peg-in coins that are expected to be part of the MWEB.</param>
    /// <param name="pegOutCoins">The peg-out coins that are expected to be part of the MWEB.</param>
    /// <returns>True if the inputs are unspent.</returns>
    static bool ValidateBlock(
        const mw::Block::CPtr& pBlock,
        const mw::Hash& mweb_hash,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) noexcept;

    /// <summary>
    /// Validates and connects the MW ext block to the end of the chain in the given CoinsView.
    /// Consumer is required to call ValidateBlock first.
    /// </summary>
    /// <pre>Block must be validated via CheckBlock before connecting it to the chain.</pre>
    /// <param name="block">The block to connect. Must not be null.</param>
    /// <param name="view">The CoinsView to connect the block to. Must not be null.</param>
    /// <throws>ValidationException if consensus rules are not met.</throws>
    static mw::BlockUndo::CPtr ConnectBlock(
        const mw::Block::CPtr& pBlock,
        const ICoinsView::Ptr& pView
    );

    /// <summary>
    /// Removes a MW ext block from the end of the chain in the given CoinsView.
    /// </summary>
    /// <param name="undoData">The MW ext block undo data to apply. Must not be null.</param>
    /// <param name="view">The CoinsView to disconnect the block from. Must not be null.</param>
    static void DisconnectBlock(
        const mw::BlockUndo::CPtr& pUndoData,
        const ICoinsView::Ptr& pView
    );

    /// <summary>
    /// Validates the chainstate and replaces the existing state if valid.
    /// This should be used during initial sync, or when syncing from beyond the horizon.
    /// </summary>
    /// <param name="data_dir">The data directory.</param>
    /// <param name="pChain">Provides access to MWEB headers and blocks. Must not be null.</param>
    /// <param name="pCoinsDB">A wrapper around the node database. Must not be null.</param>
    /// <param name="stateHeader">The MWEB header at the chain tip.</param>
    /// <param name="state">The chainstate to validate and apply. Must not be null.</param>
    /// <returns>The CoinsViewDB which represents the state of the flushed chain.</returns>
    static mw::ICoinsView::Ptr ApplyState(
        const FilePath& data_dir,
        const mw::IChain::Ptr& pChain,
        const mw::IDBWrapper::Ptr& pCoinsDB,
        const mw::Header::CPtr& pStateHeader,
        const mw::State& state
    );
};

END_NAMESPACE