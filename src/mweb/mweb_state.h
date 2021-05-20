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
    std::unique_ptr<mw::State> state;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockhash);

        if (ser_action.ForRead()) {
            std::vector<uint8_t> serialized;
            READWRITE(serialized);
            Deserializer deserializer(std::move(serialized));
            state = std::make_unique<mw::State>(mw::State::Deserialize(deserializer));
        } else {
            std::vector<uint8_t> serialized = state->Serialized();
            READWRITE(serialized);
        }
    }
};

} // namespace MWEB

#endif // LITECOIN_MIMBLEWIMBLE_MWEB_STATE_H