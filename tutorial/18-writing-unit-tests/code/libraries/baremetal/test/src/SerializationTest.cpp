//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : SerializationTest.cpp
//
// Namespace   : baremetal
//
// Class       : SerializationTest
//
// Description : Serialization tests
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#include "unittest/unittest.h"

#include "baremetal/Serialization.h"

using namespace unittest;

namespace baremetal {
namespace test {

TEST_SUITE(Baremetal)
{

class SerializationTest
    : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(SerializationTest, SerializeChar)
{
    char c = 'A';
    EXPECT_EQ("65", Serialize(c));
    EXPECT_EQ("  65", Serialize(c, 4));
    EXPECT_EQ("65  ", Serialize(c, -4));
}

TEST_FIXTURE(SerializationTest, SerializeIntegerWithPrefix)
{
    int32 i32 = 1234567890l;
    EXPECT_EQ("0b1001001100101100000001011010010", Serialize(i32, 0, 2, true));
    EXPECT_EQ("011145401322", Serialize(i32, 0, 8, true));
    EXPECT_EQ("1234567890", Serialize(i32, 0, 10, true));
    EXPECT_EQ("0x499602D2", Serialize(i32, 0, 16, true));
}

TEST_FIXTURE(SerializationTest, SerializeIntegerWithPrefixAndLeadingZeros)
{
    int32 i32 = 1234567890l;
    EXPECT_EQ("0b01001001100101100000001011010010", Serialize(i32, 32, 2, true, true));
    EXPECT_EQ("00000011145401322", Serialize(i32, 16, 8, true, true));
    EXPECT_EQ("0000001234567890", Serialize(i32, 16, 10, true, true));
    EXPECT_EQ("0x0000499602D2", Serialize(i32, 12, 16, true, true));
}

TEST_FIXTURE(SerializationTest, SerializeIntegerWithoutPrefixWithLeadingZeros)
{
    int32 i32 = 1234567890l;
    EXPECT_EQ("01001001100101100000001011010010", Serialize(i32, 32, 2, false, true));
    EXPECT_EQ("0000011145401322", Serialize(i32, 16, 8, false, true));
    EXPECT_EQ("0000001234567890", Serialize(i32, 16, 10, false, true));
    EXPECT_EQ("0000499602D2", Serialize(i32, 12, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeInt8)
{
    int8 i8 = 123;
    EXPECT_EQ("123", Serialize(i8));
    EXPECT_EQ("     123", Serialize(i8, 8));
    EXPECT_EQ("123     ", Serialize(i8, -8));
    EXPECT_EQ(" 1111011", Serialize(i8, 8, 2));
    EXPECT_EQ("     173", Serialize(i8, 8, 8));
    EXPECT_EQ("     123", Serialize(i8, 8, 10));
    EXPECT_EQ("      7B", Serialize(i8, 8, 16));
    EXPECT_EQ("    0x7B", Serialize(i8, 8, 16, true));
    EXPECT_EQ("0x0000007B", Serialize(i8, 8, 16, true, true));
    EXPECT_EQ("0000007B", Serialize(i8, 8, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeUInt8)
{
    uint8 u8 = 234;
    EXPECT_EQ("234", Serialize(u8));
    EXPECT_EQ("     234", Serialize(u8, 8));
    EXPECT_EQ("234     ", Serialize(u8, -8));
    EXPECT_EQ("11101010", Serialize(u8, 8, 2));
    EXPECT_EQ("     352", Serialize(u8, 8, 8));
    EXPECT_EQ("     234", Serialize(u8, 8, 10));
    EXPECT_EQ("      EA", Serialize(u8, 8, 16));
    EXPECT_EQ("    0xEA", Serialize(u8, 8, 16, true));
    EXPECT_EQ("0x000000EA", Serialize(u8, 8, 16, true, true));
    EXPECT_EQ("000000EA", Serialize(u8, 8, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeInt16)
{
    int16 i16 = 12345;
    EXPECT_EQ("12345", Serialize(i16));
    EXPECT_EQ("   12345", Serialize(i16, 8));
    EXPECT_EQ("12345   ", Serialize(i16, -8));
    EXPECT_EQ("  11000000111001", Serialize(i16, 16, 2));
    EXPECT_EQ("   30071", Serialize(i16, 8, 8));
    EXPECT_EQ("   12345", Serialize(i16, 8, 10));
    EXPECT_EQ("    3039", Serialize(i16, 8, 16));
    EXPECT_EQ("  0x3039", Serialize(i16, 8, 16, true));
    EXPECT_EQ("0x00003039", Serialize(i16, 8, 16, true, true));
    EXPECT_EQ("00003039", Serialize(i16, 8, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeUInt16)
{
    uint16 u16 = 34567;
    EXPECT_EQ("34567", Serialize(u16));
    EXPECT_EQ("   34567", Serialize(u16, 8));
    EXPECT_EQ("34567   ", Serialize(u16, -8));
    EXPECT_EQ("1000011100000111", Serialize(u16, 16, 2));
    EXPECT_EQ("  103407", Serialize(u16, 8, 8));
    EXPECT_EQ("   34567", Serialize(u16, 8, 10));
    EXPECT_EQ("    8707", Serialize(u16, 8, 16));
    EXPECT_EQ("  0x8707", Serialize(u16, 8, 16, true));
    EXPECT_EQ("0x00008707", Serialize(u16, 8, 16, true, true));
    EXPECT_EQ("00008707", Serialize(u16, 8, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeInt32)
{
    int32 i32 = 1234567890l;
    EXPECT_EQ("1234567890", Serialize(i32));
    EXPECT_EQ("  1234567890", Serialize(i32, 12));
    EXPECT_EQ("1234567890  ", Serialize(i32, -12));
    EXPECT_EQ(" 1001001100101100000001011010010", Serialize(i32, 32, 2));
    EXPECT_EQ(" 11145401322", Serialize(i32, 12, 8));
    EXPECT_EQ("  1234567890", Serialize(i32, 12, 10));
    EXPECT_EQ("    499602D2", Serialize(i32, 12, 16));
    EXPECT_EQ("  0x499602D2", Serialize(i32, 12, 16, true));
    EXPECT_EQ("0x0000499602D2", Serialize(i32, 12, 16, true, true));
    EXPECT_EQ("0000499602D2", Serialize(i32, 12, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeUInt32)
{
    uint32 u32 = 2345678900ul;
    EXPECT_EQ("2345678900", Serialize(u32));
    EXPECT_EQ("  2345678900", Serialize(u32, 12));
    EXPECT_EQ("2345678900  ", Serialize(u32, -12));
    EXPECT_EQ("10001011110100000011100000110100", Serialize(u32, 32, 2));
    EXPECT_EQ(" 21364034064", Serialize(u32, 12, 8));
    EXPECT_EQ("  2345678900", Serialize(u32, 12, 10));
    EXPECT_EQ("    8BD03834", Serialize(u32, 12, 16));
    EXPECT_EQ("  0x8BD03834", Serialize(u32, 12, 16, true));
    EXPECT_EQ("0x00008BD03834", Serialize(u32, 12, 16, true, true));
    EXPECT_EQ("00008BD03834", Serialize(u32, 12, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeInt64)
{
    int64 i64 = 9223372036854775807ll;
    EXPECT_EQ("9223372036854775807", Serialize(i64));
    EXPECT_EQ("     9223372036854775807", Serialize(i64, 24));
    EXPECT_EQ("9223372036854775807     ", Serialize(i64, -24));
    EXPECT_EQ(" 111111111111111111111111111111111111111111111111111111111111111", Serialize(i64, 64, 2));
    EXPECT_EQ("   777777777777777777777", Serialize(i64, 24, 8));
    EXPECT_EQ("     9223372036854775807", Serialize(i64, 24, 10));
    EXPECT_EQ("        7FFFFFFFFFFFFFFF", Serialize(i64, 24, 16));
    EXPECT_EQ("      0x7FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, true));
    EXPECT_EQ("0x000000007FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, true, true));
    EXPECT_EQ("000000007FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeUInt64)
{
    uint64 u64 = 9223372036854775808ull;
    EXPECT_EQ("9223372036854775808", Serialize(u64));
    EXPECT_EQ("     9223372036854775808", Serialize(u64, 24));
    EXPECT_EQ("9223372036854775808     ", Serialize(u64, -24));
    EXPECT_EQ("1000000000000000000000000000000000000000000000000000000000000000", Serialize(u64, 64, 2));
    EXPECT_EQ("  1000000000000000000000", Serialize(u64, 24, 8));
    EXPECT_EQ("     9223372036854775808", Serialize(u64, 24, 10));
    EXPECT_EQ("        8000000000000000", Serialize(u64, 24, 16));
    EXPECT_EQ("      0x8000000000000000", Serialize(u64, 24, 16, true));
    EXPECT_EQ("0x000000008000000000000000", Serialize(u64, 24, 16, true, true));
    EXPECT_EQ("000000008000000000000000", Serialize(u64, 24, 16, false, true));
}

TEST_FIXTURE(SerializationTest, SerializeFloat)
{
    float f = 1.23456789F;
    EXPECT_EQ("1.2345679", Serialize(f));
    EXPECT_EQ("   1.2345679", Serialize(f, 12));
    EXPECT_EQ("1.2345679   ", Serialize(f, -12));
    EXPECT_EQ("        1.23", Serialize(f, 12, 2));
    EXPECT_EQ("   1.2345679", Serialize(f, 12, 7));
    EXPECT_EQ("   1.2345679", Serialize(f, 12, 8));
}

TEST_FIXTURE(SerializationTest, SerializeDouble)
{
    double d = 1.234567890123456;
    EXPECT_EQ("1.23456789012346", Serialize(d));
    EXPECT_EQ("  1.23456789012346", Serialize(d, 18));
    EXPECT_EQ("1.23456789012346  ", Serialize(d, -18));
    EXPECT_EQ("           1.23457", Serialize(d, 18, 5));
    EXPECT_EQ("         1.2345679", Serialize(d, 18, 7));
    EXPECT_EQ("    1.234567890123", Serialize(d, 18, 12));
}

TEST_FIXTURE(SerializationTest, SerializeString)
{
    string s = "hello world";
    EXPECT_EQ("hello world", Serialize(s));
    EXPECT_EQ("    hello world", Serialize(s, 15));
    EXPECT_EQ("hello world    ", Serialize(s, -15));
    EXPECT_EQ("  \"hello world\"", Serialize(s, 15, true));
}

TEST_FIXTURE(SerializationTest, SerializeConstCharPtr)
{
    const char* s = "hello world";
    EXPECT_EQ("hello world", Serialize(s));
    EXPECT_EQ("    hello world", Serialize(s, 15));
    EXPECT_EQ("hello world    ", Serialize(s, -15));
    EXPECT_EQ("  \"hello world\"", Serialize(s, 15, true));
}

TEST_FIXTURE(SerializationTest, SerializeConstVoidPtr)
{
    const void* pvc = reinterpret_cast<const void*>(0x0123456789ABCDEF);
    EXPECT_EQ("0x0123456789ABCDEF", Serialize(pvc));
    EXPECT_EQ("  0x0123456789ABCDEF", Serialize(pvc, 20));
    EXPECT_EQ("0x0123456789ABCDEF  ", Serialize(pvc, -20));
}

TEST_FIXTURE(SerializationTest, SerializeVoidPtr)
{
    void* pv = reinterpret_cast<void*>(0x0123456789ABCDEF);
    EXPECT_EQ("0x0123456789ABCDEF", Serialize(pv));
    EXPECT_EQ("  0x0123456789ABCDEF", Serialize(pv, 20));
    EXPECT_EQ("0x0123456789ABCDEF  ", Serialize(pv, -20));
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
