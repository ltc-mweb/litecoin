#pragma once

// Forward Declarations
class CBlock;
class CTransaction;
class CValidationState;

namespace MWEB {

class Node
{
public:
    static bool CheckBlock(const CBlock& block, CValidationState& state);
    static bool CheckTransaction(const CTransaction& tx, CValidationState& state, bool fFromBlock);
};

}