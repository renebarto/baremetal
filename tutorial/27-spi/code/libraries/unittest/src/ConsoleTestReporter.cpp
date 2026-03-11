//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : ConsoleTestReporter.cpp
//
// Namespace   : unittest
//
// Class       : ConsoleTestReporter
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

#include "unittest/ConsoleTestReporter.h"

#include "baremetal/Console.h"
#include "baremetal/Format.h"
#include "baremetal/Serialization.h"
#include "unittest/TestDetails.h"
#include "unittest/TestRegistry.h"
#include "unittest/TestResults.h"

/// @file
/// Console test reporter implementation

using namespace baremetal;

namespace unittest {

const String ConsoleTestReporter::TestRunSeparator = baremetal::String("[===========]");
const String ConsoleTestReporter::TestSuiteSeparator = baremetal::String("[   SUITE   ]");
const String ConsoleTestReporter::TestFixtureSeparator = baremetal::String("[  FIXTURE  ]");
const String ConsoleTestReporter::TestSuccessSeparator = baremetal::String("[ SUCCEEDED ]");
const String ConsoleTestReporter::TestFailSeparator = baremetal::String("[  FAILED   ]");

/// <summary>
/// Constructor
/// </summary>
ConsoleTestReporter::ConsoleTestReporter()
{
}

/// <summary>
/// Start of test run callback
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites to be run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
/// <param name="numberOfTests">Number of tests to be run</param>
void ConsoleTestReporter::ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(TestRunSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
}

/// <summary>
/// Finish of test run callback
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures run</param>
/// <param name="numberOfTests">Number of tests run</param>
void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(TestRunSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
}

/// <summary>
/// Test summary callback
/// </summary>
/// <param name="results">Test run results</param>
void ConsoleTestReporter::ReportTestRunSummary(const TestResults& results)
{
    if (results.GetFailureCount() > 0)
        GetConsole().SetTerminalColor(ConsoleColor::Red);
    else
        GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(Format("%s\n", TestRunSummaryMessage(results).c_str()));
    GetConsole().ResetTerminalColor();
}

/// <summary>
/// Test run overview callback
/// </summary>
/// <param name="results">Test run results</param>
void ConsoleTestReporter::ReportTestRunOverview(const TestResults& results)
{
    GetConsole().Write(Format("%s\n", TestRunOverviewMessage(results).c_str()));
}

/// <summary>
/// Test suite start callback
/// </summary>
/// <param name="suiteName">Test suite name</param>
/// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
void ConsoleTestReporter::ReportTestSuiteStart(const String& suiteName, int numberOfTestFixtures)
{
    GetConsole().SetTerminalColor(ConsoleColor::Cyan);
    GetConsole().Write(TestSuiteSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestSuiteStartMessage(suiteName, numberOfTestFixtures).c_str()));
}

/// <summary>
/// Test suite finish callback
/// </summary>
/// <param name="suiteName">Test suite name</param>
/// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
void ConsoleTestReporter::ReportTestSuiteFinish(const String& suiteName, int numberOfTestFixtures)
{
    GetConsole().SetTerminalColor(ConsoleColor::Cyan);
    GetConsole().Write(TestSuiteSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestSuiteFinishMessage(suiteName, numberOfTestFixtures).c_str()));
}

/// <summary>
/// Test fixture start callback
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
void ConsoleTestReporter::ReportTestFixtureStart(const String& fixtureName, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Yellow);
    GetConsole().Write(TestFixtureSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestFixtureStartMessage(fixtureName, numberOfTests).c_str()));
}

/// <summary>
/// Test fixture finish callback
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
void ConsoleTestReporter::ReportTestFixtureFinish(const String& fixtureName, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Yellow);
    GetConsole().Write(TestFixtureSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestFixtureFinishMessage(fixtureName, numberOfTests).c_str()));
}

/// <summary>
/// Test start callback
/// </summary>
/// <param name="details">Test details</param>
void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
{
    DeferredTestReporter::ReportTestStart(details);
}

/// <summary>
/// Test finish callback
/// </summary>
/// <param name="details">Test details</param>
/// <param name="success">Test result, true is successful, false is failed</param>
void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
{
    DeferredTestReporter::ReportTestFinish(details, success);
    GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
    if (success)
        GetConsole().Write(TestSuccessSeparator);
    else
        GetConsole().Write(TestFailSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
}

/// <summary>
/// Test failure callback
/// </summary>
/// <param name="details">Test details</param>
/// <param name="failure">Test failure message</param>
void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const String& failure)
{
    DeferredTestReporter::ReportTestFailure(details, failure);
}

/// <summary>
/// Return the correct singular or plural form for "test"
/// </summary>
/// <param name="numberOfTests">Number of tests</param>
/// <returns></returns>
static String TestLiteral(int numberOfTests)
{
    return baremetal::String((numberOfTests == 1) ? "test" : "tests");
}

/// <summary>
/// Return the correct singular or plural form for "failure"
/// </summary>
/// <param name="numberOfTestFailures">Number of test failures</param>
/// <returns></returns>
static String TestFailureLiteral(int numberOfTestFailures)
{
    return baremetal::String((numberOfTestFailures == 1) ? "failure" : "failures");
}

/// <summary>
/// Return the correct singular or plural form for "fixture"
/// </summary>
/// <param name="numberOfTestFixtures">Number of test fixtures</param>
/// <returns></returns>
static String TestFixtureLiteral(int numberOfTestFixtures)
{
    return baremetal::String((numberOfTestFixtures == 1) ? "fixture" : "fixtures");
}

/// <summary>
/// Return the correct singular or plural form for "suite"
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites</param>
/// <returns></returns>
static String TestSuiteLiteral(int numberOfTestSuites)
{
    return baremetal::String((numberOfTestSuites == 1) ? "suite" : "suites");
}

/// <summary>
/// Create a message for test run start
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites to be run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
/// <param name="numberOfTests">Number of tests to be run</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    // clang-format off
    return Format("Running %s %s from %s %s in %s %s.",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        Serialize(numberOfTestSuites).c_str(),
        TestSuiteLiteral(numberOfTestSuites).c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test run finish
/// </summary>
/// <param name="numberOfTestSuites">Number of test suites run</param>
/// <param name="numberOfTestFixtures">Number of test fixtures run</param>
/// <param name="numberOfTests">Number of tests run</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    // clang-format off
    return Format("%s %s from %s %s in %s %s ran.",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        Serialize(numberOfTestSuites).c_str(),
        TestSuiteLiteral(numberOfTestSuites).c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test run summary
/// </summary>
/// <param name="results">Test run results</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestRunSummaryMessage(const TestResults& results)
{
    // clang-format off
    if (results.GetFailureCount() > 0)
    {
        return Format("FAILURE: %s out of %s %s failed (%s %s).\n",
            Serialize(results.GetFailedTestCount()).c_str(),
            Serialize(results.GetTotalTestCount()).c_str(),
            TestLiteral(results.GetTotalTestCount()).c_str(),
            Serialize(results.GetFailureCount()).c_str(),
            TestFailureLiteral(results.GetFailureCount()).c_str());
    }
    return Format("Success: %s %s passed.\n",
        Serialize(results.GetTotalTestCount()).c_str(),
        TestLiteral(results.GetTotalTestCount()).c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test run overview
/// </summary>
/// <param name="results">Test run results</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
{
    if (results.GetFailureCount() > 0)
    {
        String result = "Failures:\n";
        auto testResultPtr = Results().GetHead();
        while (testResultPtr != nullptr)
        {
            auto const& testResult = testResultPtr->GetResult();
            if (testResult.Failed())
            {
                auto failuresPtr = testResult.Failures().GetHead();
                while (failuresPtr != nullptr)
                {
                    result.append(TestFailureMessage(testResult, failuresPtr->GetFailure()));
                    failuresPtr = failuresPtr->GetNext();
                }
            }
            testResultPtr = testResultPtr->GetNext();
        }
        return result;
    }
    return "No failures";
}

/// <summary>
/// Create a message for test failure
/// </summary>
/// <param name="result">Test run results</param>
/// <param name="failure">Failure that occurred</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestFailureMessage(const TestResult& result, const Failure& failure)
{
    return Format("%s:%d : Failure in %s: %s\n", result.Details().SourceFileName().c_str(), failure.SourceLineNumber(),
                  result.Details().QualifiedTestName().c_str(), failure.Text().c_str());
}

/// <summary>
/// Create a message for test suite start
/// </summary>
/// <param name="suiteName">Name of test suite</param>
/// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestSuiteStartMessage(const String& suiteName, int numberOfTestFixtures)
{
    // clang-format off
    return Format("%s (%s %s)",
        suiteName.c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test suite finish
/// </summary>
/// <param name="suiteName">Name of test suite</param>
/// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestSuiteFinishMessage(const String& suiteName, int numberOfTestFixtures)
{
    // clang-format off
    return Format("%s %s from %s",
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        suiteName.c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test fixture start
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestFixtureStartMessage(const String& fixtureName, int numberOfTests)
{
    // clang-format off
    return Format("%s (%s %s)",
        fixtureName.c_str(),
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test fixture finish
/// </summary>
/// <param name="fixtureName">Test fixture name</param>
/// <param name="numberOfTests">Number of tests within test fixture</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestFixtureFinishMessage(const String& fixtureName, int numberOfTests)
{
    // clang-format off
    return Format("%s %s from %s",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        fixtureName.c_str());
    // clang-format on
}

/// <summary>
/// Create a message for test finish
/// </summary>
/// <param name="details">Test details</param>
/// <param name="success">Test result, true is successful, false is failed</param>
/// <returns>Resulting message</returns>
String ConsoleTestReporter::TestFinishMessage(const TestDetails& details, bool /*success*/)
{
    return details.QualifiedTestName();
}

} // namespace unittest
