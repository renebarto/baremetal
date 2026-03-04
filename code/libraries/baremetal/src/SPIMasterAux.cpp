//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : SPIMasterAux.cpp
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

#include "baremetal/SPIMasterAux.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/MachineInfo.h"
#include "baremetal/String.h"
#include "baremetal/Timer.h"

/// @file
/// SPIMasterAux

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("SPIMasterAux");

#if BAREMETAL_RPI_TARGET <= 4
/// @brief Number of SPI devices for RPI 3
#define SPI_DEVICES 2
#endif

/// @brief Total number of GPIO pins used for SPI (CE2 / CE1 / CE0 / MISO / MOSI / SCLK)
#define SPI_WIRES             6
/// @brief Index of SPI CE2 pin
#define SPI_GPIO_CE2          0
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_CE1          1
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_CE0          2
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_MISO         3
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_MOSI         4
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_SCLK         5

/// @brief Value in configuration table to flag invalid combination
#define NONE                  10000

/// @brief GPIO pin configurations for SPI pins. For every bus there are multiple configurations for the two SPI pins
static unsigned s_gpioConfig[SPI_DEVICES][SPI_WIRES] = {
    // CE2, CE1, CE0, MISO, MOSI, SCLK
    { 16,   17,   18,   19,   20,   21   }, // Alt4
    { NONE, NONE, NONE, NONE, NONE, NONE }, // unused
};

/// <summary>
/// Constructor for SPIMasterAux
///
/// The default value for memoryAccess will use the singleton MemoryAccess instance. A different reference to a IMemoryAccess instance can be passed for testing
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
SPIMasterAux::SPIMasterAux(IMemoryAccess& memoryAccess /* = GetMemoryAccess()*/)
    : m_memoryAccess{memoryAccess}
    , m_device{}
    , m_baseAddress{}
    , m_clockRate{}
    , m_clockDivider{}
    , m_isInitialized{}
    , m_sclkPin{}
    , m_mosiPin{}
    , m_misoPin{}
    , m_ce0Pin{}
    , m_ce1Pin{}
    , m_ce2Pin{}
    , m_coreClockRate{}
{
}

/// <summary>
/// Destructor for SPIMasterAux
/// </summary>
SPIMasterAux::~SPIMasterAux()
{
    if (m_isInitialized)
    {
        m_memoryAccess.Write32(RPI_AUX_ENABLES, m_memoryAccess.Read32(RPI_AUX_ENABLES) & ~RPI_AUX_ENABLES_SPI1);

        m_sclkPin.SetMode(GPIOMode::InputPullUp);
        m_mosiPin.SetMode(GPIOMode::InputPullUp);
        m_misoPin.SetMode(GPIOMode::InputPullUp);
        m_ce0Pin.SetMode(GPIOMode::InputPullUp);
        m_ce1Pin.SetMode(GPIOMode::InputPullUp);
        m_ce2Pin.SetMode(GPIOMode::InputPullUp);
    }
    m_isInitialized = false;
    m_baseAddress = nullptr;
}

/// <summary>
/// Initialize the SPIMasterAux for a specific device, setting the clock as specified.
/// </summary>
/// <param name="device">SPI device index</param>
/// <param name="mode">SPI clock rate to be used in Hz</param>
/// <param name="polarity">SPI clock polarity</param>
/// <param name="phase">SPI clock phase</param>
/// <returns></returns>
bool SPIMasterAux::Initialize(uint8 device, uint32 clockRate /*= 500000*/)
{
    if (m_isInitialized)
        return true;

    LOG_INFO("Initialize SPI device %d", device);
    if ((device != 1))
        return false;

    m_device = device;
    m_baseAddress = RPI_AUX_SPI1_BASE;
    m_coreClockRate = GetMachineInfo().GetClockRate(ClockID::CORE);

    assert(m_baseAddress != 0);

    m_memoryAccess.Write32(RPI_AUX_ENABLES, m_memoryAccess.Read32(RPI_AUX_ENABLES) | RPI_AUX_ENABLES_SPI1);
    m_memoryAccess.Write32(AUX_SPI1_CNTL1_REG, 0);
    m_memoryAccess.Write32(AUX_SPI1_CNTL0_REG, RPI_AUX_SPI_CNTL0_CLEAR_FIFO);
    m_ce2Pin.AssignPin(16);
    m_ce2Pin.SetMode(GPIOMode::AlternateFunction4);
    m_ce1Pin.AssignPin(17);
    m_ce1Pin.SetMode(GPIOMode::AlternateFunction4);
    m_ce0Pin.AssignPin(18);
    m_ce0Pin.SetMode(GPIOMode::AlternateFunction4);
    m_sclkPin.AssignPin(19);
    m_sclkPin.SetMode(GPIOMode::AlternateFunction4);
    m_mosiPin.AssignPin(20);
    m_mosiPin.SetMode(GPIOMode::AlternateFunction4);
    m_misoPin.AssignPin(21);
    m_misoPin.SetMode(GPIOMode::AlternateFunction4);

    assert(m_coreClockRate > 0);

    m_isInitialized = true;

    SetClock(clockRate);

    LOG_INFO("Set up SPI device %d, clock rate %d, base address %08X", device, clockRate, m_baseAddress);
    return true;
}

/// <summary>
/// Set SPI clock rate
/// </summary>
/// <param name="clockRate">Clock rate in Hz</param>
void SPIMasterAux::SetClock(unsigned clockRate)
{
    assert(m_isInitialized);

    assert(clockRate > 0);
    m_clockRate = clockRate;

    m_clockDivider = MIN(static_cast<uint16>(((m_coreClockRate + clockRate - 1) / (2 * clockRate)) - 1), RPI_AUX_SPI_CNTL0_SPEED_MAX);

    LOG_INFO("Set clock %d", clockRate);
}

size_t SPIMasterAux::Read(SPI_CEIndex ceIndex, void* buffer, size_t count)
{
    return WriteRead(ceIndex, nullptr, buffer, count);
}

size_t SPIMasterAux::Write(SPI_CEIndex ceIndex, const void* buffer, size_t count)
{
    return WriteRead(ceIndex, buffer, nullptr, count);
}

size_t SPIMasterAux::WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count)
{
    assert(m_isInitialized);

    assert(writeBuffer != nullptr || readBuffer != nullptr);
    const uint8* writeData = reinterpret_cast<const uint8*>(writeBuffer);
    uint8* readData = reinterpret_cast<uint8*>(readBuffer);

    assert(count > 0);
    size_t writeCount = count;
    size_t readCount = count;

    uint32 control0Value = m_clockDivider << RPI_AUX_SPI_CNTL0_SPEED_SHIFT;

    switch (ceIndex)
    {
    case SPI_CEIndex::CE0:
        control0Value |= RPI_AUX_SPI_CNTL0_CS0_N;
        break;

    case SPI_CEIndex::CE1:
        control0Value |= RPI_AUX_SPI_CNTL0_CS1_N;
        break;

    case SPI_CEIndex::CE2:
        control0Value |= RPI_AUX_SPI_CNTL0_CS2_N;
        break;

    default:
        assert(false);
        break;
    }

    control0Value |= RPI_AUX_SPI_CNTL0_ENABLE;
    control0Value |= RPI_AUX_SPI_CNTL0_SHIFT_OUT_MSB_FIRST;
    control0Value |= RPI_AUX_SPI_CNTL0_VARIABLE_WIDTH;
    m_memoryAccess.Write32(AUX_SPI1_CNTL0_REG, control0Value);

    m_memoryAccess.Write32(AUX_SPI1_CNTL1_REG, RPI_AUX_SPI_CNTL1_SHIFT_IN_MSB_FIRST);

    while ((writeCount > 0) || (readCount > 0))
    {
        while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_TX_FULL) && (writeCount > 0))
        {
            uint32 numBytes = MIN(writeCount, 3);
            uint32 data = 0;

            for (unsigned i = 0; i < numBytes; i++)
            {
                uint8 nByte = (writeData != 0) ? *writeData++ : 0;
                data |= nByte << (8 * (2 - i));
            }

            data |= (numBytes * 8) << 24;
            writeCount -= numBytes;

            if (writeCount != 0)
            {
                m_memoryAccess.Write32(AUX_SPI1_TXHOLD_REG, data);
            }
            else
            {
                m_memoryAccess.Write32(AUX_SPI1_IO_REG, data);
            }
        }

        while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_RX_EMPTY) && (readCount > 0))
        {
            uint32 numBytes = MIN(readCount, 3);
            uint32 data = m_memoryAccess.Read32(AUX_SPI1_IO_REG);

            if (readBuffer != 0)
            {
                switch (numBytes)
                {
                case 3:
                    *readData++ = (uint8)((data >> 16) & 0xFF);
                    // fall through

                case 2:
                    *readData++ = (uint8)((data >> 8) & 0xFF);
                    // fall through

                case 1:
                    *readData++ = (uint8)((data >> 0) & 0xFF);
                }
            }

            readCount -= numBytes;
        }

        while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_BUSY) && (readCount > 0))
        {
            uint32 numBytes = MIN(readCount, 3);
            uint32 data = m_memoryAccess.Read32(AUX_SPI1_STAT_REG);

            if (readBuffer != 0)
            {
                switch (numBytes)
                {
                case 3:
                    *readData++ = (uint8) ((data >> 16) & 0xFF);
                    // fall through

                case 2:
                    *readData++ = (uint8) ((data >> 8) & 0xFF);
                    // fall through

                case 1:
                    *readData++ = (uint8) ((data >> 0) & 0xFF);
                }
            }

            readCount -= numBytes;
        }
    }

    int result = 0;

    return result;
}

