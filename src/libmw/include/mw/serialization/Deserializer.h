#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/exceptions/DeserializationException.h>
#include <mw/traits/Serializable.h>
#include <compat/byteswap.h>

#include <algorithm>
#include <boost/optional.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

constexpr bool IsBigEndian() noexcept
{
    constexpr union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

inline char bswap(const char val) { return val; }
inline int8_t bswap(const int8_t val) { return val; }
inline uint8_t bswap(const uint8_t val) { return val; }
inline int16_t bswap(const int16_t val) { return (int16_t)bswap_16((uint16_t)val); }
inline uint16_t bswap(const uint16_t val) { return bswap_16(val); }
inline int32_t bswap(const int32_t val) { return (int32_t)bswap_32((uint32_t)val); }
inline uint32_t bswap(const uint32_t val) { return bswap_32(val); }
inline int64_t bswap(const int64_t val) { return (int64_t)bswap_64((uint64_t)val); }
inline uint64_t bswap(const uint64_t val) { return bswap_64(val); }

class Deserializer
{
public:
    Deserializer(const std::vector<uint8_t>& bytes) : m_index(0), m_bytes(bytes) {}
    Deserializer(std::vector<uint8_t>&& bytes) : m_index(0), m_bytes(std::move(bytes)) {}

    template <class T, typename SFINAE = std::enable_if_t<std::is_integral<T>::value>>
    T Read()
    {
        T value;
        ReadBigEndian<T>(value);
        return value;
    }

    template <class T, typename SFINAE = std::enable_if_t<std::is_base_of<Traits::ISerializable, T>::value>>
    decltype(auto) Read()
    {
        return T::Deserialize(*this);
    }

    template <class T, typename SFINAE = std::enable_if_t<std::is_base_of<Traits::ISerializable, T>::value>>
    boost::optional<T> ReadOpt()
    {
        if (Read<bool>()) {
            return {T::Deserialize(*this)};
        }

        return boost::none;
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of<Traits::ISerializable, T>::value>>
    std::vector<T> ReadVec()
    {
        const uint32_t num_entries = Read<uint32_t>();
        std::vector<T> vec(num_entries);
        for (uint32_t i = 0; i < num_entries; i++) {
            vec[i] = T::Deserialize(*this);
        }

        return vec;
    }

    template <class T, typename SFINAE = typename std::enable_if_t<std::is_base_of<Traits::ISerializable, T>::value>>
    std::vector<std::shared_ptr<const T>> ReadVecPtrs()
    {
        const uint32_t num_entries = Read<uint32_t>();
        std::vector<std::shared_ptr<const T>> vec(num_entries);
        for (uint32_t i = 0; i < num_entries; i++) {
            vec[i] = std::make_shared<const T>(T::Deserialize(*this));
        }

        return vec;
    }

    std::vector<uint8_t> ReadVector(const uint64_t numBytes)
    {
        if (m_index + numBytes > m_bytes.size()) {
            ThrowDeserialization("Attempted to read past end of buffer.");
        }

        const size_t index = m_index;
        m_index += numBytes;

        return std::vector<uint8_t>(m_bytes.cbegin() + index, m_bytes.cbegin() + index + numBytes);
    }

    template <size_t T>
    std::array<uint8_t, T> ReadArray()
    {
        if (m_index + T > m_bytes.size()) {
            ThrowDeserialization("Attempted to read past end of buffer.");
        }

        const size_t index = m_index;
        m_index += T;

        std::array<uint8_t, T> arr;
        std::copy_n(m_bytes.begin() + index, T, arr.begin());
        return arr;
    }

    size_t GetRemainingSize() const
    {
        return m_bytes.size() - m_index;
    }

private:
    template <class T>
    void ReadBigEndian(T& t)
    {
        if (m_index + sizeof(T) > m_bytes.size()) {
            ThrowDeserialization("Attempted to read past end of buffer.");
        }

        memcpy(&t, &m_bytes[m_index], sizeof(T));
        if (!IsBigEndian()) {
            t = bswap(t);
        }

        m_index += sizeof(T);
    }

    size_t m_index;
    std::vector<uint8_t> m_bytes;
};
