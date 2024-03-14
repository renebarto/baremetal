//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : TestResult.cpp
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

#include <unittest/TestResult.h>

#include <unittest/TestDetails.h>

/// @file
/// Test result implementation
/// 
/// Result of a single test

using namespace baremetal;

namespace unittest {

Failure::Failure(int lineNumber, const baremetal::string& text)
    : m_lineNumber{lineNumber}
    , m_text{text}
{
}

FailureEntry::FailureEntry(const Failure& failure)
    : m_failure{ failure }
    , m_next{}
{
}

FailureList::FailureList()
    : m_head{}
    , m_tail{}
{
}

FailureList::~FailureList()
{
    auto current = m_head;
    while (current != nullptr)
    {
        auto next = current->m_next;
        delete current;
        current = next;
    }
}

void FailureList::Add(const Failure& failure)
{
    auto entry = new FailureEntry(failure);
    if (m_head == nullptr)
    {
        m_head = entry;
    }
    else
    {
        auto current = m_head;
        while (current->m_next != nullptr)
            current = current->m_next;
        current->m_next = entry;
    }
    m_tail = entry;
}

TestResult::TestResult(const TestDetails& details)
    : m_details{ details }
    , m_failures{}
    , m_failed{}
{
}

void TestResult::AddFailure(const Failure& failure)
{
    m_failures.Add(failure);
    m_failed = true;
}

} // namespace unittest
