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

namespace baremetal {

// Physical GPIO pin (i.e. available on GPIO header)
class PhysicalGPIOPin : public IGPIOPin
{
private:
    uint8                 m_pinNumber;
    GPIOMode              m_mode;
    GPIOFunction          m_function;
    GPIOPullMode          m_pullMode;
    bool                  m_value;
    IMemoryAccess& m_memoryAccess;

public:
    // Creates a virtual GPIO pin
    PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());

    // Creates a virtual GPIO pin
    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());

    uint8 GetPinNumber() const override;
    // Assign a GPIO pin
    bool AssignPin(uint8 pinNumber) override;

    // Switch GPIO on
    void On() override;
    // Switch GPIO off
    void Off() override;
    // Get GPIO value
    bool Get() override;
    // Set GPIO on (true) or off (false)
    void Set(bool on) override;
    // Invert GPIO value on->off off->on
    void Invert() override;

    // Get the mode for the GPIO pin
    GPIOMode GetMode();
    // Set the mode for the GPIO pin
    bool SetMode(GPIOMode mode);
    // Get GPIO pin function
    GPIOFunction GetFunction();
    // Get GPIO pin pull mode
    GPIOPullMode GetPullMode();
    // Set GPIO pin pull mode
    void SetPullMode(GPIOPullMode pullMode);

private:
    // Set GPIO pin function
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
