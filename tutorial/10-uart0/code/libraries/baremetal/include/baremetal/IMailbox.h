//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : IMailbox.h
//
// Namespace   : baremetal
//
// Class       : IMailbox
//
// Description : Arm <-> VC mailbox abstract interface
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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

// Mailbox channels
enum class MailboxChannel
{
    ARM_MAILBOX_CH_POWER = 0,       // Power management
    ARM_MAILBOX_CH_FB = 1,          // Frame buffer
    ARM_MAILBOX_CH_VUART = 2,       // Virtual UART?
    ARM_MAILBOX_CH_VCHIQ = 3,
    ARM_MAILBOX_CH_LEDS = 4,
    ARM_MAILBOX_CH_BTNS = 5,
    ARM_MAILBOX_CH_TOUCH = 6,
    ARM_MAILBOX_CH_COUNT = 7,
    ARM_MAILBOX_CH_PROP_OUT = 8,    // Properties / tags ARM -> VC
    ARM_MAILBOX_CH_PROP_IN = 9,     // Properties / tags VC -> ARM
};

// IMailbox: Mailbox abstract interface
class IMailbox
{
public:
    virtual ~IMailbox() = default;

    virtual uintptr WriteRead(uintptr address) = 0;
};

} // namespace baremetal
