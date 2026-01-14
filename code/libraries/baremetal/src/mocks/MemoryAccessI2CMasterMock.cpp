//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessI2CMasterMock.cpp
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

#include "baremetal/mocks/MemoryAccessI2CMasterMock.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"

/// @file
/// Memory access mock class implementation

/// @brief Define log name
LOG_MODULE("MemoryAccessI2CMasterMock");

using namespace baremetal;

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(I2CMasterOperationCode code)
{
    String result{};
    switch (code)
    {
    case I2CMasterOperationCode::EnableController:
        result = "EnableController";
        break;
    case I2CMasterOperationCode::EnableRXInterrupt:
        result = "EnableRXInterrupt";
        break;
    case I2CMasterOperationCode::EnableTXInterrupt:
        result = "EnableTXInterrupt";
        break;
    case I2CMasterOperationCode::EnableDoneInterrupt:
        result = "EnableDoneInterrupt";
        break;
    case I2CMasterOperationCode::SetReadMode:
        result = "SetReadMode";
        break;
    case I2CMasterOperationCode::SetWriteMode:
        result = "SetWriteMode";
        break;
    case I2CMasterOperationCode::ClearFIFO:
        result = "ClearFIFO";
        break;
    case I2CMasterOperationCode::StartTransfer:
        result = "StartTransfer";
        break;
    case I2CMasterOperationCode::ResetClockStretchTimeout:
        result = "ResetClockStretchTimeout";
        break;
    case I2CMasterOperationCode::ResetAckError:
        result = "ResetAckError";
        break;
    case I2CMasterOperationCode::ResetDone:
        result = "ResetDone";
        break;
    case I2CMasterOperationCode::SetDataLength:
        result = "SetDataLength";
        break;
    case I2CMasterOperationCode::SetAddress:
        result = "SetAddress";
        break;
    case I2CMasterOperationCode::WriteFIFO:
        result = "WriteFIFO";
        break;
    case I2CMasterOperationCode::SetClockDivider:
        result = "SetClockDivider";
        break;
    case I2CMasterOperationCode::SetFallingEdgeDelay:
        result = "SetFallingEdgeDelay";
        break;
    case I2CMasterOperationCode::SetRisingEdgeDelay:
        result = "SetRisingEdgeDelay";
        break;
    case I2CMasterOperationCode::SetClockStretchTimeout:
        result = "SetClockStretchTimeout";
        break;
    case I2CMasterOperationCode::ReadFIFO:
        result = "ReadFIFO";
        break;
    case I2CMasterOperationCode::SetAckError:
        result = "SetAckError";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a I2C master memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const I2CMasterOperation &value)
{
    String result = Format("Operation=%s, Bus=%s, ", OperationCodeToString(value.operation).c_str(), Format("%d", value.bus).c_str());
    switch (value.operation)
    {
    case I2CMasterOperationCode::EnableController:
    case I2CMasterOperationCode::EnableRXInterrupt:
    case I2CMasterOperationCode::EnableTXInterrupt:
    case I2CMasterOperationCode::EnableDoneInterrupt:
        result += Format("Enable=%s", (value.argument != 0 ? "ON" : "OFF"));
        break;
    case I2CMasterOperationCode::SetReadMode:
    case I2CMasterOperationCode::SetWriteMode:
    case I2CMasterOperationCode::ClearFIFO:
    case I2CMasterOperationCode::StartTransfer:
    case I2CMasterOperationCode::ResetClockStretchTimeout:
    case I2CMasterOperationCode::ResetAckError:
    case I2CMasterOperationCode::ResetDone:
        break;
    case I2CMasterOperationCode::SetDataLength:
        result += Format("Length=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetAddress:
        result += Format("Address=%d", value.argument);
        break;
    case I2CMasterOperationCode::WriteFIFO:
        result += Format("Data=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetClockDivider:
        result += Format("ClockDivider=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetFallingEdgeDelay:
        result += Format("FallingEdgeDelay=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetRisingEdgeDelay:
        result += Format("RisingEdgeDelay=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetClockStretchTimeout:
        result += Format("ClockStretchTimeout=%d", value.argument);
        break;
    case I2CMasterOperationCode::ReadFIFO:
        result += Format("Data=%d", value.argument);
        break;
    case I2CMasterOperationCode::SetAckError:
        break;
    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

/// @brief GPIO base address
static regaddr I2CMasterBaseAddress[I2C_BUSES] = {
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
/// @brief Mask used to check whether an address is in the GPIO register range
static uintptr I2CMasterBaseAddressMask{0xFFFFFFFFFFFFFFE0};

/// <summary>
/// MemoryAccessI2CMasterMock constructor
/// </summary>
MemoryAccessI2CMasterMock::MemoryAccessI2CMasterMock()
    : m_registers{}
    , m_ops{}
    , m_numOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessI2CMasterMock::GetNumI2CMasterOperations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const I2CMasterOperation &MemoryAccessI2CMasterMock::GetI2CMasterOperation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessI2CMasterMock::OnRead(regaddr address)
{
    uint8 bus = 0xFF;
    if (!InRangeForI2CMaster(address, bus))
        return MemoryAccessGPIOMock::OnRead(address);

    uintptr offset = GetRegisterOffset(address, I2CMasterBaseAddress[bus]);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[bus]) + offset);
    TRACE_DEBUG("I2C Read register %016x = %08x", offset, *registerField);
    switch (offset)
    {
    case RPI_I2C_C_OFFSET:
        {
            String line{"I2C Read Control Register "};
            if (*registerField & RPI_I2C_C_ENABLE)
                line += "Enable ON  ";
            else
                line += "Enable OFF ";
            if (*registerField & RPI_I2C_C_INTR_ENABLE)
                line += "RX Interrupt ON  ";
            else
                line += "RX Interrupt OFF ";
            if (*registerField & RPI_I2C_C_INTT_ENABLE)
                line += "TX Interrupt ON  ";
            else
                line += "TX Interrupt OFF ";
            if (*registerField & RPI_I2C_C_INTD_ENABLE)
                line += "DONE Interrupt ON  ";
            else
                line += "DONE Interrupt OFF ";
            if (*registerField & RPI_I2C_C_READ)
                line += "Read ";
            else
                line += "Write ";
            TRACE_DEBUG(line.c_str());
            break;
        }
    case RPI_I2C_S_OFFSET:
        {
            String line{"I2C Read Status Register "};
            if (*registerField & RPI_I2C_S_CLKT)
                line += "CLKT ";
            else
                line += "     ";
            if (*registerField & RPI_I2C_S_ERR)
                line += "NACK ";
            else
                line += "ACK  ";
            if (*registerField & RPI_I2C_S_RXF)
                line += "RXF ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_TXE)
                line += "TXE ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_RXD)
                line += "RXD ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_TXD)
                line += "TXD ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_RXR)
                line += "RXR ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_TXW)
                line += "TXW ";
            else
                line += "    ";
            if (*registerField & RPI_I2C_S_DONE)
                line += "DONE ";
            else
                line += "     ";
            if (*registerField & RPI_I2C_S_TA)
                line += "TA ";
            else
                line += "   ";
            TRACE_DEBUG(line.c_str());
            break;
        }
    case RPI_I2C_DLEN_OFFSET:
        {
            TRACE_DEBUG("I2C Get Data Length %d", *registerField);
            break;
        }
    case RPI_I2C_A_OFFSET:
        {
            TRACE_DEBUG("I2C Get Address %02x", *registerField);
            break;
        }
    case RPI_I2C_FIFO_OFFSET:
        {
            TRACE_DEBUG("I2C Read FIFO %02x", *registerField);
            AddOperation({I2CMasterOperationCode::ReadFIFO, bus, *registerField});
            *registerField = HandleReadFIFORegister(bus);
            break;
        }
    case RPI_I2C_DIV_OFFSET:
        {
            TRACE_DEBUG("I2C Set Clock Divider %d", *registerField);
            break;
        }
    case RPI_I2C_DEL_OFFSET:
        {
            if (*registerField & 0xFFFF0000)
            {
                TRACE_DEBUG("I2C Get Falling Edge Delay %d", (*registerField >> 16));
            }
            if (*registerField & 0x0000FFFF)
            {
                TRACE_DEBUG("I2C Get Rising Edge Delay %d", (*registerField & 0x0000FFFF));
            }
            break;
        }
    case RPI_I2C_CLKT_OFFSET:
        {
            TRACE_DEBUG("I2C Get Clock Stretch Timeout %d", (*registerField & 0x0000FFFF));
            break;
        }
    default:
        LOG_ERROR("Invalid I2C register access for reading: offset %d", offset);
        break;
    }
    return *registerField;
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessI2CMasterMock::OnWrite(regaddr address, uint32 data)
{
    uint8 bus = 0xFF;
    if (!InRangeForI2CMaster(address, bus))
    {
        MemoryAccessGPIOMock::OnWrite(address, data);
        return;
    }

    uintptr offset = GetRegisterOffset(address, I2CMasterBaseAddress[bus]);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[bus]) + offset);
    TRACE_DEBUG("I2C Write register %016x = %08x", offset, data);
    switch (offset)
    {
    case RPI_I2C_C_OFFSET:
        {
            uint32 diff = (data ^ *registerField) | 0x00B0;
            if (diff & RPI_I2C_C_ENABLE)
            {
                AddOperation({I2CMasterOperationCode::EnableController, bus, (data & RPI_I2C_C_ENABLE) ? 1u : 0u});
            }
            if (diff & RPI_I2C_C_INTR_ENABLE)
            {
                AddOperation({I2CMasterOperationCode::EnableRXInterrupt, bus, (data & RPI_I2C_C_INTR_ENABLE) ? 1u : 0u});
            }
            if (diff & RPI_I2C_C_INTT_ENABLE)
            {
                AddOperation({I2CMasterOperationCode::EnableTXInterrupt, bus, (data & RPI_I2C_C_INTT_ENABLE) ? 1u : 0u});
            }
            if (diff & RPI_I2C_C_INTD_ENABLE)
            {
                AddOperation({I2CMasterOperationCode::EnableDoneInterrupt, bus, (data & RPI_I2C_C_INTD_ENABLE) ? 1u : 0u});
            }
            if (diff & RPI_I2C_C_READ)
            {
                if (data & RPI_I2C_C_READ)
                    AddOperation({I2CMasterOperationCode::SetReadMode, bus});
                else
                    AddOperation({I2CMasterOperationCode::SetWriteMode, bus});
            }
            if (data & RPI_I2C_C_CLEAR)
                AddOperation({I2CMasterOperationCode::ClearFIFO, bus});
            if (data & RPI_I2C_C_ST)
                AddOperation({I2CMasterOperationCode::StartTransfer, bus});
            HandleWriteControlRegister(bus, data);
            break;
        }
    case RPI_I2C_S_OFFSET:
        {
            if (data & RPI_I2C_S_CLKT)
                AddOperation({I2CMasterOperationCode::ResetClockStretchTimeout, bus});
            if (data & RPI_I2C_S_ERR)
                AddOperation({I2CMasterOperationCode::ResetAckError, bus});
            if (data & RPI_I2C_S_DONE)
                AddOperation({I2CMasterOperationCode::ResetDone, bus});
            HandleWriteStatusRegister(bus, data);
            break;
        }
    case RPI_I2C_DLEN_OFFSET:
        {
            AddOperation({I2CMasterOperationCode::SetDataLength, bus, data});
            *registerField = data;
            break;
        }
    case RPI_I2C_A_OFFSET:
        {
            AddOperation({I2CMasterOperationCode::SetAddress, bus, data});
            *registerField = data;
            break;
        }
    case RPI_I2C_FIFO_OFFSET:
        {
            AddOperation({I2CMasterOperationCode::WriteFIFO, bus, data});
            HandleWriteFIFORegister(bus, data);
            break;
        }
    case RPI_I2C_DIV_OFFSET:
        {
            if (data != *registerField)
            {
                AddOperation({I2CMasterOperationCode::SetClockDivider, bus, data});
            }
            *registerField = data;
            break;
        }
    case RPI_I2C_DEL_OFFSET:
        {
            uint32 diff = data & *registerField;
            if (diff & 0xFFFF0000)
            {
                AddOperation({I2CMasterOperationCode::SetFallingEdgeDelay, bus, (data >> 16)});
            }
            if (diff & 0x0000FFFF)
            {
                AddOperation({I2CMasterOperationCode::SetRisingEdgeDelay, bus, (data & 0xFFFF)});
            }
            *registerField = data;
            break;
        }
    case RPI_I2C_CLKT_OFFSET:
        {
            uint32 diff = data & *registerField;
            if (diff & 0x0000FFFF)
            {
                AddOperation({I2CMasterOperationCode::SetClockStretchTimeout, bus, (data & 0xFFFF)});
            }
            *registerField = data;
            break;
        }
    default:
        LOG_ERROR("Invalid I2C register access for writing: offset %d", offset);
        break;
    }
}

/// <summary>
/// Set callback function for address send
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessI2CMasterMock::SetSendAddressByteCallback(SendAddressByteCallback callback)
{
    m_sendAddressByteCallback = callback;
}

/// <summary>
/// Set callback function for data receive
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessI2CMasterMock::SetRecvDataByteCallback(RecvDataByteCallback callback)
{
    m_recvDataByteCallback = callback;
}

/// <summary>
/// Set callback function for data send
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessI2CMasterMock::SetSendDataByteCallback(SendDataByteCallback callback)
{
    m_sendDataByteCallback = callback;
}

/// <summary>
/// Determined if address is in range for I2C master registers for any bus, return the bus index if so
/// </summary>
/// <param name="address">Address to check</param>
/// <param name="bus">I2C bus if address is in range for this bus</param>
/// <returns>True if address is in range for registers of a I2C bus master, false otherwise</returns>
bool MemoryAccessI2CMasterMock::InRangeForI2CMaster(regaddr address, uint8 &bus)
{
    for (uint8 i = 0; i < I2C_BUSES; ++i)
    {
        uintptr baseAddr = reinterpret_cast<uintptr>(I2CMasterBaseAddress[i]);
        if ((reinterpret_cast<uintptr>(address) & I2CMasterBaseAddressMask) == baseAddr)
        {
            bus = i;
            return true;
        }
    }
    return false;
}

/// <summary>
/// Determine register address offset relative to GPIO base address
///
/// If the address is not in the correct range, an assert is fired
/// </summary>
/// <param name="address">Address to check</param>
/// <param name="baseAddress">Base address for I2C bus master</param>
/// <returns>Offset relative to I2C master base address</returns>
uint32 MemoryAccessI2CMasterMock::GetRegisterOffset(regaddr address, regaddr baseAddress)
{
    return reinterpret_cast<uintptr>(address) - reinterpret_cast<uintptr>(baseAddress);
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessI2CMasterMock::AddOperation(const I2CMasterOperation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}

/// <summary>
/// Handle writing to I2C Control Register
/// </summary>
/// <param name="bus">I2C bus</param>
/// <param name="data">Value to write to the register</param>
void MemoryAccessI2CMasterMock::HandleWriteControlRegister(uint8 bus, uint32 data)
{
    if ((data & RPI_I2C_C_CLEAR) != 0)
    {
        m_rxFifo[bus].Flush();
        m_txFifo[bus].Flush();
        m_numBytesReceived = 0;
        m_numBytesSent = 0;
    }
    m_registers[bus].Control = data & ~(RPI_I2C_C_CLEAR | RPI_I2C_C_ST);
    if (((data & RPI_I2C_C_ENABLE) != 0) && ((data & RPI_I2C_C_ST) != 0))
    {
        // We started a (new) transaction
        m_registers[bus].Status = m_registers[bus].Status | RPI_I2C_S_TA;
        if (m_sendAddressByteCallback)
        {
            if (!(*m_sendAddressByteCallback)(m_registers[bus], m_registers[bus].Address))
            {
                m_registers[bus].Status |= RPI_I2C_S_ERR;
                AddOperation({I2CMasterOperationCode::SetAckError, bus});
            }
        }
        if ((m_registers[bus].Control & RPI_I2C_C_READ) != 0)
        {
            HandleRecvData(bus);
        }
        else
        {
            HandleSendData(bus);
        }
    }
}

/// <summary>
/// Handle writing to I2C Status Register
/// </summary>
/// <param name="bus">I2C bus</param>
/// <param name="data">Value to write to the register</param>
void MemoryAccessI2CMasterMock::HandleWriteStatusRegister(uint8 bus, uint32 data)
{
    if (data & RPI_I2C_S_ERR)
    {
        m_registers[bus].Status = m_registers[bus].Status & ~RPI_I2C_S_ERR;
    }
    if (data & RPI_I2C_S_DONE)
    {
        m_registers[bus].Status = m_registers[bus].Status & ~(RPI_I2C_S_DONE | RPI_I2C_S_TA);
    }
    if (data & RPI_I2C_S_CLKT)
    {
        m_registers[bus].Status = m_registers[bus].Status & ~RPI_I2C_S_CLKT;
    }
}

/// <summary>
/// Handle writing to I2C Transmit FIFO Register
/// </summary>
/// <param name="bus">I2C bus</param>
/// <param name="data">Value to write to the register</param>
void MemoryAccessI2CMasterMock::HandleWriteFIFORegister(uint8 bus, uint8 data)
{
    if (!m_txFifo[bus].IsFull())
    {
        m_txFifo[bus].Write(data);
    }
    UpdateFIFOStatus(bus);
    HandleSendData(bus);
}

/// <summary>
/// Handle reading to I2C Receive FIFO Register
/// </summary>
/// <param name="bus">I2C bus</param>
/// <returns>Value read from the FIFO</returns>
uint8 MemoryAccessI2CMasterMock::HandleReadFIFORegister(uint8 bus)
{
    uint8 result{};
    HandleRecvData(bus);
    if (!m_rxFifo[bus].IsEmpty())
    {
        result = m_rxFifo[bus].Read();
    }
    UpdateFIFOStatus(bus);
    return result;
}

/// <summary>
/// Handle transmitting data
/// </summary>
/// <param name="bus">I2C bus</param>
void MemoryAccessI2CMasterMock::HandleSendData(uint8 bus)
{
    if (((m_registers[bus].Status & RPI_I2C_S_TA) != 0) && ((m_registers[bus].Control & RPI_I2C_C_READ) == 0))
    {
        while (!m_txFifo[bus].IsEmpty() && (m_numBytesSent < m_registers[bus].DataLength))
        {
            auto data = m_txFifo[bus].Read();
            UpdateFIFOStatus(bus);
            bool sentData{};
            if (m_sendDataByteCallback)
                sentData = (*m_sendDataByteCallback)(m_registers[bus], data);
            if (sentData)
                ++m_numBytesSent;
            else
            {
                CancelTransfer(bus);
                break;
            }
        }
    }
    if (m_numBytesSent >= m_registers[bus].DataLength)
        EndTransfer(bus);
}

/// <summary>
/// Handle receiving data
/// </summary>
/// <param name="bus">I2C bus</param>
void MemoryAccessI2CMasterMock::HandleRecvData(uint8 bus)
{
    if (((m_registers[bus].Control & RPI_I2C_C_ENABLE) != 0) && ((m_registers[bus].Control & RPI_I2C_C_READ) != 0))
    {
        while (!m_rxFifo[bus].IsFull() && (m_numBytesReceived < m_registers[bus].DataLength))
        {
            uint8 data{};
            bool receivedData{};
            if (m_recvDataByteCallback)
            {
                receivedData = (*m_recvDataByteCallback)(m_registers[bus], data);
            }
            if (receivedData)
            {
                m_rxFifo[bus].Write(data);
                UpdateFIFOStatus(bus);
                ++m_numBytesReceived;
            }
            else
            {
                CancelTransfer(bus);
                break;
            }
        }
    }
    if (m_numBytesReceived >= m_registers[bus].DataLength)
        EndTransfer(bus);
}

/// <summary>
/// Update the FIFO status bits in the I2C Status Register
/// </summary>
/// <param name="bus">I2C bus</param>
void MemoryAccessI2CMasterMock::UpdateFIFOStatus(uint8 bus)
{
    auto& txFifo = m_txFifo[bus];
    auto& rxFifo = m_rxFifo[bus];
    if (txFifo.IsEmpty())
        m_registers[bus].Status |= RPI_I2C_S_TXE;
    else
        m_registers[bus].Status &= (~RPI_I2C_S_TXE);
    if (txFifo.IsFull())
        m_registers[bus].Status |= RPI_I2C_S_TXD;
    else
        m_registers[bus].Status &= (~RPI_I2C_S_TXD);
    if (txFifo.IsOneQuarterOrLessFull())
        m_registers[bus].Status |= RPI_I2C_S_TXW;
    else
        m_registers[bus].Status &= (~RPI_I2C_S_TXW);
    if (rxFifo.IsEmpty())
        m_registers[bus].Status &= (~RPI_I2C_S_RXD);
    else
        m_registers[bus].Status |= RPI_I2C_S_RXD;
    if (rxFifo.IsFull())
        m_registers[bus].Status |= RPI_I2C_S_RXF;
    else
        m_registers[bus].Status &= (~RPI_I2C_S_RXF);
    if (rxFifo.IsThreeQuartersOrMoreFull())
        m_registers[bus].Status |= RPI_I2C_S_RXR;
    else
        m_registers[bus].Status &= (~RPI_I2C_S_RXR);
}

/// <summary>
/// Cancel a transfer, i.e. set error flag to denote NACK, and set done flag
/// </summary>
/// <param name="bus">I2C bus</param>
void MemoryAccessI2CMasterMock::CancelTransfer(uint8 bus)
{
    m_registers[bus].Status |= (RPI_I2C_S_ERR | RPI_I2C_S_DONE);
    m_registers[bus].Status &= (~RPI_I2C_S_TA);
}

/// <summary>
/// Finalize a transfer, i.e. set done flag
/// </summary>
/// <param name="bus">I2C bus</param>
void MemoryAccessI2CMasterMock::EndTransfer(uint8 bus)
{
    m_registers[bus].Status |= (RPI_I2C_S_DONE);
    m_registers[bus].Status &= (~RPI_I2C_S_TA);
}
