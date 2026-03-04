//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : SPIMasterAux.h
//
// Namespace   : baremetal
//
// Class       : SPIMasterAux
//
// Description : SPI Master functionality
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

#include "baremetal/BCMRegisters.h"
#include "baremetal/ISPIMaster.h"
#include "baremetal/IMemoryAccess.h"
#include "baremetal/PhysicalGPIOPin.h"

/// @file
/// SPI Master

namespace baremetal {

/// <summary>
/// Driver for SPI master devices
///
/// GPIO pin mapping (Raspberry Pi 3-4)
/// Device | CE2    CE1    CE0    MISO   MOSI   SCLK   | Boards
/// :----: | :---------------------------------------: | :-----
/// 1      | GPIO16 GPIO17 GPIO18 GPIO19 GPIO20 GPIO21 | Raspberry Pi 3 / 4
/// 2      |                                           | Not usable
///
/// GPIO pin mapping (Raspberry Pi 5)
/// No Aux peripheral, no SPI devices
/// </summary>
class SPIMasterAux : public ISPIMaster
{
private:
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess& m_memoryAccess;
    /// @brief SPI device index
    uint8 m_device;
    /// @brief SPI device base register address
    regaddr m_baseAddress;
    /// @brief SPI clock rate (Hz)
    uint32 m_clockRate;
    /// @brief SPI clock divider
    uint16 m_clockDivider;
    /// @brief True if class is already initialized
    bool m_isInitialized;

    /// @brief GPIO pin for SCLK wire
    PhysicalGPIOPin m_sclkPin;
    /// @brief GPIO pin for MOSI wire
    PhysicalGPIOPin m_mosiPin;
    /// @brief GPIO pin for MISO wire
    PhysicalGPIOPin m_misoPin;
    /// @brief GPIO pin for CE0 wire
    PhysicalGPIOPin m_ce0Pin;
    /// @brief GPIO pin for CE1 wire
    PhysicalGPIOPin m_ce1Pin;
    /// @brief GPIO pin for CE2 wire
    PhysicalGPIOPin m_ce2Pin;

    /// @brief Core clock rate used to determine SPI clock rate in Hz
    unsigned m_coreClockRate;

public:
    SPIMasterAux(IMemoryAccess& memoryAccess = GetMemoryAccess());

    virtual ~SPIMasterAux();

    bool Initialize(uint8 device, uint32 clockRate = 500000);

    void SetClock(uint32 clockRate) override;
    size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) override;
    size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) override;
    size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) override;

private:
};

} // namespace baremetal
