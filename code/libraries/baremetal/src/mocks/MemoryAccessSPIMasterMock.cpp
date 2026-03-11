//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessSPIMasterMock.cpp
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

#include "baremetal/mocks/MemoryAccessSPIMasterMock.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"

/// @file
/// Memory access mock class implementation

/// @brief Define log name
LOG_MODULE("MemoryAccessSPIMasterMock");

using namespace baremetal;

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(SPIMasterOperationCode code)
{
    String result{};
    switch (code)
    {
    case SPIMasterOperationCode::EnableLOSSILongDataWord:
        result = "EnableLOSSILongDataWord";
        break;
    case SPIMasterOperationCode::EnableLOSSI_DMA:
        result = "EnableLOSSI_DMA";
        break;
    case SPIMasterOperationCode::SetCSPol0:
        result = "SetCSPol0";
        break;
    case SPIMasterOperationCode::SetCSPol1:
        result = "SetCSPol1";
        break;
    case SPIMasterOperationCode::SetCSPol2:
        result = "SetCSPol2";
        break;
    case SPIMasterOperationCode::EnableLOSSI:
        result = "EnableLOSSI";
        break;
    case SPIMasterOperationCode::EnableRead:
        result = "EnableRead";
        break;
    case SPIMasterOperationCode::SetADCS:
        result = "SetADCS";
        break;
    case SPIMasterOperationCode::EnableRXInterrupt:
        result = "EnableRXInterrupt";
        break;
    case SPIMasterOperationCode::EnableDoneInterrupt:
        result = "EnableDoneInterrupt";
        break;
    case SPIMasterOperationCode::EnableDMA:
        result = "EnableDMA";
        break;
    case SPIMasterOperationCode::StartTransfer:
        result = "StartTransfer";
        break;
    case SPIMasterOperationCode::EndTransfer:
        result = "EndTransfer";
        break;
    case SPIMasterOperationCode::ClearRxFIFO:
        result = "ClearRxFIFO";
        break;
    case SPIMasterOperationCode::ClearTxFIFO:
        result = "ClearTxFIFO";
        break;
    case SPIMasterOperationCode::SetCSPol:
        result = "SetCSPol";
        break;
    case SPIMasterOperationCode::SetClockPolarity:
        result = "SetClockPolarity";
        break;
    case SPIMasterOperationCode::SetClockPhase:
        result = "SetClockPhase";
        break;
    case SPIMasterOperationCode::ActivateCS0:
        result = "ActivateCS0";
        break;
    case SPIMasterOperationCode::ActivateCS1:
        result = "ActivateCS1";
        break;
    case SPIMasterOperationCode::ActivateCS2:
        result = "ActivateCS2";
        break;
    case SPIMasterOperationCode::ActivateNone:
        result = "ActivateNone";
        break;
    case SPIMasterOperationCode::SetDataLength:
        result = "SetDataLength";
        break;
    case SPIMasterOperationCode::WriteFIFO:
        result = "WriteFIFO";
        break;
    case SPIMasterOperationCode::SetClockDivider:
        result = "SetClockDivider";
        break;
    case SPIMasterOperationCode::SetLOSSIOutputHoldDelay:
        result = "SetLOSSIOutputHoldDelay";
        break;
    case SPIMasterOperationCode::ReadFIFO:
        result = "ReadFIFO";
        break;
    case SPIMasterOperationCode::SetDMAReadPanicThreshold:
        result = "SetDMAReadPanicThreshold";
        break;
    case SPIMasterOperationCode::SetDMAReadRequestThreshold:
        result = "SetDMAReadRequestThreshold";
        break;
    case SPIMasterOperationCode::SetDMAWritePanicThreshold:
        result = "SetDMAWritePanicThreshold";
        break;
    case SPIMasterOperationCode::SetDMAWriteRequestThreshold:
        result = "SetDMAWritePanicThreshold";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a SPI master memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const SPIMasterOperation &value)
{
    String result = Format("Operation=%s, Bus=%s, ", OperationCodeToString(value.operation).c_str(), Format("%d", value.device).c_str());
    switch (value.operation)
    {
    case SPIMasterOperationCode::EnableLOSSILongDataWord:
    case SPIMasterOperationCode::EnableLOSSI_DMA:
    case SPIMasterOperationCode::EnableLOSSI:
    case SPIMasterOperationCode::EnableRead:
    case SPIMasterOperationCode::SetADCS:
    case SPIMasterOperationCode::EnableRXInterrupt:
    case SPIMasterOperationCode::EnableDoneInterrupt:
    case SPIMasterOperationCode::EnableDMA:
        result += Format("Enable=%s", (value.argument != 0 ? "ON" : "OFF"));
        break;
    case SPIMasterOperationCode::ClearRxFIFO:
    case SPIMasterOperationCode::ClearTxFIFO:
    case SPIMasterOperationCode::StartTransfer:
    case SPIMasterOperationCode::EndTransfer:
    case SPIMasterOperationCode::ActivateCS0:
    case SPIMasterOperationCode::ActivateCS1:
    case SPIMasterOperationCode::ActivateCS2:
    case SPIMasterOperationCode::ActivateNone:
        break;
    case SPIMasterOperationCode::SetDataLength:
        result += Format("Length=%d", value.argument);
        break;
    case SPIMasterOperationCode::WriteFIFO:
        result += Format("Data=%d", value.argument);
        break;
    case SPIMasterOperationCode::SetClockDivider:
        result += Format("ClockDivider=%d", value.argument);
        break;
    case SPIMasterOperationCode::ReadFIFO:
        result += Format("Data=%d", value.argument);
        break;
    case SPIMasterOperationCode::SetLOSSIOutputHoldDelay:
    case SPIMasterOperationCode::SetDMAReadPanicThreshold:
    case SPIMasterOperationCode::SetDMAReadRequestThreshold:
    case SPIMasterOperationCode::SetDMAWritePanicThreshold:
    case SPIMasterOperationCode::SetDMAWriteRequestThreshold:
        result += Format("Value=%d", value.argument);
        break;
    case SPIMasterOperationCode::SetCSPol0:
    case SPIMasterOperationCode::SetCSPol1:
    case SPIMasterOperationCode::SetCSPol2:
    case SPIMasterOperationCode::SetCSPol:
        result += Format("Polarity=%s", (value.argument != 0 ? "ACTIVE_HIGH" : "ACTIVE_LOW"));
        break;
    case SPIMasterOperationCode::SetClockPolarity:
        result += Format("Polarity=%s", (value.argument != 0 ? "IDLE_HIGH" : "IDLE_LOW"));
        break;
    case SPIMasterOperationCode::SetClockPhase:
        result += Format("Phase=%s", (value.argument != 0 ? "BEGIN" : "MIDDLE"));
        break;
    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

/// @brief GPIO base address
static regaddr SPIMasterBaseAddress[SPI_DEVICES] = {
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
/// @brief Mask used to check whether an address is in the SPI master register range
static uintptr SPIMasterBaseAddressMask{0xFFFFFFFFFFFFFFE0};

/// <summary>
/// MemoryAccessSPIMasterMock constructor
/// </summary>
MemoryAccessSPIMasterMock::MemoryAccessSPIMasterMock()
    : m_registers{}
    , m_ops{}
    , m_numOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessSPIMasterMock::GetNumSPIMasterOperations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const SPIMasterOperation &MemoryAccessSPIMasterMock::GetSPIMasterOperation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessSPIMasterMock::OnRead(regaddr address)
{
    uint8 device = 0xFF;
    if (!InRangeForSPIMaster(address, device))
        return MemoryAccessGPIOMock::OnRead(address);

    uintptr offset = GetRegisterOffset(address, SPIMasterBaseAddress[device]);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[device]) + offset);
    TRACE_DEBUG("SPI Read register %016x = %08x", offset, *registerField);
    switch (offset)
    {
    case RPI_SPI_CS_OFFSET:
        {
            String line{"SPI Read CS Register "};
            if (*registerField & RPI_SPI_CS_LEN_LONG)
                line += "DMA LOSSI 32 bit ";
            else
                line += "DMA LOSSI 8 bit  ";
            if (*registerField & RPI_SPI_CS_DMA_LEN)
                line += "DMA LOSSI ON  ";
            else
                line += "DMA LOSSI OFF ";
            if (*registerField & RPI_SPI_CS_POL2)
                line += "CS2 ACT HIGH ";
            else
                line += "CS2 ACT LOW  ";
            if (*registerField & RPI_SPI_CS_POL1)
                line += "CS1 ACT HIGH ";
            else
                line += "CS1 ACT LOW  ";
            if (*registerField & RPI_SPI_CS_POL0)
                line += "CS0 ACT HIGH ";
            else
                line += "CS0 ACT LOW  ";
            if (*registerField & RPI_SPI_CS_RXR)
                line += "RXR ";
            else
                line += "    ";
            if (*registerField & RPI_SPI_CS_TXD)
                line += "TXD ";
            else
                line += "    ";
            if (*registerField & RPI_SPI_CS_RXD)
                line += "RXD ";
            else
                line += "    ";
            if (*registerField & RPI_SPI_CS_DONE)
                line += "DONE ";
            else
                line += "     ";
            if (*registerField & RPI_SPI_CS_LEN)
                line += "LOSSI ";
            else
                line += "      ";
            if (*registerField & RPI_SPI_CS_REN)
                line += "READ ";
            else
                line += "     ";
            if (*registerField & RPI_SPI_CS_ADCS)
                line += "ADCS ON  ";
            else
                line += "ADCS OFF ";
            if (*registerField & RPI_SPI_CS_INTR)
                line += "INTR ";
            else
                line += "     ";
            if (*registerField & RPI_SPI_CS_INTD)
                line += "INTD ";
            else
                line += "     ";
            if (*registerField & RPI_SPI_CS_DMAEN)
                line += "DMA ";
            else
                line += "    ";
            if (*registerField & RPI_SPI_CS_TA)
                line += "TA ";
            else
                line += "   ";
            if (*registerField & RPI_SPI_CS_POL)
                line += "CS ACT HIGH ";
            else
                line += "CS ACT LOW  ";
            if (*registerField & RPI_SPI_CS_CLEAR_RX)
                line += "CLEAR RX ";
            else
                line += "         ";
            if (*registerField & RPI_SPI_CS_CLEAR_TX)
                line += "CLEAR TX ";
            else
                line += "         ";
            if (*registerField & RPI_SPI_CS_CPOL)
                line += "CLK HIGH ";
            else
                line += "CLK LOW  ";
            if (*registerField & RPI_SPI_CS_CPHA)
                line += "CLK BEG ";
            else
                line += "CLK MID ";
            if ((*registerField & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS0)
                line += "ACT CS0 ";
            else if ((*registerField & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS1)
                line += "ACT CS1 ";
            else if ((*registerField & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS2)
                line += "ACT CS2 ";
            else 
                line += "ACT NONE ";
            TRACE_DEBUG(line.c_str());
            break;
        }
    case RPI_SPI_FIFO_OFFSET:
        {
            TRACE_DEBUG("SPI Read FIFO %02x", *registerField);
            *registerField = HandleReadFIFORegister(device);
            AddOperation({SPIMasterOperationCode::ReadFIFO, device, *registerField});
            break;
        }
    case RPI_SPI_CLK_OFFSET:
        {
            TRACE_DEBUG("SPI Get Clock Divider %d", *registerField);
            break;
        }
    case RPI_SPI_DLEN_OFFSET:
        {
            TRACE_DEBUG("SPI Get Data Length %d", *registerField);
            break;
        }
    case RPI_SPI_LTOH_OFFSET:
        {
            break;
        }
    case RPI_SPI_DC_OFFSET:
        {
        }
    default:
        LOG_ERROR("Invalid SPI register access for reading: offset %d", offset);
        break;
    }
    return *registerField;
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessSPIMasterMock::OnWrite(regaddr address, uint32 data)
{
    uint8 device = 0xFF;
    if (!InRangeForSPIMaster(address, device))
    {
        MemoryAccessGPIOMock::OnWrite(address, data);
        return;
    }

    uintptr offset = GetRegisterOffset(address, SPIMasterBaseAddress[device]);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[device]) + offset);
    TRACE_DEBUG("SPI Write register %016x = %08x", offset, data);
    switch (offset)
    {
    case RPI_SPI_CS_OFFSET:
        {
            uint32 diff = (data ^ *registerField);
            if (diff & RPI_SPI_CS_LEN_LONG)
            {
                AddOperation({SPIMasterOperationCode::EnableLOSSILongDataWord, device, (data & RPI_SPI_CS_LEN_LONG) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_DMA_LEN)
            {
                AddOperation({SPIMasterOperationCode::EnableLOSSI_DMA, device, (data & RPI_SPI_CS_DMA_LEN) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_POL0)
            {
                AddOperation({SPIMasterOperationCode::SetCSPol0, device, (data & RPI_SPI_CS_POL0) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_POL1)
            {
                AddOperation({SPIMasterOperationCode::SetCSPol1, device, (data & RPI_SPI_CS_POL1) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_POL2)
            {
                AddOperation({SPIMasterOperationCode::SetCSPol2, device, (data & RPI_SPI_CS_POL2) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_LEN)
            {
                AddOperation({SPIMasterOperationCode::EnableLOSSI, device, (data & RPI_SPI_CS_LEN) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_REN)
            {
                AddOperation({SPIMasterOperationCode::EnableRead, device, (data & RPI_SPI_CS_REN) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_ADCS)
            {
                AddOperation({SPIMasterOperationCode::SetADCS, device, (data & RPI_SPI_CS_ADCS) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_INTR)
            {
                AddOperation({SPIMasterOperationCode::EnableRXInterrupt, device, (data & RPI_SPI_CS_INTR) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_INTD)
            {
                AddOperation({SPIMasterOperationCode::EnableDoneInterrupt, device, (data & RPI_SPI_CS_INTD) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_DMAEN)
            {
                AddOperation({SPIMasterOperationCode::EnableDMA, device, (data & RPI_SPI_CS_DMAEN) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_TA)
            {
                if (data & RPI_SPI_CS_TA)
                    AddOperation({SPIMasterOperationCode::StartTransfer, device});
                else
                    AddOperation({SPIMasterOperationCode::EndTransfer, device});
            }
            if (diff & RPI_SPI_CS_POL)
            {
                AddOperation({SPIMasterOperationCode::SetCSPol, device, (data & RPI_SPI_CS_POL) ? 1u : 0u});
            }
            if (data & RPI_SPI_CS_CLEAR)
            {
                if (data & RPI_SPI_CS_CLEAR_RX)
                {
                    AddOperation({SPIMasterOperationCode::ClearRxFIFO, device});
                }
                if (data & RPI_SPI_CS_CLEAR_TX)
                {
                    AddOperation({SPIMasterOperationCode::ClearTxFIFO, device});
                }
            }
            if (diff & RPI_SPI_CS_CPOL)
            {
                AddOperation({SPIMasterOperationCode::SetClockPolarity, device, (data & RPI_SPI_CS_CPOL) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_CPHA)
            {
                AddOperation({SPIMasterOperationCode::SetClockPhase, device, (data & RPI_SPI_CS_CPHA) ? 1u : 0u});
            }
            if (diff & RPI_SPI_CS_ACTIVATE_MASK)
            {
                if ((data & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS0)
                {
                    AddOperation({SPIMasterOperationCode::ActivateCS0, device});
                }
                else if ((data & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS1)
                {
                    AddOperation({SPIMasterOperationCode::ActivateCS1, device});
                }
                else if ((data & RPI_SPI_CS_ACTIVATE_MASK) == RPI_SPI_CS_ACTIVATE_CS2)
                {
                    AddOperation({SPIMasterOperationCode::ActivateCS2, device});
                }
                else
                {
                    AddOperation({SPIMasterOperationCode::ActivateNone, device});
                }
            }
            HandleWriteCSRegister(device, data);
            break;
        }
    case RPI_SPI_FIFO_OFFSET:
        {
            AddOperation({SPIMasterOperationCode::WriteFIFO, device, data});
            HandleWriteFIFORegister(device, data);
            break;
        }
    case RPI_SPI_CLK_OFFSET:
        {
            if (data != *registerField)
            {
                AddOperation({SPIMasterOperationCode::SetClockDivider, device, data & 0xFFFF});
            }
            *registerField = data & 0xFFFF;
            break;
        }
    case RPI_SPI_DLEN_OFFSET:
        {
            AddOperation({SPIMasterOperationCode::SetDataLength, device, data & 0xFFFF});
            *registerField = data & 0xFFFF;
            break;
        }
    case RPI_SPI_LTOH_OFFSET:
        {
            AddOperation({SPIMasterOperationCode::SetLOSSIOutputHoldDelay, device, data & 0xF});
            *registerField = data & 0xFFFF;
            break;
        }
    case RPI_SPI_DC_OFFSET:
        {
            uint32 diff = (data ^ *registerField);
            if (diff & RPI_SPI_DC_RPANIC_MASK)
            {
                AddOperation({SPIMasterOperationCode::SetDMAReadPanicThreshold, device, RPI_SPI_DC_RPANIC_VALUE(data)});
            }
            if (diff & RPI_SPI_DC_RDREQ_MASK)
            {
                AddOperation({SPIMasterOperationCode::SetDMAReadRequestThreshold, device, RPI_SPI_DC_RDREQ_VALUE(data)});
            }
            if (diff & RPI_SPI_DC_TPANIC_MASK)
            {
                AddOperation({SPIMasterOperationCode::SetDMAWritePanicThreshold, device, RPI_SPI_DC_TPANIC_VALUE(data)});
            }
            if (diff & RPI_SPI_DC_TDREQ_MASK)
            {
                AddOperation({SPIMasterOperationCode::SetDMAWriteRequestThreshold, device, RPI_SPI_DC_TDREQ_VALUE(data)});
            }
            break;
        }
    default:
        LOG_ERROR("Invalid SPI register access for writing: offset %d", offset);
        break;
    }
}

/// <summary>
/// Set callback function for send / receive data byte
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessSPIMasterMock::SetSendRecvDataByteCallback(SendRecvDataByteCallback callback)
{
    m_sendRecvByteCallback = callback;
}

/// <summary>
/// Determined if address is in range for SPI master registers for any device, return the device index if so
/// </summary>
/// <param name="address">Address to check</param>
/// <param name="device">SPI device if address is in range for this device</param>
/// <returns>True if address is in range for registers of a SPI device master, false otherwise</returns>
bool MemoryAccessSPIMasterMock::InRangeForSPIMaster(regaddr address, uint8 &device)
{
    for (uint8 i = 0; i < SPI_DEVICES; ++i)
    {
        uintptr baseAddr = reinterpret_cast<uintptr>(SPIMasterBaseAddress[i]);
        if ((reinterpret_cast<uintptr>(address) & SPIMasterBaseAddressMask) == baseAddr)
        {
            device = i;
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
/// <param name="baseAddress">Base address for SPI device master</param>
/// <returns>Offset relative to SPI master base address</returns>
uint32 MemoryAccessSPIMasterMock::GetRegisterOffset(regaddr address, regaddr baseAddress)
{
    return reinterpret_cast<uintptr>(address) - reinterpret_cast<uintptr>(baseAddress);
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessSPIMasterMock::AddOperation(const SPIMasterOperation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}

/// <summary>
/// Handle writing to SPI Control Register
/// </summary>
/// <param name="device">SPI device</param>
/// <param name="data">Value to write to the register</param>
void MemoryAccessSPIMasterMock::HandleWriteCSRegister(uint8 device, uint32 data)
{
    m_registers[device].CS = data & ~(RPI_SPI_CS_DONE | RPI_SPI_CS_CLEAR);
    UpdateFIFOStatus(device);
}

/// <summary>
/// Handle writing to SPI Transmit FIFO Register
/// </summary>
/// <param name="device">SPI device</param>
/// <param name="data">Value to write to the register</param>
void MemoryAccessSPIMasterMock::HandleWriteFIFORegister(uint8 device, uint8 data)
{
    if (!m_txFifo[device].IsFull())
    {
        m_txFifo[device].Write(data);
    }
    HandleSendReceiveData(device);
}

/// <summary>
/// Handle reading to SPI Receive FIFO Register
/// </summary>
/// <param name="device">SPI device</param>
/// <returns>Value read from the FIFO</returns>
uint8 MemoryAccessSPIMasterMock::HandleReadFIFORegister(uint8 device)
{
    uint8 result{};
    if (!m_rxFifo[device].IsEmpty())
    {
        result = m_rxFifo[device].Read();
        if (m_txFifo[device].IsEmpty())
            m_registers[device].CS |= RPI_SPI_CS_DONE;
    }
    UpdateFIFOStatus(device);
    return result;
}

/// <summary>
/// Handle transmitting data
/// </summary>
/// <param name="device">SPI device</param>
void MemoryAccessSPIMasterMock::HandleSendReceiveData(uint8 device)
{
    while (!m_txFifo[device].IsEmpty())
    {
        uint8 dataOut = m_txFifo[device].Read();
        if (m_registers[device].CS & RPI_SPI_CS_TA)
        {
            uint8 dataIn{};
            if (m_sendRecvByteCallback)
                m_sendRecvByteCallback(m_registers[device], dataOut, dataIn);
            if (!m_rxFifo[device].IsFull())
                m_rxFifo[device].Write(dataIn);
            UpdateFIFOStatus(device);
        }
    }
}

/// <summary>
/// Update the FIFO status bits in the SPI Status Register
/// </summary>
/// <param name="device">SPI device</param>
void MemoryAccessSPIMasterMock::UpdateFIFOStatus(uint8 device)
{
    uint32 value = m_registers[device].CS;
    if (m_rxFifo[device].IsFull())
        value |= RPI_SPI_CS_RXF;
    else
        value &= ~RPI_SPI_CS_RXF;
    if (m_rxFifo[device].IsEmpty())
        value &= ~RPI_SPI_CS_RXD;
    else
        value |= RPI_SPI_CS_RXD;
    if (m_rxFifo[device].IsThreeQuartersOrMoreFull())
        value |= RPI_SPI_CS_RXR;
    else
        value &= ~RPI_SPI_CS_RXR;
    if (!m_txFifo[device].IsFull())
        value |= RPI_SPI_CS_TXD;
    else
        value &= ~RPI_SPI_CS_TXD;
    m_registers[device].CS = value;
}

