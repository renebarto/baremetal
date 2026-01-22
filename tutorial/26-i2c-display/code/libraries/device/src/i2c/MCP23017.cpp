//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23017.cpp
//
// Namespace   : device
//
// Class       : MCP23017
//
// Description : I2C Master functionality
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

#include "device/i2c/MCP23017.h"

#include "baremetal/Logger.h"

using namespace device;
using namespace baremetal;

/// @file
/// MCP 23017 I2C expander support declaration

/// @brief Define log name
LOG_MODULE("MCP23017");

/// <summary>
/// Constructor for MCP23017 class
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
MCP23017::MCP23017(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
    : m_device{memoryAccess}
{
}

/// <summary>
/// Destructor for MCP23017 class
/// </summary>
MCP23017::~MCP23017()
{
    SetPortADirections(MCP23017PinDirection::In);
    SetPortBDirections(MCP23017PinDirection::In);
}

/// <summary>
/// Initialize the MCP23017 I2C expander
/// </summary>
/// <param name="bus">I2C bus index</param>
/// <param name="address">I2C slave address</param>
/// <returns>True on success, false otherwise</returns>
bool MCP23017::Initialize(uint8 bus, uint8 address)
{
    LOG_INFO("Initialize %02x", address);
    m_address = address;
    if (!m_device.Initialize(bus, I2CClockMode::Normal, 0))
        return false;

    WriteRegister(IOCONA, IOCON_BANK0 | IOCON_SEQOP | IOCON_HAEN | IOCON_ODR);
    return true;
}

/// <summary>
/// Read from the specified MCP23017 register
/// </summary>
/// <param name="registerAddress">Register index</param>
/// <returns>Value read</returns>
uint8 MCP23017::ReadRegister(MCP23017RegisterIndex registerAddress)
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
void MCP23017::WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte)
{
    const size_t BufferSize{2};
    uint8 buffer[BufferSize];
    buffer[0] = static_cast<uint8>(registerAddress);
    buffer[1] = byte;
    auto bytesWritten = m_device.Write(m_address, buffer, BufferSize);

    LOG_DEBUG("Write bytes to I2C %02x Register %02x: %02x, %d bytes written", m_address, buffer[0], buffer[1], bytesWritten);
}

/// <summary>
/// Get the I/O pin direction for the specified pin on the MCP23017
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="direction">Holds pin direction on return</param>
void MCP23017::GetPinDirection(MCP23017Pin pinNumber, MCP23017PinDirection& direction)
{
    MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? IODIRA : IODIRB;
    uint8 data = ReadRegister(registerAddress);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    direction = (data & (1 << pinShift)) ? MCP23017PinDirection::In : MCP23017PinDirection::In;
}

/// <summary>
/// Set the I/O pin direction for the specified pin on the MCP23017
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="direction">Pin direction to set</param>
void MCP23017::SetPinDirection(MCP23017Pin pinNumber, const MCP23017PinDirection& direction)
{
    MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? IODIRA : IODIRB;
    uint8 data = ReadRegister(registerAddress);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    uint8 mask = 1 << pinShift;
    uint8 pinData = (direction == MCP23017PinDirection::In) ? mask : 0;
    WriteRegister(registerAddress, (data & ~mask) | pinData);
}

/// <summary>
/// Get the value of an input pin on the MCP23017
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <returns>Requested pin value</returns>
bool MCP23017::GetPinValue(MCP23017Pin pinNumber)
{
    MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? GPIOA : GPIOB;
    uint8 data = ReadRegister(registerAddress);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    return (data & (1 << pinShift));
}

/// <summary>
/// Set the value of an output pin on the MCP23017
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="on">Value for output pin</param>
void MCP23017::SetPinValue(MCP23017Pin pinNumber, bool on)
{
    MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? GPIOA : GPIOB;
    uint8 data = ReadRegister(registerAddress);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    uint8 mask = 1 << pinShift;
    uint8 pinData = on ? mask : 0;
    WriteRegister(registerAddress, (data & ~mask) | pinData);
}

/// <summary>
/// Set I/O pin directions for all pin on Port A
/// </summary>
/// <param name="direction">Direction for pins</param>
void MCP23017::SetPortADirections(const MCP23017PinDirection& direction)
{
    WriteRegister(GPPUA, 0x00);
    WriteRegister(IODIRA, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
}

/// <summary>
/// Get value for all pins on Port A.
/// Bit 0 is the value for pin 0, etc.
/// </summary>
/// <returns></returns>
uint8 MCP23017::GetPortAValue()
{
    return ReadRegister(GPIOA);
}

/// <summary>
/// Set value for all pins on Port A
/// </summary>
/// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
void MCP23017::SetPortAValue(uint8 data)
{
    WriteRegister(GPIOA, data);
}

/// <summary>
/// Set I/O pin directions for all pin on Port B
/// </summary>
/// <param name="direction">Direction for pins</param>
void MCP23017::SetPortBDirections(const MCP23017PinDirection& direction)
{
    WriteRegister(GPPUB, 0x00);
    WriteRegister(IODIRB, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
}

/// <summary>
/// Get value for all pins on Port B.
/// Bit 0 is the value for pin 0, etc.
/// </summary>
/// <returns></returns>
uint8 MCP23017::GetPortBValue()
{
    return ReadRegister(GPIOB);
}

/// <summary>
/// Set value for all pins on Port B
/// </summary>
/// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
void MCP23017::SetPortBValue(uint8 data)
{
    WriteRegister(GPIOB, data);
}
