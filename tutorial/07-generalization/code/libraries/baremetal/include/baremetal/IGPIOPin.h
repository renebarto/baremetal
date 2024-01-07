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

#include <baremetal/Types.h>

namespace baremetal {

/// @brief GPIO mode
enum class GPIOMode
{
    /// @brief GPIO used as input
    Input,
    /// @brief GPIO used as output
    Output,
    /// @brief GPIO used as input, using pull-up
    InputPullUp,
    /// @brief GPIO used as input, using pull-down
    InputPullDown,
    /// @brief GPIO used as Alternate Function 0.
    AlternateFunction0,
    /// @brief GPIO used as Alternate Function 1.
    AlternateFunction1,
    /// @brief GPIO used as Alternate Function 2.
    AlternateFunction2,
    /// @brief GPIO used as Alternate Function 3.
    AlternateFunction3,
    /// @brief GPIO used as Alternate Function 4.
    AlternateFunction4,
    /// @brief GPIO used as Alternate Function 5.
    AlternateFunction5,
    Unknown,
};

/// @brief GPIO function
enum class GPIOFunction
{
    /// @brief GPIO used as input
    Input,
    /// @brief GPIO used as output
    Output,
    /// @brief GPIO used as Alternate Function 0.
    AlternateFunction0,
    /// @brief GPIO used as Alternate Function 1.
    AlternateFunction1,
    /// @brief GPIO used as Alternate Function 2.
    AlternateFunction2,
    /// @brief GPIO used as Alternate Function 3.
    AlternateFunction3,
    /// @brief GPIO used as Alternate Function 4.
    AlternateFunction4,
    /// @brief GPIO used as Alternate Function 5.
    AlternateFunction5,
    Unknown,
};

/// @brief GPIO pull mode
enum class GPIOPullMode
{
    /// @brief GPIO pull mode off (no pull-up or pull-down)
    Off,
    /// @brief GPIO pull mode pull-down
    PullDown,
    /// @brief GPIO pull mode pull-up
    PullUp,
    Unknown,
};

/// @brief Abstraction of a GPIO pin
class IGPIOPin
{
public:
    virtual ~IGPIOPin() = default;

    /// @brief Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
    /// @return Pin number
    virtual uint8 GetPinNumber() const = 0;
    /// @brief Assign a GPIO pin
    /// @param pin      Pin number
    /// @return true if successful, false otherwise
    virtual bool AssignPin(uint8 pinNumber) = 0;

    /// @brief Switch GPIO on
    virtual void On() = 0;
    /// @brief Switch GPIO off
    virtual void Off() = 0;
    /// @brief Get GPIO value
    virtual bool Get() = 0;
    /// @brief Set GPIO on (true) or off (false)
    virtual void Set(bool on) = 0;
    /// @brief Invert GPIO value on->off off->on
    virtual void Invert() = 0;
};

} // namespace baremetal
