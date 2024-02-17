//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestFixtureInfo.cpp
//
// Namespace   : unittest
//
// Class       : TestFixtureInfo
//
// Description : Test fixture
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

#include "unittest/TestFixtureInfo.h"

#include <baremetal/Assert.h>

using namespace baremetal;

namespace unittest {

TestFixtureInfo::TestFixtureInfo(const string& fixtureName)
    : m_next{}
    , m_head{}
    , m_tail{}
    , m_fixtureName{ fixtureName }
{
}

TestFixtureInfo::~TestFixtureInfo()
{
    TestBase* test = m_head;
    while (test != nullptr)
    {
        TestBase* currentTest = test;
        test = test->m_next;
        delete currentTest;
    }
}

void TestFixtureInfo::AddTest(TestBase* test)
{
    if (m_tail == nullptr)
    {
        assert(m_head == nullptr);
        m_head = test;
        m_tail = test;
    }
    else
    {
        m_tail->m_next = test;
        m_tail = test;
    }
}

TestBase* TestFixtureInfo::GetHead() const
{
    return m_head;
}

int TestFixtureInfo::CountTests()
{
    int numberOfTests = 0;
    TestBase* test = m_head;
    while (test != nullptr)
    {
        ++numberOfTests;
        test = test->m_next;
    }
    return numberOfTests;
}

} // namespace unittest
