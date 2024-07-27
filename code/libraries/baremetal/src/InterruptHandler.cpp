//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : InterruptHandler.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Interrupt handler
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

#include <baremetal/InterruptHandler.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/ARMRegisters.h>
#include <baremetal/Assert.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Interrupt handler function implementation

using namespace baremetal;

/// <summary>
/// Global interrupt handler callback
/// 
/// This is the entry point called by ExceptionStub.S, amd will relay the call to the InterruptSystem singleton instance
/// </summary>
void InterruptHandler()
{
    GetInterruptSystem().InterruptHandler();
}

/// <summary>
/// Create a interrupt system
///
/// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
/// </summary>
InterruptSystem::InterruptSystem()
    : m_initialized{}
    , m_irqHandlers{}
    , m_irqHandlersParams{}
    , m_memoryAccess{ GetMemoryAccess() }
{
}

/// <summary>
/// Constructs a specialized InterruptSystem instance which injects a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
InterruptSystem::InterruptSystem(IMemoryAccess& memoryAccess)
    : m_initialized{}
    , m_irqHandlers{}
    , m_irqHandlersParams{}
    , m_memoryAccess{ memoryAccess }
{
}

/// <summary>
/// Destructor
/// </summary>
InterruptSystem::~InterruptSystem()
{
    DisableIRQs();

    DisableInterrupts();
}

/// <summary>
/// Initialize interrupt system
/// </summary>
void InterruptSystem::Initialize()
{
    if (m_initialized)
        return;
    for (unsigned irqID = 0; irqID < IRQ_LINES; irqID++)
    {
        m_irqHandlers[irqID] = nullptr;
        m_irqHandlersParams[irqID] = nullptr;
    }

    DisableInterrupts();

    EnableIRQs();

    m_initialized = true;
}

/// <summary>
/// Disable all IRQ interrupts
/// </summary>
void InterruptSystem::DisableInterrupts()
{
    m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);

    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
    m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
}

/// <summary>
/// Enable and register an IRQ handler
/// 
/// Enable the IRQ with specified index, and register its handler.
/// IRQ are divided into groups:
/// - IRQ1: 0..31 (System Timer, GPU, DMA, ...)
/// - IRQ2: 32..63 (HDMI, CAM, GPIO, SPI, I2C, I2S, RNG, ...)
/// - IRQ_Basic: 64..71 (ARM standard interrupts)
/// - IRQ_ARM_LOcal: 72..83 (ARM local interrupts)
/// </summary>
/// <param name="irqID">IRQ number 0..83</param>
/// <param name="handler">Handler to register for this IRQ</param>
/// <param name="param">Parameter to pass to IRQ handler</param>
void InterruptSystem::RegisterIRQHandler(unsigned irqID, IRQHandler* handler, void* param)
{
    assert(irqID < IRQ_LINES);
    assert(m_irqHandlers[irqID] == nullptr);

    m_irqHandlers[irqID] = handler;
    m_irqHandlersParams[irqID] = param;

    EnableIRQ(irqID);
}

/// <summary>
/// Disable and unregister an IRQ handler
/// 
/// Disable the IRQ with specified index, and unregister its handler.
/// </summary>
/// <param name="irqID">IRQ number 0..83</param>
void InterruptSystem::UnregisterIRQHandler(unsigned irqID)
{
    assert(irqID < IRQ_LINES);
    assert(m_irqHandlers[irqID] != nullptr);

    DisableIRQ(irqID);

    m_irqHandlers[irqID] = nullptr;
    m_irqHandlersParams[irqID] = nullptr;
}

/// <summary>
/// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
/// </summary>
/// <param name="fiqID">FIQ interrupt number</param>
/// <param name="handler">FIQ interrupt handler</param>
/// <param name="param">FIQ interrupt data</param>
// cppcheck-suppress unusedFunction
void InterruptSystem::RegisterFIQHandler(unsigned fiqID, FIQHandler *handler, void *param)
{
    assert(fiqID <= BCM_MAX_FIQ);
    assert(handler != nullptr);
    assert(s_fiqData.handler == nullptr);

    s_fiqData.handler = handler;
    s_fiqData.param   = param;

    EnableFIQ(fiqID);
}

/// <summary>
/// Disable and unregister a FIQ interrupt handler
/// </summary>
void InterruptSystem::UnregisterFIQHandler()
{
    assert(s_fiqData.handler != nullptr);

    DisableFIQ();

    s_fiqData.handler = nullptr;
    s_fiqData.param   = nullptr;
}

/// <summary>
/// Enable an IRQ interrupt. And interrupt cannot be chained, so only one handler can be connected
/// RPI3 has 64 IRQ + 8 basic IRQ + 12 local IRQ. The local IRQ are handled differently
/// RPI4 has 256 IRQ, and no local IRQ (not handled here yes, uses the GIC)
/// </summary>
/// <param name="irqID"></param>
void InterruptSystem::EnableIRQ(unsigned irqID)
{
    assert(irqID < IRQ_LINES);

    if (irqID < ARM_IRQLOCAL_BASE)
    {
        GetInterruptSystem().m_memoryAccess.Write32(ARM_IC_IRQS_ENABLE(irqID), ARM_IRQ_MASK(irqID));
    }
    else
    {
        assert(irqID == ARM_IRQLOCAL0_CNTPNS); // The only implemented local IRQ so far
        GetInterruptSystem().m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
            GetInterruptSystem().m_memoryAccess.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT(1));
    }
}

/// <summary>
/// Disable an IRQ interrupt.
/// RPI3 has 64 IRQ + 8 basic IRQ + 12 local IRQ. The local IRQ are handled differently
/// RPI4 has 256 IRQ, and no local IRQ (not handled here yes, uses the GIC)
/// </summary>
/// <param name="irqID"></param>
void InterruptSystem::DisableIRQ(unsigned irqID)
{
    assert(irqID < IRQ_LINES);

    if (irqID < ARM_IRQLOCAL_BASE)
    {
        GetInterruptSystem().m_memoryAccess.Write32(ARM_IC_IRQS_DISABLE(irqID), ARM_IRQ_MASK(irqID));
    }
    else
    {
        assert(irqID == ARM_IRQLOCAL0_CNTPNS); // The only implemented local IRQ so far
        GetInterruptSystem().m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
            GetInterruptSystem().m_memoryAccess.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT(1));
    }
}

/// <summary>
/// Enable a FIQ interrupt. Only one can be enabled at any time
/// </summary>
/// <param name="fiqID">FIQ interrupt number</param>
void InterruptSystem::EnableFIQ(unsigned fiqID)
{
    assert(fiqID <= BCM_MAX_FIQ);

    GetInterruptSystem().m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, fiqID | FIQ_INTR_ENABLE);
}

/// <summary>
/// Disabled the enabled FIQ interrupt (if any).
/// </summary>
void InterruptSystem::DisableFIQ()
{
    GetInterruptSystem().m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
}

/// <summary>
/// Handles an interrupt.
///
/// The interrupt handler is called from assembly code (ExceptionStub.S)
/// </summary>
void InterruptSystem::InterruptHandler()
{
    uint32 localpendingIRQs = m_memoryAccess.Read32(ARM_LOCAL_IRQ_PENDING0);
    assert(!(localpendingIRQs & ~(1 << 1 | 0xF << 4 | 1 << 8)));
    if (localpendingIRQs & (1 << 1)) // the only implemented local IRQ so far
    {
        CallIRQHandler(ARM_IRQLOCAL0_CNTPNS);

        return;
    }

    uint32 pendingIRQs[ARM_IC_IRQ_REGS];
    pendingIRQs[0] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_1);
    pendingIRQs[1] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_2);
    pendingIRQs[2] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_BASIC_PENDING) & 0xFF; // Only 8 basic interrupts

    for (unsigned reg = 0; reg < ARM_IC_IRQ_REGS; reg++)
    {
        uint32 pendingIRQ = pendingIRQs[reg];
        if (pendingIRQ != 0)
        {
            unsigned irqID = reg * BCM_IRQS_PER_REG;

            do
            {
                if ((pendingIRQ & 1) && CallIRQHandler(irqID))
                {
                    return;
                }

                pendingIRQ >>= 1;
                irqID++;
            } while (pendingIRQ != 0);
        }
    }
}

/// <summary>
/// Call the IRQ handler for the specified IRQ ID
/// </summary>
/// <param name="irqID">ID of the IRQ</param>
/// <returns>True if a IRQ handler was found, false if not</returns>
bool InterruptSystem::CallIRQHandler(unsigned irqID)
{
    assert(irqID < IRQ_LINES);
    IRQHandler* handler = m_irqHandlers[irqID];

    if (handler != nullptr)
    {
        (*handler)(m_irqHandlersParams[irqID]);

        return true;
    }

    DisableIRQ(irqID);

    return false;
}

/// <summary>
/// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
///
/// This is a friend function of class InterruptSystem
/// </summary>
/// <returns>Reference to the singleton interrupt system instance</returns>
InterruptSystem& baremetal::GetInterruptSystem()
{
    static InterruptSystem system;
    system.Initialize();
    return system;
}
