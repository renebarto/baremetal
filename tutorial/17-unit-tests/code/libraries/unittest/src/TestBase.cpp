//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestBase.cpp
//
// Namespace   : unittest
//
// Class       : TestBase
//
// Description : Testcase base class
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

#include <unittest/TestBase.h>

#include <unittest/CurrentTest.h>
#include <unittest/ExecuteTest.h>
#include <unittest/TestResults.h>

using namespace baremetal;

namespace unittest {

TestBase::TestBase()
    : m_details{}
    , m_next{}
{
}

TestBase::TestBase(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
    : m_details{ testName, fixtureName, suiteName, fileName, lineNumber }
    , m_next{}
{
}

TestBase::~TestBase()
{
}

void TestBase::Run(TestResults& testResults)
{
    CurrentTest::Results() = &testResults;

    testResults.OnTestStart(m_details);

    Run();

    testResults.OnTestFinish(m_details);
}

void TestBase::Run()
{
    ExecuteTest(*this, m_details);
}

void TestBase::RunImpl() const
{
}

} // namespace unittest
