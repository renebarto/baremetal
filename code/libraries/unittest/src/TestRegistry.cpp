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

#include <unittest/TestRegistry.h>

#include <baremetal/Assert.h>
#include <baremetal/Logger.h>

using namespace baremetal;

LOG_MODULE("TestRegistry");

namespace unittest {

const char* TestRegistry::DefaultFixtureName = "DefaultFixture";
const char* TestRegistry::DefaultSuiteName = "DefaultSuite";

TestRegistry& TestRegistry::GetTestRegistry()
{
    static TestRegistry s_registry;
    return s_registry;
}

TestRegistry::TestRegistry()
    : m_head{}
    , m_tail{}
{
}

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

TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
{
    TestSuiteInfo *testSuite = m_head;
    while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
        testSuite = testSuite->m_next;
    if (testSuite == nullptr)
    {
#ifdef DEBUG_REGISTRY
        LOG_DEBUG("Find suite %s ... not found, creating new object", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
#endif
        testSuite = new TestSuiteInfo(suiteName);
        AddSuite(testSuite);
    }
    else
    {
#ifdef DEBUG_REGISTRY
        LOG_DEBUG("Find suite %s ... found", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
#endif
    }
    return testSuite;
}

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

TestSuiteInfo *TestRegistry::GetHead() const
{
    return m_head;
}

TestRegistrar::TestRegistrar(TestRegistry &registry, TestBase *test)
{
#ifdef DEBUG_REGISTRY
    LOG_DEBUG("Register test %s in fixture %s in suite %s",
        test->Details().TestName().c_str(),
        (test->Details().FixtureName().empty() ? TestRegistry::DefaultFixtureName : test->Details().FixtureName().c_str()),
        (test->Details().SuiteName().empty() ? TestRegistry::DefaultSuiteName : test->Details().SuiteName().c_str()));
#endif
    TestSuiteInfo   *testSuite   = registry.GetTestSuite(test->Details().SuiteName());
    TestFixtureInfo *testFixture = testSuite->GetTestFixture(test->Details().FixtureName());
    testFixture->AddTest(test);
}

} // namespace unittest