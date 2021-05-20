#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/Commitment.h>
#include <unordered_set>

namespace Traits
{
    class ICommitted
    {
    public:
        virtual ~ICommitted() = default;

        virtual const Commitment& GetCommitment() const noexcept = 0;
    };
}

static const struct
{
    bool operator()(const Traits::ICommitted& a, const Traits::ICommitted& b) const
    {
        return a.GetCommitment() < b.GetCommitment();
    }
} SortByCommitment;

class Commitments
{
public:
    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of<Traits::ICommitted, T>::value>>
    static std::vector<Commitment> From(const std::vector<T>& committed) noexcept
    {
        std::vector<Commitment> commitments;
        std::transform(
            committed.cbegin(), committed.cend(),
            std::back_inserter(commitments),
            [](const T& committed) { return committed.GetCommitment(); }
        );

        return commitments;
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of<Traits::ICommitted, T>::value>>
    static std::unordered_set<Commitment> SetFrom(const std::vector<T>& committed) noexcept
    {
        std::unordered_set<Commitment> commitments;
        std::transform(
            committed.cbegin(), committed.cend(),
            std::inserter(commitments, commitments.end()),
            [](const T& committed) { return committed.GetCommitment(); }
        );

        return commitments;
    }
};