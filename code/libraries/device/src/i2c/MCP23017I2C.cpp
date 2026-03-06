//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23017I2C.cpp
//
// Namespace   : device
//
// Class       : MCP23017I2C
//
// Description : MCP23017 I2C expander functionality
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

#include "device/i2c/MCP23017I2C.h"

#include "baremetal/Logger.h"

using namespace device;
using namespace baremetal;

/// @file
/// MCP 23017 I2C expander implementation

/// @brief Define log name
LOG_MODULE("MCP23017I2C");

/// <summary>
/// Constructor for MCP23017 class
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
MCP23017I2C::MCP23017I2C(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
    : MCP23017()
    , m_device{memoryAccess}
{
}

/// <summary>
/// Destructor for MCP23017 class
/// </summary>
MCP23017I2C::~MCP23017I2C()
{
    Uninitialize();
}

/// <summary>
/// Initialize the MCP23017 I2C expander
/// </summary>
/// <param name="bus">I2C bus index</param>
/// <param name="address">I2C slave address</param>
/// <returns>True on success, false otherwise</returns>
bool MCP23017I2C::Initialize(uint8 bus, uint8 address)
{
    LOG_INFO("Initialize %02x", address);
    m_address = address;
    if (!m_device.Initialize(bus, I2CClockMode::Normal, 0))
        return false;

    if (!MCP23017::Initialize())
        return false;
    return true;
}

/// <summary>
/// Read from the specified MCP23017 register
/// </summary>
/// <param name="registerAddress">Register index</param>
/// <returns>Value read</returns>
uint8 MCP23017I2C::ReadRegister(MCP23017RegisterIndex registerAddress)
{
    uint8 address = static_cast<uint8>(registerAddress);
    uint8 data{};
    auto bytesTransferred = m_device.WriteReadRepeatedStart(m_address, &address, 1, &data, 1);
    LOG_DEBUG("Read bytes from I2C %02x Register %02x: %02x, %d bytes transferred", m_address, address, data, bytesTransferred);
    return data;
}

/// <summary>
/// Write to the specified MCP23017 register
/// </summary>
/// <param name="registerAddress">Register index</param>
/// <param name="byte">Value to write</param>
void MCP23017I2C::WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte)
{
    const size_t BufferSize{2};
    uint8 buffer[BufferSize];
    buffer[0] = static_cast<uint8>(registerAddress);
    buffer[1] = byte;
    auto bytesWritten = m_device.Write(m_address, buffer, BufferSize);

    LOG_DEBUG("Write bytes to I2C %02x Register %02x: %02x, %d bytes written", m_address, buffer[0], buffer[1], bytesWritten);
}
