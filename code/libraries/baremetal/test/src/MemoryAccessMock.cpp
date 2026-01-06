//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessMock.cpp
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

#include "MemoryAccessMock.h"

#include "baremetal/Assert.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"

/// @file
/// Memory access mock class implementation

/// @brief Define log name
LOG_MODULE("MemoryAccessMock");

using namespace baremetal;

/// <summary>
/// Serialize a memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const MemoryAccessOperation &value)
{
    if (value.isWriteOperation)
        return Format("W: %p %08lx", value.address, value.data);
    return Format("R: %p %08lx", value.address, value.data);
}

/// <summary>
/// Default constructor
/// </summary>
MemoryAccessMock::MemoryAccessMock()
    : m_accessOps{}
    , m_numAccessOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessMock::GetNumOperations() const
{
    return m_numAccessOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const MemoryAccessOperation &MemoryAccessMock::GetMemoryOperation(size_t index) const
{
    assert(index < m_numAccessOps);
    return m_accessOps[index];
}

/// <summary>
/// Callback on read operation
/// </summary>
/// <param name="address">Address to read from</param>
/// <returns>32 bit unsigned integer read</returns>
uint32 MemoryAccessMock::OnRead(regaddr address)
{
    uint32 value{};
    TRACE_INFO("Read(%p, %02x))", address, value);
    return value;
}

/// <summary>
/// Callback on write operation
/// </summary>
/// <param name="address">Address to write to</param>
/// <param name="data">32 bit unsigned integer value to write</param>
void MemoryAccessMock::OnWrite(regaddr address, uint32 data)
{
    TRACE_INFO("Write(%p, %02x)", address, data);
}

/// <summary>
/// Read a 8 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>8 bit register value</returns>
uint8 MemoryAccessMock::Read8(regaddr address)
{
    uint8 value = OnRead(address) & 0xFF;
    AddOperation({address, value, false});
    return value;
}

/// <summary>
/// Write a 8 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessMock::Write8(regaddr address, uint8 data)
{
    uint32 value = static_cast<uint32>(data);
    AddOperation({ address, value, true });
    OnWrite(address, value);
}

/// <summary>
/// Read a 16 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>16 bit register value</returns>
uint16 MemoryAccessMock::Read16(regaddr address)
{
    uint16 value = OnRead(address) & 0xFFFF;
    AddOperation({address, value, false});
    return value;
}

/// <summary>
/// Write a 16 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessMock::Write16(regaddr address, uint16 data)
{
    uint32 value = static_cast<uint32>(data);
    AddOperation({ address, value, true });
    OnWrite(address, value);
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessMock::Read32(regaddr address)
{
    uint32 value = OnRead(address);
    AddOperation({address, value, false});
    return value;
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessMock::Write32(regaddr address, uint32 data)
{
    AddOperation({ address, data, true });
    OnWrite(address, data);
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessMock::AddOperation(const MemoryAccessOperation& operation)
{
    assert(m_numAccessOps < BufferSize);
    m_accessOps[m_numAccessOps++] = operation;
}
