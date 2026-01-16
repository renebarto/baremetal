//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DisplayI2CMock.h
//
// Namespace   : device
//
// Class       : HD44780DisplayI2CMock
//
// Description : HD44780DisplayI2C mock for HD44780 based character display
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

#include "baremetal/II2CMaster.h"
#include "stdlib/Macros.h"
#include "baremetal/String.h"
#include "device/i2c/HD44780DisplayI2C.h"

/// @file
/// HD44780DisplayI2CMock

namespace device {

/// @brief PCF8574 registers
class PCF8574Registers
{
public:
    uint8 data;
    /// <summary>
    /// Constructor for HD44780DisplayI2C register storage
    /// </summary>
    PCF8574Registers()
        : data{}
    {
    }
};

/// @brief HD44780DisplayI2C registers
class HD44780DisplayI2CRegisters
{
public:

    /// <summary>
    /// Constructor for HD44780DisplayI2C register storage
    /// </summary>
    HD44780DisplayI2CRegisters()
    {
    }
};

/// @brief I2C master operation codes
enum HD44780DisplayI2COperationCode
{
    /// @brief SetBacklight
    SetBacklight,
    /// @brief Command
    Command,
    /// @brief Data
    Data,
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct HD44780DisplayI2COperation
{
    /// @brief I2C master operation code
    HD44780DisplayI2COperationCode operation; // Size: 4 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    HD44780DisplayI2COperation()
        : operation{}
        , argument{}
    {
    }

    /// <summary>
    /// Constructor for read or write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theArgument">Argument value</param>
    HD44780DisplayI2COperation(HD44780DisplayI2COperationCode theOperation, uint32 theArgument = 0)
        : operation{theOperation}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const HD44780DisplayI2COperation& other) const
    {
        return (other.operation == operation) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const HD44780DisplayI2COperation& other) const
    {
        return !operator==(other);
    }
} ALIGN(8);

/// @brief MemoryAccess implementation for I2C stub
class HD44780DisplayI2CMock : public baremetal::II2CMaster
{
private:
    /// @brief I2C address
    uint8 m_address;
    /// @brief Storage for I2C PCF8574 expander
    PCF8574Registers m_expanderRegisters;
    /// @brief Storage for I2C registers
    HD44780DisplayI2CRegisters m_displayRegisters;
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    HD44780DisplayI2COperation m_ops[BufferSize] ALIGN(8);
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    HD44780DisplayI2CMock(uint8 address);

    size_t GetNumOperations() const;
    const HD44780DisplayI2COperation& GetOperation(size_t index) const;

    size_t Read(uint16 address, uint8& data) override;
    size_t Read(uint16 address, void* buffer, size_t count) override;
    size_t Write(uint16 address, uint8 data) override;
    size_t Write(uint16 address, const void* buffer, size_t count) override;
    size_t WriteReadRepeatedStart(uint16 address, const void* writeBuffer, size_t writeCount, void* readBuffer, size_t readCount) override;

private:
    void AddOperation(const HD44780DisplayI2COperation& operation);
};

} // namespace device

namespace baremetal {

baremetal::String Serialize(const device::HD44780DisplayI2COperation& value);

} // namespace baremetal
