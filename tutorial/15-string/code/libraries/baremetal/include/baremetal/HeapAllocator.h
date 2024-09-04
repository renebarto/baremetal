//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : HeapAllocator.h
//
// Namespace   : baremetal
//
// Class       : HeapAllocator
//
// Description : Heap allocation
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

#include <baremetal/Macros.h>
#include <baremetal/Synchronization.h>
#include <baremetal/Types.h>

/// @file
/// Heap allocator

namespace baremetal {

/// @brief Block alignment
#define HEAP_BLOCK_ALIGN       DATA_CACHE_LINE_LENGTH_MAX
/// @brief Block alignment mask
#define HEAP_ALIGN_MASK        (HEAP_BLOCK_ALIGN - 1)

/// @brief Maximum number of heap buckets used
#define HEAP_BLOCK_MAX_BUCKETS 20

/// <summary>
/// Administration on an allocated block of memory
/// </summary>
struct HeapBlockHeader
{
    /// @brief Heap block magic number
    uint32 magic;
    /// @brief Heap block magic number
#define HEAP_BLOCK_MAGIC 0x424C4D43
    /// @brief Size of allocated block
    uint32           size;
    /// @brief Pointer to next header
    HeapBlockHeader *next;
    /// @brief Padding to align to HEAP_BLOCK_ALIGN bytes
    uint8            align[HEAP_BLOCK_ALIGN - 16];
    /// @brief Start of actual allocated block
    uint8            data[0];
}
/// @brief Just specifies the struct is packed
PACKED;

/// <summary>
/// Bucket containing administration on allocated blocks of memory
/// </summary>
struct HeapBlockBucket
{
    /// @brief Size of bucket (size actual memory allocated, excluding bucket header)
    uint32 size;
#if BAREMETAL_MEMORY_TRACING
    /// @brief Count of blocks allocated in bucket
    unsigned count;
    /// @brief Maximum count of blocks allocated in bucket over time
    unsigned maxCount;
    /// @brief Total number of blocks allocated in bucket over time
    uint64   totalAllocatedCount;
    /// @brief Total number of bytes allocated in bucket over time
    uint64   totalAllocated;
    /// @brief Total number of blocks freed in bucket over time
    uint64   totalFreedCount;
    /// @brief Total number of bytes freed in bucket over time
    uint64   totalFreed;
#endif
    /// @brief List of free blocks in bucket to be re-used
    HeapBlockHeader *freeList;
};

/// <summary>
/// Allocates blocks from a flat memory region
/// </summary>
class HeapAllocator
{
private:
    /// @brief Name of the heap
    const char*     m_heapName;
    /// @brief Next available address
    uint8*          m_next;
    /// @brief End of available address space
    uint8*          m_limit;
    /// @brief Reserved address space
    size_t          m_reserve;
    /// @brief Allocated bucket administration
    HeapBlockBucket m_buckets[HEAP_BLOCK_MAX_BUCKETS + 1];

    /// @brief Sizes of allocation buckets
    static uint32   s_bucketSizes[];

public:
    /// <summary>
    /// Constructs a heap allocator
    /// </summary>
    /// <param name="heapName">Name of the heap for debugging purpose (must be static)</param>
    explicit HeapAllocator(const char *heapName = "heap");

    void Setup(uintptr baseAddress, size_t size, size_t reserve);

    size_t GetFreeSpace() const;
    void *Allocate(size_t size);
    void *ReAllocate(void *block, size_t size);
    void Free(void *block);

#if BAREMETAL_MEMORY_TRACING
    void   DumpStatus();

    uint64 GetCurrentAllocatedBlockCount();
    uint64 GetCurrentAllocationSize();
    uint64 GetMaxAllocatedBlockCount();
    uint64 GetTotalAllocatedBlockCount();
    uint64 GetTotalFreedBlockCount();
    uint64 GetTotalAllocationSize();
    uint64 GetTotalFreeSize();
#endif
};

} // namespace baremetal
