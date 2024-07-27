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

#include <baremetal/String.h>

/// @file
/// Abstract test reporter interface

namespace unittest
{

class TestDetails;
class TestResults;

/// <summary>
/// Test reporter abstract interface
/// </summary>
class ITestReporter
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ITestReporter() {}

    /// <summary>
    /// Start of test run callback
    /// </summary>
    /// <param name="numberOfTestSuites">Number of test suites to be run</param>
    /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
    /// <param name="numberOfTests">Number of tests to be run</param>
    virtual void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
    /// <summary>
    /// Finish of test run callback
    /// </summary>
    /// <param name="numberOfTestSuites">Number of test suites run</param>
    /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
    /// <param name="numberOfTests">Number of tests run</param>
    virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
    /// <summary>
    /// Test summary callback
    /// </summary>
    /// <param name="results">Test run results</param>
    virtual void ReportTestRunSummary(const TestResults& results) = 0;
    /// <summary>
    /// Test run overview callback
    /// </summary>
    /// <param name="results">Test run results</param>
    virtual void ReportTestRunOverview(const TestResults& results) = 0;
    /// <summary>
    /// Test suite start callback
    /// </summary>
    /// <param name="suiteName">Test suite name</param>
    /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
    virtual void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) = 0;
    /// <summary>
    /// Test suite finish callback
    /// </summary>
    /// <param name="suiteName">Test suite name</param>
    /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
    virtual void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) = 0;
    /// <summary>
    /// Test fixture start callback
    /// </summary>
    /// <param name="fixtureName">Test fixture name</param>
    /// <param name="numberOfTests">Number of tests within test fixture</param>
    virtual void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) = 0;
    /// <summary>
    /// Test fixture finish callback
    /// </summary>
    /// <param name="fixtureName">Test fixture name</param>
    /// <param name="numberOfTests">Number of tests within test fixture</param>
    virtual void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) = 0;
    /// <summary>
    /// Test start callback
    /// </summary>
    /// <param name="details">Test details</param>
    virtual void ReportTestStart(const TestDetails& details) = 0;
    /// <summary>
    /// Test finish callback
    /// </summary>
    /// <param name="details">Test details</param>
    /// <param name="success">Test result, true is successful, false is failed</param>
    virtual void ReportTestFinish(const TestDetails& details, bool success) = 0;
    /// <summary>
    /// Test failure callback
    /// </summary>
    /// <param name="details">Test details</param>
    /// <param name="failure">Test failure message</param>
    virtual void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) = 0;
};

} // namespace unittest
