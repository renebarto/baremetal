//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Mailbox.cpp
//
// Namespace   : baremetal
//
// Class       : Mailbox
//
// Description : Arm <-> VC mailbox handling
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

#include <baremetal/Mailbox.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>
#include <baremetal/Timer.h>

/// @file
/// Raspberry Pi Mailbox implementation

using namespace baremetal;

/// <summary>
/// Construct a mailbox
/// </summary>
/// <param name="channel">Channel to be used for communication</param>
/// <param name="memoryAccess">Memory access interface for registers, default to the singleton memory access instantiation</param>
Mailbox::Mailbox(MailboxChannel channel, IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
    : m_channel{ channel }
    , m_memoryAccess{ memoryAccess }
{
}

/// <summary>
/// Perform a write - read cycle on the mailbox for the selected channel (m_channel)
/// </summary>
/// <param name="address">Address of mailbox data block (converted to GPU address space)</param>
/// <returns>Address of mailbox data block, should be equal to input address</returns>
uintptr Mailbox::WriteRead(uintptr address)
{
    Flush();

    Write(address);

    uint32 result = Read();

    return result;
}

/// <summary>
/// Flush the mailbox for the selected channel (m_channel), by reading until it is empty. A short wait is added for synchronization reasons.
/// </summary>
void Mailbox::Flush()
{
    while (!(m_memoryAccess.Read32(RPI_MAILBOX0_STATUS) & RPI_MAILBOX_STATUS_EMPTY))
    {
        m_memoryAccess.Read32(RPI_MAILBOX0_READ);

        Timer::WaitMilliSeconds(20);
    }
}

/// <summary>
/// Read back the address of the data block to the mailbox for the selected channel (m_channel)
// The address should be equal to what was written, as the mailbox can only handle sequential requests for a channel
/// </summary>
/// <returns>The pointer to the mailbox data block passed in to Write() on success (in GPU address space).</returns>
uintptr Mailbox::Read()
{
    uintptr result;

    do
    {
        while (m_memoryAccess.Read32(RPI_MAILBOX0_STATUS) & RPI_MAILBOX_STATUS_EMPTY)
        {
            NOP();
        }

        result = static_cast<uintptr>(m_memoryAccess.Read32(RPI_MAILBOX0_READ));
    } while ((result & 0xF) != static_cast<uint32>(m_channel)); // channel number is in the lower 4 bits

    return result & ~0xF;
}

/// <summary>
/// Write to the mailbox on the selected channel
/// </summary>
/// <param name="data">Address of mailbox data block (converted to GPU address space)</param>
void Mailbox::Write(uintptr data)
{
    // Data must be 16 byte aligned
    if ((data & 0xF) != 0)
        return;

    while (m_memoryAccess.Read32(RPI_MAILBOX1_STATUS) & RPI_MAILBOX_STATUS_FULL)
    {
        NOP();
    }

    m_memoryAccess.Write32(RPI_MAILBOX1_WRITE, static_cast<uint32>(m_channel) | static_cast<uint32>(data)); // channel number is in the lower 4 bits
}
