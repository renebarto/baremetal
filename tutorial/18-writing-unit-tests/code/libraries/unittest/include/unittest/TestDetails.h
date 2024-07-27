//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestDetails.h
//
// Namespace   : unittest
//
// Class       : TestDetails
//
// Description : Test detail
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

#include <baremetal/String.h>

/// @file
/// Test details

namespace unittest
{

/// <summary>
/// Details of a test
/// </summary>
class TestDetails
{
private:
    /// @brief Name of test suite test is part of
    const baremetal::string m_suiteName;
    /// @brief Name of test fixture test is part of
    const baremetal::string m_fixtureName;
    /// @brief Test name
    const baremetal::string m_testName;
    /// @brief Source file name of test
    const baremetal::string m_fileName;
    /// @brief Source line number of test
    const int m_lineNumber;

public:
    /// @brief Name of default test fixture. Used for tests that are not in a test fixture
    static const char* DefaultFixtureName;
    /// @brief Name of default test suite. Used for tests and test fixtures that are not in a test suite
    static const char* DefaultSuiteName;

    TestDetails();
    TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
    TestDetails(const TestDetails& other, int lineNumber);

    baremetal::string SuiteName() const;
    baremetal::string FixtureName() const;
    baremetal::string TestName() const;
    baremetal::string QualifiedTestName() const;
    baremetal::string SourceFileName() const;
    int SourceFileLineNumber() const;
};

} // namespace unittest
