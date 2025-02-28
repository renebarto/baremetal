//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : ARMRegisters.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Locations and definitions for Raspberry Pi ARM registers
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
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

/// @file
/// Register addresses of Raspberry Pi ARM local registers.
///
/// ARM local registers are implemented in the Raspberry Pi BCM chip, but are intended to control functionality in the ARM core.
/// For specific registers, we also define the fields and their possible values.

#pragma once

#include <stdlib/Macros.h>
#include <stdlib/Types.h>

#if BAREMETAL_RPI_TARGET <= 3
/// @brief Base address for ARM Local registers
#define ARM_LOCAL_BASE 0x40000000
#else
/// @brief Base address for ARM Local registers
#define ARM_LOCAL_BASE 0xFF800000
#endif

/// @brief Raspberry Pi ARM Local Control Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_CONTROL                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000000)
/// @brief Raspberry Pi ARM Local Core Timer Prescaler Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_PRESCALER                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000008)
/// @brief Raspberry Pi ARM Local GPU Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_GPU_INT_ROUTING          reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000000C)
/// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_PM_ROUTING_SET           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000010)
/// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_PM_ROUTING_CLR           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000014)
/// @brief Raspberry Pi ARM Local Core Timer Least Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_LS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000001C)
/// @brief Raspberry Pi ARM Local Core Timer Most Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_MS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000020)
/// @brief Raspberry Pi ARM Local Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_INT_ROUTING              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000024)
/// @brief Raspberry Pi ARM Local AXI Outstanding Read/Write Counters Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_AXI_COUNT                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000002C)
/// @brief Raspberry Pi ARM Local AXI Outstanding Interrupt Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_AXI_IRQ                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000030)
/// @brief Raspberry Pi ARM Local Timer Control / Status Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_CONTROL            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000034)
/// @brief Raspberry Pi ARM Local Timer IRQ Clear / Reload Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_WRITE              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000038)

/// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_INT_CONTROL0       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000040)
/// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_INT_CONTROL1       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000044)
/// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_INT_CONTROL2       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000048)
/// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_TIMER_INT_CONTROL3       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000004C)

/// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX_INT_CONTROL0 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000050)
/// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX_INT_CONTROL1 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000054)
/// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX_INT_CONTROL2 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000058)
/// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX_INT_CONTROL3 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000005C)

/// @brief Raspberry Pi ARM Local Core 0 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_IRQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000060)
/// @brief Raspberry Pi ARM Local Core 1 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_IRQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000064)
/// @brief Raspberry Pi ARM Local Core 2 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_IRQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000068)
/// @brief Raspberry Pi ARM Local Core 3 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_IRQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000006C)

/// @brief Raspberry Pi ARM Local Core 0 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_FIQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000070)
/// @brief Raspberry Pi ARM Local Core 1 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_FIQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000074)
/// @brief Raspberry Pi ARM Local Core 2 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_FIQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000078)
/// @brief Raspberry Pi ARM Local Core 3 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_FIQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000007C)

/// @brief Raspberry Pi ARM Local Interrupt Source Timer 0
#define ARM_LOCAL_INTSRC_TIMER0            BIT1(0)
#define ARM_LOCAL_INTSRC_TIMER1            BIT1(1)
#define ARM_LOCAL_INTSRC_TIMER2            BIT1(2)
#define ARM_LOCAL_INTSRC_TIMER3            BIT1(3)
#define ARM_LOCAL_INTSRC_MAILBOX0          BIT1(4)
#define ARM_LOCAL_INTSRC_MAILBOX1          BIT1(5)
#define ARM_LOCAL_INTSRC_MAILBOX2          BIT1(6)
#define ARM_LOCAL_INTSRC_MAILBOX3          BIT1(7)
#define ARM_LOCAL_INTSRC_GPU               BIT1(8)
#define ARM_LOCAL_INTSRC_PMU               BIT1(9)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000080)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000084)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000088)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000008C)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000090)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000094)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000098)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000009C)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000AC)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000BC)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000CC)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000DC)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000EC)

/// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX0_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F0)
/// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX1_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F4)
/// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX2_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F8)
/// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_MAILBOX3_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000FC)

/// @brief Raspberry Pi ARM Local Register region end address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
#define ARM_LOCAL_END                      reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000003FFFF)
