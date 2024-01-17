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

/// @file
/// Raspberry Pi Timer

#include <baremetal/Types.h>

namespace baremetal {

class IMemoryAccess;

/// <summary>
/// Timer class. For now only contains busy waiting methods
///
/// Note that this class is created as a singleton, using the GetTimer() function.
/// </summary>
class Timer
{
    friend Timer& GetTimer();

private:
    /// <summary>
    /// Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
    /// </summary>
    IMemoryAccess& m_memoryAccess;

    /// <summary>
    /// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
    /// </summary>
    Timer();

public:
    /// <summary>
    /// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
    /// </summary>
    /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
    Timer(IMemoryAccess& memoryAccess);

    /// <summary>
    /// Wait for specified number of NOP statements. Busy wait
    /// </summary>
    /// <param name="numCycles">Number of cycles to wait</param>
    static void WaitCycles(uint32 numCycles);

#if defined(USE_PHYSICAL_COUNTER)
    /// <summary>
    /// Reads the BCM2835 System Timer counter value. See @ref RASPBERRY_PI_SYSTEM_TIMER
    /// </summary>
    /// <returns>System Timer count value</returns>
    uint64 GetSystemTimer();
#endif

    /// <summary>
    /// Wait for msec milliseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical counter). Busy wait
    /// 
    /// Depending on whether @ref BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_USE_PHYSICAL_COUNTER is defined, the timer will either use the ARM builtin timer (USE_PHYSICAL_COUNTER not defined) or the System Timer which is part of the BCM2835 chip (or newer) (USE_PHYSICAL_COUNTER defined).
    /// </summary>
    /// <param name="msec">Wait time in milliseconds</param>
    static void WaitMilliSeconds(uint64 msec);

    /// <summary>
    /// Wait for usec microseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical
    /// counter). Busy wait
    /// 
    /// Depending on whether @ref BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_USE_PHYSICAL_COUNTER is defined, the timer will either use the ARM builtin timer (USE_PHYSICAL_COUNTER not defined) or the System Timer which is part of the BCM2835 chip (or newer) (USE_PHYSICAL_COUNTER defined).
    /// </summary>
    /// <param name="usec">Wait time in microseconds</param>
    static void WaitMicroSeconds(uint64 usec);
};

/// <summary>
/// Retrieves the singleton Timer instance. It is created in the first call to this function.
/// </summary>
/// <returns>A reference to the singleton Timer</returns>
Timer& GetTimer();

} // namespace baremetal
