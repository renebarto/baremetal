//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Mailbox.h
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

#include <baremetal/IMailbox.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Raspberry Pi Mailbox

namespace baremetal {

/// @brief Mailbox: Handles access to system parameters, stored in the VC
///
/// The mailbox handles communication with the Raspberry Pi GPU using communication channels. The most frequently used is the ARM_MAILBOX_CH_PROP_OUT channel
class Mailbox : public IMailbox
{
private:
    /// <summary>
    /// Channel to be used for mailbox
    /// </summary>
    MailboxChannel m_channel;
    /// <summary>
    /// Memory access interface
    /// </summary>
    IMemoryAccess &m_memoryAccess;

public:
    Mailbox(MailboxChannel channel, IMemoryAccess &memoryAccess = GetMemoryAccess());

    uintptr WriteRead(uintptr address) override;

private:
    void   Flush();
    uintptr Read();
    void   Write(uintptr data);
};

} // namespace baremetal
