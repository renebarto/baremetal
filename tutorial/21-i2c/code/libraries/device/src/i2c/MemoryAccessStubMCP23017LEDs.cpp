//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubMCP23017LEDs.cpp
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

#include <device/i2c/MemoryAccessStubMCP23017LEDs.h>

#include <baremetal/Assert.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Format.h>
#include <baremetal/Logger.h>
#include <baremetal/String.h>
#include <device/i2c/MCP23017.h>

/// @file
/// MemoryAccessStubMCP23017LEDs

/// @brief Define log name
LOG_MODULE("MemoryAccessStubMCP23017LEDs");

using namespace baremetal;
using namespace device;

/// @brief Singleton instance
MemoryAccessStubMCP23017LEDs* MemoryAccessStubMCP23017LEDs::pThis{};

/// <summary>
/// MemoryAccessStubMCP23017LEDs constructor
/// </summary>
MemoryAccessStubMCP23017LEDs::MemoryAccessStubMCP23017LEDs()
    : m_registers{}
    , m_cycleStarted{}
    , m_selectedRegister{}
{
    pThis = this;
    SetSendAddressByteCallback(OnSendAddress);
    SetRecvDataByteCallback(OnRecvData);
    SetSendDataByteCallback(OnSendData);
}

/// <summary>
/// Reset read or write cycle
/// </summary>
void MemoryAccessStubMCP23017LEDs::ResetCycle()
{
    m_cycleStarted = false;
}

/// <summary>
/// Callback when I2C address is sent
/// </summary>
/// <param name="registers">I2C register storage, unused</param>
/// <param name="data">I2C address, unused</param>
/// <returns>True always</returns>
bool MemoryAccessStubMCP23017LEDs::OnSendAddress(I2CRegisters &/*registers*/, uint8 /*data*/)
{
    return true;
}

/// <summary>
/// Callback when I2C byte is to be received
/// </summary>
/// <param name="registers">I2C Register storage for stub</param>
/// <param name="data">Byte requested</param>
/// <returns>True always</returns>
bool MemoryAccessStubMCP23017LEDs::OnRecvData(I2CRegisters &registers, uint8 &data)
{
    uint8* registerAddress = &pThis->m_registers.IODIRA + pThis->m_selectedRegister;
    data = *registerAddress;
    LOG_DEBUG("Read register %02x: %02x", pThis->m_selectedRegister, data);
    switch (pThis->m_selectedRegister)
    {
        case IOCONA:
        case IOCONB:
        {
            string line;
            if (data & IOCON_BANK)
                line += "Bank 1 ";
            else
                line += "Bank 0 ";
            if (data & IOCON_MIRROR)
                line += "INT_MIRROR ";
            else
                line += "           ";
            if (data & IOCON_SEQOP)
                line += "SEQOP ";
            else
                line += "      ";
            if (data & IOCON_DISSLW)
                line += "DISSLW ";
            else
                line += "       ";
            if (data & IOCON_HAEN)
                line += "HAEN ";
            else
                line += "HAEN ";
            if (data & IOCON_ODR)
                line += "ODR ";
            else
                line += "    ";
            if (data & IOCON_INTPOL)
                line += "INTPOL H";
            else
                line += "INTPOL L";
            LOG_INFO("IOCON: %s", line.c_str());
            break;
        }
        case IODIRA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += (bits & 0x80 ? "I" : "O");
                bits <<= 1;
            }
            LOG_INFO("IODIR A: %s", line.c_str());
            break;
        }
        case IODIRB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += (bits & 0x80 ? "I" : "O");
                bits <<= 1;
            }
            LOG_INFO("IODIR B: %s", line.c_str());
            break;
        }
        case GPIOA:
        {
            string line;
            uint8 gpio = data;
            for (int i = 0; i < 8; ++i)
            {
                line += (gpio & 0x80 ? "X" : " ");
                gpio <<= 1;
            }
            LOG_INFO("GPIO A: %s", line.c_str());
            break;
        }
        case GPIOB:
        {
            string line;
            uint8 gpio = data;
            for (int i = 0; i < 8; ++i)
            {
                line += (gpio & 0x80 ? "X" : " ");
                gpio <<= 1;
            }
            LOG_INFO("GPIO B: %s", line.c_str());
            break;
        }
        case GPINTENA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : " ");
                bits <<= 1;
            }
            LOG_INFO("GPINTEN A: %s", line.c_str());
            break;
        }
        case GPINTENB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : " ");
                bits <<= 1;
            }
            LOG_INFO("GPINTEN B: %s", line.c_str());
            break;
        }
        case INTCONA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "DEF " : "PRV ");
                bits <<= 1;
            }
            LOG_INFO("INTCON A: %s", line.c_str());
            break;
        }
        case INTCONB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "DEF " : "PRV ");
                bits <<= 1;
            }
            LOG_INFO("INTCON B: %s", line.c_str());
            break;
        }
        case DEFVALA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("DEFVAL A: %s", line.c_str());
            break;
        }
        case DEFVALB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("DEFVAL B: %s", line.c_str());
            break;
        }
        case IPOLA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "~" : " ");
                bits <<= 1;
            }
            LOG_INFO("IPOL A: %s", line.c_str());
            break;
        }
        case IPOLB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "~" : " ");
                bits <<= 1;
            }
            LOG_INFO("IPOL B: %s", line.c_str());
            break;
        }
        case GPPUA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "U" : "");
                bits <<= 1;
            }
            LOG_INFO("GPPU A: %s", line.c_str());
            break;
        }
        case GPPUB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "U" : "");
                bits <<= 1;
            }
            LOG_INFO("GPPU B: %s", line.c_str());
            break;
        }
        case INTFA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : "");
                bits <<= 1;
            }
            LOG_INFO("INTF A: %s", line.c_str());
            break;
        }
        case INTFB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "X" : "");
                bits <<= 1;
            }
            LOG_INFO("INTF B: %s", line.c_str());
            break;
        }
        case INTCAPA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("INTCAP A: %s", line.c_str());
            break;
        }
        case INTCAPB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("INTCAP B: %s", line.c_str());
            break;
        }
        case OLATA:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("OLAT A: %s", line.c_str());
            break;
        }
        case OLATB:
        {
            string line;
            uint8 bits = data;
            for (int i = 0; i < 8; ++i)
            {
                line += Format("Pin %d ", i);
                line += (bits & 0x80 ? "1" : "0");
                bits <<= 1;
            }
            LOG_INFO("OLAT B: %s", line.c_str());
            break;
        }
    }
    pThis->ResetCycle();

    return true;
}

/// <summary>
/// Callback when I2C byte is sent
/// </summary>
/// <param name="registers">I2C Register storage for stub</param>
/// <param name="data">Byte sent</param>
/// <returns>True always</returns>
bool MemoryAccessStubMCP23017LEDs::OnSendData(I2CRegisters &registers, uint8 data)
{
    if (!pThis->m_cycleStarted)
    {
        pThis->m_selectedRegister = data;
        pThis->m_cycleStarted = true;
    }
    else
    {
        uint8* registerAddress = &pThis->m_registers.IODIRA + pThis->m_selectedRegister;
        *registerAddress = data;
        LOG_DEBUG("Write register %02x: %02x", pThis->m_selectedRegister, data);
        switch (pThis->m_selectedRegister)
        {
            case IOCONA:
            case IOCONB:
            {
                string line;
                if (data & IOCON_BANK)
                    line += "Bank 1 ";
                else
                    line += "Bank 0 ";
                if (data & IOCON_MIRROR)
                    line += "INT_MIRROR ";
                else
                    line += "           ";
                if (data & IOCON_SEQOP)
                    line += "SEQOP ";
                else
                    line += "      ";
                if (data & IOCON_DISSLW)
                    line += "DISSLW ";
                else
                    line += "       ";
                if (data & IOCON_HAEN)
                    line += "HAEN ";
                else
                    line += "HAEN ";
                if (data & IOCON_ODR)
                    line += "ODR ";
                else
                    line += "    ";
                if (data & IOCON_INTPOL)
                    line += "INTPOL H";
                else
                    line += "INTPOL L";
                LOG_INFO("IOCON: %s", line.c_str());
                break;
            }
            case IODIRA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += (bits & 0x80 ? "I" : "O");
                    bits <<= 1;
                }
                LOG_INFO("IODIR A: %s", line.c_str());
                break;
            }
            case IODIRB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += (bits & 0x80 ? "I" : "O");
                    bits <<= 1;
                }
                LOG_INFO("IODIR B: %s", line.c_str());
                break;
            }
            case GPIOA:
            {
                string line;
                uint8 gpio = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += (gpio & 0x80 ? "X" : " ");
                    gpio <<= 1;
                }
                LOG_INFO("GPIO A: %s", line.c_str());
                break;
            }
            case GPIOB:
            {
                string line;
                uint8 gpio = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += (gpio & 0x80 ? "X" : " ");
                    gpio <<= 1;
                }
                LOG_INFO("GPIO B: %s", line.c_str());
                break;
            }
            case GPINTENA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "X" : " ");
                    bits <<= 1;
                }
                LOG_INFO("GPINTEN A: %s", line.c_str());
                break;
            }
            case GPINTENB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "X" : " ");
                    bits <<= 1;
                }
                LOG_INFO("GPINTEN B: %s", line.c_str());
                break;
            }
            case INTCONA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "DEF " : "PRV ");
                    bits <<= 1;
                }
                LOG_INFO("INTCON A: %s", line.c_str());
                break;
            }
            case INTCONB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "DEF " : "PRV ");
                    bits <<= 1;
                }
                LOG_INFO("INTCON B: %s", line.c_str());
                break;
            }
            case DEFVALA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "1" : "0");
                    bits <<= 1;
                }
                LOG_INFO("DEFVAL A: %s", line.c_str());
                break;
            }
            case DEFVALB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "1" : "0");
                    bits <<= 1;
                }
                LOG_INFO("DEFVAL B: %s", line.c_str());
                break;
            }
            case IPOLA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "~" : " ");
                    bits <<= 1;
                }
                LOG_INFO("IPOL A: %s", line.c_str());
                break;
            }
            case IPOLB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "~" : " ");
                    bits <<= 1;
                }
                LOG_INFO("IPOL B: %s", line.c_str());
                break;
            }
            case GPPUA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "U" : "");
                    bits <<= 1;
                }
                LOG_INFO("GPPU A: %s", line.c_str());
                break;
            }
            case GPPUB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "U" : "");
                    bits <<= 1;
                }
                LOG_INFO("GPPU B: %s", line.c_str());
                break;
            }
            case OLATA:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "1" : "0");
                    bits <<= 1;
                }
                LOG_INFO("OLAT A: %s", line.c_str());
                break;
            }
            case OLATB:
            {
                string line;
                uint8 bits = data;
                for (int i = 0; i < 8; ++i)
                {
                    line += Format("Pin %d ", i);
                    line += (bits & 0x80 ? "1" : "0");
                    bits <<= 1;
                }
                LOG_INFO("OLAT B: %s", line.c_str());
                break;
            }
        }
        pThis->ResetCycle();
    }
    return true;
}
