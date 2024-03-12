//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestInfo.h
//
// Namespace   : unittest
//
// Class       : TestInfo
//
// Description : Testcase
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

#include <unittest/TestDetails.h>

/// @file
/// Test administration

namespace unittest
{

class Test;
class TestResults;

/// <summary>
/// Test administration class
/// </summary>
class TestInfo
{
private:
    friend class TestFixtureInfo;
    /// @brief Test details
    const TestDetails m_details;
    /// @brief Pointer to actual test
    Test* m_testInstance;
    /// @brief Pointer to next test case in list
    TestInfo* m_next;

public:
    TestInfo();
    TestInfo(const TestInfo&) = delete;
    TestInfo(TestInfo&&) = delete;
    explicit TestInfo(Test* testInstance, const TestDetails& details);

    TestInfo& operator = (const TestInfo&) = delete;
    TestInfo& operator = (TestInfo&&) = delete;

    /// <summary>
    /// Returns the test details
    /// </summary>
    /// <returns>Test details</returns>
    const TestDetails& Details() const { return m_details; }

    void Run(TestResults& testResults);
};

} // namespace unittest
