//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubMCP23017LEDs.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessStubMCP23017LEDs
//
// Description : MCP23017 memory access stub with LEDs on output pins, and controllable inputs on input pins
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

#include <stdlib/Macros.h>
#include <baremetal/stubs/MemoryAccessStubI2C.h>

/// @file
/// MemoryAccessStubMCP23017LEDs

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
        : IODIRA{ 0xFF }
        , IODIRB{ 0xFF }
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

/// @brief MemoryAccess implementation for I2C stub
class MemoryAccessStubMCP23017LEDs:
    public baremetal::MemoryAccessStubI2C
{
private:
    /// @brief Singleton instance
    static MemoryAccessStubMCP23017LEDs* pThis;
    /// @brief Storage for I2C registers
    MCP23017Registers m_registers;
    /// @brief A read / write register cycle was started
    bool m_cycleStarted;
    /// @brief Register selected for current read / write register cycle
    uint8 m_selectedRegister;

public:
    MemoryAccessStubMCP23017LEDs();

    void ResetCycle();
    static bool OnSendAddress(baremetal::I2CRegisters& registers, uint8 data);
    static bool OnRecvData(baremetal::I2CRegisters& registers, uint8& data);
    static bool OnSendData(baremetal::I2CRegisters& registers, uint8 data);
};

} // namespace device
