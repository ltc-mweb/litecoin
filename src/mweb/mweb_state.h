// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MWEB_STATE_H
#define LITECOIN_MWEB_STATE_H

#include <serialize.h>
#include <uint256.h>

namespace MWEB {

class StateRequest
{
public:
    uint256 horizon_block_hash;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(horizon_block_hash);
    }
};

//
// During IBD, downloaded blocks contain all MWEB headers, kernels, and spent
// input indices (to update bitset). All that's needed to finish verifying the
// state is the updated UTXO set (as of the "horizon checkpoint"), and any
// missing hashes (pruned_parent_hashes) necessary to rebuild the UTXO PMMR.
//
class State
{
public:
    uint256 horizon_block_hash;
    std::vector<Output> unspent_outputs;
    std::vector<mw::Hash> pruned_parent_hashes;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(horizon_block_hash);
        READWRITE(unspent_outputs);
        READWRITE(pruned_parent_hashes);
    }
};

} // namespace MWEB

#endif // LITECOIN_MWEB_STATE_H