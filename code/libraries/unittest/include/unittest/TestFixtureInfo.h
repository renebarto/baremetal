//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestFixtureInfo.h
//
// Namespace   : unittest
//
// Class       : TestFixtureInfo
//
// Description : TestInfo fixture info
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

#include <unittest/TestInfo.h>
#include <unittest/TestResults.h>

/// @file
/// Test fixture administration

namespace unittest
{

class TestInfo;

/// <summary>
/// Test fixture administration
///
/// Holds information on a test fixture, which includes its name and the list of tests that are part of it
/// </summary>
class TestFixtureInfo
{
private:
    friend class TestSuiteInfo;
    /// @brief Pointer to first test in the list
    TestInfo* m_head;
    /// @brief Pointer to last test in the list
    TestInfo* m_tail;
    /// @brief Pointer to next test fixture info in the list
    TestFixtureInfo* m_next;
    /// @brief Test fixture name
    baremetal::string m_fixtureName;

public:
    TestFixtureInfo() = delete;
    TestFixtureInfo(const TestFixtureInfo&) = delete;
    TestFixtureInfo(TestFixtureInfo&&) = delete;
    explicit TestFixtureInfo(const baremetal::string& fixtureName);
    virtual ~TestFixtureInfo();

    TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
    TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;

    /// <summary>
    /// Returns the pointer to the first test in the list for this test fixture
    /// </summary>
    /// <returns>Pointer to the first test in the list for this test fixture</returns>
    TestInfo* Head() const {  return m_head; }

    /// <summary>
    /// Returns the test fixture name
    /// </summary>
    /// <returns>Test fixture name</returns>
    const baremetal::string& Name() const { return m_fixtureName; }

    void Run(TestResults& testResults);

    int CountTests();

    void AddTest(TestInfo* test);
};

} // namespace unittest
