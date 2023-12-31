//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

/// @brief Encapsulation for the UART1 device.
///
class UART1
{
private:
    bool            m_initialized;

public:
    /// @brief Constructs a default UART1 instance.
    UART1();
    /// @brief Initialize the UART1 device. Only performed once, guarded by m_initialized.
    ///
    ///  Set baud rate and characteristics (115200 8N1) and map to GPIO
    void Initialize();
    /// @brief Read a character
    /// @return Character read
    char Read();
    /// @brief Write a character
    /// @param c Character to be written
    void Write(char c);
    /// @brief Write a string
    /// @param str String to be written
    void WriteString(const char *str);

private:
    /// @param mode GPIO mode to be selected.
    /// @return true if successful, false otherwise
    bool SetMode(uint8 pinNumber, GPIOMode mode);
    /// @brief Set GPIO pin function
    /// @param function GPIO function to be selected.
    bool SetFunction(uint8 pinNumber, GPIOFunction function);
    /// @brief Set GPIO pin pull mode
    /// @param pullMode GPIO pull mode to be used.
    bool SetPullMode(uint8 pinNumber, GPIOPullMode pullMode);
    /// @brief Switch GPIO off
    bool Off(uint8 pinNumber, GPIOMode mode);
};

} // namespace baremetal
