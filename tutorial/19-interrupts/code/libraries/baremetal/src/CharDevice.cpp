//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : CharDevice.cpp
//
// Namespace   : baremetal
//
// Class       : CharDevice
//
// Description : Abstract character read / write device
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

#include "baremetal/CharDevice.h"

using namespace baremetal;

/// @file
/// Abstract character device

/// <summary>
/// Read a specified number of bytes from the device into a buffer
/// </summary>
/// <param name="buffer">Buffer, where read data will be placed</param>
/// <param name="count">Maximum number of bytes to be read</param>
/// <returns>Number of read bytes or < 0 on failure</returns>
ssize_t CharDevice::Read(void* buffer, size_t count)
{
    if (buffer == nullptr)
        return static_cast<ssize_t>(-1);
    char* bufferPtr = reinterpret_cast<char*>(buffer);
    for (size_t i = 0; i < count; ++i)
        *bufferPtr++ = Read();
    return count;
}

/// <summary>
/// Write a specified number of bytes to the device
/// </summary>
/// <param name="buffer">Buffer, from which data will be fetched for write</param>
/// <param name="count">Number of bytes to be written</param>
/// <returns>Number of written bytes or < 0 on failure</returns>
ssize_t CharDevice::Write(const void* buffer, size_t count)
{
    if (buffer == nullptr)
        return static_cast<ssize_t>(-1);
    const char* bufferPtr = reinterpret_cast<const char*>(buffer);
    for (size_t i = 0; i < count; ++i)
    {
        // convert newline to carriage return + newline
        if (*bufferPtr == '\n')
            Write('\r');
        Write(*bufferPtr++);
    }
    return count;
}
