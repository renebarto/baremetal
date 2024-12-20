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

/// @file
/// Memory access class implementation

using namespace baremetal;

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccess::Read32(regaddr address)
{
    return *reinterpret_cast<uint32 volatile *>(address);
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccess::Write32(regaddr address, uint32 data)
{
    *reinterpret_cast<uint32 volatile *>(address) = data;
}
