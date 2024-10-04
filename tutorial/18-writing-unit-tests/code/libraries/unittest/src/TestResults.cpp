//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResults.cpp
//
// Namespace   : unittest
//
// Class       : TestResults
//
// Description : Test run results
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

#include <unittest/TestResults.h>

#include <unittest/ITestReporter.h>
#include <unittest/TestDetails.h>
#include <unittest/TestFixtureInfo.h>
#include <unittest/TestRegistry.h>
#include <unittest/TestSuiteInfo.h>

/// @file
/// Test results implementation

using namespace baremetal;

namespace unittest {

/// <summary>
/// Constructor
/// </summary>
/// <param name="testReporter">Test reporter to use</param>
TestResults::TestResults(ITestReporter* testReporter)
    : m_reporter{ testReporter }
    , m_totalTestCount{}
    , m_failedTestCount{}
    , m_failureCount{}
    , m_currentTestFailed{}
{
}

/// <summary>
/// Destructor
/// </summary>
TestResults::~TestResults()
{
}

/// <summary>
/// Start a test suite run
/// </summary>
/// <param name="suite">Test suite to start</param>
void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
{
    if (m_reporter)
        m_reporter->ReportTestSuiteStart(suite->Name(), suite->CountFixtures());
}

/// <summary>
/// Finish a test suite run
/// </summary>
/// <param name="suite">Test suite to finish</param>
void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
{
    if (m_reporter)
        m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures());
}

/// <summary>
/// Start a test fixture run
/// </summary>
/// <param name="fixture">Test fixture to start</param>
void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
{
    if (m_reporter)
        m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
}

/// <summary>
/// Finish a test fixture run
/// </summary>
/// <param name="fixture">Test fixture to finish</param>
void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
{
    if (m_reporter)
        m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests());
}

/// <summary>
/// Start a test
/// </summary>
/// <param name="details">Test details of test to start</param>
void TestResults::OnTestStart(const TestDetails& details)
{
    ++m_totalTestCount;
    m_currentTestFailed = false;
    if (m_reporter)
        m_reporter->ReportTestStart(details);
}

/// <summary>
/// Add a test failure
/// </summary>
/// <param name="details"></param>
/// <param name="message">Test failure string</param>
void TestResults::OnTestFailure(const TestDetails& details, const string& message)
{
    ++m_failureCount;
    if (!m_currentTestFailed)
    {
        ++m_failedTestCount;
        m_currentTestFailed = true;
    }
    if (m_reporter)
        m_reporter->ReportTestFailure(details, message);
}

/// <summary>
/// Finish a test
/// </summary>
/// <param name="details">Test details of test to finish</param>
void TestResults::OnTestFinish(const TestDetails& details)
{
    if (m_reporter)
        m_reporter->ReportTestFinish(details, !m_currentTestFailed);
}

} // namespace unittest
