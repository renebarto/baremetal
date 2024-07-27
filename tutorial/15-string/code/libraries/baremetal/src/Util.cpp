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
/// Compare two regions of memory
/// </summary>
/// <param name="buffer1">Pointer to first memory buffer</param>
/// <param name="buffer2">Pointer to second memory buffer</param>
/// <param name="length">Number of bytes to compare</param>
/// <returns>Returns 0 if the two regions are equal, 
/// 1 if the values in the first buffer are greater, 
/// -1 if the values in the second buffer are greater</returns>
int memcmp(const void* buffer1, const void* buffer2, size_t length)
{
    const unsigned char* p1 = reinterpret_cast<const unsigned char*>(buffer1);
    const unsigned char* p2 = reinterpret_cast<const unsigned char*>(buffer2);

    while (length-- > 0)
    {
        if (*p1 > *p2)
        {
            return 1;
        }
        else if (*p1 < *p2)
        {
            return -1;
        }

        p1++;
        p2++;
    }

    return 0;
}

/// <summary>
/// Convert character to upper case
/// </summary>
/// <param name="c">Character</param>
/// <returns>Uper case version of character c, if alphabetic, otherwise c</returns>
int toupper(int c)
{
    if (('a' <= c) && (c <= 'z'))
    {
        c -= 'a' - 'A';
    }

    return c;
}

/// <summary>
/// Convert character to lower case
/// </summary>
/// <param name="c">Character</param>
/// <returns>Lower case version of character c, if alphabetic, otherwise c</returns>
int tolower(int c)
{
    if (('A' <= c) && (c <= 'Z'))
    {
        c += 'a' - 'A';
    }

    return c;
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
/// Compare two strings
/// </summary>
/// <param name="str1">Pointer to first string</param>
/// <param name="str2">Pointer to second string</param>
/// <returns>Returns 0 if the two strings are equal, 
/// 1 if the values in the first string are greater, 
/// -1 if the values in the second string are greater</returns>
int strcmp(const char* str1, const char* str2)
{
    while ((*str1 != '\0') && (*str2 != '\0'))
    {
        if (*str1 > *str2)
        {
            return 1;
        }
        else if (*str1 < *str2)
        {
            return -1;
        }

        str1++;
        str2++;
    }

    if (*str1 > *str2)
    {
        return 1;
    }
    else if (*str1 < *str2)
    {
        return -1;
    }

    return 0;
}

/// <summary>
/// Compare two strings, case insensitive
/// </summary>
/// <param name="str1">Pointer to first string</param>
/// <param name="str2">Pointer to second string</param>
/// <returns>Returns 0 is the two strings are equal ignoring case, 
/// 1 if the values in the first string are greater, 
/// -1 if the values in the second string are greater</returns>
int strcasecmp(const char* str1, const char* str2)
{
    int chr1, chr2;

    while (((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
    {
        if (chr1 > chr2)
        {
            return 1;
        }
        else if (chr1 < chr2)
        {
            return -1;
        }

        str1++;
        str2++;
    }

    chr2 = toupper(*str2);

    if (chr1 > chr2)
    {
        return 1;
    }
    else if (chr1 < chr2)
    {
        return -1;
    }

    return 0;
}

/// <summary>
/// Compare two strings, up to maxLen characters
/// </summary>
/// <param name="str1">Pointer to first string</param>
/// <param name="str2">Pointer to second string</param>
/// <param name="maxLen">Maximum number of characters to compare</param>
/// <returns>Returns 0 is the two strings are equal, 
/// 1 if the values in the first string are greater, 
/// -1 if the values in the second string are greater</returns>
int strncmp(const char* str1, const char* str2, size_t maxLen)
{
    while ((maxLen > 0) && (*str1 != '\0') && (*str2 != '\0'))
    {
        if (*str1 > *str2)
        {
            return 1;
        }
        else if (*str1 < *str2)
        {
            return -1;
        }

        maxLen--;
        str1++;
        str2++;
    }

    if (maxLen == 0)
    {
        return 0;
    }

    if (*str1 > *str2)
    {
        return 1;
    }
    else if (*str1 < *str2)
    {
        return -1;
    }

    return 0;
}

/// <summary>
/// Compare two strings, up to maxLen characters, case insensitive
/// </summary>
/// <param name="str1">Pointer to first string</param>
/// <param name="str2">Pointer to second string</param>
/// <param name="maxLen">Maximum number of characters to compare</param>
/// <returns>Returns 0 is the two strings are equal ignoring case, 
/// 1 if the values in the first string are greater, 
/// -1 if the values in the second string are greater</returns>
int strncasecmp(const char* str1, const char* str2, size_t maxLen)
{
    int chr1, chr2;

    while ((maxLen > 0) && ((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
    {
        if (chr1 > chr2)
        {
            return 1;
        }
        else if (chr1 < chr2)
        {
            return -1;
        }

        maxLen--;
        str1++;
        str2++;
    }

    chr2 = toupper(*str2);

    if (maxLen == 0)
    {
        return 0;
    }

    if (chr1 > chr2)
    {
        return 1;
    }
    else if (chr1 < chr2)
    {
        return -1;
    }

    return 0;
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
