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
#include <baremetal/List.h>

namespace baremetal {

class InterruptSystem;
class IMemoryAccess;

struct KernelTimer;

/// @brief Handle to a kernel timer
using KernelTimerHandle = uintptr;

/// @brief Kernel timer handler
using KernelTimerHandler = void(KernelTimerHandle timerHandle, void *param, void *context);

/// @brief Periodic timer tick handler
using PeriodicTimerHandler = void(void);

/// @brief Maximum number of periodic tick handlers which can be installed
#define TIMER_MAX_PERIODIC_HANDLERS 4

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
    /// @brief True if class is already initialized
    bool m_isInitialized;
    /// <summary>
    /// Reference to the singleton InterruptSystem instantiation.
    /// </summary>
    InterruptSystem &m_interruptSystem;
    /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
    IMemoryAccess &m_memoryAccess;
    /// @brief Clock ticks per timer tick
    uint64 m_clockTicksPerSystemTick;
    /// @brief Timer tick counter
    volatile uint64 m_ticks;
    /// @brief Uptime in seconds
    volatile uint32 m_upTime;
    /// @brief Time in seconds (epoch time)
    volatile uint64 m_time;
    /// @brief Periodic tick handler functions
    PeriodicTimerHandler *m_periodicHandlers[TIMER_MAX_PERIODIC_HANDLERS];
    /// @brief Number of periodic tick handler functions installed
    volatile unsigned     m_numPeriodicHandlers;
    /// @brief Kernel timer list
    DoubleLinkedList<KernelTimer *> m_kernelTimerList;
    /// @brief Number of days is each month (0 = January, etc.)
    static const unsigned s_daysInMonth[12];
    /// @brief Name of each month (0 = January, etc.)
    static const char    *s_monthName[12];

    Timer();

public:
    Timer(IMemoryAccess &memoryAccess);
    ~Timer();

    void Initialize();

    uint64 GetTicks() const;

    uint32 GetUptime() const;

    uint64 GetTime() const;

    void GetTimeString(char* buffer, size_t bufferSize);

    void RegisterPeriodicHandler(PeriodicTimerHandler *handler);
    void UnregisterPeriodicHandler(PeriodicTimerHandler *handler);

    KernelTimerHandle StartKernelTimer(uint32 delayTicks, KernelTimerHandler *handler, void *param = nullptr, void *context = nullptr);
    void CancelKernelTimer(KernelTimerHandle handle);

    static void WaitCycles(uint32 numCycles);

    uint64 GetSystemTimer();

    static void WaitMilliSeconds(uint64 msec);
    static void WaitMicroSeconds(uint64 usec);

    static bool IsLeapYear(unsigned year);
    static unsigned GetDaysInMonth(unsigned month, unsigned year);

private:
    void PollKernelTimers();
    void InterruptHandler();
    static void InterruptHandler(void *param);
};

Timer &GetTimer();

} // namespace baremetal
