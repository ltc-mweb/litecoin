// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MIMBLEWIMBLE_MODELS_H
#define LITECOIN_MIMBLEWIMBLE_MODELS_H

#include <serialize.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <vector>
#include <memory>

struct CMWBlock
{
    using CPtr = std::shared_ptr<CMWBlock>;

    mw::Block::Ptr m_pBlock;

    // Some compilers complain without a default constructor
    CMWBlock() {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        if (ser_action.ForRead()) {
            // Deserialize
            std::vector<uint8_t> bytes;
            READWRITE(bytes);

            if (!bytes.empty()) {
                Deserializer deserializer{std::move(bytes)};
                m_pBlock = std::make_shared<mw::Block>(mw::Block::Deserialize(deserializer));
            }
        } else {
            // Serialize
            if (m_pBlock != nullptr) {
                std::vector<uint8_t> bytes = Serializer().Append(*m_pBlock).vec();
                READWRITE(bytes);
            } else {
                READWRITE(std::vector<uint8_t>{});
            }
        }
    }

    bool IsNull() const noexcept { return m_pBlock == nullptr; }
    void SetNull() noexcept { m_pBlock = nullptr; }
};

struct CMWTx
{
    using CPtr = std::shared_ptr<CMWTx>;

    mw::Transaction::CPtr m_pTransaction;

    // Some compilers complain without a default constructor
    CMWTx() {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        if (ser_action.ForRead()) {
            // Deserialize
            std::vector<uint8_t> bytes;
            READWRITE(bytes);

            if (!bytes.empty()) {
                Deserializer deserializer{std::move(bytes)};
                m_pTransaction = std::make_shared<mw::Transaction>(mw::Transaction::Deserialize(deserializer));
            }
        } else {
            // Serialize
            if (m_pTransaction != nullptr) {
                std::vector<uint8_t> bytes = Serializer().Append(*m_pTransaction).vec();
                READWRITE(bytes);
            } else {
                READWRITE(std::vector<uint8_t>{});
            }
        }
    }

    bool IsNull() const noexcept { return m_pTransaction == nullptr; }
};

#endif // LITECOIN_MIMBLEWIMBLE_MODELS_H