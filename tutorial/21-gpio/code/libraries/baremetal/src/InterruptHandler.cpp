//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : InterruptHandler.cpp
//
// Namespace   : baremetal
//
// Class       : InterruptSystem
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

#include "baremetal/InterruptHandler.h"

#include "baremetal/ARMInstructions.h"
#include "baremetal/ARMRegisters.h"
#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Interrupts.h"
#include "baremetal/Logger.h"
#include "baremetal/MemoryAccess.h"
#include "stdlib/Util.h"

/// @file
/// Interrupt handler function implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("InterruptHandler");

/// <summary>
/// Global interrupt handler function
///
/// Is called by the vector table, and relays the call to the singleton InterruptHandler instance
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
    : m_isInitialized{}
    , m_memoryAccess{GetMemoryAccess()}
    , m_irqHandlers{}
    , m_irqHandlersParam{}
{
}

/// <summary>
/// Constructs a specialized InterruptSystem instance which injects a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
InterruptSystem::InterruptSystem(IMemoryAccess& memoryAccess)
    : m_isInitialized{}
    , m_memoryAccess{memoryAccess}
    , m_irqHandlers{}
    , m_irqHandlersParam{}
{
}

/// <summary>
/// Destructor/// </summary>
InterruptSystem::~InterruptSystem()
{
    Shutdown();
}

/// <summary>
/// Initialize interrupt system
/// </summary>
void InterruptSystem::Initialize()
{
    if (m_isInitialized)
        return;

    memset(m_irqHandlers, 0, IRQ_LINES * sizeof(IRQHandler*));
    memset(m_irqHandlersParam, 0, IRQ_LINES * sizeof(void*));

    DisableInterrupts();

    EnableIRQs();

#if BAREMETAL_RPI_TARGET == 3
#else
    // direct all interrupts to core 0 with default priority
    for (unsigned n = 0; n < IRQ_LINES / 4; n++)
    {
        m_memoryAccess.Write32(RPI_GICD_IPRIORITYR0 + 4 * n, RPI_GICD_IPRIORITYR_DEFAULT | RPI_GICD_IPRIORITYR_DEFAULT << 8 | RPI_GICD_IPRIORITYR_DEFAULT << 16 | RPI_GICD_IPRIORITYR_DEFAULT << 24);

        m_memoryAccess.Write32(RPI_GICD_ITARGETSR0 + 4 * n, RPI_GICD_ITARGETSR_CORE0 | RPI_GICD_ITARGETSR_CORE0 << 8 | RPI_GICD_ITARGETSR_CORE0 << 16 | RPI_GICD_ITARGETSR_CORE0 << 24);
    }

    // set all interrupts to level triggered
    for (unsigned n = 0; n < IRQ_LINES / 16; n++)
    {
        m_memoryAccess.Write32(RPI_GICD_ICFGR0 + 4 * n, 0);
    }

    // initialize core 0 CPU interface:

    m_memoryAccess.Write32(RPI_GICC_PMR, RPI_GICC_PMR_PRIORITY);
#endif

    EnableInterrupts();

    m_isInitialized = true;
}

/// <summary>
/// Shutdown interrupt system, disable all
/// </summary>
void InterruptSystem::Shutdown()
{
    if (Logger::HaveLogger())
        LOG_INFO("InterruptSystem::Shutdown");
    DisableIRQs();

    DisableInterrupts();
    m_isInitialized = false;
}

/// <summary>
/// Disable all IRQ interrupts
/// </summary>
void InterruptSystem::DisableInterrupts()
{
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::DisableInterrupts");
#if BAREMETAL_RPI_TARGET == 3
    m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);

    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
    m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
    m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
#else
    // initialize distributor:

    m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
    m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_DISABLE);
    // disable, acknowledge and deactivate all interrupts
    for (unsigned n = 0; n < IRQ_LINES / 32; n++)
    {
        m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
        m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
        m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
    }
#endif
}

/// <summary>
/// Enable IRQ interrupts
/// </summary>
void InterruptSystem::EnableInterrupts()
{
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::EnableInterrupts");
#if BAREMETAL_RPI_TARGET == 3
#else
    m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
    m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
#endif
}

/// <summary>
/// Enable and register an IRQ handler
///
/// Enable the IRQ with specified index, and register its handler.
/// </summary>
/// <param name="irqID">IRQ ID</param>
/// <param name="handler">Handler to register for this IRQ</param>
/// <param name="param">Parameter to pass to IRQ handler</param>
void InterruptSystem::RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param)
{
    uint32 irq = static_cast<int>(irqID);
    assert(irq < IRQ_LINES);
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::RegisterIRQHandler IRQ=%d", irq);
    assert(m_irqHandlers[irq] == nullptr);

    EnableIRQ(irqID);

    m_irqHandlers[irq] = handler;
    m_irqHandlersParam[irq] = param;
}

/// <summary>
/// Disable and unregister an IRQ handler
///
/// Disable the IRQ with specified index, and unregister its handler.
/// </summary>
/// <param name="irqID">IRQ ID</param>
void InterruptSystem::UnregisterIRQHandler(IRQ_ID irqID)
{
    uint32 irq = static_cast<int>(irqID);
    assert(irq < IRQ_LINES);
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::UnregisterIRQHandler IRQ=%d", irq);
    assert(m_irqHandlers[irq] != nullptr);

    m_irqHandlers[irq] = nullptr;
    m_irqHandlersParam[irq] = nullptr;

    DisableIRQ(irqID);
}

/// <summary>
/// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
/// </summary>
/// <param name="fiqID">FIQ interrupt number</param>
/// <param name="handler">FIQ interrupt handler</param>
/// <param name="param">FIQ interrupt data</param>
// cppcheck-suppress unusedFunction
void InterruptSystem::RegisterFIQHandler(FIQ_ID fiqID, FIQHandler* handler, void* param)
{
    uint32 fiq = static_cast<int>(fiqID);
    assert(fiq <= IRQ_LINES);
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::RegisterFIQHandler IRQ=%d", fiq);
    assert(handler != nullptr);
    assert(s_fiqData.handler == nullptr);

    s_fiqData.handler = handler;
    s_fiqData.param = param;
    s_fiqData.fiqID = fiq;

    EnableFIQ(fiqID);
}

/// <summary>
/// Disable and unregister a FIQ interrupt handler
/// </summary>
/// <param name="fiqID">FIQ interrupt number</param>
void InterruptSystem::UnregisterFIQHandler(FIQ_ID fiqID)
{
    uint32 fiq = static_cast<int>(fiqID);
    assert(s_fiqData.handler != nullptr);
    assert(s_fiqData.fiqID == fiq);
    if (Logger::HaveLogger())
        LOG_DEBUG("InterruptSystem::UnregisterFIQHandler IRQ=%d", fiq);
    DisableFIQ(fiqID);

    s_fiqData.handler = nullptr;
    s_fiqData.param = nullptr;
}

/// <summary>
/// Handles an interrupt.
///
/// The interrupt handler is called from assembly code (ExceptionStub.S)
/// </summary>
void InterruptSystem::InterruptHandler()
{
#if BAREMETAL_RPI_TARGET == 3
    uint32 localpendingIRQs = m_memoryAccess.Read32(ARM_LOCAL_IRQ_PENDING0);
    if (localpendingIRQs & ARM_LOCAL_INTSRC_TIMER1) // the only implemented local IRQ so far
    {
        CallIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);

        return;
    }

    uint32 pendingIRQs[ARM_IRQS_NUM_REGS];
    pendingIRQs[0] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_1);
    pendingIRQs[1] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_2);
    pendingIRQs[2] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_BASIC_PENDING) & 0xFF; // Only 8 basic interrupts

    for (unsigned reg = 0; reg < ARM_IRQS_NUM_REGS; reg++)
    {
        uint32 pendingIRQ = pendingIRQs[reg];
        if (pendingIRQ != 0)
        {
            unsigned irqID = reg * ARM_IRQS_PER_REG;

            do
            {
                if ((pendingIRQ & 1) && CallIRQHandler(static_cast<IRQ_ID>(irqID)))
                {
                    return;
                }

                pendingIRQ >>= 1;
                irqID++;
            } while (pendingIRQ != 0);
        }
    }

#else

    uint32 iarValue = m_memoryAccess.Read32(RPI_GICC_IAR); // Read Interrupt Acknowledge Register

    uint32 irq = iarValue & RPI_GICC_IAR_INTERRUPT_ID_MASK; // Select the currently active interrupt
    if (irq < IRQ_LINES)
    {
        if (irq >= GIC_PPI(0))
        {
            // Peripheral interrupts (PPI and SPI)
            CallIRQHandler(static_cast<IRQ_ID>(irq));
        }
        else
        {
            // Handle SGI interrupt
        }
        m_memoryAccess.Write32(RPI_GICC_EOIR, iarValue); // Flag end of interrupt
    }
#ifndef NDEBUG
    else
    {
        // spurious interrupts
        assert(irq >= 1020);
        LOG_INFO("Received spurious interrupt %d", iarValue);
    }
#endif

#endif
}

/// <summary>
/// Call the IRQ handler for the specified IRQ ID
/// </summary>
/// <param name="irqID">ID of the IRQ</param>
/// <returns>True if a IRQ handler was found, false if not</returns>
bool InterruptSystem::CallIRQHandler(IRQ_ID irqID)
{
    uint32 irq = static_cast<int>(irqID);
    assert(irq < IRQ_LINES);
    IRQHandler* handler = m_irqHandlers[irq];

    if (handler != nullptr)
    {
        (*handler)(m_irqHandlersParam[irq]);

        return true;
    }
#ifndef NDEBUG
    LOG_INFO("Unhandled interrupt %d", irq);
#endif

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
    static InterruptSystem singleton;
    singleton.Initialize();
    return singleton;
}
