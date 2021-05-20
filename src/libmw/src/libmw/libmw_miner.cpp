#include <libmw/node.h>

#include "Transformers.h"

#include <mw/node/BlockBuilder.h>
#include <mw/common/Logger.h>

LIBMW_NAMESPACE
MINER_NAMESPACE

std::shared_ptr<mw::BlockBuilder> NewBuilder(const uint64_t height, const libmw::CoinsViewRef& view)
{
    return std::make_shared<mw::BlockBuilder>(height, view.pCoinsView);
}

bool AddTransaction(
    const std::shared_ptr<mw::BlockBuilder>& builder,
    const mw::Transaction::CPtr& transaction,
    const std::vector<libmw::PegIn>& pegins)
{
    assert(builder != nullptr);
    assert(transaction != nullptr);

    try {
        return builder->AddTransaction(transaction, Transform::PegIns(pegins));
    } catch (std::exception& e) {
        LOG_DEBUG_F("Failed to add transaction {}. Error: {}", transaction, e.what());
    }

    return false;
}

mw::Block::Ptr BuildBlock(const std::shared_ptr<mw::BlockBuilder>& builder)
{
    assert(builder != nullptr);
    return builder->BuildBlock();
}

END_NAMESPACE // miner
END_NAMESPACE // libmw