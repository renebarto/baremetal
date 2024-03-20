//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Checks.h
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

#pragma once

#include <baremetal/String.h>

#include <unittest/PrintValue.h>

/// @file
/// Assertion checks

namespace unittest
{

class TestResults;
class TestDetails;

/// <summary>
/// Assertion result
///
/// Contains the result of an assertion, flags whether it failed, and what the message was
/// </summary>
struct AssertionResult
{
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="failed">If true, the assertion failed, if false the assertion was successful</param>
    /// <param name="message">Message for the assertion</param>
    AssertionResult(bool failed, const baremetal::string& message)
        : failed(failed)
        , message(message)
    {
    }
    /// @brief If true, the assertion failed, if false the assertion was successful
    const bool failed;
    /// @brief Message for the assertion
    const baremetal::string message;
    /// @brief bool case operator.
    /// @return true, if the assertion failed, false if the assertion was successful
    operator bool() const { return failed; }
};

extern AssertionResult AssertionSuccess();
extern AssertionResult GenericFailure(const baremetal::string& message);
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

/// <summary>
/// Check whether argument is evaluated as true
/// </summary>
/// <typeparam name="Value">Type of argument</typeparam>
/// <param name="value">Argument</param>
/// <returns>Returns true if the argument can be evaluated as true, false otherwise</returns>
template<typename Value>
bool CheckTrue(const Value value)
{
    return !!value;
}

/// <summary>
/// Check whether argument is evaluated as false
/// </summary>
/// <typeparam name="Value">Type of argument</typeparam>
/// <param name="value">Argument</param>
/// <returns>Returns true if the argument can be evaluated as false, false otherwise</returns>
template<typename Value>
bool CheckFalse(const Value value)
{
    return !value;
}

/// <summary>
/// Utility class to convert a value to a string for comparison
/// </summary>
/// <typeparam name="ToPrint">Type of value to be converted to string</typeparam>
template <typename ToPrint>
class FormatForComparison
{
public:
    /// <summary>
    /// Convert value to a string
    /// </summary>
    /// <param name="value">Value to be converted to string</param>
    /// <returns>Resulting string</returns>
    static baremetal::string Format(const ToPrint& value)
    {
        return PrintToString(value);
    }
};

/// <summary>
/// Format a value for a failure message
/// </summary>
/// <typeparam name="T1">Type of value to be formatted</typeparam>
/// <param name="value">Value to be formatted</param>
/// <returns>Resulting string</returns>
template <typename T1>
baremetal::string FormatForComparisonFailureMessage(const T1& value)
{
    return FormatForComparison<T1>::Format(value);
}

/// <summary>
/// Evaluate whether a value can be evaluated as true, generate a success object if successful, otherwise a failure object
/// </summary>
/// <typeparam name="Value">Type of value to be evaluated</typeparam>
/// <param name="valueName">String representation of value for failure message</param>
/// <param name="value">Value to be evaluated</param>
/// <returns>Result object</returns>
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

/// <summary>
/// Evaluate whether a value can be evaluated as false, generate a success object if successful, otherwise a failure object
/// </summary>
/// <typeparam name="Value">Type of value to be evaluated</typeparam>
/// <param name="valueName">String representation of value for failure message</param>
/// <param name="value">Value to be evaluated</param>
/// <returns>Result object</returns>
template<typename Value>
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

/// <summary>
/// Compare an expected and actual value
/// </summary>
/// <typeparam name="Expected">Type of expected value</typeparam>
/// <typeparam name="Actual">Type of actual value</typeparam>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>True if the values are considered equal, false otherwise</returns>
template<typename Expected, typename Actual>
bool AreEqual(const Expected& expected, const Actual& actual)
{
    return (expected == actual);
}

/// <summary>
/// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
/// </summary>
/// <typeparam name="Expected">Type of the expected value</typeparam>
/// <typeparam name="Actual">Type of the actual value</typeparam>
/// <param name="expectedExpression">String representation of the expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
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

/// <summary>
/// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
/// </summary>
/// <typeparam name="Expected">Type of the expected value</typeparam>
/// <typeparam name="Actual">Type of the actual value</typeparam>
/// <param name="expectedExpression">String representation of the not expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <returns>Result object</returns>
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


/// <summary>
/// Helper class for {ASSERT|EXPECT}_EQ/NE
///
/// Forms generalized mechanism for calling polymorphic check functions.
/// The template argument lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
/// is a null pointer literal.  The following default implementation is
/// for lhs_is_null_literal being false.
/// </summary>
template <bool lhs_is_null_literal>
class EqHelper
{
public:
    /// <summary>
    /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
    /// </summary>
    /// <typeparam name="Expected">Type of the expected value</typeparam>
    /// <typeparam name="Actual">Type of the actual value</typeparam>
    /// <param name="expectedExpression">String representation of the expected value</param>
    /// <param name="actualExpression">String representation of the actual value</param>
    /// <param name="expected">Expected value</param>
    /// <param name="actual">Actual value</param>
    /// <returns>Result object</returns>
    template <typename Expected, typename Actual>
    static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                      const Expected& expected, const Actual& actual)
    {
        return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
    }

    /// <summary>
    /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
    /// </summary>
    /// <typeparam name="Expected">Type of the expected value</typeparam>
    /// <typeparam name="Actual">Type of the actual value</typeparam>
    /// <param name="expectedExpression">String representation of the not expected value</param>
    /// <param name="actualExpression">String representation of the actual value</param>
    /// <param name="expected">Expected value</param>
    /// <param name="actual">Actual value</param>
    /// <returns>Result object</returns>
    template <typename Expected, typename Actual>
    static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                         const Expected& expected, const Actual& actual)
    {
        return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
    }
};

/// <summary>
/// Helper class for {ASSERT|EXPECT}_EQ/NE_IGNORE_CASE
///
/// Forms generalized mechanism for calling polymorphic check functions for string comparisons ignoring case.
/// </summary>
class EqHelperStringCaseInsensitive
{
public:
    /// <summary>
    /// Evaluate whether an expected value is equal to an actual value, ignoring case, generate a success object if successful, otherwise a failure object
    /// </summary>
    /// <typeparam name="Expected">Type of the expected value</typeparam>
    /// <typeparam name="Actual">Type of the actual value</typeparam>
    /// <param name="expectedExpression">String representation of the expected value</param>
    /// <param name="actualExpression">String representation of the actual value</param>
    /// <param name="expected">Expected value</param>
    /// <param name="actual">Actual value</param>
    /// <returns>Result object</returns>
    template <typename Expected, typename Actual>
    static AssertionResult CheckEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                const Expected& expected, const Actual& actual)
    {
        return CheckEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
    }

    /// <summary>
    /// Evaluate whether an expected value is not equal to an actual value, ignoring case, generate a success object if successful, otherwise a failure object
    /// </summary>
    /// <typeparam name="Expected">Type of the expected value</typeparam>
    /// <typeparam name="Actual">Type of the actual value</typeparam>
    /// <param name="expectedExpression">String representation of the not expected value</param>
    /// <param name="actualExpression">String representation of the actual value</param>
    /// <param name="expected">Expected value</param>
    /// <param name="actual">Actual value</param>
    /// <returns>Result object</returns>
    template <typename Expected, typename Actual>
    static AssertionResult CheckNotEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
                                                   const Expected& expected, const Actual& actual)
    {
        return CheckNotEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
    }
};

/// <summary>
/// Compare an expected and actual value, with tolerance
///
/// Compares two floating point values, and returns true if the absolute difference is within tolerance
/// </summary>
/// <typeparam name="Expected">Type of expected value</typeparam>
/// <typeparam name="Actual">Type of actual value</typeparam>
/// <typeparam name="Tolerance">Type of tolerance value</typeparam>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <param name="tolerance">Tolerance value</param>
/// <returns>True if the values are considered equal, false otherwise</returns>
template< typename Expected, typename Actual, typename Tolerance >
bool AreClose(const Expected& expected, const Actual& actual, Tolerance const& tolerance)
{
    return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
}

/// <summary>
/// Evaluate whether an expected value is equal to an actual value within tolerance, generate a success object if successful, otherwise a failure object
/// </summary>
/// <typeparam name="Expected">Type of the expected value</typeparam>
/// <typeparam name="Actual">Type of the actual value</typeparam>
/// <typeparam name="Tolerance">Type of the tolerance value</typeparam>
/// <param name="expectedExpression">String representation of the expected value</param>
/// <param name="actualExpression">String representation of the actual value</param>
/// <param name="toleranceExpression">String representation of the tolerance value</param>
/// <param name="expected">Expected value</param>
/// <param name="actual">Actual value</param>
/// <param name="tolerance">Tolerance value</param>
/// <returns>Result object</returns>
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
