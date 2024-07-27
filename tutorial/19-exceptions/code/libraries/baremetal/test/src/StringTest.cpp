//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : StringTest.cpp
//
// Namespace   : baremetal
//
// Class       : StringTest
//
// Description : string class tests
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

#include "baremetal/String.h"
#include "baremetal/Util.h"

using namespace unittest;

namespace baremetal {
namespace test {

TEST_SUITE(Baremetal)
{

class StringTest
    : public TestFixture
{
public:
    const char* otherText = "abcdefghijklmnopqrstuvwxyz";
    string other;
    void SetUp() override
    {
        other = otherText;
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(StringTest, ConstructDefault)
{
    string s;
    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(size_t{ 0 }, s.size());
    EXPECT_EQ(size_t{ 0 }, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
}

TEST_FIXTURE(StringTest, ConstructConstCharPtr)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(expected);

    string s(text);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructConstCharPtrEmpty)
{
    const char* text = "";
    const char* expected = "";
    size_t expectedLength = strlen(expected);

    string s(text);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructNullPtr)
{
    const char* expected = "";
    size_t expectedLength = strlen(expected);

    string s(nullptr);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructConstCharPtrAndSize)
{
    const char* text = otherText;
    const char* expected = "abcde";
    size_t expectedLength = strlen(expected);
    size_t length = expectedLength;

    string s(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructConstCharPtrAndSizeTooLarge)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(otherText) + 3;

    string s(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructConstCharPtrAndSizeNpos)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = string::npos;

    string s(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructCountAndChar)
{
    char ch = 'X';
    const char* expected = "XXXXX";
    size_t expectedLength = strlen(expected);
    size_t length = expectedLength;

    string s(length, ch);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructCountNposAndChar)
{
    char ch = 'X';
    size_t expectedLength = 0x7FFFF;
    size_t length = string::npos;

    string s(length, ch);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0x80000 }, s.capacity());
}

TEST_FIXTURE(StringTest, ConstructCopy)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string other(text);

    string s(other);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructCopyEmpty)
{
    const char* text = "";
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string other(text);

    string s(other);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructMove)
{
    // Cannot test yet, as we don't have a move operator
}

TEST_FIXTURE(StringTest, ConstructStringFromPos)
{
    const char* expected = "ghijklmnopqrstuvwxyz";
    size_t expectedLength = strlen(expected);
    size_t pos = 6;

    string s(other, pos);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructStringFromPosTooLarge)
{
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t pos = strlen(otherText) + 1;

    string s(other, pos);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructStringFromPosNpos)
{
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t pos = string::npos;

    string s(other, pos);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructStringFromPosWithCount)
{
    const char* expected = "ghi";
    size_t expectedLength = strlen(expected);
    size_t length = expectedLength;
    size_t pos = 6;

    string s(other, pos, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstructStringFromPosWithCountTooLarge)
{
    const char* expected = "xyz";
    size_t expectedLength = strlen(expected);
    size_t length = 6;
    size_t pos = 23;

    string s(other, pos, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, ConstCharPtrCastOperator)
{
    const char* text = otherText;
    const char* expected = otherText;

    string s(text);

    ASSERT_NOT_NULL((const char*)s);
    EXPECT_EQ(0, strcmp(text, (const char*)s));
}

TEST_FIXTURE(StringTest, AssignmentOperatorConstCharPtr)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);

    string s;

    s = text;
    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignmentOperatorCopy)
{
    const char* text = otherText;
    string other(text);
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string s;

    s = other;
    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignmentOperatorMove)
{
    // Cannot test yet, as we don't have a move operator
}

TEST_FIXTURE(StringTest, Begin)
{
    string s{ otherText };

    auto begin = s.begin();
    EXPECT_EQ('a', *begin);
}

TEST_FIXTURE(StringTest, End)
{
    string s{ otherText };

    auto end = s.end();
    EXPECT_EQ('\0', *end);
}

TEST_FIXTURE(StringTest, BeginConst)
{
    string s{ otherText };

    auto const begin = s.begin();
    EXPECT_EQ('a', *begin);
}

TEST_FIXTURE(StringTest, EndConst)
{
    string s{ otherText };

    auto const end = s.end();
    EXPECT_EQ('\0', *end);
}

TEST_FIXTURE(StringTest, Iterate)
{
    string s{ otherText };
    const char* expected = otherText;
    size_t index{};

    for (auto ch : s)
    {
        EXPECT_EQ(expected[index++], ch);
    }
}

TEST_FIXTURE(StringTest, AssignConstCharPtr)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string s;

    s.assign(text);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignConstCharPtrEmpty)
{
    const char* text = "";
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string s;

    s.assign(text);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignNullPtr)
{
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    string s;

    s.assign(nullptr);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignConstCharPtrAndSize)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(expected);
    string s;

    s.assign(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignConstCharPtrAndSizeTooLarge)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(otherText) + 3;
    string s;

    s.assign(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignConstCharPtrAndSizeNpos)
{
    const char* text = otherText;
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = string::npos;
    string s;

    s.assign(text, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignCountAndChar)
{
    char ch = 'X';
    const char* expected = "XXXXX";
    size_t expectedLength = strlen(expected);
    size_t length = 5;
    string s;

    s.assign(length, ch);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignCountNposAndChar)
{
    char ch = 'X';
    size_t expectedLength = 0x7FFFF;
    size_t length = string::npos;
    string s;

    s.assign(length, ch);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0x80000 }, s.capacity());
}

TEST_FIXTURE(StringTest, AssignString)
{
    const char* text = otherText;
    string other(text);
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string s;

    s.assign(other);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringEmpty)
{
    const char* text = "";
    string other(text);
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t length = strlen(text);
    string s;

    s.assign(other);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringFromPos)
{
    const char* expected = "ghijklmnopqrstuvwxyz";
    size_t expectedLength = strlen(expected);
    size_t pos = 6;
    string s;

    s.assign(other, pos);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringFromPosTooLarge)
{
    string str = "abc";
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t pos = 6;
    size_t length = 0;
    string s;

    s.assign(str, pos);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringFromPosNPos)
{
    string str = "abc";
    const char* expected = "";
    size_t expectedLength = strlen(expected);
    size_t pos = string::npos;
    size_t length = 0;
    string s;

    s.assign(str, pos);

    EXPECT_TRUE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_EQ('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringFromPosWithCount)
{
    const char* expected = "ghi";
    size_t expectedLength = strlen(expected);
    size_t pos = 6;
    size_t length = 3;
    string s;

    s.assign(other, pos, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, AssignStringFromPosWithCountTooLarge)
{
    const char* expected = "xyz";
    size_t expectedLength = strlen(expected);
    size_t pos = 23;
    size_t length = 6;
    string s;

    s.assign(other, pos, length);

    EXPECT_FALSE(s.empty());
    ASSERT_NOT_NULL(s.data());
    ASSERT_NOT_NULL(s.c_str());
    EXPECT_NE('\0', s.data()[0]);
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_NE(size_t{ 0 }, s.capacity());
    EXPECT_TRUE(expected == s);
    EXPECT_TRUE(s == expected);
    EXPECT_EQ(expected, s);
    EXPECT_EQ(s, expected);
}

TEST_FIXTURE(StringTest, At)
{
    string s{other};

    EXPECT_EQ('d', s.at(3));
}

TEST_FIXTURE(StringTest, AtConst)
{
    const string s{other};

    EXPECT_EQ('d', s.at(3));
}

TEST_FIXTURE(StringTest, AtOutsideString)
{
    string s{other};

    EXPECT_EQ('\0', s.at(other.size()));
}

TEST_FIXTURE(StringTest, AtConstOutsideString)
{
    const string s{other};

    EXPECT_EQ('\0', s.at(other.size()));
}

TEST_FIXTURE(StringTest, AtEmptyString)
{
    string s;

    EXPECT_EQ('\0', s.at(3));
}

TEST_FIXTURE(StringTest, AtConstEmptyString)
{
    string s;

    EXPECT_EQ('\0', s.at(3));
}

TEST_FIXTURE(StringTest, Front)
{
    string s{other};

    EXPECT_EQ('a', s.front());
}

TEST_FIXTURE(StringTest, FrontConst)
{
    const string s{other};

    EXPECT_EQ('a', s.front());
}

TEST_FIXTURE(StringTest, FrontEmptyString)
{
    string s;

    EXPECT_EQ('\0', s.front());
}

TEST_FIXTURE(StringTest, FrontConstEmptyString)
{
    const string s;

    EXPECT_EQ('\0', s.front());
}

TEST_FIXTURE(StringTest, Back)
{
    string s{other};

    EXPECT_EQ('z', s.back());
}

TEST_FIXTURE(StringTest, BackConst)
{
    const string s{other};

    EXPECT_EQ('z', s.back());
}

TEST_FIXTURE(StringTest, BackEmptyString)
{
    string s;

    EXPECT_EQ('\0', s.back());
}

TEST_FIXTURE(StringTest, BackConstEmptyString)
{
    const string s;

    EXPECT_EQ('\0', s.back());
}

TEST_FIXTURE(StringTest, IndexOperator)
{
    string s{other};

    EXPECT_EQ('d', s[3]);
}

TEST_FIXTURE(StringTest, IndexOperatorConst)
{
    const string s{other};

    EXPECT_EQ('d', s[3]);
}

TEST_FIXTURE(StringTest, IndexOperatorOutsideString)
{
    string s{other};

    EXPECT_EQ('\0', s[other.size()]);
}

TEST_FIXTURE(StringTest, IndexOperatorConstOutsideString)
{
    const string s{other};

    EXPECT_EQ('\0', s[other.size()]);
}

TEST_FIXTURE(StringTest, IndexOperatorEmptyString)
{
    string s;

    EXPECT_EQ('\0', s[3]);
}

TEST_FIXTURE(StringTest, IndexOperatorConstEmptyString)
{
    const string s;

    EXPECT_EQ('\0', s[3]);
}

TEST_FIXTURE(StringTest, Data)
{
    string s{other};

    char* text = s.data();

    EXPECT_EQ(0, strcmp(text, otherText));
}

TEST_FIXTURE(StringTest, DataConst)
{
    string s{other};

    const char* text = s.data();

    EXPECT_EQ(0, strcmp(text, otherText));
}

TEST_FIXTURE(StringTest, CString)
{
    string s{other};

    const char* text = s.c_str();

    EXPECT_EQ(0, strcmp(text, otherText));
}

TEST_FIXTURE(StringTest, Reserve)
{
    string s{other};

    auto oldCapacity = s.capacity();
    size_t reserveCapacity = 4096;

    s.reserve(reserveCapacity);

    auto newCapacity = s.capacity();

    // Check first, we expect the default capacity to be 64
    ASSERT_EQ(size_t{ 64 }, oldCapacity);
    EXPECT_NE(oldCapacity, newCapacity);
    EXPECT_EQ(reserveCapacity, newCapacity);
}

TEST_FIXTURE(StringTest, AddAssignmentChar)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzA";
    size_t expectedLength = strlen(expected);
    char c = 'A';

    s += c;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddAssignmentString)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";

    s += str;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddAssignmentStringEmpty)
{
    string s{ other };
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    string str = "";

    s += str;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddAssignmentConstCharPtr)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    const char*str = "ABCDEF";

    s += str;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddAssignmentConstCharPtrEmpty)
{
    string s{ other };
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    const char* str = "";

    s += str;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddAssignmentNullPtr)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyz";
    size_t expectedLength = strlen(expected);
    const char* str = nullptr;

    s += str;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendCountAndChar)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzAAAA";
    size_t expectedLength = strlen(expected);
    size_t count = 4;
    char c = 'A';

    s.append(count, c);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendCountNposAndChar)
{
    string s{ other };
    size_t expectedLength = 0x7FFFF;
    size_t count = string::npos;
    char c = 'A';

    s.append(count, c);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(expectedLength, s.size());
    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(size_t{ 0x80000 }, s.capacity());
}

TEST_FIXTURE(StringTest, AppendString)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";

    s.append(str);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringEmpty)
{
    string s{ other };
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    string str = "";

    s.append(str);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringAtPos)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzDEF";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";
    size_t pos = 3;

    s.append(str, pos);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringAtPosTooLarge)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyz";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";
    size_t pos = strlen(str) + 3;

    s.append(str, pos);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringAtPosNpos)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyz";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";
    size_t pos = string::npos;

    s.append(str, pos);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringAtPosWithCount)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzBCD";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";
    size_t pos = 1;
    size_t count = 3;

    s.append(str, pos, count);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendStringAtPosWithCountTooLarge)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzDEF";
    size_t expectedLength = strlen(expected);
    string str = "ABCDEF";
    size_t pos = 3;
    size_t count = pos + strlen(str) + 3;

    s.append(str, pos, count);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendConstCharPtr)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    const char* str = "ABCDEF";

    s.append(str);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendConstCharPtrEmpty)
{
    string s{ other };
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    const char* str = "";

    s.append(str);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendNullPtr)
{
    string s{ other };
    const char* expected = otherText;
    size_t expectedLength = strlen(expected);
    const char* str = nullptr;

    s.append(str);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendConstCharPtrWithCount)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCD";
    size_t expectedLength = strlen(expected);
    const char* str = "ABCDEF";
    size_t count = 4;

    s.append(str, count);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendConstCharPtrWithCountTooLarge)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    const char* str = "ABCDEF";
    size_t count = strlen(str) + 3;

    s.append(str, count);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AppendConstCharPtrWithCountNpos)
{
    string s{ other };
    const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
    size_t expectedLength = strlen(expected);
    const char* str = "ABCDEF";
    size_t count = string::npos;

    s.append(str, count);

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, Clear)
{
    string s{ other };
    const char* expected = "";
    size_t expectedLength = strlen(expected);

    s.clear();

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, FindString)
{
    string s{ other };
    string subString = "c";
    size_t expected = 2;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringEndOfString)
{
    string s{ other };
    string subString = "xyz";
    size_t expected = 23;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringNoMatch)
{
    string s{ other };
    string subString = "deg";
    size_t expected = string::npos;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringEmpty)
{
    string s{ other };
    string subString = "";
    size_t expected = 0;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPos)
{
    string s{ other };
    string subString = "c";
    size_t pos = 1;
    size_t expected = 2;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPosEndOfString)
{
    string s{ other };
    string subString = "xyz";
    size_t pos = 3;
    size_t expected = 23;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPosTooLarge)
{
    string s{ other };
    string subString = "c";
    size_t pos = 3;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPosNpos)
{
    string s{ other };
    string subString = "c";
    size_t pos = string::npos;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPosEmpty)
{
    string s{ other };
    string subString = "";
    size_t pos = 3;
    size_t expected = 3;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindStringAtPosWithCount)
{
    string s{ other };
    string subString = "def";
    size_t pos = 3;
    size_t count = 3;
    size_t expected = 3;

    // Uses implicit const char* cast
    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtr)
{
    string s{ other };
    const char* subString = "c";
    size_t expected = 2;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrNoMatch)
{
    string s{ other };
    const char* subString = "deg";
    size_t expected = string::npos;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrEmpty)
{
    string s{ other };
    const char* subString = "";
    size_t expected = 0;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrNullPtr)
{
    string s{ other };
    const char* subString = nullptr;
    size_t expected = 0;

    auto actual = s.find(subString);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPos)
{
    string s{ other };
    const char* subString = "c";
    size_t pos = 1;
    size_t expected = 2;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosEndOfString)
{
    string s{ other };
    const char* subString = "xyz";
    size_t pos = 3;
    size_t expected = 23;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosTooLarge)
{
    string s{ other };
    const char* subString = "c";
    size_t pos = 3;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosNpos)
{
    string s{ other };
    const char* subString = "c";
    size_t pos = string::npos;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosEmpty)
{
    string s{ other };
    const char* subString = "";
    size_t pos = 3;
    size_t expected = 3;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosTooLargeEmpty)
{
    string s{ other };
    const char* subString = "";
    size_t pos = 26;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosNullPtr)
{
    string s{ other };
    const char* subString = nullptr;
    size_t pos = 3;
    size_t expected = 3;

    auto actual = s.find(subString, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCount)
{
    string s{ other };
    const char* subString = "def";
    size_t pos = 3;
    size_t count = 3;
    size_t expected = 3;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountNoMatch)
{
    string s{ other };
    const char* subString = "deg";
    size_t pos = 3;
    size_t count = 3;
    size_t expected = string::npos;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountMatchPart)
{
    string s{ other };
    const char* subString = "def";
    size_t pos = 3;
    size_t count = 2;
    size_t expected = 3;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountTooLarge)
{
    string s{ other };
    const char* subString = "def";
    size_t pos = 3;
    size_t count = 4;
    size_t expected = 3;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountEmpty)
{
    string s{ other };
    const char* subString = "";
    size_t pos = 3;
    size_t count = 4;
    size_t expected = 3;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountTooLargeEmpty)
{
    string s{ other };
    const char* subString = "";
    size_t pos = 20;
    size_t count = 7;
    size_t expected = 20;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountNullPtr)
{
    string s{ other };
    const char* subString = nullptr;
    size_t pos = 3;
    size_t count = 4;
    size_t expected = 3;

    auto actual = s.find(subString, pos, count);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindChar)
{
    string s{ other };
    char c = 'd';
    size_t expected = 3;

    auto actual = s.find(c);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindCharNoMatch)
{
    string s{ other };
    char c = 'A';
    size_t expected = string::npos;

    auto actual = s.find(c);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindCharAtPos)
{
    string s{ other };
    char c = 'd';
    size_t pos = 2;
    size_t expected = 3;

    auto actual = s.find(c, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindCharAtPosTooLarge)
{
    string s{ other };
    char c = 'd';
    size_t pos = 4;
    size_t expected = string::npos;

    auto actual = s.find(c, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, FindCharAtPosNpos)
{
    string s{ other };
    char c = 'd';
    size_t pos = string::npos;
    size_t expected = string::npos;

    auto actual = s.find(c, pos);

    EXPECT_EQ(expected, actual);
}

TEST_FIXTURE(StringTest, StartsWithString)
{
    string s{ other };
    string s1 = "abc";
    string s2 = "xyz";
    string s3 = "";

    EXPECT_TRUE(s.starts_with(s1));
    EXPECT_FALSE(s.starts_with(s2));
    EXPECT_TRUE(s.starts_with(s3));
}

TEST_FIXTURE(StringTest, StartsWithConstCharPtr)
{
    string s{ other };
    const char* s1 = "abc";
    const char* s2 = "xyz";
    const char* s3 = "";
    const char* s4 = nullptr;

    EXPECT_TRUE(s.starts_with(s1));
    EXPECT_FALSE(s.starts_with(s2));
    EXPECT_TRUE(s.starts_with(s3));
    EXPECT_TRUE(s.starts_with(s4));
}

TEST_FIXTURE(StringTest, StartsWithChar)
{
    string s{ other };
    char c1 = 'a';
    char c2 = 'z';

    EXPECT_TRUE(s.starts_with(c1));
    EXPECT_FALSE(s.starts_with(c2));
}

TEST_FIXTURE(StringTest, EndsWithString)
{
    string s{ other };
    string s1 = "abc";
    string s2 = "xyz";
    string s3 = "";

    EXPECT_FALSE(s.ends_with(s1));
    EXPECT_TRUE(s.ends_with(s2));
    EXPECT_TRUE(s.ends_with(s3));
}

TEST_FIXTURE(StringTest, EndsWithConstCharPtr)
{
    string s{ other };
    const char* s1 = "abc";
    const char* s2 = "xyz";
    const char* s3 = "";
    const char* s4 = nullptr;

    EXPECT_FALSE(s.ends_with(s1));
    EXPECT_TRUE(s.ends_with(s2));
    EXPECT_TRUE(s.ends_with(s3));
    EXPECT_TRUE(s.ends_with(s4));
}

TEST_FIXTURE(StringTest, EndsWithChar)
{
    string s{ other };
    char c1 = 'a';
    char c2 = 'z';

    EXPECT_FALSE(s.ends_with(c1));
    EXPECT_TRUE(s.ends_with(c2));
}

TEST_FIXTURE(StringTest, ContainsString)
{
    string s{ other };
    string s1 = "abc";
    string s2 = "xyz";
    string s3 = "ABC";
    string s4 = "";

    EXPECT_TRUE(s.contains(s1));
    EXPECT_TRUE(s.contains(s2));
    EXPECT_FALSE(s.contains(s3));
    EXPECT_TRUE(s.contains(s4));
}

TEST_FIXTURE(StringTest, ContainsConstCharPtr)
{
    string s{ other };
    const char* s1 = "abc";
    const char* s2 = "xyz";
    const char* s3 = "ABC";
    const char* s4 = "";
    const char* s5 = nullptr;

    EXPECT_TRUE(s.contains(s1));
    EXPECT_TRUE(s.contains(s2));
    EXPECT_FALSE(s.contains(s3));
    EXPECT_TRUE(s.contains(s4));
    EXPECT_TRUE(s.contains(s5));
}

TEST_FIXTURE(StringTest, ContainsChar)
{
    string s{ other };
    char c1 = 'a';
    char c2 = 'z';
    char c3 = '+';

    EXPECT_TRUE(s.contains(c1));
    EXPECT_TRUE(s.contains(c2));
    EXPECT_FALSE(s.contains(c3));
}

TEST_FIXTURE(StringTest, SubStr)
{
    string s{ other };
    string expected1 = other;
    string expected2 = "ghijklmnopqrstuvwxyz";
    string expected3 = "ghij";
    string expected4 = "";
    string expected5 = "ghijklmnopqrstuvwxyz";

    EXPECT_EQ(expected1, s.substr());
    EXPECT_EQ(expected2, s.substr(6));
    EXPECT_EQ(expected3, s.substr(6, 4));
    EXPECT_EQ(expected4, s.substr(30));
    EXPECT_EQ(expected5, s.substr(6, 30));
}

TEST_FIXTURE(StringTest, Equals)
{
    string s1{ "abcdefg" };
    string s2{ "abcdefG" };
    string s3{ "abcdefg" };
    string s4{ "" };
    const char* s5 = "abcefg";
    const char* s6 = "abcdefg";
    const char* s7 = "abcdefG";

    EXPECT_FALSE(s1.equals(s2));
    EXPECT_TRUE(s1.equals(s3));
    EXPECT_FALSE(s1.equals(s4));
    EXPECT_FALSE(s1.equals(s5));
    EXPECT_TRUE(s1.equals(s6));
    EXPECT_FALSE(s1.equals(s7));
    EXPECT_FALSE(s1.equals(""));
    EXPECT_FALSE(s1.equals(nullptr));
    EXPECT_FALSE(s4.equals(s3));
    EXPECT_TRUE(s4.equals(""));
    EXPECT_TRUE(s4.equals(nullptr));
}

TEST_FIXTURE(StringTest, EqualsCaseInsensitive)
{
    string s1{ "abcdefg" };
    string s2{ "abcdefG" };
    string s3{ "abcdefg" };
    string s4{ "" };
    const char* s5 = "abcefg";
    const char* s6 = "abcdefg";
    const char* s7 = "abcdefG";

    EXPECT_TRUE(s1.equals_case_insensitive(s2));
    EXPECT_TRUE(s1.equals_case_insensitive(s3));
    EXPECT_FALSE(s1.equals_case_insensitive(s4));
    EXPECT_FALSE(s1.equals_case_insensitive(s5));
    EXPECT_TRUE(s1.equals_case_insensitive(s6));
    EXPECT_TRUE(s1.equals_case_insensitive(s7));
    EXPECT_FALSE(s1.equals_case_insensitive(""));
    EXPECT_FALSE(s1.equals_case_insensitive(nullptr));
    EXPECT_FALSE(s4.equals_case_insensitive(s3));
    EXPECT_TRUE(s4.equals_case_insensitive(""));
    EXPECT_TRUE(s4.equals_case_insensitive(nullptr));
}

TEST_FIXTURE(StringTest, Compare)
{
    string s1{ "abcdefg" };
    string s2{ "abcdefG" };
    string s3{ "abcdefg" };
    string s4{ "bcdefg" };
    string s5{ "" };
    const char* s6 = "abcdefg";
    const char* s7 = "abcdefG";

    EXPECT_EQ(1, s1.compare(s2));
    EXPECT_EQ(-1, s2.compare(s1));
    EXPECT_EQ(0, s1.compare(s3));
    EXPECT_EQ(0, s3.compare(s1));
    EXPECT_EQ(0, s1.compare(1, 6, s4));
    EXPECT_EQ(-1, s1.compare(1, 5, s4));
    EXPECT_EQ(0, s1.compare(3, 6, s4, 2));
    EXPECT_EQ(1, s1.compare(3, 6, s4, 2, 1));
    EXPECT_EQ(0, s1.compare(3, 6, s4, 2, 6));
    EXPECT_EQ(1, s1.compare("a"));
    EXPECT_EQ(1, s1.compare("Abcdefg"));
    EXPECT_EQ(-1, s1.compare("abdecfg"));
    EXPECT_EQ(1, s1.compare(""));
    EXPECT_EQ(1, s1.compare(nullptr));
    EXPECT_EQ(-1, s5.compare("a"));
    EXPECT_EQ(0, s5.compare(""));
    EXPECT_EQ(0, s5.compare(nullptr));
}

TEST_FIXTURE(StringTest, ReplacePosCountString)
{
    string s1{ "abcde" };
    string s2{ "fghijk" };

    auto s3 = s1.replace(0, 1, s2);
    EXPECT_EQ("fghijkbcde", s1);
    EXPECT_EQ("fghijkbcde", s3);
}

TEST_FIXTURE(StringTest, ReplacePosCountStringPos)
{
    string s1{ "abcde" };
    string s2{ "fghijk" };

    auto s3 = s1.replace(1, 2, s2, 2);
    EXPECT_EQ("ahijkde", s1);
    EXPECT_EQ("ahijkde", s3);
}

TEST_FIXTURE(StringTest, ReplacePosCountStringPosCount)
{
    string s1{ "abcde" };
    string s2{ "fghijk" };

    auto s3 = s1.replace(1, 2, s2, 2, 2);
    EXPECT_EQ("ahide", s1);
    EXPECT_EQ("ahide", s3);
}

TEST_FIXTURE(StringTest, ReplacePosCountConstCharPtr)
{
    string s1{ "abcde" };

    auto s2 = s1.replace(0, 1, "uvwxyz");
    EXPECT_EQ("uvwxyzbcde", s1);
    EXPECT_EQ("uvwxyzbcde", s2);
}

TEST_FIXTURE(StringTest, ReplacePosCountConstCharPtrCount)
{
    string s1{ "abcde" };

    auto s2 = s1.replace(1, 2, "uvwxyz", 2);
    EXPECT_EQ("auvde", s1);
    EXPECT_EQ("auvde", s2);
}

TEST_FIXTURE(StringTest, ReplacePosChar)
{
    string s1{ "abcde" };

    auto s2 = s1.replace(0, 1, 'x');
    EXPECT_EQ("xbcde", s1);
    EXPECT_EQ("xbcde", s2);
}

TEST_FIXTURE(StringTest, ReplacePosCharCount)
{
    string s1{ "abcde" };

    auto s2 = s1.replace(1, 2, 'x', 3);
    EXPECT_EQ("axxxde", s1);
    EXPECT_EQ("axxxde", s2);
}

TEST_FIXTURE(StringTest, ReplaceSubstringString)
{
    string s1{ "abcde" };
    string s2{ "cd" };
    string s3{ "xy" };

    EXPECT_EQ(1, s1.replace(s2, s3));
    EXPECT_EQ("abxye", s1);
    EXPECT_EQ("cd", s2);
    EXPECT_EQ("xy", s3);
}

TEST_FIXTURE(StringTest, ReplaceSubstringStringMultiple)
{
    string s1{ "abababab" };
    string s2{ "ab" };
    string s3{ "cd" };

    EXPECT_EQ(4, s1.replace(s2, s3));
    EXPECT_EQ("cdcdcdcd", s1);
    EXPECT_EQ("ab", s2);
    EXPECT_EQ("cd", s3);
}

TEST_FIXTURE(StringTest, ReplaceSubstringConstCharPtr)
{
    string s1{ "abcde" };

    EXPECT_EQ(1, s1.replace("cd", "xy"));
    EXPECT_EQ("abxye", s1);
}

TEST_FIXTURE(StringTest, ReplaceSubstringConstCharPtrMultiple)
{
    string s1{ "abababab" };

    EXPECT_EQ(4, s1.replace("ab", "cd"));
    EXPECT_EQ("cdcdcdcd", s1);
}

TEST_FIXTURE(StringTest, Align)
{
    string s = "abcd";
    string expected1 = "    abcd";
    string expected2 = "abcd    ";
    string expected3 = "abcd";

    EXPECT_EQ(expected1, s.align(8));
    EXPECT_EQ(expected2, s.align(-8));
    EXPECT_EQ(expected3, s.align(0));
}

TEST_FIXTURE(StringTest, EqualityOperator)
{
    string s1{ "abcdefg" };
    string s2{ "abcdefG" };
    string s3{ "abcdefg" };
    string s4{ "bcdefg" };
    string s5{ "" };
    const char* s6 = "abcdefg";
    const char* s7 = "abcdefG";

    EXPECT_FALSE(s1 == s2);
    EXPECT_TRUE(s1 == s3);
    EXPECT_FALSE(s1 == s4);
    EXPECT_FALSE(s1 == s5);
    EXPECT_TRUE(s1 == s6);
    EXPECT_FALSE(s1 == s7);
    EXPECT_FALSE(s1 == "");
    EXPECT_FALSE(s1 == nullptr);
    EXPECT_FALSE(s2 == s1);
    EXPECT_TRUE(s3 == s1);
    EXPECT_FALSE(s4 == s1);
    EXPECT_FALSE(s5 == s1);
    EXPECT_TRUE(s6 == s1);
    EXPECT_FALSE(s7 == s1);
    EXPECT_FALSE("" == s1);
    EXPECT_FALSE(nullptr == s1);
    EXPECT_EQ(s1, s3);
    EXPECT_EQ(s1, s6);
}

TEST_FIXTURE(StringTest, InEqualityOperator)
{
    string s1{ "abcdefg" };
    string s2{ "abcdefG" };
    string s3{ "abcdefg" };
    string s4{ "bcdefg" };
    string s5{ "" };
    const char* s6 = "abcdefg";
    const char* s7 = "abcdefG";

    EXPECT_TRUE(s1 != s2);
    EXPECT_FALSE(s1 != s3);
    EXPECT_TRUE(s1 != s4);
    EXPECT_TRUE(s1 != s5);
    EXPECT_FALSE(s1 != s6);
    EXPECT_TRUE(s1 != s7);
    EXPECT_TRUE(s1 != "");
    EXPECT_TRUE(s1 != nullptr);
    EXPECT_TRUE(s2 != s1);
    EXPECT_FALSE(s3 != s1);
    EXPECT_TRUE(s4 != s1);
    EXPECT_TRUE(s5 != s1);
    EXPECT_FALSE(s6 != s1);
    EXPECT_TRUE(s7 != s1);
    EXPECT_TRUE("" != s1);
    EXPECT_TRUE(nullptr != s1);
    EXPECT_NE(s1, s2);
    EXPECT_NE(s1, s4);
    EXPECT_NE(s1, s5);
    EXPECT_NE(s1, s7);
}

TEST_FIXTURE(StringTest, AddOperatorStringAndString)
{
    string a = "ABC";
    string b = "def";
    const char* expected = "ABCdef";
    size_t expectedLength = strlen(expected);
    string s;
    
    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorStringAndConstCharPtr)
{
    string a = "ABC";
    const char* b = "def";
    const char* expected = "ABCdef";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorStringAndConstCharPtrEmpty)
{
    string a = "ABC";
    const char* b = "";
    const char* expected = "ABC";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorStringAndNullPtr)
{
    string a = "ABC";
    const char* b = nullptr;
    const char* expected = "ABC";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorConstCharPtrAndString)
{
    const char* a = "ABC";
    string b = "def";
    const char* expected = "ABCdef";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorConstCharPtrEmptyAndString)
{
    const char* a = "";
    string b = "def";
    const char* expected = "def";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorNullPtrAndString)
{
    const char* a = nullptr;
    string b = "def";
    const char* expected = "def";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorStringAndChar)
{
    string a = "ABC";
    char b = 'd';
    const char* expected = "ABCd";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

TEST_FIXTURE(StringTest, AddOperatorCharAndString)
{
    char a = 'd';
    string b = "ABC";
    const char* expected = "dABC";
    size_t expectedLength = strlen(expected);
    string s;

    s = a + b;

    EXPECT_EQ(expectedLength, s.length());
    EXPECT_EQ(expected, s);
}

} // suite Baremetal

} // namespace test
} // namespace baremetal