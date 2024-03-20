//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Checks.h
//
// Namespace   : unittest
//
// Class       : AssertionResult
//
// Description : Check functions
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

#include <baremetal/String.h>

#include <unittest/PrintValue.h>

namespace unittest
{

class TestResults;
class TestDetails;

struct AssertionResult
{
    AssertionResult(bool failed, const baremetal::string& message)
        : failed(failed)
        , message(message)
    {
    }
    const bool failed;
    const baremetal::string message;
    operator bool() const { return failed; }
};

extern AssertionResult AssertionSuccess();
extern AssertionResult BooleanFailure(const baremetal::string& valueExpression,
                                      const baremetal::string& expectedValue,
                                      const baremetal::string& actualValue);
extern AssertionResult EqFailure(const baremetal::string& expectedExpression,
                                 const baremetal::string& actualExpression,
                                 const baremetal::string& expectedValue,
                                 const baremetal::string& actualValue);
extern AssertionResult InEqFailure(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const baremetal::string& expectedValue,
                                   const baremetal::string& actualValue);
extern AssertionResult CloseFailure(const baremetal::string& expectedExpression,
                                    const baremetal::string& actualExpression,
                                    const baremetal::string& toleranceExpression,
                                    const baremetal::string& expectedValue,
                                    const baremetal::string& actualValue,
                                    const baremetal::string& toleranceValue);

template<typename Value>
bool CheckTrue(const Value value)
{
    return !!value;
}

template<typename Value>
bool CheckFalse(const Value value)
{
    return !value;
}

template <typename ToPrint>
class FormatForComparison
{
public:
    static baremetal::string Format(const ToPrint& value)
    {
        return PrintToString(value);
    }
};

template <typename T1>
baremetal::string FormatForComparisonFailureMessage(const T1& value)
{
    return FormatForComparison<T1>::Format(value);
}

template<typename Value>
AssertionResult CheckTrue(const baremetal::string& valueName, const Value& value)
{
    if (!CheckTrue(value))
    {
        return BooleanFailure(valueName,
                              baremetal::string("true"),
                              PrintToString(value));
    }
    return AssertionSuccess();
}

template< typename Value >
AssertionResult CheckFalse(const baremetal::string& valueName, const Value& value)
{
    if (!CheckFalse(value))
    {
        return BooleanFailure(valueName,
                              baremetal::string("false"),
                              PrintToString(value));
    }
    return AssertionSuccess();
}

template<typename Expected, typename Actual>
bool AreEqual(const Expected& expected, const Actual& actual)
{
    return (expected == actual);
}

template< typename Expected, typename Actual >
AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                   const Expected& expected, const Actual& actual)
{
    if (!AreEqual(expected, actual))
    {
        return EqFailure(expectedExpression, actualExpression,
                         FormatForComparisonFailureMessage(expected),
                         FormatForComparisonFailureMessage(actual));
    }
    return AssertionSuccess();
}

template< typename Expected, typename Actual >
AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      const Expected& expected, const Actual& actual)
{
    if (AreEqual(expected, actual))
    {
        return InEqFailure(expectedExpression, actualExpression,
                           FormatForComparisonFailureMessage(expected),
                           FormatForComparisonFailureMessage(actual));
    }
    return AssertionSuccess();
}

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                   char const* expected, char const* actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                   char* expected, char* actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                   char* expected, char const* actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                   char const* expected, char* actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      char const* expected, char const* actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      char* expected, char* actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      char* expected, char const* actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      char const* expected, char* actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const baremetal::string& expected,
                                   const baremetal::string& actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const baremetal::string& expected,
                                   const char* actual);

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const char* expected,
                                   const baremetal::string& actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const baremetal::string& expected,
                                      const baremetal::string& actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const baremetal::string& expected,
                                      const char* actual);

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const char* expected,
                                      const baremetal::string& actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char const* expected, char const* actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char* expected, char* actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char* expected, char const* actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char const* expected, char* actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char const* expected, char const* actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char* expected, char* actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char* expected, char const* actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char const* expected, char* actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const baremetal::string& expected,
                                             const baremetal::string& actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const baremetal::string& expected,
                                             const char* actual);

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const char* expected,
                                             const baremetal::string& actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const baremetal::string& expected,
                                                const baremetal::string& actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const baremetal::string& expected,
                                                const char* actual);

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const char* expected,
                                                const baremetal::string& actual);

// The helper class for {ASSERT|EXPECT}_EQ.  The template argument
// lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
// is a null pointer literal.  The following default implementation is
// for lhs_is_null_literal being false.
template <bool lhs_is_null_literal>
class EqHelper
{
public:
    // This templatized version is for the general case.
    template <typename Expected, typename Actual>
    static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      const Expected& expected, const Actual& actual)
    {
        return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
    }

    template <typename Expected, typename Actual>
    static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                         const Expected& expected, const Actual& actual)
    {
        return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
    }
};

class EqHelperStringCaseInsensitive
{
public:
    template <typename Expected, typename Actual>
    static AssertionResult CheckEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                const Expected& expected, const Actual& actual)
    {
        return CheckEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
    }

    template <typename Expected, typename Actual>
    static AssertionResult CheckNotEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                   const Expected& expected, const Actual& actual)
    {
        return CheckNotEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
    }
};

template< typename Expected, typename Actual, typename Tolerance >
bool AreClose(const Expected& expected, const Actual& actual, Tolerance const& tolerance)
{
    return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
}

template< typename Expected, typename Actual, typename Tolerance >
AssertionResult CheckClose(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                           const baremetal::string& toleranceExpression,
                           const Expected& expected, const Actual& actual, Tolerance const& tolerance)
{
    if (!AreClose(expected, actual, tolerance))
    {
        return CloseFailure(expectedExpression, actualExpression, toleranceExpression,
                            FormatForComparisonFailureMessage(expected),
                            FormatForComparisonFailureMessage(actual),
                            FormatForComparisonFailureMessage(tolerance));
    }
    return AssertionSuccess();
}

} // namespace unittest
