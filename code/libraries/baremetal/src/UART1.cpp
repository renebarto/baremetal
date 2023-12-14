//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : UART1.cpp
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

#include <baremetal/UART1.h>

namespace baremetal {

    UART1::UART1(IMemoryAccess& memoryAccess)
        : m_initialized{}
        , m_memoryAccess{ memoryAccess }
    {
    }

    // Set baud rate and characteristics (115200 8N1) and map to GPIO
    void UART1::Initialize()
    {
        if (m_initialized)
            return;

        // initialize UART

        // TODO
        m_initialized = true;
    }

    // Write a character

    void UART1::Write(char c)
    {
        // TODO
    }

    // Receive a character

    char UART1::Read()
    {
        // TODO
        return {};
    }

    void UART1::WriteString(const char* str)
    {
        while (*str)
        {
            // convert newline to carriage return + newline
            if (*str == '\n')
                Write('\r');
            Write(*str++);
        }
    }

} // namespace baremetal
