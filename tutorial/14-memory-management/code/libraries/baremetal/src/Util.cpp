//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Util.cpp
//
// Namespace   : -
//
// Class       : -
//
// Description : Utility functions
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

#include <baremetal/Util.h>

#include <baremetal/MemoryManager.h>

/// @file
/// Standard C library utility functions implementation

/// <summary>
/// Standard C memset function. Fills memory pointed to by buffer with value bytes over length bytes
/// </summary>
/// <param name="buffer">Buffer pointer</param>
/// <param name="value">Value used for filling the buffer (only lower byte is used)</param>
/// <param name="length">Size of the buffer to fill in bytes</param>
/// <returns>Pointer to buffer</returns>
void* memset(void* buffer, int value, size_t length)
{
    uint8* ptr = reinterpret_cast<uint8*>(buffer);

    while (length-- > 0)
    {
        *ptr++ = static_cast<char>(value);
    }
    return buffer;
}

/// <summary>
/// Standard C memcpy function. Copies memory pointed to by src to buffer pointed to by dest over length bytes
/// </summary>
/// <param name="dest">Destination buffer pointer</param>
/// <param name="src">Source buffer pointer</param>
/// <param name="length">Size of buffer to copy in bytes</param>
/// <returns>Pointer to destination buffer</returns>
void* memcpy(void* dest, const void* src, size_t length)
{
    uint8* dstPtr = reinterpret_cast<uint8*>(dest);
    const uint8* srcPtr = reinterpret_cast<const uint8*>(src);

    while (length-- > 0)
    {
        *dstPtr++ = *srcPtr++;
    }
    return dest;
}

/// <summary>
/// Standard C strlen function. Calculates the length of a string, in other words the index to the first '\0' character
/// </summary>
/// <param name="str">String for which to calculate the length</param>
/// <returns>Length of the string</returns>
size_t strlen(const char* str)
{
    size_t result = 0;

    while (*str++)
    {
        result++;
    }

    return result;
}

/// <summary>
/// Standard C strncpy function. Copies a string, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
/// </summary>
/// <param name="dest">Pointer to destination buffer</param>
/// <param name="src">Pointer to source buffer</param>
/// <param name="maxLen">Maximum number of characters to copy</param>
/// <returns>Pointer to destination buffer</returns>
char* strncpy(char* dest, const char* src, size_t maxLen)
{
    char* p = dest;

    while (maxLen > 0)
    {
        if (*src == '\0')
        {
            break;
        }

        *p++ = *src++;
        maxLen--;
    }

    if (maxLen == 0)
    {
        p = dest + maxLen - 1;
    }
    if (p >= dest)
        *p = '\0';

    return dest;
}

/// <summary>
/// Standard C strncat function. Appends a string to the destination, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
/// </summary>
/// <param name="dest">Pointer to destination buffer</param>
/// <param name="src">Pointer to source buffer</param>
/// <param name="maxLen">Maximum number of characters to copy</param>
/// <returns>Pointer to destination buffer</returns>
char* strncat(char* dest, const char* src, size_t maxLen)
{
    char* p = dest;

    while ((maxLen > 0) && *p)
    {
        p++;
        maxLen--;
    }

    while ((maxLen > 0) && *src)
    {
        *p++ = *src++;
        maxLen--;
    }

    if (maxLen == 0)
    {
        p = dest + maxLen - 1;
    }
    if (p >= dest)
        *p = '\0';

    return dest;
}

/// <summary>
/// Allocates a block of memory of the desired size.
/// </summary>
/// <param name="size">The desired size of the memory block</param>
/// <returns></returns>
void* malloc(size_t size)
{
    return baremetal::MemoryManager::HeapAllocate(size, HeapType::ANY);
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
