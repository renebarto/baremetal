//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : BCMRegisters.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Locations and definitions for Raspberry Pi registers in the Broadcomm SoC
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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

#include <baremetal/Macros.h>
#include <baremetal/Types.h>

#if RPI_TARGET == 3
/// @brief Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE                 0x3F000000
#else
/// @brief Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE                 0xFE000000
#endif
/// @brief End address for Raspberry PI BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)

// Raspberry Pi GPIO

/// @brief Raspberry Pi GPIO registers base address
#define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL0                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000000)
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL1                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000004)
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL2                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000008)
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL3                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000000C)
/// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL4                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000010)
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL5                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000014)
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000001C)
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000020)
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000028)
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000002C)
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000034)
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000038)
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000040)
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000044)
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000004C)
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000050)
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000058)
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000005C)
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000064)
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000068)
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000070)
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000074)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN0                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000007C)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN1                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000080)
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN0                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000088)
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN1                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000008C)
#if RPI_TARGET <= 3
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W)
#define RPI_GPIO_GPPUD     static_cast<uintptr>(RPI_GPIO_BASE + 0x00000094)
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK0 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000098)
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK1 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000009C)
#else // RPI target 4 or 5
#define RPI_GPIO_GPPINMUXSD static_cast<uintptr>(RPI_GPIO_BASE + 0x000000D0)
#define RPI_GPIO_GPPUPPDN0  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000E4)
#define RPI_GPIO_GPPUPPDN1  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000E8)
#define RPI_GPIO_GPPUPPDN2  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000EC)
#define RPI_GPIO_GPPUPPDN3  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000F0)
#endif

// Raspberry Pi auxilary registers (SPI1 / SPI2 / UART1)

/// @brief Raspberry Pi Auxilary registers base address
#define RPI_AUX_BASE                  RPI_BCM_IO_BASE + 0x00215000
/// @brief Raspberry Pi Auxiliary IRQ register
#define RPI_AUX_IRQ                   static_cast<uintptr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
/// @brief Raspberry Pi Auxiliary Enable register
#define RPI_AUX_ENABLES               static_cast<uintptr>(RPI_AUX_BASE + 0x00000004) // AUXENB

// Raspberry Pi auxilary mini UART registers (UART1)

/// @brief Raspberry Pi Mini UART (UART1) I/O register
#define RPI_AUX_MU_IO                 static_cast<uintptr>(RPI_AUX_BASE + 0x00000040)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register
#define RPI_AUX_MU_IER                static_cast<uintptr>(RPI_AUX_BASE + 0x00000044)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register
#define RPI_AUX_MU_IIR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000048)
/// @brief Raspberry Pi Mini UART (UART1) Line Control register
#define RPI_AUX_MU_LCR                static_cast<uintptr>(RPI_AUX_BASE + 0x0000004C)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register
#define RPI_AUX_MU_MCR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000050)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register
#define RPI_AUX_MU_LSR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000054)
/// @brief Raspberry Pi Mini UART (UART1) Modem Status register
#define RPI_AUX_MU_MSR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000058)
/// @brief Raspberry Pi Mini UART (UART1) Scratch register
#define RPI_AUX_MU_SCRATCH            static_cast<uintptr>(RPI_AUX_BASE + 0x0000005C)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register
#define RPI_AUX_MU_CNTL               static_cast<uintptr>(RPI_AUX_BASE + 0x00000060)
/// @brief Raspberry Pi Mini UART (UART1) Extra Status register
#define RPI_AUX_MU_STAT               static_cast<uintptr>(RPI_AUX_BASE + 0x00000064)
/// @brief Raspberry Pi Mini UART (UART1) Baudrate register
#define RPI_AUX_MU_BAUD               static_cast<uintptr>(RPI_AUX_BASE + 0x00000068)

/// @brief Raspberry Pi Auxiliary Enable register values
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI2
#define RPI_AUX_ENABLES_SPI2           BIT(2)
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI1
#define RPI_AUX_ENABLES_SPI1           BIT(1)
/// @brief Raspberry Pi Auxiliary Enable register Enable UART1
#define RPI_AUX_ENABLES_UART1          BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R)
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT(7)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R)
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W)
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W)
#define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Line Control register values
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_7    0
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT(0) | BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low
#define RPI_AUX_MU_MCR_RTS_LOW        BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high
#define RPI_AUX_MU_MCR_RTS_HIGH       0

/// @brief Raspberry Pi Mini UART (UART1) Line Status register values
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle
#define RPI_AUX_MU_LST_TX_IDLE        BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty
#define RPI_AUX_MU_LST_TX_EMPTY       BIT(5)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun
#define RPI_AUX_MU_LST_RX_OVERRUN     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready
#define RPI_AUX_MU_LST_RX_READY       BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS
#define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT(3)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS
#define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit
#define RPI_AUX_MU_CNTL_ENABLE_TX     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive
#define RPI_AUX_MU_CNTL_ENABLE_RX     BIT(0)
