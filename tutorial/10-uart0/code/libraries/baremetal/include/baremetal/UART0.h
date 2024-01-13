//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : UART0.h
//
// Namespace   : baremetal
//
// Class       : UART0
//
// Description : RPI UART0 class
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

#include <baremetal/CharDevice.h>

namespace baremetal {

class IMemoryAccess;

// Encapsulation for the UART0 device.
// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART0() needs to be used for this),
// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
class UART0 : public CharDevice
{
    friend UART0 &GetUART0();

private:
    bool            m_initialized;
    IMemoryAccess  &m_memoryAccess;

    // Constructs a default UART0 instance. Note that the constructor is private, so GetUART0() is needed to instantiate the UART0.
    UART0();

public:
    // Constructs a specialized UART0 instance with a custom IMemoryAccess instance. This is intended for testing.
    UART0(IMemoryAccess &memoryAccess);
    // Initialize the UART0 device. Only performed once, guarded by m_initialized.
    // Set baud rate and characteristics (115200 8N1) and map to GPIO
    void Initialize();
    // Read a character
    char Read() override;
    // Write a character
    void Write(char c) override;
    // Write a string
    void WriteString(const char* str);
};

// Constructs the singleton UART0 instance, if needed.
UART0 &GetUART0();

} // namespace baremetal
