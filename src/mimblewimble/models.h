// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MIMBLEWIMBLE_MODELS_H
#define LITECOIN_MIMBLEWIMBLE_MODELS_H

#include <vector>
#include <memory>

struct CMWBlock
{
    using CPtr = std::shared_ptr<CMWBlock>;

    std::vector<unsigned char> bytes;

    // Some compilers complain without a default constructor
    CMWBlock() {}

    bool IsNull() const { return bytes.empty(); }

    void SetNull()
    {
        bytes.clear();
        bytes.shrink_to_fit();
    }
};

struct CMWTx {
    using CPtr = std::shared_ptr<CMWTx>;

    std::vector<unsigned char> bytes;

    // Some compilers complain without a default constructor
    CMWTx() {}

    bool IsNull() const { return bytes.empty(); }
};

#endif // LITECOIN_MIMBLEWIMBLE_MODELS_H