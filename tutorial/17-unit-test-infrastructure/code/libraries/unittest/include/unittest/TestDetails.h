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
    TestDetails();
    TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
    TestDetails(const TestDetails& other, int lineNumber);

    /// <summary>
    /// Returns test suite name
    /// </summary>
    /// <returns>Test suite name</returns>
    const baremetal::string& SuiteName() const { return m_suiteName; }
    /// <summary>
    /// Returns test fixture name
    /// </summary>
    /// <returns>Test fixture name</returns>
    const baremetal::string& FixtureName() const { return m_fixtureName; }
    /// <summary>
    /// Returns test name
    /// </summary>
    /// <returns>Test name</returns>
    const baremetal::string& TestName() const { return m_testName; }
    /// <summary>
    /// Returns test source file name
    /// </summary>
    /// <returns>Test source file name</returns>
    const baremetal::string& SourceFileName() const { return m_fileName; }
    /// <summary>
    /// Returns test source line number
    /// </summary>
    /// <returns>Test source line number</returns>
    int SourceFileLineNumber() const { return m_lineNumber; }
};

} // namespace unittest
