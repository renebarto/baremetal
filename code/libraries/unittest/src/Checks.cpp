//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Checks.cpp
//
// Namespace   : unittest
//
// Class       : -
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

#include "baremetal/Format.h"
#include "stdlib/Util.h"

/// @file
/// Assertion checks implementation

using namespace baremetal;
using namespace unittest;

/// <summary>
/// Compare two strings ignoring case
/// </summary>
/// <param name="a">Left hand side of comparison</param>
/// <param name="b">Right hand side of comparison</param>
/// <returns>True if the strings are equal ignoring case, false otherwise</returns>
static bool EqualCaseInsensitive(const String& a, const String& b)
{
    if (a.length() != b.length())
        return false;
    return strcasecmp(a.data(), b.data()) == 0;
}

/// <summary>
/// Create a success object
/// </summary>
/// <returns>Result object</returns>
AssertionResult unittest::AssertionSuccess()
{
    return AssertionResult(false, String());
}

/// <summary>
/// Create a generic failure object with the provided message
/// </summary>
/// <param name="message">Message to be included</param>
/// <returns>Result object</returns>
AssertionResult unittest::GenericFailure(const baremetal::String& message)
{
    return AssertionResult(true, message);
}

/// <summary>
/// Create a boolean failure object
/// </summary>
/// <param name="valueExpression">String representation of the actual value</param>
/// <param name="expectedValue">Expected value</param>
/// <param name="actualValue">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::BooleanFailure(const baremetal::String& valueExpression, const baremetal::String& expectedValue,
                                         const baremetal::String& actualValue)
{
    String result = Format("Value of: %s", valueExpression.c_str());
    if (actualValue != valueExpression)
    {
        result.append(Format("\n  Actual: %s", actualValue.c_str()));
    }

    result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));

    return AssertionResult(true, result);
}

/// <summary>
/// Create a equality comparison failure object
/// </summary>
/// <param name="expectedExpression">String representation of the expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="expectedValue">Expected value</param>
/// <param name="actualValue">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::EqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
                                    const baremetal::String& expectedValue, const baremetal::String& actualValue)
{
    String result = Format("Value of: %s", actualExpression.c_str());
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

/// <summary>
/// Create a inequality comparison failure object
/// </summary>
/// <param name="expectedExpression">String representation of the not expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="expectedValue">Expected value</param>
/// <param name="actualValue">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::InEqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
                                      const baremetal::String& expectedValue, const baremetal::String& actualValue)
{
    String result = Format("Value of: %s", actualExpression.c_str());
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

/// <summary>
/// Create a comparison with tolerance failure object
/// </summary>
/// <param name="expectedExpression">String representation of the expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="toleranceExpression">String representation of the tolerance value</param>
/// <param name="expectedValue">Expected value</param>
/// <param name="actualValue">Actual value</param>
/// <param name="toleranceValue">Tolerance value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CloseFailure(const String& expectedExpression, const String& actualExpression, const String& toleranceExpression,
                                       const String& expectedValue, const String& actualValue, const String& toleranceValue)
{
    String result = Format("Value of: %s", actualExpression.c_str());
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

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult CheckStringsEqual(const String& expectedExpression, const String& actualExpression, char const* expected, char const* actual)
{
    if (expected == actual)
        return AssertionSuccess();

    if (strcmp(expected, actual))
    {
        return EqFailure(expectedExpression, actualExpression, String(expected), String(actual));
    }
    return AssertionSuccess();
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult CheckStringsNotEqual(const String& expectedExpression, const String& actualExpression, char const* expected, char const* actual)
{
    if (expected == actual)
        return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));

    if (!strcmp(expected, actual))
    {
        return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
    }
    return AssertionSuccess();
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult CheckStringsEqualIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                            char const* actual)
{
    if (expected == actual)
        return AssertionSuccess();

    if (!EqualCaseInsensitive(String(expected), String(actual)))
    {
        return EqFailure(expectedExpression, actualExpression, String(expected), String(actual));
    }
    return AssertionSuccess();
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult CheckStringsNotEqualIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                               char const* actual)
{
    if (expected == actual)
        return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));

    if (EqualCaseInsensitive(String(expected), String(actual)))
    {
        return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
    }
    return AssertionSuccess();
}

} // namespace internal

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
                                             char const* actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
                                             char* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
                                             char const* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
                                             char* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                char const* actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
                                                char* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
                                                char const* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                char* actual) // cppcheck-suppress constParameterPointer
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
                                             const String& actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
                                             const char* actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const char* expected,
                                             const String& actual)
{
    return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                const String& actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                const char* actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const char* expected,
                                                const String& actual)
{
    return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                       char const* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected, char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
                                                       char const* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                       char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                          char const* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
                                                          char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
                                                          char const* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
                                                          char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                       const String& actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                       const char* actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const char* expected,
                                                       const String& actual)
{
    return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                          const String& actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
                                                          const char* actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}

/// <summary>
/// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
/// </summary>
/// <param name="expectedExpression">String representation of expected value</param>
/// <param name="actualExpression">String representation of actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const char* expected,
                                                          const String& actual)
{
    return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
}
