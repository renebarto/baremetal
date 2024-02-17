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

#include "unittest/DeferredTestReporter.h"

namespace unittest
{

class ConsoleTestReporter : public DeferredTestReporter
{
public:
    static const baremetal::string TestRunSeparator;
    static const baremetal::string TestFixtureSeparator;
    static const baremetal::string TestSuiteSeparator;
    static const baremetal::string TestSuccessSeparator;
    static const baremetal::string TestFailSeparator;

    ConsoleTestReporter();

private:
    void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
    void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests, int milliSecondsElapsed) override;
    void ReportTestRunSummary(const TestResults* results, int milliSecondsElapsed) override;
    void ReportTestRunOverview(const TestResults* results) override;
    void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
    void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures, int milliSecondsElapsed) override;
    void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
    void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests, int milliSecondsElapsed) override;
    void ReportTestStart(TestDetails const& test) override;
    void ReportTestFinish(TestDetails const& test, bool success, int milliSecondsElapsed) override;
    void ReportTestFailure(TestDetails const& test, const baremetal::string& failure) override;

    baremetal::string TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
    baremetal::string TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests, int milliSecondsElapsed);
    baremetal::string TestSuiteStartMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
    baremetal::string TestSuiteFinishMessage(const baremetal::string& suiteName, int numberOfTestFixtures, int milliSecondsElapsed);
    baremetal::string TestFixtureStartMessage(const baremetal::string& fixtureName, int numberOfTests);
    baremetal::string TestFixtureFinishMessage(const baremetal::string& fixtureName, int numberOfTests, int milliSecondsElapsed);
    baremetal::string TestFinishMessage(TestDetails const& test, bool success, int milliSecondsElapsed);
    baremetal::string TestRunSummaryMessage(const TestResults* results, int milliSecondsElapsed);
    baremetal::string TestRunOverviewMessage(const TestResults* results);
    baremetal::string TestName(const baremetal::string& suiteName, const baremetal::string& fixtureName, const baremetal::string& testName);

};

} // namespace unittest
