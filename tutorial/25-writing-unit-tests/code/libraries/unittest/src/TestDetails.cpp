//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : TestDetails.cpp
//
// Namespace   : unittest
//
// Class       : TestDetails
//
// Description : Test details
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

#include "unittest/TestDetails.h"

#include "baremetal/Format.h"

/// @file
/// Test details implementation

using namespace baremetal;

namespace unittest {

const char* TestDetails::DefaultFixtureName = "DefaultFixture";
const char* TestDetails::DefaultSuiteName = "DefaultSuite";

/// <summary>
/// Default constructor
/// </summary>
TestDetails::TestDetails()
    : m_suiteName{}
    , m_fixtureName{}
    , m_testName{}
    , m_fileName{}
    , m_lineNumber{}
{
}

/// <summary>
/// Explicit constructor
/// </summary>
/// <param name="testName">Test name</param>
/// <param name="fixtureName">Name of test fixture test is part of</param>
/// <param name="suiteName">Name of test suite test is part of</param>
/// <param name="fileName">Source file name of test</param>
/// <param name="lineNumber">Source line number of test</param>
TestDetails::TestDetails(const String& testName, const String& fixtureName, const String& suiteName, const String& fileName, int lineNumber)
    : m_suiteName{suiteName}
    , m_fixtureName{fixtureName}
    , m_testName{testName}
    , m_fileName{fileName}
    , m_lineNumber{lineNumber}
{
}

/// <summary>
/// Construct from other test details, override source line number
/// </summary>
/// <param name="other">Test details to copy from</param>
/// <param name="lineNumber">Source line number to set</param>
TestDetails::TestDetails(const TestDetails& other, int lineNumber)
    : m_suiteName{other.m_suiteName}
    , m_fixtureName{other.m_fixtureName}
    , m_testName{other.m_testName}
    , m_fileName{other.m_fileName}
    , m_lineNumber{lineNumber}
{
}

/// <summary>
/// Returns test suite name
/// </summary>
/// <returns>Test suite name</returns>
String TestDetails::SuiteName() const
{
    return m_suiteName.empty() ? String(DefaultSuiteName) : m_suiteName;
}

/// <summary>
/// Returns test fixture name
/// </summary>
/// <returns>Test fixture name</returns>
String TestDetails::FixtureName() const
{
    return m_fixtureName.empty() ? String(DefaultFixtureName) : m_fixtureName;
}

/// <summary>
/// Returns test name
/// </summary>
/// <returns>Test name</returns>
String TestDetails::TestName() const
{
    return m_testName;
}

/// <summary>
/// Return fully qualified test name in format [suite]::[fixture]::[test]
/// </summary>
/// <returns>Resulting String</returns>
String TestDetails::QualifiedTestName() const
{
    return Format("%s::%s::%s", SuiteName().c_str(), FixtureName().c_str(), TestName().c_str());
}

/// <summary>
/// Returns test source file name
/// </summary>
/// <returns>Test source file name</returns>
String TestDetails::SourceFileName() const
{
    return m_fileName;
}

/// <summary>
/// Returns test source line number
/// </summary>
/// <returns>Test source line number</returns>
int TestDetails::SourceFileLineNumber() const
{
    return m_lineNumber;
}

} // namespace unittest
