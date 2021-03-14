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
    libmw::TxRef mweb_tx = pTx->m_mwtx.m_transaction;

    //
    // Pegin
    //
    std::vector<CTxIn> vin;
    CAmount pegin_amount = 0;
    std::vector<libmw::PegIn> pegins = mweb_tx.GetPegins();

    if (!ValidatePegIns(pTx, pegins)) {
        LogPrintf("Peg-in Mismatch\n");
        return false;
    }

    for (size_t nOut = 0; nOut < pTx->vout.size(); nOut++) {
        if (IsPegInOutput(pTx->GetOutput(nOut))) {
            vin.push_back(CTxIn{pTx->GetHash(), (uint32_t)nOut});

            assert(MoneyRange(pTx->vout[nOut].nValue));
            pegin_amount += pTx->vout[nOut].nValue;

            if (!MoneyRange(pegin_amount)) {
                LogPrintf("Invalid total peg-in amount\n");
                return false;
            }
        }
    }

    //
    // Pegout
    //
    std::vector<CTxOut> vout;
    CAmount pegout_amount = 0;
    std::vector<libmw::PegOut> pegouts = mweb_tx.GetPegouts();

    for (const libmw::PegOut& pegout : pegouts) {
        CTxDestination destination = DecodeDestination(pegout.address);
        if (!IsValidDestination(destination) || destination.type() == typeid(MWEBDestination)) {
            LogPrintf("Invalid peg-out destination\n");
            return false;
        }

        CAmount amount(pegout.amount);
        assert(MoneyRange(amount));

        CScript scriptPubKey = GetScriptForDestination(destination);
        vout.push_back(CTxOut{amount, scriptPubKey});

        pegout_amount += amount;
        if (!MoneyRange(pegout_amount)) {
            LogPrintf("Invalid total peg-out amount\n");
            return false;
        }
    }

    // Validate fee amount range
    CAmount tx_fee = mweb_tx.GetTotalFee();
    if (!MoneyRange(tx_fee)) {
        LogPrintf("Invalid MWEB fee amount\n");
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
    mweb_fees += tx_fee;
    mweb_amount_change += (CAmount(pegin_amount) - CAmount(pegout_amount + tx_fee));

    return true;
}

namespace std {
template <>
struct hash<libmw::PegIn> {
    size_t operator()(const libmw::PegIn& pegin) const
    {
        return boost::hash_value(pegin.commitment) + boost::hash_value(pegin.amount);
    }
};
} // namespace std

bool MWEB::Miner::ValidatePegIns(const CTransactionRef& pTx, const std::vector<libmw::PegIn>& pegins) const
{
    std::unordered_set<libmw::PegIn> pegin_set(pegins.cbegin(), pegins.cend());

    for (const CTxOut& output : pTx->vout) {
        int version;
        std::vector<uint8_t> program;
        if (output.scriptPubKey.IsWitnessProgram(version, program)) {
            if (version == Consensus::Mimblewimble::WITNESS_VERSION && program.size() == WITNESS_MWEB_PEGIN_SIZE) {
                libmw::PegIn pegin;
                pegin.amount = output.nValue;
                std::copy_n(std::make_move_iterator(program.begin()), WITNESS_MWEB_PEGIN_SIZE, pegin.commitment.begin());
                if (pegin_set.erase(pegin) != 1) {
                    return false;
                }
            }
        }
    }

    if (!pegin_set.empty()) {
        return false;
    }

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
    
    pblocktemplate->vTxSigOpsCost.push_back(0);
}