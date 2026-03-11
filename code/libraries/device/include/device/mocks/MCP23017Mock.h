//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MCP23017Mock.h
//
// Namespace   : device
//
// Class       : MCP23017Mock
//
// Description : MCP23017 mocking functionality shared between I2C and SPI mocks
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

/// @file
/// MCP23017Mock

namespace device {

/// @brief MCP23017 registers
class MCP23017Registers
{
public:
    /// @brief I/O Direction Register Port A
    uint8 IODIRA;
    /// @brief I/O Direction Register Port B
    uint8 IODIRB;
    /// @brief Input Polarity Register Port A
    uint8 IPOLA;
    /// @brief Input Polarity Register Port B
    uint8 IPOLB;
    /// @brief GPIO Interrupt Enable Register Port A
    uint8 GPINTENA;
    /// @brief GPIO Interrupt Enable Register Port B
    uint8 GPINTENB;
    /// @brief Default Compare Register Port A
    uint8 DEFVALA;
    /// @brief Default Compare Register Port B
    uint8 DEFVALB;
    /// @brief Interrupt Control Register Port A
    uint8 INTCONA;
    /// @brief Interrupt Control Register Port B
    uint8 INTCONB;
    /// @brief I/O Control Register Port A
    uint8 IOCONA;
    /// @brief I/O Control Register Port B
    uint8 IOCONB;
    /// @brief GPIO Pull-up Resistor Configuration Register Port A
    uint8 GPPUA;
    /// @brief GPIO Pull-up Resistor Configuration Register Port B
    uint8 GPPUB;
    /// @brief Interrupt Flag Register Port A
    uint8 INTFA;
    /// @brief Interrupt Flag Register Port B
    uint8 INTFB;
    /// @brief Interrupt Captured Register Port A
    uint8 INTCAPA;
    /// @brief Interrupt Captured Register Port B
    uint8 INTCAPB;
    /// @brief GPIO Port Register Port A
    uint8 GPIOA;
    /// @brief GPIO Port Register Port b
    uint8 GPIOB;
    /// @brief Output Latch Register Port A
    uint8 OLATA;
    /// @brief Output Latch Register Port B
    uint8 OLATB;

    /// <summary>
    /// Constructor for MCP23017 register storage
    /// </summary>
    MCP23017Registers()
        : IODIRA{0xFF}
        , IODIRB{0xFF}
        , IPOLA{}
        , IPOLB{}
        , GPINTENA{}
        , GPINTENB{}
        , DEFVALA{}
        , DEFVALB{}
        , INTCONA{}
        , INTCONB{}
        , IOCONA{}
        , IOCONB{}
        , GPPUA{}
        , GPPUB{}
        , INTFA{}
        , INTFB{}
        , INTCAPA{}
        , INTCAPB{}
        , GPIOA{}
        , GPIOB{}
        , OLATA{}
        , OLATB{}
    {
    }
};

/// @brief SPI master operation codes
enum MCP23017OperationCode
{
    /// @brief Write IO Configuration Register
    WriteIOCON,
    /// @brief Write Port A IO Direction Register
    WriteIODIRA,
    /// @brief Write Port B IO Direction Register
    WriteIODIRB,
    /// @brief Write Port A GPIO Register
    WriteGPIOA,
    /// @brief Write Port B GPIO Register
    WriteGPIOB,
    /// @brief Write Port A Interrupt Enable Register
    WriteGPINTENA,
    /// @brief Write Port B Interrupt Enable Register
    WriteGPINTENB,
    /// @brief Write Port A Interrupt Control Register
    WriteINTCONA,
    /// @brief Write Port B Interrupt Control Register
    WriteINTCONB,
    /// @brief Write Port A Default Value Register
    WriteDEFVALA,
    /// @brief Write Port B Default Value Register
    WriteDEFVALB,
    /// @brief Write Port A Input Polarity Register
    WriteIPOLA,
    /// @brief Write Port B Input Polarity Register
    WriteIPOLB,
    /// @brief Write Port A GPIO Pull-up Register
    WriteGPPUA,
    /// @brief Write Port B GPIO Pull-up Register
    WriteGPPUB,
    /// @brief Write Port A Output Latch Register
    WriteOLATA,
    /// @brief Write Port B Output Latch Register
    WriteOLATB,
    /// @brief Read IO Configuration Register
    ReadIOCON,
    /// @brief Read Port A IO Direction Register
    ReadIODIRA,
    /// @brief Read Port B IO Direction Register
    ReadIODIRB,
    /// @brief Read Port A GPIO Register
    ReadGPIOA,
    /// @brief Read Port B GPIO Register
    ReadGPIOB,
    /// @brief Read Port A Interrupt Enable Register
    ReadGPINTENA,
    /// @brief Read Port B Interrupt Enable Register
    ReadGPINTENB,
    /// @brief Read Port A Interrupt Control Register
    ReadINTCONA,
    /// @brief Read Port B Interrupt Control Register
    ReadINTCONB,
    /// @brief Read Port A Default Value Register
    ReadDEFVALA,
    /// @brief Read Port B Default Value Register
    ReadDEFVALB,
    /// @brief Read Port A Input Polarity Register
    ReadIPOLA,
    /// @brief Read Port B Input Polarity Register
    ReadIPOLB,
    /// @brief Read Port A GPIO Pull-up Register
    ReadGPPUA,
    /// @brief Read Port B GPIO Pull-up Register
    ReadGPPUB,
    /// @brief Read Port A Interrupt Flag Register
    ReadINTFA,
    /// @brief Read Port B Interrupt Flag Register
    ReadINTFB,
    /// @brief Read Port A Interrupt Captured Register
    ReadINTCAPA,
    /// @brief Read Port B Interrupt Captured Register
    ReadINTCAPB,
    /// @brief Read Port A Output Latch Register
    ReadOLATA,
    /// @brief Read Port B Output Latch Register
    ReadOLATB,
    /// @brief Read IO Configuration Register
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct MCP23017Operation
{
    /// @brief SPI master operation code
    MCP23017OperationCode operation; // Size: 4 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    MCP23017Operation()
        : operation{}
        , argument{}
    {
    }

    /// <summary>
    /// Constructor for read or write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theArgument">Argument value</param>
    MCP23017Operation(MCP23017OperationCode theOperation, uint32 theArgument = 0)
        : operation{theOperation}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const MCP23017Operation& other) const
    {
        return (other.operation == operation) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const MCP23017Operation& other) const
    {
        return !operator==(other);
    }
}
/// @cond
ALIGN(8)
/// @endcond
;

} // namespace device

namespace baremetal {

baremetal::String Serialize(const device::MCP23017Operation& value);

} // namespace baremetal
