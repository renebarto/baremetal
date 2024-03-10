//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

#include <unittest/TestDetails.h>

/// @file
/// Test details implementation

using namespace baremetal;

namespace unittest {

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
TestDetails::TestDetails(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
    : m_suiteName{ suiteName }
    , m_fixtureName{ fixtureName }
    , m_testName{ testName }
    , m_fileName{ fileName }
    , m_lineNumber{ lineNumber }
{
}

/// <summary>
/// Construct from other test details, override source line number
/// </summary>
/// <param name="other">Test details to copy from</param>
/// <param name="lineNumber">Source line number to set</param>
TestDetails::TestDetails(const TestDetails& other, int lineNumber)
    : m_suiteName{ other.m_suiteName }
    , m_fixtureName{ other.m_fixtureName }
    , m_testName{ other.m_testName }
    , m_fileName{ other.m_fileName }
    , m_lineNumber{ lineNumber }
{
}

} // namespace unittest
