#include <libmw/libmw.h>

#include <mw/node/BlockBuilder.h>
#include <mw/common/Logger.h>

LIBMW_NAMESPACE
MINER_NAMESPACE

std::shared_ptr<mw::BlockBuilder> NewBuilder(const uint64_t height, const mw::ICoinsView::Ptr& view)
{
    return std::make_shared<mw::BlockBuilder>(height, view);
}

bool AddTransaction(
    const std::shared_ptr<mw::BlockBuilder>& builder,
    const mw::Transaction::CPtr& transaction,
    const std::vector<PegInCoin>& pegins)
{
    assert(builder != nullptr);
    assert(transaction != nullptr);

    try {
        return builder->AddTransaction(transaction, pegins);
    } catch (std::exception& e) {
        LOG_DEBUG_F("Failed to add transaction {}. Error: {}", transaction, e.what());
    }

    return false;
}

END_NAMESPACE // miner
END_NAMESPACE // libmw