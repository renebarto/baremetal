//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestFixtureInfo.h
//
// Namespace   : unittest
//
// Class       : TestFixtureInfo
//
// Description : Test fixture info
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

#include <unittest/TestBase.h>
#include <unittest/TestResults.h>

namespace unittest
{

class TestBase;

class TestFixtureInfo
{
private:
    friend class TestSuiteInfo;
    friend class TestRegistrar;
    TestBase* m_head;
    TestBase* m_tail;
    TestFixtureInfo* m_next;
    baremetal::string m_fixtureName;

public:
    TestFixtureInfo() = delete;
    TestFixtureInfo(const TestFixtureInfo&) = delete;
    TestFixtureInfo(TestFixtureInfo&&) = delete;
    explicit TestFixtureInfo(const baremetal::string& fixtureName);
    virtual ~TestFixtureInfo();

    TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
    TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;

    TestBase* GetHead() const;

    const baremetal::string& Name() const { return m_fixtureName; }

    template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);

    int CountTests();
    template <typename Predicate> int CountTestsIf(Predicate predicate);

private:
    void AddTest(TestBase* test);
};

template <class Predicate> void TestFixtureInfo::RunIf(const Predicate& predicate, TestResults& testResults)
{
    testResults.OnTestFixtureStart(this);

    TestBase* test = this->GetHead();
    while (test != nullptr)
    {
        if (predicate(test))
            test->Run(testResults);
        test = test->m_next;
    }

    testResults.OnTestFixtureFinish(this);
}

template <typename Predicate> int TestFixtureInfo::CountTestsIf(Predicate predicate)
{
    int numberOfTests = 0;
    TestBase* test = this->GetHead();
    while (test != nullptr)
    {
        if (predicate(test))
            numberOfTests++;
        test = test->m_next;
    }
    return numberOfTests;
}

} // namespace unittest
