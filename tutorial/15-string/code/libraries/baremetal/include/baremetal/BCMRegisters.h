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

#include <baremetal/Macros.h>
#include <baremetal/Types.h>

/// @brief Address of GPU memory accessible from ARM, mapped as cached memory
#define GPU_CACHED_BASE                 0x40000000
/// @brief Address of GPU memory accessible from ARM, mapped as uncached memory
#define GPU_UNCACHED_BASE               0xC0000000

/// @brief User base address of GPU memory from ARM
#define GPU_MEM_BASE                    GPU_UNCACHED_BASE

/// @brief Convert ARM address to GPU bus address (also works for aliases)
#define ARM_TO_GPU(addr)                (((addr) & ~0xC0000000) | GPU_MEM_BASE)
/// @brief Convert GPU bus address to ARM address (also works for aliases)
#define GPU_TO_ARM(addr)                ((addr) & ~0xC0000000)

#if BAREMETAL_RPI_TARGET == 3
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 3
#define RPI_BCM_IO_BASE                 0x3F000000
#else
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 4 and 5
#define RPI_BCM_IO_BASE                 0xFE000000
#endif
/// @brief End address for Raspberry PI BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)

//---------------------------------------------
// Raspberry Pi System Timer
//---------------------------------------------

/// @brief Raspberry Pi System Timer Registers base address. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_BASE                 RPI_BCM_IO_BASE + 0x00003000
/// @brief System Timer Control / Status register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_CS                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000000)
/// @brief System Timer Counter Lower 32 bits register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_LO                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000004)
/// @brief System Timer Counter Higher 32 bits register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_HI                   reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000008)
/// @brief System Timer Compare 0 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_CMP0                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x0000000C)
/// @brief System Timer Compare 1 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_CMP1                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000010)
/// @brief System Timer Compare 2 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_CMP2                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000014)
/// @brief System Timer Compare 3 register. See @ref RASPBERRY_PI_SYSTEM_TIMER
#define RPI_SYSTMR_CMP3                 reinterpret_cast<regaddr>(RPI_SYSTMR_BASE + 0x00000018)

//---------------------------------------------
// Raspberry Pi Mailbox
//---------------------------------------------

/// @brief Raspberry Pi Mailbox Registers base address. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_BASE                RPI_BCM_IO_BASE + 0x0000B880
/// @brief Raspberry Pi Mailbox 0 (incoming) Read register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX0_READ               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000000)
/// @brief Raspberry Pi Mailbox 0 (incoming) Poll register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX0_POLL               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000010)
/// @brief Raspberry Pi Mailbox 0 (incoming) Sender register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX0_SENDER             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000014)
/// @brief Raspberry Pi Mailbox 0 (incoming) Status register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX0_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000018)
/// @brief Raspberry Pi Mailbox 0 (incoming) Configuration register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_CONFIG              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000001C)
/// @brief Raspberry Pi Mailbox 1 (outgoing) Write register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX1_WRITE              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000020)
/// @brief Raspberry Pi Mailbox 1 (incoming) Poll register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX1_POLL               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000030)
/// @brief Raspberry Pi Mailbox 1 (incoming) Sender register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX1_SENDER             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000034)
/// @brief Raspberry Pi Mailbox 1 (outgoing) Status register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX1_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000038)
/// @brief Raspberry Pi Mailbox 1 (outgoing) Configuration register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX1_CONFIG             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000003C)
/// @brief Raspberry Pi Mailbox buffer requestCode success value. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_RESPONSE_SUCCESS    BIT(31)
/// @brief Raspberry Pi Mailbox buffer requestCode failure value
#define RPI_MAILBOX_RESPONSE_ERROR      BIT(31) | BIT(0)
/// @brief Raspberry Pi Mailbox buffer property tag response bit
#define RPI_MAILBOX_TAG_RESPONSE        BIT(31)
/// @brief Raspberry Pi Mailbox 0 (incoming) Status register empty bit
#define RPI_MAILBOX_STATUS_EMPTY        BIT(30)
/// @brief Raspberry Pi Mailbox 1 (outgoing) Status register full bit. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_STATUS_FULL         BIT(31)
/// @brief Raspberry Pi Mailbox buffer requestCode value for request. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_REQUEST             0

//---------------------------------------------
// Raspberry Pi Power Management
//---------------------------------------------

// The power management features of Raspberry Pi are not well documented.
// Most information is extracted from the source code of an old BCM2835 watchdog timer
// A good reference is https://elixir.bootlin.com/linux/latest/source/drivers/watchdog/bcm2835_wdt.c or
// https://github.com/torvalds/linux/blob/master/drivers/watchdog/bcm2835_wdt.c

/// @brief Raspberry Pi Power management Registers base address. See @ref RASPBERRY_PI_POWER_MANAGEMENT
#define RPI_PWRMGT_BASE                 RPI_BCM_IO_BASE + 0x00100000
/// @brief Raspberry Pi Power management reset control register
#define RPI_PWRMGT_RSTC                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x0000001C)
/// @brief Raspberry Pi Power management reset sector register
#define RPI_PWRMGT_RSTS                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000020)
/// @brief Raspberry Pi Power management watchdog register
#define RPI_PWRMGT_WDOG                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000024)
/// @brief Raspberry Pi Power management magic number, to be ORed with value when setting register values
#define RPI_PWRMGT_WDOG_MAGIC           0x5A000000
/// @brief Raspberry Pi Power management clear mask.
#define RPI_PWRMGT_RSTC_CLEAR           0xFFFFFFCF
/// @brief Raspberry Pi Power management full reset bit. Can be used to check if watchdog timer is still running
#define RPI_PWRMGT_RSTC_REBOOT          BIT(5)
/// @brief Raspberry Pi Power management watchdog timer reset code
#define RPI_PWRMGT_RSTC_RESET           0x00000102
/// @brief Raspberry Pi Power management partition bit clear mask for reset sector register. Sector number is a combination of bits 0, 2, 4, 6, 8 and 10, Sector 63 is a special case forcing a halt
#define RPI_PWRMGT_RSTS_PARTITION_CLEAR 0xFFFFFAAA
// @brief Convert partition to register value. Partition value bits are interspersed with 0 bits
#define RPI_PARTITIONVALUE(x)           (((x) >> 0) & 0x01)  << 0 | (((x) >> 1) & 0x01) << 2 |  (((x) >> 2) & 0x01) << 4 |  (((x) >> 3) & 0x01) << 6 |  (((x) >> 4) & 0x01) << 8 |  (((x) >> 5) & 0x01) << 10

//---------------------------------------------
// Raspberry Pi GPIO
//---------------------------------------------

/// @brief Raspberry Pi GPIO registers base address. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000000)
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000004)
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL2                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000008)
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL3                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000000C)
/// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL4                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000010)
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL5                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000014)
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000001C)
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000020)
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000028)
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000002C)
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000034)
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000038)
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000040)
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000044)
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000004C)
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000050)
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000058)
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000005C)
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000064)
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000068)
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000070)
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000074)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000007C)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000080)
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000088)
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000008C)
#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUD                  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000094)
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000098)
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000009C)
#else // RPI target 4 or 5
/// @brief Raspberry Pi GPIO pull up/down pin multiplexer register. Undocumented
#define RPI_GPIO_GPPINMUXSD             reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000D0)
/// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E4)
/// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E8)
/// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN2              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000EC)
/// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN3              reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000F0)
#endif

//---------------------------------------------
// Raspberry Pi UART0
//---------------------------------------------

/// @brief Raspberry Pi UART0 registers base address. See @ref RASPBERRY_PI_UART0
#define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
/// @brief Raspberry Pi UART0 data register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
/// @brief Raspberry Pi UART0 flag register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
/// @brief Raspberry Pi UART0 integer baud rate divisor register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
/// @brief Raspberry Pi UART0 factional baud rate divisor register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
/// @brief Raspberry Pi UART0 line control register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
/// @brief Raspberry Pi UART0 control register register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
/// @brief Raspberry Pi UART0 interrupt FIFO level select register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
/// @brief Raspberry Pi UART0 interrupt mask set/clear register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
/// @brief Raspberry Pi UART0 raw interrupt status register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
/// @brief Raspberry Pi UART0 masked interrupt status  register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
/// @brief Raspberry Pi UART0 interrupt clear register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
/// @brief Raspberry Pi UART0 DMA control register (R/W). See @ref RASPBERRY_PI_UART0
#define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)

/// @brief Raspberry Pi UART0 flag register values
/// @brief Raspberry Pi UART0 flag register Receive data ready bit. See @ref RASPBERRY_PI_UART0
#define RPI_UART0_FR_RX_READY         BIT(4)
/// @brief Raspberry Pi UART0 flag register Transmit data empty bit. See @ref RASPBERRY_PI_UART0
#define RPI_UART0_FR_TX_EMPTY         BIT(5)

//---------------------------------------------
// Raspberry Pi auxiliary (SPI1 / SPI2 / UART1)
//---------------------------------------------

/// @brief Raspberry Pi Auxilary registers base address. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_BASE                  RPI_BCM_IO_BASE + 0x00215000
/// @brief Raspberry Pi Auxiliary IRQ register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_IRQ                   reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
/// @brief Raspberry Pi Auxiliary Enable register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000004) // AUXENB

/// @brief Raspberry Pi Auxiliary Enable register values
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI2. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_SPI2          BIT(2)
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_SPI1          BIT(1)
/// @brief Raspberry Pi Auxiliary Enable register Enable UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_UART1         BIT(0)

//---------------------------------------------
// Raspberry Pi auxiliary mini UART (UART1)
//---------------------------------------------

/// @brief Raspberry Pi Mini UART (UART1) I/O register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IO                 reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000040)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000044)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000048)
/// @brief Raspberry Pi Mini UART (UART1) Line Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000004C)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000050)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LSR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000054)
/// @brief Raspberry Pi Mini UART (UART1) Modem Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MSR                reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000058)
/// @brief Raspberry Pi Mini UART (UART1) Scratch register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_SCRATCH            reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000005C)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000060)
/// @brief Raspberry Pi Mini UART (UART1) Extra Status register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_STAT               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000064)
/// @brief Raspberry Pi Mini UART (UART1) Baudrate register. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_BAUD               reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000068)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT(7)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Line Control register values
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR_DATA_SIZE_7    0
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT(0) | BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR_RTS_LOW        BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_MCR_RTS_HIGH       0

/// @brief Raspberry Pi Mini UART (UART1) Line Status register values
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LST_TX_IDLE        BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LST_TX_EMPTY       BIT(5)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LST_RX_OVERRUN     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_LST_RX_READY       BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT(3)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_TX     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_CNTL_ENABLE_RX     BIT(0)
