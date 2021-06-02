#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <serialize.h>
#include <streams.h>
#include <version.h>

#define IMPL_SERIALIZABLE                                                   \
    std::vector<uint8_t> Serialized() const noexcept final                  \
    {                                                                       \
        std::vector<uint8_t> serialized;                                    \
        CVectorWriter stream(SER_NETWORK, PROTOCOL_VERSION, serialized, 0); \
        ::Serialize(stream, *this);                                         \
        return serialized;                                                  \
    }

namespace Traits
{
    class ISerializable
    {
    public:
        virtual ~ISerializable() = default;

        //
        // Serializes object into a byte vector.
        //
        virtual std::vector<uint8_t> Serialized() const noexcept = 0;
    };
}