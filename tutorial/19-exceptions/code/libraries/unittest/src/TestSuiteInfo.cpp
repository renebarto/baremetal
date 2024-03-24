//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestSuiteInfo.cpp
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

#include <unittest/TestSuiteInfo.h>

#include <baremetal/Assert.h>
#include <baremetal/Logger.h>
#include <unittest/TestRegistry.h>

/// @file
/// Test suite administration implementation

using namespace baremetal;

namespace unittest {

/// @brief Define log name
LOG_MODULE("TestSuiteInfo");

/// <summary>
/// Constructor
/// </summary>
/// <param name="suiteName">Test suite name</param>
TestSuiteInfo::TestSuiteInfo(const string &suiteName)
    : m_head{}
    , m_tail{}
    , m_next{}
    , m_suiteName{suiteName}
{
}

/// <summary>
/// Destructor
///
/// Cleans up all registered tests and test fixtures for this test suite
/// </summary>
TestSuiteInfo::~TestSuiteInfo()
{
    TestFixtureInfo *testFixture = m_head;
    while (testFixture != nullptr)
    {
        TestFixtureInfo *currentFixture = testFixture;
        testFixture                     = testFixture->m_next;
        delete currentFixture;
    }
}

/// <summary>
/// Find a test fixture with specified name, register a new one if not found
/// </summary>
/// <param name="fixtureName">Test fixture name to search for</param>
/// <returns>Found or created test fixture</returns>
TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
{
    TestFixtureInfo *testFixture = m_head;
    while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
        testFixture = testFixture->m_next;
    if (testFixture == nullptr)
    {
#if DEBUG_REGISTRY
        LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.c_str());
#endif
        testFixture = new TestFixtureInfo(fixtureName);
        AddFixture(testFixture);
    }
    else
    {
#if DEBUG_REGISTRY
        LOG_DEBUG("Fixture %s found", fixtureName.c_str());
#endif
    }
    return testFixture;
}

/// <summary>
/// Add a test fixture
/// </summary>
/// <param name="testFixture">Test fixture to add</param>
void TestSuiteInfo::AddFixture(TestFixtureInfo *testFixture)
{
    if (m_tail == nullptr)
    {
        assert(m_head == nullptr);
        m_head = testFixture;
        m_tail = testFixture;
    }
    else
    {
        m_tail->m_next = testFixture;
        m_tail         = testFixture;
    }
}

/// <summary>
/// Count the number of test fixtures in the test suite
/// </summary>
/// <returns>Number of test fixtures in the test suite</returns>
int TestSuiteInfo::CountFixtures()
{
    int numberOfTestFixtures = 0;
    TestFixtureInfo *testFixture = Head();
    while (testFixture != nullptr)
    {
        ++numberOfTestFixtures;
        testFixture = testFixture->m_next;
    }
    return numberOfTestFixtures;
}

/// <summary>
/// Count the number of tests in the test suite
/// </summary>
/// <returns>Number of tests in the test suite</returns>
int TestSuiteInfo::CountTests()
{
    int numberOfTests = 0;
    TestFixtureInfo *testFixture = Head();
    while (testFixture != nullptr)
    {
        numberOfTests += testFixture->CountTests();
        testFixture = testFixture->m_next;
    }
    return numberOfTests;
}

} // namespace unittest
