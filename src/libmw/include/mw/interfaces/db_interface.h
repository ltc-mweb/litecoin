#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <dbwrapper.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

MW_NAMESPACE

class DBBatch
{
public:
    using UPtr = std::unique_ptr<DBBatch>;

    DBBatch(CDBWrapper* pDB, const std::shared_ptr<CDBBatch>& pBatch)
        : m_pDB(pDB), m_pBatch(pBatch) {}

    void Write(const std::string& key, const std::vector<uint8_t>& value)
    {
        m_pBatch->Write(key, value);
    }

    void Erase(const std::string& key)
    {
        m_pBatch->Erase(key);
    }

    void Commit()
    {
        m_pDB->WriteBatch(*m_pBatch);
    }

private:
    CDBWrapper* m_pDB;
    std::shared_ptr<CDBBatch> m_pBatch;
};

class DBIterator
{
public:
    DBIterator(CDBIterator* pIterator)
        : m_pIterator(std::unique_ptr<CDBIterator>(pIterator)) {}

    void Seek(const std::string& key)
    {
        m_pIterator->Seek(key);
    }

    void Next()
    {
        m_pIterator->Next();
    }

    bool GetKey(std::string& key) const
    {
        return m_pIterator->GetKey(key);
    }

    bool Valid() const
    {
        return m_pIterator->Valid();
    }

private:
    std::unique_ptr<CDBIterator> m_pIterator;
};

class DBWrapper
{
public:
    using Ptr = std::shared_ptr<DBWrapper>;

    DBWrapper(CDBWrapper* pDB) : m_pDB(pDB) {}

    bool Read(const std::string& key, std::vector<uint8_t>& value) const // MW: TODO - Should support serializable object instead of vector?
    {
        return m_pDB->Read(key, value);
    }

    std::unique_ptr<DBIterator> NewIterator()
    {
        return std::make_unique<DBIterator>(m_pDB->NewIterator());
    }

    std::unique_ptr<DBBatch> CreateBatch()
    {
        return std::make_unique<DBBatch>(m_pDB, std::make_shared<CDBBatch>(*m_pDB));
    }

private:
    CDBWrapper* m_pDB;
};

END_NAMESPACE // mw