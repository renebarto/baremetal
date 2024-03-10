//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

#include <unittest/ConsoleTestReporter.h>

#include <baremetal/Console.h>
#include <baremetal/Format.h>
#include <baremetal/Serialization.h>
#include <unittest/TestDetails.h>
#include <unittest/TestRegistry.h>
#include <unittest/TestResults.h>

using namespace baremetal;

namespace unittest
{

const string ConsoleTestReporter::TestRunSeparator = baremetal::string("[===========]");
const string ConsoleTestReporter::TestSuiteSeparator = baremetal::string("[   SUITE   ]");
const string ConsoleTestReporter::TestFixtureSeparator = baremetal::string("[  FIXTURE  ]");
const string ConsoleTestReporter::TestSuccessSeparator = baremetal::string("[ SUCCEEDED ]");
const string ConsoleTestReporter::TestFailSeparator = baremetal::string("[  FAILED   ]");

ConsoleTestReporter::ConsoleTestReporter()
{
}

void ConsoleTestReporter::ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(TestRunSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
}

void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(TestRunSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
}

void ConsoleTestReporter::ReportTestRunSummary(const TestResults& results)
{
    if (results.GetFailureCount() > 0)
        GetConsole().SetTerminalColor(ConsoleColor::Red);
    else
        GetConsole().SetTerminalColor(ConsoleColor::Green);
    GetConsole().Write(Format("%s\n", TestRunSummaryMessage(results).c_str()));
    GetConsole().ResetTerminalColor();
}

void ConsoleTestReporter::ReportTestRunOverview(const TestResults& results)
{
    GetConsole().Write(Format("%s\n", TestRunOverviewMessage(results).c_str()));
}

void ConsoleTestReporter::ReportTestSuiteStart(const string& suiteName, int numberOfTestFixtures)
{
    GetConsole().SetTerminalColor(ConsoleColor::Cyan);
    GetConsole().Write(TestSuiteSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestSuiteStartMessage(suiteName, numberOfTestFixtures).c_str()));
}

void ConsoleTestReporter::ReportTestSuiteFinish(const string& suiteName, int numberOfTestFixtures)
{
    GetConsole().SetTerminalColor(ConsoleColor::Cyan);
    GetConsole().Write(TestSuiteSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestSuiteFinishMessage(suiteName, numberOfTestFixtures).c_str()));
}

void ConsoleTestReporter::ReportTestFixtureStart(const string& fixtureName, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Yellow);
    GetConsole().Write(TestFixtureSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestFixtureStartMessage(fixtureName, numberOfTests).c_str()));
}

void ConsoleTestReporter::ReportTestFixtureFinish(const string& fixtureName, int numberOfTests)
{
    GetConsole().SetTerminalColor(ConsoleColor::Yellow);
    GetConsole().Write(TestFixtureSeparator);
    GetConsole().ResetTerminalColor();

    GetConsole().Write(Format(" %s\n", TestFixtureFinishMessage(fixtureName, numberOfTests).c_str()));
}

void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
{
    DeferredTestReporter::ReportTestStart(details);
}

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

void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
{
    DeferredTestReporter::ReportTestFailure(details, failure);
}

static string TestLiteral(int numberOfTests)
{
    return baremetal::string((numberOfTests == 1) ? "test" : "tests");
}

static string TestFailureLiteral(int numberOfTestFailures)
{
    return baremetal::string((numberOfTestFailures == 1) ? "failure" : "failures");
}

static string TestFixtureLiteral(int numberOfTestFixtures)
{
    return baremetal::string((numberOfTestFixtures == 1) ? "fixture" : "fixtures");
}

static string TestSuiteLiteral(int numberOfTestSuites)
{
    return baremetal::string((numberOfTestSuites == 1) ? "suite" : "suites");
}

static string TestSuiteName(string name)
{
    return baremetal::string((!name.empty()) ? name : baremetal::string(TestRegistry::DefaultSuiteName));
}

static string TestFixtureName(string name)
{
    return baremetal::string((!name.empty()) ? name : baremetal::string(TestRegistry::DefaultFixtureName));
}

string ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    return Format("Running %s %s from %s %s in %s %s.",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        Serialize(numberOfTestSuites).c_str(),
        TestSuiteLiteral(numberOfTestSuites).c_str());
}

string ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    return Format("%s %s from %s %s in %s %s ran.",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        Serialize(numberOfTestSuites).c_str(),
        TestSuiteLiteral(numberOfTestSuites).c_str());
}

string ConsoleTestReporter::TestRunSummaryMessage(const TestResults& results)
{
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
}

string ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
{

    if (results.GetFailureCount() > 0)
    {
        string result = "Failures:\n";
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

string ConsoleTestReporter::TestFailureMessage(const TestResult& result, const Failure& failure)
{
    return Format("%s:%d : Failure in %s: %s\n",
        result.Details().SourceFileName().c_str(),
        failure.SourceLineNumber(), 
        TestName(result.Details().SuiteName(), result.Details().FixtureName(), result.Details().TestName()).c_str(),
        failure.Text().c_str());
}

string ConsoleTestReporter::TestSuiteStartMessage(const string& suiteName, int numberOfTestFixtures)
{
    return Format("%s (%s %s)",
        TestSuiteName(suiteName).c_str(),
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str());
}

string ConsoleTestReporter::TestSuiteFinishMessage(const string& suiteName, int numberOfTestFixtures)
{
    return Format("%s %s from %s",
        Serialize(numberOfTestFixtures).c_str(),
        TestFixtureLiteral(numberOfTestFixtures).c_str(),
        TestSuiteName(suiteName).c_str());
}

string ConsoleTestReporter::TestFixtureStartMessage(const string& fixtureName, int numberOfTests)
{
    return Format("%s (%s %s)",
        TestFixtureName(fixtureName).c_str(),
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str());
}

string ConsoleTestReporter::TestFixtureFinishMessage(const string& fixtureName, int numberOfTests)
{
    return Format("%s %s from %s",
        Serialize(numberOfTests).c_str(),
        TestLiteral(numberOfTests).c_str(),
        TestFixtureName(fixtureName).c_str());
}

string ConsoleTestReporter::TestFinishMessage(const TestDetails& details, bool /*success*/)
{
    return TestName(details.SuiteName(), details.FixtureName(), details.TestName());
}

string ConsoleTestReporter::TestName(const string& suiteName, const string& fixtureName, const string& testName)
{
    string result;
    if (!suiteName.empty())
    {
        result.append(suiteName);
        result.append("::");
    }
    if (!fixtureName.empty())
    {
        result.append(fixtureName);
        result.append("::");
    }
    result.append(testName);
    return result;
}

} // namespace unittest
