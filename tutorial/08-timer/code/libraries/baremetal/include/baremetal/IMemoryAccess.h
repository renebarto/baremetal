//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : IMemoryAccess.h
//
// Namespace   : baremetal
//
// Class       : IMemoryAccess
//
// Description : Memory read/write abstract interface
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

#include <baremetal/Types.h>

namespace baremetal {

class IMemoryAccess
{
public:
    virtual ~IMemoryAccess() = default;

    virtual uint8  Read8(regaddr address)                                                    = 0;
    virtual void   Write8(regaddr address, uint8 data)                                       = 0;
    virtual void   ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)    = 0;
    virtual uint16 Read16(regaddr address)                                                   = 0;
    virtual void   Write16(regaddr address, uint16 data)                                     = 0;
    virtual void   ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) = 0;
    virtual uint32 Read32(regaddr address)                                                   = 0;
    virtual void   Write32(regaddr address, uint32 data)                                     = 0;
    virtual void   ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) = 0;
};

} // namespace baremetal
