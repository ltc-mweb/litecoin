#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

MW_NAMESPACE

class IDBBatch
{
public:
    using UPtr = std::unique_ptr<mw::IDBBatch>;
    virtual ~IDBBatch() = default;

    virtual void Write(const std::string& key, const std::vector<uint8_t>& value) = 0;
    virtual void Erase(const std::string& key) = 0;

    virtual void Commit() = 0;
};

class IDBIterator
{
public:
    virtual ~IDBIterator() = default;

    virtual void Seek(const std::string& key) = 0;
    virtual void Next() = 0;

    virtual bool GetKey(std::string& key) const = 0;
    virtual bool Valid() const = 0;
};

class IDBWrapper
{
public:
    using Ptr = std::shared_ptr<mw::IDBWrapper>;

    virtual ~IDBWrapper() = default;

    virtual bool Read(const std::string& key, std::vector<uint8_t>& value) const = 0;
    virtual std::unique_ptr<IDBIterator> NewIterator() = 0;
    virtual std::unique_ptr<IDBBatch> CreateBatch() = 0;
};

END_NAMESPACE // mw