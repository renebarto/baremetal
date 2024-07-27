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

#include <baremetal/Assert.h>

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
    : m_irqHandler{}
    , m_irqHandlerParams{}
{
}

/// <summary>
/// Destructor
/// </summary>
InterruptSystem::~InterruptSystem()
{
}

/// <summary>
/// Initialize interrupt system
/// </summary>
void InterruptSystem::Initialize()
{
}

/// <summary>
/// Register an IRQ handler
/// </summary>
/// <param name="handler">Handler to register</param>
/// <param name="param">Parameter to pass to IRQ handler</param>
void InterruptSystem::RegisterIRQHandler(IRQHandler* handler, void* param)
{
    assert(m_irqHandler == nullptr);

    m_irqHandler = handler;
    m_irqHandlerParams = param;
}

/// <summary>
/// Unregister an IRQ handler
/// </summary>
void InterruptSystem::UnregisterIRQHandler()
{
    assert(m_irqHandler != nullptr);

    m_irqHandler = nullptr;
    m_irqHandlerParams = nullptr;
}

/// <summary>
/// Handles an interrupt.
///
/// The interrupt handler is called from assembly code (ExceptionStub.S)
/// </summary>
void InterruptSystem::InterruptHandler()
{
    if (m_irqHandler)
        m_irqHandler(m_irqHandlerParams);
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
