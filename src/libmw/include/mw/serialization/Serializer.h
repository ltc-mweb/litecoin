#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <streams.h>
#include <version.h>

#include <boost/optional.hpp>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

class Serializer
{
public:
    Serializer() 
        : m_serialized{}, m_writer(SER_NETWORK, PROTOCOL_VERSION, m_serialized, 0) {}

    template <class T>
    Serializer& Append(const T& t)
    {
        m_writer << t;
        return *this;
    }

    const std::vector<uint8_t>& vec() const { return m_serialized; }
    const uint8_t* data() const { return m_serialized.data(); }
    size_t size() const { return m_serialized.size(); }

    uint8_t& operator[] (const size_t x) { return m_serialized[x]; }
    const uint8_t& operator[] (const size_t x) const { return m_serialized[x]; }

private:
    std::vector<uint8_t> m_serialized;
    CVectorWriter m_writer;
};