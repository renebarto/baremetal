//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

#include "baremetal/UART1.h"

#include "baremetal/ARMInstructions.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/MemoryAccess.h"
#include "baremetal/PhysicalGPIOPin.h"

/// @file
/// Raspberry Pi UART1 serial device implementation

namespace baremetal {

/// <summary>
/// Constructs a default UART1 instance.
///
/// Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
/// </summary>
UART1::UART1()
    : m_isInitialized{}
    , m_memoryAccess{GetMemoryAccess()}
{
}

/// <summary>
/// Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Memory access interface</param>
UART1::UART1(IMemoryAccess& memoryAccess)
    : m_isInitialized{}
    , m_memoryAccess{memoryAccess}
{
}

/// <summary>
/// Initialize the UART1 device. Only performed once, guarded by m_isInitialized.
///
///  Set baud rate and characteristics (8N1) and map to GPIO. Baud rate maximum is 115200
/// </summary>
/// <param name="baudrate">Baud rate to set</param>
void UART1::Initialize(unsigned baudrate)
{
    if (m_isInitialized)
        return;

    // initialize UART
    auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
    m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1); // Disable UART1, AUX mini uart

    PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction5, m_memoryAccess);
    PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction5, m_memoryAccess);

    m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);                                                                                              // enable UART1, AUX mini uart
    m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                                                                                                          // Disable Tx, Rx
    m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);                                                                                                  // 8 bit mode
    m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);                                                                                                     // RTS high
    m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                                                                                                           // Disable interrupts
    m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR); // Clear FIFO
    m_memoryAccess.Write32(RPI_AUX_MU_BAUD, RPI_AUX_MU_BAUD_VALUE(baudrate));                                                                                            // Set baudrate
    m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);                                                                      // Enable Tx, Rx

    m_baudrate = baudrate;
    m_isInitialized = true;
}

/// <summary>
/// Return set baudrate
/// </summary>
/// <returns>Baudrate set for device</returns>
unsigned UART1::GetBaudrate() const
{
    return m_baudrate;
}

/// <summary>
/// Receive a character
/// </summary>
/// <returns>Character received</returns>
char UART1::Read()
{
    // wait until something is in the buffer
    // Check Rx FIFO holds data
    while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_RX_READY))
    {
        NOP();
    }
    // Read it and return
    return static_cast<char>(m_memoryAccess.Read32(RPI_AUX_MU_IO));
}

/// <summary>
/// Send a character
/// </summary>
/// <param name="c">Character to be sent</param>
void UART1::Write(char c)
{
    // wait until we can send
    // Check Tx FIFO empty
    while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_TX_EMPTY))
    {
        NOP();
    }
    // Write the character to the buffer
    m_memoryAccess.Write32(RPI_AUX_MU_IO, static_cast<uint32>(c));
}

/// <summary>
/// Write a string
/// </summary>
/// <param name="str">String to be written</param>
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

/// <summary>
/// Construct the singleton UART1 device if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton UART1 device</returns>
UART1& GetUART1()
{
    static UART1 value;

    return value;
}

} // namespace baremetal
