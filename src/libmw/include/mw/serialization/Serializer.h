#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/traits/Serializable.h>
#include <support/allocators/secure.h>
#include <mw/serialization/Deserializer.h>

#include <boost/optional.hpp>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <compat/byteswap.h>

class Serializer
{
public:
    Serializer() = default;
    Serializer(const size_t expectedSize) { m_serialized.reserve(expectedSize); }
    ~Serializer() { memory_cleanse(m_serialized.data(), m_serialized.size()); }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_integral_v<T>>>
    Serializer& Append(const T& t)
    {
        size_t current_size = m_serialized.size();
        m_serialized.resize(current_size + sizeof(T));

        if (IsBigEndian()) {
            memcpy(m_serialized.data() + current_size, (const uint8_t*)&t, sizeof(T));
        } else {
            T swapped = bswap(t);
            memcpy(m_serialized.data() + current_size, (const uint8_t*)&swapped, sizeof(T));
        }

        return *this;
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_integral_v<T>>>
    Serializer& AppendLE(const T& t)
    {
        size_t current_size = m_serialized.size();
        m_serialized.resize(current_size + sizeof(T));

        if (IsBigEndian()) {
            T swapped;
            memcpy(m_serialized.data() + current_size, (const uint8_t*)&swapped, sizeof(T));
        } else {
            memcpy(m_serialized.data() + current_size, (const uint8_t*)&t, sizeof(T));
        }

        return *this;
    }

    Serializer& Append(const std::vector<uint8_t>& vectorToAppend)
    {
        m_serialized.insert(m_serialized.end(), vectorToAppend.cbegin(), vectorToAppend.cend());
        return *this;
    }

    template <size_t T>
    Serializer& Append(const std::array<uint8_t, T>& arr)
    {
        m_serialized.insert(m_serialized.end(), arr.cbegin(), arr.cend());
        return *this;
    }

    Serializer& Append(const Traits::ISerializable& serializable)
    {
        return serializable.Serialize(*this);
    }

    Serializer& Append(const std::shared_ptr<const Traits::ISerializable>& pSerializable)
    {
        return pSerializable->Serialize(*this);
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    Serializer& AppendVec(const std::vector<T>& vec)
    {
        Append<uint32_t>((uint32_t)vec.size());
        for (const T& item : vec)
        {
            item.Serialize(*this);
        }

        return *this;
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    Serializer& AppendVec(const std::vector<std::shared_ptr<const T>>& vec)
    {
        Append<uint32_t>((uint32_t)vec.size());
        for (const std::shared_ptr<const T>& pItem : vec) {
            pItem->Serialize(*this);
        }

        return *this;
    }

    template<class T>
    Serializer& Append(const boost::optional<T>& opt)
    {
        Append<bool>(!!opt);
        if (opt) {
            Append(opt.value());
        }

        return *this;
    }

    const std::vector<uint8_t>& vec() const { return m_serialized; }
    const uint8_t* data() const { return m_serialized.data(); }
    size_t size() const { return m_serialized.size(); }

    uint8_t& operator[] (const size_t x) { return m_serialized[x]; }
    const uint8_t& operator[] (const size_t x) const { return m_serialized[x]; }

private:
    inline int8_t bswap(const int8_t val) const { return val; }
    inline uint8_t bswap(const uint8_t val) const { return val; }
    inline int16_t bswap(const int16_t val) const { return (int16_t)bswap_16((uint16_t)val); }
    inline uint16_t bswap(const uint16_t val) const { return bswap_16(val); }
    inline int32_t bswap(const int32_t val) const { return (int32_t)bswap_32((uint64_t)val); }
    inline uint32_t bswap(const uint32_t val) const { return bswap_32(val); }
    inline int64_t bswap(const int64_t val) const { return (int64_t)bswap_64((uint64_t)val); }
    inline uint64_t bswap(const uint64_t val) const { return bswap_64(val); }

    std::vector<uint8_t> m_serialized;
};