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

#include <stdlib/Macros.h>
#ifdef __cplusplus
#include <stdlib/Types.h>
#endif

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
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 4
#define RPI_BCM_IO_BASE                 0xFE000000
#define ARM_IO_BASE                     0xFF840000
#else
/// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 5
#define RPI_BCM_IO_BASE                 0x107C000000UL
#define ARM_IO_BASE                     0x107C000000UL
#endif
#if BAREMETAL_RPI_TARGET <= 4
/// @brief End address for Raspberry PI 3 / 4 BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)
#else
/// @brief End address for Raspberry PI 5 BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0x3FFFFFF)
#endif

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
// Interrupt Controller
//---------------------------------------------

#if BAREMETAL_RPI_TARGET == 3

//---------------------------------------------
// RPI 3 Interrupt Controller
//---------------------------------------------

/// @brief Raspberry Pi Interrupt Control Registers base address. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
#define RPI_INTRCTRL_BASE               RPI_BCM_IO_BASE + 0x0000B000

/// @brief Raspberry Pi Interrupt Control basic IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_IRQ_BASIC_PENDING  reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000200)
/// @brief Raspberry Pi Interrupt Control register 1 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_IRQ_PENDING_1      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000204)
/// @brief Raspberry Pi Interrupt Control register 2 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_IRQ_PENDING_2      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000208)
/// @brief Raspberry Pi Interrupt Control FIQ enable register. See @ref RASPBERRY_PI_MAILBOX
#ifdef __cplusplus
#define RPI_INTRCTRL_FIQ_CONTROL        reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x0000020C)
#else
#define RPI_INTRCTRL_FIQ_CONTROL        (RPI_INTRCTRL_BASE + 0x0000020C)
#endif
/// @brief Raspberry Pi Interrupt Control register 1 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_ENABLE_IRQS_1      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000210)
/// @brief Raspberry Pi Interrupt Control register 2 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_ENABLE_IRQS_2      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000214)
/// @brief Raspberry Pi Interrupt Control basic IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_ENABLE_BASIC_IRQS  reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000218)
/// @brief Raspberry Pi Interrupt Control register 1 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_DISABLE_IRQS_1     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x0000021C)
/// @brief Raspberry Pi Interrupt Control register 2 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_DISABLE_IRQS_2     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000220)
/// @brief Raspberry Pi Interrupt Control basic IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
#define RPI_INTRCTRL_DISABLE_BASIC_IRQS reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000224)

#else
//---------------------------------------------
// RPI 4 / 5 Interrupt Controller (GIC-400)
//---------------------------------------------

#if BAREMETAL_RPI_TARGET == 4
#define RPI_GIC_BASE                    ARM_IO_BASE
#else
#define RPI_GIC_BASE                    (ARM_IO_BASE + 0x3FF8000UL)
#endif
#define RPI_GICD_BASE                   (RPI_GIC_BASE + 0x00001000)
#define RPI_GICC_BASE                   (RPI_GIC_BASE + 0x00002000)
#define RPI_GIC_END                     (RPI_GIC_BASE + 0x00007FFF)

// The following definitions are valid for non-secure access,
// if not labeled otherwise.

// GIC distributor registers
#define RPI_GICD_CTLR                   reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0000)
#define RPI_GICD_CTLR_DISABLE           (0 << 0)
#define RPI_GICD_CTLR_ENABLE            (1 << 0)
// secure access
#define RPI_GICD_CTLR_ENABLE_GROUP0     (1 << 0)
#define RPI_GICD_CTLR_ENABLE_GROUP1     (1 << 1)
#define RPI_GICD_IGROUPR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0080)   // Secure access for group 0
#define RPI_GICD_ISENABLER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0100)
#define RPI_GICD_ICENABLER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0180)
#define RPI_GICD_ISPENDR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0200)
#define RPI_GICD_ICPENDR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0280)
#define RPI_GICD_ISACTIVER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0300)
#define RPI_GICD_ICACTIVER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0380)
#define RPI_GICD_IPRIORITYR0            reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0400)
#define RPI_GICD_IPRIORITYR_DEFAULT     0xA0
#define RPI_GICD_IPRIORITYR_FIQ         0x40
#define RPI_GICD_ITARGETSR0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0800)
#define RPI_GICD_ITARGETSR_CORE0        (1 << 0)
#define RPI_GICD_ICFGR0                 reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0C00)
#define RPI_GICD_ICFGR_LEVEL_SENSITIVE  (0 << 1)
#define RPI_GICD_ICFGR_EDGE_TRIGGERED   (1 << 1)
#define RPI_GICD_SGIR                   reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0F00)
#define RPI_GICD_SGIR_SGIINTID__MASK    0x0F
#define RPI_GICD_SGIR_CPU_TARGET_LIST__SHIFT    16
#define RPI_GICD_SGIR_TARGET_LIST_FILTER__SHIFT 24

// GIC CPU interface registers
#define RPI_GICC_CTLR                   reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0000)
#define RPI_GICC_CTLR_DISABLE           (0 << 0)
#define RPI_GICC_CTLR_ENABLE            (1 << 0)
// secure access
#define RPI_GICC_CTLR_ENABLE_GROUP0     (1 << 0)
#define RPI_GICC_CTLR_ENABLE_GROUP1     (1 << 1)
#define RPI_GICC_CTLR_FIQ_ENABLE        (1 << 3)
#define RPI_GICC_PMR                    reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0004)
#define RPI_GICC_PMR_PRIORITY           (0xF0 << 0)
#define RPI_GICC_IAR                    reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x000C)
#define RPI_GICC_IAR_INTERRUPT_ID_MASK  0x3FF
#define RPI_GICC_IAR_CPUID_SHIFT        10
#define RPI_GICC_IAR_CPUID_MASK         (3 << RPI_GICC_IAR_CPUID_SHIFT)
#define RPI_GICC_EOIR                   reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0010)
#define RPI_GICC_EOIR_EOIINTID_MASK     0x3FF
#define RPI_GICC_EOIR_CPUID_SHIFT       10
#define RPI_GICC_EOIR_CPUID_MASK        (3 << RPI_GICC_EOIR_CPUID_SHIFT)

#ifdef __cplusplus
#define RPI_INTRCTRL_FIQ_CONTROL        reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000020C)
#else
#define RPI_INTRCTRL_FIQ_CONTROL        (RPI_MAILBOX_BASE + 0x0000020C)
#endif

#endif

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
#define RPI_MAILBOX_RESPONSE_SUCCESS    BIT1(31)
/// @brief Raspberry Pi Mailbox buffer requestCode failure value
#define RPI_MAILBOX_RESPONSE_ERROR      BIT1(31) | BIT1(0)
/// @brief Raspberry Pi Mailbox buffer property tag response bit
#define RPI_MAILBOX_TAG_RESPONSE        BIT1(31)
/// @brief Raspberry Pi Mailbox 0 (incoming) Status register empty bit
#define RPI_MAILBOX_STATUS_EMPTY        BIT1(30)
/// @brief Raspberry Pi Mailbox 1 (outgoing) Status register full bit. See @ref RASPBERRY_PI_MAILBOX
#define RPI_MAILBOX_STATUS_FULL         BIT1(31)
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
#define RPI_PWRMGT_RSTC_REBOOT          BIT1(5)
/// @brief Raspberry Pi Power management watchdog timer reset code
#define RPI_PWRMGT_RSTC_RESET           0x00000102
/// @brief Raspberry Pi Power management partition bit clear mask for reset sector register. Sector number is a combination of bits 0, 2, 4, 6, 8 and 10, Sector 63 is a special case forcing a halt
#define RPI_PWRMGT_RSTS_PARTITION_CLEAR 0xFFFFFAAA
/// @brief Convert partition to register value. Partition value bits are interspersed with 0 bits
#define RPI_PARTITIONVALUE(x)           (((x) >> 0) & 0x01)  << 0 | (((x) >> 1) & 0x01) << 2 |  (((x) >> 2) & 0x01) << 4 |  (((x) >> 3) & 0x01) << 6 |  (((x) >> 4) & 0x01) << 8 |  (((x) >> 5) & 0x01) << 10

//---------------------------------------------
// Raspberry Pi GPIO
//---------------------------------------------

/// @brief Raspberry Pi GPIO registers base address. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL0_OFFSET         0x00000000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL0_OFFSET)
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL1_OFFSET         0x00000004
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL1_OFFSET)
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL2_OFFSET         0x00000008
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL2                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL2_OFFSET)
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL3_OFFSET         0x0000000C
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL3                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL3_OFFSET)
/// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL4_OFFSET         0x00000010
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 40..49) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL4                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL4_OFFSET)
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL5_OFFSET         0x00000014
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..59) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFSEL5                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL5_OFFSET)
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET0_OFFSET          0x0000001C
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPSET0_OFFSET)
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET1_OFFSET          0x00000020
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPSET1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPSET1_OFFSET)
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR0_OFFSET          0x00000028
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPCLR0_OFFSET)
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR1_OFFSET          0x0000002C
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPCLR1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPCLR1_OFFSET)
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV0_OFFSET          0x00000034
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEV0_OFFSET)
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV1_OFFSET          0x00000038
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEV1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEV1_OFFSET)
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS0_OFFSET          0x00000040
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPEDS0_OFFSET)
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS1_OFFSET          0x00000044
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPEDS1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPEDS1_OFFSET)
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN0_OFFSET          0x0000004C
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPREN0_OFFSET)
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN1_OFFSET          0x00000050
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPREN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPREN1_OFFSET)
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN0_OFFSET          0x00000058
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFEN0_OFFSET)
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN1_OFFSET          0x0000005C
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPFEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFEN1_OFFSET)
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN0_OFFSET          0x00000064
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPHEN0_OFFSET)
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN1_OFFSET          0x00000068
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPHEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPHEN1_OFFSET)
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN0_OFFSET          0x00000070
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEN0_OFFSET)
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN1_OFFSET          0x00000074
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPLEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEN1_OFFSET)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN0_OFFSET         0x0000007C
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAREN0_OFFSET)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN1_OFFSET         0x00000080
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAREN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAREN1_OFFSET)
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN0_OFFSET         0x00000088
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAFEN0_OFFSET)
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN1_OFFSET         0x0000008C
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPAFEN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAFEN1_OFFSET)
#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUD_OFFSET           0x00000094
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUD                  reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUD_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK0_OFFSET       0x00000098
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUDCLK0_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK1_OFFSET       0x0000009C
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUDCLK1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUDCLK1_OFFSET)
#else // RPI target 4 or 5
/// @brief Raspberry Pi GPIO pull up/down pin multiplexer register offset. Undocumented
#define RPI_GPIO_GPPINMUXSD_OFFSET      0x000000D0
/// @brief Raspberry Pi GPIO pull up/down pin multiplexer register. Undocumented
#define RPI_GPIO_GPPINMUXSD             reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPINMUXSD_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN0_OFFSET       0x000000E4
/// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN0              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN0_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN1_OFFSET       0x000000E8
/// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN1              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN1_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN2_OFFSET       0x000000EC
/// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN2              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN2_OFFSET)
/// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN3_OFFSET       0x000000F0
/// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
#define RPI_GPIO_GPPUPPDN3              reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN3_OFFSET)
#endif
/// @brief End of GPIO register region
#define RPI_GPIO_END                    RPI_GPIO_BASE + 0x00000100

//---------------------------------------------
// Raspberry Pi UART0
//---------------------------------------------

/// @brief Raspberry Pi UART0 registers base address. See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
/// @brief Raspberry Pi UART0 data register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
/// @brief Raspberry Pi UART0 flag register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
/// @brief Raspberry Pi UART0 integer baud rate divisor register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
/// @brief Raspberry Pi UART0 factional baud rate divisor register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
/// @brief Raspberry Pi UART0 line control register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
/// @brief Raspberry Pi UART0 control register register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
/// @brief Raspberry Pi UART0 interrupt FIFO level select register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
/// @brief Raspberry Pi UART0 interrupt mask set/clear register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
/// @brief Raspberry Pi UART0 raw interrupt status register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
/// @brief Raspberry Pi UART0 masked interrupt status  register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
/// @brief Raspberry Pi UART0 interrupt clear register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
/// @brief Raspberry Pi UART0 DMA control register (R/W). See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)

/// @brief Raspberry Pi UART0 flag register values
/// @brief Raspberry Pi UART0 flag register Receive data ready bit. See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_FR_RX_READY         BIT1(4)
/// @brief Raspberry Pi UART0 flag register Transmit data empty bit. See @ref RASPBERRY_PI_PL011_UART
#define RPI_UART0_FR_TX_EMPTY         BIT1(5)

//---------------------------------------------
// Raspberry Pi I2C
//---------------------------------------------

/// @brief Raspberry Pi I2C bus 0 registers base address.
#define RPI_I2C0_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00205000)
/// @brief Raspberry Pi I2C bus 1 registers base address.
#define RPI_I2C1_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00804000)

#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi I2C bus 2 registers base address.
#define RPI_I2C2_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00805000)
#elif BAREMETAL_RPI_TARGET == 4
/// @brief Raspberry Pi I2C bus 3 registers base address.
#define RPI_I2C3_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00205600)
/// @brief Raspberry Pi I2C bus 4 registers base address.
#define RPI_I2C4_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00205800)
/// @brief Raspberry Pi I2C bus 5 registers base address.
#define RPI_I2C5_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00205A80)
/// @brief Raspberry Pi I2C bus 6 registers base address.
#define RPI_I2C6_BASE                 reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00205C00)
#endif

/// @brief Raspberry Pi I2C control register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_C_OFFSET              0x00000000
/// @brief Disable BSC (I2C) controller
#define RPI_I2C_C_DISABLE             BIT0(15)
/// @brief Enable BSC (I2C) controller
#define RPI_I2C_C_ENABLE              BIT1(15)
/// @brief Disable interrupt on receive 3/4 full
#define RPI_I2C_C_INTR_DISABLE        BIT0(10)
/// @brief Enable interrupt on receive 3/4 full
#define RPI_I2C_C_INTR_ENABLE         BIT1(10)
/// @brief Disable interrupt on transmit 3/4 empty
#define RPI_I2C_C_INTT_DISABLE        BIT0(9)
/// @brief Enable interrupt on receive 3/4 empty
#define RPI_I2C_C_INTT_ENABLE         BIT1(9)
/// @brief Disable interrupt on transfer done
#define RPI_I2C_C_INTD_DISABLE        BIT0(8)
/// @brief Enable interrupt on transfer done
#define RPI_I2C_C_INTD_ENABLE         BIT1(8)
/// @brief Transfer start (start condition)
#define RPI_I2C_C_ST                  BIT1(7)
/// @brief Clear FIFO
#define RPI_I2C_C_CLEAR               BITS(4, 5)
/// @brief Write transfer
#define RPI_I2C_C_WRITE               BIT0(0)
/// @brief Read transfer
#define RPI_I2C_C_READ                BIT1(0)

/// @brief Raspberry Pi I2C status register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_S_OFFSET              0x00000004
/// @brief Clock stretch timeout
#define RPI_I2C_S_CLKT                BIT1(9)
/// @brief ACK error
#define RPI_I2C_S_ERR                 BIT1(8)
/// @brief Receive FIFO full
#define RPI_I2C_S_RXF                 BIT1(7)
/// @brief Transmit FIFO empty
#define RPI_I2C_S_TXE                 BIT1(6)
/// @brief Receive FIFO has data
#define RPI_I2C_S_RXD                 BIT1(5)
/// @brief Transmit FIFO has space
#define RPI_I2C_S_TXD                 BIT1(4)
/// @brief Receive FIFO is 3/4 full
#define RPI_I2C_S_RXR                 BIT1(3)
/// @brief Transmit FIFO is 1/4 full
#define RPI_I2C_S_TXW                 BIT1(2)
/// @brief Transfer is done
#define RPI_I2C_S_DONE                BIT1(1)
/// @brief Transfer is active
#define RPI_I2C_S_TA                  BIT1(0)

/// /// @brief Raspberry Pi I2C data length register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_DLEN_OFFSET           0x00000008
/// @brief Raspberry Pi I2C slave address register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_A_OFFSET              0x0000000C
/// @brief Raspberry Pi I2C data FIFO register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_FIFO_OFFSET           0x00000010
/// @brief Raspberry Pi I2C clock divider register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_DIV_OFFSET            0x00000014
/// @brief Raspberry Pi I2C data delay register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_DEL_OFFSET            0x00000018
/// @brief Raspberry Pi I2C clock stretch timeout register (R/W) offset relative to RPI_I2Cx_BASE
#define RPI_I2C_CLKT_OFFSET           0x0000001C
/// @brief Raspberry Pi I2C register address from base address and offset
#define RPI_I2C_REG_ADDRESS(base, offset)   reinterpret_cast<regaddr>((base) + (offset))

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
#define RPI_AUX_ENABLES_SPI2          BIT1(2)
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_SPI1          BIT1(1)
/// @brief Raspberry Pi Auxiliary Enable register Enable UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
#define RPI_AUX_ENABLES_UART1         BIT1(0)

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
#if BAREMETAL_RPI_TARGET == 3
/// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 3
#define AUX_UART_CLOCK 250000000
#else
/// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 4
#define AUX_UART_CLOCK 500000000
#endif
/// @brief Calculate Raspberry Pi Mini UART (UART1) baud rate value from frequency
#define RPI_AUX_MU_BAUD_VALUE(baud)   static_cast<uint32>((AUX_UART_CLOCK / (baud * 8)) - 1)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT1(1)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts. See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT1(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT1(7)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT1(6)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W). See @ref RASPBERRY_PI_UART1
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT1(2)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref RASPBERRY_PI_UART1
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
