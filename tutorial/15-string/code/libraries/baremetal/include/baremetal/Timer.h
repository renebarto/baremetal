//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Timer.h
//
// Namespace   : baremetal
//
// Class       : Timer
//
// Description : Timer class
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

/// @file
/// Raspberry Pi Timer

#include <stdlib/Types.h>

namespace baremetal
{

class IMemoryAccess;

/// <summary>
/// Timer class. For now only contains busy waiting methods
///
/// Note that this class is created as a singleton, using the GetTimer() function.
/// </summary>
class Timer
{
    /// <summary>
    /// Retrieves the singleton Timer instance. It is created in the first call to this function. This is a friend function of class Timer
    /// </summary>
    /// <returns>A reference to the singleton Timer</returns>
    friend Timer &GetTimer();

private:
    /// <summary>
    /// Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
    /// </summary>
    IMemoryAccess &m_memoryAccess;

    Timer();

public:
    Timer(IMemoryAccess &memoryAccess);

    void GetTimeString(char* buffer, size_t bufferSize);

    static void WaitCycles(uint32 numCycles);

    uint64 GetSystemTimer();

    static void WaitMilliSeconds(uint64 msec);
    static void WaitMicroSeconds(uint64 usec);
};

Timer &GetTimer();

} // namespace baremetal
