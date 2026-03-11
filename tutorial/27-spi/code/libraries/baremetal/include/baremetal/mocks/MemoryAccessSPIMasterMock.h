//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessSPIMasterMock.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessSPIMasterMock
//
// Description : SPI master memory access mock class
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

#include "baremetal/SPIMaster.h"
#include "baremetal/String.h"
#include "stdlib/Macros.h"

/// @file
/// Memory access mock class

namespace baremetal {

/// @brief SPI master registers storage
struct SPIMasterRegisters
{
    /// @brief RPI_SPI_CS value
    uint32 CS; // 0x00
    /// @brief RPI_SPI_FIFO value
    uint32 FIFO; // 0x04
    /// @brief RPI_SPI_CLK value
    uint32 CLK; // 0x08
    /// @brief RPI_SPI_DLEN value
    uint32 DLEN; // 0x0C
    /// @brief RPI_SPI_LTOH value
    uint32 LTOH; // 0x10
    /// @brief RPI_SPI_DC value
    uint32 DC; // 0x14

    /// <summary>
    /// Constructor for SPIMasterRegisters
    ///
    /// Sets default register values
    /// </summary>
    SPIMasterRegisters()
        : CS{0x00041000}
        , FIFO{}
        , CLK{}
        , DLEN{}
        , LTOH{0x00000001}
        , DC{0x30201020}
    {
    }
} PACKED;

/// @brief SPI master operation codes
enum SPIMasterOperationCode
{
    /// @brief Enable long data word in LOSSI mode
    EnableLOSSILongDataWord,
    /// @brief Enable DMA in LOSSI mode
    EnableLOSSI_DMA,
    /// @brief Set CS / CE polarity for CS0 pin
    SetCSPol0,
    /// @brief Set CS / CE polarity for CS1 pin
    SetCSPol1,
    /// @brief Set CS / CE polarity for CS2 pin
    SetCSPol2,
    /// @brief Enable LOSSI mode
    EnableLOSSI,
    /// @brief Enable read
    EnableRead,
    /// @brief Set ADCS (Automatic De-assert Chip Select
    SetADCS,
    /// @brief Enable RX interrupt
    EnableRXInterrupt,
    /// @brief Enable Done interrupt
    EnableDoneInterrupt,
    /// @brief Enable DMA
    EnableDMA,
    /// @brief Start transfer
    StartTransfer,
    /// @brief End transfer
    EndTransfer,
    /// @brief Set CS / CE polarity for all pins
    SetCSPol,
    /// @brief Clear receive FIFO
    ClearRxFIFO,
    /// @brief Clear transmit FIFO
    ClearTxFIFO,
    /// @brief Set clock polarity
    SetClockPolarity,
    /// @brief Set clock phase
    SetClockPhase,
    /// @brief Activate CS0
    ActivateCS0,
    /// @brief Activate CS1
    ActivateCS1,
    /// @brief Activate CS2
    ActivateCS2,
    /// @brief Activate no CS
    ActivateNone,
    /// @brief Set data length
    SetDataLength,
    /// @brief Write to FIFO
    WriteFIFO,
    /// @brief Set clock divider
    SetClockDivider,
    /// @brief Set LOSSI output hold delay
    SetLOSSIOutputHoldDelay,
    /// @brief Set DMA read panic threshold
    SetDMAReadPanicThreshold,
    /// @brief Set DMA read request threshold
    SetDMAReadRequestThreshold,
    /// @brief Set DMA write panic threshold
    SetDMAWritePanicThreshold,
    /// @brief Set DMA write request threshold
    SetDMAWriteRequestThreshold,
    /// @brief Read from FIFO
    ReadFIFO,
};

/// <summary>
/// Data structure to contain a memory access operation
/// </summary>
struct SPIMasterOperation
{
    /// @brief SPI master operation code
    SPIMasterOperationCode operation; // Size: 4 bytes
    /// @brief SPI device
    uint8 device; // Size: 1 bytes
    /// @brief Argument (if any)
    uint32 argument; // Size: 4 bytes

    /// <summary>
    /// Default constructor
    /// </summary>
    SPIMasterOperation()
        : operation{}
        , device{}
        , argument{}
    {
    }
     /// <summary>
    /// Constructor for read or 2write operation concerning pin function
    /// </summary>
    /// <param name="theOperation">Operation code</param>
    /// <param name="theDevice">SPI device index</param>
    /// <param name="theArgument">Argument value</param>
    SPIMasterOperation(SPIMasterOperationCode theOperation, uint8 theDevice, uint32 theArgument = 0)
        : operation{theOperation}
        , device{theDevice}
        , argument{theArgument}
    {
    }
    /// <summary>
    /// Check memory access operations for equality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if equal, false otherwise</returns>
    bool operator==(const SPIMasterOperation& other) const
    {
        return (other.operation == operation) &&
            (other.device == device) &&
            (other.argument == argument);
    }
    /// <summary>
    /// Check memory access operations for inequality
    /// </summary>
    /// <param name="other">Value to compare to</param>
    /// <returns>True if unequal, false otherwise</returns>
    bool operator!=(const SPIMasterOperation& other) const
    {
        return !operator==(other);
    }
} 
/// @cond
ALIGN(8)
/// @endcond
;

String Serialize(const SPIMasterOperation& value);

#if BAREMETAL_RPI_TARGET == 3
/// @brief Number of SPI buses for RPI 3
#define SPI_DEVICES 1
#else
/// @brief Number of SPI buses for RPI 4
#define SPI_DEVICES 7
#endif

/// @brief FIFO size (both read and write)
#define SPI_FIFO_SIZE 16

/// @brief Callback for sending/receiving data
using SendRecvDataByteCallback = void(SPIMasterRegisters& registers, uint8 dataOut, uint8& dataIn);

/// <summary>
/// Memory access mock class
/// </summary>
class MemoryAccessSPIMasterMock : public MemoryAccessGPIOMock
{
private:
    /// @brief Saved GPIO register values
    SPIMasterRegisters m_registers[SPI_DEVICES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Receive FIFO
    FIFO<SPI_FIFO_SIZE> m_rxFifo[SPI_DEVICES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Send FIFO
    FIFO<SPI_FIFO_SIZE> m_txFifo[SPI_DEVICES]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Pointer to send / receive data callback
    SendRecvDataByteCallback* m_sendRecvByteCallback;
    /// @brief Number of data bytes sent / received
    uint8 m_numBytesTransferred;
    /// @brief Size of memory access operation array
    static constexpr size_t BufferSize = 1000;
    /// List op memory access operations
    SPIMasterOperation m_ops[BufferSize]
    /// @cond
    ALIGN(8)
    /// @endcond
    ;
    /// @brief Number of registered memory access operations
    size_t m_numOps;

public:
    MemoryAccessSPIMasterMock();

    size_t GetNumSPIMasterOperations() const;
    const SPIMasterOperation& GetSPIMasterOperation(size_t index) const;

    uint32 OnRead(regaddr address) override;
    void OnWrite(regaddr address, uint32 data) override;

    void SetSendRecvDataByteCallback(SendRecvDataByteCallback callback);

private:
    bool InRangeForSPIMaster(regaddr address, uint8& device);
    uint32 GetRegisterOffset(regaddr address, regaddr baseAddress);
    void AddOperation(const SPIMasterOperation& operation);
    void HandleWriteCSRegister(uint8 device, uint32 data);
    void HandleWriteFIFORegister(uint8 device, uint8 data);
    uint8 HandleReadFIFORegister(uint8 device);
    void HandleSendReceiveData(uint8 device);
    void UpdateFIFOStatus(uint8 device);
};

} // namespace baremetal
