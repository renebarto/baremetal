//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Interrupts.cpp
//
// Namespace   : baremetal
//
// Class       : Interrupts
//
// Description : Interrupt numbers and enable / disable functionality
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

#include "baremetal/Interrupts.h"

#include "baremetal/ARMRegisters.h"
#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/MemoryAccess.h"

/// @file
/// Interrupt numbers and enable / disable functionality

/// @brief Define log name
LOG_MODULE("Interrupts");

/// @brief Convert irq number to bit pattern for register on GIC (Raspberry Pi 4/5)
#define RPI_GICD_IRQ_MASK(irq) BIT1(irq % 32)

/// <summary>
/// Enable the requested IRQ
/// </summary>
/// <param name="irqID">IRQ to enable</param>
void baremetal::EnableIRQ(IRQ_ID irqID)
{
    int irq = static_cast<int>(irqID);
    assert(irq < IRQ_LINES);
    MemoryAccess mem;
#if BAREMETAL_RPI_TARGET == 3

    if (irq < ARM_IRQ_LOCAL_BASE)
    {
        mem.Write32(ARM_IC_IRQS_ENABLE(irq), ARM_IRQ_MASK(irq));
    }
    else
    {
        // The only implemented local IRQs so far
        assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
        mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(irq - ARM_IRQ_LOCAL_BASE));
    }

#else

    mem.Write32(RPI_GICD_ISENABLER0 + 4 * (irq / 32), RPI_GICD_IRQ_MASK(irq));

#endif
}

/// <summary>
/// Disable the requested IRQ
/// </summary>
/// <param name="irqID">IRQ to disable</param>
void baremetal::DisableIRQ(IRQ_ID irqID)
{
    int irq = static_cast<int>(irqID);
    assert(irq < IRQ_LINES);
    MemoryAccess mem;
#if BAREMETAL_RPI_TARGET == 3

    if (irq < ARM_IRQ_LOCAL_BASE)
    {
        mem.Write32(ARM_IC_IRQS_DISABLE(irq), ARM_IRQ_MASK(irq));
    }
    else
    {
        // The only implemented local IRQs so far
        assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
        mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(irq - ARM_IRQ_LOCAL_BASE));
    }

#else

    mem.Write32(RPI_GICD_ICENABLER0 + 4 * (irq / 32), RPI_GICD_IRQ_MASK(irq));

#endif
}

/// <summary>
/// Enable the requested FIQ
/// </summary>
/// <param name="fiqID">FIQ to enable</param>
void baremetal::EnableFIQ(FIQ_ID fiqID)
{
    int fiq = static_cast<int>(fiqID);
    assert(fiq <= IRQ_LINES);
    MemoryAccess mem;
#if BAREMETAL_RPI_TARGET == 3

    if (fiq < ARM_IRQ_LOCAL_BASE)
    {
        mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, fiq | BIT1(7));
    }
    else
    {
        // The only implemented local IRQs so far
        assert(fiqID == FIQ_ID::FIQ_LOCAL_CNTPNS);
        mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
                    mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(fiq - ARM_IRQ_LOCAL_BASE + 4)); // FIQ enable bits are bit 4..7
    }

#else

    LOG_PANIC("FIQ not supported yet");

#endif
}

/// <summary>
/// Disable the requested FIQ
/// </summary>
/// <param name="fiqID">FIQ to disable</param>
void baremetal::DisableFIQ(FIQ_ID fiqID)
{
    int fiq = static_cast<int>(fiqID);
    assert(fiq <= IRQ_LINES);
    MemoryAccess mem;
#if BAREMETAL_RPI_TARGET == 3

    if (fiq < ARM_IRQ_LOCAL_BASE)
    {
        mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
    }
    else
    {
        // The only implemented local IRQs so far
        assert(fiqID == FIQ_ID::FIQ_LOCAL_CNTPNS);
        mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
                    mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(fiq - ARM_IRQ_LOCAL_BASE + 4)); // FIQ enable bits are bit 4..7
    }

#else

    LOG_PANIC("FIQ not supported yet");

#endif
}
