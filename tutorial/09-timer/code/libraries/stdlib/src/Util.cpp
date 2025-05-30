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

#include <stdlib/Util.h>

/// @file
/// Standard C library utility functions implementation

/// <summary>
/// Standard C memset function. Fills memory pointed to by buffer with value bytes over length bytes
/// </summary>
/// <param name="buffer">Buffer pointer</param>
/// <param name="value">Value used for filling the buffer (only lower byte is used)</param>
/// <param name="length">Size of the buffer to fill in bytes</param>
/// <returns>Pointer to buffer</returns>
void *memset(void *buffer, int value, size_t length)
{
    uint8 *ptr = reinterpret_cast<uint8 *>(buffer);

    while (length-- > 0)
    {
        *ptr++ = static_cast<char>(value);
    }
    return buffer;
}
