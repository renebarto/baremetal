//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "stdlib/Types.h"

/// @file
/// Abstract Mailbox interface

namespace baremetal {

/// <summary>
/// Mailbox channel
/// </summary>
enum class MailboxChannel
{
    /// Power management
    ARM_MAILBOX_CH_POWER = 0,
    /// Frame buffer
    ARM_MAILBOX_CH_FB = 1,
    /// Virtual UART
    ARM_MAILBOX_CH_VUART = 2,
    /// VCHIQ / GPU
    ARM_MAILBOX_CH_VCHIQ = 3,
    /// LEDs
    ARM_MAILBOX_CH_LEDS = 4,
    /// Buttons
    ARM_MAILBOX_CH_BTNS = 5,
    /// Touch screen
    ARM_MAILBOX_CH_TOUCH = 6,
    /// ?
    ARM_MAILBOX_CH_COUNT = 7,
    /// Properties / tags ARM -> VC
    ARM_MAILBOX_CH_PROP_OUT = 8,
    /// Properties / tags VC -> ARM
    ARM_MAILBOX_CH_PROP_IN = 9,
};

/// <summary>
/// Mailbox abstract interface
/// </summary>
class IMailbox
{
public:
    /// <summary>
    /// Default destructor needed for abstract interface
    /// </summary>
    virtual ~IMailbox() = default;

    /// <summary>
    /// Perform a write - read cycle on the mailbox
    /// </summary>
    /// <param name="address">Address of mailbox data block (converted to GPU address space)</param>
    /// <returns>Address of mailbox data block, should be equal to input address</returns>
    virtual uintptr WriteRead(uintptr address) = 0;
};

} // namespace baremetal
