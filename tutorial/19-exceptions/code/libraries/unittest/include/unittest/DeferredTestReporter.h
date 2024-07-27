//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : DeferredTestReporter.h
//
// Namespace   : unittest
//
// Class       : DeferredTestReporter
//
// Description : Deferred test reporter, which saves test results
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

#include <unittest/ITestReporter.h>
#include <unittest/TestResult.h>

/// @file
/// Deferred test reporter
///
/// Saves failures during the test run, so they can be sown in the overview after the complete test run

namespace unittest
{

/// <summary>
/// Test result entry
/// </summary>
class ResultEntry
{
private:
    friend class ResultList;
    /// @brief Test result
    TestResult m_result;
    /// @brief Pointer to next entry in list
    ResultEntry* m_next;

public:
    explicit ResultEntry(const TestResult& result);
    /// <summary>
    /// Return test result
    /// </summary>
    /// <returns>Test result</returns>
    TestResult& GetResult() { return m_result; }
    /// <summary>
    /// Return next entry pointer
    /// </summary>
    /// <returns>Next entry pointer</returns>
    ResultEntry* GetNext() { return m_next; }
};

/// <summary>
/// Test result entry list
/// </summary>
class ResultList
{
private:
    /// @brief Start of list
    ResultEntry* m_head;
    /// @brief End of list
    ResultEntry* m_tail;

public:
    ResultList();
    ~ResultList();

    void Add(const TestResult& result);
    /// <summary>
    /// Return start of list pointer
    /// </summary>
    /// <returns>Start of list pointer</returns>
    ResultEntry* GetHead() const { return m_head; }
    /// <summary>
    /// Return end of list pointer
    /// </summary>
    /// <returns>End of list pointer</returns>
    ResultEntry* GetTail() const { return m_tail; }
};

/// <summary>
/// Deferred test reporter
///
/// Implements abstract ITestReporter interface
/// </summary>
class DeferredTestReporter : public ITestReporter
{
private:
    /// @brief Test result list for current test run
    ResultList m_results;

public:
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

    ResultList& Results();
};

} // namespace unittest
