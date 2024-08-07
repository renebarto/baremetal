//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : RPIProperties.cpp
//
// Namespace   : baremetal
//
// Class       : RPIProperties
//
// Description : Access to BCM2835/6/7 properties using mailbox
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

#include <baremetal/RPIProperties.h>

#include <baremetal/BCMRegisters.h>
#include <baremetal/RPIPropertiesInterface.h>
#include <baremetal/Util.h>

namespace baremetal {

struct PropertySerial
{
    Property tag;
    uint32   serial[2];
} PACKED;

struct PropertyClockRate
{
    Property tag;
    uint32   clockID;
    uint32   rate;      // Hz
    uint32   skipTurbo; // If 1, do not set turbo mode, if 0, set turbo mode if necessary
} PACKED;

RPIProperties::RPIProperties(IMailbox &mailbox)
    : m_mailbox{mailbox}
{
}

bool RPIProperties::GetBoardSerial(uint64 &serial)
{
    PropertySerial         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));

    if (result)
    {
        serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
    }

    return result;
}

bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
{
    PropertyClockRate      tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID   = static_cast<uint32>(clockID);
    tag.rate      = freqHz;
    tag.skipTurbo = skipTurbo;
    auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));

    return result;
}

} // namespace baremetal
