//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MCP23008Mock.cpp
//
// Namespace   : device
//
// Class       : MCP23008Mock
//
// Description : MCP23008 mocking functionality shared between I2C and SPI mocks
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

#include "device/mocks/MCP23008Mock.h"

#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"
#include "device/expander/MCP23008.h"

/// @file
/// MCP23008Mock

/// @brief Define log name
LOG_MODULE("MCP23008Mock");

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
