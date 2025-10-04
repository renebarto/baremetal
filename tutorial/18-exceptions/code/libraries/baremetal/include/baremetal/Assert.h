//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Assert.h
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : Assertion functions
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

#include "stdlib/Macros.h"
#include "stdlib/Types.h"

/// @file
/// Assertion functions

namespace baremetal {

/// @brief Assertion callback function, which can be installed to handle a failed assertion
using AssertionCallback = void(const char* expression, const char* fileName, int lineNumber);

#ifdef NDEBUG
/// If building for release, assert is replaced by nothing
#define assert(expr) expr;
#else

void AssertionFailed(const char* expression, const char* fileName, int lineNumber);

/// @brief Assertion. If the assertion fails, AssertionFailed is called.
///
/// <param name="expression">Expression to evaluate.
/// If true the assertion succeeds and nothing happens, if false the assertion fails, and the assertion failure handler is invoked.</param>
#define assert(expression) (likely(expression) ? ((void)0) : baremetal::AssertionFailed(#expression, __FILE__, __LINE__))

#endif

void ResetAssertionCallback();
void SetAssertionCallback(AssertionCallback* callback);

} // namespace baremetal
