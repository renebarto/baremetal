//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResults.cpp
//
// Namespace   : unittest
//
// Class       : TestResults
//
// Description : Test run results
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

#include <baremetal/Format.h>
#include <baremetal/Logger.h>
#include <baremetal/String.h>
#include <unittest/TestDetails.h>
#include <unittest/TestFixtureInfo.h>
#include <unittest/TestSuiteInfo.h>

/// @file
/// Test results implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("TestResults");

namespace unittest
{

/// <summary>
/// Return fully qualified test name in format [suite]::[fixture]::[test]
/// </summary>
/// <param name="details">Test details</param>
/// <returns>Resulting string</returns>
static string QualifiedTestName(const TestDetails& details)
{
    return Format("%s::%s::%s",
        details.SuiteName().empty() ? "DefaultSuite" : details.SuiteName().c_str(),
        details.FixtureName().empty() ? "DefaultFixture" : details.FixtureName().c_str(),
        details.TestName().c_str());
}

/// <summary>
/// Start a test suite run
/// </summary>
/// <param name="suite">Test suite to start</param>
void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
{
    LOG_INFO(suite->Name() + " Start suite");
}

/// <summary>
/// Finish a test suite run
/// </summary>
/// <param name="suite">Test suite to finish</param>
void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
{
    LOG_INFO(suite->Name() + " Finish suite");
}

/// <summary>
/// Start a test fixture run
/// </summary>
/// <param name="fixture">Test fixture to start</param>
void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
{
    LOG_INFO(fixture->Name() + " Start fixture");
}

/// <summary>
/// Finish a test fixture run
/// </summary>
/// <param name="fixture">Test fixture to finish</param>
void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
{
    LOG_INFO(fixture->Name() + " Finish fixture");
}

/// <summary>
/// Start a test
/// </summary>
/// <param name="details">Test details of test to start</param>
void TestResults::OnTestStart(const TestDetails& details)
{
    LOG_INFO(QualifiedTestName(details) + " Start test");
}

void TestResults::OnTestRun(const TestDetails& details, const string& message)
{
    string fullMessage = QualifiedTestName(details) + Format(" (%s:%d)",
        details.SourceFileName().c_str(),
        details.SourceFileLineNumber()) + " --> " + message;
    LOG_DEBUG(fullMessage);
}

/// <summary>
/// Finish a test
/// </summary>
/// <param name="details">Test details of test to finish</param>
void TestResults::OnTestFinish(const TestDetails& details)
{
    LOG_INFO(QualifiedTestName(details) + " Finish test");
}

} // namespace unittest
