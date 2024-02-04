//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Assert.cpp
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
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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

#include <baremetal/Assert.h>

#include <baremetal/Logger.h>
#include <baremetal/System.h>

/// @brief Define log name
LOG_MODULE("Assert");

namespace baremetal {

static void AssertionFailedDefault(const char* expression, const char* fileName, int lineNumber);

static AssertionCallback *s_callback = AssertionFailedDefault;

/// <summary>
/// Log assertion failure and halt, is not expected to return (but may if a different assertion failure function is set up)
/// </summary>
/// <param name="expression">Expression to be printed</param>
/// <param name="fileName">Filename of file causing the failed assertion</param>
/// <param name="lineNumber">Line number causing the failed assertion</param>
void AssertionFailed(const char *expression, const char *fileName, int lineNumber)
{
    if (s_callback != nullptr)
        s_callback(expression, fileName, lineNumber);
}

/// <summary>
/// Default failed assertion handler
/// </summary>
/// <param name="expression">Expression to be printed</param>
/// <param name="fileName">Filename of file causing the failed assertion</param>
/// <param name="lineNumber">Line number causing the failed assertion</param>
static void AssertionFailedDefault(const char *expression, const char *fileName, int lineNumber)
{
    GetLogger().Write(fileName, lineNumber, LogSeverity::Panic, "assertion failed: %s", expression);
}

/// <summary>
/// Reset the assertion failure handler to the default
/// </summary>
void ResetAssertionCallback()
{
    s_callback = AssertionFailedDefault;
}

/// <summary>
/// Sets up a custom assertion failure handler
/// </summary>
/// <param name="callback">Assertion failure handler</param>
void SetAssertionCallback(AssertionCallback* callback)
{
    s_callback = callback;
}

} // namespace baremetal
