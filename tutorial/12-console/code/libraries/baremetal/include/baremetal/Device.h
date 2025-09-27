//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Device.h
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

#pragma once

#include "stdlib/Types.h"

/// @file
/// Abstract device

namespace baremetal {

/// <summary>
/// Generic device interface
/// </summary>
class Device
{
public:
    virtual ~Device() = default;

    /// <summary>
    /// Determines whether the device is a block device.
    /// </summary>
    /// <returns>True if the device is a block device; otherwise, false.</returns>
    virtual bool IsBlockDevice() = 0;
    virtual ssize_t Read(void* buffer, size_t count);
    virtual ssize_t Write(const void* buffer, size_t count);
    virtual void Flush();

    virtual ssize_t Seek(size_t offset);
    virtual ssize_t GetSize() const;
};

} // namespace baremetal
