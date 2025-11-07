//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : DeferredTestReporter.cpp
//
// Namespace   : unittest
//
// Class       : DeferredTestReporter
//
// Description : Test detail
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

#include "unittest/DeferredTestReporter.h"

#include "unittest/TestDetails.h"

/// @file
/// Deferred test reporter implementation

using namespace baremetal;

namespace unittest {

/// <summary>
/// Constructor
/// </summary>
/// <param name="result">Test result to be stored</param>
ResultEntry::ResultEntry(const TestResult& result)
    : m_result{result}
    , m_next{}
{
}

/// <summary>
/// Constructor
/// </summary>
ResultList::ResultList()
    : m_head{}
    , m_tail{}
{
}

/// <summary>
/// Destructor
/// </summary>
ResultList::~ResultList()
{
    auto current = m_head;
    while (current != nullptr)
    {
        auto next = current->m_next;
        delete current;
        current = next;
    }
}

/// <summary>
/// Add a test result to the list
/// </summary>
/// <param name="result">Test result to add</param>
void ResultList::Add(const TestResult& result)
{
    auto entry = new ResultEntry(result);
    if (m_head == nullptr)
    {
        m_head = entry;
    }
    else
    {
        auto current = m_head;
        while (current->m_next != nullptr)
            current = current->m_next;
        current->m_next = entry;
    }
    m_tail = entry;
}

/// <summary>
/// Start of test run callback (empty)
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites to be run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
/// <param name="numberOfTests">Number of tests to be run</param>
void DeferredTestReporter::ReportTestRunStart(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
{
}

/// <summary>
/// Finish of test run callback (empty)
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures run</param>
/// <param name="numberOfTests">Number of tests run</param>
void DeferredTestReporter::ReportTestRunFinish(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
{
}

/// <summary>
/// Test summary callback (empty)
/// </summary>
/// <param name="results">Test run results</param>
void DeferredTestReporter::ReportTestRunSummary(const TestResults& /*results*/)
{
}

/// <summary>
/// Test run overview callback (empty)
/// </summary>
/// <param name="results">Test run results</param>
void DeferredTestReporter::ReportTestRunOverview(const TestResults& /*results*/)
{
}

/// <summary>
/// Test suite start callback (empty)
/// </summary>
/// <param name="suiteName">Test suite name</param>
/// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
void DeferredTestReporter::ReportTestSuiteStart(const String& /*suiteName*/, int /*numberOfTestFixtures*/)
{
}

/// <summary>
/// Test suite finish callback (empty)
/// </summary>
/// <param name="suiteName">Test suite name</param>
/// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
void DeferredTestReporter::ReportTestSuiteFinish(const String& /*suiteName*/, int /*numberOfTestFixtures*/)
{
}

/// <summary>
/// Test fixture start callback (empty)
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
void DeferredTestReporter::ReportTestFixtureStart(const String& /*fixtureName*/, int /*numberOfTests*/)
{
}

/// <summary>
/// Test fixture finish callback (empty)
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
void DeferredTestReporter::ReportTestFixtureFinish(const String& /*fixtureName*/, int /*numberOfTests*/)
{
}

/// <summary>
/// Test start callback
/// </summary>
/// <param name="details">Test details</param>
void DeferredTestReporter::ReportTestStart(const TestDetails& details)
{
    m_results.Add(TestResult(details));
}

/// <summary>
/// Test finish callback (empty)
/// </summary>
/// <param name="details">Test details</param>
/// <param name="success">Test result, true is successful, false is failed</param>
void DeferredTestReporter::ReportTestFinish(const TestDetails& /*details*/, bool /*success*/)
{
}

/// <summary>
/// Test failure callback
/// </summary>
/// <param name="details">Test details</param>
/// <param name="failure">Test failure message</param>
void DeferredTestReporter::ReportTestFailure(const TestDetails& details, const String& failure)
{
    TestResult& result = m_results.GetTail()->GetResult();
    result.AddFailure(Failure(details.SourceFileLineNumber(), failure));
}

/// <summary>
/// Return test result list
/// </summary>
/// <returns>Test result list</returns>
ResultList& DeferredTestReporter::Results()
{
    return m_results;
}

} // namespace unittest
