//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ITestReporter.h
//
// Namespace   : unittest
//
// Class       : ITestReporter
//
// Description : Test reporter abstract class
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

class TestDetails;
class TestResults;

class ITestReporter
{
public:
    virtual ~ITestReporter() {}

    virtual void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
    virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
    virtual void ReportTestRunSummary(const TestResults& results) = 0;
    virtual void ReportTestRunOverview(const TestResults& results) = 0;
    virtual void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTests) = 0;
    virtual void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTests) = 0;
    virtual void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) = 0;
    virtual void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) = 0;
    virtual void ReportTestStart(const TestDetails& details) = 0;
    virtual void ReportTestFinish(const TestDetails& details, bool success) = 0;
    virtual void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) = 0;
};

} // namespace unittest
