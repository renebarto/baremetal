//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessI2CMasterMock.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessI2CMasterMock
//
// Description : I2C master memory access mock class
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

#include "baremetal/mocks/FIFO.h"
#include "baremetal/mocks/MemoryAccessGPIOMock.h"

#include "baremetal/I2CMaster.h"
#include "baremetal/String.h"
#include "stdlib/Macros.h"

/// @file
/// Memory access mock class

namespace baremetal {

/// @brief I2C master registers storage
struct I2CMasterRegisters
{
    /// @brief RPI_I2C_C value
    uint32 Control; // 0x00
    /// @brief RPI_I2C_S value
    uint32 Status; // 0x04
    /// @brief RPI_I2C_DLEN value
    uint32 DataLength; // 0x08
    /// @brief RPI_I2C_A value
    uint32 Address; // 0x0C
    /// @brief RPI_I2C_FIFO value on write
    uint32 FIFOWrite; // 0x10
    /// @brief RPI_I2C_DIV value
    uint32 ClockDivider; // 0x14
    /// @brief RPI_I2C_DEL value
    uint32 DataDelay; // 0x18
    /// @brief RPI_I2C_CLKT value
    uint32 ClockStretchTimeout; // 0x1C

    /// <summary>
    /// Constructor for I2CMasterRegisters
    ///
    /// Sets default register values
    /// </summary>
    I2CMasterRegisters()
        : Control{}
        , Status{0x00000050}
        , DataLength{}
        , Address{}
        , FIFOWrite{}
        , ClockDivider{0x000005DC}
        , DataDelay{0x00300030}
        , ClockStretchTimeout{0x00000040}
    {
    }
} PACKED;

/// @brief I2C master operation codes
enum I2CMasterOperationCode
{
    /// @brief Enable or disable I2C controller
    EnableController,
    /// @brief Enable RX interrupt
    EnableRXInterrupt,
    /// @brief Enable TX interrupt
    EnableTXInterrupt,
    /// @brief Enable Done interrupt
    EnableDoneInterrupt,
    /// @brief Set read mode
    SetReadMode,
    /// @brief Set write mode
    SetWriteMode,
    /// @brief Clear FIFO
    ClearFIFO,
    /// @brief Start transfer
    StartTransfer,
    /// @brief Reset clock stretch timeout
    ResetClockStretchTimeout,
    /// @brief Reset acknowledge error
    ResetAckError,
    /// @brief Reset done
    ResetDone,
    /// @brief Set data length
    SetDataLength,
    /// @brief Set address
    SetAddress,
    /// @brief Write to FIFO
    WriteFIFO,
    /// @brief Set clock divider
    SetClockDivider,
    /// @brief Set falling edge delay
    SetFallingEdgeDelay,
    /// @brief Set rising edge delay
    SetRisingEdgeDelay,
    /// @brief Set clock stretch timeout
    SetClockStretchTimeout,
    /// @brief Read from FIFO
    ReadFIFO,
    /// @brief Set acknowledge error
    SetAckError,
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct I2CMasterOperation
{
    /// @brief I2C master operation code
    I2CMasterOperationCode operation; // Size: 4 bytes
    /// @brief I2C bus
    uint8 bus; // Size: 1 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    I2CMasterOperation()
        : operation{}
        , bus{}
        , argument{}
    {
    }
     /// <summary>
    /// Constructor for read or 2write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theBus">I2C bus</param>
    /// <param name="theArgument">Argument value</param>
    I2CMasterOperation(I2CMasterOperationCode theOperation, uint8 theBus, uint32 theArgument = 0)
        : operation{theOperation}
        , bus{theBus}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const I2CMasterOperation& other) const
    {
        return (other.operation == operation) &&
            (other.bus == bus) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const I2CMasterOperation& other) const
    {
        return !operator==(other);
    }
} 
/// @cond
ALIGN(8)
/// @endcond
;

String Serialize(const I2CMasterOperation& value);

#if BAREMETAL_RPI_TARGET == 3
/// @brief Number of I2C buses for RPI 3
#define I2C_BUSES 2
#else
/// @brief Number of I2C buses for RPI 4
#define I2C_BUSES 7
#endif

/// @brief FIFO size (both read and write)
#define I2C_FIFO_SIZE 16

/// @brief Callback for sending address
using SendAddressByteCallback = bool(I2CMasterRegisters& registers, uint8 address);
/// @brief Callback for receiving data
using RecvDataByteCallback = bool(I2CMasterRegisters& registers, uint8& data);
/// @brief Callback for sending data
using SendDataByteCallback = bool(I2CMasterRegisters& registers, uint8 data);

/// <summary>
/// Memory access mock class
/// </summary>
class MemoryAccessI2CMasterMock : public MemoryAccessGPIOMock
{
private:
    /// @brief Saved GPIO register values
    I2CMasterRegisters m_registers[I2C_BUSES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Receive FIFO
    FIFO<I2C_FIFO_SIZE> m_rxFifo[I2C_BUSES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Send FIFO
    FIFO<I2C_FIFO_SIZE> m_txFifo[I2C_BUSES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Pointer to send address callback
    SendAddressByteCallback* m_sendAddressByteCallback;
    /// @brief Pointer to receive data callback
    RecvDataByteCallback* m_recvDataByteCallback;
    /// @brief Pointer to send data callback
    SendDataByteCallback* m_sendDataByteCallback;
    /// @brief Number of data bytes received
    uint8 m_numBytesReceived;
    /// @brief Number of data bytes sent
    uint8 m_numBytesSent;
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    I2CMasterOperation m_ops[BufferSize]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    MemoryAccessI2CMasterMock();

    size_t GetNumI2CMasterOperations() const;
    const I2CMasterOperation& GetI2CMasterOperation(size_t index) const;

    uint32 OnRead(regaddr address) override;
    void OnWrite(regaddr address, uint32 data) override;

    void SetSendAddressByteCallback(SendAddressByteCallback callback);
    void SetRecvDataByteCallback(RecvDataByteCallback callback);
    void SetSendDataByteCallback(SendDataByteCallback callback);

private:
    bool InRangeForI2CMaster(regaddr address, uint8& bus);
    uint32 GetRegisterOffset(regaddr address, regaddr baseAddress);
    void AddOperation(const I2CMasterOperation& operation);
    void HandleWriteControlRegister(uint8 bus, uint32 data);
    void HandleWriteStatusRegister(uint8 bus, uint32 data);
    void HandleWriteFIFORegister(uint8 bus, uint8 data);
    uint8 HandleReadFIFORegister(uint8 bus);
    void HandleSendData(uint8 bus);
    void HandleRecvData(uint8 bus);
    void UpdateFIFOStatus(uint8 bus);
    void CancelTransfer(uint8 bus);
    void EndTransfer(uint8 bus);
};

} // namespace baremetal
