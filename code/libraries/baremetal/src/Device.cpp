//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Device.cpp
//
// Namespace   : baremetal
//
// Class       : Device
//
// Description : Generic device interface
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
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

#include "baremetal/Device.h"

using namespace baremetal;

/// @file
/// Generic device

/// <summary>
/// Read a specified number of bytes from the device into a buffer
/// </summary>
/// <param name="buffer">Buffer, where read data will be placed</param>
/// <param name="count">Maximum number of bytes to be read</param>
/// <returns>Number of read bytes or < 0 on failure</returns>
ssize_t Device::Read(void* buffer, size_t count)
{
    return static_cast<ssize_t>(-1);
}

/// <summary>
/// Write a specified number of bytes to the device
/// </summary>
/// <param name="buffer">Buffer, from which data will be fetched for write</param>
/// <param name="count">Number of bytes to be written</param>
/// <returns>Number of written bytes or < 0 on failure</returns>
ssize_t Device::Write(const void* buffer, size_t count)
{
    return static_cast<ssize_t>(-1);
}

/// <summary>
/// Flush any buffers for device
/// </summary>
void Device::Flush()
{
    // Do nothing
}

/// <summary>
/// Seek to a specified offset in the device file.
///
/// This is only supported by block devices.
/// </summary>
/// <param name="offset">Byte offset from start</param>
/// <returns>The resulting offset, (ssize_t) -1 on error</returns>
ssize_t Device::Seek(size_t offset)
{
    return static_cast<uint64>(-1);
}

/// <summary>
/// Get size for a device file
///
/// This is only supported by block devices.
/// </summary>
/// <returns>Total byte size of a block device, (ssize_t) -1 on error</returns>
ssize_t Device::GetSize() const
{
    return static_cast<ssize_t>(-1);
}
