//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResult.h
//
// Namespace   : unittest
//
// Class       : TestResult
//
// Description : Test result
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

class TestDetails;

struct Failure
{
    int m_value;
    baremetal::string m_text;
    Failure(int value, const baremetal::string& text);
};

struct FailureEntry
{
    Failure m_failure;
    FailureEntry* m_next;

    explicit FailureEntry(const Failure& failure);
};

class FailureList
{
public:
    FailureEntry* m_head;
    FailureEntry* m_tail;

    FailureList();
    ~FailureList();

    void Add(const Failure& failure);
};

class TestResult
{
public:
    TestResult() = delete;
    explicit TestResult(const TestDetails & details);

    baremetal::string m_suiteName;
    baremetal::string m_fixtureName;
    baremetal::string m_testName;
    baremetal::string m_fileName;
    int m_lineNumber;

    void AddFailure(const Failure& failure);
    const FailureList & Failures() const { return m_failures; }
    FailureList & Failures() { return m_failures; }
    bool &Failed() { return m_failed; }
    bool Failed() const { return m_failed; }

private:
    FailureList m_failures;
    bool m_failed;
};

} // namespace unittest
