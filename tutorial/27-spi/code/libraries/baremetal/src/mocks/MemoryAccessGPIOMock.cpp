//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessGPIOMock.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccessGPIOMock
//
// Description : GPIO memory access mock class
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

#include "baremetal/mocks/MemoryAccessGPIOMock.h"

#include "baremetal/Assert.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"

/// @file
/// Memory access mock class implementation

/// @brief Define log name
LOG_MODULE("MemoryAccessGPIOMock");

using namespace baremetal;

/// <summary>
/// Convert pin function to string
/// </summary>
/// <param name="function">Pin function</param>
/// <returns>String representing pin function</returns>
static String PinFunctionToString(GPIOFunction function)
{
    String result{};
    switch (function)
    {
    case GPIOFunction::Input:
        result = "Input";
        break;
    case GPIOFunction::Output:
        result = "Output";
        break;
    case GPIOFunction::AlternateFunction0:
        result = "Alt0";
        break;
    case GPIOFunction::AlternateFunction1:
        result = "Alt1";
        break;
    case GPIOFunction::AlternateFunction2:
        result = "Alt2";
        break;
    case GPIOFunction::AlternateFunction3:
        result = "Alt3";
        break;
    case GPIOFunction::AlternateFunction4:
        result = "Alt4";
        break;
    case GPIOFunction::AlternateFunction5:
        result = "Alt5";
        break;
    case GPIOFunction::Unknown:
        result = "Unknown";
        break;
    }
    return result;
}

/// <summary>
/// Convert pull up/down mode to string
/// </summary>
/// <param name="mode">Pull up/down mode</param>
/// <returns>String representing pull up/down mode</returns>
static String PullModeToString(uint8 mode)
{
    String result{};
    switch (mode & 0x03)
    {
    case 0:
        result = "None";
        break;
    case 1:
#if BAREMETAL_RPI_TARGET == 3
        result = "PullDown";
#else
        result = "PullUp";
#endif
        break;
    case 2:
#if BAREMETAL_RPI_TARGET == 3
        result = "PullUp";
#else
        result = "PullDown";
#endif
        break;
    case 3:
        result = "Reserved";
        break;
    }
    return result;
}

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(PhysicalGPIOPinOperationCode code)
{
    String result{};
    switch (code)
    {
    case PhysicalGPIOPinOperationCode::GetPinValue:
        result = "GetPinValue";
        break;
    case PhysicalGPIOPinOperationCode::GetPinEventStatus:
        result = "GetPinEventStatus";
        break;
    case PhysicalGPIOPinOperationCode::SetPinMode:
        result = "SetPinMode";
        break;
    case PhysicalGPIOPinOperationCode::SetPinValue:
        result = "SetPinValue";
        break;
    case PhysicalGPIOPinOperationCode::ClearPinEventStatus:
        result = "ClearPinEventStatus";
        break;
    case PhysicalGPIOPinOperationCode::SetPinRisingEdgeInterruptEnable:
        result = "SetPinRisingEdgeInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPinFallingEdgeInterruptEnable:
        result = "SetPinFallingEdgeInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPinHighLevelInterruptEnable:
        result = "SetPinHighLevelInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPinLowLevelInterruptEnable:
        result = "SetPinLowLevelInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPinAsyncRisingEdgeInterruptEnable:
        result = "SetPinAsyncRisingEdgeInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPinAsyncFallingEdgeInterruptEnable:
        result = "SetPinAsyncFallingEdgeInterruptEnable";
        break;
    case PhysicalGPIOPinOperationCode::SetPullUpDownMode:
        result = "SetPullUpDownMode";
        break;
    case PhysicalGPIOPinOperationCode::SetPinPullUpDownClock:
        result = "SetPinPullUpDownClock";
        break;
    case PhysicalGPIOPinOperationCode::SetPinMuxMode:
        result = "SetPinMuxMode";
        break;
    case PhysicalGPIOPinOperationCode::SetPinPullUpDownMode:
        result = "SetPinPullUpDownMode";
        break;

    }
    return result;
}

/// <summary>
/// Serialize a GPIO memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const PhysicalGPIOPinOperation &value)
{
    String result = Format("Operation=%s, Pin=%s, ", OperationCodeToString(value.operation).c_str(), (value.pin == 0xFF ? "None" : Format("%d", value.pin).c_str()));
    switch (value.operation)
    {
    case PhysicalGPIOPinOperationCode::SetPinMode:
        result += Format("Function=%s", PinFunctionToString(value.function).c_str());
        break;
    case PhysicalGPIOPinOperationCode::SetPullUpDownMode:
    case PhysicalGPIOPinOperationCode::SetPinPullUpDownMode:
        result += Format("PullMode=%s", PullModeToString(static_cast<uint8>(value.argument)).c_str());
        break;
    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

/// @brief GPIO base address
static uintptr GPIOBaseAddress{RPI_GPIO_BASE};
/// @brief Mask used to check whether an address is in the GPIO register range
static uintptr GPIOBaseAddressMask{0xFFFFFFFFFFFFFF00};

/// <summary>
/// MemoryAccessGPIOMock constructor
/// </summary>
MemoryAccessGPIOMock::MemoryAccessGPIOMock()
    : m_registers{}
    , m_accessOps{}
    , m_numAccessOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t MemoryAccessGPIOMock::GetNumGPIOOperations() const
{
    return m_numAccessOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const PhysicalGPIOPinOperation &MemoryAccessGPIOMock::GetGPIOOperation(size_t index) const
{
    assert(index < m_numAccessOps);
    return m_accessOps[index];
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessGPIOMock::OnRead(regaddr address)
{
    if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
        return MemoryAccessMock::OnRead(address);

    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
    switch (offset)
    {
    case RPI_GPIO_GPFSEL0_OFFSET:
    case RPI_GPIO_GPFSEL1_OFFSET:
    case RPI_GPIO_GPFSEL2_OFFSET:
    case RPI_GPIO_GPFSEL3_OFFSET:
    case RPI_GPIO_GPFSEL4_OFFSET:
    case RPI_GPIO_GPFSEL5_OFFSET:
        break;
    case RPI_GPIO_GPLEV0_OFFSET:
    case RPI_GPIO_GPLEV1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET) / 4 * 32;
            String line{"GPIO Read Pin Level "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                AddOperation({ PhysicalGPIOPinOperationCode::GetPinValue, pin, value });
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            TRACE_DEBUG(line.c_str());
            break;
        }
    case RPI_GPIO_GPEDS0_OFFSET:
    case RPI_GPIO_GPEDS1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
            String line{"GPIO Read Pin Event Detect Status "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                AddOperation({ PhysicalGPIOPinOperationCode::GetPinEventStatus, pin, value });
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            TRACE_DEBUG(line.c_str());
            break;
        }
    case RPI_GPIO_GPREN0_OFFSET:
    case RPI_GPIO_GPREN1_OFFSET:
    case RPI_GPIO_GPFEN0_OFFSET:
    case RPI_GPIO_GPFEN1_OFFSET:
    case RPI_GPIO_GPHEN0_OFFSET:
    case RPI_GPIO_GPHEN1_OFFSET:
    case RPI_GPIO_GPLEN0_OFFSET:
    case RPI_GPIO_GPLEN1_OFFSET:
    case RPI_GPIO_GPAREN0_OFFSET:
    case RPI_GPIO_GPAREN1_OFFSET:
    case RPI_GPIO_GPAFEN0_OFFSET:
    case RPI_GPIO_GPAFEN1_OFFSET:
        break;
#if BAREMETAL_RPI_TARGET == 3
    case RPI_GPIO_GPPUD_OFFSET:
    case RPI_GPIO_GPPUDCLK0_OFFSET:
    case RPI_GPIO_GPPUDCLK1_OFFSET:
        break;
#elif BAREMETAL_RPI_TARGET == 4
    case RPI_GPIO_GPPINMUXSD_OFFSET:
    case RPI_GPIO_GPPUPPDN0_OFFSET:
    case RPI_GPIO_GPPUPPDN1_OFFSET:
    case RPI_GPIO_GPPUPPDN2_OFFSET:
    case RPI_GPIO_GPPUPPDN3_OFFSET:
        break;
#endif
    default:
        LOG_ERROR("Invalid register access for reading: offset %d", offset);
        break;
    }
    return *registerField;
}

/// <summary>
/// Write a 32 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessGPIOMock::OnWrite(regaddr address, uint32 data)
{
    if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
    {
        MemoryAccessMock::OnWrite(address, data);
        return;
    }

    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
    switch (offset)
    {
    case RPI_GPIO_GPFSEL0_OFFSET:
    case RPI_GPIO_GPFSEL1_OFFSET:
    case RPI_GPIO_GPFSEL2_OFFSET:
    case RPI_GPIO_GPFSEL3_OFFSET:
    case RPI_GPIO_GPFSEL4_OFFSET:
    case RPI_GPIO_GPFSEL5_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
            {
                int shift = pinIndex * 3;
                if (((diff >> shift) & 0x00000007) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000007;
                    uint8 lookup[]{0, 1, 7, 6, 2, 3, 4, 5};
                    auto pinMode = static_cast<GPIOFunction>(lookup[value]);
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinMode, pin, pinMode});
                    String modeName = PinFunctionToString(pinMode);
                    TRACE_DEBUG("GPIO Set Pin %d Mode %s", pin, modeName.c_str());
                }
            }
            break;
        }
    case RPI_GPIO_GPSET0_OFFSET:
    case RPI_GPIO_GPSET1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPSET0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value)
                    {
                        AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 1});
                        TRACE_DEBUG("GPIO Set Pin %d ON", pin);
                    }
                }
            }
            break;
        }
    case RPI_GPIO_GPCLR0_OFFSET:
    case RPI_GPIO_GPCLR1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value)
                    {
                        AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 0});
                        TRACE_DEBUG("GPIO Set Pin %d OFF", pin);
                    }
                }
            }
            break;
        }
    case RPI_GPIO_GPEDS0_OFFSET:
    case RPI_GPIO_GPEDS1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                    {
                        AddOperation({PhysicalGPIOPinOperationCode::ClearPinEventStatus, pin, value});
                        TRACE_DEBUG("GPIO Clear Pin %d Event Status", pin);
                    }
                }
            }
            break;
        }
    case RPI_GPIO_GPREN0_OFFSET:
    case RPI_GPIO_GPREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinRisingEdgeInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect OFF", pin);
                }
            }
            break;
        }
    case RPI_GPIO_GPFEN0_OFFSET:
    case RPI_GPIO_GPFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinFallingEdgeInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect OFF", pin);
                 }
            }
            break;
        }
    case RPI_GPIO_GPHEN0_OFFSET:
    case RPI_GPIO_GPHEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinHighLevelInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d High Level Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d High Level Detect OFF", pin);
                }
            }
            break;
        }
    case RPI_GPIO_GPLEN0_OFFSET:
    case RPI_GPIO_GPLEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinLowLevelInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Low Level Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Low Level Detect OFF", pin);
                }
            }
            break;
        }
    case RPI_GPIO_GPAREN0_OFFSET:
    case RPI_GPIO_GPAREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncRisingEdgeInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect OFF", pin);
                }
            }
            break;
        }
    case RPI_GPIO_GPAFEN0_OFFSET:
    case RPI_GPIO_GPAFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncFallingEdgeInterruptEnable, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect OFF", pin);
                }
            }
            break;
        }
#if BAREMETAL_RPI_TARGET == 3
    case RPI_GPIO_GPPUD_OFFSET:
        {
            uint8 value = data & 0x00000003;
            AddOperation({PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, value});
            String modeName = PullModeToString(value);
            TRACE_DEBUG("GPIO Set Pin Pull Up/Down Mode %s", modeName.c_str());
            break;
        }
    case RPI_GPIO_GPPUDCLK0_OFFSET:
    case RPI_GPIO_GPPUDCLK1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, pin, value});
                    if (value != 0)
                        TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock ON", pin);
                    else
                        TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock OFF", pin);
                }
            }
            break;
        }
#elif BAREMETAL_RPI_TARGET == 4
    case RPI_GPIO_GPPINMUXSD_OFFSET:
        {
            uint32 value = *registerField;
            TRACE_DEBUG("GPIO Set Pin Mux Mode %x", value);
            AddOperation({PhysicalGPIOPinOperationCode::SetPinMuxMode, 0xFF, value});
            break;
        }
    case RPI_GPIO_GPPUPPDN0_OFFSET:
    case RPI_GPIO_GPPUPPDN1_OFFSET:
    case RPI_GPIO_GPPUPPDN2_OFFSET:
    case RPI_GPIO_GPPUPPDN3_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
            {
                int shift = pinIndex * 2;
                if (((diff >> shift) & 0x00000003) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000003;
                    String modeName = PullModeToString(value);
                    AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, pin, value});
                    TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Mode %s", pin, modeName.c_str());
                }
            }
            break;
        }
#endif
    default:
        LOG_ERROR("Invalid GPIO register access for writing: offset %d", offset);
        break;
    }
    *registerField = data;
}

/// <summary>
/// Determine register address offset relative to GPIO base address
///
/// If the address is not in the correct range, an assert is fired
/// </summary>
/// <param name="address">Address to check</param>
/// <returns>Offset relative to GPIO base address</returns>
uint32 MemoryAccessGPIOMock::GetRegisterOffset(regaddr address)
{
    return reinterpret_cast<uintptr>(address) - GPIOBaseAddress;
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void MemoryAccessGPIOMock::AddOperation(const PhysicalGPIOPinOperation& operation)
{
    assert(m_numAccessOps < BufferSize);
    m_accessOps[m_numAccessOps++] = operation;
}
