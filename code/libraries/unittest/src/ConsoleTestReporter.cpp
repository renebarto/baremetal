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

#include "unittest/ConsoleTestReporter.h"

#include "unittest/TestDetails.h"
#include "unittest/TestResults.h"
#include <baremetal/Console.h>
#include <baremetal/Serialization.h>

using namespace baremetal;

namespace unittest
{

enum class ConsoleColor
{
    Default = -1,
    Black = 0,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
};

const char* GetAnsiColorCode(ConsoleColor color)
{
    switch (color)
    {
    case ConsoleColor::Black:
        return "0";
    case ConsoleColor::Red:
        return "1";
    case ConsoleColor::Green:
        return "2";
    case ConsoleColor::Yellow:
        return "3";
    case ConsoleColor::Blue:
        return "4";
    case ConsoleColor::Magenta:
        return "5";
    case ConsoleColor::Cyan:
        return "6";
    case ConsoleColor::White:
        return "7";
    default:
        return 0;
    };
}

static void SetTerminalColor(ConsoleColor foregroundColor = ConsoleColor::Default, ConsoleColor backgroundColor = ConsoleColor::Default)
{
    string command = string("\033[0");
    if (foregroundColor != ConsoleColor::Default)
    {
        command.append(";3");
        command.append(GetAnsiColorCode(foregroundColor));
    }
    if (backgroundColor != ConsoleColor::Default)
    {
        command.append(";4");
        command.append(GetAnsiColorCode(backgroundColor));
    }
    command.append("m");
    GetConsole().Write(command);
}

static void ResetTerminalColor()
{
    SetTerminalColor();
}

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
    SetTerminalColor(ConsoleColor::Green);

    GetConsole().Write(TestRunSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests)); 
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests, int milliSecondsElapsed)
{
    SetTerminalColor(ConsoleColor::Green);

    GetConsole().Write(TestRunSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests, milliSecondsElapsed));
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestRunSummary(const TestResults* results, int milliSecondsElapsed)
{
    if (results->GetFailureCount() > 0)
        SetTerminalColor(ConsoleColor::Red);
    else
        SetTerminalColor(ConsoleColor::Green);

    GetConsole().Write(TestRunSummaryMessage(results, milliSecondsElapsed));
    GetConsole().Write("\n");

    ResetTerminalColor();
}

void ConsoleTestReporter::ReportTestRunOverview(const TestResults * results)
{
    GetConsole().Write(TestRunOverviewMessage(results));
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestSuiteStart(const string& suiteName, int numberOfTestFixtures)
{
    SetTerminalColor(ConsoleColor::Cyan);

    GetConsole().Write(TestSuiteSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestSuiteStartMessage(suiteName, numberOfTestFixtures)); 
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestSuiteFinish(const string& suiteName, int numberOfTestFixtures, int milliSecondsElapsed)
{
    SetTerminalColor(ConsoleColor::Cyan);

    GetConsole().Write(TestSuiteSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestSuiteFinishMessage(suiteName, numberOfTestFixtures, milliSecondsElapsed));
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestFixtureStart(const string& fixtureName, int numberOfTests)
{
    SetTerminalColor(ConsoleColor::Yellow);

    GetConsole().Write(TestFixtureSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestFixtureStartMessage(fixtureName, numberOfTests)); 
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestFixtureFinish(const string& fixtureName, int numberOfTests, int milliSecondsElapsed)
{
    SetTerminalColor(ConsoleColor::Yellow);

    GetConsole().Write(TestFixtureSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestFixtureFinishMessage(fixtureName, numberOfTests, milliSecondsElapsed));
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
{
    DeferredTestReporter::ReportTestStart(details);
}

void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success, int milliSecondsElapsed)
{
    DeferredTestReporter::ReportTestFinish(details, success, milliSecondsElapsed);
    SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);

    if (success)
        GetConsole().Write(TestSuccessSeparator);
    else
        GetConsole().Write(TestFailSeparator);

    ResetTerminalColor();

    GetConsole().Write(" "); 
    GetConsole().Write(TestFinishMessage(details, success, milliSecondsElapsed));
    GetConsole().Write("\n");
}

void ConsoleTestReporter::ReportTestFailure(TestDetails const& details, const string& failure)
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
    return baremetal::string((numberOfTestFixtures == 1) ? "test fixture" : "test fixtures");
}

static string TestSuiteLiteral(int numberOfTestSuites)
{
    return baremetal::string((numberOfTestSuites == 1) ? "test suite" : "test suites");
}

static string TestSuiteName(string name)
{
    return baremetal::string((!name.empty()) ? name : "DefaultSuite");
}

static string TestFixtureName(string name)
{
    return baremetal::string((!name.empty()) ? name : "DefaultFixture");
}

string ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
{
    const size_t BufferSize = 20;
    char buffer[BufferSize];
    string result;
    result.append("Running ");
    Serialize(buffer, BufferSize, numberOfTests);
    result.append(buffer); 
    result.append(" "); 
    result.append(TestLiteral(numberOfTests));
    result.append(" from ");
    result.append(Serialize(numberOfTestFixtures));
    result.append(" ");
    result.append(TestFixtureLiteral(numberOfTestFixtures));
    result.append(" in ");
    result.append(Serialize(numberOfTestSuites));
    result.append(" ");
    result.append(TestSuiteLiteral(numberOfTestSuites));
    result.append(".");
    return result;
}

string ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests, int milliSecondsElapsed)
{
    string result;
    result.append(Serialize(numberOfTests));
    result.append(" "); 
    result.append(TestLiteral(numberOfTests)); 
    result.append(" from "); 
    result.append(Serialize(numberOfTestFixtures));
    result.append(" "); 
    result.append(TestFixtureLiteral(numberOfTestFixtures));
    result.append(" in ");
    result.append(Serialize(numberOfTestSuites));
    result.append(" ");
    result.append(TestSuiteLiteral(numberOfTestSuites));
    result.append(" ran. (");
    result.append(Serialize(milliSecondsElapsed));
    result.append(" ms total)");
    return result;
}

string ConsoleTestReporter::TestRunSummaryMessage(const TestResults* results, int milliSecondsElapsed)
{
    string result;

    if (results->GetFailureCount() > 0)
    {
        result.append("FAILURE: ");
        result.append(Serialize(results->GetFailedTestCount()));
        result.append(" out of ");
        result.append(Serialize(results->GetTotalTestCount()));
        result.append(" tests failed (");
        result.append(Serialize(results->GetFailureCount()));
        result.append(" ");
        result.append(TestFailureLiteral(results->GetFailureCount()));
        result.append(").\n");
    }
    else
    {
        result.append("Success: ");
        result.append(Serialize(results->GetTotalTestCount()));
        result.append(" ");
        result.append(TestLiteral(results->GetTotalTestCount()));
        result.append(" passed.\n");
    }
    result.append("Test time: ");
    result.append(Serialize(milliSecondsElapsed));
    result.append(" ms.");

    return result;
}

string ConsoleTestReporter::TestRunOverviewMessage(const TestResults* results)
{
    string result;

    if (results->GetFailureCount() > 0)
    {
        result.append("Failures:"); result.append("\n");
        auto testResultPtr = Results().m_head;
        while (testResultPtr != nullptr)
        {
            auto testResult = testResultPtr->m_result;
            if (testResult.Failed())
            {
                auto failuresPtr = testResult.Failures().m_head;
                while (failuresPtr != nullptr)
                {
                    auto failure = failuresPtr->m_failure;
                    result.append(testResult.m_fileName);
                    result.append(":");
                    result.append(Serialize(failure.m_value));
                    result.append(" : Failure in ");
                    result.append(TestName(testResult.m_suiteName, testResult.m_fixtureName, testResult.m_testName));
                    result.append(": "); 
                    result.append(failure.m_text);
                    failuresPtr = failuresPtr->m_next;
                }
            }
            testResultPtr = testResultPtr->m_next;
        }
    }
    else
    {
        result.append("No failures");
    }

    return result;
}

string ConsoleTestReporter::TestSuiteStartMessage(const string& suiteName, int numberOfTestFixtures)
{
    string result;
    result.append(TestSuiteName(suiteName));
    result.append(" (");
    result.append(Serialize(numberOfTestFixtures));
    result.append(" ");
    result.append(TestFixtureLiteral(numberOfTestFixtures));
    result.append(")");
    return result;
}

string ConsoleTestReporter::TestSuiteFinishMessage(const string& suiteName, int numberOfTestFixtures, int milliSecondsElapsed)
{
    string result;
    result.append(Serialize(numberOfTestFixtures));
    result.append(" ");
    result.append(TestFixtureLiteral(numberOfTestFixtures));
    result.append(" from ");
    result.append(TestSuiteName(suiteName));
    result.append(" (");
    result.append(Serialize(milliSecondsElapsed));
    result.append(" ms total)");
    return result;
}

string ConsoleTestReporter::TestFixtureStartMessage(const string& fixtureName, int numberOfTests)
{
    string result;
    result.append(TestFixtureName(fixtureName));
    result.append(" (");
    result.append(Serialize(numberOfTests));
    result.append(" ");
    result.append(TestLiteral(numberOfTests));
    result.append(")");
    return result;
}

string ConsoleTestReporter::TestFixtureFinishMessage(const string& fixtureName, int numberOfTests, int milliSecondsElapsed)
{
    string result;
    result.append(Serialize(numberOfTests));
    result.append(" ");
    result.append(TestLiteral(numberOfTests));
    result.append(" from ");
    result.append(TestFixtureName(fixtureName));
    result.append(" (");
    result.append(Serialize(milliSecondsElapsed));
    result.append(" ms total)");
    return result;
}

string ConsoleTestReporter::TestFinishMessage(TestDetails const& details, bool /*success*/, int milliSecondsElapsed)
{
    string result;
    result.append(TestName(details.SuiteName(), details.FixtureName(), details.TestName()));
    result.append(" (");
    result.append(Serialize(milliSecondsElapsed));
    result.append(" ms)");
    return result;
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
