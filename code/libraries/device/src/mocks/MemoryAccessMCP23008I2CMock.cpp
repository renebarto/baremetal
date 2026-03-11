//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23008I2CMock.cpp
//
// Namespace   : device
//
// Class       : MemoryAccessMCP23008I2CMock
//
// Description : MCP23008 SPI memory access stub
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

#include "device/mocks/MemoryAccessMCP23008I2CMock.h"

#include "baremetal/Assert.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"
#include "device/i2c/MCP23008I2C.h"

/// @file
/// MemoryAccessMCP23008I2CMock

/// @brief Define log name
LOG_MODULE("MemoryAccessMCP23008I2CMock");

using namespace baremetal;
using namespace device;

/// @brief Singleton instance
MemoryAccessMCP23008I2CMock* MemoryAccessMCP23008I2CMock::m_pThis{};

/// <summary>
/// MemoryAccessMCP23008I2CMock constructor
/// </summary>
MemoryAccessMCP23008I2CMock::MemoryAccessMCP23008I2CMock()
    : m_registers{}
    , m_cycleStarted{}
    , m_selectedRegister{}
{
    m_pThis = this;
    SetSendAddressByteCallback(OnSendAddress);
    SetRecvDataByteCallback(OnRecvData);
    SetSendDataByteCallback(OnSendData);
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessMCP23008I2CMock::GetNumMCP23008Operations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const MCP23008Operation &MemoryAccessMCP23008I2CMock::GetMCP23008Operation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

/// <summary>
/// Reset read or write cycle
/// </summary>
void MemoryAccessMCP23008I2CMock::ResetCycle()
{
    m_cycleStarted = false;
}

/// <summary>
/// Callback when I2C address is sent
/// </summary>
/// <param name="registers">I2C register storage, unused</param>
/// <param name="data">I2C address, unused</param>
/// <returns>True always</returns>
bool MemoryAccessMCP23008I2CMock::OnSendAddress(I2CMasterRegisters& /*registers*/, uint8 /*data*/)
{
    return true;
}

/// <summary>
/// Callback when I2C byte is to be received
/// </summary>
/// <param name="registers">I2C Register storage for stub</param>
/// <param name="data">Byte requested</param>
/// <returns>True always</returns>
bool MemoryAccessMCP23008I2CMock::OnRecvData(I2CMasterRegisters& registers, uint8& data)
{
    auto result = m_pThis->OnReadRegister(m_pThis->m_selectedRegister, data);
    LOG_DEBUG("Read register %02x: %02x", m_pThis->m_selectedRegister, data);
    m_pThis->ResetCycle();

    return result;
}

/// <summary>
/// Callback when I2C byte is sent
/// </summary>
/// <param name="registers">I2C Register storage for stub</param>
/// <param name="data">Byte sent</param>
/// <returns>True always</returns>
bool MemoryAccessMCP23008I2CMock::OnSendData(I2CMasterRegisters& registers, uint8 data)
{
    if (!m_pThis->m_cycleStarted)
    {
        m_pThis->m_selectedRegister = data;
        m_pThis->m_cycleStarted = true;
    }
    else
    {
        LOG_DEBUG("Write register %02x: %02x", m_pThis->m_selectedRegister, data);
        m_pThis->OnWriteRegister(m_pThis->m_selectedRegister, data);
        m_pThis->ResetCycle();
    }
    return true;
}

/// <summary>
/// Read MCP23008 register
/// </summary>
/// <param name="registerIndex">Index of the register</param>
/// <param name="data">Data read on return</param>
/// <returns>True if successful, false otherwise</returns>
bool MemoryAccessMCP23008I2CMock::OnReadRegister(uint8 registerIndex, uint8& data)
{
    uint8* registerAddress = &m_pThis->m_registers.IODIR + m_pThis->m_selectedRegister;
    data = *registerAddress;
    switch (m_pThis->m_selectedRegister)
    {
    case IOCON:
        AddOperation({ ReadIOCON, data });
        break;
    case IODIR:
        AddOperation({ ReadIODIR, data });
        break;
    case GPIO:
        AddOperation({ ReadGPIO, data });
        break;
    case GPINTEN:
        AddOperation({ ReadGPINTEN, data });
        break;
    case INTCON:
        AddOperation({ ReadINTCON, data });
        break;
    case DEFVAL:
        AddOperation({ ReadDEFVAL, data });
        break;
    case IPOL:
        AddOperation({ ReadIPOL, data });
        break;
    case GPPU:
        AddOperation({ ReadGPPU, data });
        break;
    case INTF:
        AddOperation({ ReadINTF, data });
        break;
    case INTCAP:
        AddOperation({ ReadINTCAP, data });
        break;
    case OLAT:
        AddOperation({ ReadOLAT, data });
        break;
    }
    return true;
}

/// <summary>
/// Write MCP23008 register
/// </summary>
/// <param name="registerIndex">Index of the register</param>
/// <param name="data">Data to be written</param>
/// <returns>True if successful, false otherwise</returns>
bool MemoryAccessMCP23008I2CMock::OnWriteRegister(uint8 registerIndex, uint8 data)
{
    uint8* registerAddress = &m_pThis->m_registers.IODIR + m_pThis->m_selectedRegister;
    *registerAddress = data;
    switch (m_pThis->m_selectedRegister)
    {
    case IOCON:
        AddOperation({ WriteIOCON, data});
        break;
    case IODIR:
        AddOperation({ WriteIODIR, data });
        break;
    case GPIO:
        AddOperation({ WriteGPIO, data});
        break;
    case GPINTEN:
        AddOperation({ WriteGPINTEN, data});
        break;
    case INTCON:
        AddOperation({ WriteINTCON, data});
        break;
    case DEFVAL:
        AddOperation({ WriteDEFVAL, data});
        break;
    case IPOL:
        AddOperation({ WriteIPOL, data});
        break;
    case GPPU:
        AddOperation({ WriteGPPU, data});
        break;
    case OLAT:
        AddOperation({ WriteOLAT, data});
        break;
    }
    return true;
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessMCP23008I2CMock::AddOperation(const MCP23008Operation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}
