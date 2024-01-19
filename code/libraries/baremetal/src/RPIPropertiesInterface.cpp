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

namespace baremetal {

struct PropertySimple
{
    Property tag;
    uint32   value;
} PACKED;

RPIPropertiesInterface::RPIPropertiesInterface(IMailbox &mailbox)
    : m_mailbox{mailbox}
{
}

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

bool RPIPropertiesInterface::CheckTagResult(void *tag)
{
    Property *header = reinterpret_cast<Property *>(tag);

    if ((header->tagRequestResponse & RPI_MAILBOX_TAG_RESPONSE) == 0)
        return false;

    header->tagRequestResponse &= ~RPI_MAILBOX_TAG_RESPONSE;
    return (header->tagRequestResponse != 0);
}

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