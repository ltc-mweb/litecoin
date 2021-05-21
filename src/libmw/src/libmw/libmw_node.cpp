#include <libmw/node.h>

#include <mw/common/Logger.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/INode.h>
#include <mw/node/Snapshot.h>
#include <mw/node/State.h>
#include <mw/wallet/Wallet.h>
#include <numeric>

static mw::INode::Ptr NODE = nullptr;

LIBMW_NAMESPACE
NODE_NAMESPACE

libmw::CoinsViewRef Initialize(
    const libmw::ChainParams& chainParams,
    const mw::Header::CPtr& header,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
    const std::function<void(const std::string&)>& log_callback)
{
    LoggerAPI::Initialize(log_callback);
    NODE = mw::InitializeNode(FilePath{ chainParams.dataDirectory.native() }, header, pDBWrapper);

    return libmw::CoinsViewRef{ NODE->GetDBView() };
}

void Shutdown()
{
    NODE.reset();
}

libmw::CoinsViewRef ApplyState(
    const libmw::IChain::Ptr& pChain,
    const libmw::IDBWrapper::Ptr& pCoinsDB,
    const mw::Header::CPtr& stateHeader,
    const mw::State& state)
{
    auto pCoinsViewDB = NODE->ApplyState(
        pCoinsDB,
        pChain,
        stateHeader,
        state.utxos,
        state.kernels,
        state.leafset,
        state.pruned_parent_hashes
    );

    return libmw::CoinsViewRef{ pCoinsViewDB };
}

bool CheckBlock(
    const mw::Block::CPtr& block,
    const mw::Hash& hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    assert(block != nullptr);

    try {
        NODE->ValidateBlock(block, hash, pegInCoins, pegOutCoins);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate {}. Error: {}", *block, e);
    }

    return false;
}

mw::BlockUndo::CPtr ConnectBlock(const mw::Block::CPtr& block, const CoinsViewRef& view)
{
    return NODE->ConnectBlock(block, view.pCoinsView);
}

void DisconnectBlock(const mw::BlockUndo::CPtr& undoData, const CoinsViewRef& view)
{
    NODE->DisconnectBlock(undoData, view.pCoinsView);
}

void FlushCache(const libmw::CoinsViewRef& view, const std::unique_ptr<libmw::IDBBatch>& pBatch)
{
    LOG_TRACE("Flushing cache");
    auto pViewCache = dynamic_cast<mw::CoinsViewCache*>(view.pCoinsView.get());
    assert(pViewCache != nullptr);

    pViewCache->Flush(pBatch);
    LOG_TRACE("Cache flushed");
}

std::unique_ptr<mw::State> SnapshotState(const libmw::CoinsViewRef& view)
{
    assert(view.pCoinsView != nullptr);
    return { std::make_unique<mw::State>(mw::Snapshot::Build(view.pCoinsView)) };
}

bool CheckTransaction(const mw::Transaction::CPtr& transaction)
{
    assert(transaction != nullptr);

    try {
        transaction->Validate();
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate {}. Error: {}", transaction->Print(), e);
    }

    return false;
}

bool CheckTxInputs(const libmw::CoinsViewRef& view, const mw::Transaction::CPtr& transaction, uint64_t nSpendHeight)
{
    assert(view.pCoinsView != nullptr);
    assert(transaction != nullptr);

    try {
        for (const Input& input : transaction->GetInputs()) {
            auto utxos = view.pCoinsView->GetUTXOs(input.GetCommitment());
            if (utxos.empty()) {
                ThrowValidation(EConsensusError::UTXO_MISSING);
            }

            if (utxos.back()->IsPeggedIn() && nSpendHeight < utxos.back()->GetBlockHeight() + libmw::PEGIN_MATURITY) {
                ThrowValidation(EConsensusError::PEGIN_MATURITY);
            }
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate: {}. Error: {}", transaction->Print(), e);
    }

    return false;
}

bool HasCoin(const libmw::CoinsViewRef& view, const Commitment& commitment)
{
    assert(view.pCoinsView != nullptr);

    return !view.pCoinsView->GetUTXOs(commitment).empty();
}

bool HasCoinInCache(const libmw::CoinsViewRef& view, const Commitment& commitment)
{
    assert(view.pCoinsView != nullptr);

    auto pCoinsView = std::dynamic_pointer_cast<mw::CoinsViewCache>(view.pCoinsView);
    assert(pCoinsView != nullptr);

    return pCoinsView->HasCoinInCache(commitment);
}

END_NAMESPACE // node
END_NAMESPACE // libmw