#include <mweb/mweb_node.h>
#include <consensus/validation.h>
#include <mw/node/Node.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>

using namespace MWEB;

bool Node::CheckBlock(const CBlock& block, CValidationState& state)
{
    if (block.mwBlock.IsNull()) {
        return true;
    }

    uint256 mweb256 = block.GetMWEBHash();
    if (mweb256 == uint256()) {
        return state.DoS(100, false, REJECT_INVALID, "bad-hogex", false, "HogEx missing or invalid");
    }

    auto pHogEx = block.GetHogEx();
    assert(pHogEx != nullptr);

    std::vector<PegInCoin> pegins;
    std::vector<CTxIn> expected_inputs;

    for (const CTransactionRef& pTx : block.vtx) {
        for (size_t nOut = 0; nOut < pTx->vout.size(); nOut++) {
            int version;
            std::vector<uint8_t> program;
            if (pTx->vout[nOut].scriptPubKey.IsWitnessProgram(version, program)) {
                if (version == Consensus::Mimblewimble::WITNESS_VERSION && program.size() == WITNESS_MWEB_PEGIN_SIZE) {
                    pegins.push_back(PegInCoin(pTx->vout[nOut].nValue, Commitment{std::move(program)}));
                    expected_inputs.push_back(CTxIn(pTx->GetHash(), nOut));
                }
            }
        }
    }

    uint8_t first_pegin = pegins.size() == pHogEx->vin.size() ? 0 : 1; // MW: TODO - Determine this

    if (pegins.size() + first_pegin == pHogEx->vin.size()) {
        for (size_t nIn = first_pegin; nIn < pHogEx->vin.size(); nIn++) {
            if (expected_inputs[nIn - first_pegin] != pHogEx->vin[nIn]) {
                return state.DoS(100, false, REJECT_INVALID, "pegin-mismatch", false, "HogEx pegins do not match block's pegins");
            }
        }
    } else {
        return state.DoS(100, false, REJECT_INVALID, "pegin-mismatch", false, "HogEx pegins do not match block's pegins");
    }

    std::vector<PegOutCoin> pegouts;
    for (size_t i = 1; i < pHogEx->vout.size(); i++) {
        const CScript& pubkey = pHogEx->vout[i].scriptPubKey;
        pegouts.push_back(PegOutCoin(pHogEx->vout[i].nValue, {pubkey.begin(), pubkey.end()}));
    }

    if (!mw::Node::ValidateBlock(block.mwBlock.m_block, mw::Hash(mweb256.begin()), pegins, pegouts)) {
        return state.DoS(100, false, REJECT_INVALID, "bad-blk-mw", false, "mw::Node::ValidateBlock failed");
    }

    return true;
}

bool Node::CheckTransaction(const CTransaction& tx, CValidationState& state, bool fFromBlock)
{
    // HasMWData() is true only when mweb txs being shared outside of a block (for use by mempools).
    // Blocks themselves do not store mweb txs like normal txs.
    // They are instead stored and processed separately in the mweb block.
    if (fFromBlock && tx.HasMWData()) {
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-mwdata-in-block");
    }

    // MWEB: CheckTransaction
    if (tx.HasMWData()) {
        try {
            tx.m_mwtx.m_transaction->Validate();
        } catch (const std::exception& e) {
            return state.DoS(10, false, REJECT_INVALID, "bad-mweb-txn");
        }
    }

    return true;
}