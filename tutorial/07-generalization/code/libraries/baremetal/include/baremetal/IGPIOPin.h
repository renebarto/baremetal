//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : IGPIOPin.h
//
// Namespace   : baremetal
//
// Class       : IGPIOPin
//
// Description : GPIO pin abstraction
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

#include <baremetal/Types.h>

namespace baremetal {

// GPIO function
enum class GPIOFunction
{
    // GPIO used as input
    Input,
    // GPIO used as output
    Output,
    // GPIO used as Alternate Function 0.
    AlternateFunction0,
    // GPIO used as Alternate Function 1.
    AlternateFunction1,
    // GPIO used as Alternate Function 2.
    AlternateFunction2,
    // GPIO used as Alternate Function 3.
    AlternateFunction3,
    // GPIO used as Alternate Function 4.
    AlternateFunction4,
    // GPIO used as Alternate Function 5.
    AlternateFunction5,
    Unknown,
};

// GPIO pull mode
enum class GPIOPullMode
{
    // GPIO pull mode off (no pull-up or pull-down)
    Off,
    // GPIO pull mode pull-down
    PullDown,
    // GPIO pull mode pull-up
    PullUp,
    Unknown,
};

// GPIO mode
enum class GPIOMode
{
    // GPIO used as input
    Input,
    // GPIO used as output
    Output,
    // GPIO used as input, using pull-up
    InputPullUp,
    // GPIO used as input, using pull-down
    InputPullDown,
    // GPIO used as Alternate Function 0.
    AlternateFunction0,
    // GPIO used as Alternate Function 1.
    AlternateFunction1,
    // GPIO used as Alternate Function 2.
    AlternateFunction2,
    // GPIO used as Alternate Function 3.
    AlternateFunction3,
    // GPIO used as Alternate Function 4.
    AlternateFunction4,
    // GPIO used as Alternate Function 5.
    AlternateFunction5,
    Unknown,
};

// Abstraction of a GPIO pin
class IGPIOPin
{
public:
    virtual ~IGPIOPin() = default;

    // Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
    virtual uint8 GetPinNumber() const = 0;
    // Assign a GPIO pin
    virtual bool AssignPin(uint8 pinNumber) = 0;

    // Switch GPIO on
    virtual void On() = 0;
    // Switch GPIO off
    virtual void Off() = 0;
    // Get GPIO value
    virtual bool Get() = 0;
    // Set GPIO on (true) or off (false)
    virtual void Set(bool on) = 0;
    // Invert GPIO value on->off off->on
    virtual void Invert() = 0;
};

} // namespace baremetal
