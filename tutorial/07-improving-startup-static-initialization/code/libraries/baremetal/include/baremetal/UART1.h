//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : UART1.h
//
// Namespace   : baremetal
//
// Class       : UART1
//
// Description : RPI UART1 class
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

/// @file
/// Raspberry Pi UART1 serial device declaration

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

/// @brief GPIO function
enum class GPIOFunction
{
    /// @brief GPIO used as input
    Input,
    /// @brief GPIO used as output
    Output,
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
    /// @brief GPIO function unknown / not set / invalid
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
    /// @brief GPIO pull mode unknown / not set / invalid
    Unknown,
};

/// <summary>
/// Encapsulation for the UART1 device.
/// </summary>
class UART1
{
    /// <summary>
    /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
    /// </summary>
    /// <returns>Reference to the singleton UART1 instance</returns>
    friend UART1 &GetUART1();

private:
    /// @brief Flags if device was initialized. Used to guard against multiple initialization
    bool m_isInitialized;

public:
    UART1();

    void Initialize();
    char Read();
    void Write(char c);
    void WriteString(const char *str);

private:
    // Set GPIO pin mode
    bool SetMode(uint8 pinNumber, GPIOMode mode);
    // Set GPIO pin function
    bool SetFunction(uint8 pinNumber, GPIOFunction function);
    // Set GPIO pin pull mode
    bool SetPullMode(uint8 pinNumber, GPIOPullMode pullMode);
    // Switch GPIO off
    bool Off(uint8 pinNumber, GPIOMode mode);
};

/// <summary>
/// Construct the singleton UART1 device if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton UART1 device</returns>
UART1 &GetUART1();

} // namespace baremetal
