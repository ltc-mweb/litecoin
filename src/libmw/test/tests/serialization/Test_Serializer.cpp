// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>

#include <mw/serialization/Serializer.h>

BOOST_FIXTURE_TEST_SUITE(TestSerializer, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(SerializerTest)
{
    // Append<T> (BigEndian)
    {
        BOOST_REQUIRE(std::vector<uint8_t>({ 0, 0, 0, 0, 73, 150, 2, 210 }) == Serializer().Append((uint64_t)1234567890ull).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 255, 255, 255, 255, 255, 255, 255, 202 }) == Serializer().Append((int64_t)-54ll).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 73, 150, 2, 210 }) == Serializer().Append((uint32_t)1234567890ul).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 255, 255, 255, 202 }) == Serializer().Append((int32_t)-54l).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 48, 57 }) == Serializer().Append((uint16_t)12345).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 255, 202 }) == Serializer().Append((int16_t)-54).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 25 }) == Serializer().Append((uint8_t)25).vec());
        BOOST_REQUIRE(std::vector<uint8_t>({ 202 }) == Serializer().Append((int8_t)-54).vec());
    }

    // Append(vector), Append(array)
    {
        Serializer serializer;
        serializer.Append(std::vector<uint8_t>({ 1, 2, 3 }));
        serializer.Append(std::array<uint8_t, 3>({ 4, 5, 6 }));
        BOOST_REQUIRE(std::vector<uint8_t>({ 1, 2, 3, 4, 5, 6 }) == serializer.vec());
    }

    // TODO: Finish this
    // Append(const Serializable&)
    // Append(const std::shared_ptr<const Serializable>)
    // Append(const boost::optional<Serializable>&)
    // uint8_t& operator[]
    // const uint8_t& operator[]
}

BOOST_AUTO_TEST_SUITE_END()