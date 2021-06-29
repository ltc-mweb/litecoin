#pragma once

#include <mw/common/Macros.h>
#include <mw/file/FilePath.h>
#include <mw/node/CoinsView.h>
#include <mw/models/block/Header.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/interfaces/db_interface.h>
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
        const mw::DBWrapper::Ptr& pDBWrapper
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
};

END_NAMESPACE