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

/// @file
/// Abstract GPIO pin. Could be either a virtual or physical pin

#include <stdlib/Types.h>

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
    /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction0,
    /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction1,
    /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction2,
    /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction3,
    /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction4,
    /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
    AlternateFunction5,
    /// @brief GPIO mode unknown / not set / invalid
    Unknown,
};

/// <summary>
/// Abstraction of a GPIO pin
/// </summary>
class IGPIOPin
{
public:
    /// <summary>
    /// Default destructor needed for abstract interface
    /// </summary>
    virtual ~IGPIOPin() = default;

    /// <summary>
    /// Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
    /// </summary>
    /// <returns>Pin number</returns>
    virtual uint8 GetPinNumber() const = 0;
    /// <summary>
    /// Assign a GPIO pin
    /// </summary>
    /// <param name="pinNumber">Pin number</param>
    /// <returns>true if successful, false otherwise</returns>
    virtual bool AssignPin(uint8 pinNumber) = 0;

    /// <summary>
    /// Switch GPIO on
    /// </summary>
    virtual void On() = 0;
    /// <summary>
    /// Switch GPIO off
    /// </summary>
    virtual void Off() = 0;
    /// <summary>
    /// Get GPIO value
    /// </summary>
    /// <returns>GPIO value, true if on, false if off</returns>
    virtual bool Get() = 0;
    /// <summary>
    /// Set GPIO on (true) or off (false)
    /// </summary>
    /// <param name="on">Value to set, on (true) or off (false)</param>
    virtual void Set(bool on) = 0;
    /// <summary>
    /// Invert GPIO value on->off off->on
    /// </summary>
    virtual void Invert() = 0;
    /// <summary>
    /// Get GPIO event status
    /// </summary>
    /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
    virtual bool GetEvent() = 0;
    /// <summary>
    /// Clear GPIO event status
    /// </summary>
    virtual void ClearEvent() = 0;
};

} // namespace baremetal
