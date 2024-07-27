//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : RPIPropertiesInterface.h
//
// Namespace   : baremetal
//
// Class       : RPIPropertiesInterface
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

#pragma once

#include <baremetal/IMailbox.h>
#include <baremetal/Macros.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Functionality handling for Raspberry Pi Mailbox

namespace baremetal {

/// @brief Raspberry Pi mailbox property tags
enum class PropertyID : uint32
{
    /// @brief End tag. Should always be last tag in Mailbox buffer
    PROPTAG_END                     = 0x00000000,
    /// @brief Get firmware revision
    PROPTAG_GET_FIRMWARE_REVISION   = 0x00000001,
    /// @brief Set cursor info. Defines graphical cursor (width, height, bitmap and hotspot position
    PROPTAG_SET_CURSOR_INFO         = 0x00008010,
    /// @brief Set cursor state. Defines location and visibility of graphical cursor
    PROPTAG_SET_CURSOR_STATE        = 0x00008011,
    /// @brief Get Raspberry Pi board model
    PROPTAG_GET_BOARD_MODEL         = 0x00010001,
    /// @brief Get Raspberry Pi board revision
    PROPTAG_GET_BOARD_REVISION      = 0x00010002,
    /// @brief Get Raspberry Pi board MAC address
    PROPTAG_GET_MAC_ADDRESS         = 0x00010003,
    /// @brief Raspberry Pi board serial number
    PROPTAG_GET_BOARD_SERIAL        = 0x00010004,
    /// @brief Get ARM memory base address and size
    PROPTAG_GET_ARM_MEMORY          = 0x00010005,
    /// @brief Get VideoCore memory base address and size
    PROPTAG_GET_VC_MEMORY           = 0x00010006,
    /// @brief Get power state for a device
    PROPTAG_GET_POWER_STATE         = 0x00020001,
    /// @brief Set power state for a device
    PROPTAG_SET_POWER_STATE         = 0x00028001,
    /// @brief Get clock rate for a clock ID in Hz
    PROPTAG_GET_CLOCK_RATE          = 0x00030002,
    /// @brief Get maximum clock rate for a clock ID in Hz
    PROPTAG_GET_MAX_CLOCK_RATE      = 0x00030004,
    /// @brief Get temperature for a specific ID in thousands of a degree Celsius
    PROPTAG_GET_TEMPERATURE         = 0x00030006,
    /// @brief Get minimum clock rate for a clock ID in Hz
    PROPTAG_GET_MIN_CLOCK_RATE      = 0x00030007,
    /// @brief Get turbo setting
    PROPTAG_GET_TURBO               = 0x00030009,
    /// @brief Get maximum safe temperature for a specific ID in thousands of a degree Celsius
    PROPTAG_GET_MAX_TEMPERATURE     = 0x0003000A,
    /// @brief Read and return EDID for attached HDMI/DVI device
    PROPTAG_GET_EDID_BLOCK          = 0x00030020,
    /// @brief Get onboard LED status (status LED, power LED)
    PROPTAG_GET_LED_STATE           = 0x00030041,
    /// @brief @todo To be defined
    PROPTAG_GET_THROTTLED           = 0x00030046,
    /// @brief Get measured clock rate for a clock ID in Hz
    PROPTAG_GET_CLOCK_RATE_MEASURED = 0x00030047,
    /// @brief @todo To be defined
    PROPTAG_NOTIFY_XHCI_RESET       = 0x00030058,
    /// @brief @todo To be defined
    PROPTAG_TEST_LED_STATE          = 0x00034041,
    /// @brief Set clock rate for a clock ID in Hz
    PROPTAG_SET_CLOCK_RATE          = 0x00038002,
    /// @brief Set turbo state
    PROPTAG_SET_TURBO               = 0x00038009,
    /// @brief @todo To be defined
    PROPTAG_SET_DOMAIN_STATE        = 0x00038030,
    /// @brief @todo To be defined
    PROPTAG_SET_LED_STATE           = 0x00038041,
    /// @brief @todo To be defined
    PROPTAG_SET_SDHOST_CLOCK        = 0x00038042,
    /// @brief @todo To be defined
    PROPTAG_ALLOCATE_DISPLAY_BUFFER = 0x00040001,
    /// @brief @todo To be defined
    PROPTAG_GET_DISPLAY_DIMENSIONS  = 0x00040003,
    /// @brief @todo To be defined
    PROPTAG_GET_PITCH               = 0x00040008,
    /// @brief @todo To be defined
    PROPTAG_GET_TOUCHBUF            = 0x0004000F,
    /// @brief @todo To be defined
    PROPTAG_GET_GPIO_VIRTBUF        = 0x00040010,
    /// @brief @todo To be defined
    PROPTAG_GET_NUM_DISPLAYS        = 0x00040013,
    /// @brief @todo To be defined
    PROPTAG_SET_PHYS_WIDTH_HEIGHT   = 0x00048003,
    /// @brief @todo To be defined
    PROPTAG_SET_VIRT_WIDTH_HEIGHT   = 0x00048004,
    /// @brief @todo To be defined
    PROPTAG_SET_DEPTH               = 0x00048005,
    /// @brief @todo To be defined
    PROPTAG_SET_PIXEL_ORDER         = 0x00048006,
    /// @brief @todo To be defined
    PROPTAG_SET_VIRTUAL_OFFSET      = 0x00048009,
    /// @brief @todo To be defined
    PROPTAG_SET_PALETTE             = 0x0004800B,
    /// @brief @todo To be defined
    PROPTAG_WAIT_FOR_VSYNC          = 0x0004800E,
    /// @brief @todo To be defined
    PROPTAG_SET_BACKLIGHT           = 0x0004800F,
    /// @brief @todo To be defined
    PROPTAG_SET_DISPLAY_NUM         = 0x00048013,
    /// @brief @todo To be defined
    PROPTAG_SET_TOUCHBUF            = 0x0004801F,
    /// @brief @todo To be defined
    PROPTAG_SET_GPIO_VIRTBUF        = 0x00048020,
    /// @brief @todo To be defined
    PROPTAG_GET_COMMAND_LINE        = 0x00050001,
    /// @brief @todo To be defined
    PROPTAG_GET_DMA_CHANNELS        = 0x00060001,
};

/// @brief Buffer passed to the Raspberry Pi Mailbox
struct MailboxBuffer
{
    /// @brief Total size of buffer in bytes, including bufferSize field. Buffer must be aligned to 16 bytes
    uint32 bufferSize;
    /// @brief Request code 
    uint32 requestCode;
    /// @brief Property tags to be handled, ended by end tag (PROPTAG_END). Each tag must be aligned to 4 bytes
    uint8  tags[0];
} PACKED;

/// @brief Property tag, one for each request
struct Property
{
    /// @brief Property ID, see PropertyID
    uint32 tagID;
    /// @brief Size of property tag buffer in bytes, so excluding tagID, tagBufferSize and tagRequestResponse, must be aligned to 4 bytes
    uint32 tagBufferSize;
    /// @brief Size of buffer for return data in bytes and return status
    uint32 tagRequestResponse;
    /// @brief Property tag request and response data, padded to align to 4 bytes
    uint8  tagBuffer[0];
} PACKED;

/// <summary>
/// Basic tag structure for a simple property request sending or receiving a 32 bit unsigned number.
/// 
/// This is also used for sanity checks on the size of the request
/// </summary>
struct PropertySimple
{
    /// @brief Tag ID of the the requested property
    Property tag;
    /// @brief A 32 bit unsigned value being send or requested
    uint32   value;
} PACKED;

/// <summary>
/// Low level functionality for requests on Mailbox interface
/// </summary>
class RPIPropertiesInterface
{
private:
    /// @brief Reference to mailbox for functions requested
    IMailbox &m_mailbox;

public:
    explicit RPIPropertiesInterface(IMailbox &mailbox);

    bool GetTag(PropertyID tagID, void *tag, unsigned tagSize);

private:
    size_t FillTag(PropertyID tagID, void *tag, unsigned tagSize);
    bool   CheckTagResult(void *tag);
    bool   GetTags(void *tags, unsigned tagsSize);
};

} // namespace baremetal
