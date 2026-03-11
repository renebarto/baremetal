//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23017SPIMock.cpp
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

#include "device/mocks/MemoryAccessMCP23017SPIMock.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "device/spi/MCP23017SPI.h"

/// @file
/// MemoryAccessMCP23017SPIMock

/// @brief Define log name
LOG_MODULE("MemoryAccessMCP23017SPIMock");

using namespace baremetal;
using namespace device;

/// @brief Singleton instance
MemoryAccessMCP23017SPIMock* MemoryAccessMCP23017SPIMock::m_pThis{};

/// <summary>
/// MemoryAccessMCP23017SPIMock constructor
/// </summary>
MemoryAccessMCP23017SPIMock::MemoryAccessMCP23017SPIMock()
    : m_registers{}
    , m_cycleStarted{}
    , m_readRegister{}
    , m_writeRegister{}
    , m_selectedRegisterReceived{}
    , m_selectedRegister{}
{
    m_pThis = this;
    SetSendRecvDataByteCallback(OnSendRecvData);
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessMCP23017SPIMock::GetNumMCP23017Operations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const MCP23017Operation &MemoryAccessMCP23017SPIMock::GetMCP23017Operation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

/// <summary>
/// Reset read or write cycle
/// </summary>
void MemoryAccessMCP23017SPIMock::ResetCycle()
{
    m_cycleStarted = false;
    m_selectedRegisterReceived = false;
    m_readRegister = false;
    m_writeRegister = false;
}

/// <summary>
/// Callback when SPI byte is to be received
/// </summary>
/// <param name="registers">SPI Register storage for stub</param>
/// <param name="dataOut">Byte sent</param>
/// <param name="dataIn">Byte requested</param>
void MemoryAccessMCP23017SPIMock::OnSendRecvData(SPIMasterRegisters& registers, uint8 dataOut, uint8& dataIn)
{
    dataIn = 0;
    if (!m_pThis->m_cycleStarted)
    {
        if (dataOut == 0b01000000)
        {
            m_pThis->m_writeRegister = true;
            m_pThis->m_cycleStarted = true;
        }
        else if (dataOut == 0b01000001)
        {
            m_pThis->m_readRegister = true;
            m_pThis->m_cycleStarted = true;
        }
        else
        {
            dataIn = 0xFF;
        }
    }
    else if (!m_pThis->m_selectedRegisterReceived)
    {
        m_pThis->m_selectedRegister = dataOut;
        m_pThis->m_selectedRegisterReceived = true;
    }
    else
    {
        if (m_pThis->m_writeRegister)
        {
            m_pThis->OnWriteRegister(m_pThis->m_selectedRegister, dataOut);
        }
        else if (m_pThis->m_readRegister)
        {
            m_pThis->OnReadRegister(m_pThis->m_selectedRegister, dataIn);
        }
        m_pThis->ResetCycle();
    }
}

/// <summary>
/// Read MCP23017 register
/// </summary>
/// <param name="registerIndex">Index of the register</param>
/// <param name="data">Data read on return</param>
/// <returns>True if successful, false otherwise</returns>
bool MemoryAccessMCP23017SPIMock::OnReadRegister(uint8 registerIndex, uint8& data)
{
    uint8* registerAddress = &m_pThis->m_registers.IODIRA + m_pThis->m_selectedRegister;
    data = *registerAddress;
    switch (m_pThis->m_selectedRegister)
    {
    case IOCONA:
    case IOCONB:
        AddOperation({ ReadIOCON, data });
        break;
    case IODIRA:
        AddOperation({ ReadIODIRA, data });
        break;
    case IODIRB:
        AddOperation({ ReadIODIRB, data });
        break;
    case GPIOA:
        AddOperation({ ReadGPIOA, data });
        break;
    case GPIOB:
        AddOperation({ ReadGPIOB, data });
        break;
    case GPINTENA:
        AddOperation({ ReadGPINTENA, data });
        break;
    case GPINTENB:
        AddOperation({ ReadGPINTENB, data });
        break;
    case INTCONA:
        AddOperation({ ReadINTCONA, data });
        break;
    case INTCONB:
        AddOperation({ ReadINTCONA, data });
        break;
    case DEFVALA:
        AddOperation({ ReadDEFVALA, data });
        break;
    case DEFVALB:
        AddOperation({ ReadDEFVALB, data });
        break;
    case IPOLA:
        AddOperation({ ReadIPOLA, data });
        break;
    case IPOLB:
        AddOperation({ ReadIPOLB, data });
        break;
    case GPPUA:
        AddOperation({ ReadGPPUA, data });
        break;
    case GPPUB:
        AddOperation({ ReadGPPUB, data });
        break;
    case INTFA:
        AddOperation({ ReadINTFA, data });
        break;
    case INTFB:
        AddOperation({ ReadINTFB, data });
        break;
    case INTCAPA:
        AddOperation({ ReadINTCAPA, data });
        break;
    case INTCAPB:
        AddOperation({ ReadINTCAPB, data });
        break;
    case OLATA:
        AddOperation({ ReadOLATA, data });
        break;
    case OLATB:
        AddOperation({ ReadOLATB, data });
        break;
    }
    return true;
}

/// <summary>
/// Write MCP23017 register
/// </summary>
/// <param name="registerIndex">Index of the register</param>
/// <param name="data">Data to be written</param>
/// <returns>True if successful, false otherwise</returns>
bool MemoryAccessMCP23017SPIMock::OnWriteRegister(uint8 registerIndex, uint8 data)
{
    uint8* registerAddress = &m_pThis->m_registers.IODIRA + m_pThis->m_selectedRegister;
    *registerAddress = data;
    switch (m_pThis->m_selectedRegister)
    {
    case IOCONA:
    case IOCONB:
        AddOperation({ WriteIOCON, data});
        break;
    case IODIRA:
        AddOperation({ WriteIODIRA, data });
        break;
    case IODIRB:
        AddOperation({ WriteIODIRB, data });
        break;
    case GPIOA:
        AddOperation({ WriteGPIOA, data});
        break;
    case GPIOB:
        AddOperation({ WriteGPIOB, data});
        break;
    case GPINTENA:
        AddOperation({ WriteGPINTENA, data});
        break;
    case GPINTENB:
        AddOperation({ WriteGPINTENB, data});
        break;
    case INTCONA:
        AddOperation({ WriteINTCONA, data});
        break;
    case INTCONB:
        AddOperation({ WriteINTCONB, data});
        break;
    case DEFVALA:
        AddOperation({ WriteDEFVALA, data});
        break;
    case DEFVALB:
        AddOperation({ WriteDEFVALB, data});
        break;
    case IPOLA:
        AddOperation({ WriteIPOLA, data});
        break;
    case IPOLB:
        AddOperation({ WriteIPOLB, data});
        break;
    case GPPUA:
        AddOperation({ WriteGPPUA, data});
        break;
    case GPPUB:
        AddOperation({ WriteGPPUB, data});
        break;
    case OLATA:
        AddOperation({ WriteOLATA, data});
        break;
    case OLATB:
        AddOperation({ WriteOLATB, data});
        break;
    }
    return true;
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessMCP23017SPIMock::AddOperation(const MCP23017Operation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}
