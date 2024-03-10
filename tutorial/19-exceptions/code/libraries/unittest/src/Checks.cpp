//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Checks.cpp
//
// Namespace   : unittest
//
// Class       : Checks
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

#include "unittest/Checks.h"

#include <baremetal/Format.h>
#include <baremetal/Util.h>

using namespace baremetal;

namespace unittest {

static bool EqualCaseInsensitive(const string& a, const string& b)
{
    if (a.length() != b.length())
        return false;
    return strcasecmp(a.data(), b.data()) == 0;
}

AssertionResult AssertionSuccess()
{
    return AssertionResult(false, string());
}

AssertionResult BooleanFailure(const string& valueExpression, const string& expectedValue, const string& actualValue)
{
    string result = Format("Value of: %s", valueExpression.c_str());
    if (actualValue != valueExpression)
    {
        result.append(Format("\n  Actual: %s", actualValue.c_str()));
    }

    result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));

    return AssertionResult(true, result);
}

AssertionResult EqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
{
    string result = Format("Value of: %s", actualExpression.c_str());
    if (actualValue != actualExpression)
    {
        result.append(Format("\n  Actual: %s", actualValue.c_str()));
    }

    result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
    if (expectedValue != expectedExpression)
    {
        result.append(Format("\n  Which is: %s", expectedValue.c_str()));
    }
    result.append("\n");

    return AssertionResult(true, result);
}

AssertionResult InEqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
{
    string result = Format("Value of: %s", actualExpression.c_str());
    if (actualValue != actualExpression)
    {
        result.append(Format("\n  Actual: %s", actualValue.c_str()));
    }

    result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
    if (expectedValue != expectedExpression)
    {
        result.append(Format("\n  Which is: %s", expectedValue.c_str()));
    }
    result.append("\n");

    return AssertionResult(true, result);
}

AssertionResult CloseFailure(const string& expectedExpression,
                             const string& actualExpression,
                             const string& toleranceExpression,
                             const string& expectedValue,
                             const string& actualValue,
                             const string& toleranceValue)
{
    string result = Format("Value of: %s", actualExpression.c_str());
    if (actualValue != actualExpression)
    {
        result.append(Format("\n  Actual: %s", actualValue.c_str()));
    }

    result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
    if (expectedValue != expectedExpression)
    {
        result.append(Format("\n  Which is: %s", expectedValue.c_str()));
    }
    result.append(Format("\n  Tolerance: %s", toleranceExpression.c_str()));
    if (toleranceValue != toleranceExpression)
    {
        result.append(Format("\n  (+/-) %s", toleranceValue.c_str()));
    }

    return AssertionResult(true, result);
}

namespace internal {

AssertionResult CheckStringsEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    if (expected == actual)
        return AssertionSuccess();

    if (strcmp(expected, actual))
    {
        return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
    }
    return AssertionSuccess();
}

AssertionResult CheckStringsNotEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    if (expected == actual)
        return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));

    if (!strcmp(expected, actual))
    {
        return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
    }
    return AssertionSuccess();
}

AssertionResult CheckStringsEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    if (expected == actual)
        return AssertionSuccess();

    if (!EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
    {
        return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
    }
    return AssertionSuccess();
}

AssertionResult CheckStringsNotEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    if (expected == actual)
        return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));

    if (EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
    {
        return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
    }
    return AssertionSuccess();
}

} // namespace internal

AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
                                   char *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
                                   char const *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected,
                                   char *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
                                      char *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
                                      char const *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected,
                                      char *actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const baremetal::string& expected,
                                   const baremetal::string& actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const baremetal::string& expected,
                                   const char* actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
                                   const baremetal::string& actualExpression,
                                   const char* expected,
                                   const baremetal::string& actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const baremetal::string& expected,
                                      const baremetal::string& actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const baremetal::string& expected,
                                      const char* actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
                                      const baremetal::string& actualExpression,
                                      const char* expected,
                                      const baremetal::string& actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char const* expected, char const* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char* expected, char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char* expected, char const* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                             char const* expected, char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char const* expected, char const* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char* expected, char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char* expected, char const* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                char const* expected, char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const baremetal::string& expected,
                                             const baremetal::string& actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const baremetal::string& expected,
                                             const char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                             const baremetal::string& actualExpression,
                                             const char* expected,
                                             const baremetal::string& actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const baremetal::string& expected,
                                                const baremetal::string& actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const baremetal::string& expected,
                                                const char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
                                                const baremetal::string& actualExpression,
                                                const char* expected,
                                                const baremetal::string& actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

} // namespace unittest
