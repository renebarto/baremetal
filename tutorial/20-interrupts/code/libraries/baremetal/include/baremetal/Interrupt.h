//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Interrupt.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Interrupt definitions
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
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

/// @file
/// Interrupt handler function

#if BAREMETAL_RPI_TARGET <= 3

// IRQ

#define BCM_IRQS_PER_REG         32
#define BCM_IRQS_BASIC_REG       8
#define ARM_IRQS_LOCAL_REG       12

/// @brief IRQ 1 register holds IRQ 0..31
#define BCM_IRQ1_BASE            0
/// @brief IRQ 2 register holds IRQ 32..63
#define BCM_IRQ2_BASE            (BCM_IRQ1_BASE + BCM_IRQS_PER_REG)
/// @brief IRQ basic register holds 8 IRQs see @ref  RASPBERRY_PI_INTERRUPT_CONTROL
#define BCM_IRQBASIC_BASE        (BCM_IRQ2_BASE + BCM_IRQS_PER_REG)
/// @brief IRQ local register holds 12 IRQs see @ref RASPBERRY_PI_BCM_LOCAL_DEVICE_REGISTERS
#define ARM_IRQLOCAL_BASE        (BCM_IRQBASIC_BASE + BCM_IRQS_BASIC_REG)

#define ARM_IC_IRQ_REGS 3

#define ARM_IC_IRQ_PENDING(irq)                                                                                                                                \
    ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_IRQ_PENDING_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_IRQ_PENDING_2 : RPI_INTRCTRL_IRQ_BASIC_PENDING))
#define ARM_IC_IRQS_ENABLE(irq)                                                                                                                                \
    ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_ENABLE_IRQS_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_ENABLE_IRQS_2 : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
#define ARM_IC_IRQS_DISABLE(irq)                                                                                                                               \
    ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_DISABLE_IRQS_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_DISABLE_IRQS_2 : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
#define ARM_IRQ_MASK(irq) (1 << ((irq) & (BCM_IRQS_PER_REG - 1)))

// IRQ 1 interrupts

/// @brief RPI System Timer Compare 0 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER0           (BCM_IRQ1_BASE + 0)
/// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER1           (BCM_IRQ1_BASE + 1)
/// @brief RPI System Timer Compare 2 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER2           (BCM_IRQ1_BASE + 2)
/// @brief RPI System Timer Compare 3 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER3           (BCM_IRQ1_BASE + 3)
/// @brief RPI Codec 0 interrupt (bcm2835-cprman)
#define BCM_IRQ_CODEC0           (BCM_IRQ1_BASE + 4)
/// @brief RPI Codec 1 interrupt (bcm2835-cprman)
#define BCM_IRQ_CODEC1           (BCM_IRQ1_BASE + 5)
/// @brief RPI Codec 2 interrupt
#define BCM_IRQ_CODEC2           (BCM_IRQ1_BASE + 6)
/// @brief RPI JPEG interrupt
#define BCM_IRQ_JPEG             (BCM_IRQ1_BASE + 7)
#define BCM_IRQ_ISP              (BCM_IRQ1_BASE + 8)
/// @brief RPI USB interrupt (bcm2708-usb)
#define BCM_IRQ_USB              (BCM_IRQ1_BASE + 9)
/// @brief RPI VC4 3D interrupt (vc4-v3d)
#define BCM_IRQ_3D               (BCM_IRQ1_BASE + 10)
/// @brief RPI Transposer (TXP) interrupt (bcm2835-txp)
#define BCM_IRQ_TRANSPOSER       (BCM_IRQ1_BASE + 11)
#define BCM_IRQ_MULTICORESYNC0   (BCM_IRQ1_BASE + 12)
#define BCM_IRQ_MULTICORESYNC1   (BCM_IRQ1_BASE + 13)
#define BCM_IRQ_MULTICORESYNC2   (BCM_IRQ1_BASE + 14)
#define BCM_IRQ_MULTICORESYNC3   (BCM_IRQ1_BASE + 15)
/// @brief RPI DMA channel 0 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA0             (BCM_IRQ1_BASE + 16)
/// @brief RPI DMA channel 1 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA1             (BCM_IRQ1_BASE + 17)
/// @brief RPI DMA channel 2 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA2             (BCM_IRQ1_BASE + 18)
/// @brief RPI DMA channel 3 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA3             (BCM_IRQ1_BASE + 19)
/// @brief RPI DMA channel 4 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA4             (BCM_IRQ1_BASE + 20)
/// @brief RPI DMA channel 5 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA5             (BCM_IRQ1_BASE + 21)
/// @brief RPI DMA channel 6 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA6             (BCM_IRQ1_BASE + 22)
/// @brief RPI DMA channel 7 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA7             (BCM_IRQ1_BASE + 23)
/// @brief RPI DMA channel 8 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA8             (BCM_IRQ1_BASE + 24)
/// @brief RPI DMA channel 9 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA9             (BCM_IRQ1_BASE + 25)
/// @brief RPI DMA channel 10 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA10            (BCM_IRQ1_BASE + 26)
/// @brief RPI DMA channel 11/12/13/14 interrupt (bcm2835-dma)
#define BCM_IRQ_DMA11            (BCM_IRQ1_BASE + 27)
/// @brief RPI DMA shared interrupt (bcm2835-dma)
#define BCM_IRQ_DMA_SHARED       (BCM_IRQ1_BASE + 28)
/// @brief RPI Auxiliary Peripheral interrupt (bcm2835-aux-uart, bcm43438-bt)
#define BCM_IRQ_AUX              (BCM_IRQ1_BASE + 29)
#define BCM_IRQ_ARM              (BCM_IRQ1_BASE + 30)
#define BCM_IRQ_VPUDMA           (BCM_IRQ1_BASE + 31)

/// @brief RPI USB Host interrupt (bcm2708-usb)
#define BCM_IRQ_HOSTPORT         (BCM_IRQ2_BASE + 0)
/// @brief RPI Videoscaler interrupt (bcm2835-hvs)
#define BCM_IRQ_VIDEOSCALER      (BCM_IRQ2_BASE + 1)
#define BCM_IRQ_CCP2TX           (BCM_IRQ2_BASE + 2)
#define BCM_IRQ_SDC              (BCM_IRQ2_BASE + 3)
/// @brief RPI DSI interrupt (bcm2835-dsi0)
#define BCM_IRQ_DSI0             (BCM_IRQ2_BASE + 4)
/// @brief RPI AVE interrupt (bcm2711-pixelvalve2)
#define BCM_IRQ_AVE              (BCM_IRQ2_BASE + 5)
/// @brief RPI CAM 0 interrupt (bcm2835-unicam)
#define BCM_IRQ_CAM0             (BCM_IRQ2_BASE + 6)
/// @brief RPI CAM 1 interrupt (bcm2835-unicam)
#define BCM_IRQ_CAM1             (BCM_IRQ2_BASE + 7)
/// @brief RPI HDMI 0 interrupt (bcm2835-hdmi)
#define BCM_IRQ_HDMI0            (BCM_IRQ2_BASE + 8)
/// @brief RPI HDMI 1 interrupt (bcm2835-hdmi)
#define BCM_IRQ_HDMI1            (BCM_IRQ2_BASE + 9)
/// @brief RPI Pixel valve 2 interrupt (bcm2835-pixelvalve2, bcm2711-pixelvalve3)
#define BCM_IRQ_PIXELVALVE1      (BCM_IRQ2_BASE + 10)
/// @brief RPI I2C slave interrupt
#define BCM_IRQ_I2CSPISLV        (BCM_IRQ2_BASE + 11)
/// @brief RPI DSI1 interrupt (bcm2835-dsi1)
#define BCM_IRQ_DSI1             (BCM_IRQ2_BASE + 12)
/// @brief RPI Pixel valve 0 interrupt (bcm2835-pixelvalve0, bcm2711-pixelvalve0)
#define BCM_IRQ_PWA0             (BCM_IRQ2_BASE + 13)
/// @brief RPI Pixel valve 1 interrupt (bcm2835-pixelvalve1, bcm2711-pixelvalve1, bcm2711-pixelvalve4)
#define BCM_IRQ_PWA1             (BCM_IRQ2_BASE + 14)
#define BCM_IRQ_CPR              (BCM_IRQ2_BASE + 15)
/// @brief RPI SMI interrupt (bcm2835-smi, rpi-firmware-kms)
#define BCM_IRQ_SMI              (BCM_IRQ2_BASE + 16)
/// @brief RPI GPIO 0 interrupt (bcm2835-gpio)
#define BCM_IRQ_GPIO0            (BCM_IRQ2_BASE + 17)
/// @brief RPI GPIO 1 interrupt (bcm2835-gpio)
#define BCM_IRQ_GPIO1            (BCM_IRQ2_BASE + 18)
/// @brief RPI GPIO 2 interrupt
#define BCM_IRQ_GPIO2            (BCM_IRQ2_BASE + 19)
/// @brief RPI GPIO 3 interrupt
#define BCM_IRQ_GPIO3            (BCM_IRQ2_BASE + 20)
/// @brief RPI I2C interrupt (bcm2835-i2c)
#define BCM_IRQ_I2C              (BCM_IRQ2_BASE + 21)
/// @brief RPI SPI interrupt (bcm2835-spi)
#define BCM_IRQ_SPI              (BCM_IRQ2_BASE + 22)
/// @brief RPI I2C audio interrupt
#define BCM_IRQ_I2SPCM           (BCM_IRQ2_BASE + 23)
/// @brief RPI EMMC / SDIO interrupt (bcm2835-sdhost)
#define BCM_IRQ_SDIO             (BCM_IRQ2_BASE + 24)
/// @brief RPI UART interrupt (arm,primecell, serial@7e201000, bcm43438-bt)
#define BCM_IRQ_UART             (BCM_IRQ2_BASE + 25)
#define BCM_IRQ_SLIMBUS          (BCM_IRQ2_BASE + 26)
/// @brief RPI VEC interrupt (bcm2835-vec, bcm2711-vec)
#define BCM_IRQ_VEC              (BCM_IRQ2_BASE + 27)
#define BCM_IRQ_CPG              (BCM_IRQ2_BASE + 28)
/// @brief RPI RNG interrupt (bcm2835-rng)
#define BCM_IRQ_RNG              (BCM_IRQ2_BASE + 29)
/// @brief RPI SDHCI (bcm2835-sdhci, bcm2711-emmc2)
#define BCM_IRQ_ARASANSDIO       (BCM_IRQ2_BASE + 30)
#define BCM_IRQ_AVSPMON          (BCM_IRQ2_BASE + 31)

/// @brief RPI ARM Timer interrupt interrupt
#define BCM_IRQ_BCM_TIMER        (BCM_IRQBASIC_BASE + 0)
/// @brief RPI ARM Mailbox interrupt interrupt (bcm2835-mbox)
#define BCM_IRQ_BCM_MAILBOX      (BCM_IRQBASIC_BASE + 1)
/// @brief RPI ARM Doorbell 0 interrupt interrupt (bcm2835-vchiq, bcm2711-vchiq)
#define BCM_IRQ_BCM_DOORBELL_0   (BCM_IRQBASIC_BASE + 2)
/// @brief RPI ARM Doorbell 1 interrupt interrupt
#define BCM_IRQ_BCM_DOORBELL_1   (BCM_IRQBASIC_BASE + 3)
/// @brief RPI ARM GPU 0 halted interrupt (bcm2835-cprman)
#define BCM_IRQ_VPU0_HALTED      (BCM_IRQBASIC_BASE + 4)
/// @brief RPI ARM GPU 1 halted interrupt (bcm2835-cprman)
#define BCM_IRQ_VPU1_HALTED      (BCM_IRQBASIC_BASE + 5)
/// @brief RPI ARM Illegal access type 1 interrupt
#define BCM_IRQ_ILLEGAL_TYPE0    (BCM_IRQBASIC_BASE + 6)
/// @brief RPI ARM Illegal access type 0 interrupt
#define BCM_IRQ_ILLEGAL_TYPE1    (BCM_IRQBASIC_BASE + 7)

/// @brief RPI ARM Local Counter-timer Physical Secure Timer interrupt (armv7-timer)
#define ARM_IRQLOCAL0_CNTPS      (ARM_IRQLOCAL_BASE + 0)
/// @brief RPI ARM Local Counter-timer Physical Timer interrupt (armv7-timer)
#define ARM_IRQLOCAL0_CNTPNS     (ARM_IRQLOCAL_BASE + 1)
/// @brief RPI ARM Local Counter-timer Hypervisor Timer interrupt (armv7-timer)
#define ARM_IRQLOCAL0_CNTHP      (ARM_IRQLOCAL_BASE + 2)
/// @brief RPI ARM Local Counter-timer Virtual Timer interrupt (armv7-timer)
#define ARM_IRQLOCAL0_CNTV       (ARM_IRQLOCAL_BASE + 3)
/// @brief RPI ARM Local Mailbox 0 interrupt
#define ARM_IRQLOCAL0_MAILBOX0   (ARM_IRQLOCAL_BASE + 4)
/// @brief RPI ARM Local Mailbox 1 interrupt
#define ARM_IRQLOCAL0_MAILBOX1   (ARM_IRQLOCAL_BASE + 5)
/// @brief RPI ARM Local Mailbox 2 interrupt
#define ARM_IRQLOCAL0_MAILBOX2   (ARM_IRQLOCAL_BASE + 6)
/// @brief RPI ARM Local Mailbox 3 interrupt
#define ARM_IRQLOCAL0_MAILBOX3   (ARM_IRQLOCAL_BASE + 7)
/// @brief RPI ARM Local GPU interrupt
#define ARM_IRQLOCAL0_GPU        (ARM_IRQLOCAL_BASE + 8) // cascaded GPU interrupts
/// @brief RPI ARM Local Performance Monitor Unit interrupt (cortex-a7-pmu)
#define ARM_IRQLOCAL0_PMU        (ARM_IRQLOCAL_BASE + 9)
/// @brief RPI ARM Local AXI interrupt
#define ARM_IRQLOCAL0_AXI_IDLE   (ARM_IRQLOCAL_BASE + 10) // on core 0 only
/// @brief RPI ARM Local Timer interrupt
#define ARM_IRQLOCAL0_LOCALTIMER (ARM_IRQLOCAL_BASE + 11)

#define IRQ_LINES                (BCM_IRQS_PER_REG * 2 + BCM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)

#elif BAREMETAL_RPI_TARGET == 4

// conversion from device tree source
#define GIC_PPI(n)             (16 + (n)) // private per core
#define GIC_SPI(n)             (32 + (n)) // shared between cores

// IRQs
#define BCM_IRQLOCAL0_CNTPNS   GIC_PPI(14)

#define BCM_IRQ_BCM_MAILBOX    GIC_SPI(0x21)
#define BCM_IRQ_BCM_DOORBELL_0 GIC_SPI(0x22) // (bcm2711-vchiq)
#define BCM_IRQ_HOSTPORT       GIC_SPI(0x28) // (bcm2708-usb)
/// @brief RPI System Timer Compare 0 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER0         GIC_SPI(0x40) // (bcm2835-system-timer)
/// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER1         GIC_SPI(0x41) // (bcm2835-system-timer)
/// @brief RPI System Timer Compare 2 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER2         GIC_SPI(0x42) // (bcm2835-system-timer)
/// @brief RPI System Timer Compare 3 interrupt (bcm2835-system-timer)
#define BCM_IRQ_TIMER3         GIC_SPI(0x43) // (bcm2835-system-timer)
/// @brief RPI Transposer (TXP) interrupt (bcm2835-txp)
#define BCM_IRQ_USB            GIC_SPI(0x49) // (bcm2708-usb)
#define BCM_IRQ_3D             GIC_SPI(0x4A) // (2711-v3d)
#define BCM_IRQ_TRANSPOSER     GIC_SPI(0x4B) // (bcm2835-system-timer)
#define BCM_IRQ_DMA0           GIC_SPI(0x50) // (bcm2835-dma)
#define BCM_IRQ_DMA1           GIC_SPI(0x51) // (bcm2835-dma)
#define BCM_IRQ_DMA2           GIC_SPI(0x52) // (bcm2835-dma)
#define BCM_IRQ_DMA3           GIC_SPI(0x53) // (bcm2835-dma)
#define BCM_IRQ_DMA4           GIC_SPI(0x54) // (bcm2835-dma)
#define BCM_IRQ_DMA5           GIC_SPI(0x55) // (bcm2835-dma)
#define BCM_IRQ_DMA6           GIC_SPI(0x56) // (bcm2835-dma)
#define BCM_IRQ_DMA7           GIC_SPI(0x57) // (bcm2835-dma)
#define BCM_IRQ_DMA8           GIC_SPI(0x57) // same value
#define BCM_IRQ_DMA9           GIC_SPI(0x58) // (bcm2835-dma)
#define BCM_IRQ_DMA10          GIC_SPI(0x58) // same value
#define BCM_IRQ_DMA11          GIC_SPI(0x59) // (bcm2711-dma)
#define BCM_IRQ_DMA12          GIC_SPI(0x5A) // (bcm2711-dma)
#define BCM_IRQ_DMA13          GIC_SPI(0x5B) // (bcm2711-dma)
#define BCM_IRQ_DMA14          GIC_SPI(0x5C) // (bcm2711-dma)
#define BCM_IRQ_AUX            GIC_SPI(0x5D) // (bcm2835-aux-uart)
#define BCM_IRQ_INTC           GIC_SPI(0x60) // (bcm2711-l2-intc) (HDMI0, HDMI1)
#define BCM_IRQ_VIDEOSCALER    GIC_SPI(0x61) // (bcm2711-hvs)
#define BCM_IRQ_CCP2TX         GIC_SPI(0x62) // (rpivid-vid-decoder)
#define BCM_IRQ_DSI0           GIC_SPI(0x64) // (bcm2835-dsi0)
#define BCM_IRQ_AVE            GIC_SPI(0x65) // (bcm2711-pixelvalve2)
#define BCM_IRQ_CAM0           GIC_SPI(0x66) // (bcm2835-unicam)
#define BCM_IRQ_CAM1           GIC_SPI(0x67) // (bcm2835-unicam)
#define BCM_IRQ_PWA3           GIC_SPI(0x6A) // (bcm2711-pixelvalve3)
#define BCM_IRQ_DSI0           GIC_SPI(0x6C) // (bcm2711-dsi1)
#define BCM_IRQ_PWA0           GIC_SPI(0x6D) // (bcm2711-pixelvalve0)
#define BCM_IRQ_PWA1           GIC_SPI(0x6E) // (bcm2711-pixelvalve1, bcm2711-pixelvalve4)
#define BCM_IRQ_SMI            GIC_SPI(0x70) // (rpi-firmware-kms-2711, bcm2835-smi)
#define BCM_IRQ_GPIO0          GIC_SPI(0x71) // (bcm2711-gpio)
#define BCM_IRQ_GPIO1          GIC_SPI(0x72) // (bcm2711-gpio)
#define BCM_IRQ_GPIO2          GIC_SPI(0x73)
#define BCM_IRQ_GPIO3          GIC_SPI(0x74)
#define BCM_IRQ_I2C            GIC_SPI(0x75) // (bcm2835-i2c)
#define BCM_IRQ_SPI            GIC_SPI(0x76) // (bcm2835-spi)
#define BCM_IRQ_SDIO           GIC_SPI(0x78) // (bcm2835-sdhost)
#define BCM_IRQ_UART           GIC_SPI(0x79) // (arm,primecell)
#define BCM_IRQ_VEC            GIC_SPI(0x7B) // (bcm2711-vec)
#define BCM_IRQ_ARASANSDIO     GIC_SPI(0x7E) // (bcm2835-sdhci, bcm2711-emmc2)
#define BCM_IRQ_PCIE_HOST_INTA GIC_SPI(0x8F) // (bcm2711-pcie)
#define BCM_IRQ_PCIE_HOST_MSI2 GIC_SPI(0x93) // (bcm2711-pcie)
#define BCM_IRQ_PCIE_HOST_MSI  GIC_SPI(0x94) // (bcm2711-pcie)
#define BCM_IRQ_BCM54213_0     GIC_SPI(0x9D) // (bcm2711-genet-v5)
#define BCM_IRQ_BCM54213_1     GIC_SPI(0x9E) // (bcm2711-genet-v5)
#define BCM_IRQ_XHCI_INTERNAL  GIC_SPI(0xB0) // (generic-xhci)

#define IRQ_LINES              256

#else // BAREMETAL_RPI_TARGET >= 5

// conversion from device tree source
#define GIC_PPI(n)             (16 + (n)) // private per core
#define GIC_SPI(n)             (32 + (n)) // shared between cores

// IRQs
#define BCM_IRQ_VIDEOSCALER    GIC_SPI(0x09) // (bcm2712-hvs)

#endif

// FIQ

#if BAREMETAL_RPI_TARGET <= 3

#define BCM_FIQ_TIMER0         0
#define BCM_FIQ_TIMER1         1
#define BCM_FIQ_TIMER2         2
#define BCM_FIQ_TIMER3         3
#define BCM_FIQ_CODEC0         4
#define BCM_FIQ_CODEC1         5
#define BCM_FIQ_CODEC2         6
#define BCM_FIQ_JPEG           7
#define BCM_FIQ_ISP            8
#define BCM_FIQ_USB            9
#define BCM_FIQ_3D             10
#define BCM_FIQ_TRANSPOSER     11
#define BCM_FIQ_MULTICORESYNC0 12
#define BCM_FIQ_MULTICORESYNC1 13
#define BCM_FIQ_MULTICORESYNC2 14
#define BCM_FIQ_MULTICORESYNC3 15
#define BCM_FIQ_DMA0           16
#define BCM_FIQ_DMA1           17
#define BCM_FIQ_DMA2           18
#define BCM_FIQ_DMA3           19
#define BCM_FIQ_DMA4           20
#define BCM_FIQ_DMA5           21
#define BCM_FIQ_DMA6           22
#define BCM_FIQ_DMA7           23
#define BCM_FIQ_DMA8           24
#define BCM_FIQ_DMA9           25
#define BCM_FIQ_DMA10          26
#define BCM_FIQ_DMA11          27
#define BCM_FIQ_DMA_SHARED     28
#define BCM_FIQ_AUX            29
#define BCM_FIQ_ARM            30
#define BCM_FIQ_VPUDMA         31
#define BCM_FIQ_HOSTPORT       32
#define BCM_FIQ_VIDEOSCALER    33
#define BCM_FIQ_CCP2TX         34
#define BCM_FIQ_SDC            35
#define BCM_FIQ_DSI0           36
#define BCM_FIQ_AVE            37
#define BCM_FIQ_CAM0           38
#define BCM_FIQ_CAM1           39
#define BCM_FIQ_HDMI0          40
#define BCM_FIQ_HDMI1          41
#define BCM_FIQ_PIXELVALVE1    42
#define BCM_FIQ_I2CSPISLV      43
#define BCM_FIQ_DSI1           44
#define BCM_FIQ_PWA0           45
#define BCM_FIQ_PWA1           46
#define BCM_FIQ_CPR            47
#define BCM_FIQ_SMI            48
#define BCM_FIQ_GPIO0          49
#define BCM_FIQ_GPIO1          50
#define BCM_FIQ_GPIO2          51
#define BCM_FIQ_GPIO3          52
#define BCM_FIQ_I2C            53
#define BCM_FIQ_SPI            54
#define BCM_FIQ_I2SPCM         55
#define BCM_FIQ_SDIO           56
#define BCM_FIQ_UART           57
#define BCM_FIQ_SLIMBUS        58
#define BCM_FIQ_VEC            59
#define BCM_FIQ_CPG            60
#define BCM_FIQ_RNG            61
#define BCM_FIQ_ARASANSDIO     62
#define BCM_FIQ_AVSPMON        63
#define BCM_FIQ_BCM_TIMER      64
#define BCM_FIQ_BCM_MAILBOX    65
#define BCM_FIQ_BCM_DOORBELL_0 66
#define BCM_FIQ_BCM_DOORBELL_1 67
#define BCM_FIQ_VPU0_HALTED    68
#define BCM_FIQ_VPU1_HALTED    69
#define BCM_FIQ_ILLEGAL_TYPE0  70
#define BCM_FIQ_ILLEGAL_TYPE1  71

#define BCM_MAX_FIQ            71

#else // BAREMETAL_RPI_TARGET >= 4

// FIQs
#define BCM_FIQ_TIMER1 BCM_IRQ_TIMER1
#define BCM_FIQ_GPIO3  BCM_IRQ_GPIO3
#define BCM_FIQ_UART   BCM_IRQ_UART

#define BCM_MAX_FIQ    IRQ_LINES

#endif // if BAREMETAL_RPI_TARGET <= 3

#define FIQ_INTR_ENABLE   BIT(7)
