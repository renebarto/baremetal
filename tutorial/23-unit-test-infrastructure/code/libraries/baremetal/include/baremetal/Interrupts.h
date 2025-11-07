//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Interrupts.h
//
// Namespace   : baremetal
//
// Class       : Interrupts
//
// Description : Interrupt numbers and enable / disable functionality
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

#include "baremetal/BCMRegisters.h"

namespace baremetal {

#if BAREMETAL_RPI_TARGET == 3

// IRQs
/// @brief Number of IRQ lines per register for regular ARM_IRQ1 and ARM_IRQ2
#define ARM_IRQS_PER_REG   32
/// @brief Number of IRQ lines for ARM_IRQ_BASIC
#define ARM_IRQS_BASIC_REG 8
/// @brief Number of IRQ lines for ARM_IRQ_LOCAL
#define ARM_IRQS_LOCAL_REG 12

/// @brief Number of normal IRQ registers (IRQ1, IRQ2, Basic IRQ)
#define ARM_IRQS_NUM_REGS  3

/// @brief Total number of IRQ lines Raspberry Pi 3
#define IRQ_LINES          (ARM_IRQS_PER_REG * 2 + ARM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)

/// @brief Offset for interrupts in IRQ1 group
#define ARM_IRQ1_BASE      0
/// @brief Offset for interrupts in IRQ2 group
#define ARM_IRQ2_BASE      (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
/// @brief Offset for interrupts in Basic IRQ group
#define ARM_IRQ_BASIC_BASE (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
/// @brief Offset for interrupts in ARM local group
#define ARM_IRQ_LOCAL_BASE (ARM_IRQ_BASIC_BASE + ARM_IRQS_BASIC_REG)
/// @brief Calculate IRQ number for IRQ in IRQ1 group
#define ARM_IRQ1(n)        (ARM_IRQ1_BASE + (n))
/// @brief Calculate IRQ number for IRQ in IRQ2 group
#define ARM_IRQ2(n)        (ARM_IRQ2_BASE + (n))
/// @brief Calculate IRQ number for IRQ in Basic IRQ group
#define ARM_IRQ_BASIC(n)   (ARM_IRQ_BASIC_BASE + (n))
/// @brief Calculate IRQ number for IRQ in ARM local group
#define ARM_IRQ_LOCAL(n)   (ARM_IRQ_LOCAL_BASE + (n))

/// @brief Check whether an IRQ is pending (only for IRQ1, IRQ2, Basic IRQ group)
#define ARM_IC_IRQ_PENDING(irq)                                                                                                                      \
    ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_IRQ_PENDING_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_IRQ_PENDING_2 : RPI_INTRCTRL_IRQ_BASIC_PENDING))
/// @brief Enable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
#define ARM_IC_IRQS_ENABLE(irq)                                                                                                                      \
    ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_ENABLE_IRQS_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_ENABLE_IRQS_2 : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
/// @brief Disable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
#define ARM_IC_IRQS_DISABLE(irq)                                                                                                                     \
    ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_DISABLE_IRQS_1                                                                                             \
                           : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_DISABLE_IRQS_2 : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
/// @brief Calculate bitmask for an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
#define ARM_IRQ_MASK(irq) BIT1((irq) & (ARM_IRQS_PER_REG - 1))

/// <summary>
/// @brief IRQ interrupt numbers
/// </summary>
enum class IRQ_ID
{
    /// @brief BCM timer 0
    IRQ_TIMER0 = ARM_IRQ1(0x00), // bcm2835-system-timer
    /// @brief BCM timer 1
    IRQ_TIMER1 = ARM_IRQ1(0x01),
    /// @brief BCM timer 2
    IRQ_TIMER2 = ARM_IRQ1(0x02),
    /// @brief BCM timer 3
    IRQ_TIMER3 = ARM_IRQ1(0x03),
    /// @brief H.264 codec 0
    IRQ_CODEC0 = ARM_IRQ1(0x04),
    /// @brief H.264 codec 1
    IRQ_CODEC1 = ARM_IRQ1(0x05),
    /// @brief H.264 codec 2
    IRQ_CODEC2 = ARM_IRQ1(0x06),
    /// @brief JPEG
    IRQ_JPEG = ARM_IRQ1(0x07),
    /// @brief ISP
    IRQ_ISP = ARM_IRQ1(0x08),
    /// @brief USB
    IRQ_USB = ARM_IRQ1(0x09), // bcm2708-usb
    /// @brief VideoCore 3D
    IRQ_3D = ARM_IRQ1(0x0A), // vc4-v3d
    /// @brief TXP / Transpose
    IRQ_TRANSPOSER = ARM_IRQ1(0x0B), // bcm2835-txp
    /// @brief Multicore sync 0
    IRQ_MULTICORESYNC0 = ARM_IRQ1(0x0C),
    /// @brief Multicore sync 1
    IRQ_MULTICORESYNC1 = ARM_IRQ1(0x0D),
    /// @brief Multicore sync 2
    IRQ_MULTICORESYNC2 = ARM_IRQ1(0x0E),
    /// @brief Multicore sync 3
    IRQ_MULTICORESYNC3 = ARM_IRQ1(0x0F),
    /// @brief DMA channel 0 interrupt
    IRQ_DMA0 = ARM_IRQ1(0x10),
    /// @brief DMA channel 1 interrupt
    IRQ_DMA1 = ARM_IRQ1(0x11),
    /// @brief DMA channel 2 interrupt, I2S PCM TX
    IRQ_DMA2 = ARM_IRQ1(0x12),
    /// @brief DMA channel 3 interrupt, I2S PCM RX
    IRQ_DMA3 = ARM_IRQ1(0x13),
    /// @brief DMA channel 4 interrupt, SMI
    IRQ_DMA4 = ARM_IRQ1(0x14),
    /// @brief DMA channel 5 interrupt, PWM
    IRQ_DMA5 = ARM_IRQ1(0x15),
    /// @brief DMA channel 6 interrupt, SPI TX
    IRQ_DMA6 = ARM_IRQ1(0x16),
    /// @brief DMA channel 7 interrupt, SPI RX
    IRQ_DMA7 = ARM_IRQ1(0x17),
    /// @brief DMA channel 8 interrupt
    IRQ_DMA8 = ARM_IRQ1(0x18),
    /// @brief DMA channel 9 interrupt
    IRQ_DMA9 = ARM_IRQ1(0x19),
    /// @brief DMA channel 10 interrupt
    IRQ_DMA10 = ARM_IRQ1(0x1A),
    /// @brief DMA channel 11-14 interrupt, EMMC
    IRQ_DMA11 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, UART TX
    IRQ_DMA12 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, undocumented
    IRQ_DMA13 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, UART RX
    IRQ_DMA14 = ARM_IRQ1(0x1B),
    /// @brief DMA channel shared interrupt
    IRQ_DMA_SHARED = ARM_IRQ1(0x1C),
    /// @brief AUX interrupt (UART1, SPI0/1)
    IRQ_AUX = ARM_IRQ1(0x1D),
    /// @brief ARM interrupt
    IRQ_ARM = ARM_IRQ1(0x1E),
    /// @brief VPU interrupt
    IRQ_VPUDMA = ARM_IRQ1(0x1F),

    /// @brief USB Host Port interrupt
    IRQ_HOSTPORT = ARM_IRQ2(0x00),
    /// @brief JPEHVS interruptG
    IRQ_VIDEOSCALER = ARM_IRQ2(0x01),
    /// @brief CCP2TX interrupt
    IRQ_CCP2TX = ARM_IRQ2(0x02),
    /// @brief SDC interrupt
    IRQ_SDC = ARM_IRQ2(0x03),
    /// @brief DSI 0 (display) interrupt
    IRQ_DSI0 = ARM_IRQ2(0x04),
    /// @brief AVE interrupt
    IRQ_AVE = ARM_IRQ2(0x05),
    /// @brief CSI 0 (camera) interrupt
    IRQ_CAM0 = ARM_IRQ2(0x06),
    /// @brief CSI 1 (camera) interrupt
    IRQ_CAM1 = ARM_IRQ2(0x07),
    /// @brief HDMI 0 interrupt
    IRQ_HDMI0 = ARM_IRQ2(0x08),
    /// @brief HDMI 1 interrupt
    IRQ_HDMI1 = ARM_IRQ2(0x09),
    /// @brief GPU pixel valve 2 interrupt
    IRQ_PIXELVALVE2 = ARM_IRQ2(0x0A),
    /// @brief I2C / SPI slave interrupt
    IRQ_I2CSPISLV = ARM_IRQ2(0x0B),
    /// @brief DSI 1 (display) interrupt
    IRQ_DSI1 = ARM_IRQ2(0x0C),
    /// @brief GPU pixel valve 0 interrupt
    IRQ_PIXELVALVE0 = ARM_IRQ2(0x0D),
    /// @brief GPU pixel valve 1 interrupt
    IRQ_PIXELVALVE1 = ARM_IRQ2(0x0E),
    /// @brief CPR interrupt
    IRQ_CPR = ARM_IRQ2(0x0F),
    /// @brief SMI (firmware) interrupt
    IRQ_SMI = ARM_IRQ2(0x10),
    /// @brief GPIO 0 interrupt
    IRQ_GPIO0 = ARM_IRQ2(0x11),
    /// @brief GPIO 1 interrupt
    IRQ_GPIO1 = ARM_IRQ2(0x12),
    /// @brief GPIO 2 interrupt
    IRQ_GPIO2 = ARM_IRQ2(0x13),
    /// @brief GPIO 3 interrupt
    IRQ_GPIO3 = ARM_IRQ2(0x14),
    /// @brief I2C interrupt
    IRQ_I2C = ARM_IRQ2(0x15),
    /// @brief SPI interrupt
    IRQ_SPI = ARM_IRQ2(0x16),
    /// @brief I2S interrupt
    IRQ_I2SPCM = ARM_IRQ2(0x17),
    /// @brief SD host interrupt
    IRQ_SDHOST = ARM_IRQ2(0x18),
    /// @brief PL011 UART interrupt (UART0)
    IRQ_UART = ARM_IRQ2(0x19),
    /// @brief SLIMBUS interrupt
    IRQ_SLIMBUS = ARM_IRQ2(0x1A),
    /// @brief GPU? vector interrupt
    IRQ_VEC = ARM_IRQ2(0x1B),
    /// @brief CPG interrupt
    IRQ_CPG = ARM_IRQ2(0x1C),
    /// @brief RNG (random number generator) interrupt
    IRQ_RNG = ARM_IRQ2(0x1D),
    /// @brief EMMC interrupt
    IRQ_ARASANSDIO = ARM_IRQ2(0x1E),
    /// @brief AVSPMON interrupt
    IRQ_AVSPMON = ARM_IRQ2(0x1F),

    /// @brief ARM timer interrupt
    IRQ_ARM_TIMER = ARM_IRQ_BASIC(0x00),
    /// @brief ARM mailbox interrupt
    IRQ_ARM_MAILBOX = ARM_IRQ_BASIC(0x01),
    /// @brief ARM doorbell (VCHIQ) 0 interrupt
    IRQ_ARM_DOORBELL_0 = ARM_IRQ_BASIC(0x02),
    /// @brief ARM doorbell (VCHIQ) 1 interrupt
    IRQ_ARM_DOORBELL_1 = ARM_IRQ_BASIC(0x03),
    /// @brief VPU halted 0 interrupt
    IRQ_VPU0_HALTED = ARM_IRQ_BASIC(0x04),
    /// @brief VPU halted 1 interrupt
    IRQ_VPU1_HALTED = ARM_IRQ_BASIC(0x05),
    /// @brief Illegal type 0 interrupt
    IRQ_ILLEGAL_TYPE0 = ARM_IRQ_BASIC(0x06),
    /// @brief Illegal type 1 interrupt
    IRQ_ILLEGAL_TYPE1 = ARM_IRQ_BASIC(0x07),

    /// @brief Non secure Physical ARM timer
    IRQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00), // armv7-timer
    /// @brief Secure Physical ARM timer
    IRQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
    /// @brief Hypervisor Physical ARM timer
    IRQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
    /// @brief Virtual ARM timer
    IRQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
    /// @brief Mailbox 0
    IRQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
    /// @brief Mailbox 1
    IRQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
    /// @brief Mailbox 2
    IRQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
    /// @brief Mailbox 3
    IRQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
    /// @brief Cascaded GPU interrupts
    IRQ_LOCAL_GPU = ARM_IRQ_LOCAL(0x08),
    /// @brief Performance Monitoring Unit
    IRQ_LOCAL_PMU = ARM_IRQ_LOCAL(0x09),
    /// @brief AXI bus idle, on core 0 only
    IRQ_LOCAL_AXI_IDLE = ARM_IRQ_LOCAL(0x0A),
    /// @brief Local timer
    IRQ_LOCAL_LOCALTIMER = ARM_IRQ_LOCAL(0x0B),
};

/// <summary>
/// @brief FIQ interrupt numbers
/// </summary>
enum class FIQ_ID
{
    /// @brief BCM timer 0
    FIQ_TIMER0 = ARM_IRQ1(0x00),
    /// @brief BCM timer 1
    FIQ_TIMER1 = ARM_IRQ1(0x01),
    /// @brief BCM timer 2
    FIQ_TIMER2 = ARM_IRQ1(0x02),
    /// @brief BCM timer 3
    FIQ_TIMER3 = ARM_IRQ1(0x03),
    /// @brief H.264 codec 0
    FIQ_CODEC0 = ARM_IRQ1(0x04),
    /// @brief H.264 codec 1
    FIQ_CODEC1 = ARM_IRQ1(0x05),
    /// @brief H.264 codec 2
    FIQ_CODEC2 = ARM_IRQ1(0x06),
    /// @brief JPEG
    FIQ_JPEG = ARM_IRQ1(0x07),
    /// @brief ISP
    FIQ_ISP = ARM_IRQ1(0x08),
    /// @brief USB
    FIQ_USB = ARM_IRQ1(0x09),
    /// @brief VideoCore 3D
    FIQ_3D = ARM_IRQ1(0x0A),
    /// @brief TXP / Transpose
    FIQ_TRANSPOSER = ARM_IRQ1(0x0B),
    /// @brief Multicore sync 0
    FIQ_MULTICORESYNC0 = ARM_IRQ1(0x0C),
    /// @brief Multicore sync 1
    FIQ_MULTICORESYNC1 = ARM_IRQ1(0x0D),
    /// @brief Multicore sync 2
    FIQ_MULTICORESYNC2 = ARM_IRQ1(0x0E),
    /// @brief Multicore sync 3
    FIQ_MULTICORESYNC3 = ARM_IRQ1(0x0F),
    /// @brief DMA channel 0 interrupt
    FIQ_DMA0 = ARM_IRQ1(0x10),
    /// @brief DMA channel 1 interrupt
    FIQ_DMA1 = ARM_IRQ1(0x11),
    /// @brief DMA channel 2 interrupt, I2S PCM TX
    FIQ_DMA2 = ARM_IRQ1(0x12),
    /// @brief DMA channel 3 interrupt, I2S PCM RX
    FIQ_DMA3 = ARM_IRQ1(0x13),
    /// @brief DMA channel 4 interrupt, SMI
    FIQ_DMA4 = ARM_IRQ1(0x14),
    /// @brief DMA channel 5 interrupt, PWM
    FIQ_DMA5 = ARM_IRQ1(0x15),
    /// @brief DMA channel 6 interrupt, SPI TX
    FIQ_DMA6 = ARM_IRQ1(0x16),
    /// @brief DMA channel 7 interrupt, SPI RX
    FIQ_DMA7 = ARM_IRQ1(0x17),
    /// @brief DMA channel 8 interrupt
    FIQ_DMA8 = ARM_IRQ1(0x18),
    /// @brief DMA channel 9 interrupt
    FIQ_DMA9 = ARM_IRQ1(0x19),
    /// @brief DMA channel 10 interrupt
    FIQ_DMA10 = ARM_IRQ1(0x1A),
    /// @brief DMA channel 11-14 interrupt, EMMC
    FIQ_DMA11 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, UART TX
    FIQ_DMA12 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, undocumented
    FIQ_DMA13 = ARM_IRQ1(0x1B),
    /// @brief DMA channel 11-14 interrupt, UART RX
    FIQ_DMA14 = ARM_IRQ1(0x1B),
    /// @brief DMA channel shared interrupt
    FIQ_DMA_SHARED = ARM_IRQ1(0x1C),
    /// @brief AUX interrupt (UART1, SPI0/1)
    FIQ_AUX = ARM_IRQ1(0x1D),
    /// @brief ARM interrupt
    FIQ_ARM = ARM_IRQ1(0x1E),
    /// @brief VPU interrupt
    FIQ_VPUDMA = ARM_IRQ1(0x1F),

    /// @brief USB Host Port interrupt
    FIQ_HOSTPORT = ARM_IRQ2(0x00),
    /// @brief JPEHVS interruptG
    FIQ_VIDEOSCALER = ARM_IRQ2(0x01),
    /// @brief CCP2TX interrupt
    FIQ_CCP2TX = ARM_IRQ2(0x02),
    /// @brief SDC interrupt
    FIQ_SDC = ARM_IRQ2(0x03),
    /// @brief DSI 0 (display) interrupt
    FIQ_DSI0 = ARM_IRQ2(0x04),
    /// @brief AVE interrupt
    FIQ_AVE = ARM_IRQ2(0x05),
    /// @brief CSI 0 (camera) interrupt
    FIQ_CAM0 = ARM_IRQ2(0x06),
    /// @brief CSI 1 (camera) interrupt
    FIQ_CAM1 = ARM_IRQ2(0x07),
    /// @brief HDMI 0 interrupt
    FIQ_HDMI0 = ARM_IRQ2(0x08),
    /// @brief HDMI 1 interrupt
    FIQ_HDMI1 = ARM_IRQ2(0x09),
    /// @brief GPU pixel valve 2 interrupt
    FIQ_PIXELVALVE2 = ARM_IRQ2(0x0A),
    /// @brief I2C / SPI slave interrupt
    FIQ_I2CSPISLV = ARM_IRQ2(0x0B),
    /// @brief DSI 1 (display) interrupt
    FIQ_DSI1 = ARM_IRQ2(0x0C),
    /// @brief GPU pixel valve 0 interrupt
    FIQ_PIXELVALVE0 = ARM_IRQ2(0x0D),
    /// @brief GPU pixel valve 1 interrupt
    FIQ_PIXELVALVE1 = ARM_IRQ2(0x0E),
    /// @brief CPR interrupt
    FIQ_CPR = ARM_IRQ2(0x0F),
    /// @brief SMI (firmware) interrupt
    FIQ_SMI = ARM_IRQ2(0x10),
    /// @brief GPIO 0 interrupt
    FIQ_GPIO0 = ARM_IRQ2(0x11),
    /// @brief GPIO 1 interrupt
    FIQ_GPIO1 = ARM_IRQ2(0x12),
    /// @brief GPIO 2 interrupt
    FIQ_GPIO2 = ARM_IRQ2(0x13),
    /// @brief GPIO 3 interrupt
    FIQ_GPIO3 = ARM_IRQ2(0x14),
    /// @brief I2C interrupt
    FIQ_I2C = ARM_IRQ2(0x15),
    /// @brief SPI interrupt
    FIQ_SPI = ARM_IRQ2(0x16),
    /// @brief I2S interrupt
    FIQ_I2SPCM = ARM_IRQ2(0x17),
    /// @brief SD host interrupt
    FIQ_SDHOST = ARM_IRQ2(0x18),
    /// @brief PL011 UART interrupt (UART0)
    FIQ_UART = ARM_IRQ2(0x19),
    /// @brief SLIMBUS interrupt
    FIQ_SLIMBUS = ARM_IRQ2(0x1A),
    /// @brief GPU? vector interrupt
    FIQ_VEC = ARM_IRQ2(0x1B),
    /// @brief CPG interrupt
    FIQ_CPG = ARM_IRQ2(0x1C),
    /// @brief RNG (random number generator) interrupt
    FIQ_RNG = ARM_IRQ2(0x1D),
    /// @brief EMMC interrupt
    FIQ_ARASANSDIO = ARM_IRQ2(0x1E),
    /// @brief AVSPMON interrupt
    FIQ_AVSPMON = ARM_IRQ2(0x1F),

    /// @brief ARM timer interrupt
    FIQ_ARM_TIMER = ARM_IRQ_BASIC(0x00),
    /// @brief ARM mailbox interrupt
    FIQ_ARM_MAILBOX = ARM_IRQ_BASIC(0x01),
    /// @brief ARM doorbell (VCHIQ) 0 interrupt
    FIQ_ARM_DOORBELL_0 = ARM_IRQ_BASIC(0x02),
    /// @brief ARM doorbell (VCHIQ) 1 interrupt
    FIQ_ARM_DOORBELL_1 = ARM_IRQ_BASIC(0x03),
    /// @brief VPU halted 0 interrupt
    FIQ_VPU0_HALTED = ARM_IRQ_BASIC(0x04),
    /// @brief VPU halted 1 interrupt
    FIQ_VPU1_HALTED = ARM_IRQ_BASIC(0x05),
    /// @brief Illegal type 0 interrupt
    FIQ_ILLEGAL_TYPE0 = ARM_IRQ_BASIC(0x06),
    /// @brief Illegal type 1 interrupt
    FIQ_ILLEGAL_TYPE1 = ARM_IRQ_BASIC(0x07),

    /// @brief Secure Physical ARM timer
    FIQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00),
    /// @brief Non secure Physical ARM timer
    FIQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
    /// @brief Hypervisor Physical ARM timer
    FIQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
    /// @brief Virtual ARM timer
    FIQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
    /// @brief Mailbox 0
    FIQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
    /// @brief Mailbox 1
    FIQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
    /// @brief Mailbox 2
    FIQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
    /// @brief Mailbox 3
    FIQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
    /// @brief Cascaded GPU interrupts
    FIQ_LOCAL_GPU = ARM_IRQ_LOCAL(0x08),
    /// @brief Performance Monitoring Unit
    FIQ_LOCAL_PMU = ARM_IRQ_LOCAL(0x09),
    /// @brief AXI bus idle, on core 0 only
    FIQ_LOCAL_AXI_IDLE = ARM_IRQ_LOCAL(0x0A),
    /// @brief Local timer
    FIQ_LOCAL_LOCALTIMER = ARM_IRQ_LOCAL(0x0B),
};

#else

// IRQs
/// @brief Software generated interrupt, per core
#define GIC_SGI(n) (0 + (n))
/// @brief Private peripheral interrupt, per core
#define GIC_PPI(n) (16 + (n))
/// @brief Shared peripheral interrupt, shared between cores
#define GIC_SPI(n) (32 + (n))

/// @brief Total number of IRQ lines Raspberry Pi 4 and 5
#define IRQ_LINES  256

/// <summary>
/// @brief IRQ interrupt numbers
/// </summary>
enum class IRQ_ID
{
    /// @brief Hypervisor Physical ARM timer
    IRQ_LOCAL_CNTHP = GIC_PPI(0x0A),
    /// @brief Virtual ARM timer
    IRQ_LOCAL_CNTV = GIC_PPI(0x0B),
    /// @brief Secure Physical ARM timer
    IRQ_LOCAL_CNTPS = GIC_PPI(0x0D),
    /// @brief Non secure Physical ARM timer
    IRQ_LOCAL_CNTPNS = GIC_PPI(0x0E),

    /// @brief ARM core mailbox 0, core 0
    IRQ_LOCAL_MAILBOX_0_0 = GIC_SPI(0x00),
    /// @brief ARM core mailbox 1, core 0
    IRQ_LOCAL_MAILBOX_1_0 = GIC_SPI(0x01),
    /// @brief ARM core mailbox 2, core 0
    IRQ_LOCAL_MAILBOX_2_0 = GIC_SPI(0x02),
    /// @brief ARM core mailbox 3, core 0
    IRQ_LOCAL_MAILBOX_3_0 = GIC_SPI(0x03),
    /// @brief ARM core mailbox 0, core 1
    IRQ_LOCAL_MAILBOX_0_1 = GIC_SPI(0x04),
    /// @brief ARM core mailbox 1, core 1
    IRQ_LOCAL_MAILBOX_1_1 = GIC_SPI(0x05),
    /// @brief ARM core mailbox 2, core 1
    IRQ_LOCAL_MAILBOX_2_1 = GIC_SPI(0x06),
    /// @brief ARM core mailbox 3, core 1
    IRQ_LOCAL_MAILBOX_3_1 = GIC_SPI(0x07),
    /// @brief ARM core mailbox 0, core 2
    IRQ_LOCAL_MAILBOX_0_2 = GIC_SPI(0x08),
    /// @brief ARM core mailbox 1, core 2
    IRQ_LOCAL_MAILBOX_1_2 = GIC_SPI(0x09),
    /// @brief ARM core mailbox 2, core 2
    IRQ_LOCAL_MAILBOX_2_2 = GIC_SPI(0x0A),
    /// @brief ARM core mailbox 3, core 2
    IRQ_LOCAL_MAILBOX_3_2 = GIC_SPI(0x0B),
    /// @brief ARM core mailbox 0, core 3
    IRQ_LOCAL_MAILBOX_0_3 = GIC_SPI(0x0C),
    /// @brief ARM core mailbox 1, core 3
    IRQ_LOCAL_MAILBOX_1_3 = GIC_SPI(0x0D),
    /// @brief ARM core mailbox 2, core 3
    IRQ_LOCAL_MAILBOX_2_3 = GIC_SPI(0x0E),
    /// @brief ARM core mailbox 3, core 3
    IRQ_LOCAL_MAILBOX_3_3 = GIC_SPI(0x0F),

    /// @brief PMU (performance monitoring unit) core 0 interrupt
    IRQ_PMU0 = GIC_SPI(0x10),
    /// @brief PMU (performance monitoring unit) core 1 interrupt
    IRQ_PMU1 = GIC_SPI(0x11),
    /// @brief PMU (performance monitoring unit) core 2 interrupt
    IRQ_PMU2 = GIC_SPI(0x12),
    /// @brief PMU (performance monitoring unit) core 3 interrupt
    IRQ_PMU3 = GIC_SPI(0x13),
    /// @brief AXI bus error
    IRQ_LOCAL_AXI_ERR = GIC_SPI(0x14),
    /// @brief ARM mailbox interrupt
    IRQ_ARM_MAILBOX = GIC_SPI(0x21),
    /// @brief ARM doorbell (VCHIQ) 0 interrupt
    IRQ_ARM_DOORBELL_0 = GIC_SPI(0x22),
    /// @brief ARM doorbell (VCHIQ) 1 interrupt
    IRQ_ARM_DOORBELL_1 = GIC_SPI(0x23),
    /// @brief VPU halted 0 interrupt
    IRQ_VPU0_HALTED = GIC_SPI(0x24),
    /// @brief VPU halted 1 interrupt
    IRQ_VPU1_HALTED = GIC_SPI(0x25),
    /// @brief ARM address error interrupt
    IRQ_ILLEGAL_TYPE0 = GIC_SPI(0x26),
    /// @brief ARM AXI error interrupt
    IRQ_ILLEGAL_TYPE1 = GIC_SPI(0x27),
    /// @brief USB Host port interrupt
    IRQ_HOSTPORT = GIC_SPI(0x28),

    /// @brief BCM system timer 0 interrupt, when compare value is hit
    IRQ_TIMER0 = GIC_SPI(0x40),
    /// @brief BCM system timer 1 interrupt, when compare value is hit
    IRQ_TIMER1 = GIC_SPI(0x41),
    /// @brief BCM system timer 2 interrupt, when compare value is hit
    IRQ_TIMER2 = GIC_SPI(0x42),
    /// @brief BCM system timer 3 interrupt, when compare value is hit
    IRQ_TIMER3 = GIC_SPI(0x43),
    /// @brief H.264 codec 0 interrupt
    IRQ_CODEC0 = GIC_SPI(0x44),
    /// @brief H.264 codec 1 interrupt
    IRQ_CODEC1 = GIC_SPI(0x45),
    /// @brief H.264 codec 2 interrupt
    IRQ_CODEC2 = GIC_SPI(0x46),
    /// @brief JPEG interrupt
    IRQ_JPEG = GIC_SPI(0x49),
    /// @brief ISP interrupt
    IRQ_ISP = GIC_SPI(0x49),
    /// @brief USB interrupt
    IRQ_USB = GIC_SPI(0x49),
    /// @brief VideoCore 3D interrupt
    IRQ_3D = GIC_SPI(0x4A),
    /// @brief GPU transposer interrupt
    IRQ_TRANSPOSER = GIC_SPI(0x4B),
    /// @brief Multicore sync 0 interrupt
    IRQ_MULTICORESYNC0 = GIC_SPI(0x4C),
    /// @brief Multicore sync 1 interrupt
    IRQ_MULTICORESYNC1 = GIC_SPI(0x4D),
    /// @brief Multicore sync 2 interrupt
    IRQ_MULTICORESYNC2 = GIC_SPI(0x4E),
    /// @brief Multicore sync 3 interrupt
    IRQ_MULTICORESYNC3 = GIC_SPI(0x4F),
    /// @brief DMA channel 0 interrupt
    IRQ_DMA0 = GIC_SPI(0x50),
    /// @brief DMA channel 1 interrupt
    IRQ_DMA1 = GIC_SPI(0x51),
    /// @brief DMA channel 2 interrupt, I2S PCM TX
    IRQ_DMA2 = GIC_SPI(0x52),
    /// @brief DMA channel 3 interrupt, I2S PCM RX
    IRQ_DMA3 = GIC_SPI(0x53),
    /// @brief DMA channel 4 interrupt, SMI
    IRQ_DMA4 = GIC_SPI(0x54),
    /// @brief DMA channel 5 interrupt, PWM
    IRQ_DMA5 = GIC_SPI(0x55),
    /// @brief DMA channel 6 interrupt, SPI TX
    IRQ_DMA6 = GIC_SPI(0x56),
    /// @brief DMA channel 7/8 interrupt, SPI RX
    IRQ_DMA7 = GIC_SPI(0x57),
    /// @brief DMA channel 7/8 interrupt, undocumented
    IRQ_DMA8 = GIC_SPI(0x57),
    /// @brief DMA channel 9/10 interrupt, undocumented
    IRQ_DMA9 = GIC_SPI(0x58),
    /// @brief DMA channel 9/10 interrupt, HDMI
    IRQ_DMA10 = GIC_SPI(0x58),
    /// @brief DMA channel 11 interrupt, EMMC
    IRQ_DMA11 = GIC_SPI(0x59),
    /// @brief DMA channel 12 interrupt, UART TX
    IRQ_DMA12 = GIC_SPI(0x5A),
    /// @brief DMA channel 13 interrupt, undocumented
    IRQ_DMA13 = GIC_SPI(0x5B),
    /// @brief DMA channel 14 interrupt, UART RX
    IRQ_DMA14 = GIC_SPI(0x5C),
    /// @brief AUX UART 1 / SPI 0/1 interrupt
    IRQ_AUX_UART = GIC_SPI(0x5D),
    /// @brief AUX UART 1 / SPI 0/1 interrupt
    IRQ_AUX_SPI = GIC_SPI(0x5D),
    /// @brief ARM interrupt
    IRQ_ARM = GIC_SPI(0x5E),
    /// @brief DMA channel 15 interrupt
    IRQ_DMA15 = GIC_SPI(0x5F),
    /// @brief HMDI CEC interrupt
    IRQ_HDMI_CEC = GIC_SPI(0x60),
    /// @brief HVS (video scaler) interrupt
    IRQ_VIDEOSCALER = GIC_SPI(0x61),
    /// @brief Video decoder interrupt
    IRQ_DECODER = GIC_SPI(0x62),
    /// @brief DSI 0 (display) interrupt
    IRQ_DSI0 = GIC_SPI(0x64),
    /// @brief GPU pixel valve 2 interrupt
    IRQ_PIXELVALVE2 = GIC_SPI(0x65),
    /// @brief CSI 0 (camera) interrupt
    IRQ_CAM0 = GIC_SPI(0x66),
    /// @brief CSI 1 (camera) interrupt
    IRQ_CAM1 = GIC_SPI(0x67),
    /// @brief HDMI 0 interrupt
    IRQ_HDMI0 = GIC_SPI(0x68),
    /// @brief HDMI 1 interrupt
    IRQ_HDMI1 = GIC_SPI(0x69),
    /// @brief GPU pixel valve 3 interrupt
    IRQ_PIXELVALVE3 = GIC_SPI(0x6A),
    /// @brief SPI BSC slave interrupt
    IRQ_SPI_BSC = GIC_SPI(0x6A),
    /// @brief DSI 1 (display) interrupt
    IRQ_DSI1 = GIC_SPI(0x6C),
    /// @brief GPU pixel valve 0 interrupt
    IRQ_PIXELVALVE0 = GIC_SPI(0x6D),
    /// @brief GPU pixel valve 1 / 4 interrupt
    IRQ_PIXELVALVE1 = GIC_SPI(0x6E),
    /// @brief CPR interrupt
    IRQ_CPR = GIC_SPI(0x6F),
    /// @brief SMI (firmware) interrupt
    IRQ_FIRMWARE = GIC_SPI(0x70),
    /// @brief GPIO 0 interrupt
    IRQ_GPIO0 = GIC_SPI(0x71),
    /// @brief GPIO 1 interrupt
    IRQ_GPIO1 = GIC_SPI(0x72),
    /// @brief GPIO 2 interrupt
    IRQ_GPIO2 = GIC_SPI(0x73),
    /// @brief GPIO 3 interrupt
    IRQ_GPIO3 = GIC_SPI(0x74),
    /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
    IRQ_I2C = GIC_SPI(0x75),
    /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
    IRQ_SPI = GIC_SPI(0x76),
    /// @brief I2S interrupt
    IRQ_I2SPCM = GIC_SPI(0x77),
    /// @brief SD host interrupt
    IRQ_SDHOST = GIC_SPI(0x78),
    /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
    IRQ_UART = GIC_SPI(0x79),
    /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
    IRQ_SLIMBUS = GIC_SPI(0x7A),
    /// @brief GPU? VEC interrupt
    IRQ_VEC = GIC_SPI(0x7B),
    /// @brief CPG interrupt
    IRQ_CPG = GIC_SPI(0x7C),
    /// @brief RNG (random number generator) interrupt
    IRQ_RNG = GIC_SPI(0x7D),
    /// @brief EMMC / EMMC2 interrupt
    IRQ_ARASANSDIO = GIC_SPI(0x7E),
    /// @brief Ethernet PCIe secure interrupt
    IRQ_ETH_PCIE_S = GIC_SPI(0x7F),
    /// @brief PCI Express AVS interrupt
    IRQ_AVS = GIC_SPI(0x89),
    /// @brief PCI Express Ethernet A interrupt
    IRQ_PCIE_INTA = GIC_SPI(0x8F),
    /// @brief PCI Express Ethernet B interrupt
    IRQ_PCIE_INTB = GIC_SPI(0x90),
    /// @brief PCI Express Ethernet C interrupt
    IRQ_PCIE_INTC = GIC_SPI(0x91),
    /// @brief PCI Express Ethernet D interrupt
    IRQ_PCIE_INTD = GIC_SPI(0x92),
    /// @brief PCI Express Host A interrupt
    IRQ_PCIE_HOST_INTA = GIC_SPI(0x93),
    /// @brief PCI Express Host MSI interrupt
    IRQ_PCIE_HOST_MSI = GIC_SPI(0x94),
    /// @brief Ethernet interrupt
    IRQ_GENET_0_A = GIC_SPI(0x9D),
    /// @brief Ethernet interrupt
    IRQ_GENET_0_B = GIC_SPI(0x9E),
    /// @brief USB XHCI interrupt
    IRQ_XHCI_INTERNAL = GIC_SPI(0xB0),
};

/// <summary>
/// @brief IRQ interrupt numbers
/// </summary>
enum class FIQ_ID
{
    /// @brief Hypervisor Physical ARM timer
    FIQ_LOCAL_CNTHP = GIC_PPI(0x0A),
    /// @brief Virtual ARM timer
    FIQ_LOCAL_CNTV = GIC_PPI(0x0B),
    /// @brief Secure Physical ARM timer
    FIQ_LOCAL_CNTPS = GIC_PPI(0x0D),
    /// @brief Non secure Physical ARM timer
    FIQ_LOCAL_CNTPNS = GIC_PPI(0x0E),

    /// @brief ARM core mailbox 0, core 0
    FIQ_LOCAL_MAILBOX_0_0 = GIC_SPI(0x00),
    /// @brief ARM core mailbox 1, core 0
    FIQ_LOCAL_MAILBOX_1_0 = GIC_SPI(0x01),
    /// @brief ARM core mailbox 2, core 0
    FIQ_LOCAL_MAILBOX_2_0 = GIC_SPI(0x02),
    /// @brief ARM core mailbox 3, core 0
    FIQ_LOCAL_MAILBOX_3_0 = GIC_SPI(0x03),
    /// @brief ARM core mailbox 0, core 1
    FIQ_LOCAL_MAILBOX_0_1 = GIC_SPI(0x04),
    /// @brief ARM core mailbox 1, core 1
    FIQ_LOCAL_MAILBOX_1_1 = GIC_SPI(0x05),
    /// @brief ARM core mailbox 2, core 1
    FIQ_LOCAL_MAILBOX_2_1 = GIC_SPI(0x06),
    /// @brief ARM core mailbox 3, core 1
    FIQ_LOCAL_MAILBOX_3_1 = GIC_SPI(0x07),
    /// @brief ARM core mailbox 0, core 2
    FIQ_LOCAL_MAILBOX_0_2 = GIC_SPI(0x08),
    /// @brief ARM core mailbox 1, core 2
    FIQ_LOCAL_MAILBOX_1_2 = GIC_SPI(0x09),
    /// @brief ARM core mailbox 2, core 2
    FIQ_LOCAL_MAILBOX_2_2 = GIC_SPI(0x0A),
    /// @brief ARM core mailbox 3, core 2
    FIQ_LOCAL_MAILBOX_3_2 = GIC_SPI(0x0B),
    /// @brief ARM core mailbox 0, core 3
    FIQ_LOCAL_MAILBOX_0_3 = GIC_SPI(0x0C),
    /// @brief ARM core mailbox 1, core 3
    FIQ_LOCAL_MAILBOX_1_3 = GIC_SPI(0x0D),
    /// @brief ARM core mailbox 2, core 3
    FIQ_LOCAL_MAILBOX_2_3 = GIC_SPI(0x0E),
    /// @brief ARM core mailbox 3, core 3
    FIQ_LOCAL_MAILBOX_3_3 = GIC_SPI(0x0F),

    /// @brief PMU (performance monitoring unit) core 0 interrupt
    FIQ_PMU0 = GIC_SPI(0x10),
    /// @brief PMU (performance monitoring unit) core 1 interrupt
    FIQ_PMU1 = GIC_SPI(0x11),
    /// @brief PMU (performance monitoring unit) core 2 interrupt
    FIQ_PMU2 = GIC_SPI(0x12),
    /// @brief PMU (performance monitoring unit) core 3 interrupt
    FIQ_PMU3 = GIC_SPI(0x13),
    /// @brief AXI bus error
    FIQ_LOCAL_AXI_ERR = GIC_SPI(0x14),
    /// @brief ARM mailbox interrupt
    FIQ_ARM_MAILBOX = GIC_SPI(0x21),
    /// @brief ARM doorbell (VCHIQ) 0 interrupt
    FIQ_ARM_DOORBELL_0 = GIC_SPI(0x22),
    /// @brief ARM doorbell (VCHIQ) 1 interrupt
    FIQ_ARM_DOORBELL_1 = GIC_SPI(0x23),
    /// @brief VPU halted 0 interrupt
    FIQ_VPU0_HALTED = GIC_SPI(0x24),
    /// @brief VPU halted 1 interrupt
    FIQ_VPU1_HALTED = GIC_SPI(0x25),
    /// @brief ARM address error interrupt
    FIQ_ILLEGAL_TYPE0 = GIC_SPI(0x26),
    /// @brief ARM AXI error interrupt
    FIQ_ILLEGAL_TYPE1 = GIC_SPI(0x27),
    /// @brief USB Host port interrupt
    FIQ_HOSTPORT = GIC_SPI(0x28),

    /// @brief BCM system timer 0 interrupt, when compare value is hit
    FIQ_TIMER0 = GIC_SPI(0x40),
    /// @brief BCM system timer 1 interrupt, when compare value is hit
    FIQ_TIMER1 = GIC_SPI(0x41),
    /// @brief BCM system timer 2 interrupt, when compare value is hit
    FIQ_TIMER2 = GIC_SPI(0x42),
    /// @brief BCM system timer 3 interrupt, when compare value is hit
    FIQ_TIMER3 = GIC_SPI(0x43),
    /// @brief H.264 codec 0 interrupt
    FIQ_CODEC0 = GIC_SPI(0x44),
    /// @brief H.264 codec 1 interrupt
    FIQ_CODEC1 = GIC_SPI(0x45),
    /// @brief H.264 codec 2 interrupt
    FIQ_CODEC2 = GIC_SPI(0x46),
    /// @brief JPEG interrupt
    FIQ_JPEG = GIC_SPI(0x49),
    /// @brief ISP interrupt
    FIQ_ISP = GIC_SPI(0x49),
    /// @brief USB interrupt
    FIQ_USB = GIC_SPI(0x49),
    /// @brief VideoCore 3D interrupt
    FIQ_3D = GIC_SPI(0x4A),
    /// @brief GPU transposer interrupt
    FIQ_TRANSPOSER = GIC_SPI(0x4B),
    /// @brief Multicore sync 0 interrupt
    FIQ_MULTICORESYNC0 = GIC_SPI(0x4C),
    /// @brief Multicore sync 1 interrupt
    FIQ_MULTICORESYNC1 = GIC_SPI(0x4D),
    /// @brief Multicore sync 2 interrupt
    FIQ_MULTICORESYNC2 = GIC_SPI(0x4E),
    /// @brief Multicore sync 3 interrupt
    FIQ_MULTICORESYNC3 = GIC_SPI(0x4F),
    /// @brief DMA channel 0 interrupt
    FIQ_DMA0 = GIC_SPI(0x50),
    /// @brief DMA channel 1 interrupt
    FIQ_DMA1 = GIC_SPI(0x51),
    /// @brief DMA channel 2 interrupt, I2S PCM TX
    FIQ_DMA2 = GIC_SPI(0x52),
    /// @brief DMA channel 3 interrupt, I2S PCM RX
    FIQ_DMA3 = GIC_SPI(0x53),
    /// @brief DMA channel 4 interrupt, SMI
    FIQ_DMA4 = GIC_SPI(0x54),
    /// @brief DMA channel 5 interrupt, PWM
    FIQ_DMA5 = GIC_SPI(0x55),
    /// @brief DMA channel 6 interrupt, SPI TX
    FIQ_DMA6 = GIC_SPI(0x56),
    /// @brief DMA channel 7/8 interrupt, SPI RX
    FIQ_DMA7 = GIC_SPI(0x57),
    /// @brief DMA channel 7/8 interrupt, undocumented
    FIQ_DMA8 = GIC_SPI(0x57),
    /// @brief DMA channel 9/10 interrupt, undocumented
    FIQ_DMA9 = GIC_SPI(0x58),
    /// @brief DMA channel 9/10 interrupt, HDMI
    FIQ_DMA10 = GIC_SPI(0x58),
    /// @brief DMA channel 11 interrupt, EMMC
    FIQ_DMA11 = GIC_SPI(0x59),
    /// @brief DMA channel 12 interrupt, UART TX
    FIQ_DMA12 = GIC_SPI(0x5A),
    /// @brief DMA channel 13 interrupt, undocumented
    FIQ_DMA13 = GIC_SPI(0x5B),
    /// @brief DMA channel 14 interrupt, UART RX
    FIQ_DMA14 = GIC_SPI(0x5C),
    /// @brief AUX UART 1 / SPI 0/1 interrupt
    FIQ_AUX_UART = GIC_SPI(0x5D),
    /// @brief AUX UART 1 / SPI 0/1 interrupt
    FIQ_AUX_SPI = GIC_SPI(0x5D),
    /// @brief ARM interrupt
    FIQ_ARM = GIC_SPI(0x5E),
    /// @brief DMA channel 15 interrupt
    FIQ_DMA15 = GIC_SPI(0x5F),
    /// @brief HMDI CEC interrupt
    FIQ_HDMI_CEC = GIC_SPI(0x60),
    /// @brief HVS (video scaler) interrupt
    FIQ_VIDEOSCALER = GIC_SPI(0x61),
    /// @brief Video decoder interrupt
    FIQ_DECODER = GIC_SPI(0x62),
    /// @brief DSI 0 (display) interrupt
    FIQ_DSI0 = GIC_SPI(0x64),
    /// @brief GPU pixel valve 2 interrupt
    FIQ_PIXELVALVE2 = GIC_SPI(0x65),
    /// @brief CSI 0 (camera) interrupt
    FIQ_CAM0 = GIC_SPI(0x66),
    /// @brief CSI 1 (camera) interrupt
    FIQ_CAM1 = GIC_SPI(0x67),
    /// @brief HDMI 0 interrupt
    FIQ_HDMI0 = GIC_SPI(0x68),
    /// @brief HDMI 1 interrupt
    FIQ_HDMI1 = GIC_SPI(0x69),
    /// @brief GPU pixel valve 3 interrupt
    FIQ_PIXELVALVE3 = GIC_SPI(0x6A),
    /// @brief SPI BSC slave interrupt
    FIQ_SPI_BSC = GIC_SPI(0x6A),
    /// @brief DSI 1 (display) interrupt
    FIQ_DSI1 = GIC_SPI(0x6C),
    /// @brief GPU pixel valve 0 interrupt
    FIQ_PIXELVALVE0 = GIC_SPI(0x6D),
    /// @brief GPU pixel valve 1 / 4 interrupt
    FIQ_PIXELVALVE1 = GIC_SPI(0x6E),
    /// @brief CPR interrupt
    FIQ_CPR = GIC_SPI(0x6F),
    /// @brief SMI (firmware) interrupt
    FIQ_FIRMWARE = GIC_SPI(0x70),
    /// @brief GPIO 0 interrupt
    FIQ_GPIO0 = GIC_SPI(0x71),
    /// @brief GPIO 1 interrupt
    FIQ_GPIO1 = GIC_SPI(0x72),
    /// @brief GPIO 2 interrupt
    FIQ_GPIO2 = GIC_SPI(0x73),
    /// @brief GPIO 3 interrupt
    FIQ_GPIO3 = GIC_SPI(0x74),
    /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
    FIQ_I2C = GIC_SPI(0x75),
    /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
    FIQ_SPI = GIC_SPI(0x76),
    /// @brief I2S interrupt
    FIQ_I2SPCM = GIC_SPI(0x77),
    /// @brief SD host interrupt
    FIQ_SDHOST = GIC_SPI(0x78),
    /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
    FIQ_UART = GIC_SPI(0x79),
    /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
    FIQ_SLIMBUS = GIC_SPI(0x7A),
    /// @brief GPU? VEC interrupt
    FIQ_VEC = GIC_SPI(0x7B),
    /// @brief CPG interrupt
    FIQ_CPG = GIC_SPI(0x7C),
    /// @brief RNG (random number generator) interrupt
    FIQ_RNG = GIC_SPI(0x7D),
    /// @brief EMMC / EMMC2 interrupt
    FIQ_ARASANSDIO = GIC_SPI(0x7E),
    /// @brief Ethernet PCIe secure interrupt
    FIQ_ETH_PCIE_S = GIC_SPI(0x7F),
    /// @brief PCI Express AVS interrupt
    FIQ_AVS = GIC_SPI(0x89),
    /// @brief PCI Express Ethernet A interrupt
    FIQ_PCIE_INTA = GIC_SPI(0x8F),
    /// @brief PCI Express Ethernet B interrupt
    FIQ_PCIE_INTB = GIC_SPI(0x90),
    /// @brief PCI Express Ethernet C interrupt
    FIQ_PCIE_INTC = GIC_SPI(0x91),
    /// @brief PCI Express Ethernet D interrupt
    FIQ_PCIE_INTD = GIC_SPI(0x92),
    /// @brief PCI Express Host A interrupt
    FIQ_PCIE_HOST_INTA = GIC_SPI(0x93),
    /// @brief PCI Express Host MSI interrupt
    FIQ_PCIE_HOST_MSI = GIC_SPI(0x94),
    /// @brief Ethernet interrupt
    FIQ_GENET_0_A = GIC_SPI(0x9D),
    /// @brief Ethernet interrupt
    FIQ_GENET_0_B = GIC_SPI(0x9E),
    /// @brief USB XHCI interrupt
    FIQ_XHCI_INTERNAL = GIC_SPI(0xB0),
};

#endif

void EnableIRQ(IRQ_ID irqID);
void DisableIRQ(IRQ_ID irqID);

void EnableFIQ(FIQ_ID fiqID);
void DisableFIQ(FIQ_ID fiqID);

} // namespace baremetal
