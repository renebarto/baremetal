//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

//#include <baremetal/GPIOControl.h>
#include <baremetal/IGPIOPin.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Physical GPIO pin

namespace baremetal {

enum class GPIOInterrupt
{
    RisingEdge,
    FallingEdge,
    HighLevel,
    LowLevel,
    AsyncRisingEdge,
    OnAsyncFallingEdge,
    Unknown
};

using GPIOInterruptHandler = void(void *param);

/// @brief Physical GPIO pin (i.e. available on GPIO header)
class PhysicalGPIOPin : public IGPIOPin
{
private:
    uint8                 m_pinNumber;
    unsigned              m_regOffset;
    uint32                m_regMask;
    GPIOMode              m_mode;
    GPIOFunction          m_function;
    GPIOPullMode          m_pullMode;
    bool                  m_value;

    GPIOInterruptHandler *m_handler;
    void                 *m_param;
    bool                  m_autoAck;
    GPIOInterrupt         m_interrupt;
    GPIOInterrupt         m_interrupt2;
    IMemoryAccess        &m_memoryAccess;

public:
    /// @brief Creates a virtual GPIO pin
    PhysicalGPIOPin(IMemoryAccess &memoryAccess);
    // PhysicalGPIOPin(const PhysicalGPIOPin &other);

    /// @brief Creates a virtual GPIO pin
    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess);

    uint8 GetPinNumber() const override;
    /// @brief Assign a GPIO pin
    /// @param pin      Pin number
    /// @return true if successful, false otherwise
    bool AssignPin(uint8 pinNumber) override;
    /// @brief Get the mode for the GPIO pin
    /// @return mode GPIO mode. See \ref GPIOMode
    GPIOMode GetMode();
    /// @brief Set the mode for the GPIO pin
    /// @param mode GPIO mode to be selected. See \ref GPIOMode
    /// @return true if successful, false otherwise
    bool SetMode(GPIOMode mode);
    /// @brief Get GPIO pin function
    /// @return GPIO pin function used. See \ref GPIOFunction
    GPIOFunction GetFunction();
    /// @brief Get GPIO pin pull mode
    /// @return GPIO pull mode used. See \ref GPIOPullMode
    GPIOPullMode GetPullMode();
    /// @brief Set GPIO pin pull mode
    /// @param pullMode GPIO pull mode to be used. See \ref GPIOPullMode
    void SetPullMode(GPIOPullMode pullMode);

    void ConnectInterrupt(GPIOInterruptHandler *handler, void *param, bool autoAck = true);
    void DisconnectInterrupt();
    bool AutoAckInterrupt() const override;

    void EnableInterrupt(GPIOInterrupt interrupt);
    void DisableInterrupt();

    void EnableInterrupt2(GPIOInterrupt interrupt);
    void DisableInterrupt2();

    void AcknowledgeInterrupt();
    void AutoAcknowledgeInterrupt() override;

    void InterruptHandler() override;
    void DisableAllInterrupts();

    /// @brief Switch GPIO on
    void On() override;
    /// @brief Switch GPIO off
    void Off() override;
    /// @brief Get GPIO value
    bool Get() override;
    /// @brief Set GPIO on (true) or off (false)
    void Set(bool on) override;
    /// @brief Invert GPIO value on->off off->on
    void Invert() override;

private:
    /// @brief Set GPIO pin function
    /// @param function GPIO function to be selected. See \ref GPIOFunction, \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
