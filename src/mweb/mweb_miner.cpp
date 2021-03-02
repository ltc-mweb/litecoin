#include <mweb/mweb_miner.h>
#include <script/standard.h>
#include <consensus/consensus.h>
#include <consensus/tx_verify.h>
#include <policy/policy.h>
#include <validation.h>
#include <logging.h>
#include <key_io.h>
#include <miner.h>

void MWEB::Miner::NewBlock(const uint64_t nHeight)
{
    mweb_builder = libmw::miner::NewBuilder(nHeight, pcoinsTip->GetMWView());
    mweb_fees = 0;
    mweb_amount_change = 0;
    hogex_inputs.clear();
    hogex_outputs.clear();
}

bool MWEB::Miner::AddMWEBTransaction(CTxMemPool::txiter iter)
{
    CTransactionRef pTx = iter->GetSharedTx();
    libmw::TxRef mw_tx = pTx->m_mwtx.m_transaction;

    //
    // Pegin
    //
    std::vector<CTxIn> vin;
    uint64_t pegin_amount = 0;
    std::vector<libmw::PegIn> pegins;

    for (size_t nOut = 0; nOut < pTx->vout.size(); nOut++) {
        const auto& output = pTx->vout[nOut];
        int version;
        std::vector<uint8_t> program;
        if (output.scriptPubKey.IsWitnessProgram(version, program)) {
            if (version == Consensus::Mimblewimble::WITNESS_VERSION && program.size() == WITNESS_MWEB_PEGIN_SIZE) {
                vin.push_back(CTxIn{pTx->GetHash(), (uint32_t)nOut});
                pegin_amount += output.nValue;

                libmw::PegIn pegin;
                pegin.amount = output.nValue;
                std::copy_n(std::make_move_iterator(program.begin()), WITNESS_MWEB_PEGIN_SIZE, pegin.commitment.begin());
                pegins.push_back(std::move(pegin));
            }
        }
    }

    // MW: TODO - Verify pegins match.

    //
    // Pegout
    //
    std::vector<CTxOut> vout;
    uint64_t pegout_amount = 0;
    std::vector<libmw::PegOut> pegouts = mw_tx.GetPegouts();

    for (const libmw::PegOut& pegout : pegouts) {
        CTxDestination destination = DecodeDestination(pegout.address);
        if (!IsValidDestination(destination)) {
            LogPrintf("Invalid pegout destination\n");
            return false;
        }

        CScript scriptPubKey = GetScriptForDestination(destination);
        vout.push_back(CTxOut{CAmount(pegout.amount), scriptPubKey});
        pegout_amount += pegout.amount;
    }

    // Validate amount ranges
    if (!MoneyRange(pegin_amount) || !MoneyRange(pegout_amount)) {
        LogPrintf("Invalid pegin/pegout amount\n");
        return false;
    }

    //
    // Add transaction to MWEB
    //
    if (!libmw::miner::AddTransaction(mweb_builder, pTx->m_mwtx.m_transaction, pegins)) {
        LogPrintf("Failed to add MWEB transaction\n");
        return false;
    }

    hogex_inputs.insert(hogex_inputs.end(), vin.cbegin(), vin.cend());
    hogex_outputs.insert(hogex_outputs.end(), vout.cbegin(), vout.cend());
    const uint64_t tx_fee = mw_tx.GetTotalFee();
    mweb_fees += tx_fee;
    mweb_amount_change += (CAmount(pegin_amount) - CAmount(pegout_amount + tx_fee));

    return true;
}

void MWEB::Miner::AddHogExTransaction(const CBlockIndex* pIndexPrev, CBlock* pblock, CBlockTemplate* pblocktemplate, CAmount& nFees)
{
    CMutableTransaction hogExTransaction;
    hogExTransaction.m_hogEx = true;

    CBlock prevBlock;
    assert(ReadBlockFromDisk(prevBlock, pIndexPrev, Params().GetConsensus()));

    CAmount previous_amount = 0;

    //
    // Add previous HogAddr as new HogEx input
    //
    if (prevBlock.vtx.size() >= 2 && prevBlock.vtx.back()->IsHogEx()) {
        assert(!prevBlock.vtx.back()->vout.empty());
        previous_amount = prevBlock.vtx.back()->vout[0].nValue;

        CTxIn prevHogExIn(prevBlock.vtx.back()->GetHash(), 0);
        hogExTransaction.vin.push_back(std::move(prevHogExIn));
    }

    //
    // Add Peg-in inputs
    //
    hogExTransaction.vin.insert(hogExTransaction.vin.end(), hogex_inputs.cbegin(), hogex_inputs.cend());

    //
    // Add New HogAddr
    //
    libmw::BlockRef mw_block = libmw::miner::BuildBlock(mweb_builder);
    libmw::BlockHash mweb_hash = mw_block.GetHash();

    CTxOut hogAddr;
    hogAddr.scriptPubKey = CScript() << OP_9 << std::vector<uint8_t>(mweb_hash.begin(), mweb_hash.end());
    hogAddr.nValue = previous_amount + mweb_amount_change;
    assert(MoneyRange(hogAddr.nValue));
    hogExTransaction.vout.push_back(std::move(hogAddr));

    LogPrintf("MWEB Supply: %ld\n", hogAddr.nValue);

    //
    // Add Peg-out outputs
    //
    hogExTransaction.vout.insert(hogExTransaction.vout.end(), hogex_outputs.cbegin(), hogex_outputs.cend());

    //
    // Update block & template
    //
    nFees += mweb_fees;
    pblock->vtx.emplace_back(MakeTransactionRef(std::move(hogExTransaction)));
    pblock->mwBlock = MWEB::Block(mw_block);
    pblocktemplate->vTxFees.push_back(mweb_fees);
    
     // MWEB: TODO - Confirm that pcoinsTip is correct, and that we have the right locks for it. Also, account for sigop counts when adding the pegin/out instead (preferably in mempool using MempoolTxEntry).
    int64_t nSigOpCount = GetTransactionSigOpCost(*pblock->vtx.back(), *pcoinsTip, STANDARD_SCRIPT_VERIFY_FLAGS);
    pblocktemplate->vTxSigOpsCost.push_back(nSigOpCount);
}