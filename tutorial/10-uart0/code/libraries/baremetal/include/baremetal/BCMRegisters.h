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

#pragma once

#include <baremetal/Macros.h>
#include <baremetal/Types.h>

#define GPU_CACHED_BASE                 0x40000000
#define GPU_UNCACHED_BASE               0xC0000000

#define GPU_MEM_BASE                    GPU_UNCACHED_BASE

// Convert ARM address to GPU bus address (also works for aliases)
#define ARM_TO_GPU(addr)                (((addr) & ~0xC0000000) | GPU_MEM_BASE)
#define GPU_TO_ARM(addr)                ((addr) & ~0xC0000000)

#if BAREMETAL_RPI_TARGET == 3
// Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE                 0x3F000000
#else
// Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE                 0xFE000000
#endif
// End address for Raspberry PI BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)

//---------------------------------------------
// Raspberry Pi System Timer
//---------------------------------------------

#define RPI_SYSTMR_BASE                 RPI_BCM_IO_BASE + 0x00003000
#define RPI_SYSTMR_CS                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000000)
#define RPI_SYSTMR_LO                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000004)
#define RPI_SYSTMR_HI                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000008)
#define RPI_SYSTMR_CMP0                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x0000000C)
#define RPI_SYSTMR_CMP1                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000010)
#define RPI_SYSTMR_CMP2                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000014)
#define RPI_SYSTMR_CMP3                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000018)

//---------------------------------------------
// Raspberry Pi Mailbox
//---------------------------------------------

#define RPI_MAILBOX_BASE                RPI_BCM_IO_BASE + 0x0000B880
#define RPI_MAILBOX0_READ               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000000)
#define RPI_MAILBOX0_POLL               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000010)
#define RPI_MAILBOX0_SENDER             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000014)
#define RPI_MAILBOX0_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000018)
#define RPI_MAILBOX_CONFIG              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000001C)
#define RPI_MAILBOX1_WRITE              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000020)
#define RPI_MAILBOX1_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000038)
#define RPI_MAILBOX_RESPONSE_SUCCESS    BIT(31)
#define RPI_MAILBOX_RESPONSE_ERROR      BIT(31) | BIT(0)
#define RPI_MAILBOX_TAG_RESPONSE        BIT(31)
#define RPI_MAILBOX_STATUS_EMPTY        BIT(30)
#define RPI_MAILBOX_STATUS_FULL         BIT(31)
#define RPI_MAILBOX_REQUEST             0

//---------------------------------------------
// Raspberry Pi Power Management
//---------------------------------------------

#define RPI_PWRMGT_BASE                 RPI_BCM_IO_BASE + 0x00100000
#define RPI_PWRMGT_RSTC                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x0000001C)
#define RPI_PWRMGT_RSTS                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000020)
#define RPI_PWRMGT_WDOG                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000024)
#define RPI_PWRMGT_WDOG_MAGIC           0x5A000000
#define RPI_PWRMGT_RSTC_CLEAR           0xFFFFFFCF
#define RPI_PWRMGT_RSTC_REBOOT          0x00000020
#define RPI_PWRMGT_RSTC_RESET           0x00000102
#define RPI_PWRMGT_RSTS_PART_CLEAR      0xFFFFFAAA

//---------------------------------------------
// Raspberry Pi GPIO
//---------------------------------------------

// Raspberry Pi GPIO registers base address
#define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
// Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000000)
// Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000004)
// Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL2                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000008)
// Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL3                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000000C)
// Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL4                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000010)
// Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL5                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000014)
// Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000001C)
// Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000020)
// Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000028)
// Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000002C)
// Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000034)
// Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000038)
// Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000040)
// Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000044)
// Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000004C)
// Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000050)
// Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000058)
// Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000005C)
// Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000064)
// Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000068)
// Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000070)
// Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000074)
// Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000007C)
// Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000080)
// Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000088)
// Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000008C)
#if BAREMETAL_RPI_TARGET == 3
// Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W)
#define RPI_GPIO_GPPUD                  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000094)
// Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000098)
// Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000009C)
#else // RPI target 4 or 5
#define RPI_GPIO_GPPINMUXSD             reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000D0)
#define RPI_GPIO_GPPUPPDN0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E4)
#define RPI_GPIO_GPPUPPDN1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E8)
#define RPI_GPIO_GPPUPPDN2              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000EC)
#define RPI_GPIO_GPPUPPDN3              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000F0)
#endif

//---------------------------------------------
// Raspberry Pi UART0
//---------------------------------------------

// Raspberry Pi UART0 registers base address
#define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
// Raspberry Pi UART0 data register (R/W)
#define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
// Raspberry Pi UART0 flag register (R/W)
#define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
// Raspberry Pi UART0 integer baud rate divisor register (R/W)
#define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
// Raspberry Pi UART0 factional baud rate divisor register (R/W)
#define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
// Raspberry Pi UART0 line control register (R/W)
#define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
// Raspberry Pi UART0 control register register (R/W)
#define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
// Raspberry Pi UART0 interrupt FIFO level select register (R/W)
#define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
// Raspberry Pi UART0 interrupt mask set/clear register (R/W)
#define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
// Raspberry Pi UART0 raw interrupt status register (R/W)
#define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
// Raspberry Pi UART0 masked interrupt status  register (R/W)
#define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
// Raspberry Pi UART0 interrupt clear register (R/W)
#define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
// Raspberry Pi UART0 DMA control register (R/W)
#define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)

#define RPI_UART0_FR_RX_READY         BIT(4)
#define RPI_UART0_FR_TX_EMPTY         BIT(5)

//---------------------------------------------
// Raspberry Pi auxiliary (SPI1 / SPI2 / UART1)
//---------------------------------------------

// Raspberry Pi Auxilary registers base address
#define RPI_AUX_BASE                  RPI_BCM_IO_BASE + 0x00215000
// Raspberry Pi Auxiliary IRQ register
#define RPI_AUX_IRQ                   reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
// Raspberry Pi Auxiliary Enable register
#define RPI_AUX_ENABLES               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000004) // AUXENB

//---------------------------------------------
// Raspberry Pi auxilary mini UART registers (UART1)
//---------------------------------------------

// Raspberry Pi Mini UART (UART1) I/O register
#define RPI_AUX_MU_IO                 reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000040)
// Raspberry Pi Mini UART (UART1) Interrupt Enable register
#define RPI_AUX_MU_IER                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000044)
// Raspberry Pi Mini UART (UART1) Interrupt Identify register
#define RPI_AUX_MU_IIR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000048)
// Raspberry Pi Mini UART (UART1) Line Control register
#define RPI_AUX_MU_LCR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000004C)
// Raspberry Pi Mini UART (UART1) Modem Control register
#define RPI_AUX_MU_MCR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000050)
// Raspberry Pi Mini UART (UART1) Line Status register
#define RPI_AUX_MU_LSR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000054)
// Raspberry Pi Mini UART (UART1) Modem Status register
#define RPI_AUX_MU_MSR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000058)
// Raspberry Pi Mini UART (UART1) Scratch register
#define RPI_AUX_MU_SCRATCH            reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000005C)
// Raspberry Pi Mini UART (UART1) Extra Control register
#define RPI_AUX_MU_CNTL               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000060)
// Raspberry Pi Mini UART (UART1) Extra Status register
#define RPI_AUX_MU_STAT               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000064)
// Raspberry Pi Mini UART (UART1) Baudrate register
#define RPI_AUX_MU_BAUD               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000068)

// Raspberry Pi Mini UART (UART1) Interrupt Enable register values
// Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT(1)
// Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT(0)

// Raspberry Pi Mini UART (UART1) Interrupt Identify register values
// Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R)
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT(7)
// Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R)
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT(6)
// Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W)
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT(2)
// Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W)
#define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT(1)

// Raspberry Pi Mini UART (UART1) Line Control register values
// Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_7    0
// Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT(0) | BIT(1)

// Raspberry Pi Mini UART (UART1) Modem Control register values
// Raspberry Pi Mini UART (UART1) Modem Control register set RTS low
#define RPI_AUX_MU_MCR_RTS_LOW        BIT(1)
// Raspberry Pi Mini UART (UART1) Modem Control register set RTS high
#define RPI_AUX_MU_MCR_RTS_HIGH       0

// Raspberry Pi Mini UART (UART1) Line Status register values
// Raspberry Pi Mini UART (UART1) Line Status register transmit idle
#define RPI_AUX_MU_LST_TX_IDLE        BIT(6)
// Raspberry Pi Mini UART (UART1) Line Status register transmit empty
#define RPI_AUX_MU_LST_TX_EMPTY       BIT(5)
// Raspberry Pi Mini UART (UART1) Line Status register receive overrun
#define RPI_AUX_MU_LST_RX_OVERRUN     BIT(1)
// Raspberry Pi Mini UART (UART1) Line Status register receive ready
#define RPI_AUX_MU_LST_RX_READY       BIT(0)

// Raspberry Pi Mini UART (UART1) Extra Control register values
// Raspberry Pi Mini UART (UART1) Extra Control register enable CTS
#define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT(3)
// Raspberry Pi Mini UART (UART1) Extra Control register enable RTS
#define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT(2)
// Raspberry Pi Mini UART (UART1) Extra Control register enable transmit
#define RPI_AUX_MU_CNTL_ENABLE_TX     BIT(1)
// Raspberry Pi Mini UART (UART1) Extra Control register enable receive
#define RPI_AUX_MU_CNTL_ENABLE_RX     BIT(0)
