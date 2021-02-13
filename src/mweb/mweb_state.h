// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H
#define LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H

#include <serialize.h>
#include <libmw/libmw.h>
#include <uint256.h>

namespace MWEB {

class StateRequest
{
public:
    // An MWEB::StateRequest message
    uint256 blockhash;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockhash);
    }
};

class State
{
public:
    // An MWEB::State message
    uint256 blockhash;
    std::vector<uint8_t> serialized_state;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockhash);
        READWRITE(serialized_state);
    }
};

} // namespace MWEB

#endif // LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H