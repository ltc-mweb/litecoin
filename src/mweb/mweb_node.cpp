#include <mweb/mweb_node.h>

#include <chain.h>
#include <consensus/validation.h>
#include <mw/node/BlockValidator.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <undo.h>
#include <util/system.h>
#include <validation.h>

using namespace MWEB;

bool Node::CheckBlock(const CBlock& block, BlockValidationState& state)
{
    if (block.mweb_block.IsNull()) {
        return true;
    }

    uint256 mweb256 = block.GetMWEBHash();
    if (mweb256 == uint256()) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-hogex", "HogEx missing or invalid");
    }

    for (size_t i = 0; i < block.vtx.size() - 1; i++) {
        if (block.vtx[i]->IsHogEx()) {
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-hogex-position", "hogex in wrong position");
        }
    }

    // HasMWEBTx() is true only when mweb txs being shared outside of a block (for use by mempools).
    // Blocks themselves do not store mweb txs like normal txs.
    // They are instead stored and processed separately in the mweb block.
    // So at this point, no txs should contain MWEB data.
    for (const CTransactionRef& pTx : block.vtx) {
        if (pTx->HasMWEBTx()) {
            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "unexpected-mweb-data", "Block contains transactions with MWEB data attached");
        }
    }

    std::vector<PegInCoin> pegins;
    for (size_t i = 0; i < block.vtx.size() - 1; i++) {
        for (const CTxOut& out : block.vtx[i]->vout) {
            mw::Hash kernel_hash;
            if (out.scriptPubKey.IsMWEBPegin(&kernel_hash)) {
                pegins.push_back(PegInCoin{out.nValue, std::move(kernel_hash)});
            }
        }
    }

    auto pHogEx = block.GetHogEx();
    assert(pHogEx != nullptr);

    std::vector<PegOutCoin> pegouts;
    for (size_t i = 1; i < pHogEx->vout.size(); i++) {
        const CScript& pubkey = pHogEx->vout[i].scriptPubKey;
        pegouts.push_back(PegOutCoin(pHogEx->vout[i].nValue, {pubkey.begin(), pubkey.end()}));
    }

    if (!BlockValidator::ValidateBlock(block.mweb_block.m_block, mw::Hash(mweb256.begin()), pegins, pegouts)) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-blk-mw", "BlockValidator::ValidateBlock failed");
    }

    return true;
}

bool Node::ContextualCheckBlock(const CBlock& block, const Consensus::Params& consensus_params, const CBlockIndex* pindexPrev, BlockValidationState& state)
{
    if (!IsMWEBEnabled(pindexPrev, consensus_params)) {
        // No MWEB data is allowed in blocks that don't commit to MWEB data, as this would otherwise leave room for spam
        if (!block.mweb_block.IsNull()) {
            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "unexpected-mweb-data", "MWEB::Node::ContextualCheckBlock(): MWEB not activated, but extension block found");
        }

        return true;
    }

    if (block.mweb_block.IsNull()) {
        return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "mweb-missing", "MWEB::Node::ContextualCheckBlock(): MWEB activated but extension block not found");
    }

    if (block.mweb_block.GetHeight() != (pindexPrev->nHeight + 1)) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "mweb-height-mismatch", "MWEB::Node::ConnectBlock(): Invalid MWEB block height");
    }

    auto pHogEx = block.GetHogEx();
    assert(pHogEx != nullptr);

    size_t next_pegin_idx = 0;
    if (IsMWEBEnabled(pindexPrev->pprev, consensus_params)) {
        const COutPoint& prev_hogex_out = pHogEx->vin[next_pegin_idx++].prevout;
        if (prev_hogex_out.n != 0 || pindexPrev->hogex_hash != prev_hogex_out.hash) {
            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "invalid-hogex-input", "First input of HogEx does not point to previous HogEx");
        }
    }

    CAmount hogex_input_amount = pindexPrev->mweb_amount;
    for (size_t nTx = 1; nTx < block.vtx.size() - 1; nTx++) {
        const CTransactionRef& pTx = block.vtx[nTx];
        for (size_t nOut = 0; nOut < pTx->vout.size(); nOut++) {
            if (pTx->vout[nOut].scriptPubKey.IsMWEBPegin()) {
                if (pHogEx->vin.size() <= next_pegin_idx) {
                    return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "pegins-missing", "Pegins missing from HogEx");
                }

                const COutPoint& hogex_input = pHogEx->vin[next_pegin_idx++].prevout;
                if (hogex_input.n != nOut || hogex_input.hash != pTx->GetHash()) {
                    return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "pegin-mismatch", "HogEx pegins do not match block's pegins");
                }

                hogex_input_amount += pTx->vout[nOut].nValue;
                if (!MoneyRange(hogex_input_amount)) {
                    return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "accumulated-pegin-outofrange", "MWEB::Node::ContextualCheckBlock(): accumulated pegin amount for the block out of range.");
                }
            }
        }
    }

    if (next_pegin_idx != pHogEx->vin.size()) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "extra-hogex-input", "HogEx contains unexpected input(s)");
    }

    // MWEB: For HogEx transaction, the fee must be equal to the total fee of the extension block.
    CAmount hogex_fee = hogex_input_amount - pHogEx->GetValueOut();
    if (hogex_fee != block.mweb_block.GetTotalFee()) {
        return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "bad-txns-mweb-fee-mismatch", "MWEB::Node::ContextualCheckBlock(): HogEx fee does not match MWEB fee.");
    }

    // Check HogEx transaction: `new value == (prev value + pegins) - (pegouts + fees)`
    CAmount mweb_amount = pindexPrev->mweb_amount + block.mweb_block.GetSupplyChange();
    if (mweb_amount != block.GetMWEBAmount()) {
        return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "mweb-amount-mismatch", "MWEB::Node::ContextualCheckBlock(): HogEx amount does not match expected MWEB amount");
    }

    return true;
}

bool Node::ConnectBlock(const CBlock& block, const Consensus::Params& consensus_params, const CBlockIndex* pindexPrev, CBlockUndo& blockundo, mw::CoinsViewCache& mweb_view, BlockValidationState& state)
{
    if (!ContextualCheckBlock(block, consensus_params, pindexPrev, state)) {
        return false;
    }

    if (!block.mweb_block.IsNull()) {
        try {
            blockundo.mwundo = mweb_view.ApplyBlock(block.mweb_block.m_block);
        } catch (const std::exception& e) {
            // MW: TODO - Distinguish between invalid blocks and mutated blocks
            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "mweb-connect-failed", strprintf("MWEB::Node::ConnectBlock(): Failed to connect mw block: %s", e.what()));
        }
    }

    return true;
}

bool Node::CheckTransaction(const CTransaction& tx, TxValidationState& state)
{
    if (tx.IsCoinBase() || tx.IsHogEx()) {
        for (size_t i = tx.IsHogEx() ? 1 : 0; i < tx.vout.size(); i++) {
            if (tx.vout[i].scriptPubKey.IsMWEBPegin()) {
                return state.Invalid(TxValidationResult::TX_CONSENSUS, strprintf("bad-%s-contains-pegin", tx.IsHogEx() ? "hogex" : "cb"));
            }
        }
    }

    if (tx.HasMWEBTx()) {
        try {
            tx.mweb_tx.m_transaction->Validate();
        } catch (const std::exception& e) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-mweb-txn");
        }
    }

    return true;
}