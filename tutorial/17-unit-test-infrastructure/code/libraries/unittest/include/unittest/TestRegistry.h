//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestRegistry.h
//
// Namespace   : unittest
//
// Class       : TestRegistry, TestRegistrar
//
// Description : Test registration
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

#include <unittest/TestSuiteInfo.h>

/// @file
/// Test registry

namespace unittest
{

class Test;
class TestSuiteInfo;
class TestResults;

/// <summary>
/// Test registry
/// </summary>
class TestRegistry
{
private:
    friend class TestRegistrar;
    /// @brief Pointer to first test suite in the list
    TestSuiteInfo* m_head;
    /// @brief Pointer to last test suite in the list
    TestSuiteInfo* m_tail;

public:
    TestRegistry();
    TestRegistry(const TestRegistry&) = delete;
    TestRegistry(TestRegistry&&) = delete;
    virtual ~TestRegistry();

    TestRegistry& operator = (const TestRegistry&) = delete;
    TestRegistry& operator = (TestRegistry&&) = delete;

    /// <summary>
    /// Returns a pointer to the first test suite in the list
    /// </summary>
    /// <returns>Pointer to the first test suite in the list</returns>
    TestSuiteInfo* Head() const { return m_head; }

    template <typename Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
    template <typename Predicate> int CountSuitesIf(Predicate predicate);
    template <typename Predicate> int CountFixturesIf(Predicate predicate);
    template <typename Predicate> int CountTestsIf(Predicate predicate);

    static TestRegistry& GetTestRegistry();

private:
    TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
    void AddSuite(TestSuiteInfo* testSuite);
};

/// <summary>
/// Test registrar
///
/// This is a utility class to register a test to the registry, as part of a test declaration
/// </summary>
class TestRegistrar
{
public:
    TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details);
};

template <typename Predicate> void TestRegistry::RunIf(const Predicate& predicate, TestResults& testResults)
{
    TestSuiteInfo* testSuite = Head();

    while (testSuite != nullptr)
    {
        if (predicate(testSuite))
            testSuite->RunIf(predicate, testResults);
        testSuite = testSuite->m_next;
    }
}

template <typename Predicate> int TestRegistry::CountSuitesIf(Predicate predicate)
{
    int numberOfTestSuites = 0;
    TestSuiteInfo* testSuite = Head();
    while (testSuite != nullptr)
    {
        if (predicate(testSuite))
            ++numberOfTestSuites;
        testSuite = testSuite->m_next;
    }
    return numberOfTestSuites;
}

template <typename Predicate> int TestRegistry::CountFixturesIf(Predicate predicate)
{
    int numberOfTestFixtures = 0;
    TestSuiteInfo* testSuite = Head();
    while (testSuite != nullptr)
    {
        if (predicate(testSuite))
            numberOfTestFixtures += testSuite->CountFixturesIf(predicate);
        testSuite = testSuite->m_next;
    }
    return numberOfTestFixtures;
}

template <typename Predicate> int TestRegistry::CountTestsIf(Predicate predicate)
{
    int numberOfTests = 0;
    TestSuiteInfo* testSuite = Head();
    while (testSuite != nullptr)
    {
        if (predicate(testSuite))
            numberOfTests += testSuite->CountTestsIf(predicate);
        testSuite = testSuite->m_next;
    }
    return numberOfTests;
}

} // namespace unittest
