//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23008SPI.cpp
//
// Namespace   : device
//
// Class       : MCP23008SPI
//
// Description : MCP23008 SPI expander functionality
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

#include "device/spi/MCP23008SPI.h"

#include "baremetal/Logger.h"

using namespace device;
using namespace baremetal;

/// @file
/// MCP 23008 I2C expander support declaration

/// @brief Define log name
LOG_MODULE("MCP23008SPI");

/// <summary>
/// Constructor for MCP23008SPI class
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
MCP23008SPI::MCP23008SPI(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
    : MCP23008()
    , m_device{memoryAccess}
{
}

/// <summary>
/// Destructor for MCP23008SPI class
/// </summary>
MCP23008SPI::~MCP23008SPI()
{
    Uninitialize();
}

/// <summary>
/// Initialize the MCP23008 SPI expander
/// </summary>
/// <param name="device">SPI device index</param>
/// <param name="ceIndex">SPI CE/CS to activate</param>
/// <returns>True on success, false otherwise</returns>
bool MCP23008SPI::Initialize(uint8 device, SPI_CEIndex ceIndex)
{
    LOG_INFO("Initialize SPI %02x, CE %02x", device, static_cast<uint8>(ceIndex));
    m_ceIndex = ceIndex;
    if (!m_device.Initialize(device))
        return false;

    if (!MCP23008::Initialize())
        return false;
    return true;
}

/// <summary>
/// Read from the specified MCP23008 register
/// </summary>
/// <param name="registerAddress">Register index</param>
/// <returns>Value read</returns>
uint8 MCP23008SPI::ReadRegister(MCP23008RegisterIndex registerAddress)
{
    const size_t BufferSize{2};
    uint8 buffer[BufferSize];
    buffer[0] = 0b01000001;
    buffer[1] = static_cast<uint8>(registerAddress);
    uint8 data{};
    auto bytesTransferred = m_device.Write(m_ceIndex, buffer, BufferSize);
    bytesTransferred += m_device.Read(m_ceIndex, &data, 1);
    LOG_DEBUG("Read bytes from SPI CE %02x Register %02x: %02x, %d bytes transferred", m_ceIndex, buffer[1], data, bytesTransferred);
    return data;
}

/// <summary>
/// Write to the specified MCP23008 register
/// </summary>
/// <param name="registerAddress">Register index</param>
/// <param name="byte">Value to write</param>
void MCP23008SPI::WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte)
{
    const size_t BufferSize{3};
    uint8 buffer[BufferSize];
    buffer[0] = 0b01000000;
    buffer[1] = static_cast<uint8>(registerAddress);
    buffer[2] = byte;
    auto bytesWritten = m_device.Write(m_ceIndex, buffer, BufferSize);

    LOG_DEBUG("Write bytes to SPI CE %02x Register %02x: %02x, %d bytes written", m_ceIndex, buffer[1], buffer[2], bytesWritten);
}
