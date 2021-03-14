#pragma once

#include <txmempool.h>
#include <libmw/libmw.h>

// Forward Declarations
class CBlockTemplate;

namespace MWEB {

class Miner
{
public:
    void NewBlock(const uint64_t nHeight);
    bool AddMWEBTransaction(CTxMemPool::txiter iter);
    void AddHogExTransaction(const CBlockIndex* pIndexPrev, CBlock* pblock, CBlockTemplate* pblocktemplate, CAmount& nFees);

private:
    bool ValidatePegIns(const CTransactionRef& pTx, const std::vector<libmw::PegIn>& pegins) const;

    // MWEB Attributes
    libmw::BlockBuilderRef mweb_builder;
    CAmount mweb_amount_change;
    CAmount mweb_fees;
    std::vector<CTxIn> hogex_inputs;
    std::vector<CTxOut> hogex_outputs;
};

} // namespace MWEB