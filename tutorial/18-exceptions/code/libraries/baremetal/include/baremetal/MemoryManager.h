//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryManager.h
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

#include "baremetal/HeapAllocator.h"
#include "stdlib/Types.h"

/// @file
/// Memory management

/// <summary>
/// Page slot for requesting coherent memory region
/// </summary>
enum class CoherentPageSlot
{
    /// @brief Coherent memory page slot for Raspberry Pi mailbox
    PropertyMailbox = 0,
};

/// <summary>
/// Type of heap for requested memory block
/// </summary>
enum class HeapType
{
    /// @brief Memory below 1 GB
    LOW = 0,
    /// @brief Memory above 1 GB
    HIGH = 1,
    /// @brief High memory (if available) or low memory (otherwise)
    ANY = 2,
    /// @brief 30-bit DMA-able memory
    DMA30 = LOW,
};

namespace baremetal {

/// <summary>
/// Handles memory allocation, re-allocation, and de-allocation for heap and paging memory, as well as assignment of coherent memory slots.
///
/// This is a singleton, in that it is not possible to create a default instance (GetMemoryManager() needs to be used for this).
/// </summary>
class MemoryManager
{
    /// <summary>
    /// Construct the singleton MemoryManager instance if needed, and return a reference to the instance. This is a friend function of class MemoryManager
    /// </summary>
    /// <returns>Reference to the singleton MemoryManager instance</returns>
    friend MemoryManager& GetMemoryManager();

private:
    /// @brief Total memory size below 1Gb
    size_t m_memSize;
    /// @brief Total memory size above 1Gb (up to 3 Gb boundary)
    size_t m_memSizeHigh;

    /// @brief Heap allocator for low memory (below 1Gb)
    HeapAllocator m_heapLow;
#if BAREMETAL_RPI_TARGET >= 4
    /// @brief Heap allocator for low memory (above 1Gb)
    HeapAllocator m_heapHigh;
#endif
    MemoryManager();

public:
    static uintptr GetCoherentPage(CoherentPageSlot slot);

    static void* HeapAllocate(size_t size, HeapType type);
    static void* HeapReAllocate(void* block, size_t size);
    static void HeapFree(void* block);
    static size_t GetHeapFreeSpace(HeapType type);
    static void DumpStatus();
};

MemoryManager& GetMemoryManager();

} // namespace baremetal
