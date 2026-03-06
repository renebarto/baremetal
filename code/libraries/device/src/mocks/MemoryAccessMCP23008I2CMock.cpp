//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23008I2CMock.cpp
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

#include "device/mocks/MemoryAccessMCP23008I2CMock.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
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

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(MCP23008OperationCode code)
{
    String result{};
    switch (code)
    {
    case MCP23008OperationCode::WriteIOCON:
        result = "WriteIOCON";
        break;
    case MCP23008OperationCode::WriteIODIR:
        result = "WriteIODIR";
        break;
    case MCP23008OperationCode::WriteGPIO:
        result = "WriteGPIO";
        break;
    case MCP23008OperationCode::WriteGPINTEN:
        result = "WriteGPINTEN";
        break;
    case MCP23008OperationCode::WriteINTCON:
        result = "WriteINTCON";
        break;
    case MCP23008OperationCode::WriteDEFVAL:
        result = "WriteDEFVAL";
        break;
    case MCP23008OperationCode::WriteIPOL:
        result = "WriteIPOL";
        break;
    case MCP23008OperationCode::WriteGPPU:
        result = "WriteGPPU";
        break;
    case MCP23008OperationCode::WriteOLAT:
        result = "WriteOLAT";
        break;
    case MCP23008OperationCode::ReadIOCON:
        result = "ReadIOCON";
        break;
    case MCP23008OperationCode::ReadIODIR:
        result = "ReadIODIR";
        break;
    case MCP23008OperationCode::ReadGPIO:
        result = "ReadGPIO";
        break;
    case MCP23008OperationCode::ReadGPINTEN:
        result = "ReadGPINTEN";
        break;
    case MCP23008OperationCode::ReadINTCON:
        result = "ReadINTCON";
        break;
    case MCP23008OperationCode::ReadDEFVAL:
        result = "ReadDEFVAL";
        break;
    case MCP23008OperationCode::ReadIPOL:
        result = "ReadIPOL";
        break;
    case MCP23008OperationCode::ReadGPPU:
        result = "ReadGPPU";
        break;
    case MCP23008OperationCode::ReadINTF:
        result = "ReadINTF";
        break;
    case MCP23008OperationCode::ReadINTCAP:
        result = "ReadINTCAP";
        break;
    case MCP23008OperationCode::ReadOLAT:
        result = "ReadOLAT";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a GPIO memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const MCP23008Operation &value)
{
    String result = Format("Operation=%s, ", OperationCodeToString(value.operation).c_str());
    switch (value.operation)
    {
    case MCP23008OperationCode::WriteIOCON:
    case MCP23008OperationCode::ReadIOCON:
        {
            String line = "IO configuation: ";
            if (value.argument & IOCON_SEQOP)
                line += "SEQOP ";
            else
                line += "      ";
            if (value.argument & IOCON_DISSLW)
                line += "DISSLW ";
            else
                line += "       ";
            if (value.argument & IOCON_HAEN)
                line += "HAEN ";
            else
                line += "HAEN ";
            if (value.argument & IOCON_ODR)
                line += "ODR ";
            else
                line += "    ";
            if (value.argument & IOCON_INTPOL)
                line += "INTPOL H";
            else
                line += "INTPOL L";
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteIODIR:
    case MCP23008OperationCode::ReadIODIR:
        {
            String line = "Direction: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (bits & 0x80 ? "I" : "O");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteGPIO:
    case MCP23008OperationCode::ReadGPIO:
        {
            String line = "GPIO: ";
            uint8 gpio = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (gpio & 0x80 ? "X" : " ");
                gpio <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteGPINTEN:
    case MCP23008OperationCode::ReadGPINTEN:
        {
            String line = "Interrupt enable: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : " ");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteINTCON:
    case MCP23008OperationCode::ReadINTCON:
        {
            String line = "Interrupt control: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "DEF " : "PRV ");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteDEFVAL:
    case MCP23008OperationCode::ReadDEFVAL:
        {
            String line = "Default value: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteIPOL:
    case MCP23008OperationCode::ReadIPOL:
        {
            String line = "Input polarity: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "~" : " ");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::WriteGPPU:
    case MCP23008OperationCode::ReadGPPU:
        {
            String line = "GPIO Pull-up: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "U" : "");
                bits <<= 1;
            }
            result += line;
            break;
        }
    case MCP23008OperationCode::WriteOLAT:
    case MCP23008OperationCode::ReadOLAT:
        {
            String line = "Output latch port A: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            result += line;
            break;
        }
    case MCP23008OperationCode::ReadINTF:
        {
            String line = "Interrupt flags: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : "");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23008OperationCode::ReadINTCAP:
        {
            String line = "Interrupt capture: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            result += line;
        }
        break;

    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

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
