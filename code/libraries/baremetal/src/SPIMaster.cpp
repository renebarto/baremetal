//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : SPIMaster.cpp
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

#include "baremetal/SPIMaster.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/MachineInfo.h"
#include "baremetal/String.h"
#include "baremetal/Timer.h"

/// @file
/// SPIMaster

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("SPIMaster");

#if BAREMETAL_RPI_TARGET == 3
/// @brief Number of SPI devices for RPI 3
#define SPI_DEVICES 1
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Number of SPI devices for RPI 4 (of which 2 are not available)
#define SPI_DEVICES 7
#else
/// @brief RPI5 is not currently supported.
#endif

/// @brief Total number of GPIO pins used for SPI (CE1 / CE0 / MISO / MOSI / SCLK)
#define SPI_WIRES             5
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_CE1          0
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_CE0          1
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_MISO         2
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_MOSI         3
/// @brief Index of SPI CE1 pin
#define SPI_GPIO_SCLK         4

/// @brief Total number of different GPIO pin alternative functions
#define SPI_VALUES            2
/// @brief Index for pin number in configuration table
#define SPI_VALUE_PIN         0
/// @brief Index for alternative function in configuration table
#define SPI_VALUE_ALT         1

/// @brief Value in configuration table to flag invalid combination
#define NONE                  { 10000, 10000 }

/// @brief GPIO pin configurations for SPI pins. For every bus there are multiple configurations for the two SPI pins
static unsigned s_gpioConfig[SPI_DEVICES][SPI_WIRES][SPI_VALUES] = {
    // CE1,      CE0,       MISO,      MOSI,      SCLK
    { { 7, 0 },  { 8, 0 },  { 9, 0 },  { 10, 0 }, { 11, 0 } },
#if BAREMETAL_RPI_TARGET == 4
    { NONE,      NONE,      NONE,      NONE,      NONE      }, // unused
    { NONE,      NONE,      NONE,      NONE,      NONE      }, // unused
    { { 24, 5 }, { 0,  3 }, { 1,  3 }, { 2,  3 }, { 3 , 3 } }, // Alt3, Alt5 (CE1)
    { { 25, 5 }, { 4,  3 }, { 5,  3 }, { 6,  3 }, { 7 , 3 } }, // Alt3, Alt5 (CE1)
    { { 26, 5 }, { 12, 3 }, { 13, 3 }, { 14, 3 }, { 15, 3 } }, // Alt3, Alt5 (CE1)
    { { 27, 5 }, { 18, 3 }, { 19, 3 }, { 20, 3 }, { 21, 3 } }  // Alt3, Alt5 (CE1)
#endif
};

/// @brief Macro to determine GPIO function to be selected 
#define ALT_FUNC(alt) (((alt) == 0) ? GPIOMode::AlternateFunction0 : (((alt) == 3) ? GPIOMode::AlternateFunction3 : GPIOMode::AlternateFunction5))

/// @brief SPI register bases addresses for each bus, depening on the RPI model
static regaddr s_baseAddress[SPI_DEVICES] = {
    RPI_SPI0_BASE,
#if BAREMETAL_RPI_TARGET == 4
    0,
    0,
    RPI_SPI3_BASE,
    RPI_SPI4_BASE,
    RPI_SPI5_BASE,
    RPI_SPI6_BASE,
#endif
};

/// <summary>
/// Constructor for SPIMaster
///
/// The default value for memoryAccess will use the singleton MemoryAccess instance. A different reference to a IMemoryAccess instance can be passed for testing
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
SPIMaster::SPIMaster(IMemoryAccess& memoryAccess /* = GetMemoryAccess()*/)
    : m_memoryAccess{memoryAccess}
    , m_device{}
    , m_baseAddress{}
    , m_clockPolarity{}
    , m_clockPhase{}
    , m_csHoldTimeMicroSeconds{}
    , m_clockRate{}
    , m_isInitialized{}
    , m_sclkPin{memoryAccess}
    , m_mosiPin{memoryAccess}
    , m_misoPin{memoryAccess}
    , m_ce0Pin{memoryAccess}
    , m_ce1Pin{memoryAccess}
    , m_coreClockRate{}
{
}

/// <summary>
/// Destructor for SPIMaster
/// </summary>
SPIMaster::~SPIMaster()
{
    if (m_isInitialized)
    {
        m_ce1Pin.SetMode(GPIOMode::InputPullUp);
        m_ce0Pin.SetMode(GPIOMode::InputPullUp);
        m_misoPin.SetMode(GPIOMode::InputPullUp);
        m_mosiPin.SetMode(GPIOMode::InputPullUp);
        m_sclkPin.SetMode(GPIOMode::InputPullUp);
    }
    m_isInitialized = false;
    m_baseAddress = nullptr;
}

/// <summary>
/// Initialize the SPIMaster for a specific device, setting the clock as specified.
/// </summary>
/// <param name="device">SPI device index</param>
/// <param name="clockRate">SPI clock rate to be used in Hz</param>
/// <param name="polarity">SPI clock polarity</param>
/// <param name="phase">SPI clock phase</param>
/// <returns></returns>
bool SPIMaster::Initialize(uint8 device, uint32 clockRate /*= 500000*/, SPIClockPolarity polarity /*= SPIClockPolarity::IdleLow*/, SPIClockPhase phase /*= SPIClockPhase::Middle*/)
{
    if (m_isInitialized)
        return true;

    LOG_INFO("Initialize SPI device %d", device);
    if ((device >= SPI_DEVICES) || (s_gpioConfig[device][SPI_GPIO_SCLK][SPI_VALUE_PIN] >= NUM_GPIO))
        return false;

    m_device = device;
    m_baseAddress = s_baseAddress[m_device];
    assert(m_baseAddress != 0);

    m_ce1Pin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_CE1][SPI_VALUE_PIN]);
    m_ce1Pin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_CE1][SPI_VALUE_ALT]));
    m_ce0Pin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_CE0][SPI_VALUE_PIN]);
    m_ce0Pin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_CE0][SPI_VALUE_ALT]));
    m_misoPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_MISO][SPI_VALUE_PIN]);
    m_misoPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_MISO][SPI_VALUE_ALT]));
    m_mosiPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_MOSI][SPI_VALUE_PIN]);
    m_mosiPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_MOSI][SPI_VALUE_ALT]));
    m_sclkPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_SCLK][SPI_VALUE_PIN]);
    m_sclkPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_SCLK][SPI_VALUE_ALT]));

    m_coreClockRate = GetMachineInfo().GetClockRate(ClockID::CORE);
    assert(m_coreClockRate > 0);

    m_isInitialized = true;

    SetClock(clockRate);
    SetClockMode(polarity, phase);

    LOG_INFO("Set up SPI device %d, clock rate %d, base address %08X", device, clockRate, m_baseAddress);
    return true;
}

/// <summary>
/// Set SPI clock rate
/// </summary>
/// <param name="clockRate">Clock rate in Hz</param>
void SPIMaster::SetClock(unsigned clockRate)
{
    assert(m_isInitialized);

    assert(4000 <= clockRate && clockRate <= 125000000);
    m_clockRate = clockRate;

    uint32 divider = static_cast<uint32>(m_coreClockRate / clockRate);
    uint32 value = ((divider << RPI_SPI_CLK_CDIV_SHIFT) & RPI_SPI_CLK_CDIV_MASK);
    m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CLK_OFFSET), value);
    LOG_INFO("Set clock core %d, divider %d, clockrate %d", m_coreClockRate, divider, clockRate);
}

/// <summary>
/// Set SPI clock polarity and phase
/// </summary>
/// <param name="polarity">Clock polarity</param>
/// <param name="phase">Clock phase</param>
void SPIMaster::SetClockMode(SPIClockPolarity polarity, SPIClockPhase phase)
{
    assert(m_isInitialized);
    assert(m_baseAddress != nullptr);

    m_clockPolarity = polarity;
    m_clockPhase = phase;

    uint32 value = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET));
    value &= ~(RPI_SPI_CS_CPOL | RPI_SPI_CS_CPHA);
    value |= ((m_clockPolarity == SPIClockPolarity::IdleHigh) ? RPI_SPI_CS_CPOL_IDLE_HIGH : RPI_SPI_CS_CPOL_IDLE_LOW) | ((m_clockPhase == SPIClockPhase::Beginning) ? RPI_SPI_CS_CPHA_BEGIN_BIT : RPI_SPI_CS_CPHA_MIDDLE_BIT);
    m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);
    LOG_INFO("Set clock polarity %d, phase %d", m_clockPolarity, m_clockPhase);
}

/// <summary>
/// Set the time to hold the CS line active after the transfer is completed, in order to allow some devices to complete internal operations before the
/// line is de-asserted. The time is specified in microseconds. The default value is 0, which means that the CS line will be de-asserted immediately
/// after the transfer is completed.
/// </summary>
/// <param name="csHoldTimeMicroSeconds">CS hold time in microseconds</param>
void SPIMaster::SetCSHoldTime(uint32 csHoldTimeMicroSeconds)
{
    assert(m_isInitialized);
    assert(csHoldTimeMicroSeconds < 200);
    m_csHoldTimeMicroSeconds = csHoldTimeMicroSeconds;
}

/// <summary>
/// READ bytes FROM device
/// </summary>
/// <param name="ceIndex">CE / CS pin to activate</param>
/// <param name="buffer">Buffer for data to be received</param>
/// <param name="count">Number of bytes to receive</param>
/// <returns>Number of bytes transferred</returns>
size_t SPIMaster::Read(SPI_CEIndex ceIndex, void* buffer, size_t count)
{
    return WriteRead(ceIndex, nullptr, buffer, count);
}

/// <summary>
/// Write bytes to device
/// </summary>
/// <param name="ceIndex">CE / CS pin to activate</param>
/// <param name="buffer">Buffer containing data to send</param>
/// <param name="count">Number of bytes to send</param>
/// <returns>Number of bytes transferred</returns>
size_t SPIMaster::Write(SPI_CEIndex ceIndex, const void* buffer, size_t count)
{
    return WriteRead(ceIndex, buffer, nullptr, count);
}

/// <summary>
/// Read / Write bytes from / to device
/// Data on SPI is always transferred in both directions at the same time, so every byte written will also cause a byte to be read. If the caller is
/// only interested in writing or reading, the other buffer can be set to nullptr. In this case, the bytes read or written will be discarded.
/// </summary>
/// <param name="ceIndex">CE / CS pin to activate</param>
/// <param name="writeBuffer">Buffer containing data to send</param>
/// <param name="readBuffer">Buffer for data to be received</param>
/// <param name="count">Number of bytes to send / receive</param>
/// <returns>Number of bytes transferred</returns>
size_t SPIMaster::WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count)
{
    assert(m_isInitialized);
    assert(m_baseAddress != nullptr);
    assert(writeBuffer != nullptr || readBuffer != nullptr);

    const uint8* writeData = reinterpret_cast<const uint8*>(writeBuffer);
    uint8* readData = reinterpret_cast<uint8*>(readBuffer);

    assert(count > 0);
    assert(count <= 0xFFFF);
    TRACE_DEBUG("Set data size %d", count);
    m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_DLEN_OFFSET), count);

    TRACE_DEBUG("Start transfer");
    assert(ceIndex <= SPI_CEIndex::CE1 || ceIndex == SPI_CEIndex::None);
    uint32 value = (m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & ~RPI_SPI_CS_ACTIVATE_NONE)
                 | (static_cast<uint32>(ceIndex) << RPI_SPI_CS_ACTIVATE_SHIFT)
                 | RPI_SPI_CS_CLEAR | RPI_SPI_CS_TA;
    m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);

    TRACE_DEBUG("Read/Write");

    unsigned writeCount = 0;
    unsigned readCount = 0;

    while ((writeCount < count) || (readCount < count))
    {
        while (writeCount < count)
        {
            if (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_TXD))
                break;

            uint32 data = 0;
            if (writeData != 0)
            {
                data = *writeData++;
            }

            m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET), data);

            writeCount++;
        }

        while (readCount < count)
        {
            if (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_RXD))
                break;

            uint32 data = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET));
            if (readData != 0)
            {
                *readData++ = static_cast<uint8>(data);
            }

            readCount++;
        }
    }

    while (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_DONE))
    {
        while (m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_RXD)
        {
            m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET));
        }
    }

    if (m_csHoldTimeMicroSeconds > 0)
    {
        Timer::WaitMicroSeconds(m_csHoldTimeMicroSeconds);

        m_csHoldTimeMicroSeconds = 0;
    }

    value = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & ~RPI_SPI_CS_TA;
    m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);

    return static_cast<size_t>(count);
}
