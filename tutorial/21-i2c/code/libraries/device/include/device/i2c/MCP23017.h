//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23017.h
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

#pragma once

#include <baremetal/I2CMaster.h>

/// @file
/// MCP 23017 I2C expander support declaration

namespace device {

/// @brief Select register bank for MCP23017.
/// MCP23017 has two register banks, which differ in the relative register numbers
#define BANK 0

#if BANK == 0

/// @brief MCP23017 registers when BANK = 0
enum MCP23017RegisterIndex {
    /// @brief I/O Direction Register Port A
    IODIRA = 0,
    /// @brief I/O Direction Register Port B
    IODIRB = 1,
    /// @brief Input Polarity Register Port A
    IPOLA = 2,
    /// @brief Input Polarity Register Port B
    IPOLB = 3,
    /// @brief GPIO Interrupt Enable Register Port A
    GPINTENA = 4,
    /// @brief GPIO Interrupt Enable Register Port B
    GPINTENB = 5,
    /// @brief Default Compare Register Port A
    DEFVALA = 6,
    /// @brief Default Compare Register Port B
    DEFVALB = 7,
    /// @brief Interrupt Control Register Port A
    INTCONA = 8,
    /// @brief Interrupt Control Register Port B
    INTCONB = 9,
    /// @brief I/O Control Register Port A
    IOCONA = 10,
    /// @brief I/O Control Register Port B
    IOCONB = 11,
    /// @brief GPIO Pull-up Resistor Configuration Register Port A
    GPPUA = 12,
    /// @brief GPIO Pull-up Resistor Configuration Register Port B
    GPPUB = 13,
    /// @brief Interrupt Flag Register Port A
    INTFA = 14,
    /// @brief Interrupt Flag Register Port B
    INTFB = 15,
    /// @brief Interrupt Captured Register Port A
    INTCAPA = 16,
    /// @brief Interrupt Captured Register Port B
    INTCAPB = 17,
    /// @brief GPIO Port Register Port A
    GPIOA = 18,
    /// @brief GPIO Port Register Port b
    GPIOB = 19,
    /// @brief Output Latch Register Port A
    OLATA = 20,
    /// @brief Output Latch Register Port B
    OLATB = 21,
};

#else

/// @brief MCP23017 registers when BANK = 1
enum MCP23017RegisterIndex {
    /// @brief I/O Direction Register Port A
    IODIRA = 0,
    /// @brief I/O Direction Register Port B
    IODIRB = 16,
    /// @brief Input Polarity Register Port A
    IPOLA = 1,
    /// @brief Input Polarity Register Port B
    IPOLB = 17,
    /// @brief GPIO Interrupt Enable Register Port A
    GPINTENA = 2,
    /// @brief GPIO Interrupt Enable Register Port B
    GPINTENB = 18,
    /// @brief Default Compare Register Port A
    DEFVALA = 3,
    /// @brief Default Compare Register Port B
    DEFVALB = 19,
    /// @brief Interrupt Control Register Port A
    INTCONA = 4,
    /// @brief Interrupt Control Register Port B
    INTCONB = 20,
    /// @brief I/O Control Register Port A
    IOCONA = 5,
    /// @brief I/O Control Register Port B
    IOCONB = 21,
    /// @brief GPIO Pull-up Resistor Configuration Register Port A
    GPPUA = 6,
    /// @brief GPIO Pull-up Resistor Configuration Register Port B
    GPPUB = 22,
    /// @brief Interrupt Flag Register Port A
    INTFA = 7,
    /// @brief Interrupt Flag Register Port B
    INTFB = 23,
    /// @brief Interrupt Captured Register Port A
    INTCAPA = 8,
    /// @brief Interrupt Captured Register Port B
    INTCAPB = 24,
    /// @brief GPIO Port Register Port A
    GPIOA = 9,
    /// @brief GPIO Port Register Port b
    GPIOB = 25,
    /// @brief Output Latch Register Port A
    OLATA = 10,
    /// @brief Output Latch Register Port B
    OLATB = 26,
};

#endif

/// @brief IO Configuration Register Bank selection
#define IOCON_BANK BIT1(7)
/// @brief IO Configuration Register Interrupt A/B mirroring
#define IOCON_MIRROR BIT1(6)
/// @brief IO Configuration Register sequential operation (automatic register index increment, subsequent write will be to next register)
#define IOCON_SEQOP BIT1(5)
/// @brief IO Configuration Register SDA slew rate
#define IOCON_DISSLW BIT1(4)
/// @brief IO Configuration Register Hardware Address Enable (for MCP23S017 only)
#define IOCON_HAEN BIT1(3)
/// @brief IO Configuration Register Open Drain Interrupt pin
#define IOCON_ODR BIT1(2)
/// @brief IO Configuration Register Interrupt polarity
#define IOCON_INTPOL BIT1(1)

/// @brief MCP23017 pin direction
enum class PinDirection
{
    /// @brief Input pin
    In,
    /// @brief Output pin
    Out,
};

/// <summary>
/// Driver for MCP23017 I2C expander device
///
/// This device is normally on I2C address 20-27 depending on how its A0-A2 pins are connected.
/// The device support two 8 bit ports name port A and B.
/// Each pin can be either an input or output.
/// </summary>
class MCP23017
{
private:
    /// @brief I2C base device
    baremetal::I2CMaster m_device;
    /// @brief I2C device address
    uint8 m_address;

public:
    MCP23017(baremetal::IMemoryAccess &memoryAccess = baremetal::GetMemoryAccess());
    ~MCP23017();

    bool Initialize(uint8 bus, uint8 address);

    uint8 ReadRegister(MCP23017RegisterIndex registerAddress);
    void WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte);
    void GetPinDirection(uint8 pinNumber, PinDirection &direction);
    void SetPinDirection(uint8 pinNumber, const PinDirection &direction);
    bool GetPinValue(uint8 pinNumber);
    void SetPinValue(uint8 pinNumber, bool on);
    void SetPortADirections(const PinDirection &direction);
    uint8 GetPortAValue();
    void SetPortAValue(uint8 data);
    void SetPortBDirections(const PinDirection &direction);
    uint8 GetPortBValue();
    void SetPortBValue(uint8 data);
};

} // namespace device
