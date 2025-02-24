//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include <baremetal/CharDevice.h>

/// @file
/// Raspberry Pi UART0 serial device

namespace baremetal {

class IMemoryAccess;

/// <summary>
/// Encapsulation for the UART0 device.
///
/// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART0() needs to be used for this),
/// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
/// </summary>
class UART0 : public CharDevice
{
    /// <summary>
    /// Construct the singleton UART0 instance if needed, and return a reference to the instance. This is a friend function of class UART0
    /// </summary>
    /// <returns>Reference to the singleton UART0 instance</returns>
    friend UART0 &GetUART0();

private:
    /// @brief Flags if device was initialized. Used to guard against multiple initialization
    bool            m_isInitialized;
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess  &m_memoryAccess;

    UART0();

public:
    UART0(IMemoryAccess &memoryAccess);

    void Initialize();
    char Read() override;
    void Write(char c) override;
    void WriteString(const char* str);
};

UART0 &GetUART0();

} // namespace baremetal
