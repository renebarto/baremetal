//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

#include <baremetal/Types.h>

/// @file
/// Memory access interface

namespace baremetal {

/// @brief Memory access interface
///
/// Abstraction of memory access for 8/16/32 bit unsigned integer types
class IMemoryAccess
{
public:
    /// @brief Virtual destructor. Always needs to be defined for abstract classes
    virtual ~IMemoryAccess() = default;

    /// @brief Read a unsigned 8 bit integer
    /// @param address Memory location to read from
    /// @return Read unsigned 8 bit value
    virtual uint8  Read8(uintptr address)                                                    = 0;
    /// @brief Write a unsigned 8 bit integer
    /// @param address Memory location to write to
    /// @param data    Unsigned 8 bit value to write
    virtual void   Write8(uintptr address, uint8 data)                                       = 0;
    /// @brief Read, modify and write a unsigned 8 bit integer
    /// 
    /// Writes the value (data << shift & ~mask) | (read(address) & mask)
    /// @param address Memory location to write to
    /// @param mask    Mask value to be used for value read (and inverted for data written)
    /// @param data    Unsigned 8 bit value to write, after shifting
    /// @param shift   Bit shift imposed upon data value before writing
    virtual void   ReadModifyWrite8(uintptr address, uint8 mask, uint8 data, uint8 shift)    = 0;
    /// @brief Read a unsigned 16 bit integer
    /// @param address Memory location to read from
    /// @return Read unsigned 16 bit value
    virtual uint16 Read16(uintptr address)                                                   = 0;
    /// @brief Write a unsigned 16 bit integer
    /// @param address Memory location to write to
    /// @param data    Unsigned 16 bit value to write
    virtual void   Write16(uintptr address, uint16 data)                                     = 0;
    /// @brief Read, modify and write a unsigned 16 bit integer
    /// 
    /// Writes the value (data << shift & ~mask) | (read(address) & mask)
    /// @param address Memory location to write to
    /// @param mask    Mask value to be used for value read (and inverted for data written)
    /// @param data    Unsigned 16 bit value to write, after shifting
    /// @param shift   Bit shift imposed upon data value before writing
    virtual void   ReadModifyWrite16(uintptr address, uint16 mask, uint16 data, uint8 shift) = 0;
    /// @brief Read a unsigned 32 bit integer
    /// @param address Memory location to read from
    /// @return Read unsigned 32 bit value
    virtual uint32 Read32(uintptr address)                                                   = 0;
    /// @brief Write a unsigned 32 bit integer
    /// @param address Memory location to write to
    /// @param data    Unsigned 32 bit value to write
    virtual void   Write32(uintptr address, uint32 data)                                     = 0;
    /// @brief Read, modify and write a unsigned 32 bit integer
    /// 
    /// Writes the value (data << shift & ~mask) | (read(address) & mask)
    /// @param address Memory location to write to
    /// @param mask    Mask value to be used for value read (and inverted for data written)
    /// @param data    Unsigned 32 bit value to write, after shifting
    /// @param shift   Bit shift imposed upon data value before writing
    virtual void   ReadModifyWrite32(uintptr address, uint32 mask, uint32 data, uint8 shift) = 0;
};

} // namespace baremetal
