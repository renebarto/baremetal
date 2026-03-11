//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : AssertMacros.h
//
// Namespace   : unittest
//
// Class       : Assertion macros
//
// Description : Test evaluation & assertion macros
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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

#pragma once

#include "unittest/Checks.h"
#include "unittest/CurrentTest.h"

/// @file
/// Assertion macros

#ifdef ASSERT_TRUE
#error unittest redefines ASSERT_TRUE
#endif

#ifdef ASSERT_FALSE
#error unittest redefines ASSERT_FALSE
#endif

#ifdef ASSERT_EQ
#error unittest redefines ASSERT_EQ
#endif

#ifdef ASSERT_NE
#error unittest redefines ASSERT_NE
#endif

#ifdef ASSERT_NEAR
#error unittest redefines ASSERT_NEAR
#endif

#ifdef EXPECT_TRUE
#error unittest redefines EXPECT_TRUE
#endif

#ifdef EXPECT_FALSE
#error unittest redefines EXPECT_FALSE
#endif

#ifdef EXPECT_EQ
#error unittest redefines EXPECT_EQ
#endif

#ifdef EXPECT_NE
#error unittest redefines EXPECT_NE
#endif

#ifdef EXPECT_NEAR
#error unittest redefines EXPECT_NEAR
#endif

namespace unittest {

/// @brief Internal namespace
namespace internal {

// Two overloaded helpers for checking at compile time whether an
// expression is a null pointer literal (i.e. nullptr or any 0-valued
// compile-time integral constant).  Their return values have
// different sizes, so we can use sizeof() to test which version is
// picked by the compiler.  These helpers have no implementations, as
// we only need their signatures.
//
// Given IsNullLiteralHelper(x), the compiler will pick the first
// version if x can be implicitly converted to Secret*, and pick the
// second version otherwise.  Since Secret is a secret and incomplete
// type, the only expression a user can write that has type Secret* is
// a null pointer literal.  Therefore, we know that x is a null
// pointer literal if and only if the first version is picked by the
// compiler.
class Secret;
/// <summary>
/// Conversion check function to check whether argument is a pointer
///
/// Not implemented, never called, only declared for return type size
/// </summary>
/// <param name="p">Argument</param>
/// <returns>Unused</returns>
char IsNullLiteralHelper(Secret* p);
/// <summary>
/// Conversion check function to check whether argument is not a pointer
///
/// Not implemented, never called, only declared for return type size
/// </summary>
/// <returns>Unused</returns>
char (&IsNullLiteralHelper(...))[2];

} // namespace internal

} // namespace unittest

/// @brief Boolean expression to check whether the argument is a null literal. Returns true if the argument is nullptr, false otherwise
#define IS_NULL_LITERAL(x) (sizeof(::unittest::internal::IsNullLiteralHelper(x)) == 1)
/// @brief Generic expect macro. Checks if the argument is true, if so generates a failure
#define UT_EXPECT_RESULT(value)                                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (const ::unittest::AssertionResult UT_AssertionResult = (value))                                                                          \
            ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__),                \
                                                              UT_AssertionResult.message);                                                           \
    } while (0)
/// @brief Generic assert macro. Checks if the argument is true, if so generates a failure
#define UT_ASSERT_RESULT(value)                                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (const ::unittest::AssertionResult UT_AssertionResult = (value))                                                                          \
        {                                                                                                                                            \
            ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__),                \
                                                              UT_AssertionResult.message);                                                           \
        }                                                                                                                                            \
    } while (0)

/// @brief Expect predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_EXPECT_RESULT if predicate function
/// returns false
#define EXPECT_PRED_FORMAT1(pred_format, v1)     UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), v1))
/// @brief Assert predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_ASSERT_RESULT if predicate function
/// returns false
#define ASSERT_PRED_FORMAT1(pred_format, v1)     UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), v1))

/// @brief Expect predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using
/// UT_EXPECT_RESULT if predicate function returns false
#define EXPECT_PRED_FORMAT2(pred_format, v1, v2) UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), v1, v2))
/// @brief Assert predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using
/// UT_ASSERT_RESULT if predicate function returns false
#define ASSERT_PRED_FORMAT2(pred_format, v1, v2) UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), v1, v2))

/// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_EXPECT_RESULT if predicate function returns
/// false
#define EXPECT_PRED_FORMAT3(pred_format, v1, v2, v3)                                                                                                 \
    UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), baremetal::String(#v3), v1, v2, v3))
/// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_ASSERT_RESULT if predicate function returns
/// false
#define ASSERT_PRED_FORMAT3(pred_format, v1, v2, v3)                                                                                                 \
    UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), baremetal::String(#v3), v1, v2, v3))

/// @brief Force failure with message
#define FAIL(message) UT_EXPECT_RESULT(GenericFailure(message))
/// @brief Assert that value is true
#define ASSERT_TRUE(value)                                                                                                                           \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT1(::unittest::CheckTrue, value);                                                                                           \
    } while (0)
/// @brief Expect that value is true
#define EXPECT_TRUE(value)                                                                                                                           \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT1(::unittest::CheckTrue, value);                                                                                           \
    } while (0)

/// @brief Assert that value is false
#define ASSERT_FALSE(value)                                                                                                                          \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT1(::unittest::CheckFalse, value);                                                                                          \
    } while (0)
/// @brief Expect that value is false
#define EXPECT_FALSE(value)                                                                                                                          \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT1(::unittest::CheckFalse, value);                                                                                          \
    } while (0)

/// @brief Assert that actual value is equal to expected value
#define ASSERT_EQ(expected, actual)                                                                                                                  \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual);                                          \
    } while (0)
/// @brief Expect that actual value is equal to expected value
#define EXPECT_EQ(expected, actual)                                                                                                                  \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual);                                          \
    } while (0)

/// @brief Assert that actual value is not equal to expected value
#define ASSERT_NE(expected, actual)                                                                                                                  \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual);                                       \
    } while (0)
/// @brief Expect that actual value is not equal to expected value
#define EXPECT_NE(expected, actual)                                                                                                                  \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual);                                       \
    } while (0)

/// @brief Assert that actual value is equal to expected value ignoring case
#define ASSERT_EQ_IGNORE_CASE(expected, actual)                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual);                                      \
    } while (0)
/// @brief Expect that actual value is equal to expected value ignoring case
#define EXPECT_EQ_IGNORE_CASE(expected, actual)                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual);                                      \
    } while (0)

/// @brief Assert that actual value is not equal to expected value ignoring case
#define ASSERT_NE_IGNORE_CASE(expected, actual)                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual);                                   \
    } while (0)
/// @brief Expect that actual value is not equal to expected value ignoring case
#define EXPECT_NE_IGNORE_CASE(expected, actual)                                                                                                      \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual);                                   \
    } while (0)

/// @brief Assert that actual value is equal to expected value within tolerance (for floating point comparison)
#define ASSERT_NEAR(expected, actual, tolerance)                                                                                                     \
    do                                                                                                                                               \
    {                                                                                                                                                \
        ASSERT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance);                                                                    \
    } while (0)
/// @brief Expect that actual value is equal to expected value within tolerance (for floating point comparison)
#define EXPECT_NEAR(expected, actual, tolerance)                                                                                                     \
    do                                                                                                                                               \
    {                                                                                                                                                \
        EXPECT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance);                                                                    \
    } while (0)

/// @brief Assert that value is nullptr
#define ASSERT_NULL(value)     ASSERT_EQ(nullptr, value)
/// @brief Expect that value is nullptr
#define EXPECT_NULL(value)     EXPECT_EQ(nullptr, value)
/// @brief Assert that value is not nullptr
#define ASSERT_NOT_NULL(value) ASSERT_NE(nullptr, value)
/// @brief Expect that value is not nullptr
#define EXPECT_NOT_NULL(value) EXPECT_NE(nullptr, value)
