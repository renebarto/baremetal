//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23008.h
//
// Namespace   : device
//
// Class       : MCP23008
//
// Description : MCP23008 I2C expander functionality
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

#include "baremetal/I2CMaster.h"

/// @file
/// MCP 23008 I2C expander support declaration

namespace device {

/// @brief MCP23008 registers
enum MCP23008RegisterIndex
{
    /// @brief I/O Direction Register
    IODIR = 0,
    /// @brief Input Polarity Register
    IPOL = 1,
    /// @brief GPIO Interrupt Enable Register
    GPINTEN = 2,
    /// @brief Default Compare Register
    DEFVAL = 3,
    /// @brief Interrupt Control Register
    INTCON = 4,
    /// @brief I/O Control Register
    IOCON = 5,
    /// @brief GPIO Pull-up Resistor Configuration Register
    GPPU = 6,
    /// @brief Interrupt Flag Register
    INTF = 7,
    /// @brief Interrupt Captured Register
    INTCAP = 8,
    /// @brief GPIO Port Register
    GPIO = 9,
    /// @brief Output Latch Register
    OLAT = 10,
};

/// @brief IO Configuration Register sequential operation (automatic register index increment, subsequent write will be to next register)
#define IOCON_SEQOP  BIT1(5)
/// @brief IO Configuration Register SDA slew rate
#define IOCON_DISSLW BIT1(4)
/// @brief IO Configuration Register Hardware Address Enable (for MCP23S017 only)
#define IOCON_HAEN   BIT1(3)
/// @brief IO Configuration Register Open Drain Interrupt pin
#define IOCON_ODR    BIT1(2)
/// @brief IO Configuration Register Interrupt polarity
#define IOCON_INTPOL BIT1(1)

/// @brief MCP23008 pin direction
enum class MCP23008PinDirection
{
    /// @brief Input pin
    In,
    /// @brief Output pin
    Out,
};

/// @brief MCP23008 pin index
enum class MCP23008Pin
{
    /// @brief Pin 0
    Pin0,
    /// @brief Pin 1
    Pin1,
    /// @brief Pin 2
    Pin2,
    /// @brief Pin 3
    Pin3,
    /// @brief Pin 4
    Pin4,
    /// @brief Pin 5
    Pin5,
    /// @brief Pin 6
    Pin6,
    /// @brief Pin 7
    Pin7,
};

/// <summary>
/// Driver for MCP23008 I2C expander device
///
/// This device is normally on I2C address 20-27 depending on how its A0-A2 pins are connected.
/// The device supports two 8 bit ports name port A and B.
/// Each pin can be either an input or output.
/// </summary>
class MCP23008
{
private:
    /// @brief I2C base device
    baremetal::I2CMaster m_device;
    /// @brief I2C device address
    uint8 m_address;

public:
    MCP23008(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
    ~MCP23008();

    bool Initialize(uint8 bus, uint8 address);

    uint8 ReadRegister(MCP23008RegisterIndex registerAddress);
    void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte);
    void GetPinDirection(MCP23008Pin pinNumber, MCP23008PinDirection& direction);
    void SetPinDirection(MCP23008Pin pinNumber, const MCP23008PinDirection& direction);
    bool GetPinValue(MCP23008Pin pinNumber);
    void SetPinValue(MCP23008Pin pinNumber, bool on);
    void SetPortDirections(const MCP23008PinDirection& direction);
    uint8 GetPortValue();
    void SetPortValue(uint8 data);
};

} // namespace device
