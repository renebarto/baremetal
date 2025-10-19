//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : System.cpp
//
// Namespace   : baremetal
//
// Class       : System
//
// Description : System initialization and process functions
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

#include "baremetal/System.h"

#include "baremetal/ARMInstructions.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/MemoryAccess.h"
#include "baremetal/MemoryManager.h"
#include "baremetal/SysConfig.h"
#include "baremetal/Timer.h"
#include "baremetal/UART0.h"
#include "baremetal/UART1.h"
#include "baremetal/Version.h"
#include "stdlib/Util.h"

/// @file
/// System startup / shutdown functionality implementation

using namespace baremetal;

/// @brief Define log name for this module
LOG_MODULE("System");

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// __dso_handle is a "guard" that is used to identify dynamic shared objects during global destruction. It is only known to the compiler / linker
/// </summary>
void* __dso_handle WEAK;

/// @brief WEAK version of __cxa_atexit
void __cxa_atexit(void (*func)(void* param), void* param, void* handle) WEAK;

/// <summary>
/// __cxa_atexit() will call the destructor of the static of a dynamic library when this dynamic library is unloaded before the program exits.
/// </summary>
/// <param name="func">Registered function to be called</param>
/// <param name="param">Parameter to be passed to registered function</param>
/// <param name="handle">Handle of the shared library to be unloaded (its __dso_handle)</param>
void __cxa_atexit([[maybe_unused]] void (*func)(void* param), [[maybe_unused]] void* param, [[maybe_unused]] void* handle)
{
}

#ifdef __cplusplus
}
#endif

/// @brief Wait time in milliseconds to ensure that UART info is written before system halt or reboot
static const uint32 WaitTime = 10;

/// <summary>
/// Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
/// </summary>
System::System()
    : m_memoryAccess{GetMemoryAccess()}
{
}

/// <summary>
/// Constructs a specialized System instance with a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Memory access interface</param>
System::System(IMemoryAccess& memoryAccess)
    : m_memoryAccess{memoryAccess}
{
}

/// <summary>
/// Halts the system. This function will not return
/// </summary>
void System::Halt()
{
    LOG_INFO("Halt");
    Timer::WaitMilliSeconds(WaitTime);

    // power off the SoC (GPU + CPU)
    auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
    r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
    r |= RPI_PWRMGT_PARTITIONVALUE(63); // Partition 63 used to indicate halt
    m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r);
    m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_TIMER_SECONDS(1));
    m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);

    for (;;) // Satisfy [[noreturn]]
    {
        DataSyncBarrier();
        WaitForInterrupt();
    }
}

/// <summary>
/// Reboots the system. This function will not return
/// </summary>
void System::Reboot()
{
    LOG_INFO("Reboot");
    Timer::WaitMilliSeconds(WaitTime);

    DisableIRQs();
    DisableFIQs();

    // power off the SoC (GPU + CPU)
    auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
    r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
    m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
    m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_TIMER_SECONDS(5));
    m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);

    for (;;) // Satisfy [[noreturn]]
    {
        DataSyncBarrier();
        WaitForInterrupt();
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void sysinit()
{
    EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
    EnableIRQs(); // go to TASK_LEVEL

    // clear BSS
    extern unsigned char __bss_start;
    extern unsigned char __bss_end;
    memset(&__bss_start, 0, &__bss_end - &__bss_start);

    // halt, if KERNEL_MAX_SIZE is not properly set
    // cannot inform the user here
    if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
    {
        GetSystem().Halt();
    }

    // We need to create a memory system first, as core 0 needs to be the first to enable the MMU.
    // Will trace, however, there is no instantiated device connected to console yet.
    GetMemoryManager();

    // Call constructors of static objects
    extern void (*__init_start)(void);
    extern void (*__init_end)(void);
    for (void (**func)(void) = &__init_start; func < &__init_end; func++)
    {
        (**func)();
    }

    Device* logDevice{};
#if defined(BAREMETAL_CONSOLE_UART0)
    auto& uart = GetUART0();
    uart.Initialize(115200);
    uart.WriteString("\nSetting up UART0\n");
    logDevice = &uart;
#elif defined(BAREMETAL_CONSOLE_UART1)
    auto& uart = GetUART1();
    uart.Initialize(115200);
    uart.WriteString("\nSetting up UART1\n");
    logDevice = &uart;
#endif
    GetConsole().AssignDevice(logDevice);
    GetLogger().Initialize();

    LOG_INFO("Starting up");

    extern int main();

    if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
    {
#if BAREMETAL_MEMORY_TRACING
        GetMemoryManager().DumpStatus();
#endif
        GetSystem().Reboot();
    }

#if BAREMETAL_MEMORY_TRACING
    GetMemoryManager().DumpStatus();
#endif
    GetSystem().Halt();
}

#ifdef __cplusplus
}
#endif

/// <summary>
/// Construct the singleton system handler if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton system handler</returns>
System& baremetal::GetSystem()
{
    static System value;
    return value;
}
