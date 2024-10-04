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
#include <unittest/TestDetails.h>

/// @file
/// Test result
///
/// Result of a single test

namespace unittest
{

/// <summary>
/// Failure
///
/// Holds information on an occurrred test failure
/// </summary>
class Failure
{
private:
    /// @brief Line number on which failure occurred
    int m_lineNumber;
    /// @brief Failure message
    baremetal::string m_text;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="lineNumber">Line number on which failure occurred</param>
    /// <param name="text">Failure message, can be empty</param>
    Failure(int lineNumber, const baremetal::string& text);
    /// <summary>
    /// Return line number on which failure occurred
    /// </summary>
    /// <returns>Line number on which failure occurred</returns>
    int SourceLineNumber() const { return m_lineNumber; }
    /// <summary>
    /// Returns failure message
    /// </summary>
    /// <returns>Failure message</returns>
    const baremetal::string& Text() const { return m_text; }
};

/// <summary>
/// Container for failure
/// </summary>
class FailureEntry
{
private:
    friend class FailureList;
    /// @brief Failure information
    Failure m_failure;
    /// @brief Pointer to next failure entry in the list
    FailureEntry* m_next;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="failure">Failure information to set</param>
    explicit FailureEntry(const Failure& failure);
    /// <summary>
    /// Returns failure information
    /// </summary>
    /// <returns>Failure information</returns>
    const Failure& GetFailure() const { return m_failure; }
    /// <summary>
    /// Return pointer to next failure entry
    /// </summary>
    /// <returns>Pointer to next failure entry</returns>
    const FailureEntry* GetNext() const { return m_next; }
};

/// <summary>
/// List of failures
/// </summary>
class FailureList
{
private:
    /// @brief Pointer to first failure entry in the list
    FailureEntry* m_head;
    /// @brief Pointer to last failure entry in the list
    FailureEntry* m_tail;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    FailureList();
    /// <summary>
    /// Destructor
    /// </summary>
    ~FailureList();

    /// <summary>
    /// Returns pointer to first failure in the list
    /// </summary>
    /// <returns>Pointer to first failure in the list</returns>
    const FailureEntry* GetHead() const { return m_head; }
    /// <summary>
    /// Add a failure to the list
    /// </summary>
    /// <param name="failure">Failure information to add</param>
    void Add(const Failure& failure);
};

/// <summary>
/// Results for a single test
/// </summary>
class TestResult
{
private:
    /// @brief Details of the test
    TestDetails m_details;
    /// @brief List of failure for the test
    FailureList m_failures;
    /// @brief Failure flag, true if at least one failure occurred
    bool m_failed;

public:
    TestResult() = delete;
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="details">Test details</param>
    explicit TestResult(const TestDetails & details);

    /// <summary>
    /// Add a failure to the list for this test
    /// </summary>
    /// <param name="failure">Failure information</param>
    void AddFailure(const Failure& failure);
    /// <summary>
    /// Return the list of failures for this test
    /// </summary>
    /// <returns>List of failures for this test</returns>
    const FailureList& Failures() const { return m_failures; }
    /// <summary>
    /// Returns failure flag
    /// </summary>
    /// <returns>Failure flag, true if at least one failure occurred</returns>
    bool Failed() const { return m_failed; }
    /// <summary>
    /// Returns the test details
    /// </summary>
    /// <returns>Test details</returns>
    const TestDetails& Details() const { return m_details; }
};

} // namespace unittest
