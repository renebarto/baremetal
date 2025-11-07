//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HeapAllocator.cpp
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

#include "baremetal/HeapAllocator.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/SysConfig.h"
#include "stdlib/Util.h"

/// @file
/// Heap allocator implementation

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("HeapAllocator");

uint32 HeapAllocator::s_bucketSizes[] = {HEAP_BLOCK_BUCKET_SIZES};

/// <summary>
/// Constructs a heap allocator
/// </summary>
/// <param name="heapName">Name of the heap for debugging purpose (must be static)</param>
HeapAllocator::HeapAllocator(const char* heapName)
    : m_heapName{heapName}
    , m_next{}
    , m_limit{}
    , m_reserve{}
    , m_buckets{}
{
    memset(m_buckets, 0, sizeof(m_buckets));

    size_t numBuckets = sizeof(s_bucketSizes) / sizeof(s_bucketSizes[0]);
    if (numBuckets > HEAP_BLOCK_MAX_BUCKETS)
    {
        numBuckets = HEAP_BLOCK_MAX_BUCKETS;
    }

    for (size_t i = 0; i < numBuckets; ++i)
    {
        m_buckets[i].size = s_bucketSizes[i];
    }
}

/// <summary>
/// Sets up the heap allocator
/// </summary>
/// <param name="baseAddress">Base address of memory region (must be 16 bytes aligned)</param>
/// <param name="size">Size of memory region</param>
/// <param name="reserve">Free space reserved for handling of "Out of memory" messages.
/// (Allocate() returns nullptr, if reserve is 0 and memory region is full)</param>
void HeapAllocator::Setup(uintptr baseAddress, size_t size, size_t reserve)
{
    m_next = reinterpret_cast<uint8*>(baseAddress);
    m_limit = reinterpret_cast<uint8*>(baseAddress + size);
    m_reserve = reserve;
#if BAREMETAL_MEMORY_TRACING
    DumpStatus();
#endif
}

/// <summary>
/// Calculate and return the amount of free (unallocated) space in all buckets
/// @note Unused blocks on a free list do not count here.
/// </summary>
/// <returns>Free space of the memory region, which is not allocated by blocks.</returns>
size_t HeapAllocator::GetFreeSpace() const
{
    return m_limit - m_next;
}

/// <summary>
/// Allocate a block of memory
/// \note Resulting block is always 16 bytes aligned
/// \note If reserve in Setup() is non-zero, the system panics if heap is full.
/// </summary>
/// <param name="size">Block size to be allocated</param>
/// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
void* HeapAllocator::Allocate(size_t size)
{
    if (m_next == nullptr)
    {
        return nullptr;
    }

    HeapBlockBucket* bucket;
    for (bucket = m_buckets; bucket->size > 0; bucket++)
    {
        if (size <= bucket->size)
        {
            size = bucket->size;

#if BAREMETAL_MEMORY_TRACING
            if (++bucket->count > bucket->maxCount)
            {
                bucket->maxCount = bucket->count;
            }
            ++bucket->totalAllocatedCount;
            bucket->totalAllocated += size;

#endif
            break;
        }
    }

    HeapBlockHeader* blockHeader{bucket->freeList};
    if ((bucket->size > 0) && (blockHeader != nullptr))
    {
        assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
        bucket->freeList = blockHeader->next;
#if BAREMETAL_MEMORY_TRACING_DETAIL
        TRACE_NO_ALLOC_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
        TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
#endif
    }
    else
    {
        blockHeader = reinterpret_cast<HeapBlockHeader*>(m_next);

        uint8* nextBlock = m_next;
        nextBlock += (sizeof(HeapBlockHeader) + size + HEAP_BLOCK_ALIGN - 1) & ~HEAP_ALIGN_MASK;

        if ((nextBlock <= m_next) || // may have wrapped
            (nextBlock > m_limit - m_reserve))
        {
#if BAREMETAL_MEMORY_TRACING
            DumpStatus();
#endif
            LOG_NO_ALLOC_ERROR("%s: Out of memory", m_heapName);
            return nullptr;
        }

        m_next = nextBlock;

        blockHeader->magic = HEAP_BLOCK_MAGIC;
        blockHeader->size = static_cast<uint32>(size);

#if BAREMETAL_MEMORY_TRACING_DETAIL
        TRACE_NO_ALLOC_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
        TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
#endif
    }

    blockHeader->next = nullptr;

    void* result = blockHeader->data;
    assert((reinterpret_cast<uintptr>(result) & HEAP_ALIGN_MASK) == 0);

    return result;
}

/// <summary>
/// Reallocate block of memory
/// </summary>
/// <param name="block">Block of memory to be reallocated to the new size</param>
/// <param name="size">Block size to be allocated</param>
/// <returns>Pointer to new allocated block (nullptr if heap is full or not set-up)</returns>
void* HeapAllocator::ReAllocate(void* block, size_t size)
{
    if (block == nullptr)
    {
        return Allocate(size);
    }

    if (size == 0)
    {
        Free(block);

        return nullptr;
    }

    const HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
    assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
    if (blockHeader->size >= size)
    {
        return block;
    }

    void* newBlock = Allocate(size);
    if (newBlock == nullptr)
    {
        return nullptr;
    }

    memcpy(newBlock, block, blockHeader->size);

    Free(block);

    return newBlock;
}

/// <summary>
/// Free (de-allocate) block of memory.
/// \note Memory space of blocks, which are bigger than the largest bucket size, cannot be returned to a free list and is lost.
/// </summary>
/// <param name="block">Memory block to be freed</param>
void HeapAllocator::Free(void* block)
{
    if (block == nullptr)
    {
        return;
    }

    HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
    assert(blockHeader->magic == HEAP_BLOCK_MAGIC);

    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; bucket++)
    {
        if (blockHeader->size == bucket->size)
        {
            blockHeader->next = bucket->freeList;
            bucket->freeList = blockHeader;

#if BAREMETAL_MEMORY_TRACING
            bucket->count--;
            ++bucket->totalFreedCount;
            bucket->totalFreed += blockHeader->size;
#if BAREMETAL_MEMORY_TRACING_DETAIL
            TRACE_NO_ALLOC_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
            TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
#endif
#endif

            return;
        }
    }

#if BAREMETAL_MEMORY_TRACING
    LOG_NO_ALLOC_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
#endif
}

#if BAREMETAL_MEMORY_TRACING
/// <summary>
/// Display the current status of the heap allocator
/// </summary>
void HeapAllocator::DumpStatus()
{
    TRACE_NO_ALLOC_DEBUG("Heap allocator info:     %s", m_heapName);
    TRACE_NO_ALLOC_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
    TRACE_NO_ALLOC_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
    TRACE_NO_ALLOC_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
    TRACE_NO_ALLOC_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
    TRACE_NO_ALLOC_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
    TRACE_NO_ALLOC_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
    TRACE_NO_ALLOC_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());

#if BAREMETAL_MEMORY_TRACING_DETAIL
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        TRACE_NO_ALLOC_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu",
                             bucket->size, bucket->count, bucket->maxCount, bucket->totalAllocatedCount, bucket->totalAllocated,
                             bucket->totalFreedCount, bucket->totalFreed);
    }
#endif
}

/// <summary>
/// Returns the number of currently allocated memory blocks for this heap allocator.
/// </summary>
/// <returns>Number of currently allocated memory blocks for this heap allocator</returns>
uint64 HeapAllocator::GetCurrentAllocatedBlockCount()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->count;
    }
    return total;
}

/// <summary>
/// Returns the total size of currently allocated memory blocks for this heap allocator.
/// </summary>
/// <returns>Total size of currently allocated memory blocks for this heap allocator</returns>
uint64 HeapAllocator::GetCurrentAllocationSize()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->count * bucket->size;
    }
    return total;
}

/// <summary>
/// Returns the maximum number of currently allocated memory blocks for this heap allocator over time.
/// </summary>
/// <returns>Maximum number of currently allocated memory blocks for this heap allocator over time</returns>
uint64 HeapAllocator::GetMaxAllocatedBlockCount()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->maxCount;
    }
    return total;
}

/// <summary>
/// Returns the total number of allocated memory blocks for this heap allocator over time.
/// </summary>
/// <returns>Total number of allocated memory blocks for this heap allocator over time</returns>
uint64 HeapAllocator::GetTotalAllocatedBlockCount()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->totalAllocatedCount;
    }
    return total;
}

/// <summary>
/// Returns the total number of freed memory blocks for this heap allocator over time.
/// </summary>
/// <returns>Total number of freed memory blocks for this heap allocator over time</returns>
uint64 HeapAllocator::GetTotalFreedBlockCount()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->totalFreedCount;
    }
    return total;
}

/// <summary>
/// Returns the total size of allocated memory blocks for this heap allocator over time.
/// </summary>
/// <returns>Total size of allocated memory blocks for this heap allocator over time</returns>
uint64 HeapAllocator::GetTotalAllocationSize()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->totalAllocated;
    }
    return total;
}

/// <summary>
/// Returns the total size of freed memory blocks for this heap allocator over time.
/// </summary>
/// <returns>Total size of freed memory blocks for this heap allocator over time</returns>
uint64 HeapAllocator::GetTotalFreeSize()
{
    uint64 total{};
    for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
    {
        total += bucket->totalFreed;
    }
    return total;
}

#endif
