//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23008SPI.h
//
// Namespace   : device
//
// Class       : MCP23008SPI
//
// Description : MCP23008 SPI expander functionality
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

#include "baremetal/SPIMaster.h"
#include "device/expander/MCP23008.h"

/// @file
/// MCP23008 SPI expander support declaration

namespace device {

/// <summary>
/// Driver for MCP23008 SPI expander device
///
/// The device supports one 8 bit port.
/// Each pin can be either an input or output.
/// </summary>
class MCP23008SPI
    : public MCP23008
{
private:
    /// @brief I2C base device
    baremetal::SPIMaster m_device;
    /// @brief I2C device address
    baremetal::SPI_CEIndex m_ceIndex;

public:
    MCP23008SPI(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
    ~MCP23008SPI();

    bool Initialize(uint8 device, baremetal::SPI_CEIndex ceIndex);

    uint8 ReadRegister(MCP23008RegisterIndex registerAddress) override;
    void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte) override;
};

} // namespace device
