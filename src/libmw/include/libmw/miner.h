#pragma once

#include "defs.h"

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>

LIBMW_NAMESPACE
MINER_NAMESPACE

/// <summary>
/// Creates a new BlockBuilder for assembling an MW ext block incrementally (tx by tx).
/// </summary>
/// <param name="height">The height of the block being built.</param>
/// <param name="view">The CoinsView representing the latest state of the active chain. Must not be null.</param>
/// <returns>A non-null BlockBuilder</returns>
std::shared_ptr<mw::BlockBuilder> NewBuilder(const uint64_t height, const libmw::CoinsViewRef& view);

bool AddTransaction(
    const std::shared_ptr<mw::BlockBuilder>& builder,
    const mw::Transaction::CPtr& transaction,
    const std::vector<PegInCoin>& pegins
);

mw::Block::Ptr BuildBlock(const std::shared_ptr<mw::BlockBuilder>& builder);

END_NAMESPACE // miner
END_NAMESPACE // libmw