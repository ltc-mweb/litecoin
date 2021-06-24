#pragma once

#include <consensus/params.h>
#include <mw/node/CoinsView.h>

// Forward Declarations
class CBlock;
class CBlockUndo;
class CBlockIndex;
class CTransaction;
class CValidationState;

namespace MWEB {

class Node
{
public:
    static bool CheckBlock(const CBlock& block, CValidationState& state);
    static bool ContextualCheckBlock(const CBlock& block, const Consensus::Params& consensus_params, const CBlockIndex* pindexPrev, CValidationState& state);
    static bool ConnectBlock(const CBlock& block, const Consensus::Params& consensus_params, const CBlockIndex* pindexPrev, CBlockUndo& blockundo, mw::CoinsViewCache& mweb_view, CValidationState& state);
    static bool CheckTransaction(const CTransaction& tx, CValidationState& state);
};

}