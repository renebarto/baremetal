//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Version.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Baremetal version information
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

#include <baremetal/Version.h>

#include <baremetal/Format.h>
#include <baremetal/Util.h>

/// @file
/// Build version implementation

/// @brief Buffer size of version string buffer
static const size_t BufferSize = 20;
/// @brief Version string buffer
static char s_baremetalVersionString[BufferSize]{};
/// @brief Flag to check if version set up was already done
static bool s_baremetalVersionSetupDone = false;

/// <summary>
/// Set up version string
///
/// The version string is written into a buffer without allocating memory.
/// This is important, as we may be logging before memory management is set up.
/// </summary>
void baremetal::SetupVersion()
{
    if (!s_baremetalVersionSetupDone)
    {
        FormatNoAlloc(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_PATCH_VERSION);
        s_baremetalVersionSetupDone = true;
    }
}

/// <summary>
/// Return version string
/// </summary>
/// <returns>Version string</returns>
const char* baremetal::GetVersion()
{
    return s_baremetalVersionString;
}
