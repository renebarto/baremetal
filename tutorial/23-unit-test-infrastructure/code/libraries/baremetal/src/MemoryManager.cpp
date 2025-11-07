//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "baremetal/MemoryManager.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/MachineInfo.h"
#include "baremetal/SysConfig.h"

/// @file
/// Memory management implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("MemoryManager");

/// <summary>
/// Constructs a MemoryManager instance
///
/// Retrieves amount of physical RAM available, and sets up heap managers for low (below 1Gb) and high (above 3 Gb, only Raspberry Pi 4 or higher)
/// memory.
/// </summary>
MemoryManager::MemoryManager()
    : m_memSize{}
    , m_memSizeHigh{}
    , m_heapLow{"heaplow"}
#if BAREMETAL_RPI_TARGET >= 4
    , m_heapHigh{"heaphigh"}
#endif
{
    MachineInfo& machineInfo = GetMachineInfo();
    machineInfo.Initialize();
    auto baseAddress = machineInfo.GetARMMemoryBaseAddress();
    auto size = machineInfo.GetARMMemorySize();
    if (size == 0)
    {
        baseAddress = 0;
        size = ARM_MEM_SIZE;
    }

    assert(baseAddress == 0);
    m_memSize = size;

    size_t blockReserve = m_memSize - MEM_HEAP_START - PAGE_RESERVE;
    m_heapLow.Setup(MEM_HEAP_START, blockReserve, 0x40000);

#if BAREMETAL_RPI_TARGET >= 4
    auto ramSize = machineInfo.GetRAMSize();
    if (ramSize > 1024)
    {
        uint64 highSize = (ramSize - 1024) * MEGABYTE;
        if (highSize > MEM_HIGHMEM_END + 1 - MEM_HIGHMEM_START)
        {
            highSize = MEM_HIGHMEM_END + 1 - MEM_HIGHMEM_START;
        }

        m_memSizeHigh = static_cast<size_t>(highSize);

        m_heapHigh.Setup(MEM_HIGHMEM_START, m_memSizeHigh, 0);
    }
#endif
}

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

/// <summary>
/// Allocate memory from the specified heap
/// </summary>
/// <param name="size">Size of block to allocate</param>
/// <param name="type">Heap type to allocate from</param>
/// <returns>Pointer to allocated block of memory, or nullptr on failure</returns>
void* MemoryManager::HeapAllocate(size_t size, HeapType type)
{
    auto& memoryManager = GetMemoryManager();
#if BAREMETAL_RPI_TARGET >= 4
    void* block;

    switch (type)
    {
    case HeapType::LOW:
        return memoryManager.m_heapLow.Allocate(size);
    case HeapType::HIGH:
        return memoryManager.m_heapHigh.Allocate(size);
    case HeapType::ANY:
        return (block = memoryManager.m_heapHigh.Allocate(size)) != nullptr ? block : memoryManager.m_heapLow.Allocate(size);
    default:
        return nullptr;
    }
#else
    switch (type)
    {
    case HeapType::LOW:
    case HeapType::ANY:
        return memoryManager.m_heapLow.Allocate(size);
    default:
        return nullptr;
    }
#endif
}

/// <summary>
/// Reallocate block of memory
/// </summary>
/// <param name="block">Block of memory to be reallocated to the new size</param>
/// <param name="size">Block size to be allocated</param>
/// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
void* MemoryManager::HeapReAllocate(void* block, size_t size) // block may be nullptr
{
    auto& memoryManager = GetMemoryManager();
#if BAREMETAL_RPI_TARGET >= 4
    if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
    {
        return memoryManager.m_heapLow.ReAllocate(block, size);
    }
    return memoryManager.m_heapHigh.ReAllocate(block, size);
#else
    return memoryManager.m_heapLow.ReAllocate(block, size);
#endif
}

/// <summary>
/// Free (de-allocate) block of memory.
/// </summary>
/// <param name="block">Memory block to be freed</param>
void MemoryManager::HeapFree(void* block)
{
    auto& memoryManager = GetMemoryManager();
#if BAREMETAL_RPI_TARGET >= 4
    if (reinterpret_cast<uintptr>(block) < MEM_HIGHMEM_START)
    {
        memoryManager.m_heapLow.Free(block);
    }
    else
    {
        memoryManager.m_heapHigh.Free(block);
    }
#else
    memoryManager.m_heapLow.Free(block);
#endif
}

/// <summary>
/// Calculate and return the amount of free (unallocated) space for the specified heap
/// </summary>
/// <param name="type">Heap to return free space for</param>
/// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
size_t MemoryManager::GetHeapFreeSpace(HeapType type)
{
    auto& memoryManager = GetMemoryManager();
#if BAREMETAL_RPI_TARGET >= 4
    switch (type)
    {
    case HeapType::LOW:
        return memoryManager.m_heapLow.GetFreeSpace();
    case HeapType::HIGH:
        return memoryManager.m_heapHigh.GetFreeSpace();
    case HeapType::ANY:
        return memoryManager.m_heapLow.GetFreeSpace() + memoryManager.m_heapHigh.GetFreeSpace();
    default:
        return 0;
    }
#else
    switch (type)
    {
    case HeapType::LOW:
    case HeapType::ANY:
        return memoryManager.m_heapLow.GetFreeSpace();
    default:
        return 0;
    }
#endif
}

/// <summary>
/// Display the current status of all heap allocators
/// </summary>
void MemoryManager::DumpStatus()
{
#if BAREMETAL_MEMORY_TRACING
    auto& memoryManager = GetMemoryManager();
    LOG_DEBUG("Low heap:");
    memoryManager.m_heapLow.DumpStatus();
#if BAREMETAL_RPI_TARGET >= 4
    LOG_DEBUG("High heap:");
    memoryManager.m_heapHigh.DumpStatus();
#endif
#endif
}

/// <summary>
/// Construct the singleton MemoryManager instance if needed, and return a reference to the instance
/// </summary>
/// <returns>Reference to the singleton MemoryManager</returns>
MemoryManager& baremetal::GetMemoryManager()
{
    static MemoryManager instance;
    return instance;
}
