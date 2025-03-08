//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : PhysicalGPIOPin.h
//
// Namespace   : baremetal
//
// Class       : PhysicalGPIOPin
//
// Description : Physical GPIO pin
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

#include <baremetal/IGPIOPin.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Physical GPIO pin

namespace baremetal {

/// @brief GPIO function
enum class GPIOFunction;

/// @brief GPIO pull mode
enum class GPIOPullMode;

#if BAREMETAL_RPI_TARGET == 3
/// @brief Total count of GPIO pins, numbered from 0 through 53
#define NUM_GPIO 54
#else
/// @brief Total count of GPIO pins, numbered from 0 through 56
#define NUM_GPIO 57
#endif

/// @brief Interrupt type to enable
enum class GPIOInterruptTypes : uint8
{
    /// @brief Interrupt on rising edge
    RisingEdge = 0x01,
    /// @brief Interrupt on falling edge
    FallingEdge = 0x02,
    /// @brief Interrupt on low level
    HighLevel = 0x04,
    /// @brief Interrupt on high level
    LowLevel = 0x08,
    /// @brief Interrupt on asynchronous rising edge
    AsyncRisingEdge = 0x10,
    /// @brief Interrupt on asynchronous falling edge
    AsyncFallingEdge = 0x20,
    /// @brief No interrupts
    None = 0x00,
    /// @brief All interrupts
    All = 0x3F,
};
/// <summary>
/// Combine two GPIO pin interrupt types, by performing a binary or on the two bit patterns
/// </summary>
/// <param name="lhs">First GPIO pin interrupt type</param>
/// <param name="rhs">Second GPIO pin interrupt type</param>
/// <returns>Combined GPIO pin interrupt type</returns>
inline GPIOInterruptTypes operator | (GPIOInterruptTypes lhs, GPIOInterruptTypes rhs)
{
    return static_cast<GPIOInterruptTypes>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs));
}

/// <summary>
/// GPIO pin interrupt handler
/// </summary>
using GPIOPinInterruptHandler = void(IGPIOPin* pin, void *param);

/// <summary>
/// Physical GPIO pin (i.e. available on GPIO header) 
/// </summary>
class PhysicalGPIOPin
    : public IGPIOPin
{
private:
    /// @brief Configured GPIO pin number (0..53)
    uint8                    m_pinNumber;
    /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
    GPIOMode                 m_mode;
    /// @brief Configured GPIO function.
    GPIOFunction             m_function;
    /// @brief Configured GPIO pull mode (only for input function).
    GPIOPullMode             m_pullMode;
    /// @brief Current value of the GPIO pin (true for on, false for off).
    bool                     m_value;
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess&           m_memoryAccess;
    /// @brief Register offset for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
    unsigned                 m_regOffset;
    /// @brief Register mask for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
    uint32                   m_regMask;
    /// @brief Interrupt handler for the pin
    GPIOPinInterruptHandler* m_handler;
    /// @brief Interrupt handler parameter for the pin
    void*                    m_handlerParam;
    /// @brief Auto acknowledge interrupt for the pin. If true, the interrupt handler will of the GPIOManager will automatically reset the event state
    bool                     m_autoAcknowledge;
    /// @brief GPIO interrupt types enabled
    uint8                    m_interruptMask;

public:
    PhysicalGPIOPin(IMemoryAccess &memoryAccess = GetMemoryAccess());

    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess = GetMemoryAccess());

    uint8 GetPinNumber() const override;
    bool AssignPin(uint8 pinNumber) override;

    void On() override;
    void Off() override;
    bool Get() override;
    void Set(bool on) override;
    void Invert() override;
    bool GetEvent() override;
    void ClearEvent() override;

    GPIOMode GetMode();
    bool SetMode(GPIOMode mode);
    GPIOFunction GetFunction();
    GPIOPullMode GetPullMode();
    void SetPullMode(GPIOPullMode pullMode);

    bool GetAutoAcknowledgeInterrupt() const override;
    void AcknowledgeInterrupt() override;
    void InterruptHandler() override;

    void ConnectInterrupt(GPIOPinInterruptHandler *handler, void *param, bool autoAcknowledge = true);
    void DisconnectInterrupt();

    void EnableInterrupt(GPIOInterruptTypes interruptTypes);
    void DisableInterrupt(GPIOInterruptTypes interruptTypes);
    void DisableAllInterrupts();

private:
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
