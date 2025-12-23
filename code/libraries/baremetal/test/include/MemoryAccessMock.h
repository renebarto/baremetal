//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessMock.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessMock
//
// Description : Memory read/write mock class
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

#include "baremetal/IMemoryAccess.h"
#include "baremetal/String.h"
#include "stdlib/Macros.h"

/// @file
/// Memory access mock class

namespace baremetal {

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct MemoryAccessOperation
{
    /// @brief True if write operation, false if read
    bool isWriteOperation;
    /// @brief Address read from or written to
    regaddr address;
    /// @brief On read, the data returned, on write the data written
    uint32 data;

    /// <summary>
    /// Default constructor
    /// </summary>
    MemoryAccessOperation()
        : isWriteOperation{}
        , address{}
        , data{}
    {
    }
    /// <summary>
    /// Consturctor for read operations, without data
    /// </summary>
    /// <param name="aIsWriteOperation">True if this is a write operation, false for a read operation. Should be false.</param>
    /// <param name="aAddress">Addres to read from / write to</param>
    MemoryAccessOperation(bool aIsWriteOperation, regaddr aAddress)
        : isWriteOperation{aIsWriteOperation}
        , address{aAddress}
        , data{}
    {
    }
    /// <summary>
    /// Consturctor for write operations, with data
    /// </summary>
    /// <param name="aIsWriteOperation">True if this is a write operation, false for a read operation. Should be true.</param>
    /// <param name="aAddress">Addres to read from / write to</param>
    /// <param name="aData">Data to write</param>
    MemoryAccessOperation(bool aIsWriteOperation, regaddr aAddress, uint32 aData)
        : isWriteOperation{aIsWriteOperation}
        , address{aAddress}
        , data{aData}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const MemoryAccessOperation& other) const
    {
        return (other.isWriteOperation == isWriteOperation) && (other.address == address) && (!isWriteOperation || (other.data == data));
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const MemoryAccessOperation& other) const
    {
        return !operator==(other);
    }
};

String Serialize(const MemoryAccessOperation& value);

/// <summary>
/// Memory access mock class
/// </summary>
class MemoryAccessMock : public IMemoryAccess
{
private:
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    MemoryAccessOperation m_accessOps[BufferSize] ALIGN(8);
    /// @brief Number of registered memory acess operations
    size_t m_numAccessOps;

public:
    MemoryAccessMock();

    size_t GetNumOperations() const;
    const MemoryAccessOperation& GetOperation(size_t index) const;

    uint8 Read8(regaddr address) override;
    void Write8(regaddr address, uint8 data) override;

    uint16 Read16(regaddr address) override;
    void Write16(regaddr address, uint16 data) override;

    uint32 Read32(regaddr address) override;
    void Write32(regaddr address, uint32 data) override;
};

} // namespace baremetal
