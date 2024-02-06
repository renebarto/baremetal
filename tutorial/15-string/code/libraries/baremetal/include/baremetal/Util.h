//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Util.h
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

#pragma once

#include <baremetal/Types.h>

/// @file
/// Standard C library utility functions

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* buffer, int value, size_t length);
void* memcpy(void* dest, const void* src, size_t length);
int memcmp(const void* buffer1, const void* buffer2, size_t length);

int toupper(int c);
int tolower(int c);
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strcasecmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t maxLen);
int strncasecmp(const char* str1, const char* str2, size_t maxLen);
char* strncpy(char* dest, const char* src, size_t maxLen);
char* strncat(char* dest, const char* src, size_t maxLen);

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);
void free(void* ptr);

#ifdef __cplusplus
}
#endif

/// <summary>
/// Determine the number of bits needed to represent the specified value
/// </summary>
/// <param name="value">Value to check</param>
/// <returns>Number of bits used for value</returns>
inline constexpr unsigned NextPowerOf2Bits(size_t value)
{
    unsigned bitCount{ 0 };
    size_t temp = value;
    while (temp >= 1)
    {
        ++bitCount;
        temp >>= 1;
    }
    return bitCount;
}

/// <summary>
/// Determine the next power of 2 greater than or equal to the specified value
/// </summary>
/// <param name="value">Value to check</param>
/// <returns>Power of two greater or equal to value</returns>
inline constexpr size_t NextPowerOf2(size_t value)
{
    return 1 << NextPowerOf2Bits((value != 0) ? value - 1 : 0);
}
