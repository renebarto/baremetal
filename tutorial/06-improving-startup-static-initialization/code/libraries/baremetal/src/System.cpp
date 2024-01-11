//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

#include <baremetal/System.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/SysConfig.h>
#include <baremetal/UART1.h>
#include <baremetal/Util.h>

using namespace baremetal;

System& baremetal::GetSystem()
{
    static System value;
    return value;
}

System::System()
{
}

void System::Halt()
{
    GetUART1().WriteString("Halt\n");

    // power off the SoC (GPU + CPU)
    auto r = *(RPI_PWRMGT_RSTS);
    r &= ~0xFFFFFAAA;
    r |= 0x555; // partition 63 used to indicate halt
    *(RPI_PWRMGT_RSTS) = (RPI_PWRMGT_WDOG_MAGIC | r);
    *(RPI_PWRMGT_WDOG) = (RPI_PWRMGT_WDOG_MAGIC | 10);
    *(RPI_PWRMGT_RSTC) = (RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_FULLRST);

    for (;;) // Satisfy [[noreturn]]
    {
        DataSyncBarrier();
        WaitForInterrupt();
    }
}

void System::Reboot()
{
    GetUART1().WriteString("Reboot\n");

    DisableIRQs();
    DisableFIQs();

    // power off the SoC (GPU + CPU)
    auto r = *(RPI_PWRMGT_RSTS);
    r &= ~0xFFFFFAAA;
    *(RPI_PWRMGT_RSTS) = (RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
    *(RPI_PWRMGT_WDOG) = (RPI_PWRMGT_WDOG_MAGIC | 10);
    *(RPI_PWRMGT_RSTC) = (RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_FULLRST);

    for (;;) // Satisfy [[noreturn]]
    {
        DataSyncBarrier();
        WaitForInterrupt();
    }
}

#ifdef __cplusplus
extern "C"
{
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

    // Call constructors of static objects
    extern void (*__init_start)(void);
    extern void (*__init_end)(void);
    for (void (**func)(void) = &__init_start; func < &__init_end; func++)
    {
        (**func)();
    }

    GetUART1().WriteString("Starting up\n");

    extern int main();

    if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
    {
        GetSystem().Reboot();
    }

    GetSystem().Halt();
}

#ifdef __cplusplus
}
#endif
