//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Timer.cpp
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

#include <baremetal/Timer.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Raspberry Pi Timer implementation

/// @brief Number of milliseconds in a second
#define MSEC_PER_SEC  1000
/// @brief Number of microseconds in a second
#define USEC_PER_SEC  1000000
/// @brief Number of microseconds in a millisecond
#define USEC_PER_MSEC USEC_PER_SEC / MSEC_PER_SEC

using namespace baremetal;

/// <summary>
/// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
/// </summary>
Timer::Timer()
    : m_memoryAccess{ GetMemoryAccess() }
{
}

/// <summary>
/// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
Timer::Timer(IMemoryAccess &memoryAccess)
    : m_memoryAccess{ memoryAccess }
{
}

/// <summary>
/// Wait for specified number of NOP statements. Busy wait
/// </summary>
/// <param name="numCycles">Number of cycles to wait</param>
void Timer::WaitCycles(uint32 numCycles)
{
    if (numCycles)
    {
        while (numCycles--)
        {
            NOP();
        }
    }
}

/// <summary>
/// Wait for msec milliseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical counter). Busy wait
///
/// The timer used for the delays is the ARM builtin timer.
/// </summary>
/// <param name="msec">Wait time in milliseconds</param>
void Timer::WaitMilliSeconds(uint64 msec)
{
    WaitMicroSeconds(msec * USEC_PER_MSEC);
}

/// <summary>
/// Wait for usec microseconds using ARM timer registers (when not using physical counter) or BCM2835 system timer peripheral (when using physical
/// counter). Busy wait
///
/// The timer used is the ARM builtin timer.
/// </summary>
/// <param name="usec">Wait time in microseconds</param>
void Timer::WaitMicroSeconds(uint64 usec)
{
    unsigned long freq{};
    unsigned long start{};
    unsigned long current{};
    // Get the current counter frequency (ticks per second)
    GetTimerFrequency(freq);
    // Read the current counter
    GetTimerCounter(start);
    // Calculate required count increase
    unsigned long wait = (freq / USEC_PER_SEC) * usec;
    // Loop while counter increase is less than wait
    // Careful: busy wait
    do
    {
        GetTimerCounter(current);
    }
    while (current - start < wait);
}

/// <summary>
/// Reads the BCM2835 System Timer counter value. See @ref RASPBERRY_PI_SYSTEM_TIMER
/// </summary>
/// <returns>System Timer count value</returns>
uint64 Timer::GetSystemTimer()
{
    uint32 highWord = -1;
    uint32 lowWord{};
    // We must read MMIO area as two separate 32 bit reads
    highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
    lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
    // We have to repeat it if high word changed during read
    if (highWord != m_memoryAccess.Read32(RPI_SYSTMR_HI))
    {
        highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
        lowWord = m_memoryAccess.Read32(RPI_SYSTMR_LO);
    }
    // compose long int value
    return (static_cast<uint64>(highWord) << 32 | lowWord);
}

/// <summary>
/// Retrieves the singleton Timer instance. It is created in the first call to this function.
/// </summary>
/// <returns>A reference to the singleton Timer</returns>
Timer &baremetal::GetTimer()
{
    static Timer timer;
    return timer;
}
