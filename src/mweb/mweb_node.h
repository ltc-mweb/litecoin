#pragma once

// Forward Declarations
class CBlock;
class CValidationState;

namespace MWEB {

class Node
{
public:
    static bool CheckBlock(const CBlock& block, CValidationState& state);
};

}