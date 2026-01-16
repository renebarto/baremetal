//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : HD44780DisplayI2CMock.cpp
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

#include "device/mocks/HD44780DisplayI2CMock.h"

#include "baremetal/Assert.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"
#include "device/i2c/HD44780DisplayI2C.h"

/// @file
/// HD44780DisplayI2CMock

/// @brief Define log name
LOG_MODULE("HD44780DisplayI2CMock");

using namespace baremetal;
using namespace device;

static const uint8 RSBit = BIT1(0);
static const uint8 RWBit = BIT1(1);
static const uint8 ENBit = BIT1(2);
static const uint8 BacklightBit = BIT1(3);

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(HD44780DisplayI2COperationCode code)
{
    String result{};
    switch (code)
    {
    case HD44780DisplayI2COperationCode::SetBacklight:
        result = "SetBacklight";
        break;
    case HD44780DisplayI2COperationCode::Command:
        result = "Command";
        break;
    case HD44780DisplayI2COperationCode::Data:
        result = "Data";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a GPIO memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const HD44780DisplayI2COperation &value)
{
    String result = Format("Operation=%s, ", OperationCodeToString(value.operation).c_str());
    switch (value.operation)
    {
    case HD44780DisplayI2COperationCode::SetBacklight:
        {
            String line = (value.argument == 0) ? "OFF" : "ON";
            result += line;
        }
        break;

    case HD44780DisplayI2COperationCode::Command:
    case HD44780DisplayI2COperationCode::Data:
        {
            String line = Format("%02x", value.argument);
            result += line;
        }
        break;

    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

/// <summary>
/// HD44780DisplayI2CMock constructor
/// </summary>
HD44780DisplayI2CMock::HD44780DisplayI2CMock(uint8 address)
    : m_address{ address }
    , m_expanderRegisters{}
    , m_displayRegisters{}
    , m_ops{}
    , m_numOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t HD44780DisplayI2CMock::GetNumOperations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const HD44780DisplayI2COperation &HD44780DisplayI2CMock::GetOperation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

size_t HD44780DisplayI2CMock::Read(uint16 address, uint8& data)
{
    if (m_address == address)
    {
        data = m_expanderRegisters.data;
        return 1;
    }
    return {};
}

size_t HD44780DisplayI2CMock::Read(uint16 address, void* buffer, size_t count)
{
    uint8* ptr = reinterpret_cast<uint8*>(buffer);
    size_t bytesRead{};
    for (size_t i = 0; i < count; ++i)
    {
        if (1 != Read(address, *ptr))
            break;
        ++bytesRead;
    }
    return bytesRead;
}

size_t HD44780DisplayI2CMock::Write(uint16 address, uint8 data)
{
    if (m_address == address)
    {
        uint8 prevData = m_expanderRegisters.data;
        m_expanderRegisters.data = data;
        uint8 diff = data ^ prevData;
        if (diff & BacklightBit)
        {
            AddOperation({ HD44780DisplayI2COperationCode::SetBacklight, static_cast<uint32>(data >> 3) & 0x00000001 });
        }
        if (diff & ENBit)
        {
            if ((data & ENBit) == 0) // Enable 1->0 = trigger
            {
                if ((data & RSBit) == 0)
                    AddOperation({ HD44780DisplayI2COperationCode::Command, static_cast<uint32>(data >> 4) & 0x0000000F });
                else
                    AddOperation({ HD44780DisplayI2COperationCode::Data, static_cast<uint32>(data >> 4) & 0x0000000F });
            }
        }

        return 1;
    }
    return {};
}

size_t HD44780DisplayI2CMock::Write(uint16 address, const void* buffer, size_t count)
{
    const uint8* ptr = reinterpret_cast<const uint8*>(buffer);
    size_t bytesWritten{};
    for (size_t i = 0; i < count; ++i)
    {
        if (1 != Write(address, *ptr))
            break;
        ++bytesWritten;
    }
    return bytesWritten;
}

size_t HD44780DisplayI2CMock::WriteReadRepeatedStart(uint16 address, const void* writeBuffer, size_t writeCount, void* readBuffer, size_t readCount)
{
    size_t bytesReadWritten = Write(address, writeBuffer, writeCount);
    bytesReadWritten += Read(address, readBuffer, readCount);
    return bytesReadWritten;
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void HD44780DisplayI2CMock::AddOperation(const HD44780DisplayI2COperation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}
