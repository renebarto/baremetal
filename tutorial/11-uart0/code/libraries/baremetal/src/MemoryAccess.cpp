//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccess.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccess
//
// Description : Memory read/write
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

#include "baremetal/MemoryAccess.h"

/// @file
/// Memory access class implementation

using namespace baremetal;

/// <summary>
/// Read a 8 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>8 bit register value</returns>
uint8 MemoryAccess::Read8(regaddr address)
{
    return *reinterpret_cast<uint8 volatile*>(address);
}

/// <summary>
/// Write a 8 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccess::Write8(regaddr address, uint8 data)
{
    *reinterpret_cast<uint8 volatile*>(address) = data;
}

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
void MemoryAccess::ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)
{
    auto value = Read8(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write8(address, value);
}

/// <summary>
/// Read a 16 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>16 bit register value</returns>
uint16 MemoryAccess::Read16(regaddr address)
{
    return *reinterpret_cast<uint16 volatile*>(address);
}

/// <summary>
/// Write a 16 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccess::Write16(regaddr address, uint16 data)
{
    *reinterpret_cast<uint16 volatile*>(address) = data;
}

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
void MemoryAccess::ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift)
{
    auto value = Read16(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write16(address, value);
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccess::Read32(regaddr address)
{
    return *reinterpret_cast<uint32 volatile*>(address);
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccess::Write32(regaddr address, uint32 data)
{
    *reinterpret_cast<uint32 volatile*>(address) = data;
}

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
void MemoryAccess::ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift)
{
    auto value = Read32(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write32(address, value);
}

/// <summary>
/// Construct the singleton memory access interface if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton memory access interface</returns>
MemoryAccess& baremetal::GetMemoryAccess()
{
    static MemoryAccess value;
    return value;
}
