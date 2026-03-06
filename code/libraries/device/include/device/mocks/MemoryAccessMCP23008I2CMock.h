//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23008I2CMock.h
//
// Namespace   : device
//
// Class       : MemoryAccessMCP23008I2CMock
//
// Description : MCP23008 memory access stub with LEDs on output pins, and controllable inputs on input pins
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

#include "baremetal/mocks/MemoryAccessI2CMasterMock.h"
#include "stdlib/Macros.h"
#include "baremetal/String.h"
#include "device/i2c/MCP23008I2C.h"

/// @file
/// MemoryAccessMCP23008I2CMock

namespace device {

/// @brief MCP23008 registers
class MCP23008Registers
{
public:
    /// @brief I/O Direction Register
    uint8 IODIR;
    /// @brief Input Polarity Register
    uint8 IPOL;
    /// @brief GPIO Interrupt Enable Register
    uint8 GPINTEN;
    /// @brief Default Compare Register
    uint8 DEFVAL;
    /// @brief Interrupt Control Register
    uint8 INTCON;
    /// @brief I/O Control Register
    uint8 IOCON;
    /// @brief GPIO Pull-up Resistor Configuration Register
    uint8 GPPU;
    /// @brief Interrupt Flag Register
    uint8 INTF;
    /// @brief Interrupt Captured Register
    uint8 INTCAP;
    /// @brief GPIO Port Register
    uint8 GPIO;
    /// @brief Output Latch Register
    uint8 OLAT;

    /// <summary>
    /// Constructor for MCP23008 register storage
    /// </summary>
    MCP23008Registers()
        : IODIR{0xFF}
        , IPOL{}
        , GPINTEN{}
        , DEFVAL{}
        , INTCON{}
        , IOCON{}
        , GPPU{}
        , INTF{}
        , INTCAP{}
        , GPIO{}
        , OLAT{}
    {
    }
};

/// @brief I2C master operation codes
enum MCP23008OperationCode
{
    /// @brief Write IO Configuration Register
    WriteIOCON,
    /// @brief Write IO Direction Register
    WriteIODIR,
    /// @brief Write GPIO Register
    WriteGPIO,
    /// @brief Write Interrupt Enable Register
    WriteGPINTEN,
    /// @brief Write Interrupt Control Register
    WriteINTCON,
    /// @brief Write Default Value Register
    WriteDEFVAL,
    /// @brief Write Input Polarity Register
    WriteIPOL,
    /// @brief Write GPIO Pull-up Register
    WriteGPPU,
    /// @brief Write Output Latch Register
    WriteOLAT,
    /// @brief Read IO Configuration Register
    ReadIOCON,
    /// @brief Read IO Direction Register
    ReadIODIR,
    /// @brief Read GPIO Register
    ReadGPIO,
    /// @brief Read Interrupt Enable Register
    ReadGPINTEN,
    /// @brief Read Interrupt Control Register
    ReadINTCON,
    /// @brief Read Default Value Register
    ReadDEFVAL,
    /// @brief Read Input Polarity Register
    ReadIPOL,
    /// @brief Read GPIO Pull-up Register
    ReadGPPU,
    /// @brief Read Interrupt Flag Register
    ReadINTF,
    /// @brief Read Interrupt Captured Register
    ReadINTCAP,
    /// @brief Read Output Latch Register
    ReadOLAT,
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct MCP23008Operation
{
    /// @brief I2C master operation code
    MCP23008OperationCode operation; // Size: 4 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    MCP23008Operation()
        : operation{}
        , argument{}
    {
    }

    /// <summary>
    /// Constructor for read or write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theArgument">Argument value</param>
    MCP23008Operation(MCP23008OperationCode theOperation, uint32 theArgument = 0)
        : operation{theOperation}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const MCP23008Operation& other) const
    {
        return (other.operation == operation) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const MCP23008Operation& other) const
    {
        return !operator==(other);
    }
}
/// @cond
ALIGN(8)
/// @endcond
;

/// @brief MemoryAccess implementation for I2C stub
class MemoryAccessMCP23008I2CMock : public baremetal::MemoryAccessI2CMasterMock
{
private:
    /// @brief Singleton instance
    static MemoryAccessMCP23008I2CMock* m_pThis;
    /// @brief Storage for I2C registers
    MCP23008Registers m_registers;
    /// @brief A read / write register cycle was started
    bool m_cycleStarted;
    /// @brief Register selected for current read / write register cycle
    uint8 m_selectedRegister;
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    MCP23008Operation m_ops[BufferSize]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    MemoryAccessMCP23008I2CMock();

    size_t GetNumMCP23008Operations() const;
    const MCP23008Operation& GetMCP23008Operation(size_t index) const;

    void ResetCycle();
    static bool OnSendAddress(baremetal::I2CMasterRegisters& registers, uint8 data);
    static bool OnRecvData(baremetal::I2CMasterRegisters& registers, uint8& data);
    static bool OnSendData(baremetal::I2CMasterRegisters& registers, uint8 data);

    virtual bool OnReadRegister(uint8 registerIndex, uint8& data);
    virtual bool OnWriteRegister(uint8 registerIndex, uint8 data);

private:
    void AddOperation(const MCP23008Operation& operation);
};

} // namespace device

namespace baremetal {

baremetal::String Serialize(const device::MCP23008Operation& value);

} // namespace baremetal
