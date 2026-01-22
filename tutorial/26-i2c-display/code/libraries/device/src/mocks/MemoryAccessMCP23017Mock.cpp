//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MemoryAccessMCP23017Mock.cpp
//
// Namespace   : device
//
// Class       : MemoryAccessMCP23017Mock
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

#include "device/mocks/MemoryAccessMCP23017Mock.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"
#include "device/i2c/MCP23017.h"

/// @file
/// MemoryAccessMCP23017Mock

/// @brief Define log name
LOG_MODULE("MemoryAccessMCP23017Mock");

using namespace baremetal;
using namespace device;

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(MCP23017OperationCode code)
{
    String result{};
    switch (code)
    {
    case MCP23017OperationCode::WriteIOCON:
        result = "WriteIOCON";
        break;
    case MCP23017OperationCode::WriteIODIRA:
        result = "WriteIODIRA";
        break;
    case MCP23017OperationCode::WriteIODIRB:
        result = "WriteIODIRB";
        break;
    case MCP23017OperationCode::WriteGPIOA:
        result = "WriteGPIOA";
        break;
    case MCP23017OperationCode::WriteGPIOB:
        result = "WriteGPIOB";
        break;
    case MCP23017OperationCode::WriteGPINTENA:
        result = "WriteGPINTENA";
        break;
    case MCP23017OperationCode::WriteGPINTENB:
        result = "WriteGPINTENB";
        break;
    case MCP23017OperationCode::WriteINTCONA:
        result = "WriteINTCONA";
        break;
    case MCP23017OperationCode::WriteINTCONB:
        result = "WriteINTCONB";
        break;
    case MCP23017OperationCode::WriteDEFVALA:
        result = "WriteDEFVALA";
        break;
    case MCP23017OperationCode::WriteDEFVALB:
        result = "WriteDEFVALB";
        break;
    case MCP23017OperationCode::WriteIPOLA:
        result = "WriteIPOLA";
        break;
    case MCP23017OperationCode::WriteIPOLB:
        result = "WriteIPOLB";
        break;
    case MCP23017OperationCode::WriteGPPUA:
        result = "WriteGPPUA";
        break;
    case MCP23017OperationCode::WriteGPPUB:
        result = "WriteGPPUB";
        break;
    case MCP23017OperationCode::WriteOLATA:
        result = "WriteOLATA";
        break;
    case MCP23017OperationCode::WriteOLATB:
        result = "WriteOLATB";
        break;
    case MCP23017OperationCode::ReadIOCON:
        result = "ReadIOCON";
        break;
    case MCP23017OperationCode::ReadIODIRA:
        result = "ReadIODIRA";
        break;
    case MCP23017OperationCode::ReadIODIRB:
        result = "ReadIODIRB";
        break;
    case MCP23017OperationCode::ReadGPIOA:
        result = "ReadGPIOA";
        break;
    case MCP23017OperationCode::ReadGPIOB:
        result = "ReadGPIOB";
        break;
    case MCP23017OperationCode::ReadGPINTENA:
        result = "ReadGPINTENA";
        break;
    case MCP23017OperationCode::ReadGPINTENB:
        result = "ReadGPINTENB";
        break;
    case MCP23017OperationCode::ReadINTCONA:
        result = "ReadINTCONA";
        break;
    case MCP23017OperationCode::ReadINTCONB:
        result = "ReadINTCONB";
        break;
    case MCP23017OperationCode::ReadDEFVALA:
        result = "ReadDEFVALA";
        break;
    case MCP23017OperationCode::ReadDEFVALB:
        result = "ReadDEFVALB";
        break;
    case MCP23017OperationCode::ReadIPOLA:
        result = "ReadIPOLA";
        break;
    case MCP23017OperationCode::ReadIPOLB:
        result = "ReadIPOLB";
        break;
    case MCP23017OperationCode::ReadGPPUA:
        result = "ReadGPPUA";
        break;
    case MCP23017OperationCode::ReadGPPUB:
        result = "ReadGPPUB";
        break;
    case MCP23017OperationCode::ReadINTFA:
        result = "ReadINTFA";
        break;
    case MCP23017OperationCode::ReadINTFB:
        result = "ReadINTFB";
        break;
    case MCP23017OperationCode::ReadINTCAPA:
        result = "ReadINTCAPA";
        break;
    case MCP23017OperationCode::ReadINTCAPB:
        result = "ReadINTCAPB";
        break;
    case MCP23017OperationCode::ReadOLATA:
        result = "ReadOLATA";
        break;
    case MCP23017OperationCode::ReadOLATB:
        result = "ReadOLATB";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a GPIO memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const MCP23017Operation &value)
{
    String result = Format("Operation=%s, ", OperationCodeToString(value.operation).c_str());
    switch (value.operation)
    {
    case MCP23017OperationCode::WriteIOCON:
    case MCP23017OperationCode::ReadIOCON:
        {
            String line = "IO configuation: ";
            if (value.argument & IOCON_BANK1)
                line += "Bank 1 ";
            else
                line += "Bank 0 ";
            if (value.argument & IOCON_MIRROR)
                line += "INT_MIRROR ";
            else
                line += "           ";
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
    case MCP23017OperationCode::WriteIODIRA:
    case MCP23017OperationCode::ReadIODIRA:
        {
            String line = "Direction Port A: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (bits & 0x80 ? "I" : "O");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23017OperationCode::WriteIODIRB:
    case MCP23017OperationCode::ReadIODIRB:
        {
            String line = "Direction Port B: ";
            uint8 bits = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (bits & 0x80 ? "I" : "O");
                bits <<= 1;
            }
            result += line;
        }
        break;
    case MCP23017OperationCode::WriteGPIOA:
    case MCP23017OperationCode::ReadGPIOA:
        {
            String line = "GPIO Port A: ";
            uint8 gpio = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (gpio & 0x80 ? "X" : " ");
                gpio <<= 1;
            }
            result += line;
        }
        break;
    case MCP23017OperationCode::WriteGPIOB:
    case MCP23017OperationCode::ReadGPIOB:
        {
            String line = "GPIO Port B: ";
            uint8 gpio = value.argument;
            for (int i = 0; i < 8; ++i)
            {
                line += (gpio & 0x80 ? "X" : " ");
                gpio <<= 1;
            }
            result += line;
        }
        break;
    case MCP23017OperationCode::WriteGPINTENA:
    case MCP23017OperationCode::ReadGPINTENA:
        {
            String line = "Interrupt enable Port A: ";
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
    case MCP23017OperationCode::WriteGPINTENB:
    case MCP23017OperationCode::ReadGPINTENB:
        {
            String line = "Interrupt enable Port B: ";
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
    case MCP23017OperationCode::WriteINTCONA:
    case MCP23017OperationCode::ReadINTCONA:
        {
            String line = "Interrupt control Port A: ";
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
    case MCP23017OperationCode::WriteINTCONB:
    case MCP23017OperationCode::ReadINTCONB:
        {
            String line = "Interrupt control Port B: ";
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
    case MCP23017OperationCode::WriteDEFVALA:
    case MCP23017OperationCode::ReadDEFVALA:
        {
            String line = "Default value Port A: ";
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
    case MCP23017OperationCode::WriteDEFVALB:
    case MCP23017OperationCode::ReadDEFVALB:
        {
            String line = "Default value Port B: ";
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
    case MCP23017OperationCode::WriteIPOLA:
    case MCP23017OperationCode::ReadIPOLA:
        {
            String line = "Input polarity Port A: ";
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
    case MCP23017OperationCode::WriteIPOLB:
    case MCP23017OperationCode::ReadIPOLB:
        {
            String line = "Input polarity Port B: ";
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
    case MCP23017OperationCode::WriteGPPUA:
    case MCP23017OperationCode::ReadGPPUA:
        {
            String line = "GPIO Pull-up Port A: ";
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
    case MCP23017OperationCode::WriteGPPUB:
    case MCP23017OperationCode::ReadGPPUB:
        {
            String line = "GPIO Pull-up Port B: ";
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
    case MCP23017OperationCode::WriteOLATA:
    case MCP23017OperationCode::ReadOLATA:
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
    case MCP23017OperationCode::WriteOLATB:
    case MCP23017OperationCode::ReadOLATB:
        {
            String line = "Output latch port B: ";
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
    case MCP23017OperationCode::ReadINTFA:
        {
            String line = "Interrupt flags Port A: ";
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
    case MCP23017OperationCode::ReadINTFB:
        {
            String line = "Interrupt flags Port ZB: ";
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
    case MCP23017OperationCode::ReadINTCAPA:
        {
            String line = "Interrupt capture Port A: ";
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
    case MCP23017OperationCode::ReadINTCAPB:
        {
            String line = "Interrupt capture Port B: ";
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
MemoryAccessMCP23017Mock* MemoryAccessMCP23017Mock::m_pThis{};

/// <summary>
/// MemoryAccessMCP23017Mock constructor
/// </summary>
MemoryAccessMCP23017Mock::MemoryAccessMCP23017Mock()
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
size_t MemoryAccessMCP23017Mock::GetNumMCP23017Operations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const MCP23017Operation &MemoryAccessMCP23017Mock::GetMCP23017Operation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

/// <summary>
/// Reset read or write cycle
/// </summary>
void MemoryAccessMCP23017Mock::ResetCycle()
{
    m_cycleStarted = false;
}

/// <summary>
/// Callback when I2C address is sent
/// </summary>
/// <param name="registers">I2C register storage, unused</param>
/// <param name="data">I2C address, unused</param>
/// <returns>True always</returns>
bool MemoryAccessMCP23017Mock::OnSendAddress(I2CMasterRegisters& /*registers*/, uint8 /*data*/)
{
    return true;
}

/// <summary>
/// Callback when I2C byte is to be received
/// </summary>
/// <param name="registers">I2C Register storage for stub</param>
/// <param name="data">Byte requested</param>
/// <returns>True always</returns>
bool MemoryAccessMCP23017Mock::OnRecvData(I2CMasterRegisters& registers, uint8& data)
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
bool MemoryAccessMCP23017Mock::OnSendData(I2CMasterRegisters& registers, uint8 data)
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
/// Read MCP23017 register
/// </summary>
/// <param name="registerIndex">Index of the register</param>
/// <param name="data">Data read on return</param>
/// <returns>True if successful, false otherwise</returns>
bool MemoryAccessMCP23017Mock::OnReadRegister(uint8 registerIndex, uint8& data)
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
bool MemoryAccessMCP23017Mock::OnWriteRegister(uint8 registerIndex, uint8 data)
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
void MemoryAccessMCP23017Mock::AddOperation(const MCP23017Operation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}
