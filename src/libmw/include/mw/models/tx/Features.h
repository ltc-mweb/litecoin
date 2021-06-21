#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <cstdint>
#include <serialize.h>

// MW: TODO - Not needed if we decide not to require peg-in outputs to mature.
enum EOutputFeatures : uint8_t
{
    // No Flags
    DEFAULT_OUTPUT = 0,

    // Output is a pegged-in output, must not be spent until maturity
    PEGGED_IN = 1
};

class Features
{
public:
    Features() noexcept : m_features(0) {}
    Features(const uint8_t features) noexcept : m_features(features) {}
    Features(const EOutputFeatures features) noexcept : m_features((uint8_t)features) {}

    bool operator==(const uint8_t features) const noexcept { return m_features == features; }
    bool operator==(const EOutputFeatures& features) const noexcept { return m_features == features; }

    bool IsSet(const uint8_t feature) const noexcept { return (m_features & feature) == feature; }
    uint8_t Get() const noexcept { return m_features; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_features);
    }

private:
    uint8_t m_features;
};