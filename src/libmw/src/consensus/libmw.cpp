#include <libmw/libmw.h>
#include <mw/models/tx/Transaction.h>

std::string libmw::TxRef::ToString() const noexcept
{
    assert(pTransaction != nullptr);

    return pTransaction->Print();
}

std::vector<uint8_t> libmw::SerializeTx(const libmw::TxRef& tx)
{
    assert(tx.pTransaction != nullptr);
    return tx.pTransaction->Serialized();
}

libmw::TxRef libmw::DeserializeTx(const std::vector<uint8_t>& bytes)
{
    Deserializer deserializer{ bytes };
    auto pTx = std::make_shared<mw::Transaction>(mw::Transaction::Deserialize(deserializer));
    return libmw::TxRef{ pTx };
}