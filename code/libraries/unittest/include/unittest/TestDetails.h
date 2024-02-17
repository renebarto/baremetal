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

namespace unittest
{

class TestDetails
{
private:
    const baremetal::string m_suiteName;
    const baremetal::string m_fixtureName;
    const baremetal::string m_testName;
    const baremetal::string m_fileName;
    const int m_lineNumber;

public:
    TestDetails();
    TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
    TestDetails(const TestDetails& details, int lineNumber);
    TestDetails(const TestDetails&) = delete;
    TestDetails(TestDetails&&) = delete;

    TestDetails& operator = (const TestDetails&) = delete;
    TestDetails& operator = (TestDetails&&) = delete;

    int LineNumber() const { return m_lineNumber; }
    const baremetal::string& SuiteName() const { return m_suiteName; }
    const baremetal::string& FixtureName() const { return m_fixtureName; }
    const baremetal::string& TestName() const { return m_testName; }
};

} // namespace unittest