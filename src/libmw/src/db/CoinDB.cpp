#include <mw/db/CoinDB.h>
#include "common/Database.h"

static const DBTable UTXO_TABLE = { 'U' };

CoinDB::CoinDB(mw::DBWrapper* pDBWrapper, mw::DBBatch* pBatch)
    : m_pDatabase(std::make_unique<Database>(pDBWrapper, pBatch)) { }

CoinDB::~CoinDB() { }

std::unordered_map<mw::Hash, UTXO::CPtr> CoinDB::GetUTXOs(const std::vector<mw::Hash>& output_hashes) const
{
    std::unordered_map<mw::Hash, UTXO::CPtr> utxos;

    for (const mw::Hash& output_hash : output_hashes) {
        auto pUTXO = m_pDatabase->Get<UTXO>(UTXO_TABLE, output_hash.ToHex());
        if (pUTXO != nullptr) {
            utxos.insert({output_hash, pUTXO->item});
        }
    }

    return utxos;
}

void CoinDB::AddUTXOs(const std::vector<UTXO::CPtr>& utxos)
{
    std::vector<DBEntry<UTXO>> entries;
    std::transform(
        utxos.cbegin(), utxos.cend(),
        std::back_inserter(entries),
        [](const UTXO::CPtr& pUTXO) { return DBEntry<UTXO>(pUTXO->GetOutputHash().ToHex(), pUTXO); }
    );

    m_pDatabase->Put(UTXO_TABLE, entries);
}

void CoinDB::RemoveUTXOs(const std::vector<mw::Hash>& output_hashes)
{
    for (const mw::Hash& output_hash : output_hashes) {
        m_pDatabase->Delete(UTXO_TABLE, output_hash.ToHex());
    }
}

void CoinDB::RemoveAllUTXOs()
{
    m_pDatabase->DeleteAll(UTXO_TABLE);
}