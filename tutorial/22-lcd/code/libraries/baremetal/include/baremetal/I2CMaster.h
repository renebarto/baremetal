//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : I2CMaster.h
//
// Namespace   : baremetal
//
// Class       : I2CMaster
//
// Description : I2C Master functionality
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

#include <baremetal/II2CMaster.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/IMemoryAccess.h>
#include <baremetal/PhysicalGPIOPin.h>

/// @file
/// I2C Master

namespace baremetal {

/// <summary>
/// Driver for I2C master devices
///
/// GPIO pin mapping (Raspberry Pi 3-4)
/// bus       | config 0      | config 1      | config 2      | Boards
/// :-------: | :-----------: | :-----------: | :-----------: | :-----
/// ^         | SDA    SCL    | SDA    SCL    | SDA    SCL    | ^
/// 0         | GPIO0  GPIO1  | GPIO28 GPIO29 | GPIO44 GPIO45 | Raspberry Pi 3 / 4
/// 1         | GPIO2  GPIO3  |               |               | Raspberry Pi 3 only
/// 2         |               |               |               | None
/// 3         | GPIO2  GPIO3  | GPIO4  GPIO5  |               | Raspberry Pi 4 only
/// 4         | GPIO6  GPIO7  | GPIO8  GPIO9  |               | Raspberry Pi 4 only
/// 5         | GPIO10 GPIO11 | GPIO12 GPIO13 |               | Raspberry Pi 4 only
/// 6         | GPIO22 GPIO23 |               |               | Raspberry Pi 4 only
///
/// GPIO pin mapping (Raspberry Pi 5)
/// bus       | config 0      | config 1      | config 2      | Boards
/// :-------: | :-----------: | :-----------: | :-----------: | :-----
/// ^         | SDA    SCL    | SDA    SCL    | SDA    SCL    | ^
/// 0         | GPIO0  GPIO1  | GPIO8  GPIO9  |               | Raspberry Pi 5 only
/// 1         | GPIO2  GPIO3  | GPIO10 GPIO11 |               | Raspberry Pi 5 only
/// 2         | GPIO4  GPIO5  | GPIO12 GPIO13 |               | Raspberry Pi 5 only
/// 3         | GPIO6  GPIO7  | GPIO14 GPIO15 | GPIO22 GPIO23 | Raspberry Pi 5 only
/// </summary>
class I2CMaster
    : public II2CMaster
{
private:
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess&  m_memoryAccess;
    /// @brief I2C bus index
    uint8           m_bus;
    /// @brief I2C bus base register address
    regaddr         m_baseAddress;
    /// @brief I2C bus clock rate
    I2CClockMode    m_clockMode;
    /// @brief I2C bus GPIO configuration index used
    uint32          m_config;
    /// @brief True if class is already initialized
    bool            m_isInitialized;

    /// @brief GPIO pin for SDA wire
    PhysicalGPIOPin m_sdaPin;
    /// @brief GPIO pin for SCL wire
    PhysicalGPIOPin m_sclPin;

    /// @brief Core clock rate used to determine I2C clock rate in Hz
    unsigned        m_coreClockRate;
    /// @brief I2C clock rate in Hz
    unsigned        m_clockSpeed;

public:
    I2CMaster(IMemoryAccess &memoryAccess = GetMemoryAccess());

    virtual ~I2CMaster();

    bool Initialize(uint8 bus, I2CClockMode mode = I2CClockMode::Normal, uint32 config = 0);

    void SetClock(unsigned clockRate);
    bool Scan(uint16 address);
    size_t Read(uint16 address, uint8 &data) override;
    size_t Read(uint16 address, void *buffer, size_t count) override;
    size_t Write(uint16 address, uint8 data) override;
    size_t Write(uint16 address, const void *buffer, size_t count) override;
    size_t WriteReadRepeatedStart(uint16 address, const void *writeBuffer, size_t writeCount, void *readBuffer, size_t readCount) override;

private:
    uint32 ReadControlRegister();
    void WriteControlRegister(uint32 data);
    void StartReadTransfer();
    void StartWriteTransfer();
    void ClearFIFO();
    void WriteAddressRegister(uint8 data);
    void WriteDataLengthRegister(uint8 data);
    uint32 ReadStatusRegister();
    void WriteStatusRegister(uint32 data);
    bool HasClockStretchTimeout();
    bool HasAck();
    bool HasNAck();
    bool ReceiveFIFOFull();
    bool ReceiveFIFOHasData();
    bool ReceiveFIFONeedsReading();
    bool TransmitFIFOEmpty();
    bool TransmitFIFOHasSpace();
    bool TransmitFIFONeedsWriting();
    bool TransferDone();
    bool TransferActive();
    void ClearClockStretchTimeout();
    void ClearNAck();
    void ClearDone();
    void ClearAllStatus();
    uint8 ReadFIFORegister();
    void WriteFIFORegister(uint8 data);
};

} // namespace baremetal
