//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubI2C.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessStubI2C
//
// Description : I2C register memory access stub
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

#include "baremetal/stubs/MemoryAccessStubGPIO.h"
#include "stdlib/Macros.h"

/// @file
/// MemoryAccessStubI2C

namespace baremetal {

/// @brief I2C registers storage
struct I2CRegisters
{
    /// @brief I2C Control Register (C)
    uint32 ControlRegister;
    /// @brief I2C Status Register (C)
    uint32 StatusRegister;
    /// @brief I2C Data Length Register (DLEN)
    uint32 DataLengthRegister;
    /// @brief I2C Address Register (A)
    uint32 AddressRegister;
    /// @brief I2C FIFO Register (FIFO)
    uint32 FIFORegister;
    /// @brief I2C ClockDivider Register (DIV)
    uint32 ClockDividerRegister;
    /// @brief I2C Data Delay Register (DEL)
    uint32 DataDelayRegister;
    /// @brief I2C Clock Stretch Timeout Register (CLKT)
    uint32 ClockStretchTimeoutRegister;

    /// <summary>
    /// Constructor for I2CRegisters
    ///
    /// Sets default register values
    /// </summary>
    I2CRegisters()
        : ControlRegister{}
        , StatusRegister{0x00000050}
        , DataLengthRegister{}
        , AddressRegister{}
        , FIFORegister{}
        , ClockDividerRegister{0x000005DC}
        , DataDelayRegister{0x00300030}
        , ClockStretchTimeoutRegister{0x00000040}
    {
    }
} PACKED;

/// @brief FIFO size (both read and write)
#define I2C_FIFO_SIZE 16

/// @brief FIFO template class
template <int N>
class FIFO
{
private:
    /// @brief Data in FIFO (max 16 bytes)
    uint8 m_data[N]; // Size 16
    /// @brief Read index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
    int m_readIndex; // Size 4
    /// @brief Write index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
    int m_writeIndex; // Size 4
    /// @brief Flag whether the FIFO is full
    bool m_isFull; // Size 1
    /// @brief Force alignment to 16 bytes
    uint8 align[7]; // Size 7

public:
    /// <summary>
    /// FIFO constructor
    /// </summary>
    FIFO()
        : m_data{}
        , m_readIndex{}
        , m_writeIndex{}
        , m_isFull{}
    {
    }
    /// <summary>
    /// Read a byte from the FIFO
    /// </summary>
    /// <returns>Byte read. If nothing can be read, returns 0</returns>
    uint8 Read()
    {
        uint8 result{};
        if (!IsEmpty())
        {
            result = m_data[m_readIndex];
            m_readIndex = (m_readIndex + 1) % N;
            m_isFull = false;
        }
        return result;
    }
    /// <summary>
    /// Write a byte to the FIFO
    /// </summary>
    /// <param name="data">Data to write</param>
    void Write(uint8 data)
    {
        if (!IsFull())
        {
            m_data[m_writeIndex] = data;
            m_writeIndex = (m_writeIndex + 1) % N;
            if (m_readIndex == m_writeIndex)
                m_isFull = true;
        }
    }
    /// <summary>
    /// Check if FIFO is empty
    /// </summary>
    /// <returns>true if FIFO is empty, false otherwise</returns>
    bool IsEmpty()
    {
        return (m_readIndex == m_writeIndex) && !m_isFull;
    }
    /// <summary>
    /// Check if FIFO is full
    /// </summary>
    /// <returns>true if FIFO is full, false otherwise</returns>
    bool IsFull()
    {
        return (m_readIndex == m_writeIndex) && m_isFull;
    }
    /// <summary>
    /// Check if FIFO is at most 25% full
    /// </summary>
    /// <returns>true if FIFO is at most 25% full, false otherwise</returns>
    bool IsOneQuarterOrLessFull()
    {
        return ((m_writeIndex - m_readIndex + I2C_FIFO_SIZE) % I2C_FIFO_SIZE) <= (I2C_FIFO_SIZE / 4);
    }
    /// <summary>
    /// Check if FIFO is at least 75% full (at most 25% empty)
    /// </summary>
    /// <returns>true if FIFO is at least 75% full, false otherwise</returns>
    bool IsThreeQuartersOrMoreFull()
    {
        return ((m_readIndex - m_writeIndex + I2C_FIFO_SIZE) % I2C_FIFO_SIZE) <= (I2C_FIFO_SIZE / 4);
    }
    /// <summary>
    /// Flush the FIFO
    /// </summary>
    void Flush()
    {
        m_readIndex = m_writeIndex = 0;
        m_isFull = false;
    }
};

/// @brief Callback for sending address
using SendAddressByteCallback = bool(I2CRegisters& registers, uint8 address);
/// @brief Callback for receiving data
using RecvDataByteCallback = bool(I2CRegisters& registers, uint8& data);
/// @brief Callback for sending data
using SendDataByteCallback = bool(I2CRegisters& registers, uint8 data);

/// @brief MemoryAccess implementation for I2C stub
class MemoryAccessStubI2C : public MemoryAccessStubGPIO
{
private:
    /// @brief Storage for I2C registers
    I2CRegisters m_registers;
    /// @brief I2C master base address
    uintptr m_i2cMasterBaseAddress;
    /// @brief Receive FIFO
    FIFO<I2C_FIFO_SIZE> m_rxFifo;
    /// @brief Send FIFO
    FIFO<I2C_FIFO_SIZE> m_txFifo;
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

public:
    MemoryAccessStubI2C();
    void SetBus(uint8 bus);

    uint8 Read8(regaddr address) override;
    void Write8(regaddr address, uint8 data) override;

    uint16 Read16(regaddr address) override;
    void Write16(regaddr address, uint16 data) override;

    uint32 Read32(regaddr address) override;
    void Write32(regaddr address, uint32 data) override;

    void SetSendAddressByteCallback(SendAddressByteCallback callback);
    void SetRecvDataByteCallback(RecvDataByteCallback callback);
    void SetSendDataByteCallback(SendDataByteCallback callback);

private:
    uint32 GetRegisterOffset(regaddr address);
    void HandleWriteControlRegister(uint32 data);
    void HandleWriteStatusRegister(uint32 data);
    void HandleWriteFIFORegister(uint8 data);
    uint8 HandleReadFIFORegister();
    void HandleSendData();
    void HandleRecvData();
    void UpdateFIFOStatus();
    void CancelTransfer();
    void EndTransfer();
};

} // namespace baremetal
