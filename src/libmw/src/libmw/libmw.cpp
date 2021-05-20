#include <libmw/libmw.h>

#include "Transformers.h"

#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/INode.h>
#include <mw/node/State.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE

libmw::HeaderRef DeserializeHeader(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    auto pHeader = std::make_shared<mw::Header>(mw::Header::Deserialize(deserializer));
    return libmw::HeaderRef{ pHeader };
}

std::vector<uint8_t> SerializeHeader(const libmw::HeaderRef& header)
{
    assert(header.pHeader != nullptr);
    return header.pHeader->Serialized();
}

libmw::BlockUndoRef DeserializeBlockUndo(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    auto pBlockUndo = std::make_shared<mw::BlockUndo>(mw::BlockUndo::Deserialize(deserializer));
    return libmw::BlockUndoRef{ pBlockUndo };
}

std::vector<uint8_t> SerializeBlockUndo(const libmw::BlockUndoRef& blockUndo)
{
    assert(blockUndo.pUndo != nullptr);
    return blockUndo.pUndo->Serialized();
}

libmw::StateRef DeserializeState(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    mw::State state = mw::State::Deserialize(deserializer);
    return { std::make_shared<mw::State>(std::move(state)) };
}

std::vector<uint8_t> SerializeState(const libmw::StateRef& state)
{
    assert(state.pState != nullptr);
    return state.pState->Serialized();
}

END_NAMESPACE // libmw