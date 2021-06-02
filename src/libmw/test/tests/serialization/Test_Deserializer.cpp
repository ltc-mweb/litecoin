// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/test_bitcoin.h>
#include <mw/serialization/Deserializer.h>

BOOST_FIXTURE_TEST_SUITE(TestDeserializer, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(DeserializerTest)
{
    // Read<T> (Big Endian)
    {
        BOOST_REQUIRE(Deserializer({ 0, 0, 0, 0, 73, 150, 2, 210 }).Read<uint64_t>() == 1234567890ull);
        BOOST_REQUIRE(Deserializer({ 255, 255, 255, 255, 255, 255, 255, 202 }).Read<int64_t>() == -54ll);
        BOOST_REQUIRE(Deserializer({ 73, 150, 2, 210 }).Read<uint32_t>() == 1234567890ul);
        BOOST_REQUIRE(Deserializer({ 255, 255, 255, 202 }).Read<int32_t>() == -54l);
        BOOST_REQUIRE(Deserializer({ 48, 57 }).Read<uint16_t>() == (uint16_t)12345);
        BOOST_REQUIRE(Deserializer({ 255, 202 }).Read<int16_t>() == (int16_t)-54);
        BOOST_REQUIRE(Deserializer({ 25 }).Read<uint8_t>() == (uint8_t)25);
        BOOST_REQUIRE(Deserializer({ 202 }).Read<int8_t>() == (int8_t)-54);
    }

    // TODO: ReadVec, ReadVector, ReadArray, ReadOpt
}

BOOST_AUTO_TEST_SUITE_END()