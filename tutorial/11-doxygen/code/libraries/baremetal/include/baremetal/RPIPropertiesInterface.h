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
#include <baremetal/Macros.h>
#include <baremetal/MemoryAccess.h>

namespace baremetal {

// Raspberry Pi mailbox property tags
enum class PropertyID : uint32
{
    PROPTAG_END                     = 0x00000000,
    PROPTAG_GET_FIRMWARE_REVISION   = 0x00000001,
    PROPTAG_SET_CURSOR_INFO         = 0x00008010,
    PROPTAG_SET_CURSOR_STATE        = 0x00008011,
    PROPTAG_GET_BOARD_MODEL         = 0x00010001,
    PROPTAG_GET_BOARD_REVISION      = 0x00010002,
    PROPTAG_GET_MAC_ADDRESS         = 0x00010003,
    PROPTAG_GET_BOARD_SERIAL        = 0x00010004,
    PROPTAG_GET_ARM_MEMORY          = 0x00010005,
    PROPTAG_GET_VC_MEMORY           = 0x00010006,
    PROPTAG_GET_POWER_STATE         = 0x00020001,
    PROPTAG_SET_POWER_STATE         = 0x00028001,
    PROPTAG_GET_CLOCK_RATE          = 0x00030002,
    PROPTAG_GET_MAX_CLOCK_RATE      = 0x00030004,
    PROPTAG_GET_TEMPERATURE         = 0x00030006,
    PROPTAG_GET_MIN_CLOCK_RATE      = 0x00030007,
    PROPTAG_GET_TURBO               = 0x00030009,
    PROPTAG_GET_MAX_TEMPERATURE     = 0x0003000A,
    PROPTAG_GET_EDID_BLOCK          = 0x00030020,
    PROPTAG_GET_LED_STATE           = 0x00030041,
    PROPTAG_GET_THROTTLED           = 0x00030046,
    PROPTAG_GET_CLOCK_RATE_MEASURED = 0x00030047,
    PROPTAG_NOTIFY_XHCI_RESET       = 0x00030058,
    PROPTAG_TEST_LED_STATE          = 0x00034041,
    PROPTAG_SET_CLOCK_RATE          = 0x00038002,
    PROPTAG_SET_TURBO               = 0x00038009,
    PROPTAG_SET_DOMAIN_STATE        = 0x00038030,
    PROPTAG_SET_LED_STATE           = 0x00038041,
    PROPTAG_SET_SDHOST_CLOCK        = 0x00038042,
    PROPTAG_ALLOCATE_DISPLAY_BUFFER = 0x00040001,
    PROPTAG_GET_DISPLAY_DIMENSIONS  = 0x00040003,
    PROPTAG_GET_PITCH               = 0x00040008,
    PROPTAG_GET_TOUCHBUF            = 0x0004000F,
    PROPTAG_GET_GPIO_VIRTBUF        = 0x00040010,
    PROPTAG_GET_NUM_DISPLAYS        = 0x00040013,
    PROPTAG_SET_PHYS_WIDTH_HEIGHT   = 0x00048003,
    PROPTAG_SET_VIRT_WIDTH_HEIGHT   = 0x00048004,
    PROPTAG_SET_DEPTH               = 0x00048005,
    PROPTAG_SET_PIXEL_ORDER         = 0x00048006,
    PROPTAG_SET_VIRTUAL_OFFSET      = 0x00048009,
    PROPTAG_SET_PALETTE             = 0x0004800B,
    PROPTAG_WAIT_FOR_VSYNC          = 0x0004800E,
    PROPTAG_SET_BACKLIGHT           = 0x0004800F,
    PROPTAG_SET_DISPLAY_NUM         = 0x00048013,
    PROPTAG_SET_TOUCHBUF            = 0x0004801F,
    PROPTAG_SET_GPIO_VIRTBUF        = 0x00048020,
    PROPTAG_GET_COMMAND_LINE        = 0x00050001,
    PROPTAG_GET_DMA_CHANNELS        = 0x00060001,
};

struct MailboxBuffer
{
    uint32 bufferSize;      // bytes
    uint32 requestCode;
    uint8  tags[0];
    // end tag follows
} PACKED;

struct Property
{
    uint32 tagID;               // See PropertyID
    uint32 tagBufferSize;       // bytes, multiple of 4
    uint32 tagRequestResponse;  // bytes
    uint8  tagBuffer[0];        // must be padded to be 4 byte aligned
} PACKED;

struct PropertySimple
{
    Property tag;
    uint32   value;
} PACKED;

class RPIPropertiesInterface
{
private:
    IMailbox &m_mailbox;

public:
    explicit RPIPropertiesInterface(IMailbox &mailbox);

    bool   GetTag(PropertyID tagID, void *tag, unsigned tagSize);

private:
    size_t FillTag(PropertyID tagID, void *tag, unsigned tagSize);
    bool   CheckTagResult(void *tag);
    bool   GetTags(void *tags, unsigned tagsSize);
};

} // namespace baremetal
