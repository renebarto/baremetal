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

#include "stdlib/Types.h"

/// @file
/// Abstract memory access interface

namespace baremetal {

/// <summary>
/// Abstract memory access interface
/// </summary>
class IMemoryAccess
{
public:
    /// <summary>
    /// Default destructor needed for abstract interface
    /// </summary>
    virtual ~IMemoryAccess() = default;

    /// <summary>
    /// Read a 8 bit value from register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <returns>8 bit register value</returns>
    virtual uint8 Read8(regaddr address) = 0;
    /// <summary>
    /// Write a 8 bit value to register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="data">Data to write</param>
    virtual void Write8(regaddr address, uint8 data) = 0;
    /// <summary>
    /// Read, modify and write a 8 bit value to register at address
    ///
    /// The operation will read the value from the specified register, then AND it with the inverse of the mask (8 bits) provided
    /// The data provided (8 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
    /// The result will then be written back to the register
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
    /// <param name="data">Data to write (after shifting left by shift bits)</param>
    /// <param name="shift">Shift to apply to the data to write (shift left)</param>
    virtual void ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift) = 0;
    /// <summary>
    /// Read a 16 bit value from register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <returns>16 bit register value</returns>
    virtual uint16 Read16(regaddr address) = 0;
    /// <summary>
    /// Write a 16 bit value to register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="data">Data to write</param>
    virtual void Write16(regaddr address, uint16 data) = 0;
    /// <summary>
    /// Read, modify and write a 16 bit value to register at address
    ///
    /// The operation will read the value from the specified register, then AND it with the inverse of the mask (16 bits) provided
    /// The data provided (16 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
    /// The result will then be written back to the register
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
    /// <param name="data">Data to write (after shifting left by shift bits)</param>
    /// <param name="shift">Shift to apply to the data to write (shift left)</param>
    virtual void ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) = 0;
    /// <summary>
    /// Read a 32 bit value from register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <returns>32 bit register value</returns>
    virtual uint32 Read32(regaddr address) = 0;
    /// <summary>
    /// Write a 32 bit value to register at address
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="data">Data to write</param>
    virtual void Write32(regaddr address, uint32 data) = 0;
    /// <summary>
    /// Read, modify and write a 32 bit value to register at address
    ///
    /// The operation will read the value from the specified register, then AND it with the inverse of the mask (32 bits) provided
    /// The data provided (32 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
    /// The result will then be written back to the register
    /// </summary>
    /// <param name="address">Address of register</param>
    /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
    /// <param name="data">Data to write (after shifting left by shift bits)</param>
    /// <param name="shift">Shift to apply to the data to write (shift left)</param>
    virtual void ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) = 0;
};

} // namespace baremetal
