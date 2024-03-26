//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : InterruptHandler.h
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

#pragma once

#include <baremetal/Interrupt.h>
#include <baremetal/Macros.h>
#include <baremetal/Types.h>

/// @file
/// Interrupt handler function

#ifdef __cplusplus
extern "C" {
#endif

void InterruptHandler();

#ifdef __cplusplus
}
#endif

namespace baremetal {

/// <summary>
/// FIQ handler function
/// </summary>
using FIQHandler = void(void* param);

/// <summary>
/// IRQ handler function
/// </summary>
using IRQHandler = void(void* param);

/// <summary>
/// FIQ data
///
/// This is data stored in Exceptions.S
/// </summary>
struct FIQData
{
    /// @brief FIQ handler
    FIQHandler* handler;
    /// @brief Parameter to pass to registered FIQ handler
    void*       param;
    /// @brief ID of FIQ
    uint32      fiqID;
} PACKED;

class IMemoryAccess;

/// <summary>
/// InterruptSystem: Handles IRQ and FIQ interrupts for Raspberry Pi
/// This is a SINGLETON class
/// </summary>
class InterruptSystem
{
    /// @brief True if class is already initialized
    bool m_initialized;
    /// @brief Pointer to registered IRQ handler for each IRQ
    IRQHandler* m_irqHandlers[IRQ_LINES];
    /// @brief Parameter to pass to registered IRQ handler
    void* m_irqHandlersParams[IRQ_LINES];
    /// @brief Memory access interface
    IMemoryAccess& m_memoryAccess;

    /// <summary>
    /// Construct the singleton InterruptSystem instance if needed, and return a reference to the instance. This is a friend function of class InterruptSystem
    /// </summary>
    /// <returns>Reference to the singleton InterruptSystem instance</returns>
    friend InterruptSystem& GetInterruptSystem();

private:

    /// @brief Create a interrupt system. Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
    InterruptSystem();

public:
    InterruptSystem(IMemoryAccess& memoryAccess);
    ~InterruptSystem();

    void Initialize();

    void DisableInterrupts();

    void RegisterIRQHandler(unsigned irqID, IRQHandler* handler, void* param);
    void UnregisterIRQHandler(unsigned irqID);

    void RegisterFIQHandler(unsigned fiqID, FIQHandler* handler, void* param);
    void UnregisterFIQHandler();

    static void EnableIRQ(unsigned irqID);
    static void DisableIRQ(unsigned irqID);

    static void EnableFIQ(unsigned fiqID);
    static void DisableFIQ();

    void InterruptHandler();

private:
    bool CallIRQHandler(unsigned irqID);
};

InterruptSystem& GetInterruptSystem();

} // namespace baremetal

/// @brief FIQ administration, see Exception.S
extern baremetal::FIQData s_fiqData;
