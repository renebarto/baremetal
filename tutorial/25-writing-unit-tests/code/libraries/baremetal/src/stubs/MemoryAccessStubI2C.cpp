//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubI2C.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccessStubI2C
//
// Description : I2C register memory access stub implementation
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

#include "baremetal/stubs/MemoryAccessStubI2C.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"

/// @file
/// MemoryAccessStubI2C

/// @brief Define log name
LOG_MODULE("MemoryAccessStubI2C");

using namespace baremetal;

/// @brief I2C bus 0 register base address
static regaddr I2CBaseAddress0{RPI_I2C0_BASE};
/// @brief I2C bus 1 register base address
static regaddr I2CBaseAddress1{RPI_I2C1_BASE};
#if BAREMETAL_RPI_TARGET == 3
/// @brief I2C bus 2 register base address
static regaddr I2CBaseAddress2{RPI_I2C2_BASE};
#elif BAREMETAL_RPI_TARGET == 4
/// @brief I2C bus 3 register base address
static regaddr I2CBaseAddress3{RPI_I2C3_BASE};
/// @brief I2C bus 4 register base address
static regaddr I2CBaseAddress4{RPI_I2C4_BASE};
/// @brief I2C bus 5 register base address
static regaddr I2CBaseAddress5{RPI_I2C5_BASE};
/// @brief I2C bus 6 register base address
static regaddr I2CBaseAddress6{RPI_I2C6_BASE};
#endif
/// @brief Mask to check whether an address is in the range for a specific I2C bus
static uintptr I2CBaseAddressMask{0xFFFFFFFFFFFFFFE0};

/// <summary>
/// MemoryAccessStubI2C constructor
/// </summary>
MemoryAccessStubI2C::MemoryAccessStubI2C()
    : m_registers{}
    , m_i2cMasterBaseAddress{}
    , m_rxFifo{}
    , m_txFifo{}
    , m_sendAddressByteCallback{}
    , m_recvDataByteCallback{}
    , m_sendDataByteCallback{}
    , m_numBytesReceived{}
    , m_numBytesSent{}
{
}

/// <summary>
/// Set I2C device bus number
/// </summary>
/// <param name="bus">I2C device bus number (0..6) depending on Raspberry Pi model</param>
void MemoryAccessStubI2C::SetBus(uint8 bus)
{
    switch (bus)
    {
    case 0:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress0);
        break;
    case 1:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress1);
        break;
#if BAREMETAL_RPI_TARGET == 3
    case 2:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress2);
        break;
#elif BAREMETAL_RPI_TARGET == 4
    case 3:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress3);
        break;
    case 4:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress4);
        break;
    case 5:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress5);
        break;
    case 6:
        m_i2cMasterBaseAddress = reinterpret_cast<uintptr>(I2CBaseAddress6);
        break;
#endif
    default:
        assert(false);
    }
}

/// <summary>
/// Read a 8 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>8 bit register value</returns>
uint8 MemoryAccessStubI2C::Read8(regaddr address)
{
    LOG_PANIC("Call to Read8 should not happen");
    return {};
}

/// <summary>
/// Write a 8 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessStubI2C::Write8(regaddr address, uint8 data)
{
    LOG_PANIC("Call to Write8 should not happen");
}

/// <summary>
/// Read a 16 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>16 bit register value</returns>
uint16 MemoryAccessStubI2C::Read16(regaddr address)
{
    LOG_PANIC("Call to Read16 should not happen");
    return {};
}

/// <summary>
/// Write a 16 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessStubI2C::Write16(regaddr address, uint16 data)
{
    LOG_PANIC("Call to Write16 should not happen");
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessStubI2C::Read32(regaddr address)
{
    uintptr addr = reinterpret_cast<uintptr>(address);
    if ((addr >= RPI_GPIO_BASE) && (addr < RPI_GPIO_END))
    {
        return MemoryAccessStubGPIO::Read32(address);
    }

    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
    LOG_DEBUG("I2C Read register %016x = %08x", offset, *registerField);
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
            LOG_DEBUG(line.c_str());
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
            LOG_DEBUG(line.c_str());
            break;
        }
    case RPI_I2C_DLEN_OFFSET:
        {
            LOG_DEBUG("I2C Get Data Length %d", *registerField);
            break;
        }
    case RPI_I2C_A_OFFSET:
        {
            LOG_DEBUG("I2C Get Address %02x", *registerField);
            break;
        }
    case RPI_I2C_FIFO_OFFSET:
        {
            LOG_DEBUG("I2C Read FIFO %02x", *registerField);
            *registerField = HandleReadFIFORegister();
            break;
        }
    case RPI_I2C_DIV_OFFSET:
        {
            LOG_DEBUG("I2C Set Clock Divider %d", *registerField);
            break;
        }
    case RPI_I2C_DEL_OFFSET:
        {
            if (*registerField & 0xFFFF0000)
            {
                LOG_DEBUG("I2C Get Falling Edge Delay %d", (*registerField >> 16));
            }
            if (*registerField & 0x0000FFFF)
            {
                LOG_DEBUG("I2C Get Rising Edge Delay %d", (*registerField & 0x0000FFFF));
            }
            break;
        }
    case RPI_I2C_CLKT_OFFSET:
        {
            LOG_DEBUG("I2C Get Clock Stretch Timeout %d", (*registerField & 0x0000FFFF));
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
void MemoryAccessStubI2C::Write32(regaddr address, uint32 data)
{
    uintptr addr = reinterpret_cast<uintptr>(address);
    if ((addr >= RPI_GPIO_BASE) && (addr < RPI_GPIO_END))
    {
        MemoryAccessStubGPIO::Write32(address, data);
        return;
    }

    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
    LOG_DEBUG("I2C Write register %016x = %08x", offset, data);
    switch (offset)
    {
    case RPI_I2C_C_OFFSET:
        {
            uint32 diff = (data ^ *registerField) | 0x00B0;
            if (diff & RPI_I2C_C_ENABLE)
            {
                if (data & RPI_I2C_C_ENABLE)
                    LOG_DEBUG("I2C Enable Controller");
                else
                    LOG_DEBUG("I2C Disable Controller");
            }
            if (diff & RPI_I2C_C_INTR_ENABLE)
            {
                if (data & RPI_I2C_C_INTR_ENABLE)
                    LOG_DEBUG("I2C Enable RX Interrupt");
                else
                    LOG_DEBUG("I2C Disable RX Interrupt");
            }
            if (diff & RPI_I2C_C_INTT_ENABLE)
            {
                if (data & RPI_I2C_C_INTT_ENABLE)
                    LOG_DEBUG("I2C Enable TX Interrupt");
                else
                    LOG_DEBUG("I2C Disable TX Interrupt");
            }
            if (diff & RPI_I2C_C_INTD_ENABLE)
            {
                if (data & RPI_I2C_C_INTD_ENABLE)
                    LOG_DEBUG("I2C Enable Done Interrupt");
                else
                    LOG_DEBUG("I2C Disable Done Interrupt");
            }
            if (diff & RPI_I2C_C_READ)
            {
                if (data & RPI_I2C_C_READ)
                    LOG_DEBUG("I2C Read Mode");
                else
                    LOG_DEBUG("I2C Write Mode");
            }
            if (data & RPI_I2C_C_CLEAR)
                LOG_DEBUG("I2C Clear FIFO");
            if (data & RPI_I2C_C_ST)
                LOG_DEBUG("I2C Start Transfer");
            HandleWriteControlRegister(data);
            break;
        }
    case RPI_I2C_S_OFFSET:
        {
            if (data & RPI_I2C_S_CLKT)
                LOG_DEBUG("I2C Reset Clock Stretch Timeout");
            if (data & RPI_I2C_S_ERR)
                LOG_DEBUG("I2C Reset Ack Error");
            if (data & RPI_I2C_S_DONE)
                LOG_DEBUG("I2C Reset Done");
            HandleWriteStatusRegister(data);
            break;
        }
    case RPI_I2C_DLEN_OFFSET:
        {
            LOG_DEBUG("I2C Set Data Length %d", data);
            *registerField = data;
            break;
        }
    case RPI_I2C_A_OFFSET:
        {
            LOG_DEBUG("I2C Set Address %02x", data);
            *registerField = data;
            break;
        }
    case RPI_I2C_FIFO_OFFSET:
        {
            LOG_DEBUG("I2C Write FIFO %02x", data);
            HandleWriteFIFORegister(data);
            break;
        }
    case RPI_I2C_DIV_OFFSET:
        {
            if (data != *registerField)
            {
                LOG_DEBUG("I2C Set Clock Divider %d", data);
            }
            *registerField = data;
            break;
        }
    case RPI_I2C_DEL_OFFSET:
        {
            uint32 diff = data & *registerField;
            if (diff & 0xFFFF0000)
            {
                LOG_DEBUG("I2C Set Falling Edge Delay %d", (data >> 16));
            }
            if (diff & 0x0000FFFF)
            {
                LOG_DEBUG("I2C Set Rising Edge Delay %d", (data & 0x0000FFFF));
            }
            *registerField = data;
            break;
        }
    case RPI_I2C_CLKT_OFFSET:
        {
            uint32 diff = data & *registerField;
            if (diff & 0x0000FFFF)
            {
                LOG_DEBUG("I2C Set Clock Stretch Timeout %d", (data & 0x0000FFFF));
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
void MemoryAccessStubI2C::SetSendAddressByteCallback(SendAddressByteCallback callback)
{
    m_sendAddressByteCallback = callback;
}

/// <summary>
/// Set callback function for data receive
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessStubI2C::SetRecvDataByteCallback(RecvDataByteCallback callback)
{
    m_recvDataByteCallback = callback;
}

/// <summary>
/// Set callback function for data send
/// </summary>
/// <param name="callback">Address of callback function</param>
void MemoryAccessStubI2C::SetSendDataByteCallback(SendDataByteCallback callback)
{
    m_sendDataByteCallback = callback;
}

/// <summary>
/// Determine register address offset, by checking against the different bus numbers
///
/// If no bus matches the address, an assert if fired
/// </summary>
/// <param name="address">Address to check</param>
/// <returns>Offset relative to the base address for the respective bus, if one exists. Will return 0 (but assert) otherwise</returns>
uint32 MemoryAccessStubI2C::GetRegisterOffset(regaddr address)
{
    assert((reinterpret_cast<uintptr>(address) & I2CBaseAddressMask) == m_i2cMasterBaseAddress);
    return reinterpret_cast<uintptr>(address) - m_i2cMasterBaseAddress;
}

/// <summary>
/// Handle writing to I2C Control Register
/// </summary>
/// <param name="data">Value to write to the register</param>
void MemoryAccessStubI2C::HandleWriteControlRegister(uint32 data)
{
    if ((data & RPI_I2C_C_CLEAR) != 0)
    {
        m_rxFifo.Flush();
        m_txFifo.Flush();
        m_numBytesReceived = 0;
        m_numBytesSent = 0;
    }
    m_registers.ControlRegister = data & ~(RPI_I2C_C_CLEAR | RPI_I2C_C_ST);
    if (((data & RPI_I2C_C_ENABLE) != 0) && ((data & RPI_I2C_C_ST) != 0))
    {
        // We started a (new) transaction
        m_registers.StatusRegister = m_registers.StatusRegister | RPI_I2C_S_TA;
        if (m_sendAddressByteCallback)
        {
            if (!(*m_sendAddressByteCallback)(m_registers, m_registers.AddressRegister))
                m_registers.StatusRegister |= RPI_I2C_S_ERR;
        }
        if ((m_registers.ControlRegister & RPI_I2C_C_READ) != 0)
        {
            HandleRecvData();
        }
        else
        {
            HandleSendData();
        }
    }
}

/// <summary>
/// Handle writing to I2C Status Register
/// </summary>
/// <param name="data">Value to write to the register</param>
void MemoryAccessStubI2C::HandleWriteStatusRegister(uint32 data)
{
    if (data & RPI_I2C_S_ERR)
    {
        m_registers.StatusRegister = m_registers.StatusRegister & ~RPI_I2C_S_ERR;
    }
    if (data & RPI_I2C_S_DONE)
    {
        m_registers.StatusRegister = m_registers.StatusRegister & ~(RPI_I2C_S_DONE | RPI_I2C_S_TA);
    }
    if (data & RPI_I2C_S_CLKT)
    {
        m_registers.StatusRegister = m_registers.StatusRegister & ~RPI_I2C_S_CLKT;
    }
}

/// <summary>
/// Handle writing to I2C Transmit FIFO Register
/// </summary>
/// <param name="data">Value to write to the register</param>
void MemoryAccessStubI2C::HandleWriteFIFORegister(uint8 data)
{
    if (!m_txFifo.IsFull())
    {
        m_txFifo.Write(data);
    }
    UpdateFIFOStatus();
    HandleSendData();
}

/// <summary>
/// Handle reading to I2C Receive FIFO Register
/// </summary>
/// <returns>Value read from the FIFO</returns>
uint8 MemoryAccessStubI2C::HandleReadFIFORegister()
{
    uint8 result{};
    HandleRecvData();
    if (!m_rxFifo.IsEmpty())
    {
        result = m_rxFifo.Read();
    }
    UpdateFIFOStatus();
    return result;
}

/// <summary>
/// Handle transmitting data
/// </summary>
void MemoryAccessStubI2C::HandleSendData()
{
    if (((m_registers.StatusRegister & RPI_I2C_S_TA) != 0) && ((m_registers.ControlRegister & RPI_I2C_C_READ) == 0))
    {
        while (!m_txFifo.IsEmpty() && (m_numBytesSent < m_registers.DataLengthRegister))
        {
            auto data = m_txFifo.Read();
            UpdateFIFOStatus();
            bool sentData{};
            if (m_sendDataByteCallback)
                sentData = (*m_sendDataByteCallback)(m_registers, data);
            if (sentData)
                ++m_numBytesSent;
            else
            {
                CancelTransfer();
                break;
            }
        }
    }
    if (m_numBytesSent >= m_registers.DataLengthRegister)
        EndTransfer();
}

/// <summary>
/// Handle receiving data
/// </summary>
void MemoryAccessStubI2C::HandleRecvData()
{
    if (((m_registers.ControlRegister & RPI_I2C_C_ENABLE) != 0) && ((m_registers.ControlRegister & RPI_I2C_C_READ) != 0))
    {
        while (!m_rxFifo.IsFull() && (m_numBytesReceived < m_registers.DataLengthRegister))
        {
            uint8 data{};
            bool receivedData{};
            if (m_recvDataByteCallback)
            {
                receivedData = (*m_recvDataByteCallback)(m_registers, data);
            }
            if (receivedData)
            {
                m_rxFifo.Write(data);
                UpdateFIFOStatus();
                ++m_numBytesReceived;
            }
            else
            {
                CancelTransfer();
                break;
            }
        }
    }
    if (m_numBytesReceived >= m_registers.DataLengthRegister)
        EndTransfer();
}

/// <summary>
/// Update the FIFO status bits in the I2C Status Register
/// </summary>
void MemoryAccessStubI2C::UpdateFIFOStatus()
{
    if (m_txFifo.IsEmpty())
        m_registers.StatusRegister |= RPI_I2C_S_TXE;
    else
        m_registers.StatusRegister &= (~RPI_I2C_S_TXE);
    if (m_txFifo.IsFull())
        m_registers.StatusRegister |= RPI_I2C_S_TXD;
    else
        m_registers.StatusRegister &= (~RPI_I2C_S_TXD);
    if (m_txFifo.IsOneQuarterOrLessFull())
        m_registers.StatusRegister |= RPI_I2C_S_TXW;
    else
        m_registers.StatusRegister &= (~RPI_I2C_S_TXW);
    if (m_rxFifo.IsEmpty())
        m_registers.StatusRegister &= (~RPI_I2C_S_RXD);
    else
        m_registers.StatusRegister |= RPI_I2C_S_RXD;
    if (m_rxFifo.IsFull())
        m_registers.StatusRegister |= RPI_I2C_S_RXF;
    else
        m_registers.StatusRegister &= (~RPI_I2C_S_RXF);
    if (m_rxFifo.IsThreeQuartersOrMoreFull())
        m_registers.StatusRegister |= RPI_I2C_S_RXR;
    else
        m_registers.StatusRegister &= (~RPI_I2C_S_RXR);
}

/// <summary>
/// Cancel a transfer, i.e. set error flag to denote NACK, and set done flag
/// </summary>
void MemoryAccessStubI2C::CancelTransfer()
{
    m_registers.StatusRegister |= (RPI_I2C_S_ERR | RPI_I2C_S_DONE);
    m_registers.StatusRegister &= (~RPI_I2C_S_TA);
}

/// <summary>
/// Finalize a transfer, i.e. set done flag
/// </summary>
void MemoryAccessStubI2C::EndTransfer()
{
    m_registers.StatusRegister |= (RPI_I2C_S_DONE);
    m_registers.StatusRegister &= (~RPI_I2C_S_TA);
}
