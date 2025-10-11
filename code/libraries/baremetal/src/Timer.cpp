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
#include "baremetal/Format.h"
#include "baremetal/InterruptHandler.h"
#include "baremetal/Logger.h"
#include "baremetal/MemoryAccess.h"
#include "stdlib/Util.h"

/// @file
/// Raspberry Pi Timer implementation

namespace baremetal {

/// @brief Define log name
LOG_MODULE("Timer");

/// @brief Magic number for kernel timer (KTMC)
#define KERNEL_TIMER_MAGIC 0x4B544D43

/// <summary>
/// Kernel timer administration
/// </summary>
/// <typeparam name="Pointer"></typeparam>
struct KernelTimer
{
#ifndef NDEBUG
    /// @brief Magic number to check if element is valid
    unsigned m_magic;
#endif
    /// @brief Kernel timer deadline in timer ticks
    unsigned m_elapsesAtTicks;
    /// @brief Pointer to kernel timer handler
    KernelTimerHandler* m_handler;
    /// @brief Kernel timer handler parameter
    void* m_param;
    /// @brief Kernel timer handler context
    void* m_context;

    /// <summary>
    /// Construct a kernel timer administration element
    /// </summary>
    /// <param name="elapseTimeTicks">Timer deadline in timer ticks</param>
    /// <param name="handler">Kernel timer handler pointer</param>
    /// <param name="param">Kernel timer handler parameter</param>
    /// <param name="context">Kernerl timer handler context</param>
    KernelTimer(unsigned elapseTimeTicks, KernelTimerHandler* handler, void* param, void* context)
        :
#ifndef NDEBUG
        m_magic{KERNEL_TIMER_MAGIC}
        ,
#endif
        m_elapsesAtTicks{elapseTimeTicks}
        , m_handler{handler}
        , m_param{param}
        , m_context{context}

    {
    }
    /// <summary>
    /// Verify magic number
    /// </summary>
    /// <returns>True if the magic number is correct, false otherwise</returns>
    bool CheckMagic() const
    {
        return m_magic == KERNEL_TIMER_MAGIC;
    }
};
/// @brief Kernel timer element, element which is stored in the kernel time list
using KernelTimerElement = DoubleLinkedList<KernelTimer*>::Element;

const unsigned Timer::s_daysInMonth[12]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const char* Timer::s_monthName[12]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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
    , m_kernelTimerList{}
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
    , m_kernelTimerList{}
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

    KernelTimerElement* element;
    while ((element = m_kernelTimerList.GetFirst()) != 0)
    {
        CancelKernelTimer(reinterpret_cast<KernelTimerHandle>(m_kernelTimerList.GetPointer(element)));
    }
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
/// The current time will be in the format "MMM dd HH:MM:SS.mmm", according to our time zone, if the time is valid.
/// If the time is not known yet, it we be the uptime in the format "ddd.HH:MM:SS.mmm".
/// If not yet initialized, an empty string is returned
/// </summary>
/// <param name="buffer">Buffer to write the time string to</param>
/// <param name="bufferSize">Size of the buffer</param>
void Timer::GetTimeString(char* buffer, size_t bufferSize)
{
    uint64 time = m_time;
    uint64 ticks = m_ticks;

    if (bufferSize == 0)
    {
        return;
    }
    if (!m_isInitialized)
    {
        *buffer = '\0';
        return;
    }

    unsigned second = time % 60;
    time /= 60; // Time is now in minute
    unsigned minute = time % 60;
    time /= 60; // Time is now in hour
    unsigned hour = time % 24;
    time /= 24; // Time is now in days
    unsigned daysTotal = time;

    unsigned year = 1970; // Epoch start
    while (true)
    {
        unsigned daysInYear = IsLeapYear(year) ? 366 : 365;
        if (time < daysInYear)
        {
            break;
        }

        time -= daysInYear;
        year++;
    }

    unsigned month = 0;
    while (1)
    {
        unsigned daysInMonth = GetDaysInMonth(month, year);
        if (time < daysInMonth)
        {
            break;
        }

        time -= daysInMonth;
        month++;
    }

    unsigned monthDay = time + 1;

#if (TICKS_PER_SECOND != MSEC_PER_SEC)
    ticks = ticks * MSEC_PER_SEC / TICKS_PER_SECOND;
#endif
    auto milliSeconds = ticks % MSEC_PER_SEC;

    if (year > 1975) // Just a sanity check to see if we have an actual time
    {
        FormatNoAlloc(buffer, bufferSize, "%s %2u, %04u %02u:%02u:%02u.%03u", s_monthName[month], monthDay, year, hour, minute, second, milliSeconds);
    }
    else
    {
        FormatNoAlloc(buffer, bufferSize, "%u.%02u:%02u:%02u.%03u", daysTotal, hour, minute, second, milliSeconds);
    }
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
/// Starts a kernel timer. After delayTicks timer ticks, it elapses and call the kernel timer handler.
/// </summary>
/// <param name="delayTicks">Delay time for timer in timer ticks</param>
/// <param name="handler">Kernel timer handler to call when time elapses</param>
/// <param name="param">Parameter to pass to kernel timer handler</param>
/// <param name="context">Kernel timer handler context</param>
/// <returns>Handle to kernel timer</returns>
KernelTimerHandle Timer::StartKernelTimer(unsigned delayTicks, KernelTimerHandler* handler, void* param, void* context)
{
    unsigned elapseTimeTicks = m_ticks + delayTicks;
    assert(handler != nullptr);

    KernelTimer* timer = new KernelTimer(elapseTimeTicks, handler, param, context);
    assert(timer != nullptr);
    LOG_DEBUG("Create new timer to expire at %d ticks, handle %p", elapseTimeTicks, timer);

    KernelTimerElement* prevElement{};
    KernelTimerElement* element = m_kernelTimerList.GetFirst();
    while (element != nullptr)
    {
        const KernelTimer* timer2 = m_kernelTimerList.GetPointer(element);
        assert(timer2 != nullptr);
        assert(timer2->m_magic == KERNEL_TIMER_MAGIC);

        if (static_cast<int>(timer2->m_elapsesAtTicks - elapseTimeTicks) > 0)
        {
            break;
        }

        prevElement = element;
        element = m_kernelTimerList.GetNext(element);
    }

    if (element != nullptr)
    {
        m_kernelTimerList.InsertBefore(element, timer);
    }
    else
    {
        m_kernelTimerList.InsertAfter(prevElement, timer);
    }

    return reinterpret_cast<KernelTimerHandle>(timer);
}

/// <summary>
/// Cancels and removes a kernel timer.
/// </summary>
/// <param name="handle">Handle to kernel timer to cancel</param>
void Timer::CancelKernelTimer(KernelTimerHandle handle)
{
    KernelTimer* timer = reinterpret_cast<KernelTimer*>(handle);
    assert(timer != 0);
    LOG_DEBUG("Cancel timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);

    KernelTimerElement* element = m_kernelTimerList.Find(timer);
    if (element != nullptr)
    {
        assert(timer->m_magic == KERNEL_TIMER_MAGIC);

        m_kernelTimerList.Remove(element);

#ifndef NDEBUG
        timer->m_magic = 0;
#endif
        delete timer;
    }
}

/// <summary>
/// Update all registered kernel timers, and handle expiration of timers
/// </summary>
void Timer::PollKernelTimers()
{
    auto element = m_kernelTimerList.GetFirst();
    while (element != nullptr)
    {
        KernelTimer* timer = m_kernelTimerList.GetPointer(element);
        assert(timer != nullptr);
        assert(timer->m_magic == KERNEL_TIMER_MAGIC);

        if (static_cast<int>(timer->m_elapsesAtTicks - m_ticks) > 0)
        {
            break;
        }

        LOG_DEBUG("Expire timer, expire time %d ticks, handle %p", timer->m_elapsesAtTicks, timer);

        m_kernelTimerList.Remove(element);

        KernelTimerHandler* handler = timer->m_handler;
        assert(handler != nullptr);
        (*handler)(reinterpret_cast<KernelTimerHandle>(timer), timer->m_param, timer->m_context);

#ifndef NDEBUG
        timer->m_magic = 0;
#endif
        delete timer;

        // The list may have changed due to the handler callback, so re-initialize
        element = m_kernelTimerList.GetFirst();
    }
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
/// Determine if the specified year is a leap year
/// </summary>
/// <param name="year">Year</param>
/// <returns>Returns true if year is a leap year, false otherwise</returns>
bool Timer::IsLeapYear(unsigned year)
{
    if (year % 100 == 0)
    {
        return year % 400 == 0;
    }

    return year % 4 == 0;
}

/// <summary>
/// Calculates the number days in the specified month of the specified year
/// </summary>
/// <param name="month">Month, 0=January, 1=February, etc.</param>
/// <param name="year">Year</param>
/// <returns></returns>
unsigned Timer::GetDaysInMonth(unsigned month, unsigned year)
{
    if (month == 1 && IsLeapYear(year))
    {
        return 29;
    }

    return s_daysInMonth[month];
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

    PollKernelTimers();

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
Timer& GetTimer()
{
    static Timer timer;
    timer.Initialize();
    return timer;
}

} // namespace baremetal