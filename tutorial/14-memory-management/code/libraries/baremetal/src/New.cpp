//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : New.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Generic memory allocation functions
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

#include <baremetal/New.h>

#include <baremetal/SysConfig.h>

/// @file
/// Basic memory allocation functions implementation

using namespace baremetal;

/// <summary>
/// Class specific placement allocation for single value.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <param name="type">Heap type to allocate from</param>
/// <returns>Pointer to allocated block of memory or nullptr</returns>
void* operator new (size_t size, HeapType type)
{
    return MemoryManager::HeapAllocate(size, type);
}

/// <summary>
/// Class specific placement allocation for array.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <param name="type">Heap type to allocate from</param>
/// <returns>Pointer to allocated block of memory or nullptr</returns>
void* operator new[](size_t size, HeapType type)
{
    return MemoryManager::HeapAllocate(size, type);
}

/// <summary>
/// Non allocating placement allocation for single value.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <param name="address">Address to be used</param>
/// <returns>Pointer to block of memory</returns>
void* operator new (size_t size, void* address)
{
    return address;
}

/// <summary>
/// Non allocating placement allocation for array.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <param name="address">Address to be used</param>
/// <returns>Pointer to block of memory</returns>
void* operator new[](size_t size, void* address)
{
    return address;
}

/// <summary>
/// Standard allocation for single value.
///
/// Allocates from default heap.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <returns>Pointer to allocated block of memory or nullptr</returns>
void* operator new (size_t size)
{
    return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
}

/// <summary>
/// Standard allocation for array.
///
/// Allocates from default heap.
/// </summary>
/// <param name="size">Size of block to allocate in bytes</param>
/// <returns>Pointer to allocated block of memory or nullptr</returns>
void* operator new[](size_t size)
{
    return MemoryManager::HeapAllocate(size, HEAP_DEFAULT_NEW);
}

/// <summary>
/// Standard de-allocation for single value.
/// </summary>
/// <param name="address">Block to free</param>
void operator delete (void* address) noexcept
{
    MemoryManager::HeapFree(address);
}

/// <summary>
/// Standard de-allocation for array.
/// </summary>
/// <param name="address">Block to free</param>
void operator delete[](void* address) noexcept
{
    MemoryManager::HeapFree(address);
}

/// <summary>
/// Standard de-allocation with size for single value.
/// </summary>
/// <param name="address">Block to free</param>
void operator delete (void* address, size_t /*size*/) noexcept
{
    MemoryManager::HeapFree(address);
}

/// <summary>
/// Standard de-allocation for array.
/// </summary>
/// <param name="address">Block to free</param>
//// <param name="size">Size of block to free</param>
void operator delete[](void* address, size_t /*size*/) noexcept
{
    MemoryManager::HeapFree(address);
}
