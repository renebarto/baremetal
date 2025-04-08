//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubGPIO.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccessStubGPIO
//
// Description : GPIO register memory access stub implementation
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

#include <baremetal/stubs/MemoryAccessStubGPIO.h>

#include <baremetal/Assert.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/Format.h>
#include <baremetal/Logger.h>
#include <baremetal/String.h>

/// @file
/// MemoryAccessStubGPIO

/// @brief Define log name
LOG_MODULE("MemoryAccessStubGPIO");

using namespace baremetal;

/// @brief GPIO base address
static uintptr GPIOBaseAddress{ RPI_GPIO_BASE };
/// @brief Mask used to check whether an address is in the GPIO register range
static uintptr GPIOBaseAddressMask{ 0xFFFFFFFFFFFFFF00 };

/// <summary>
/// MemoryAccessStubGPIO constructor
/// </summary>
MemoryAccessStubGPIO::MemoryAccessStubGPIO()
    : m_registers{}
{
}

/// <summary>
/// Read a 8 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>8 bit register value</returns>
uint8 MemoryAccessStubGPIO::Read8(regaddr address)
{
    LOG_PANIC("Call to Read8 should not happen");
    return {};
}

/// <summary>
/// Write a 8 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessStubGPIO::Write8(regaddr address, uint8 data)
{
    LOG_PANIC("Call to Write8 should not happen");
}

/// <summary>
/// Read a 16 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>16 bit register value</returns>
uint16 MemoryAccessStubGPIO::Read16(regaddr address)
{
    LOG_PANIC("Call to Read16 should not happen");
    return {};
}

/// <summary>
/// Write a 16 bit value to register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <param name="data">Data to write</param>
void MemoryAccessStubGPIO::Write16(regaddr address, uint16 data)
{
    LOG_PANIC("Call to Write16 should not happen");
}

/// <summary>
/// Read a 32 bit value from register at address
/// </summary>
/// <param name="address">Address of register</param>
/// <returns>32 bit register value</returns>
uint32 MemoryAccessStubGPIO::Read32(regaddr address)
{
    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
//    LOG_DEBUG("GPIO read register %016x = %08x", offset, *registerField);
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
            string line{ "GPIO Read Pin Mode "};
            for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
            {
                int shift = pinIndex * 3;
                uint8 pin = pinBase + pinIndex;
                uint8 pinMode = (*registerField >> shift) & 0x00000007;
                line += Format(" - Pin %d mode %x", pin, pinMode);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPLEV0_OFFSET:
        case RPI_GPIO_GPLEV1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Level "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += " - Pin %d ON ";
                else
                    line += " - Pin %d OFF";
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPEDS0_OFFSET:
        case RPI_GPIO_GPEDS1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Event Detect Status "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += " - Pin %d ON ";
                else
                    line += " - Pin %d OFF";
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPREN0_OFFSET:
        case RPI_GPIO_GPREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Rising Edge Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPFEN0_OFFSET:
        case RPI_GPIO_GPFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Falling Edge Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPHEN0_OFFSET:
        case RPI_GPIO_GPHEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin High Level Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPLEN0_OFFSET:
        case RPI_GPIO_GPLEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Low Level Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPAREN0_OFFSET:
        case RPI_GPIO_GPAREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Async Rising Edge Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
        case RPI_GPIO_GPAFEN0_OFFSET:
        case RPI_GPIO_GPAFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET)  / 4 * 32;
            string line{ "GPIO Read Pin Async Falling Edge Detect Enable "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
#if BAREMETAL_RPI_TARGET == 3
        case RPI_GPIO_GPPUD_OFFSET:
        {
            uint8 value = *registerField & 0x00000003;
            LOG_DEBUG("GPIO Read Pull Up/Down Mode %x", value);
            break;
        }
        case RPI_GPIO_GPPUDCLK0_OFFSET:
        case RPI_GPIO_GPPUDCLK1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
            string line{ "GPIO Read Pin Pull Up/Down Enable Clock "};
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000001;
                if (value)
                    line += Format(" - Pin %d ON ", pin);
                else
                    line += Format(" - Pin %d OFF", pin);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
#elif BAREMETAL_RPI_TARGET == 4
        case RPI_GPIO_GPPINMUXSD_OFFSET:
        {
            uint32 value = *registerField;
            LOG_DEBUG("GPIO Read Pin Mux Mode %x", value);
            break;
        }
        case RPI_GPIO_GPPUPPDN0_OFFSET:
        case RPI_GPIO_GPPUPPDN1_OFFSET:
        case RPI_GPIO_GPPUPPDN2_OFFSET:
        case RPI_GPIO_GPPUPPDN3_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
            string line{ "GPIO Read Pin Pull Up/Down Mode "};
            for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
            {
                int shift = pinIndex * 2;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000003;
                line += Format(" - Pin %d Pull up/down mode %x", pin, value);
            }
            LOG_DEBUG(line.c_str());
            break;
        }
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
void MemoryAccessStubGPIO::Write32(regaddr address, uint32 data)
{
    uintptr offset = GetRegisterOffset(address);
    uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
//    LOG_DEBUG("GPIO write register %016x = %08x", offset, data);
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
                    uint8 pinMode = (data >> shift) & 0x00000007;
                    LOG_DEBUG("GPIO Set Pin %d mode to %x", pin, pinMode);
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
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d ON", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPCLR0_OFFSET:
        case RPI_GPIO_GPCLR1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPEDS0_OFFSET:
        case RPI_GPIO_GPEDS1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Clear Pin %d Event Status", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPREN0_OFFSET:
        case RPI_GPIO_GPREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Rising Edge Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Rising Edge Detect OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPFEN0_OFFSET:
        case RPI_GPIO_GPFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Falling Edge Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Falling Edge Detect OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPHEN0_OFFSET:
        case RPI_GPIO_GPHEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d High Level Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d High Level Detect OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPLEN0_OFFSET:
        case RPI_GPIO_GPLEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Low Level Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Low Level Detect OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPAREN0_OFFSET:
        case RPI_GPIO_GPAREN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Async Rising Edge Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Async Rising Edge Detect OFF", pin);
                }
            }
            break;
        }
        case RPI_GPIO_GPAFEN0_OFFSET:
        case RPI_GPIO_GPAFEN1_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET)  / 4 * 32;
            uint32 diff = data ^ *registerField;
            for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
            {
                int shift = pinIndex;
                if (((diff >> shift) & 0x00000001) != 0)
                {
                    uint8 pin = pinBase + pinIndex;
                    uint8 value = (data >> shift) & 0x00000001;
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Async Falling Edge Detect ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Async Falling Edge Detect OFF", pin);
                }
            }
            break;
        }
#if BAREMETAL_RPI_TARGET == 3
        case RPI_GPIO_GPPUD_OFFSET:
        {
            uint32 diff = data ^ *registerField;
            if ((diff & 0x00000003) != 0)
            {
                uint8 value = data & 0x00000003;
                LOG_DEBUG("GPIO Set Pin Pull Up/Down Mode %x", value);
            }
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
                    if (value != 0)
                        LOG_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock ON", pin);
                    else
                        LOG_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock OFF", pin);
                }
            }
            break;
        }
#elif BAREMETAL_RPI_TARGET == 4
        case RPI_GPIO_GPPINMUXSD_OFFSET:
        {
            uint32 value = *registerField;
            LOG_DEBUG("GPIO Set Pin Mux Mode %x", value);
            break;
        }
        case RPI_GPIO_GPPUPPDN0_OFFSET:
        case RPI_GPIO_GPPUPPDN1_OFFSET:
        case RPI_GPIO_GPPUPPDN2_OFFSET:
        case RPI_GPIO_GPPUPPDN3_OFFSET:
        {
            uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
            string line{ "GPIO Set Pin Pull Up/Down Mode "};
            for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
            {
                int shift = pinIndex * 2;
                uint8 pin = pinBase + pinIndex;
                uint8 value = (*registerField >> shift) & 0x00000003;
                line += Format(" - Pin %d Pull up/down mode %x", pin, value);
            }
            LOG_DEBUG(line.c_str());
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
uint32 MemoryAccessStubGPIO::GetRegisterOffset(regaddr address)
{
    assert((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) == GPIOBaseAddress);
    return reinterpret_cast<uintptr>(address) - GPIOBaseAddress;
}
