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

#include <unittest/ITestReporter.h>
#include <unittest/TestResult.h>

namespace unittest
{

struct ResultEntry
{
    TestResult m_result;
    ResultEntry* m_next;
    explicit ResultEntry(const TestResult& result);
};

class ResultList
{
public:
    ResultEntry* m_head;
    ResultEntry* m_tail;

    ResultList();
    ~ResultList();

    void Add(const TestResult& result);
};

class DeferredTestReporter : public ITestReporter
{
private:
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
