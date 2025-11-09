//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : I2CMaster.cpp
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

#include "baremetal/I2CMaster.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/MachineInfo.h"
#include "baremetal/String.h"
#include "baremetal/Timer.h"

/// @file
/// I2CMaster

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("I2CMaster");

#if BAREMETAL_RPI_TARGET == 3
/// @brief Number of I2C buses for RPI 3
#define I2C_BUSES 2
#else
/// @brief Number of I2C buses for RPI 4
#define I2C_BUSES 7
#endif

/// @brief Total number of different GPIO pin configurations for I2C pins
#define I2C_CONFIGURATIONS    3

/// @brief Total number of GPIO pins used for I2C (SCL / SDA)
#define I2C_GPIOS             2
/// @brief Index of I2C SDA pin
#define I2C_GPIO_SDA          0
/// @brief Index of I2C SCL pin
#define I2C_GPIO_SCL          1

/// @brief Value in configuration table to flag invalid combination
#define NONE                  {10000, 10000}

/// @brief I2C addresses below RESERVED_ADDRESS_LOW are reserved
#define RESERVED_ADDRESS_LOW  0x07
/// @brief I2C addresses staring from RESERVED_ADDRESS_HIGH are reserved
#define RESERVED_ADDRESS_HIGH 0x78
/// @brief Highest 10 bit address
#define ADDRESS_10BIT_HIGH    0x3FF

/// @brief GPIO pin configurations for I2C pins. For every bus there are multiple configurations for the two I2C pins
static unsigned s_gpioConfig[I2C_BUSES][I2C_CONFIGURATIONS][I2C_GPIOS] = {
    // SDA, SCL
    {{0, 1},   {28, 29}, {44, 45}}, // Alt0, Alt0, Alt1
    {{2, 3},   NONE,     NONE    }, // Alt0
#if BAREMETAL_RPI_TARGET == 4
    {NONE,     NONE,     NONE    }, // unused
    {{2, 3},   {4, 5},   NONE    }, // Alt5, Alt5
    {{6, 7},   {8, 9},   NONE    }, // Alt5, Alt5
    {{10, 11}, {12, 13}, NONE    }, // Alt5, Alt5
    {{22, 23}, NONE,     NONE    }  // Alt5
#endif
};

/// @brief I2C register bases addresses for each bus, depening on the RPI model
static regaddr s_baseAddress[I2C_BUSES] = {
    RPI_I2C0_BASE,
    RPI_I2C1_BASE,
#if BAREMETAL_RPI_TARGET == 4
    0,
    RPI_I2C3_BASE,
    RPI_I2C4_BASE,
    RPI_I2C5_BASE,
    RPI_I2C6_BASE,
#endif
};

/// @brief FIFO register mask. Only one byte can be written at once
#define RPI_I2C_FIFO_MASK 0xFF

/// @brief Size of FIFO for both read and write operations
#define RPI_I2C_FIFO_SIZE 16

/// @brief Macro to determine GPIO function to be selected for a specific bus and configuration
#define ALT_FUNC(bus, config)                                                                                                                        \
    ((bus) == 0 && (config) == 2 ? GPIOMode::AlternateFunction1 : ((bus) < 2 ? GPIOMode::AlternateFunction0 : GPIOMode::AlternateFunction5))

/// <summary>
/// Constructor for I2CMaster
///
/// The default value for memoryAccess will use the singleton MemoryAccess instance. A different reference to a IMemoryAccess instance can be passed
/// for testing
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
I2CMaster::I2CMaster(IMemoryAccess& memoryAccess /* = GetMemoryAccess()*/)
    : m_memoryAccess{memoryAccess}
    , m_bus{}
    , m_baseAddress{}
    , m_clockMode{}
    , m_config{}
    , m_isInitialized{}
    , m_sdaPin{memoryAccess}
    , m_sclPin{memoryAccess}
    , m_coreClockRate{}
    , m_clockSpeed{}
{
}

/// <summary>
/// Destructor for I2CMaster
/// </summary>
I2CMaster::~I2CMaster()
{
    if (m_isInitialized)
    {
        m_sdaPin.SetMode(GPIOMode::InputPullUp);
        m_sclPin.SetMode(GPIOMode::InputPullUp);
    }
    m_isInitialized = false;
    m_baseAddress = nullptr;
}

/// <summary>
/// Initialize the I2CMaster for a specific bus, using the specified configuration index, and setting the clock as specified.
/// </summary>
/// <param name="bus">Device number (see: comment for I2CMaster class</param>
/// <param name="mode">I2C clock rate to be used</param>
/// <param name="config">Configuration index to be used. Determines which GPIO pins are to be used for the I2C bus</param>
/// <returns></returns>
bool I2CMaster::Initialize(uint8 bus, I2CClockMode mode, uint32 config)
{
    if (m_isInitialized)
        return true;

    LOG_INFO("Initialize bus %d, mode %d, config %d", bus, static_cast<int>(mode), config);
    if ((bus >= I2C_BUSES) || (config >= I2C_CONFIGURATIONS) || (s_gpioConfig[bus][config][I2C_GPIO_SDA] >= NUM_GPIO))
        return false;

    m_bus = bus;
    m_clockMode = mode;
    m_config = config;
    m_baseAddress = s_baseAddress[m_bus];
    m_coreClockRate = GetMachineInfo().GetClockRate(ClockID::CORE);

    assert(m_baseAddress != 0);

    m_sdaPin.AssignPin(s_gpioConfig[m_bus][m_config][I2C_GPIO_SDA]);
    m_sdaPin.SetMode(ALT_FUNC(m_bus, m_config));

    m_sclPin.AssignPin(s_gpioConfig[m_bus][m_config][I2C_GPIO_SCL]);
    m_sclPin.SetMode(ALT_FUNC(m_bus, m_config));

    assert(m_coreClockRate > 0);

    m_isInitialized = true;

    SetClock(m_clockMode == I2CClockMode::FastPlus ? 1000000 : (m_clockMode == I2CClockMode::Fast ? 400000 : 100000));

    LOG_INFO("Set up bus %d, config %d, base address %08X", bus, config, m_baseAddress);
    return true;
}

/// <summary>
/// Set I2C clock rate
/// </summary>
/// <param name="clockRate">Clock rate in Hz</param>
void I2CMaster::SetClock(unsigned clockRate)
{
    assert(m_isInitialized);

    assert(clockRate > 0);
    m_clockSpeed = clockRate;

    uint16 divider = static_cast<uint16>(m_coreClockRate / clockRate);
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_DIV_OFFSET), divider);
    LOG_INFO("Set clock %d", clockRate);
}

/// <summary>
/// Scan I2C bus for existing of a device at the specified address
/// </summary>
/// <param name="address">I2C address to be scanned</param>
/// <returns>Returns true if a device was found, false otherwise</returns>
bool I2CMaster::Scan(uint16 address)
{
    uint8 data{};
    return Write(address, data) == 1;
}

/// <summary>
/// Read a single byte from the I2C device with specified address
/// </summary>
/// <param name="address">Device address</param>
/// <param name="data">Data to be received</param>
/// <returns>Returns number of bytes read, or errorcode < 0 on failure</returns>
size_t I2CMaster::Read(uint16 address, uint8& data)
{
    return Read(address, &data, 1);
}

/// <summary>
/// Read a requested number of bytes from the I2C device with specified address
/// </summary>
/// <param name="address">Device address</param>
/// <param name="buffer">Pointer to buffer to store data read</param>
/// <param name="count">Requested number of bytes to read</param>
/// <returns>Returns number of bytes read, or errorcode < 0 on failure</returns>
size_t I2CMaster::Read(uint16 address, void* buffer, size_t count)
{
    assert(m_isInitialized);

    if ((address >= ADDRESS_10BIT_HIGH) || (count == 0))
    {
        return I2C_MASTER_INVALID_PARM;
    }
    if (address >= RESERVED_ADDRESS_HIGH)
    {
        // We need to start a write / read cycle
        // The address part contains bits 8 and 9 of the address, the first write data byte the bits 0 through 7 of the address.
        // After this we read the device
        uint8 secondAddressByte = address & 0xFF;
        uint8 firstAddressByte = static_cast<uint8>(0x78 | ((address >> 8) & 0x03));
        return WriteReadRepeatedStart(firstAddressByte, &secondAddressByte, 1, buffer, count);
    }

    uint8 addressByte{static_cast<uint8>(address & 0x7F)};

    uint8* data = reinterpret_cast<uint8*>(buffer);
    assert(data != nullptr);

    size_t result = 0;

    WriteAddressRegister(addressByte);

    ClearFIFO();
    ClearAllStatus();
    WriteDataLengthRegister(count);
    StartReadTransfer();

    // Transfer active
    while (!TransferDone())
    {
        while (ReceiveFIFOHasData())
        {
            *data++ = ReadFIFORegister();
            TRACE_DEBUG("Read byte from %02x, data %02x", address, *(data - 1));

            count--;
            result++;
        }
    }

    // Transfer has finished, grab any remaining stuff from FIFO
    while ((count > 0) && ReceiveFIFOHasData())
    {
        *data++ = ReadFIFORegister();
        TRACE_DEBUG("Read extra byte from %02x, data %02x", address, *(data - 1));

        count--;
        result++;
    }

    TRACE_DEBUG("Read result = %d", result);

    uint32 status = ReadStatusRegister();
    if (status & RPI_I2C_S_ERR)
    {
        // Clear error bit
        ClearNAck();

        result = I2C_MASTER_ERROR_NACK;
    }
    else if (status & RPI_I2C_S_CLKT)
    {
        ClearClockStretchTimeout();
        result = I2C_MASTER_ERROR_CLKT;
    }
    else if (count > 0)
    {
        result = I2C_MASTER_DATA_LEFT;
    }

    // Clear done bit
    ClearDone();

    return result;
}

/// <summary>
/// Write a single byte to the I2C device with specified address
/// </summary>
/// <param name="address">Device address</param>
/// <param name="data">Data to write</param>
/// <returns>Returns number of bytes written, or errorcode < 0 on failure</returns>
size_t I2CMaster::Write(uint16 address, uint8 data)
{
    return Write(address, &data, 1);
}

/// <summary>
/// Write a requested number of bytes to the I2C device with specified address
/// </summary>
/// <param name="address">Device address</param>
/// <param name="buffer">Pointer to buffer containing data to sebd</param>
/// <param name="count">Requested number of bytes to write</param>
/// <returns>Returns number of bytes written, or errorcode < 0 on failure</returns>
size_t I2CMaster::Write(uint16 address, const void* buffer, size_t count)
{
    assert(m_isInitialized);

    if (address >= ADDRESS_10BIT_HIGH)
    {
        return I2C_MASTER_INVALID_PARM;
    }

    if ((count != 0) && (buffer == nullptr))
    {
        return I2C_MASTER_INVALID_PARM;
    }

    const uint8* data = reinterpret_cast<const uint8*>(buffer);
    assert(data != nullptr);

    int result = 0;

    ClearFIFO();

    unsigned bytesWritten{};
    uint8 addressByte{static_cast<uint8>(address & 0x7F)};
    if (address >= RESERVED_ADDRESS_HIGH)
    {
        // We need to write the low 8 bits of the address first
        // The address part contains bits 8 and 9 of the address, the first write data byte the bits 0 through 7 of the address.
        uint8 secondAddressByte = address & 0xFF;
        addressByte = static_cast<uint8>(0x78 | ((address >> 8) & 0x03));
        WriteFIFORegister(secondAddressByte);
        bytesWritten++;
    }

    WriteAddressRegister(addressByte);
    ClearAllStatus();
    WriteDataLengthRegister(static_cast<uint8>(bytesWritten + count));

    // Fill FIFO
    for (; count > 0 && bytesWritten < RPI_I2C_FIFO_SIZE; bytesWritten++)
    {
        WriteFIFORegister(*data++);
        count--;
        result++;
    }

    StartWriteTransfer();

    // Transfer active
    while (!TransferDone())
    {
        while ((count > 0) && TransmitFIFOHasSpace())
        {
            TRACE_DEBUG("Write extra byte to %02x, data %02x", address, *data);
            WriteFIFORegister(*data++);
            count--;
            result++;
        }
        Timer::WaitMilliSeconds(1);
    }

    TRACE_DEBUG("Write result = %d", result);

    // Check status
    uint32 status = ReadStatusRegister();
    if (status & RPI_I2C_S_ERR)
    {
        // Clear error bit
        ClearNAck();

        result = I2C_MASTER_ERROR_NACK;
    }
    else if (status & RPI_I2C_S_CLKT)
    {
        ClearClockStretchTimeout();
        result = I2C_MASTER_ERROR_CLKT;
    }
    else if (count > 0)
    {
        result = I2C_MASTER_DATA_LEFT;
    }

    // Clear done bit
    ClearDone();

    while (ReceiveFIFOHasData())
    {
        uint8 data = ReadFIFORegister();
        TRACE_DEBUG("Read byte = %02x", data);
    }

    return result;
}

/// <summary>
/// Consecutive write and read operation with repeated start
/// </summary>
/// <param name="address">I2C address of target device</param>
/// <param name="writeBuffer">Write data for will be taken from here</param>
/// <param name="writeCount">Number of bytes to be written (max. 16)</param>
/// <param name="readBuffer">Read data will be stored here</param>
/// <param name="readCount">Number of bytes to be read</param>
/// <returns>Returns number of bytes read, or errorcode < 0 on failure</returns>
size_t I2CMaster::WriteReadRepeatedStart(uint16 address, const void* writeBuffer, size_t writeCount, void* readBuffer, size_t readCount)
{
    assert(m_isInitialized);

    if (address >= ADDRESS_10BIT_HIGH)
    {
        return I2C_MASTER_INVALID_PARM;
    }

    if ((writeCount == 0) || (writeCount > ((address >= 0x78) ? RPI_I2C_FIFO_SIZE - 1 : RPI_I2C_FIFO_SIZE)) || (writeBuffer == nullptr) ||
        (readCount == 0) || (readBuffer == nullptr))
    {
        return I2C_MASTER_INVALID_PARM;
    }

    const uint8* writeData = reinterpret_cast<const uint8*>(writeBuffer);
    assert(writeData != nullptr);

    int result = 0;

    ClearFIFO();

    unsigned bytesWritten{};
    uint8 addressByte{static_cast<uint8>(address & 0x7F)};
    if (address >= RESERVED_ADDRESS_HIGH)
    {
        // We need to write the low 8 bits of the address first
        // The address part contains bits 8 and 9 of the address, the first write data byte the bits 0 through 7 of the address.
        uint8 secondAddressByte = address & 0xFF;
        addressByte = static_cast<uint8>(0x78 | ((address >> 8) & 0x03));
        WriteFIFORegister(secondAddressByte);
        bytesWritten++;
    }

    WriteAddressRegister(addressByte);
    ClearAllStatus();
    WriteDataLengthRegister(static_cast<uint8>(bytesWritten + writeCount));

    // Fill FIFO
    for (; writeCount > 0 && bytesWritten < RPI_I2C_FIFO_SIZE; bytesWritten++)
    {
        WriteFIFORegister(*writeData++);

        writeCount--;
        result++;
    }

    StartWriteTransfer();

    // Poll to check transfer has started
    while (!TransferActive())
    {
        if (TransferDone())
        {
            break;
        }
    }

    uint8* readData = reinterpret_cast<uint8*>(readBuffer);

    WriteDataLengthRegister(readCount);
    StartReadTransfer();

    assert(m_clockSpeed > 0);
    Timer::WaitMicroSeconds((writeCount + 1) * 9 * 1000000 / m_clockSpeed);

    // Transfer active
    while (!TransferDone())
    {
        while ((readCount > 0) && ReceiveFIFOHasData())
        {
            *readData++ = ReadFIFORegister();

            readCount--;
            result++;
        }
    }

    // Transfer has finished, grab any remaining stuff from FIFO
    while ((readCount > 0) && ReceiveFIFOHasData())
    {
        *readData++ = ReadFIFORegister();

        readCount--;
        result++;
    }

    // Check status
    uint32 status = ReadStatusRegister();
    if (status & RPI_I2C_S_ERR)
    {
        // Clear error bit
        ClearNAck();

        result = I2C_MASTER_ERROR_NACK;
    }
    else if (status & RPI_I2C_S_CLKT)
    {
        ClearClockStretchTimeout();
        result = I2C_MASTER_ERROR_CLKT;
    }
    else if (readCount > 0)
    {
        result = I2C_MASTER_DATA_LEFT;
    }

    // Clear done bit
    ClearDone();

    return result;
}

/// <summary>
/// Read the I2C Control Register
/// </summary>
/// <returns>Value read from I2C Control Register</returns>
uint32 I2CMaster::ReadControlRegister()
{
    auto result = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_C_OFFSET));
    if (GetLogger().IsLogSeverityEnabled(LogSeverity::Data))
    {
        String text;
        text += (result & RPI_I2C_C_ENABLE) ? "EN " : "   ";
        text += (result & RPI_I2C_C_INTR_ENABLE) ? "IR " : "  ";
        text += (result & RPI_I2C_C_INTT_ENABLE) ? "IT " : "  ";
        text += (result & RPI_I2C_C_INTD_ENABLE) ? "ID " : "  ";
        text += (result & RPI_I2C_C_ST) ? "ST " : "  ";
        text += (result & RPI_I2C_C_CLEAR) ? "CL " : "  ";
        text += (result & RPI_I2C_C_READ) ? "RD " : "WR ";
        TRACE_DATA("Read I2C Control, %s", text.c_str());
    }
    return result;
}

/// <summary>
/// Write to the I2C Control Register
/// </summary>
/// <param name="data">Value to write</param>
void I2CMaster::WriteControlRegister(uint32 data)
{
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_C_OFFSET), data);
    if (GetLogger().IsLogSeverityEnabled(LogSeverity::Data))
    {
        String text;
        text += (data & RPI_I2C_C_ENABLE) ? "EN " : "   ";
        text += (data & RPI_I2C_C_INTR_ENABLE) ? "IR " : "  ";
        text += (data & RPI_I2C_C_INTT_ENABLE) ? "IT " : "  ";
        text += (data & RPI_I2C_C_INTD_ENABLE) ? "ID " : "  ";
        text += (data & RPI_I2C_C_ST) ? "ST " : "  ";
        text += (data & RPI_I2C_C_CLEAR) ? "CL " : "  ";
        text += (data & RPI_I2C_C_READ) ? "RD " : "WR ";
        TRACE_DATA("Write I2C Control, %s", text.c_str());
    }
}

/// <summary>
/// Start a read transfer
/// </summary>
void I2CMaster::StartReadTransfer()
{
    WriteControlRegister(ReadControlRegister() | RPI_I2C_C_ENABLE | RPI_I2C_C_ST | RPI_I2C_C_READ);
}

/// <summary>
/// Start a write transfer
/// </summary>
void I2CMaster::StartWriteTransfer()
{
    WriteControlRegister((ReadControlRegister() & ~RPI_I2C_C_READ) | RPI_I2C_C_ENABLE | RPI_I2C_C_ST | RPI_I2C_C_WRITE);
}

/// <summary>
/// Clear the FIFO. This clears both read and write FIFO
/// </summary>
void I2CMaster::ClearFIFO()
{
    WriteControlRegister(ReadControlRegister() | RPI_I2C_C_CLEAR);
}

/// <summary>
/// Write to the I2C Address Register
/// </summary>
/// <param name="data">Value to write</param>
void I2CMaster::WriteAddressRegister(uint8 data)
{
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_A_OFFSET), data);
    TRACE_DEBUG("Write I2C Address, %02x", data);
}

/// <summary>
/// Write to the I2C Data Length Register
/// </summary>
/// <param name="data">Value to write</param>
void I2CMaster::WriteDataLengthRegister(uint8 data)
{
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_DLEN_OFFSET), data);
    TRACE_DEBUG("Write I2C Length, %08x", data);
}

/// <summary>
/// Read the I2C Status Register
/// </summary>
/// <returns>Value read from I2C Status Register</returns>
uint32 I2CMaster::ReadStatusRegister()
{
    auto data = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_S_OFFSET));
    if (GetLogger().IsLogSeverityEnabled(LogSeverity::Data))
    {
        String text;
        text += (data & RPI_I2C_S_CLKT) ? "CLKT " : "     ";
        text += (data & RPI_I2C_S_ERR) ? "ERR " : "    ";
        text += (data & RPI_I2C_S_RXF) ? "RXF " : "    ";
        text += (data & RPI_I2C_S_TXE) ? "TXE " : "    ";
        text += (data & RPI_I2C_S_RXD) ? "RXD " : "    ";
        text += (data & RPI_I2C_S_TXD) ? "TXD " : "    ";
        text += (data & RPI_I2C_S_RXR) ? "RXR " : "    ";
        text += (data & RPI_I2C_S_TXW) ? "TXW " : "    ";
        text += (data & RPI_I2C_S_DONE) ? "DONE " : "     ";
        text += (data & RPI_I2C_S_TA) ? "TA " : "     ";
        TRACE_DATA("Read I2C Status, %s", text.c_str());
    }
    return data;
}

/// <summary>
/// Write to the I2C Status Register
/// </summary>
/// <param name="data">Value to write</param>
void I2CMaster::WriteStatusRegister(uint32 data)
{
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_S_OFFSET), data);
    if (GetLogger().IsLogSeverityEnabled(LogSeverity::Data))
    {
        String text;
        text += (data & RPI_I2C_S_CLKT) ? "CLKT " : "     ";
        text += (data & RPI_I2C_S_ERR) ? "ERR " : "    ";
        text += (data & RPI_I2C_S_DONE) ? "DONE " : "     ";
        TRACE_DATA("Write I2C Status, %s", text.c_str());
    }
}

/// <summary>
/// Check whether a clock stretch timeout has occurred
/// </summary>
/// <returns>Returns true if a clock stretch timeout has occurred</returns>
bool I2CMaster::HasClockStretchTimeout()
{
    return (ReadStatusRegister() & RPI_I2C_S_CLKT) != 0;
}

/// <summary>
/// Check whether the latest byte has received an ACK
/// </summary>
/// <returns>Returns true if the latest byte has received an ACK</returns>
bool I2CMaster::HasAck()
{
    return (ReadStatusRegister() & RPI_I2C_S_ERR) == 0;
}

/// <summary>
/// Check whether the latest byte has not received an ACK
/// </summary>
/// <returns>Returns true if the latest byte has not received an ACK</returns>
bool I2CMaster::HasNAck()
{
    return (ReadStatusRegister() & RPI_I2C_S_ERR) != 0;
}

/// <summary>
/// Check whether the read FIFO is full
/// </summary>
/// <returns>Returns true if the read FIFO is full</returns>
bool I2CMaster::ReceiveFIFOFull()
{
    return (ReadStatusRegister() & RPI_I2C_S_RXF) != 0;
}

/// <summary>
/// Check whether the read FIFO contains data
/// </summary>
/// <returns>Returns true if the read FIFO contains data</returns>
bool I2CMaster::ReceiveFIFOHasData()
{
    return (ReadStatusRegister() & RPI_I2C_S_RXD) != 0;
}

/// <summary>
/// Check whether the read FIFO needs to be read. This means the FIFO is 3/4 full or more
/// </summary>
/// <returns>Returns true if the read FIFO needs to be read</returns>
bool I2CMaster::ReceiveFIFONeedsReading()
{
    return (ReadStatusRegister() & RPI_I2C_S_RXR) != 0;
}

/// <summary>
/// Check whether the write FIFO is empty
/// </summary>
/// <returns>Returns true if the write FIFO is empty</returns>
bool I2CMaster::TransmitFIFOEmpty()
{
    return (ReadStatusRegister() & RPI_I2C_S_TXE) != 0;
}

/// <summary>
/// Check whether the write FIFO has space to write
/// </summary>
/// <returns>Returns true if the write FIFO has space to write</returns>
bool I2CMaster::TransmitFIFOHasSpace()
{
    return (ReadStatusRegister() & RPI_I2C_S_TXD) != 0;
}

/// <summary>
/// Check whether the write FIFO needs to be written to. This means the FIFO is 1/4 full or less
/// </summary>
/// <returns>Returns true if the write FIFO needs to be written to</returns>
bool I2CMaster::TransmitFIFONeedsWriting()
{
    return (ReadStatusRegister() & RPI_I2C_S_TXW) != 0;
}

/// <summary>
/// Check whether the transfer is done
/// </summary>
/// <returns>Returns true if the transfer is done</returns>
bool I2CMaster::TransferDone()
{
    return (ReadStatusRegister() & RPI_I2C_S_DONE) != 0;
}

/// <summary>
/// Check whether a transfer is active
/// </summary>
/// <returns>Returns true if a transfer is active</returns>
bool I2CMaster::TransferActive()
{
    return (ReadStatusRegister() & RPI_I2C_S_TA) != 0;
}

/// <summary>
/// Clear the CLKT (Clock Stretch Timeout) bit in the I2C Status Register
/// </summary>
void I2CMaster::ClearClockStretchTimeout()
{
    WriteStatusRegister(RPI_I2C_S_CLKT);
}

/// <summary>
/// Clear the ERR bit in the I2C Status Register
/// </summary>
void I2CMaster::ClearNAck()
{
    WriteStatusRegister(RPI_I2C_S_ERR);
}

/// <summary>
/// Clear the DONE bit in the I2C Status Register
/// </summary>
void I2CMaster::ClearDone()
{
    WriteStatusRegister(RPI_I2C_S_DONE);
}

/// <summary>
/// Clear the CLKT, ERR and DONE bits in the I2C Status Register
/// </summary>
void I2CMaster::ClearAllStatus()
{
    WriteStatusRegister(RPI_I2C_S_CLKT | RPI_I2C_S_ERR | RPI_I2C_S_DONE);
}

/// <summary>
/// Read the I2C FIFO Register
/// </summary>
/// <returns>Value read from I2C FIFO Register</returns>
uint8 I2CMaster::ReadFIFORegister()
{
    uint8 data = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_FIFO_OFFSET)) & RPI_I2C_FIFO_MASK;
    TRACE_DEBUG("Read FIFO, data %08x", data);
    return data;
}

/// <summary>
/// Write to the I2C FIFO Register
/// </summary>
/// <param name="data">Value to write</param>
void I2CMaster::WriteFIFORegister(uint8 data)
{
    m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_FIFO_OFFSET), data);
    TRACE_DEBUG("Write FIFO, data %08x", data);
}
