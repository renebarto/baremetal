//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : StringTest.cpp
//
// Namespace   : baremetal
//
// Class       : StringTest
//
// Description : String class tests
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

#include "baremetal/String.h"
#include "stdlib/Util.h"

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Minimum string allocation size
static constexpr size_t MinimumAllocationSize = 256;

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

    class StringTest : public TestFixture
    {
    public:
        const char* otherText = "abcdefghijklmnopqrstuvwxyz";
        String other;
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
        String s;
        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(size_t{0}, s.size());
        EXPECT_EQ(size_t{0}, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
    }

    TEST_FIXTURE(StringTest, ConstructConstCharPtr)
    {
        const char* text = otherText;
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        size_t length = strlen(expected);

        String s(text);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(text);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, ConstructNullPtr)
    {
        const char* expected = "";
        size_t expectedLength = strlen(expected);

        String s(nullptr);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
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

        String s(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        size_t length = String::npos;

        String s(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(length, ch);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, ConstructCountNposAndChar)
    {
        char ch = 'X';
        size_t expectedLength = 0x7FFFF;
        size_t length = String::npos;

        String s(length, ch);

        EXPECT_FALSE(s.empty());
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0x80000}, s.capacity());
    }

    TEST_FIXTURE(StringTest, ConstructCopy)
    {
        const char* text = otherText;
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        size_t length = strlen(text);
        String other(text);

        String s(other);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String other(text);

        String s(other);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(other, pos);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(other, pos);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, ConstructStringFromPosNpos)
    {
        const char* expected = "";
        size_t expectedLength = strlen(expected);
        size_t pos = String::npos;

        String s(other, pos);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
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

        String s(other, pos, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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

        String s(other, pos, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, ConstCharPtrCastOperator)
    {
        const char* text = otherText;
        const char* expected = otherText;

        String s(text);

        ASSERT_NOT_NULL((const char*)s);
        EXPECT_EQ(0, strcmp(text, (const char*)s));
    }

    TEST_FIXTURE(StringTest, AssignmentOperatorConstCharPtr)
    {
        const char* text = otherText;
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        size_t length = strlen(text);

        String s;

        s = text;
        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignmentOperatorCopy)
    {
        const char* text = otherText;
        String other(text);
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        size_t length = strlen(text);
        String s;

        s = other;
        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s{otherText};

        auto begin = s.begin();
        EXPECT_EQ('a', *begin);
    }

    TEST_FIXTURE(StringTest, End)
    {
        String s{otherText};

        auto end = s.end();
        EXPECT_EQ('\0', *end);
    }

    TEST_FIXTURE(StringTest, BeginConst)
    {
        String s{otherText};

        auto const begin = s.begin();
        EXPECT_EQ('a', *begin);
    }

    TEST_FIXTURE(StringTest, EndConst)
    {
        String s{otherText};

        auto const end = s.end();
        EXPECT_EQ('\0', *end);
    }

    TEST_FIXTURE(StringTest, Iterate)
    {
        String s{otherText};
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
        String s;

        s.assign(text);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(text);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignNullPtr)
    {
        const char* expected = "";
        size_t expectedLength = strlen(expected);
        String s;

        s.assign(nullptr);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        size_t length = String::npos;
        String s;

        s.assign(text, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(length, ch);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignCountNposAndChar)
    {
        char ch = 'X';
        size_t expectedLength = 0x7FFFF;
        size_t length = String::npos;
        String s;

        s.assign(length, ch);

        EXPECT_FALSE(s.empty());
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0x80000}, s.capacity());
    }

    TEST_FIXTURE(StringTest, AssignString)
    {
        const char* text = otherText;
        String other(text);
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        size_t length = strlen(text);
        String s;

        s.assign(other);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignStringEmpty)
    {
        const char* text = "";
        String other(text);
        const char* expected = "";
        size_t expectedLength = strlen(expected);
        size_t length = strlen(text);
        String s;

        s.assign(other);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(other, pos);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignStringFromPosTooLarge)
    {
        String str = "abc";
        const char* expected = "";
        size_t expectedLength = strlen(expected);
        size_t pos = 6;
        size_t length = 0;
        String s;

        s.assign(str, pos);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, AssignStringFromPosNPos)
    {
        String str = "abc";
        const char* expected = "";
        size_t expectedLength = strlen(expected);
        size_t pos = String::npos;
        size_t length = 0;
        String s;

        s.assign(str, pos);

        EXPECT_TRUE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_EQ('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0}, s.capacity());
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
        String s;

        s.assign(other, pos, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
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
        String s;

        s.assign(other, pos, length);

        EXPECT_FALSE(s.empty());
        ASSERT_NOT_NULL(s.data());
        ASSERT_NOT_NULL(s.c_str());
        EXPECT_NE('\0', s.data()[0]);
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_NE(size_t{0}, s.capacity());
        EXPECT_TRUE(expected == s);
        EXPECT_TRUE(s == expected);
        EXPECT_EQ(expected, s);
        EXPECT_EQ(s, expected);
    }

    TEST_FIXTURE(StringTest, At)
    {
        String s{other};

        EXPECT_EQ('d', s.at(3));
    }

    TEST_FIXTURE(StringTest, AtConst)
    {
        const String s{other};

        EXPECT_EQ('d', s.at(3));
    }

    TEST_FIXTURE(StringTest, AtOutsideString)
    {
        String s{other};

        EXPECT_EQ('\0', s.at(other.size()));
    }

    TEST_FIXTURE(StringTest, AtConstOutsideString)
    {
        const String s{other};

        EXPECT_EQ('\0', s.at(other.size()));
    }

    TEST_FIXTURE(StringTest, AtEmptyString)
    {
        String s;

        EXPECT_EQ('\0', s.at(3));
    }

    TEST_FIXTURE(StringTest, AtConstEmptyString)
    {
        String s;

        EXPECT_EQ('\0', s.at(3));
    }

    TEST_FIXTURE(StringTest, Front)
    {
        String s{other};

        EXPECT_EQ('a', s.front());
    }

    TEST_FIXTURE(StringTest, FrontConst)
    {
        const String s{other};

        EXPECT_EQ('a', s.front());
    }

    TEST_FIXTURE(StringTest, FrontEmptyString)
    {
        String s;

        EXPECT_EQ('\0', s.front());
    }

    TEST_FIXTURE(StringTest, FrontConstEmptyString)
    {
        const String s;

        EXPECT_EQ('\0', s.front());
    }

    TEST_FIXTURE(StringTest, Back)
    {
        String s{other};

        EXPECT_EQ('z', s.back());
    }

    TEST_FIXTURE(StringTest, BackConst)
    {
        const String s{other};

        EXPECT_EQ('z', s.back());
    }

    TEST_FIXTURE(StringTest, BackEmptyString)
    {
        String s;

        EXPECT_EQ('\0', s.back());
    }

    TEST_FIXTURE(StringTest, BackConstEmptyString)
    {
        const String s;

        EXPECT_EQ('\0', s.back());
    }

    TEST_FIXTURE(StringTest, IndexOperator)
    {
        String s{other};

        EXPECT_EQ('d', s[3]);
    }

    TEST_FIXTURE(StringTest, IndexOperatorConst)
    {
        const String s{other};

        EXPECT_EQ('d', s[3]);
    }

    TEST_FIXTURE(StringTest, IndexOperatorOutsideString)
    {
        String s{other};

        EXPECT_EQ('\0', s[other.size()]);
    }

    TEST_FIXTURE(StringTest, IndexOperatorConstOutsideString)
    {
        const String s{other};

        EXPECT_EQ('\0', s[other.size()]);
    }

    TEST_FIXTURE(StringTest, IndexOperatorEmptyString)
    {
        String s;

        EXPECT_EQ('\0', s[3]);
    }

    TEST_FIXTURE(StringTest, IndexOperatorConstEmptyString)
    {
        const String s;

        EXPECT_EQ('\0', s[3]);
    }

    TEST_FIXTURE(StringTest, Data)
    {
        String s{other};

        char* text = s.data();

        EXPECT_EQ(0, strcmp(text, otherText));
    }

    TEST_FIXTURE(StringTest, DataConst)
    {
        String s{other};

        const char* text = s.data();

        EXPECT_EQ(0, strcmp(text, otherText));
    }

    TEST_FIXTURE(StringTest, CString)
    {
        String s{other};

        const char* text = s.c_str();

        EXPECT_EQ(0, strcmp(text, otherText));
    }

    TEST_FIXTURE(StringTest, Reserve)
    {
        String s{other};

        auto oldCapacity = s.capacity();
        size_t reserveCapacity = 4096;

        s.reserve(reserveCapacity);

        auto newCapacity = s.capacity();

        // Check first, we expect the default capacity to be 64
        ASSERT_EQ(MinimumAllocationSize, oldCapacity);
        EXPECT_NE(oldCapacity, newCapacity);
        EXPECT_EQ(reserveCapacity, newCapacity);
    }

    TEST_FIXTURE(StringTest, AddAssignmentChar)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzA";
        size_t expectedLength = strlen(expected);
        char c = 'A';

        s += c;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddAssignmentString)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";

        s += str;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddAssignmentStringEmpty)
    {
        String s{other};
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        String str = "";

        s += str;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddAssignmentConstCharPtr)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
        size_t expectedLength = strlen(expected);
        const char* str = "ABCDEF";

        s += str;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddAssignmentConstCharPtrEmpty)
    {
        String s{other};
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        const char* str = "";

        s += str;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddAssignmentNullPtr)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyz";
        size_t expectedLength = strlen(expected);
        const char* str = nullptr;

        s += str;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendCountAndChar)
    {
        String s{other};
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
        String s{other};
        size_t expectedLength = 0x7FFFF;
        size_t count = String::npos;
        char c = 'A';

        s.append(count, c);

        EXPECT_FALSE(s.empty());
        EXPECT_EQ(expectedLength, s.size());
        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(size_t{0x80000}, s.capacity());
    }

    TEST_FIXTURE(StringTest, AppendString)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";

        s.append(str);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringEmpty)
    {
        String s{other};
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        String str = "";

        s.append(str);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringAtPos)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzDEF";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";
        size_t pos = 3;

        s.append(str, pos);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringAtPosTooLarge)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyz";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";
        size_t pos = strlen(str) + 3;

        s.append(str, pos);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringAtPosNpos)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyz";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";
        size_t pos = String::npos;

        s.append(str, pos);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringAtPosWithCount)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzBCD";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";
        size_t pos = 1;
        size_t count = 3;

        s.append(str, pos, count);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendStringAtPosWithCountTooLarge)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzDEF";
        size_t expectedLength = strlen(expected);
        String str = "ABCDEF";
        size_t pos = 3;
        size_t count = pos + strlen(str) + 3;

        s.append(str, pos, count);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendConstCharPtr)
    {
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
        size_t expectedLength = strlen(expected);
        const char* str = "ABCDEF";

        s.append(str);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendConstCharPtrEmpty)
    {
        String s{other};
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        const char* str = "";

        s.append(str);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendNullPtr)
    {
        String s{other};
        const char* expected = otherText;
        size_t expectedLength = strlen(expected);
        const char* str = nullptr;

        s.append(str);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AppendConstCharPtrWithCount)
    {
        String s{other};
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
        String s{other};
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
        String s{other};
        const char* expected = "abcdefghijklmnopqrstuvwxyzABCDEF";
        size_t expectedLength = strlen(expected);
        const char* str = "ABCDEF";
        size_t count = String::npos;

        s.append(str, count);

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, Clear)
    {
        String s{other};
        const char* expected = "";
        size_t expectedLength = strlen(expected);

        s.clear();

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, FindString)
    {
        String s{other};
        String subString = "c";
        size_t expected = 2;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringEndOfString)
    {
        String s{other};
        String subString = "xyz";
        size_t expected = 23;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringNoMatch)
    {
        String s{other};
        String subString = "deg";
        size_t expected = String::npos;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringEmpty)
    {
        String s{other};
        String subString = "";
        size_t expected = 0;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPos)
    {
        String s{other};
        String subString = "c";
        size_t pos = 1;
        size_t expected = 2;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPosEndOfString)
    {
        String s{other};
        String subString = "xyz";
        size_t pos = 3;
        size_t expected = 23;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPosTooLarge)
    {
        String s{other};
        String subString = "c";
        size_t pos = 3;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPosNpos)
    {
        String s{other};
        String subString = "c";
        size_t pos = String::npos;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPosEmpty)
    {
        String s{other};
        String subString = "";
        size_t pos = 3;
        size_t expected = 3;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindStringAtPosWithCount)
    {
        String s{other};
        String subString = "def";
        size_t pos = 3;
        size_t count = 3;
        size_t expected = 3;

        // Uses implicit const char* cast
        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtr)
    {
        String s{other};
        const char* subString = "c";
        size_t expected = 2;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrNoMatch)
    {
        String s{other};
        const char* subString = "deg";
        size_t expected = String::npos;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrEmpty)
    {
        String s{other};
        const char* subString = "";
        size_t expected = 0;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrNullPtr)
    {
        String s{other};
        const char* subString = nullptr;
        size_t expected = 0;

        auto actual = s.find(subString);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPos)
    {
        String s{other};
        const char* subString = "c";
        size_t pos = 1;
        size_t expected = 2;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosEndOfString)
    {
        String s{other};
        const char* subString = "xyz";
        size_t pos = 3;
        size_t expected = 23;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosTooLarge)
    {
        String s{other};
        const char* subString = "c";
        size_t pos = 3;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosNpos)
    {
        String s{other};
        const char* subString = "c";
        size_t pos = String::npos;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosEmpty)
    {
        String s{other};
        const char* subString = "";
        size_t pos = 3;
        size_t expected = 3;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosTooLargeEmpty)
    {
        String s{other};
        const char* subString = "";
        size_t pos = 26;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosNullPtr)
    {
        String s{other};
        const char* subString = nullptr;
        size_t pos = 3;
        size_t expected = 3;

        auto actual = s.find(subString, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCount)
    {
        String s{other};
        const char* subString = "def";
        size_t pos = 3;
        size_t count = 3;
        size_t expected = 3;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountNoMatch)
    {
        String s{other};
        const char* subString = "deg";
        size_t pos = 3;
        size_t count = 3;
        size_t expected = String::npos;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountMatchPart)
    {
        String s{other};
        const char* subString = "def";
        size_t pos = 3;
        size_t count = 2;
        size_t expected = 3;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountTooLarge)
    {
        String s{other};
        const char* subString = "def";
        size_t pos = 3;
        size_t count = 4;
        size_t expected = 3;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountEmpty)
    {
        String s{other};
        const char* subString = "";
        size_t pos = 3;
        size_t count = 4;
        size_t expected = 3;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountTooLargeEmpty)
    {
        String s{other};
        const char* subString = "";
        size_t pos = 20;
        size_t count = 7;
        size_t expected = 20;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindConstCharPtrAtPosWithCountNullPtr)
    {
        String s{other};
        const char* subString = nullptr;
        size_t pos = 3;
        size_t count = 4;
        size_t expected = 3;

        auto actual = s.find(subString, pos, count);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindChar)
    {
        String s{other};
        char c = 'd';
        size_t expected = 3;

        auto actual = s.find(c);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindCharNoMatch)
    {
        String s{other};
        char c = 'A';
        size_t expected = String::npos;

        auto actual = s.find(c);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindCharAtPos)
    {
        String s{other};
        char c = 'd';
        size_t pos = 2;
        size_t expected = 3;

        auto actual = s.find(c, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindCharAtPosTooLarge)
    {
        String s{other};
        char c = 'd';
        size_t pos = 4;
        size_t expected = String::npos;

        auto actual = s.find(c, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, FindCharAtPosNpos)
    {
        String s{other};
        char c = 'd';
        size_t pos = String::npos;
        size_t expected = String::npos;

        auto actual = s.find(c, pos);

        EXPECT_EQ(expected, actual);
    }

    TEST_FIXTURE(StringTest, StartsWithString)
    {
        String s{other};
        String s1 = "abc";
        String s2 = "xyz";
        String s3 = "";

        EXPECT_TRUE(s.starts_with(s1));
        EXPECT_FALSE(s.starts_with(s2));
        EXPECT_TRUE(s.starts_with(s3));
    }

    TEST_FIXTURE(StringTest, StartsWithConstCharPtr)
    {
        String s{other};
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
        String s{other};
        char c1 = 'a';
        char c2 = 'z';

        EXPECT_TRUE(s.starts_with(c1));
        EXPECT_FALSE(s.starts_with(c2));
    }

    TEST_FIXTURE(StringTest, EndsWithString)
    {
        String s{other};
        String s1 = "abc";
        String s2 = "xyz";
        String s3 = "";

        EXPECT_FALSE(s.ends_with(s1));
        EXPECT_TRUE(s.ends_with(s2));
        EXPECT_TRUE(s.ends_with(s3));
    }

    TEST_FIXTURE(StringTest, EndsWithConstCharPtr)
    {
        String s{other};
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
        String s{other};
        char c1 = 'a';
        char c2 = 'z';

        EXPECT_FALSE(s.ends_with(c1));
        EXPECT_TRUE(s.ends_with(c2));
    }

    TEST_FIXTURE(StringTest, ContainsString)
    {
        String s{other};
        String s1 = "abc";
        String s2 = "xyz";
        String s3 = "ABC";
        String s4 = "";

        EXPECT_TRUE(s.contains(s1));
        EXPECT_TRUE(s.contains(s2));
        EXPECT_FALSE(s.contains(s3));
        EXPECT_TRUE(s.contains(s4));
    }

    TEST_FIXTURE(StringTest, ContainsConstCharPtr)
    {
        String s{other};
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
        String s{other};
        char c1 = 'a';
        char c2 = 'z';
        char c3 = '+';

        EXPECT_TRUE(s.contains(c1));
        EXPECT_TRUE(s.contains(c2));
        EXPECT_FALSE(s.contains(c3));
    }

    TEST_FIXTURE(StringTest, SubStr)
    {
        String s{other};
        String expected1 = other;
        String expected2 = "ghijklmnopqrstuvwxyz";
        String expected3 = "ghij";
        String expected4 = "";
        String expected5 = "ghijklmnopqrstuvwxyz";

        EXPECT_EQ(expected1, s.substr());
        EXPECT_EQ(expected2, s.substr(6));
        EXPECT_EQ(expected3, s.substr(6, 4));
        EXPECT_EQ(expected4, s.substr(30));
        EXPECT_EQ(expected5, s.substr(6, 30));
    }

    TEST_FIXTURE(StringTest, Equals)
    {
        String s1{"abcdefg"};
        String s2{"abcdefG"};
        String s3{"abcdefg"};
        String s4{""};
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
        String s1{"abcdefg"};
        String s2{"abcdefG"};
        String s3{"abcdefg"};
        String s4{""};
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
        String s1{"abcdefg"};
        String s2{"abcdefG"};
        String s3{"abcdefg"};
        String s4{"bcdefg"};
        String s5{""};
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
        String s1{"abcde"};
        String s2{"fghijk"};

        auto s3 = s1.replace(0, 1, s2);
        EXPECT_EQ("fghijkbcde", s1);
        EXPECT_EQ("fghijkbcde", s3);
    }

    TEST_FIXTURE(StringTest, ReplacePosCountStringPos)
    {
        String s1{"abcde"};
        String s2{"fghijk"};

        auto s3 = s1.replace(1, 2, s2, 2);
        EXPECT_EQ("ahijkde", s1);
        EXPECT_EQ("ahijkde", s3);
    }

    TEST_FIXTURE(StringTest, ReplacePosCountStringPosCount)
    {
        String s1{"abcde"};
        String s2{"fghijk"};

        auto s3 = s1.replace(1, 2, s2, 2, 2);
        EXPECT_EQ("ahide", s1);
        EXPECT_EQ("ahide", s3);
    }

    TEST_FIXTURE(StringTest, ReplacePosCountConstCharPtr)
    {
        String s1{"abcde"};

        auto s2 = s1.replace(0, 1, "uvwxyz");
        EXPECT_EQ("uvwxyzbcde", s1);
        EXPECT_EQ("uvwxyzbcde", s2);
    }

    TEST_FIXTURE(StringTest, ReplacePosCountConstCharPtrCount)
    {
        String s1{"abcde"};

        auto s2 = s1.replace(1, 2, "uvwxyz", 2);
        EXPECT_EQ("auvde", s1);
        EXPECT_EQ("auvde", s2);
    }

    TEST_FIXTURE(StringTest, ReplacePosChar)
    {
        String s1{"abcde"};

        auto s2 = s1.replace(0, 1, 'x');
        EXPECT_EQ("xbcde", s1);
        EXPECT_EQ("xbcde", s2);
    }

    TEST_FIXTURE(StringTest, ReplacePosCharCount)
    {
        String s1{"abcde"};

        auto s2 = s1.replace(1, 2, 'x', 3);
        EXPECT_EQ("axxxde", s1);
        EXPECT_EQ("axxxde", s2);
    }

    TEST_FIXTURE(StringTest, ReplaceSubstringString)
    {
        String s1{"abcde"};
        String s2{"cd"};
        String s3{"xy"};

        EXPECT_EQ(1, s1.replace(s2, s3));
        EXPECT_EQ("abxye", s1);
        EXPECT_EQ("cd", s2);
        EXPECT_EQ("xy", s3);
    }

    TEST_FIXTURE(StringTest, ReplaceSubstringStringMultiple)
    {
        String s1{"abababab"};
        String s2{"ab"};
        String s3{"cd"};

        EXPECT_EQ(4, s1.replace(s2, s3));
        EXPECT_EQ("cdcdcdcd", s1);
        EXPECT_EQ("ab", s2);
        EXPECT_EQ("cd", s3);
    }

    TEST_FIXTURE(StringTest, ReplaceSubstringConstCharPtr)
    {
        String s1{"abcde"};

        EXPECT_EQ(1, s1.replace("cd", "xy"));
        EXPECT_EQ("abxye", s1);
    }

    TEST_FIXTURE(StringTest, ReplaceSubstringConstCharPtrMultiple)
    {
        String s1{"abababab"};

        EXPECT_EQ(4, s1.replace("ab", "cd"));
        EXPECT_EQ("cdcdcdcd", s1);
    }

    TEST_FIXTURE(StringTest, Align)
    {
        String s = "abcd";
        String expected1 = "    abcd";
        String expected2 = "abcd    ";
        String expected3 = "abcd";

        EXPECT_EQ(expected1, s.align(8));
        EXPECT_EQ(expected2, s.align(-8));
        EXPECT_EQ(expected3, s.align(0));
    }

    TEST_FIXTURE(StringTest, EqualityOperator)
    {
        String s1{"abcdefg"};
        String s2{"abcdefG"};
        String s3{"abcdefg"};
        String s4{"bcdefg"};
        String s5{""};
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
        String s1{"abcdefg"};
        String s2{"abcdefG"};
        String s3{"abcdefg"};
        String s4{"bcdefg"};
        String s5{""};
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
        String a = "ABC";
        String b = "def";
        const char* expected = "ABCdef";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorStringAndConstCharPtr)
    {
        String a = "ABC";
        const char* b = "def";
        const char* expected = "ABCdef";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorStringAndConstCharPtrEmpty)
    {
        String a = "ABC";
        const char* b = "";
        const char* expected = "ABC";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorStringAndNullPtr)
    {
        String a = "ABC";
        const char* b = nullptr;
        const char* expected = "ABC";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorConstCharPtrAndString)
    {
        const char* a = "ABC";
        String b = "def";
        const char* expected = "ABCdef";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorConstCharPtrEmptyAndString)
    {
        const char* a = "";
        String b = "def";
        const char* expected = "def";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorNullPtrAndString)
    {
        const char* a = nullptr;
        String b = "def";
        const char* expected = "def";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorStringAndChar)
    {
        String a = "ABC";
        char b = 'd';
        const char* expected = "ABCd";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

    TEST_FIXTURE(StringTest, AddOperatorCharAndString)
    {
        char a = 'd';
        String b = "ABC";
        const char* expected = "dABC";
        size_t expectedLength = strlen(expected);
        String s;

        s = a + b;

        EXPECT_EQ(expectedLength, s.length());
        EXPECT_EQ(expected, s);
    }

} // suite Baremetal

} // namespace test
} // namespace baremetal