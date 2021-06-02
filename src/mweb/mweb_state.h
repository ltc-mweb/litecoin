// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H
#define LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H

#include <mw/node/State.h>
#include <serialize.h>
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
    mw::State state;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockhash);
        READWRITE(state);
    }
};

} // namespace MWEB

#endif // LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H