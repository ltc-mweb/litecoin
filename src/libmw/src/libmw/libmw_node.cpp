#include <libmw/libmw.h>

#include <mw/common/Logger.h>
#include <mw/consensus/Params.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/Node.h>
#include <mw/node/Snapshot.h>
#include <mw/node/State.h>
#include <numeric>

LIBMW_NAMESPACE
NODE_NAMESPACE

std::unique_ptr<mw::State> SnapshotState(const mw::ICoinsView::Ptr& view)
{
    assert(view != nullptr);
    return { std::make_unique<mw::State>(mw::Snapshot::Build(view)) };
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

bool CheckTxInputs(const mw::ICoinsView::Ptr& view, const mw::Transaction::CPtr& transaction, uint64_t nSpendHeight)
{
    assert(view != nullptr);
    assert(transaction != nullptr);

    try {
        for (const Input& input : transaction->GetInputs()) {
            auto utxos = view->GetUTXOs(input.GetCommitment());
            if (utxos.empty()) {
                ThrowValidation(EConsensusError::UTXO_MISSING);
            }

            if (utxos.back()->IsPeggedIn() && nSpendHeight < utxos.back()->GetBlockHeight() + mw::PEGIN_MATURITY) {
                ThrowValidation(EConsensusError::PEGIN_MATURITY);
            }
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to validate: {}. Error: {}", transaction->Print(), e);
    }

    return false;
}

END_NAMESPACE // node
END_NAMESPACE // libmw