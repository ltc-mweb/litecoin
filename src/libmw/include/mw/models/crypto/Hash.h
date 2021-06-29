#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/common/Traits.h>
#include <mw/models/crypto/BigInteger.h>
#include <boost/functional/hash.hpp>

MW_NAMESPACE

using Hash = BigInt<32>;

END_NAMESPACE

namespace std
{
    template<>
    struct hash<mw::Hash>
    {
        size_t operator()(const mw::Hash& hash) const
        {
            return boost::hash_value(hash.vec());
        }
    };
}

static const struct
{
    bool operator()(const Traits::IHashable& a, const Traits::IHashable& b) const
    {
        return a.GetHash() < b.GetHash();
    }
} SortByHash;