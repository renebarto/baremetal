//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ConsoleTestReporter.h
//
// Namespace   : unittest
//
// Class       : ConsoleTestReporter
//
// Description : Console test reporter
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

#include <unittest/DeferredTestReporter.h>

/// @file
/// Console test reporter
///
/// Prints test information to the console, using color.
/// Uses DeferredTestReporter to save failures so they can be printed after running the test, as an overview

namespace unittest
{

/// <summary>
/// Console test reporter
/// </summary>
class ConsoleTestReporter
    : public DeferredTestReporter
{
public:
    /// @brief Separator for complete test run
    static const baremetal::string TestRunSeparator;
    /// @brief Separator for test fixture
    static const baremetal::string TestFixtureSeparator;
    /// @brief Separator for test suite
    static const baremetal::string TestSuiteSeparator;
    /// @brief Indicator for successful test
    static const baremetal::string TestSuccessSeparator;
    /// @brief Indicator for failed test
    static const baremetal::string TestFailSeparator;

    ConsoleTestReporter();

private:
    void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
    void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
    void ReportTestRunSummary(const TestResults& results) override;
    void ReportTestRunOverview(const TestResults& results) override;
    void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
    void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) override;
    void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
    void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) override;
    void ReportTestStart(const TestDetails& details) override;
    void ReportTestFinish(const TestDetails& details, bool success) override;
    void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) override;

    baremetal::string TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
    baremetal::string TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
    baremetal::string TestSuiteStartMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
    baremetal::string TestSuiteFinishMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
    baremetal::string TestFixtureStartMessage(const baremetal::string& fixtureName, int numberOfTests);
    baremetal::string TestFixtureFinishMessage(const baremetal::string& fixtureName, int numberOfTests);
    baremetal::string TestFailureMessage(const TestResult& result, const Failure& failure);
    baremetal::string TestFinishMessage(const TestDetails& details, bool success);
    baremetal::string TestRunSummaryMessage(const TestResults& results);
    baremetal::string TestRunOverviewMessage(const TestResults& results);
};

} // namespace unittest
