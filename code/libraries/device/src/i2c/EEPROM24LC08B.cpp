//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : EEPROM24LC08B.cpp
//
// Namespace   : device
//
// Class       : EEPROM24LC08B
//
// Description : 24LC08B EEPROM functionality
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

#include "device/i2c/EEPROM24LC08B.h"

#include "baremetal/Logger.h"
#include "stdlib/Util.h"

using namespace device;
using namespace baremetal;

/// @file
/// MCP 23017 I2C expander support declaration

/// @brief Define log name
LOG_MODULE("EEPROM24LC08B");

/// <summary>
/// Constructor for EEPROM24LC08B class
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
EEPROM24LC08B::EEPROM24LC08B(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
    : m_device{memoryAccess}
{
}

/// <summary>
/// Destructor for EEPROM24LC08B class
/// </summary>
EEPROM24LC08B::~EEPROM24LC08B()
{
}

/// <summary>
/// Initialize the 24LC08B I2C EEPROM
/// </summary>
/// <param name="bus">I2C bus index</param>
/// <param name="address">I2C slave address</param>
/// <returns>True on success, false otherwise</returns>
bool EEPROM24LC08B::Initialize(uint8 bus, uint8 address)
{
    LOG_INFO("Initialize %02x", address);
    m_address = address;
    if (!m_device.Initialize(bus, I2CClockMode::Normal, 0))
        return false;

    return true;
}

/// <summary>
/// Scan for the 24LC08B I2C EEPROM
/// </summary>
/// <param name="bus">I2C bus index</param>
/// <param name="address">I2C slave address</param>
/// <returns>True on success, false otherwise</returns>
bool EEPROM24LC08B::Scan()
{
    LOG_INFO("Scan for device at address %02x", m_address);
    if (!m_device.Scan(m_address))
        return false;
    uint8 dummy{};
    if (m_device.Read(m_address, dummy) != 1)
        return false;

    return true;
}

/// <summary>
/// Read a block of data from the 24LC08B EEPROM
/// </summary>
/// <param name="address">Start address</param>
/// <param name="buffer">Address of buffer to read data into</param>
/// <param name="numBytes">Number of bytes to read</param>
/// <returns>True if successful, false otherwise</returns>
bool EEPROM24LC08B::ReadData(uint8 address, uint8* data, uint8 numBytes)
{
    auto bytesWritten = m_device.Write(m_address, address);
    if (bytesWritten != 1)
    {
        LOG_ERROR("Failed to write address byte to I2C %02x", m_address);
        return false;
    }
    auto bytesRead = m_device.Read(m_address, data, numBytes);
    LOG_DEBUG("Read bytes from I2C %02x Block start %02x, block size %02x, %02x bytes read", m_address, address, numBytes, bytesRead);
    return bytesRead == size_t{numBytes};
}

/// <summary>
/// Write to the specified EEPROM24LC08B register
/// </summary>
/// <param name="address">Start address</param>
/// <param name="buffer">Address of buffer to write data from</param>
/// <param name="numBytes">Number of bytes to write</param>
/// <returns>True if successful, false otherwise</returns>
bool EEPROM24LC08B::WriteData(uint8 address, const uint8* data, uint8 numBytes)
{
    const size_t BufferSize{257};
    uint8 buffer[BufferSize];
    buffer[0] = static_cast<uint8>(address);
    memcpy(buffer + 1, data, numBytes);
    auto bytesWritten = m_device.Write(m_address, buffer, numBytes + 1);

    LOG_DEBUG("Write bytes to I2C %02x Block start %02x, block size %02x, %02x bytes written", m_address, address, numBytes, bytesWritten);
    return bytesWritten == size_t{numBytes} + 1;
}

