//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessStubGPIO.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccessStubGPIO
//
// Description : GPIO register memory access stub
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

#include "baremetal/IMemoryAccess.h"
#include "stdlib/Macros.h"

/// @file
/// MemoryAccessStubGPIO

namespace baremetal {

/// @brief GPIO registers storage
struct GPIORegisters
{
    /// @brief RPI_GPIO_GPFSEL0 value
    uint32 FunctionSelect0; // 0x00
    /// @brief RPI_GPIO_GPFSEL1 value
    uint32 FunctionSelect1; // 0x04
    /// @brief RPI_GPIO_GPFSEL2 value
    uint32 FunctionSelect2; // 0x08
    /// @brief RPI_GPIO_GPFSEL3 value
    uint32 FunctionSelect3; // 0x0C
    /// @brief RPI_GPIO_GPFSEL4 value
    uint32 FunctionSelect4; // 0x10
    /// @brief RPI_GPIO_GPFSEL5 value
    uint32 FunctionSelect5; // 0x14
    /// @brief Reserved value
    uint32 Reserved_1; // 0x18
    /// @brief RPI_GPIO_GPSET0 value
    uint32 Set0; // 0x1C
    /// @brief RPI_GPIO_GPSET1 value
    uint32 Set1; // 0x20
    /// @brief Reserved value
    uint32 Reserved_2; // 0x24
    /// @brief RPI_GPIO_GPCLR0 value
    uint32 Clear0; // 0x28
    /// @brief RPI_GPIO_GPCLR1 value
    uint32 Clear1; // 0x2C
    /// @brief Reserved value
    uint32 Reserved_3; // 0x30
    /// @brief RPI_GPIO_GPLEV0 value
    uint32 PinLevel0; // 0x34
    /// @brief RPI_GPIO_GPLEV1 value
    uint32 PinLevel1; // 0x38
    /// @brief Reserved value
    uint32 Reserved_4; // 0x3C
    /// @brief RPI_GPIO_GPEDS0 value
    uint32 EventDetectStatus0; // 0x40
    /// @brief RPI_GPIO_GPEDS1 value
    uint32 EventDetectStatus1; // 0x44
    /// @brief Reserved value
    uint32 Reserved_5; // 0x48
    /// @brief RPI_GPIO_GPREN0 value
    uint32 RisingEdgeDetectEn0; // 0x4C
    /// @brief RPI_GPIO_GPREN1 value
    uint32 RisingEdgeDetectEn1; // 0x50
    /// @brief Reserved value
    uint32 Reserved_6; // 0x54
    /// @brief RPI_GPIO_GPFEN0 value
    uint32 FallingEdgeDetectEn0; // 0x58
    /// @brief RPI_GPIO_GPFEN1 value
    uint32 FallingEdgeDetectEn1; // 0x5C
    /// @brief Reserved value
    uint32 Reserved_7; // 0x60
    /// @brief RPI_GPIO_GPHEN0 value
    uint32 HighDetectEn0; // 0x64
    /// @brief RPI_GPIO_GPHEN1 value
    uint32 HighDetectEn1; // 0x68
    /// @brief Reserved value
    uint32 Reserved_8; // 0x6C
    /// @brief RPI_GPIO_GPLEN0 value
    uint32 LowDetectEn0; // 0x70
    /// @brief RPI_GPIO_GPLEN1 value
    uint32 LowDetectEn1; // 0x74
    /// @brief Reserved value
    uint32 Reserved_9; // 0x78
    /// @brief RPI_GPIO_GPAREN0 value
    uint32 AsyncRisingEdgeDetectEn0; // 0x7C
    /// @brief RPI_GPIO_GPAREN1 value
    uint32 AsyncRisingEdgeDetectEn1; // 0x80
    /// @brief Reserved value
    uint32 Reserved_10; // 0x84
    /// @brief RPI_GPIO_GPAFEN0 value
    uint32 AsyncFallingEdgeDetectEn0; // 0x88
    /// @brief RPI_GPIO_GPAFEN1 value
    uint32 AsyncFallingEdgeDetectEn1; // 0x8C
    /// @brief Reserved value
    uint32 Reserved_11; // 0x90
    /// @brief RPI_GPIO_GPPUD value
    uint32 PullUpDownEnable; // 0x94
    /// @brief RPI_GPIO_GPPUDCLK0 value
    uint32 PullUpDownEnableClock0; // 0x98
    /// @brief RPI_GPIO_GPPUDCLK1 value
    uint32 PullUpDownEnableClock1; // 0x9C
    /// @brief Reserved value
    uint32 Reserved_12[4]; // 0xA0-AC
    /// @brief Test register value
    uint32 Test; // 0xB0
#if BAREMETAL_RPI_TARGET == 4
    /// @brief Reserved value
    uint32 Reserved_13;     // 0xB4
    uint32 Reserved_14[10]; // 0xB8-DC
    uint32 Reserved_15;     // 0xE0
    /// @brief RPI_GPIO_GPPUPPDN0 value
    uint32 PullUpDown0; // 0xE4
    /// @brief RPI_GPIO_GPPUPPDN1 value
    uint32 PullUpDown1; // 0xE8
    /// @brief RPI_GPIO_GPPUPPDN2 value
    uint32 PullUpDown2; // 0xEC
    /// @brief RPI_GPIO_GPPUPPDN3 value
    uint32 PullUpDown3; // 0xF0
    /// @brief Reserved value
    uint32 Reserved_16;    // 0xF4: Alignment
    uint32 Reserved_17[2]; // 0xF8-FC: Alignment
#endif

    /// <summary>
    /// Constructor for GPIORegisters
    ///
    /// Sets default register values
    /// </summary>
    GPIORegisters()
        : FunctionSelect0{}
        , FunctionSelect1{}
        , FunctionSelect2{}
        , FunctionSelect3{}
        , FunctionSelect4{}
        , FunctionSelect5{}
        , Reserved_1{}
        , Set0{}
        , Set1{}
        , Reserved_2{}
        , Clear0{}
        , Clear1{}
        , Reserved_3{}
        , PinLevel0{}
        , PinLevel1{}
        , Reserved_4{}
        , EventDetectStatus0{}
        , EventDetectStatus1{}
        , Reserved_5{}
        , RisingEdgeDetectEn0{}
        , RisingEdgeDetectEn1{}
        , Reserved_6{}
        , FallingEdgeDetectEn0{}
        , FallingEdgeDetectEn1{}
        , Reserved_7{}
        , HighDetectEn0{}
        , HighDetectEn1{}
        , Reserved_8{}
        , LowDetectEn0{}
        , LowDetectEn1{}
        , Reserved_9{}
        , AsyncRisingEdgeDetectEn0{}
        , AsyncRisingEdgeDetectEn1{}
        , Reserved_10{}
        , AsyncFallingEdgeDetectEn0{}
        , AsyncFallingEdgeDetectEn1{}
        , Reserved_11{}
        , PullUpDownEnable{}
        , PullUpDownEnableClock0{}
        , PullUpDownEnableClock1{}
        , Reserved_12{}
        , Test{}
#if BAREMETAL_RPI_TARGET == 4
        , Reserved_13{}
        , Reserved_14{}
        , Reserved_15{}
        , PullUpDown0{0xAAA95555}
        , PullUpDown1{0xA0AAAAAA}
        , PullUpDown2{0x50AAA95A}
        , PullUpDown3{0x00055555}
        , Reserved_16{}
        , Reserved_17{}
#endif
    {
    }
} PACKED;

/// @brief MemoryAccess implementation for GPIO stub
class MemoryAccessStubGPIO : public IMemoryAccess
{
private:
    /// @brief Saved GPIO register values
    GPIORegisters m_registers;

public:
    MemoryAccessStubGPIO();
    uint8 Read8(regaddr address) override;
    void Write8(regaddr address, uint8 data) override;

    uint16 Read16(regaddr address) override;
    void Write16(regaddr address, uint16 data) override;

    uint32 Read32(regaddr address) override;
    void Write32(regaddr address, uint32 data) override;

private:
    uint32 GetRegisterOffset(regaddr address);
};

} // namespace baremetal
