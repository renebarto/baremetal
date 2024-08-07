//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestRunner.h
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

#include <unittest/ITestReporter.h>
#include <unittest/TestRegistry.h>
#include <unittest/TestResults.h>

/// @file
/// Test runner

namespace unittest {

class ITestReporter;
class TestInfo;
class TestFixtureInfo;
class TestResults;
class TestSuiteInfo;

/// <summary>
/// True predicate
///
/// Is used as a predicate to select all tests in all test fixtures in all test suites
/// </summary>
struct True
{
    /// <summary>
    /// Returns test selection value
    /// </summary>
    /// <returns>Returns true for all tests, meaning that all tests are selected</returns>
    bool operator()(const TestInfo* const ) const
    {
        return true;
    }
    /// <summary>
    /// Returns test fixture selection value
    /// </summary>
    /// <returns>Returns true for all test fixtures, meaning that all test fixtures are selected</returns>
    bool operator()(const TestFixtureInfo* const ) const
    {
        return true;
    }
    /// <summary>
    /// Returns test suite selection value
    /// </summary>
    /// <returns>Returns true for all test suites, meaning that all test suites are selected</returns>
    bool operator()(const TestSuiteInfo* const ) const
    {
        return true;
    }
};

/// <summary>
/// Selection predicate
///
/// Is used as a predicate to include specific tests in specific test fixtures in specific test suites
/// </summary>
class InSelection
{
private:
    /// @brief Test suite name to select
    const char* m_suiteName;
    /// @brief Test fixture name to select
    const char* m_fixtureName;
    /// @brief Test name to select
    const char* m_testName;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="suiteName">Test suite name to select (nullptr to select all)</param>
    /// <param name="fixtureName">Test fixture name to select (nullptr to select all)</param>
    /// <param name="testName">Test name to select (nullptr to select all)</param>
    InSelection(const char* suiteName, const char* fixtureName, const char* testName)
        : m_suiteName{ suiteName }
        , m_fixtureName{ fixtureName }
        , m_testName{ testName }
    {
    }
    bool operator()(const TestInfo* const test) const;
    bool operator()(const TestFixtureInfo* const fixture) const;
    bool operator()(const TestSuiteInfo* const suite) const;
};

/// <summary>
/// Test runner
///
/// Runs a selected set of tests, and reports using a test reporter
/// </summary>
class TestRunner
{
private:
    /// @brief Selected test reports
    ITestReporter* m_reporter;
    /// @brief Accumulated results for all tests run
    TestResults m_testResults;

public:
    TestRunner(const TestRunner&) = delete;
    explicit TestRunner(ITestReporter* reporter);
    ~TestRunner();

    TestRunner & operator = (const TestRunner&) = delete;

    template <class Predicate>
    int RunTestsIf(TestRegistry const& registry, const Predicate& predicate);

private:
    template <class Predicate>
    void Start(const Predicate& predicate) const;

    template <class Predicate>
    int Finish(const Predicate& predicate) const;
};

/// <summary>
/// Runs all tests selected by the predicate
///
/// The test registry is built at static initialization time and contains all registered tests.
/// </summary>
/// <typeparam name="Predicate">Predicate class for test selected</typeparam>
/// <param name="registry">Test registry</param>
/// <param name="predicate">Test selection predicate</param>
/// <returns>Number of test failures</returns>
template <class Predicate>
int TestRunner::RunTestsIf(TestRegistry const& registry, const Predicate& predicate)
{
    Start(predicate);

    TestRegistry::GetTestRegistry().RunIf(predicate, m_testResults);

    return Finish(predicate);
}

/// <summary>
/// Start a test run
/// </summary>
/// <typeparam name="Predicate">Predicate class for test selected</typeparam>
/// <param name="predicate">Test selection predicate</param>
template <class Predicate>
void TestRunner::Start(const Predicate& predicate) const
{
    TestRegistry& registry = TestRegistry::GetTestRegistry();
    int numberOfTestSuites = registry.CountSuitesIf(predicate);
    int numberOfTestFixtures = registry.CountFixturesIf(predicate);
    int numberOfTests = registry.CountTestsIf(predicate);
    m_reporter->ReportTestRunStart(numberOfTestSuites, numberOfTestFixtures, numberOfTests);
}

/// <summary>
/// Finish a test run
/// </summary>
/// <typeparam name="Predicate">Predicate class for test selected</typeparam>
/// <param name="predicate">Test selection predicate</param>
/// <returns>Number of test failures</returns>
template <class Predicate>
int TestRunner::Finish(const Predicate & predicate) const
{
    m_reporter->ReportTestRunSummary(m_testResults);
    m_reporter->ReportTestRunOverview(m_testResults);

    TestRegistry& registry = TestRegistry::GetTestRegistry();
    int numberOfTestSuites = registry.CountSuitesIf(predicate);
    int numberOfTestFixtures = registry.CountFixturesIf(predicate);
    int numberOfTests = registry.CountTestsIf(predicate);
    m_reporter->ReportTestRunFinish(numberOfTestSuites, numberOfTestFixtures, numberOfTests);

    return m_testResults.GetFailureCount();
}

int RunAllTests(ITestReporter* reporter = nullptr);

/// <summary>
/// Run tests selected by predicate
/// </summary>
/// <typeparam name="Predicate">Predicate class for test selected</typeparam>
/// <param name="reporter">Test reporter to use</param>
/// <param name="predicate">Test selection predicate</param>
/// <returns>Number of test failures</returns>
template <class Predicate>
int RunSelectedTests(ITestReporter* reporter, const Predicate& predicate)
{
    TestRunner runner(reporter);
    return runner.RunTestsIf(TestRegistry::GetTestRegistry(), predicate);
}

} // namespace unittest
