//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : CXAGuard.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Guard functions, used to initialize static objects inside a function
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

// The semantic of the functions __cxa_guard_*() herein is based
// on the file cxa_guard.cxx of the LLVM Compiler Infrastructure,
// with this license:
//
// ==============================================================================
// LLVM Release License
// ==============================================================================
// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2003-2010 University of Illinois at Urbana-Champaign.
// All rights reserved.
//
// Developed by:
//
//     LLVM Team
//
//     University of Illinois at Urbana-Champaign
//
//     http://llvm.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.

#include "stdlib/Types.h"

/// @file
/// Static variable acquire / release functionality

// Byte index into the guard object
/// @brief Index to byte signalling that the static object was acquired and released
#define INDEX_HAS_RUN 0
/// @brief Index to byte signalling that the static object was acquired but not released yes
#define INDEX_IN_USE  1

/// <summary>
/// Acquire guard for static initialization
/// </summary>
/// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
/// <returns>Returns 1 if acquire is successful, 0 if already done</returns>
extern "C" int __cxa_guard_acquire(volatile uint8* guardObject)
{
    // Did we already initialize this object?
    if (guardObject[INDEX_HAS_RUN] != 0)
    {
        return 0; // Do not run constructor
    }

    // Lock this guard while acquired
    guardObject[INDEX_IN_USE] = 1;

    return 1; // Run constructor
}

/// <summary>
/// Release the acquired guard
/// </summary>
/// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
extern "C" void __cxa_guard_release(volatile uint8* guardObject)
{
    // Set acquire / release cycle complete
    guardObject[INDEX_HAS_RUN] = 1;
    guardObject[INDEX_IN_USE] = 0;
}

/// <summary>
/// Abort the static object initialization, release the acquired object
/// </summary>
/// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
extern "C" void __cxa_guard_abort(volatile uint8* guardObject)
{
    guardObject[INDEX_IN_USE] = 0;
}
