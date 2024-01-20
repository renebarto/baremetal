//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : RPIProperties.h
//
// Namespace   : baremetal
//
// Class       : RPIProperties
//
// Description : Access to BCM2835/2836/2837/2711/2712 properties using mailbox
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

#include <baremetal/IMailbox.h>
#include <baremetal/Types.h>

/// @file
/// Top level functionality handling for Raspberry Pi Mailbox

namespace baremetal {

/// <summary>
/// Clock ID number. Used to retrieve and set the clock frequency for several clocks
/// </summary>
enum class ClockID : uint32
{
    /// @brief EMMC clock
    EMMC      = 1,
    /// @brief UART0 clock
    UART      = 2,
    /// @brief ARM processor clock
    ARM       = 3,
    /// @brief Core SoC clock
    CORE      = 4,
    /// @brief EMMC clock 2
    EMMC2     = 12,
    /// @brief Pixel clock
    PIXEL_BVB = 14,
};

/// <summary>
/// Top level functionality for requests on Mailbox interface
/// </summary>
class RPIProperties
{
private:
    /// @brief Reference to mailbox for functions requested
    IMailbox &m_mailbox;

public:
    explicit RPIProperties(IMailbox &mailbox);

    bool GetBoardSerial(uint64 &serial);
    bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
};

} // namespace baremetal
