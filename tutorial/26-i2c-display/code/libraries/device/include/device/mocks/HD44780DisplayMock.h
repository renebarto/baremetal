//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DisplayMock.h
//
// Namespace   : device
//
// Class       : HD44780DisplayMock
//
// Description : HD44780Display mock for HD44780 based character display
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

#include "stdlib/Macros.h"
#include "baremetal/String.h"
#include "device/display/HD44780Display.h"

/// @file
/// HD44780DisplayMock

namespace device {

/// @brief I2C master operation codes
enum HD44780DisplayOperationCode
{
    /// @brief Initialization command 1
    CommandInit1,
    /// @brief Initialization command 2
    CommandInit2,
    /// @brief Command Clear Display
    CommandClearDisplay,
    /// @brief Command Return Home
    CommandReturnHome,
    /// @brief Command Entry Mode Set
    CommandEntryModeSet,
    /// @brief Command Display Control
    CommandDisplayControl,
    /// @brief Command Dursor Move or Shift
    CommandDisplayCursorMoveOrShift,
    /// @brief Command Function Set
    CommandFunctionSet,
    /// @brief Command Set CGRAM Address
    CommandSetCGRAMAddress,
    /// @brief Command Set DDRAM Address
    CommandSetDDRAMAddress,
    /// @brief Data
    Data,
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct HD44780DisplayOperation
{
    /// @brief I2C master operation code
    HD44780DisplayOperationCode operation; // Size: 4 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    HD44780DisplayOperation()
        : operation{}
        , argument{}
    {
    }

    /// <summary>
    /// Constructor for read or write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theArgument">Argument value</param>
    HD44780DisplayOperation(HD44780DisplayOperationCode theOperation, uint32 theArgument = 0)
        : operation{theOperation}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const HD44780DisplayOperation& other) const
    {
        return (other.operation == operation) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const HD44780DisplayOperation& other) const
    {
        return !operator==(other);
    }
} ALIGN(8);

/// @brief MemoryAccess implementation for I2C stub
class HD44780DisplayMock : public HD44780Display
{
private:
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    HD44780DisplayOperation m_ops[BufferSize] ALIGN(8);
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    HD44780DisplayMock(uint8 numColumns, uint8 numRows);

    size_t GetNumOperations() const;
    const HD44780DisplayOperation& GetOperation(size_t index) const;

    void WriteHalfByte(uint8 data) override;

private:
    void AddOperation(const HD44780DisplayOperation& operation);
};

} // namespace device

namespace baremetal {

baremetal::String Serialize(const device::HD44780DisplayOperation& value);

} // namespace baremetal
