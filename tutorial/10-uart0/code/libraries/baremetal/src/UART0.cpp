//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : UART0.cpp
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

#include <baremetal/UART0.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Mailbox.h>
#include <baremetal/MemoryAccess.h>
#include <baremetal/PhysicalGPIOPin.h>
#include <baremetal/RPIProperties.h>

namespace baremetal {

UART0::UART0()
    : m_initialized{}
    , m_memoryAccess{GetMemoryAccess()}
{
}

UART0::UART0(IMemoryAccess &memoryAccess)
    : m_initialized{}
    , m_memoryAccess{memoryAccess}
{
}

// Set baud rate and characteristics (115200 8N1) and map to GPIO
void UART0::Initialize()
{
    if (m_initialized)
        return;
    // initialize UART
    m_memoryAccess.Write32(RPI_UART0_CR, 0); // turn off UART0

    Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT, m_memoryAccess);
    RPIProperties properties(mailbox);
    if (!properties.SetClockRate(ClockID::UART, 4000000, false))
        return;

    // map UART0 to GPIO pins
    PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction0, m_memoryAccess);
    PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction0, m_memoryAccess);
    m_memoryAccess.Write32(RPI_UART0_ICR, 0x7FF); // clear interrupts
    m_memoryAccess.Write32(RPI_UART0_IBRD, 2);    // 115200 baud
    m_memoryAccess.Write32(RPI_UART0_FBRD, 0xB);
    m_memoryAccess.Write32(RPI_UART0_LCRH, 0x7 << 4); // 8n1, enable FIFOs
    m_memoryAccess.Write32(RPI_UART0_CR, 0x301);      // enable Tx, Rx, UART
    m_initialized = true;
}

// Write a character
void UART0::Write(char c)
{
    // wait until we can send
    // Check Tx FIFO empty
    while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_TX_EMPTY)
    {
        NOP();
    }
    // Write the character to the buffer
    m_memoryAccess.Write32(RPI_UART0_DR, static_cast<unsigned int>(c));
}

// Receive a character
char UART0::Read()
{
    // wait until something is in the buffer
    // Check Rx FIFO holds data
    while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_RX_READY)
    {
        NOP();
    }
    // Read it and return
    return static_cast<char>(m_memoryAccess.Read32(RPI_UART0_DR));
}

void UART0::WriteString(const char *str)
{
    while (*str)
    {
        // convert newline to carriage return + newline
        if (*str == '\n')
            Write('\r');
        Write(*str++);
    }
}

UART0 &GetUART0()
{
    static UART0 value;
    value.Initialize();
    return value;
}

} // namespace baremetal
