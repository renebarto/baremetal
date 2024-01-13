//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

#include <baremetal/Timer.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>

#define MSEC_PER_SEC  1000
#define USEC_PER_SEC  1000000
#define USEC_PER_MSEC USEC_PER_SEC / MSEC_PER_SEC

using namespace baremetal;

Timer::Timer()
    : m_memoryAccess{ GetMemoryAccess() }
{
}

Timer::Timer(IMemoryAccess& memoryAccess)
    : m_memoryAccess{ memoryAccess }
{
}

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

void Timer::WaitMilliSeconds(uint64 msec)
{
    WaitMicroSeconds(msec * USEC_PER_MSEC);
}

#if !defined(USE_PHYSICAL_COUNTER)
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
    unsigned long wait = ((freq / USEC_PER_SEC) * usec) / USEC_PER_SEC;
    // Loop while counter increase is less than wait
    // Careful: busy wait
    do
    {
        GetTimerCounter(current);
    } while (current - start < wait);
}
#else
// Get System Timer counter (BCM2835 peripheral)
uint64 Timer::GetSystemTimer()
{
    uint32 highWord = -1;
    uint32 lowWord{};
    // We must read MMIO area as two separate 32 bit reads
    highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
    lowWord  = m_memoryAccess.Read32(RPI_SYSTMR_LO);
    // We have to repeat it if high word changed during read
    if (highWord != m_memoryAccess.Read32(RPI_SYSTMR_HI))
    {
        highWord = m_memoryAccess.Read32(RPI_SYSTMR_HI);
        lowWord  = m_memoryAccess.Read32(RPI_SYSTMR_LO);
    }
    // compose long int value
    return (static_cast<uint64>(highWord) << 32 | lowWord);
}

void Timer::WaitMicroSeconds(uint64 usec)
{
    auto start = GetTimer().GetSystemTimer();
    // We must check if it's non-zero, because QEMU does not emulate
    // system timer, and returning constant zero would mean infinite loop
    if (start)
    {
        while (GetTimer().GetSystemTimer() - start < usec)
            NOP();
    }
}
#endif

Timer& baremetal::GetTimer()
{
    static Timer timer;
    return timer;
}
