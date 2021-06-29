#include <mw/node/Node.h>

#include <mw/db/MMRInfoDB.h>
#include <mw/node/validation/BlockValidator.h>
#include <mw/consensus/Aggregation.h>
#include <mw/common/Logger.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/PruneList.h>
#include <unordered_map>

using namespace mw;

mw::CoinsViewDB::Ptr Node::Init(
    const FilePath& datadir,
    const mw::Header::CPtr& pBestHeader,
    const std::shared_ptr<mw::DBWrapper>& pDBWrapper)
{
    auto current_mmr_info = MMRInfoDB(pDBWrapper.get(), nullptr).GetLatest();
    uint32_t file_index = current_mmr_info ? current_mmr_info->index : 0;
    uint32_t compact_index = current_mmr_info ? current_mmr_info->compact_index : 0;

    auto pLeafSet = LeafSet::Open(datadir, file_index);
    auto pPruneList = PruneList::Open(datadir, compact_index);

    MMR::Ptr pOutputMMR = MMR::Open('O', datadir, file_index, pDBWrapper, pPruneList);

    return std::make_shared<mw::CoinsViewDB>(
        pBestHeader,
        pDBWrapper,
        pLeafSet,
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
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate {}. Error: {}", *pBlock, e);
    }

    return false;
}