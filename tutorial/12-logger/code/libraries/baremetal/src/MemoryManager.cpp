//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : MemoryManager.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryManager
//
// Description : Memory handling
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
// 
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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

#include <baremetal/MemoryManager.h>

#include <baremetal/SysConfig.h>

/// @file
/// Memory management implementation

using namespace baremetal;

/// <summary>
/// Return the coherent memory page (allocated with the GPU) for the requested page slot
/// </summary>
/// <param name="slot">Page slot to return the address for</param>
/// <returns>Page slot coherent memory address</returns>
uintptr MemoryManager::GetCoherentPage(CoherentPageSlot slot)
{
    uint64 pageAddress = MEM_COHERENT_REGION;

    pageAddress += static_cast<uint32>(slot) * PAGE_SIZE;

    return pageAddress;
}
