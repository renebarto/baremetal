//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : SPIMaster.h
//
// Namespace   : baremetal
//
// Class       : SPIMaster
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
/// SPI clock polarity
/// </summary>
enum class SPIClockPolarity
{
    /// @brief Clock is low when idle
    IdleLow,
    /// @brief Clock is high when idle
    IdleHigh,
};

/// <summary>
/// SPI clock phase
/// </summary>
enum class SPIClockPhase
{
    /// @brief First SCLK transition at middle of data bit.
    Middle,
    /// @brief First SCLK transition at beginning of data bit.
    Beginning,
};

/// <summary>
/// Driver for SPI master devices
///
/// GPIO pin mapping (Raspberry Pi 3-4)
/// Device | CE1    CE0    MISO   MOSI   SCLK   | Boards
/// :----: | :--------------------------------: | :-----
/// 0      | GPIO07 GPIO08 GPIO09 GPIO10 GPIO11 | Raspberry Pi 3 / 4
/// 1      |                                    | Defined in SPIMasterAux class, not usable here
/// 2      |                                    | Not usable
/// 3      | GPIO24 GPIO00 GPIO01 GPIO02 GPIO03 | Raspberry Pi 4 only
/// 4      | GPIO25 GPIO04 GPIO05 GPIO06 GPIO07 | Raspberry Pi 4 only
/// 5      | GPIO26 GPIO12 GPIO13 GPIO14 GPIO15 | Raspberry Pi 4 only
/// 4      | GPIO27 GPIO18 GPIO19 GPIO20 GPIO21 | Raspberry Pi 4 only
/// GPIO0/1 are normally reserved for ID EEPROM.
///
/// GPIO pin mapping (Raspberry Pi 5)
/// Device |  CE1    CE0    MISO   MOSI   SCLK   | Boards
/// :----: | :---------------------------------: | :-----
/// 0      |  GPIO9  GPIO10 GPIO11 GPIO8  GPIO7  | Raspberry Pi 5 only
/// 1      |  GPIO19 GPIO20 GPIO21 GPIO18 GPIO17 | Raspberry Pi 5 only
/// 2      |  GPIO1  GPIO2  GPIO3  GPIO0  GPIO24 | Raspberry Pi 5 only
/// 3      |  GPIO5  GPIO6  GPIO7  GPIO4  GPIO25 | Raspberry Pi 5 only
/// 4      |                                     | None
/// 5      |  GPIO13 GPIO14 GPIO15 GPIO12 GPIO26 | Raspberry Pi 5 only
/// GPIO0/1 are normally reserved for ID EEPROM.
/// </summary>
class SPIMaster : public ISPIMaster
{
private:
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess& m_memoryAccess;
    /// @brief SPI device index
    uint8 m_device;
    /// @brief SPI device base register address
    regaddr m_baseAddress;
    /// @brief SPI clock polarity
    SPIClockPolarity m_clockPolarity;
    /// @brief SPI clock phase
    SPIClockPhase m_clockPhase;
    /// @brief SPI output hold time in microseconds. This is the time to wait after the last SCLK transition before de-asserting CS and ending the
    /// transfer, to ensure the slave device has time to process the last bit.
    uint32 m_csHoldTimeMicroSeconds;
    /// @brief SPI clock rate (Hz)
    uint32 m_clockRate;
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

    /// @brief Core clock rate used to determine SPI clock rate in Hz
    unsigned m_coreClockRate;

public:
    SPIMaster(IMemoryAccess& memoryAccess = GetMemoryAccess());

    virtual ~SPIMaster();

    bool Initialize(uint8 device, uint32 clockRate = 500000, SPIClockPolarity polarity = SPIClockPolarity::IdleLow, SPIClockPhase phase = SPIClockPhase::Middle);

    void SetClock(uint32 clockRate) override;
    void SetClockMode(SPIClockPolarity polarity, SPIClockPhase phase);
    void SetCSHoldTime(uint32 csHoldTimeMicroSeconds);
    size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) override;
    size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) override;
    size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) override;

private:
};

} // namespace baremetal
