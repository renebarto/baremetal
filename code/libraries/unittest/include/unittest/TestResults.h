//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResults.h
//
// Namespace   : unittest
//
// Class       : TestResults
//
// Description : Test results
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

#pragma once

#include <baremetal/String.h>

/// @file
/// Test results
///
/// Results for a complete test run

namespace unittest
{

class ITestReporter;
class TestDetails;
class TestSuiteInfo;
class TestFixtureInfo;

/// <summary>
/// Test results
///
/// Holds the test results for a complete test run
/// </summary>
class TestResults
{
private:
    /// @brief Selected test reporter
    ITestReporter* m_reporter;
    /// @brief Total count fo tests in test run
    int m_totalTestCount;
    /// @brief Total count of failed tests in test run
    int m_failedTestCount;
    /// @brief Total count of failures in test run
    int m_failureCount;
    /// @brief Flag for failure in current test, set to true if at least one failure occurred in the current test
    bool m_currentTestFailed;

public:
    explicit TestResults(ITestReporter* reporter = nullptr);
    TestResults(const TestResults&) = delete;
    TestResults(TestResults&&) = delete;
    virtual ~TestResults();

    TestResults& operator = (const TestResults&) = delete;
    TestResults& operator = (TestResults&&) = delete;

    void OnTestSuiteStart(TestSuiteInfo* suite);
    void OnTestSuiteFinish(TestSuiteInfo* suite);
    void OnTestFixtureStart(TestFixtureInfo* fixture);
    void OnTestFixtureFinish(TestFixtureInfo* fixture);
    void OnTestStart(const TestDetails& details);
    void OnTestFailure(const TestDetails& details, const baremetal::string& message);
    void OnTestFinish(const TestDetails& details);

    /// <summary>
    /// Returns the number of tests in the test run
    /// </summary>
    /// <returns>Number of tests in the test run</returns>
    int GetTotalTestCount() const { return m_totalTestCount; }
    /// <summary>
    /// Returns the number of failed tests in the test run
    /// </summary>
    /// <returns>Number of failed tests in the test run</returns>
    int GetFailedTestCount() const { return m_failedTestCount; }
    /// <summary>
    /// Returns the number of failures in the test run
    /// </summary>
    /// <returns>Number of failures in the test run</returns>
    int GetFailureCount() const { return m_failureCount; }

};

} // namespace unittest
