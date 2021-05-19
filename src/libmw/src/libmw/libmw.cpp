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

libmw::BlockRef DeserializeBlock(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    auto pBlock = std::make_shared<mw::Block>(mw::Block::Deserialize(deserializer));
    return libmw::BlockRef{ pBlock };
}

std::vector<uint8_t> SerializeBlock(const libmw::BlockRef& block)
{
    assert(block.pBlock != nullptr);
    return block.pBlock->Serialized();
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

libmw::TxRef DeserializeTx(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    auto pTx = std::make_shared<mw::Transaction>(mw::Transaction::Deserialize(deserializer));
    return libmw::TxRef{ pTx };
}

std::vector<uint8_t> SerializeTx(const libmw::TxRef& tx)
{
    assert(tx.pTransaction != nullptr);
    return tx.pTransaction->Serialized();
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

libmw::Coin DeserializeCoin(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };

    libmw::Coin coin;
    coin.features = deserializer.Read<uint8_t>();
    coin.address_index = deserializer.Read<uint32_t>();
    auto key_opt = deserializer.ReadOpt<::BlindingFactor>();
    coin.key = key_opt ? boost::make_optional<libmw::BlindingFactor>(key_opt.value().array()) : boost::none;
    auto blind_opt = deserializer.ReadOpt<::BlindingFactor>();
    coin.blind = blind_opt ? boost::make_optional<libmw::BlindingFactor>(blind_opt.value().array()) : boost::none;
    coin.amount = deserializer.Read<uint64_t>();
    coin.commitment = deserializer.Read<::Commitment>().array();
    return coin;
}

std::vector<uint8_t> SerializeCoin(const libmw::Coin& coin)
{
    return Serializer()
        .Append<uint8_t>(coin.features)
        .Append<uint32_t>(coin.address_index)
        .Append(coin.key)
        .Append(coin.blind)
        .Append<uint64_t>(coin.amount)
        .Append(coin.commitment)
        .vec();
}

END_NAMESPACE // libmw