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
#include <baremetal/Logger.h>
#include <baremetal/RPIPropertiesInterface.h>
#include <baremetal/Util.h>

/// @file
/// Top level functionality handling for Raspberry Pi Mailbox implementation

/// @brief Define log name
LOG_MODULE("RPIProperties");

namespace baremetal {

/// <summary>
/// Mailbox property tag structure for requesting board serial number.
/// </summary>
struct PropertySerial
{
    /// Tag ID, must be equal to PROPTAG_GET_BOARD_REVISION.
    Property tag;
    /// The requested serial number/ This is a 64 bit unsigned number, divided up into two times a 32 bit number
    uint32   serial[2];
} PACKED;

/// <summary>
/// Mailbox property tag structure for requesting board serial number.
/// </summary>
struct PropertyClockRate
{
    /// @brief Tag ID, must be equal to PROPTAG_GET_CLOCK_RATE, PROPTAG_GET_MAX_CLOCK_RATE, PROPTAG_GET_MIN_CLOCK_RATE, PROPTAG_GET_CLOCK_RATE_MEASURED or PROPTAG_SET_CLOCK_RATE.
    Property tag;
    /// @brief Clock ID, selected the clock for which information is requested or set
    uint32   clockID;
    /// @brief Requested or set clock frequency, in Hz
    uint32   rate;
    /// @brief If 1, do not switch to turbo setting if ARM clock is above default.
    /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.
    uint32   skipTurbo;
} PACKED;

/// <summary>
/// Mailbox property tag structure for requesting MAC address.
/// </summary>
struct PropertyMACAddress
{
    /// Tag ID, must be equal to PROPTAG_GET_MAC_ADDRESS.
    Property tag;
    /// MAC Address (6 bytes)
    uint8    address[6];
    /// Padding to align to 4 bytes
    uint8    padding[2];
} PACKED;

/// <summary>
/// Mailbox property tag structure for requesting memory information.
/// </summary>
struct PropertyMemory
{
    /// Tag ID, must be equal to PROPTAG_GET_ARM_MEMORY or PROPTAG_GET_VC_MEMORY.
    Property tag;
    /// Base address
    uint32   baseAddress;
    /// Size in bytes
    uint32   size;
} PACKED;

/// <summary>
/// Constructor
/// </summary>
/// <param name="mailbox">Mailbox to be used for requests. Can be a fake for testing purposes</param>
RPIProperties::RPIProperties(IMailbox &mailbox)
    : m_mailbox{mailbox}
{
}

/// <summary>
/// Retrieve FW revision number
/// </summary>
/// <param name="revision">FW revision (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetFirmwareRevision(uint32& revision)
{
    PropertySimple         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_FIRMWARE_REVISION, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetFirmwareRevision");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        revision = tag.value;
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Revision: %08lx", tag.value);
#endif
    }

    return result;
}

/// <summary>
/// Retrieve Raspberry Pi board model
/// </summary>
/// <param name="model">Board model (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetBoardModel(BoardModel& model)
{
    PropertySimple         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_MODEL, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetBoardModel");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        model = static_cast<BoardModel>(tag.value);
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Model: %08lx", tag.value);
#endif
    }

    return result;
}

/// <summary>
/// Retrieve Raspberry Pi board revision
/// </summary>
/// <param name="revision">Board revision (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetBoardRevision(BoardRevision& revision)
{
    PropertySimple         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_REVISION, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetBoardRevision");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        revision = static_cast<BoardRevision>(tag.value);
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Revision: %08lx", tag.value);
#endif
    }

    return result;
}

/// <summary>
/// Retrieve network MAC address
/// </summary>
/// <param name="address">MAC address (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetBoardMACAddress(uint8 address[6])
{
    PropertyMACAddress     tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_MAC_ADDRESS, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetBoardMACAddress");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        memcpy(address, tag.address, sizeof(tag.address));
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Address:");
        GetConsole().Write(address, sizeof(tag.address));
#endif
    }

    return result;
}

/// <summary>
/// Request board serial number
/// </summary>
/// <param name="serial">On return, set to serial number, if successful</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetBoardSerial(uint64 &serial)
{
    PropertySerial         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetBoardSerial");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Serial: %016llx", serial);
#endif
    }

    return result;
}

/// <summary>
/// Retrieve ARM assigned memory base address and size
/// </summary>
/// <param name="baseAddress">ARM assigned base address (out)</param>
/// <param name="size">ARM assigned memory size in bytes (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetARMMemory(uint32& baseAddress, uint32& size)
{
    PropertyMemory         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_ARM_MEMORY, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetARMMemory");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        baseAddress = tag.baseAddress;
        size = tag.size;
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Base address: %08lx", baseAddress);
        LOG_DEBUG("Size:         %08lx", size);
#endif
    }

    return result;
}

/// <summary>
/// Retrieve VideoCore assigned memory base address and size
/// </summary>
/// <param name="baseAddress">VideoCore assigned base address (out)</param>
/// <param name="size">VideoCore assigned memory size in bytes (out)</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIProperties::GetVCMemory(uint32& baseAddress, uint32& size)
{
    PropertyMemory         tag{};
    RPIPropertiesInterface interface(m_mailbox);

    auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_VC_MEMORY, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetARMMemory");

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        baseAddress = tag.baseAddress;
        size = tag.size;
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Base address: %08lx", baseAddress);
        LOG_DEBUG("Size:         %08lx", size);
#endif
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
    PropertyClockRate      tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID = static_cast<uint32>(clockID);
    auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetClockRate");
    LOG_DEBUG("Clock ID:   %08lx", tag.clockID);

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        freqHz = tag.rate;
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Rate:       %08lx", tag.rate);
#endif
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
    PropertyClockRate      tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID = static_cast<uint32>(clockID);
    auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));

#if BAREMETAL_DEBUG_TRACING
    LOG_DEBUG("GetMeasuredClockRate");
    LOG_DEBUG("Clock ID:   %08lx", tag.clockID);

    LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
#endif
    if (result)
    {
        freqHz = tag.rate;
#if BAREMETAL_DEBUG_TRACING
        LOG_DEBUG("Rate:       %08lx", tag.rate);
#endif
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
    PropertyClockRate      tag{};
    RPIPropertiesInterface interface(m_mailbox);

    tag.clockID   = static_cast<uint32>(clockID);
    tag.rate      = freqHz;
    tag.skipTurbo = skipTurbo;
    auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));

    // Do not write to console here, as this call is needed to set up the console

    return result;
}

} // namespace baremetal
