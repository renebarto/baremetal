//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResults.cpp
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

#include <unittest/TestResults.h>

#include <unittest/ITestReporter.h>
#include <unittest/TestSuiteInfo.h>

using namespace baremetal;

namespace unittest {

TestResults::TestResults(ITestReporter* testReporter)
    : m_reporter{ testReporter }
    , m_totalTestCount{}
    , m_failedTestCount{}
    , m_failureCount{}
    , m_currentTestFailed{}
{
}

TestResults::~TestResults()
{
}

void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
{
    if (m_reporter)
        m_reporter->ReportTestSuiteStart(suite->Name(), suite->CountFixtures());
}

void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite, int milliSecondsElapsed)
{
    if (m_reporter)
        m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures(), milliSecondsElapsed);
}

void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
{
    if (m_reporter)
        m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
}

void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture, int milliSecondsElapsed)
{
    if (m_reporter)
        m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests(), milliSecondsElapsed);
}

void TestResults::OnTestStart(TestDetails const& details)
{
    ++m_totalTestCount;
    m_currentTestFailed = false;
    if (m_reporter)
        m_reporter->ReportTestStart(details);
}

void TestResults::OnTestFailure(TestDetails const& details, const string& result)
{
    ++m_failureCount;
    if (!m_currentTestFailed)
    {
        ++m_failedTestCount;
        m_currentTestFailed = true;
    }
    if (m_reporter)
        m_reporter->ReportTestFailure(details, result);
}

void TestResults::OnTestFinish(TestDetails const& details, int milliSecondsElapsed)
{
    if (m_reporter)
        m_reporter->ReportTestFinish(details, !m_currentTestFailed, milliSecondsElapsed);
}

int TestResults::GetTotalTestCount() const
{
    return m_totalTestCount;
}

int TestResults::GetFailedTestCount() const
{
    return m_failedTestCount;
}

int TestResults::GetFailureCount() const
{
    return m_failureCount;
}

} // namespace unittest
