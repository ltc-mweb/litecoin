#include <libmw/node.h>

#include "Transformers.h"

#include <mw/node/BlockBuilder.h>
#include <mw/common/Logger.h>

LIBMW_NAMESPACE
MINER_NAMESPACE

libmw::BlockBuilderRef NewBuilder(const uint64_t height, const libmw::CoinsViewRef& view)
{
    return libmw::BlockBuilderRef{ std::make_shared<mw::BlockBuilder>(height, view.pCoinsView) };
}

bool AddTransaction(
    const libmw::BlockBuilderRef& builder,
    const mw::Transaction::CPtr& transaction,
    const std::vector<libmw::PegIn>& pegins)
{
    assert(builder.pBuilder != nullptr);
    assert(transaction != nullptr);

    try {
        return builder.pBuilder->AddTransaction(transaction, Transform::PegIns(pegins));
    } catch (std::exception& e) {
        LOG_DEBUG_F("Failed to add transaction {}. Error: {}", transaction, e.what());
    }

    return false;
}

mw::Block::Ptr BuildBlock(const libmw::BlockBuilderRef& builder)
{
    assert(builder.pBuilder != nullptr);
    return builder.pBuilder->BuildBlock();
}

END_NAMESPACE // miner
END_NAMESPACE // libmw