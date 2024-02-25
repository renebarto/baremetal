//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

#include <unittest/DeferredTestReporter.h>

#include <unittest/TestDetails.h>

using namespace baremetal;

namespace unittest
{

ResultEntry::ResultEntry(const TestResult& result)
    : m_result{ result }
    , m_next{}
{
}

ResultList::ResultList()
    : m_head{}
    , m_tail{}
{
}

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

void DeferredTestReporter::ReportTestRunStart(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
{
}

void DeferredTestReporter::ReportTestRunFinish(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
{
}

void DeferredTestReporter::ReportTestRunSummary(const TestResults& /*results*/)
{
}

void DeferredTestReporter::ReportTestRunOverview(const TestResults& /*results*/)
{
}

void DeferredTestReporter::ReportTestSuiteStart(const string& /*suiteName*/, int /*numberOfTestFixtures*/)
{
}

void DeferredTestReporter::ReportTestSuiteFinish(const string& /*suiteName*/, int /*numberOfTests*/)
{
}

void DeferredTestReporter::ReportTestFixtureStart(const string& /*fixtureName*/, int /*numberOfTests*/)
{
}

void DeferredTestReporter::ReportTestFixtureFinish(const string& /*fixtureName*/, int /*numberOfTests*/)
{
}

void DeferredTestReporter::ReportTestStart(const TestDetails& details)
{
    m_results.Add(TestResult(details));
}

void DeferredTestReporter::ReportTestFinish(const TestDetails& /*details*/, bool /*success*/)
{
    TestResult& result = m_results.m_tail->m_result;
}

void DeferredTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
{
    TestResult& result = m_results.m_tail->m_result;
    result.AddFailure(Failure(details.SourceFileLineNumber(), failure));
}

ResultList& DeferredTestReporter::Results()
{
    return m_results;
}

} // namespace unittest
