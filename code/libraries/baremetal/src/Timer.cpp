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

#include "baremetal/Timer.h"

#include "baremetal/ARMInstructions.h"
#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/InterruptHandler.h"
#include "baremetal/MemoryAccess.h"
#include "stdlib/Util.h"

/// @file
/// Raspberry Pi Timer implementation

using namespace baremetal;

/// <summary>
/// Constructs a default Timer instance (a singleton). Note that the constructor is private, so GetTimer() is needed to instantiate the Timer.
/// </summary>
Timer::Timer()
    : m_interruptSystem{GetInterruptSystem()}
    , m_memoryAccess{GetMemoryAccess()}
    , m_clockTicksPerSystemTick{}
    , m_ticks{}
    , m_upTime{}
    , m_time{}
    , m_periodicHandlers{}
    , m_numPeriodicHandlers{}
{
}

/// <summary>
/// Constructs a specialized Timer instance which injects a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
Timer::Timer(IMemoryAccess& memoryAccess)
    : m_interruptSystem{GetInterruptSystem()}
    , m_memoryAccess{memoryAccess}
    , m_clockTicksPerSystemTick{}
    , m_ticks{}
    , m_upTime{}
    , m_time{}
    , m_periodicHandlers{}
    , m_numPeriodicHandlers{}
{
}

/// <summary>
/// Destructor
///
/// Disables the timer, as well as the timer interrupt
/// </summary>
Timer::~Timer()
{
    SetTimerControl(~CNTP_CTL_EL0_ENABLE);

    m_interruptSystem.UnregisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
}

/// <summary>
/// Timer initialization
///
/// Add a timer interrupt handler, calculate the number of clock ticks per timer tick, set the next timer deadline.
/// Then enables the timer
/// </summary>
void Timer::Initialize()
{
    if (m_isInitialized)
        return;

    memset(m_periodicHandlers, 0, TIMER_MAX_PERIODIC_HANDLERS * sizeof(PeriodicTimerHandler*));
    m_interruptSystem.RegisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS, InterruptHandler, this);

    uint64 counterFreq{};
    GetTimerFrequency(counterFreq);
    assert(counterFreq % TICKS_PER_SECOND == 0);
    m_clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;

    uint64 counter{};
    GetTimerCounter(counter);
    SetTimerCompareValue(counter + m_clockTicksPerSystemTick);
    SetTimerControl(CNTP_CTL_EL0_ENABLE);

    m_isInitialized = true;
}

/// <summary>
/// Return the current timer tick cound
/// </summary>
/// <returns>The current timer tick count</returns>
uint64 Timer::GetTicks() const
{
    return m_ticks;
}

/// <summary>
/// Return the uptime in seconds
/// </summary>
/// <returns>Uptime in seconds</returns>
uint32 Timer::GetUptime() const
{
    return m_upTime;
}

/// <summary>
/// Return the current time in seconds (epoch time)
/// </summary>
/// <returns>Current time in seconds (epoch time)</returns>
uint64 Timer::GetTime() const
{
    return m_time;
}

/// <summary>
/// Writes a representation of the current time to a buffer, or of the uptime if the current time is not valid.
///
/// For now returns an empty string
/// </summary>
/// <param name="buffer">Buffer to write the time string to</param>
/// <param name="bufferSize">Size of the buffer</param>
void Timer::GetTimeString(char* buffer, size_t bufferSize)
{
    if ((buffer == nullptr) || (bufferSize == 0))
    {
        return;
    }
    *buffer = '\0';
}

/// <summary>
/// Register a periodic timer handler
///
/// Registers a periodic timer handler function. The handler function will be called every timer tick.
/// </summary>
/// <param name="handler">Pointer to periodic timer handler to register</param>
void Timer::RegisterPeriodicHandler(PeriodicTimerHandler* handler)
{
    assert(handler != nullptr);
    assert(m_numPeriodicHandlers < TIMER_MAX_PERIODIC_HANDLERS);

    size_t index{};
    for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
    {
        if (m_periodicHandlers[index] == nullptr)
            break;
    }
    assert(index < TIMER_MAX_PERIODIC_HANDLERS);
    m_periodicHandlers[index] = handler;

    DataSyncBarrier();

    m_numPeriodicHandlers++;
}

/// <summary>
/// Unregister a periodic timer handler
///
/// Removes aperiodic timer handler function from the registration. The handler function will no longer be called.
/// </summary>
/// <param name="handler">Pointer to periodic timer handler to unregister</param>
void Timer::UnregisterPeriodicHandler(const PeriodicTimerHandler* handler)
{
    assert(handler != nullptr);
    assert(m_numPeriodicHandlers > 0);

    size_t index{};
    for (index = 0; index < TIMER_MAX_PERIODIC_HANDLERS; ++index)
    {
        if (m_periodicHandlers[index] == handler)
            break;
    }
    assert(index < TIMER_MAX_PERIODIC_HANDLERS);
    m_periodicHandlers[index] = nullptr;

    DataSyncBarrier();

    m_numPeriodicHandlers--;
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
    auto& timer = GetTimer();
    uint64 startTime = timer.GetSystemTimer();
    uint64 currentTime{};
    // Loop while counter elapse time is less than wait
    // Careful: busy wait
    do
    {
        currentTime = timer.GetSystemTimer();
    } while (currentTime - startTime < usec);
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
/// Interrupt handler for the timer
///
/// Sets the next timer deadline, increments the timer tick count, as well as the time if needed, and calls the periodic handlers.
/// </summary>
void Timer::InterruptHandler()
{
    uint64 compareValue;
    GetTimerCompareValue(compareValue);
    SetTimerCompareValue(compareValue + m_clockTicksPerSystemTick);

    if (++m_ticks % TICKS_PER_SECOND == 0)
    {
        m_upTime++;
        m_time++;
    }

    for (unsigned i = 0; i < m_numPeriodicHandlers; i++)
    {
        if (m_periodicHandlers[i] != nullptr)
            (*m_periodicHandlers[i])();
    }
}

/// <summary>
/// Static interrupt handler
///
/// Calls the instance interrupt handler
/// </summary>
/// <param name="param"></param>
void Timer::InterruptHandler(void* param)
{
    Timer* instance = reinterpret_cast<Timer*>(param);
    assert(instance != nullptr);

    instance->InterruptHandler();
}

/// <summary>
/// Retrieves the singleton Timer instance. It is created in the first call to this function.
/// </summary>
/// <returns>A reference to the singleton Timer</returns>
Timer& baremetal::GetTimer()
{
    static Timer timer;
    timer.Initialize();
    return timer;
}
