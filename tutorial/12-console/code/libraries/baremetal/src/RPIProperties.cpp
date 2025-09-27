//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "baremetal/RPIProperties.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/RPIPropertiesInterface.h"
#include "stdlib/Util.h"

/// @file
/// Top level functionality handling for Raspberry Pi Mailbox implementation

namespace baremetal {

/// <summary>
/// Mailbox property tag structure for requesting board serial number.
/// </summary>
struct PropertyTagSerial
{
    /// Tag ID, must be equal to PROPTAG_GET_BOARD_REVISION.
    PropertyTag tag;
    /// The requested serial number/ This is a 64 bit unsigned number, divided up into two times a 32 bit number
    uint32 serial[2];
} PACKED;

//// <summary>
/// Mailbox property tag structure for requesting board serial number.
/// </summary>
struct PropertyTagClockRate
{
    /// @brief Tag ID, must be equal to PROPTAG_GET_CLOCK_RATE, PROPTAG_GET_MAX_CLOCK_RATE, PROPTAG_GET_MIN_CLOCK_RATE, PROPTAG_GET_CLOCK_RATE_MEASURED or PROPTAG_SET_CLOCK_RATE.
    PropertyTag tag;
    /// @brief Clock ID, selected the clock for which information is requested or set
    uint32 clockID;
    /// @brief Requested or set clock frequency, in Hz
    uint32 rate;
    /// @brief If 1, do not switch to turbo setting if ARM clock is above default.
    /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.
    uint32 skipTurbo;
} PACKED;

// <summary>
/// Constructor
/// </summary>
/// <param name="mailbox">Mailbox to be used for requests. Can be a fake for testing purposes</param>
RPIProperties::RPIProperties(IMailbox& mailbox)
    : m_mailbox{mailbox}
{
}

/// <summary>
/// Request board serial number
/// </summary>
/// <param name="serial">On return, set to serial number, if successful</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetBoardSerial(uint64& serial)
{
    PropertyTagSerial tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));

    if (result)
    {
        serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
    }

    return result;
}

/// <summary>
/// Get clock rate for specified clock
/// </summary>
/// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
/// <param name="freqHz">Clock frequencyy in Hz (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
{
    PropertyTagClockRate tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID = static_cast<uint32>(clockID);
    auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));

    if (result)
    {
        freqHz = tag.rate;
    }

    return result;
}

/// <summary>
/// Get measured clock rate for specified clock
/// </summary>
/// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
/// <param name="freqHz">Clock frequencyy in Hz (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
{
    PropertyTagClockRate tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID = static_cast<uint32>(clockID);
    auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));

    if (result)
    {
        freqHz = tag.rate;
    }

    return result;
}

/// <summary>
/// Set clock rate for specified clock
/// </summary>
/// <param name="clockID">ID of clock to be set</param>
/// <param name="freqHz">Clock frequencyy in Hz</param>
/// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
/// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
{
    PropertyTagClockRate tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID = static_cast<uint32>(clockID);
    tag.rate = freqHz;
    tag.skipTurbo = skipTurbo;
    auto result = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));

    // Do not write to console here, as this call is needed to set up the console

    return result;
}

} // namespace baremetal
