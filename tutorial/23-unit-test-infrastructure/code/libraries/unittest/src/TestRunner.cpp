//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : TestRunner.cpp
//
// Namespace   : unittest
//
// Class       : TestRunner
//
// Description : Test runner
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

#include "unittest/TestRunner.h"

/// @file
/// Test runner implementation

namespace unittest {

/// <summary>
/// Returns test selection value
/// </summary>
/// <param name="test">Test to check against selection</param>
/// <returns>Returns true if the test name selection is set to nullptr, or the test name matches the selection</returns>
bool InSelection::operator()(const TestInfo* const test) const
{
    return (m_testName == nullptr) || (test->Details().TestName() == m_testName);
}

/// <summary>
/// Returns test fixture selection value
/// </summary>
/// <param name="fixture">Test fixture to check against selection</param>
/// <returns>Returns true if the test fixture name selection is set to nullptr, or the test fixture name matches the selection</returns>
bool InSelection::operator()(const TestFixtureInfo* const fixture) const
{
    return (m_fixtureName == nullptr) || (fixture->Name() == m_fixtureName);
}

/// <summary>
/// Returns test suite selection value
/// </summary>
/// <param name="suite">Test suite to check against selection</param>
/// <returns>Returns true if the test suite name selection is set to nullptr, or the test suite name matches the selection</returns>
bool InSelection::operator()(const TestSuiteInfo* const suite) const
{
    return (m_suiteName == nullptr) || (suite->Name() == m_suiteName);
}

/// <summary>
/// Constructor
/// </summary>
/// <param name="reporter">Test reporter to use, can be nullptr, in which case no reporting is done</param>
TestRunner::TestRunner(ITestReporter* reporter)
    : m_reporter{reporter}
    , m_testResults{reporter}
{
}

/// <summary>
/// Run all tests with specified test reporter
/// </summary>
/// <param name="reporter">Test reporter to use</param>
/// <returns></returns>
int RunAllTests(ITestReporter* reporter)
{
    return RunSelectedTests(reporter, True());
}

} // namespace unittest
