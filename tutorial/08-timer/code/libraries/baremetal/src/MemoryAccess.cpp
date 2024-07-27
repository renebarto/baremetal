//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

#include <baremetal/MemoryAccess.h>

using namespace baremetal;

uint8 MemoryAccess::Read8(regaddr address)
{
    return *reinterpret_cast<uint8 volatile*>(address);
}

void MemoryAccess::Write8(regaddr address, uint8 data)
{
    *reinterpret_cast<uint8 volatile*>(address) = data;
}

void MemoryAccess::ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)
{
    auto value = Read8(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write8(address, value);
}

uint16 MemoryAccess::Read16(regaddr address)
{
    return *reinterpret_cast<uint16 volatile*>(address);
}

void MemoryAccess::Write16(regaddr address, uint16 data)
{
    *reinterpret_cast<uint16 volatile*>(address) = data;
}

void MemoryAccess::ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift)
{
    auto value = Read16(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write16(address, value);
}

uint32 MemoryAccess::Read32(regaddr address)
{
    return *address;
}

void MemoryAccess::Write32(regaddr address, uint32 data)
{
    *address = data;
}

void MemoryAccess::ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift)
{
    auto value = Read32(address);
    value &= ~mask;
    value |= ((data << shift) & mask);
    Write32(address, value);
}

MemoryAccess &baremetal::GetMemoryAccess()
{
    static MemoryAccess value;
    return value;
}
