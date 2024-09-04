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

/// <summary>
/// Physical GPIO pin (i.e. available on GPIO header) 
/// </summary>
class PhysicalGPIOPin : public IGPIOPin
{
private:
    /// @brief Configured GPIO pin number (0..53)
    uint8           m_pinNumber;
    /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
    GPIOMode        m_mode;
    /// @brief Configured GPIO function.
    GPIOFunction    m_function;
    /// @brief Configured GPIO pull mode (only for input function).
    GPIOPullMode    m_pullMode;
    /// @brief Current value of the GPIO pin (true for on, false for off).
    bool            m_value;
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess&  m_memoryAccess;

public:
    PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());

    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());

    uint8 GetPinNumber() const override;
    bool AssignPin(uint8 pinNumber) override;

    void On() override;
    void Off() override;
    bool Get() override;
    void Set(bool on) override;
    void Invert() override;

    GPIOMode GetMode();
    bool SetMode(GPIOMode mode);
    GPIOFunction GetFunction();
    GPIOPullMode GetPullMode();
    void SetPullMode(GPIOPullMode pullMode);

private:
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
