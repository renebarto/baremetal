//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : CurrentTest.cpp
//
// Namespace   : unittest
//
// Class       : CurrentTest
//
// Description : Current test info
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

#include "unittest/CurrentTest.h"

/// @file
/// Current test information implementation

namespace unittest {

/// <summary>
/// Returns reference to static test results pointer
///
/// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test results during the test run.
/// </summary>
/// <returns>A reference to the current test results pointer</returns>
TestResults*& CurrentTest::Results()
{
    static TestResults* testResults = nullptr;
    return testResults;
}

/// <summary>
/// Returns reference to static test details pointer
///
/// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test details during the test run.
/// </summary>
/// <returns>A reference to the current test details pointer</returns>
const TestDetails*& CurrentTest::Details()
{
    static const TestDetails* testDetails = nullptr;
    return testDetails;
}

} // namespace unittest
