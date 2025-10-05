//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Malloc.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Memory allocation functions
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

#include "baremetal/Malloc.h"

#include "baremetal/MemoryManager.h"
#include "baremetal/SysConfig.h"

/// @file
/// Standard C memory allocation functions implementation

/// <summary>
/// Allocates a block of memory of the desired size.
/// </summary>
/// <param name="size">The desired size of the memory block</param>
/// <returns></returns>
void* malloc(size_t size)
{
    return baremetal::MemoryManager::HeapAllocate(size, HEAP_DEFAULT_MALLOC);
}

/// <summary>
/// Allocates a contiguous block of memory for the desired number of cells of the desired size each.
///
/// The memory allocated is num x size bytes
/// </summary>
/// <param name="num">Number of cells to allocate memory for</param>
/// <param name="size">Size of each cell</param>
/// <returns></returns>
void* calloc(size_t num, size_t size)
{
    return malloc(num * size);
}

/// <summary>
/// Re-allocates memory previously allocated with malloc() or calloc() to a new size
/// </summary>
/// <param name="ptr">Pointer to memory block to be re-allocated</param>
/// <param name="new_size">The desired new size of the memory block</param>
/// <returns></returns>
void* realloc(void* ptr, size_t new_size)
{
    return baremetal::MemoryManager::HeapReAllocate(ptr, new_size);
}

/// <summary>
/// Frees memory previously allocated with malloc() or calloc()
/// </summary>
/// <param name="ptr">Pointer to memory block to be freed</param>
void free(void* ptr)
{
    baremetal::MemoryManager::HeapFree(ptr);
}
