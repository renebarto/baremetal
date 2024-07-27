//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestRegistry.cpp
//
// Namespace   : unittest
//
// Class       : TestRegistry
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

#include <unittest/TestRegistry.h>

#include <baremetal/Assert.h>
#include <baremetal/Logger.h>
#include <unittest/TestInfo.h>

/// @file
/// Test registry implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("TestRegistry");

namespace unittest {

/// <summary>
/// Returns the test registry (singleton)
/// </summary>
/// <returns>Test registry reference</returns>
TestRegistry& TestRegistry::GetTestRegistry()
{
    static TestRegistry s_registry;
    return s_registry;
}

/// <summary>
/// Constructor
/// </summary>
TestRegistry::TestRegistry()
    : m_head{}
    , m_tail{}
{
}

/// <summary>
/// Destructor
///
/// Cleans up all registered test suites, test fixtures and tests
/// </summary>
TestRegistry::~TestRegistry()
{
    TestSuiteInfo *testSuite = m_head;
    while (testSuite != nullptr)
    {
        TestSuiteInfo *currentSuite = testSuite;
        testSuite = testSuite->m_next;
        delete currentSuite;
    }
}

/// <summary>
/// Find a test suite with specified name, register a new one if not found
/// </summary>
/// <param name="suiteName">Test suite name to search for</param>
/// <returns>Found or created test suite</returns>
TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
{
    TestSuiteInfo *testSuite = m_head;
    while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
        testSuite = testSuite->m_next;
    if (testSuite == nullptr)
    {
#if DEBUG_REGISTRY
        LOG_DEBUG("Find suite %s ... not found, creating new object", suiteName.c_str());
#endif
        testSuite = new TestSuiteInfo(suiteName);
        AddSuite(testSuite);
    }
    else
    {
#if DEBUG_REGISTRY
        LOG_DEBUG("Find suite %s ... found", suiteName.c_str());
#endif
    }
    return testSuite;
}

/// <summary>
/// Add a test suite
/// </summary>
/// <param name="testSuite">Test suite to add</param>
void TestRegistry::AddSuite(TestSuiteInfo *testSuite)
{
    if (m_tail == nullptr)
    {
        assert(m_head == nullptr);
        m_head = testSuite;
        m_tail = testSuite;
    }
    else
    {
        m_tail->m_next = testSuite;
        m_tail = testSuite;
    }
}

/// <summary>
/// Constructor
///
/// Finds or registers the test suite specified in the test details of the test.
/// Finds or registers the test fixture specified in the test details of the test.
/// Adds the test to the fixture found or created.
/// </summary>
/// <param name="registry">Test registry</param>
/// <param name="testInstance">Test instance to link to</param>
/// <param name="details">Test details</param>
TestRegistrar::TestRegistrar(TestRegistry &registry, Test *testInstance, const TestDetails& details)
{
#if DEBUG_REGISTRY
    LOG_DEBUG("Register test %s in fixture %s in suite %s",
        details.TestName().c_str(),
        (details.FixtureName().c_str()),
        (details.SuiteName().c_str()));
#endif
    TestSuiteInfo   *testSuite   = registry.GetTestSuite(details.SuiteName());
    TestFixtureInfo *testFixture = testSuite->GetTestFixture(details.FixtureName());
    TestInfo        *test = new TestInfo(testInstance, details);
    testFixture->AddTest(test);
}

} // namespace unittest
