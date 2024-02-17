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

namespace unittest
{

class ITestReporter;
class TestDetails;
class TestSuiteInfo;
class TestFixtureInfo;
class TestFailure;
class TestResult;

class TestResults
{
public:
    explicit TestResults(ITestReporter* reporter = nullptr);
    TestResults(const TestResults&) = delete;
    TestResults(TestResults&&) = delete;
    virtual ~TestResults();

    TestResults& operator = (const TestResults&) = delete;
    TestResults& operator = (TestResults&&) = delete;

    void OnTestSuiteStart(TestSuiteInfo* suite);
    void OnTestSuiteFinish(TestSuiteInfo* suite, int milliSecondsElapsed);
    void OnTestFixtureStart(TestFixtureInfo* fixture);
    void OnTestFixtureFinish(TestFixtureInfo* fixture, int milliSecondsElapsed);
    void OnTestStart(TestDetails const&details);
    void OnTestFailure(TestDetails const&details, const baremetal::string& message);
    void OnTestFinish(TestDetails const&details, int milliSecondsElapsed);

    int GetTotalTestCount() const;
    int GetFailedTestCount() const;
    int GetFailureCount() const;

private:
    ITestReporter* m_reporter;
    int m_totalTestCount;
    int m_failedTestCount;
    int m_failureCount;
    bool m_currentTestFailed;
};

} // namespace unittest
