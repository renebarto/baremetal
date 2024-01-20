//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : RPIPropertiesInterface.cpp
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

#include <baremetal/RPIPropertiesInterface.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryManager.h>
#include <baremetal/Util.h>

/// @file
/// Functionality handling for Raspberry Pi Mailbox implementation

namespace baremetal {

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
 /// Constructs a RPI properties interface object
 /// </summary>
 /// <param name="mailbox">Mailbox to be used for requests. Can be a fake for testing purposes</param>
RPIPropertiesInterface::RPIPropertiesInterface(IMailbox &mailbox)
    : m_mailbox{mailbox}
{
}

/// <summary>
/// Request property tag. The tag data for request must be filled in, the header will be filled in to the buffer.
/// The buffer must be large enough to hold the complete tag including its header.
/// On successful return, the buffer will be filled with the response data
/// </summary>
/// <param name="tagID">Property tag to be requested</param>
/// <param name="tag">Buffer to tag data, large enough to hold complete Property</param>
/// <param name="tagSize">Size of the tag data buffer in bytes</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIPropertiesInterface::GetTag(PropertyID tagID, void *tag, unsigned tagSize)
{
    if (FillTag(tagID, tag, tagSize) != tagSize)
        return false;

    auto result = GetTags(tag, tagSize);

    if (!result)
    {
        return false;
    }

    return CheckTagResult(tag);
}

/// <summary>
/// Check whether the property tag was successfully requested, by checking the tagRequestResponse field in the Property header
/// </summary>
/// <param name="tag">Buffer to property tag data</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIPropertiesInterface::CheckTagResult(void *tag)
{
    Property *header = reinterpret_cast<Property *>(tag);

    if ((header->tagRequestResponse & RPI_MAILBOX_TAG_RESPONSE) == 0)
        return false;

    header->tagRequestResponse &= ~RPI_MAILBOX_TAG_RESPONSE;
    return (header->tagRequestResponse != 0);
}

/// <summary>
/// Fill in tag header for the requested property tag.
/// </summary>
/// <param name="tagID">Property tag to be requested</param>
/// <param name="tag">Buffer to tag data, large enough to hold complete Property</param>
/// <param name="tagSize">Size of the tag data buffer in bytes</param>
/// <returns>Tag size in bytes</returns>
size_t RPIPropertiesInterface::FillTag(PropertyID tagID, void *tag, unsigned tagSize)
{
    if ((tag == nullptr) || (tagSize < sizeof(PropertySimple)))
        return 0;

    Property *header        = reinterpret_cast<Property *>(tag);
    header->tagID           = static_cast<uint32>(tagID);
    header->tagBufferSize   = tagSize - sizeof(Property);
    header->tagRequestResponse = 0;

    return tagSize;
}

/// <summary>
/// Fill in the Mailbox buffer with the tags requested, and perform the request.
/// Will fill in the mailbox buffer header, and the tag data, append the end tag, and perform the mailbox request.
/// </summary>
/// <param name="tags">Buffer to tag data, for all requested properties, except the end tag</param>
/// <param name="tagsSize">Size of the tag data buffer in bytes</param>
/// <returns>Return true on success, false on failure</returns>
bool RPIPropertiesInterface::GetTags(void *tags, unsigned tagsSize)
{
    if ((tags == nullptr) || (tagsSize < sizeof(PropertySimple)))
        return false;

    unsigned bufferSize = sizeof(MailboxBuffer) + tagsSize + sizeof(uint32);
    if ((bufferSize & 3) != 0)
        return false;

    MailboxBuffer *buffer = reinterpret_cast<MailboxBuffer *>(MemoryManager::GetCoherentPage(CoherentPageSlot::PropertyMailbox));

    buffer->bufferSize  = bufferSize;
    buffer->requestCode = RPI_MAILBOX_REQUEST;
    memcpy(buffer->tags, tags, tagsSize);

    uint32 *endTag = reinterpret_cast<uint32 *>(buffer->tags + tagsSize);
    *endTag        = static_cast<uint32>(PropertyID::PROPTAG_END);

    DataSyncBarrier();

    uintptr bufferAddress = ARM_TO_GPU(reinterpret_cast<uintptr>(buffer));
    if (m_mailbox.WriteRead(bufferAddress) != bufferAddress)
    {
        return false;
    }

    DataMemBarrier();

    if (buffer->requestCode != RPI_MAILBOX_RESPONSE_SUCCESS)
    {
        return false;
    }

    memcpy(tags, buffer->tags, tagsSize);

    return true;
}

} // namespace baremetal