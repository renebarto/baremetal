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

// Encapsulation for the UART1 device.
class UART1
{
    friend UART1& GetUART1();

private:
    bool            m_initialized;

public:
    // Constructs a default UART1 instance.
    UART1();
    // Initialize the UART1 device. Only performed once, guarded by m_initialized.
    //
    //  Set baud rate and characteristics (115200 8N1) and map to GPIO
    void Initialize();
    // Read a character
    char Read();
    // Write a character
    void Write(char c);
    // Write a string
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

UART1& GetUART1();

} // namespace baremetal
