//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : II2CMaster.h
//
// Namespace   : baremetal
//
// Class       : I2CMaster
//
// Description : I2C Master abstract interface
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

#include "baremetal/I2CErrorCodes.h"
#include "stdlib/Types.h"

/// @file
/// I2C Master abstract interface

namespace baremetal {

#if BAREMETAL_RPI_TARGET <= 4

/// <summary>
/// I2C speed selection
/// </summary>
enum class I2CClockMode
{
    /// @brief I2C @ 100 KHz
    Normal,
    /// @brief I2C @ 400 KHz
    Fast,
    /// @brief I2C @ 1 MHz
    FastPlus,
};

/// <summary>
/// I2CMaster abstract interface. Can be inherited for creating a mock
/// </summary>
class II2CMaster
{
public:
    /// <summary>
    /// Default destructor needed for abstract class
    /// </summary>
    virtual ~II2CMaster() = default;

    /// <summary>
    /// Read a single byte
    /// </summary>
    /// <param name="address">I2C address</param>
    /// <param name="data">Data read</param>
    /// <returns>Number of bytes actually read. Should be 1 for successful read, 0 if failed, negative if an error occurs</returns>
    virtual size_t Read(uint16 address, uint8& data) = 0;
    /// <summary>
    /// Read multiple bytes into buffer
    /// </summary>
    /// <param name="address">I2C address</param>
    /// <param name="buffer">Pointer to buffer to store data received</param>
    /// <param name="count">Requested byte count for read</param>
    /// <returns>Number of bytes actually read, or negative if an error occurs</returns>
    virtual size_t Read(uint16 address, void* buffer, size_t count) = 0;
    /// <summary>
    /// Write a single byte
    /// </summary>
    /// <param name="address">I2C address</param>
    /// <param name="data">Data byte to write</param>
    /// <returns>Number of bytes actually written. Should be 1 for successful write, 0 if failed, negative if an error occurs</returns>
    virtual size_t Write(uint16 address, uint8 data) = 0;
    /// <summary>
    /// Write multiple bytes to device
    /// </summary>
    /// <param name="address">I2C address</param>
    /// <param name="buffer">Pointer to buffer containing data to be sent</param>
    /// <param name="count">Requested byte count for write</param>
    /// <returns>Number of bytes actually written, or negative if an error occurs</returns>
    virtual size_t Write(uint16 address, const void* buffer, size_t count) = 0;
    /// <summary>
    /// Write then read from device
    /// </summary>
    /// <param name="address">I2C address</param>
    /// <param name="writeBuffer">Pointer to buffer containing data to be sent</param>
    /// <param name="writeCount">Requested byte count for write</param>
    /// <param name="readBuffer">Pointer to buffer to store data received</param>
    /// <param name="readCount">Requested byte count for read</param>
    /// <returns>Number of bytes actually written and read (cumulated), or negative if an error occurs</returns>
    virtual size_t WriteReadRepeatedStart(uint16 address, const void* writeBuffer, size_t writeCount, void* readBuffer, size_t readCount) = 0;
};

#else

#error RPI 5 not supported yet

#endif

} // namespace baremetal
