#include <mweb/mweb_node.h>
#include <consensus/validation.h>
#include <primitives/block.h>
#include <script/interpreter.h>
#include <libmw/libmw.h>

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

    // MW: TODO - Check HogEx transaction (pegins must match block's transactions)

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

    //std::vector<PegInCoin> pegins = block.GetPegInCoins();
    //if (pegins.size() == block.vtx.back()->vin.size()) {
    //    // MW: TODO - First MWEB's HogEx
    //} else if (pegins.size() + 1 == block.vtx.back()->vin.size()) {
    //    for (size_t i = 1; i < block.vtx.back()->vin.size(); i++) {
    //        const CTxIn& pegin_input = block.vtx.back()->vin[i];
    //        if (pegins[i].commitment != block.)
    //    }
    //}

    if (!mw::Node::ValidateBlock(block.mwBlock.m_block, mw::Hash(mweb256.begin()), pegins, block.GetPegOutCoins())) {
        return state.DoS(100, false, REJECT_INVALID, "bad-blk-mw", false, "mw::Node::ValidateBlock failed");
    }

    return true;
}