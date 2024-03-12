//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestSuiteInfo.h
//
// Namespace   : unittest
//
// Class       : TestSuiteInfo
//
// Description : Test suite info
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

#include <unittest/TestFixtureInfo.h>
#include <unittest/TestResults.h>

/// @file
/// Test suite administration

namespace unittest
{

class TestFixtureInfo;

/// <summary>
/// Test suite administration
///
/// Holds information on a test suite, which includes its name and the list of test fixtures that are part of it
/// </summary>
class TestSuiteInfo
{
private:
    friend class TestRegistry;
    /// @brief Pointer to first test fixture in the list
    TestFixtureInfo* m_head;
    /// @brief Pointer to last test fixture in the list
    TestFixtureInfo* m_tail;
    /// @brief Pointer to next test suite info in the list
    TestSuiteInfo* m_next;
    /// @brief Test suite name
    baremetal::string m_suiteName;

public:
    TestSuiteInfo() = delete;
    TestSuiteInfo(const TestSuiteInfo&) = delete;
    TestSuiteInfo(TestSuiteInfo&&) = delete;
    explicit TestSuiteInfo(const baremetal::string& suiteName);
    virtual ~TestSuiteInfo();

    TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
    TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;

    /// <summary>
    /// Returns the pointer to the first test fixture in the list for this test suite
    /// </summary>
    /// <returns>Pointer to the first test fixture in the list for this test suite</returns>
    TestFixtureInfo* Head() const { return m_head; }

    /// <summary>
    /// Returns the test suite name
    /// </summary>
    /// <returns>Test suite name</returns>
    const baremetal::string& Name() const { return m_suiteName; }

    void Run(TestResults& testResults);

    int CountFixtures();
    int CountTests();

//private:
    TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
    void AddFixture(TestFixtureInfo* testFixture);
};

} // namespace unittest
