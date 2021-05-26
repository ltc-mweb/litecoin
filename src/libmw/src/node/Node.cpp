#include <mw/node/Node.h>
#include "CoinsViewFactory.h"

#include <mw/db/MMRInfoDB.h>
#include <mw/node/validation/BlockValidator.h>
#include <mw/consensus/Aggregation.h>
#include <mw/common/Logger.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/FileBackend.h>
#include <unordered_map>

using namespace mw;

mw::CoinsViewDB::Ptr Node::Init(
    const FilePath& datadir,
    const mw::Header::CPtr& pBestHeader,
    const std::shared_ptr<mw::IDBWrapper>& pDBWrapper)
{
    auto current_mmr_info = MMRInfoDB(pDBWrapper.get(), nullptr).GetLatest();
    uint32_t file_index = current_mmr_info ? current_mmr_info->index : 0;
    uint32_t compact_index = current_mmr_info ? current_mmr_info->compact_index : 0;

    auto pLeafSet = mmr::LeafSet::Open(datadir, file_index);
    auto pPruneList = mmr::PruneList::Open(datadir, compact_index);

    auto pKernelsBackend = mmr::FileBackend::Open('K', datadir, file_index, pDBWrapper, nullptr);
    mmr::MMR::Ptr pKernelsMMR = std::make_shared<mmr::MMR>(pKernelsBackend);

    auto pOutputBackend = mmr::FileBackend::Open('O', datadir, file_index, pDBWrapper, pPruneList);
    mmr::MMR::Ptr pOutputMMR = std::make_shared<mmr::MMR>(pOutputBackend);

    return std::make_shared<mw::CoinsViewDB>(
        pBestHeader,
        pDBWrapper,
        pLeafSet,
        pKernelsMMR,
        pOutputMMR
    );
}

bool Node::ValidateBlock(
    const mw::Block::CPtr& pBlock,
    const mw::Hash& mweb_hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) noexcept
{
    assert(pBlock != nullptr);

    try {
        BlockValidator().Validate(pBlock, mweb_hash, pegInCoins, pegOutCoins);
        LOG_TRACE_F("Block {} validated", pBlock);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate {}. Error: {}", *pBlock, e);
    }

    return false;
}

mw::BlockUndo::CPtr Node::ConnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    assert(pBlock != nullptr);
    assert(pView != nullptr);

    LOG_TRACE_F("Connecting block {}", pBlock);

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    auto pUndo = pCache->ApplyBlock(pBlock);
    pCache->Flush(nullptr);

    LOG_TRACE_F("Block {} connected", pBlock);
    return pUndo;
}

void Node::DisconnectBlock(const mw::BlockUndo::CPtr& pUndoData, const mw::ICoinsView::Ptr& pView)
{
    assert(pUndoData != nullptr);
    assert(pView != nullptr);

    auto pHeader = pView->GetBestHeader();
    LOG_TRACE_F("Disconnecting block {}", pHeader);

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    pCache->UndoBlock(pUndoData);
    pCache->Flush(nullptr);

    LOG_TRACE_F("Block {} disconnected. New tip: {}", pHeader, pView->GetBestHeader());
}

mw::ICoinsView::Ptr Node::ApplyState(
    const FilePath& datadir,
    const mw::IChain::Ptr& pChain,
    const mw::IDBWrapper::Ptr& pDBWrapper,
    const mw::Header::CPtr& pStateHeader,
    const mw::State& state)
{
    return CoinsViewFactory::CreateDBView(
        pDBWrapper,
        pChain,
        datadir,
        pStateHeader,
        state.utxos,
        state.kernels,
        state.leafset,
        state.pruned_parent_hashes
    );
}