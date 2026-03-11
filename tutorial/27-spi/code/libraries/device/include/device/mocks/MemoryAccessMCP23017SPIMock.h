//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23017SPIMock.h
//
// Namespace   : device
//
// Class       : MemoryAccessMCP23017SPIMock
//
// Description : MCP23017 SPI memory access stub
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

#include "baremetal/mocks/MemoryAccessSPIMasterMock.h"
#include "stdlib/Macros.h"
#include "baremetal/String.h"
#include "device/spi/MCP23017SPI.h"
#include "device/mocks/MCP23017Mock.h"

/// @file
/// MemoryAccessMCP23017SPIMock

namespace device {

/// @brief MemoryAccess implementation for SPI stub
class MemoryAccessMCP23017SPIMock : public baremetal::MemoryAccessSPIMasterMock
{
private:
    /// @brief Singleton instance
    static MemoryAccessMCP23017SPIMock* m_pThis;
    /// @brief Storage for SPI registers
    MCP23017Registers m_registers;
    /// @brief A read / write register cycle was started
    bool m_cycleStarted;
    /// @brief Read register 
    bool m_readRegister;
    /// @brief Write register 
    bool m_writeRegister;
    /// @brief A read / write register cycle was started
    bool m_selectedRegisterReceived;
    /// @brief Register selected for current read / write register cycle
    uint8 m_selectedRegister;
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    MCP23017Operation m_ops[BufferSize]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    MemoryAccessMCP23017SPIMock();

    size_t GetNumMCP23017Operations() const;
    const MCP23017Operation& GetMCP23017Operation(size_t index) const;

    void ResetCycle();
    static void OnSendRecvData(baremetal::SPIMasterRegisters& registers, uint8 dataOut, uint8& dataIn);

    virtual bool OnReadRegister(uint8 registerIndex, uint8& data);
    virtual bool OnWriteRegister(uint8 registerIndex, uint8 data);

private:
    void AddOperation(const MCP23017Operation& operation);
};

} // namespace device
