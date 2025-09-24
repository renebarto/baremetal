//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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
/// Register addresses of Raspberry Pi peripheral registers.
///
/// For specific registers, we also define the fields and their possible values.

#pragma once

#include "baremetal/Macros.h"
#include "baremetal/Types.h"

#if BAREMETAL_RPI_TARGET == 3
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 3
#define RPI_BCM_IO_BASE 0x3F000000
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 4
#define RPI_BCM_IO_BASE 0xFE000000
#define ARM_IO_BASE     0xFF840000
#else
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 5
#define RPI_BCM_IO_BASE 0x107C000000UL
#define ARM_IO_BASE     0x107C000000UL
#endif
#if BAREMETAL_RPI_TARGET <= 4
/// @brief End address for Raspberry PI 3 / 4 BCM I/O
#define RPI_BCM_IO_END (RPI_BCM_IO_BASE + 0xFFFFFF)
#else
/// @brief End address for Raspberry PI 5 BCM I/O
#define RPI_BCM_IO_END (RPI_BCM_IO_BASE + 0x3FFFFFF)
#endif

//---------------------------------------------
// Raspberry Pi GPIO
//---------------------------------------------

/// @brief Raspberry Pi GPIO registers base address. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_BASE    RPI_BCM_IO_BASE + 0x00200000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000000)
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000004)
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL2 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000008)
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL3 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000000C)
/// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL4 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000010)
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL5 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000014)
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000001C)
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000020)
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000028)
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000002C)
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000034)
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000038)
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000040)
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000044)
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000004C)
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000050)
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000058)
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000005C)
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000064)
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000068)
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000070)
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref
/// RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000074)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000007C)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000080)
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000088)
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000008C)
#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUD     reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000094)
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000098)
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000009C)
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Raspberry Pi GPIO pull up/down pin multiplexer register. Undocumented
#define RPI_GPIO_GPPINMUXSD reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000D0)
/// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See
/// @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E4)
/// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only.
/// See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E8)
/// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only.
/// See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN2  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000EC)
/// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only.
/// See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN3  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000F0)
#else // RPI target 5
// Not supported yet
#endif

//---------------------------------------------
// Raspberry Pi auxiliary (SPI1 / SPI2 / UART1)
//---------------------------------------------

/// @brief Raspberry Pi Auxilary registers base address. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_BASE          RPI_BCM_IO_BASE + 0x00215000
/// @brief Raspberry Pi Auxiliary IRQ register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_IRQ           reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
/// @brief Raspberry Pi Auxiliary Enable register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000004) // AUXENB

/// @brief Raspberry Pi Auxiliary Enable register values
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI2. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_SPI2  BIT1(2)
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_SPI1  BIT1(1)
/// @brief Raspberry Pi Auxiliary Enable register Enable UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_UART1 BIT1(0)

//---------------------------------------------
// Raspberry Pi auxiliary mini UART (UART1)
//---------------------------------------------

/// @brief Raspberry Pi Mini UART (UART1) I/O register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IO         reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000040)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000044)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000048)
/// @brief Raspberry Pi Mini UART (UART1) Line Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000004C)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000050)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000054)
/// @brief Raspberry Pi Mini UART (UART1) Modem Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MSR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000058)
/// @brief Raspberry Pi Mini UART (UART1) Scratch register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_SCRATCH    reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000005C)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000060)
/// @brief Raspberry Pi Mini UART (UART1) Extra Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_STAT       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000064)
/// @brief Raspberry Pi Mini UART (UART1) Baudrate register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_BAUD       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000068)
#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 3
#define AUX_UART_CLOCK 250000000
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 4
#define AUX_UART_CLOCK 500000000
#else
// Not supported yet
#endif
/// @brief Calculate Raspberry Pi Mini UART (UART1) baud rate value from frequency
#define RPI_AUX_MU_BAUD_VALUE(baud)   static_cast<uint32>((AUX_UART_CLOCK / (baud * 8)) - 1)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts. See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT1(1)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts. See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT1(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R). See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT1(7)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R). See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT1(6)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W). See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT1(2)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref
/// RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT1(1)

/// @brief Raspberry Pi Mini UART (UART1) Line Control register values
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR_DATA_SIZE_7    0
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT1(0) | BIT1(1)

/// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR_RTS_LOW        BIT1(1)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR_RTS_HIGH       BIT0(1)

/// @brief Raspberry Pi Mini UART (UART1) Line Status register values
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR_TX_IDLE        BIT1(6)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR_TX_EMPTY       BIT1(5)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR_RX_OVERRUN     BIT1(1)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR_RX_READY       BIT1(0)

/// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT1(3)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT1(2)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_TX     BIT1(1)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_RX     BIT1(0)
