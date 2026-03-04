//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : ISPIMaster.h
//
// Namespace   : baremetal
//
// Class       : SPIMaster
//
// Description : SPI Master abstract interface
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

#pragma once

#include "stdlib/Types.h"

/// @file
/// SPI Master abstract interface

namespace baremetal {

#if BAREMETAL_RPI_TARGET <= 4

/// <summary>
/// Chip select / chip enable index
/// </summary>
enum class SPI_CEIndex : uint8
{
    CE0 = 0,
    CE1 = 1,
    CE2 = 2,
    None = 3
};

/// <summary>
/// SPIMaster abstract interface. Can be inherited for creating a mock
/// </summary>
class ISPIMaster
{
public:
    /// <summary>
    /// Default destructor needed for abstract class
    /// </summary>
    virtual ~ISPIMaster() = default;

    /// <summary>
    /// Set SPI clock rate in Hz. The actual clock rate may be different depending on the hardware capabilities and the core clock rate, but it should
    /// be the closest possible to the requested clock rate without exceeding it.
    /// </summary>
    /// <param name="clockRate">Requested clock rate</param>
    virtual void SetClock(uint32 clockRate) = 0;

    /// <summary>
    /// Read bytes into buffer
    /// </summary>
    /// <param name="ceIndex">Index of CE/CS signal to activate</param>
    /// <param name="buffer">Pointer to buffer to store data received</param>
    /// <param name="count">Requested byte count for read</param>
    /// <returns>Number of bytes actually read, or negative if an error occurs</returns>
    virtual size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) = 0;
    /// <summary>
    /// Write bytes to device
    /// </summary>
    /// <param name="ceIndex">Index of CE/CS signal to activate</param>
    /// <param name="buffer">Pointer to buffer containing data to be sent</param>
    /// <param name="count">Requested byte count for write</param>
    /// <returns>Number of bytes actually written, or negative if an error occurs</returns>
    virtual size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) = 0;
    /// <summary>
    /// Write then read from device
    /// </summary>
    /// <param name="ceIndex">Index of CE/CS signal to activate</param>
    /// <param name="writeBuffer">Pointer to buffer containing data to be sent</param>
    /// <param name="readBuffer">Pointer to buffer to store data received</param>
    /// <param name="count">Requested byte count to read / write</param>
    /// <returns>Number of bytes actually written / read, or negative if an error occurs</returns>
    virtual size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) = 0;
};

#else

#error RPI 5 not supported yet

#endif

} // namespace baremetal
