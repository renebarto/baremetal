# Tutorial 27: SPI {#TUTORIAL_27_SPI}

@tableofcontents

## Tutorial setup {#TUTORIAL_27_SPI_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/27-SPI`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_27_SPI_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-27.a`
- a library `output/Debug/lib/device-27.a`
- a library `output/Debug/lib/stdlib-27.a`
- a library `output/Debug/lib/unittest-27.a`
- an application `output/Debug/bin/27-SPI.elf`
- an image in `deploy/Debug/27-SPI-image`

## SPI {#TUTORIAL_27_SPI_SPI}

In [Tutorial 06: Console UART1](06-console-uart1.md) and [Tutorial 11: UART0](11-uart0.md) we have covered serial communication using the UART peripherals.
In this tutorial we will cover another serial communication protocol: SPI.
You will see some similaries with the UART and I2C peripherals, but also some differences.

SPI stands for Serial Peripheral Interface and is a synchronous serial communication protocol.
It is commonly used for communication between microcontrollers and peripheral devices such as sensors, displays, DAC / ADC and EEPROM / flash memory chips.
The difference with UART and I2C is that SPI is a much higher bandwidth, as it can operate at much higher clock speeds and supports full-duplex communication.

SPI interfaces consist of a master device and one or more slave devices. The master device initiates communication and controls the clock signal, while the slave devices respond to the master's commands.
Raspberry Pi support both master and slave mode. This is similar to I2C.

SPI is a point-to-point communication protocol, meaning that each slave device is connected to the master device through a dedicated chip select (CE) line.
The master device uses the CE line to select which slave device it wants to communicate with.

Raspberry Pi supports multiple SPI peripherals, depending on the model:

| SPI peripheral       | Number of CE pins | Supported on model | Notes                                 |
|----------------------|-------------------|--------------------|---------------------------------------|
| SPI0 (master)        | 3                 | Raspberry Pi 3 / 4 |                                       |
| SPI0 (master)        | 4                 | Raspberry Pi 5     | Quad lane
| SPI1 (master)        | 3                 | Raspberry Pi 3 / 4 | Combined with UART1 in auxiliary device, also named mini SPI
| SPI1 (master)        | 3                 | Raspberry Pi 5     | Dual lane
| SPI2 (master)        | 3                 | Raspberry Pi 3 / 4 | Combined with UART1 in auxiliary device, also named mini SPI, not available on GPIO header
| SPI2 (master)        | 2                 | Raspberry Pi 5     | Dual lane
| SPI3 (master)        | 2                 | Raspberry Pi 4     | 
| SPI3 (master)        | 2                 | Raspberry Pi 5     | Dual lane
| SPI4 (master)        | 2                 | Raspberry Pi 4     | 
| SPI4 (slave)         | 1                 | Raspberry Pi 5     | Single lane
| SPI5 (master)        | 2                 | Raspberry Pi 4     | 
| SPI5 (master)        | 2                 | Raspberry Pi 5     | Dual lane
| SPI6 (master)        | 2                 | Raspberry Pi 4     | 
| SPI6 (master)        | 3                 | Raspberry Pi 5     | Dual lane, not available on GPIO header
| SPI7 (slave)         | 1                 | Raspberry Pi 5     | Single lane, not available on GPIO header
| SPI8 (master)        | 2                 | Raspberry Pi 5     | Dual lane, not available on GPIO header
| SPI slave device     | 1                 | Raspberry Pi 3 / 4 | Combined with I2C slave device        |

SPI0, 3, 4, 5, 6 on Raspberry Pi 3 and 4 are all programmed in the same way, see also [Raspberry Pi SPI](#RASPBERRY_PI_SPI).
SPI1 and SPI2 are part of the auxiliary device and have a different programming interface, but the same functionality, see also [Raspberry Pi Mini SPI (SPI1/2)](#RASPBERRY_PI_MINI_SPI_SPI12)
For Raspberry Pi 5, programming is different. We'll not cover this for now.

There are also some differences between the SPI peripherals, for example SPI0,3,4,5,6 have a FIFO buffer of 16 words of 32 bits for both transmit and receive and supports DMA, while SPI1 and SPI2 have a smaller FIFO buffer of 16 bytes for both transmit and receive and do not support DMA.
This means that performance of SPI1 and SPI2 is lower than SPI0,3,4,5,6.
Due to the limited capabilities of SPI1 and SPI2, we will focus on SPI0 in this tutorial. We will add the definitions for SPI1 and SPI2, but we will not use them in the code.

### Step 1 - SPI1 and SPI2 {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2}

Let's start with an extension of `BCMRegisters.h` to add the registers for SPI1 and SPI2.

#### BCMRegisters.h {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2_BCMREGISTERSH}

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
561: //---------------------------------------------
562: // Raspberry Pi auxiliary (SPI1 / SPI2 / UART1)
563: //---------------------------------------------
564: 
565: /// @brief Raspberry Pi Auxilary registers base address. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
566: #define RPI_AUX_BASE                           RPI_BCM_IO_BASE + 0x00215000
567: /// @brief Raspberry Pi Auxiliary IRQ register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
568: #define RPI_AUX_IRQ                            reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
569: /// @brief Raspberry Pi Auxiliary Enable register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
570: #define RPI_AUX_ENABLES                        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000004) // AUXENB
571: 
572: /// @brief Raspberry Pi Auxiliary Interrupt register values
573: /// @brief Raspberry Pi Auxiliary Interrupt register status SPI2. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
574: #define RPI_AUX_IRQ_SPI2                       BIT1(2)
575: /// @brief Raspberry Pi Auxiliary Interrupt register status SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
576: #define RPI_AUX_IRQ_SPI1                       BIT1(1)
577: /// @brief Raspberry Pi Auxiliary Interrupt register status UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
578: #define RPI_AUX_IRQ_UART1                      BIT1(0)
579: 
580: /// @brief Raspberry Pi Auxiliary Enable register values. The corresponding bit will need to be set in order to enable access to the peripheral's
581: /// registers and functionality. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
582: /// @brief Raspberry Pi Auxiliary Enable register Enable SPI2. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
583: #define RPI_AUX_ENABLES_SPI2                   BIT1(2)
584: /// @brief Raspberry Pi Auxiliary Enable register Enable SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
585: #define RPI_AUX_ENABLES_SPI1                   BIT1(1)
586: /// @brief Raspberry Pi Auxiliary Enable register Enable UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
587: #define RPI_AUX_ENABLES_UART1                  BIT1(0)
588: 
589: //---------------------------------------------
590: // Raspberry Pi auxiliary mini UART (UART1)
591: //---------------------------------------------
592: 
593: /// @brief Raspberry Pi Mini UART register base address. See @ref RASPBERRY_PI_UART1
594: #define RPI_AUX_MU_BASE                        RPI_AUX_BASE + 0x00000040
595: /// @brief Raspberry Pi Mini UART (UART1) I/O register. See @ref RASPBERRY_PI_UART1
596: #define RPI_AUX_MU_IO                          reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000000)
597: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register. See @ref RASPBERRY_PI_UART1
598: #define RPI_AUX_MU_IER                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000004)
599: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register. See @ref RASPBERRY_PI_UART1
600: #define RPI_AUX_MU_IIR                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000008)
601: /// @brief Raspberry Pi Mini UART (UART1) Line Control register. See @ref RASPBERRY_PI_UART1
602: #define RPI_AUX_MU_LCR                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x0000000C)
603: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register. See @ref RASPBERRY_PI_UART1
604: #define RPI_AUX_MU_MCR                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000010)
605: /// @brief Raspberry Pi Mini UART (UART1) Line Status register. See @ref RASPBERRY_PI_UART1
606: #define RPI_AUX_MU_LSR                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000014)
607: /// @brief Raspberry Pi Mini UART (UART1) Modem Status register. See @ref RASPBERRY_PI_UART1
608: #define RPI_AUX_MU_MSR                         reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000018)
609: /// @brief Raspberry Pi Mini UART (UART1) Scratch register. See @ref RASPBERRY_PI_UART1
610: #define RPI_AUX_MU_SCRATCH                     reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x0000001C)
611: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register. See @ref RASPBERRY_PI_UART1
612: #define RPI_AUX_MU_CNTL                        reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000020)
613: /// @brief Raspberry Pi Mini UART (UART1) Extra Status register. See @ref RASPBERRY_PI_UART1
614: #define RPI_AUX_MU_STAT                        reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000024)
615: /// @brief Raspberry Pi Mini UART (UART1) Baudrate register. See @ref RASPBERRY_PI_UART1
616: #define RPI_AUX_MU_BAUD                        reinterpret_cast<regaddr>(RPI_AUX_MU_BASE + 0x00000028)
617: /// @brief Calculate Raspberry Pi Mini UART (UART1) baud rate value from frequency
618: #define RPI_AUX_MU_BAUD_VALUE(clockRate, baud) static_cast<uint32>((clockRate / (baud * 8)) - 1)
619: 
620: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
621: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts. See @ref RASPBERRY_PI_UART1
622: #define RPI_AUX_MU_IER_TX_IRQ_ENABLE           BIT1(1)
623: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts. See @ref RASPBERRY_PI_UART1
624: #define RPI_AUX_MU_IER_RX_IRQ_ENABLE           BIT1(0)
625: 
626: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
627: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R). See @ref RASPBERRY_PI_UART1
628: #define RPI_AUX_MU_IIR_TX_FIFO_ENABLE          BIT1(7)
629: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R). See @ref RASPBERRY_PI_UART1
630: #define RPI_AUX_MU_IIR_RX_FIFO_ENABLE          BIT1(6)
631: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W). See @ref RASPBERRY_PI_UART1
632: #define RPI_AUX_MU_IIR_TX_FIFO_CLEAR           BIT1(2)
633: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref RASPBERRY_PI_UART1
634: #define RPI_AUX_MU_IIR_RX_FIFO_CLEAR           BIT1(1)
635: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref RASPBERRY_PI_UART1
636: #define RPI_AUX_MU_IIR_INTERRUPT_PENDING       BIT1(0)
637: 
638: /// @brief Raspberry Pi Mini UART (UART1) Line Control register values
639: /// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters. See @ref RASPBERRY_PI_UART1
640: #define RPI_AUX_MU_LCR_DATA_SIZE_7             0
641: /// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters. See @ref RASPBERRY_PI_UART1
642: #define RPI_AUX_MU_LCR_DATA_SIZE_8             BIT1(0) | BIT1(1)
643: 
644: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
645: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low. See @ref RASPBERRY_PI_UART1
646: #define RPI_AUX_MU_MCR_RTS_LOW                 BIT1(1)
647: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high. See @ref RASPBERRY_PI_UART1
648: #define RPI_AUX_MU_MCR_RTS_HIGH                BIT0(1)
649: 
650: /// @brief Raspberry Pi Mini UART (UART1) Line Status register values
651: /// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle. See @ref RASPBERRY_PI_UART1
652: #define RPI_AUX_MU_LSR_TX_IDLE                 BIT1(6)
653: /// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty. See @ref RASPBERRY_PI_UART1
654: #define RPI_AUX_MU_LSR_TX_EMPTY                BIT1(5)
655: /// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun. See @ref RASPBERRY_PI_UART1
656: #define RPI_AUX_MU_LSR_RX_OVERRUN              BIT1(1)
657: /// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready. See @ref RASPBERRY_PI_UART1
658: #define RPI_AUX_MU_LSR_RX_READY                BIT1(0)
659: 
660: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
661: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS. See @ref RASPBERRY_PI_UART1
662: #define RPI_AUX_MU_CNTL_ENABLE_CTS             BIT1(3)
663: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS. See @ref RASPBERRY_PI_UART1
664: #define RPI_AUX_MU_CNTL_ENABLE_RTS             BIT1(2)
665: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit. See @ref RASPBERRY_PI_UART1
666: #define RPI_AUX_MU_CNTL_ENABLE_TX              BIT1(1)
667: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive. See @ref RASPBERRY_PI_UART1
668: #define RPI_AUX_MU_CNTL_ENABLE_RX              BIT1(0)
669: 
670: /// @todo extend with register values for RPI_AUX_MU_STAT, RPI_AUX_MU_BAUD
671: 
672: //---------------------------------------------
673: // Raspberry Pi auxiliary mini SPI (SPI1/2)
674: //---------------------------------------------
675: 
676: /// @brief Raspberry Pi Mini SPI1 register base address. See @ref RASPBERRY_PI_MINI_SPI_SPI12
677: #define RPI_AUX_SPI1_BASE                      reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000080)
678: /// @brief Raspberry Pi Mini SPI1 Control register 0. See @ref RASPBERRY_PI_MINI_SPI_SPI12
679: #define AUX_SPI1_CNTL0_REG                     reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x00000000)
680: /// @brief Raspberry Pi Mini SPI1 Control register 1. See @ref RASPBERRY_PI_MINI_SPI_SPI12
681: #define AUX_SPI1_CNTL1_REG                     reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x00000004)
682: /// @brief Raspberry Pi Mini SPI1 Status register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
683: #define AUX_SPI1_STAT_REG                      reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x00000008)
684: /// @brief Raspberry Pi Mini SPI1 Peek register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
685: #define AUX_SPI1_PEEK_REG                      reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x0000000C)
686: /// @brief Raspberry Pi Mini SPI1 I/O register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
687: #define AUX_SPI1_IO_REG                        reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x00000020)
688: /// @brief Raspberry Pi Mini SPI1 TXHold register 0. See @ref RASPBERRY_PI_MINI_SPI_SPI12
689: #define AUX_SPI1_TXHOLD_REG                    reinterpret_cast<regaddr>(RPI_AUX_SPI1_BASE + 0x00000030)
690: 
691: /// @brief Raspberry Pi Mini SPI2 register base address. See @ref RASPBERRY_PI_MINI_SPI_SPI12
692: #define RPI_AUX_SPI2_BASE                      reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x000000C0)
693: /// @brief Raspberry Pi Mini SPI2 Control register 0. See @ref RASPBERRY_PI_MINI_SPI_SPI12
694: #define AUX_SPI2_CNTL0_REG                     reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x00000000)
695: /// @brief Raspberry Pi Mini SPI2 Control register 1. See @ref RASPBERRY_PI_MINI_SPI_SPI12
696: #define AUX_SPI2_CNTL1_REG                     reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x00000004)
697: /// @brief Raspberry Pi Mini SPI2 Status register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
698: #define AUX_SPI2_STAT_REG                      reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x00000008)
699: /// @brief Raspberry Pi Mini SPI2 Peek register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
700: #define AUX_SPI2_PEEK_REG                      reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x0000000C)
701: /// @brief Raspberry Pi Mini SPI2 I/O register. See @ref RASPBERRY_PI_MINI_SPI_SPI12
702: #define AUX_SPI2_IO_REG                        reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x00000020)
703: /// @brief Raspberry Pi Mini SPI2 TXHold register 0. See @ref RASPBERRY_PI_MINI_SPI_SPI12
704: #define AUX_SPI2_TXHOLD_REG                    reinterpret_cast<regaddr>(RPI_AUX_SPI2_BASE + 0x00000030)
705: 
706: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 values
707: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed bit shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
708: #define RPI_AUX_SPI_CNTL0_SPEED_SHIFT          20
709: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
710: #define RPI_AUX_SPI_CNTL0_SPEED_MASK           BITS(RPI_AUX_SPI_CNTL0_SPEED_SHIFT, RPI_AUX_SPI_CNTL0_SPEED_SHIFT + 11)
711: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
712: #define RPI_AUX_SPI_CNTL0_SPEED_VALUE(x)       static_cast<uint32>((x & RPI_AUX_SPI_CNTL0_SPEED_MASK) >> RPI_AUX_SPI_CNTL0_SPEED_SHIFT)
713: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed value minimum value. See @ref RASPBERRY_PI_MINI_SPI_SPI12
714: #define RPI_AUX_SPI_CNTL0_SPEED_MIN            0
715: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed value maximum value. See @ref RASPBERRY_PI_MINI_SPI_SPI12
716: #define RPI_AUX_SPI_CNTL0_SPEED_MAX            ((1 << 12) - 1)
717: 
718: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip selects bit shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
719: #define RPI_AUX_SPI_CNTL0_CS_SHIFT             17
720: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip selects bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
721: #define RPI_AUX_SPI_CNTL0_CS_MASK              BITS(RPI_AUX_SPI_CNTL0_CS_SHIFT, RPI_AUX_SPI_CNTL0_CS_SHIFT + 2)
722: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip selects bit value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
723: #define RPI_AUX_SPI_CNTL0_CS_VALUE(x)          static_cast<uint32>((x & RPI_AUX_SPI_CNTL0_CS_MASK) >> RPI_AUX_SPI_CNTL0_CS_SHIFT)
724: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip select 0 low. See @ref RASPBERRY_PI_MINI_SPI_SPI12
725: #define RPI_AUX_SPI_CNTL0_CS0_N                0x000C0000   // CS 0 low
726: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip select 1 low. See @ref RASPBERRY_PI_MINI_SPI_SPI12
727: #define RPI_AUX_SPI_CNTL0_CS1_N                0x000A0000   // CS 1 low
728: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 chip select 2 low. See @ref RASPBERRY_PI_MINI_SPI_SPI12
729: #define RPI_AUX_SPI_CNTL0_CS2_N                0x00060000   // CS 2 low
730: 
731: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 post input mode. See @ref RASPBERRY_PI_MINI_SPI_SPI12
732: #define RPI_AUX_SPI_CNTL0_POST_INPUT_MODE      BIT1(16)
733: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 variable CS. See @ref RASPBERRY_PI_MINI_SPI_SPI12
734: #define RPI_AUX_SPI_CNTL0_VARIABLE_CS          BIT1(15)
735: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 variable width. See @ref RASPBERRY_PI_MINI_SPI_SPI12
736: #define RPI_AUX_SPI_CNTL0_VARIABLE_WIDTH       BIT1(14)
737: 
738: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 DOUT hold time bit shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
739: #define RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_SHIFT 12
740: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 DOUT hold time bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
741: #define RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_MASK  BITS(RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_SHIFT, RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_SHIFT + 1)
742: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 DOUT hold time value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
743: #define RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_VALUE(x) static_cast<uint32>((x & RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_MASK) >> RPI_AUX_SPI_CNTL0_DOUT_HOLD_TIME_SHIFT)
744: 
745: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 enable. See @ref RASPBERRY_PI_MINI_SPI_SPI12
746: #define RPI_AUX_SPI_CNTL0_ENABLE               BIT1(11)
747: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 in rising. See @ref RASPBERRY_PI_MINI_SPI_SPI12
748: #define RPI_AUX_SPI_CNTL0_IN_RISING            BIT1(10)
749: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 clear FIFO. See @ref RASPBERRY_PI_MINI_SPI_SPI12
750: #define RPI_AUX_SPI_CNTL0_CLEAR_FIFO           BIT1(9)
751: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 out rising. See @ref RASPBERRY_PI_MINI_SPI_SPI12
752: #define RPI_AUX_SPI_CNTL0_OUT_RISING           BIT1(8)
753: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 invert clock. See @ref RASPBERRY_PI_MINI_SPI_SPI12
754: #define RPI_AUX_SPI_CNTL0_INVERT_CLOCK         BIT1(7)
755: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift out MSB first. See @ref RASPBERRY_PI_MINI_SPI_SPI12
756: #define RPI_AUX_SPI_CNTL0_SHIFT_OUT_MSB_FIRST  BIT1(6)
757: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift out LSB first. See @ref RASPBERRY_PI_MINI_SPI_SPI12
758: #define RPI_AUX_SPI_CNTL0_SHIFT_OUT_LSB_FIRST  BIT0(6)
759: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift length bit shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
760: #define RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_SHIFT   0
761: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift length bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
762: #define RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_MASK    BITS(RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_SHIFT, RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_SHIFT + 1)
763: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift length value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
764: #define RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_VALUE(x) static_cast<uint32>((x & RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_MASK) >> RPI_AUX_SPI_CNTL0_SHIFT_LENGTH_SHIFT)
765: 
766: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 values
767: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 CS high time bit shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
768: #define RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_SHIFT   8
769: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 CS high time bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
770: #define RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_MASK    BITS(RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_SHIFT, RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_SHIFT + 2)
771: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 CS high time value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
772: #define RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_VALUE(x) static_cast<uint32>((x & RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_MASK) >> RPI_AUX_SPI_CNTL1_CS_HIGH_TIME_SHIFT)
773: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 TX empty interrupt enable. See @ref RASPBERRY_PI_MINI_SPI_SPI12
774: #define RPI_AUX_SPI_CNTL1_TX_EMPTY_IRQ         BIT1(7)
775: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 1 DONE interrupt enable. See @ref RASPBERRY_PI_MINI_SPI_SPI12
776: #define RPI_AUX_SPI_CNTL1_DONE_IRQ             BIT1(6)
777: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift in MSB first. See @ref RASPBERRY_PI_MINI_SPI_SPI12
778: #define RPI_AUX_SPI_CNTL1_SHIFT_IN_MSB_FIRST   BIT1(1)
779: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 shift in LSB first. See @ref RASPBERRY_PI_MINI_SPI_SPI12
780: #define RPI_AUX_SPI_CNTL1_SHIFT_IN_LSB_FIRST   BIT0(1)
781: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 keep input. See @ref RASPBERRY_PI_MINI_SPI_SPI12
782: #define RPI_AUX_SPI_CNTL1_KEEP_INPUT           BIT1(0)
783: 
784: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register values
785: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register TX FIFO level shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
786: #define RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_SHIFT   24
787: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register TX FIFO level bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
788: #define RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_MASK    BITS(RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_SHIFT, RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_SHIFT + 3)
789: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register TX FIFO level value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
790: #define RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_VALUE(x) static_cast<uint32>((x & RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_MASK) >> RPI_AUX_SPI_STAT_TX_FIFO_LEVEL_SHIFT)
791: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register RX FIFO level shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
792: #define RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_SHIFT   16
793: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register RX FIFO level bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
794: #define RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_MASK    BITS(RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_SHIFT, RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_SHIFT + 3)
795: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register RX FIFO level value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
796: #define RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_VALUE(x) static_cast<uint32>((x & RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_MASK) >> RPI_AUX_SPI_STAT_RX_FIFO_LEVEL_SHIFT)
797: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register TX full. See @ref RASPBERRY_PI_MINI_SPI_SPI12
798: #define RPI_AUX_SPI_STAT_TX_FULL               BIT1(10)
799: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register TX empty. See @ref RASPBERRY_PI_MINI_SPI_SPI12
800: #define RPI_AUX_SPI_STAT_TX_EMPTY              BIT1(9)
801: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register RX full. See @ref RASPBERRY_PI_MINI_SPI_SPI12
802: #define RPI_AUX_SPI_STAT_RX_FULL               BIT1(8)
803: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register RX empty. See @ref RASPBERRY_PI_MINI_SPI_SPI12
804: #define RPI_AUX_SPI_STAT_RX_EMPTY              BIT1(7)
805: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register busy. See @ref RASPBERRY_PI_MINI_SPI_SPI12
806: #define RPI_AUX_SPI_STAT_BUSY                  BIT1(6)
807: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register bit count shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
808: #define RPI_AUX_SPI_STAT_BIT_COUNT_SHIFT       0
809: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register bit count bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
810: #define RPI_AUX_SPI_STAT_BIT_COUNT_MASK        BITS(RPI_AUX_SPI_STAT_BIT_COUNT_SHIFT, RPI_AUX_SPI_STAT_BIT_COUNT_SHIFT + 5)
811: /// @brief Raspberry Pi Mini SPI (SPI1/2) Status register bit count value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
812: #define RPI_AUX_SPI_STAT_BIT_COUNT_VALUE(x)    static_cast<uint32>((x & RPI_AUX_SPI_STAT_BIT_COUNT_MASK) >> RPI_AUX_SPI_STAT_BIT_COUNT_SHIFT)
813: 
814: /// @brief Raspberry Pi Mini SPI (SPI1/2) Peek register values
815: /// @brief Raspberry Pi Mini SPI (SPI1/2) Peek register data shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
816: #define RPI_AUX_SPI_PEEK_DATA_SHIFT            0
817: /// @brief Raspberry Pi Mini SPI (SPI1/2) Peek register data bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
818: #define RPI_AUX_SPI_PEEK_DATA_MASK             BITS(RPI_AUX_SPI_PEEK_DATA_SHIFT, RPI_AUX_SPI_PEEK_DATA_SHIFT + 15)
819: /// @brief Raspberry Pi Mini SPI (SPI1/2) Peek register data value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
820: #define RPI_AUX_SPI_PEEK_DATA_VALUE(x)         static_cast<uint32>((x & RPI_AUX_SPI_PEEK_DATA_MASK) >> RPI_AUX_SPI_PEEK_DATA_SHIFT)
821: 
822: /// @brief Raspberry Pi Mini SPI (SPI1/2) I/O register values
823: /// @brief Raspberry Pi Mini SPI (SPI1/2) I/O register data shift. See @ref RASPBERRY_PI_MINI_SPI_SPI12
824: #define RPI_AUX_SPI_IO_DATA_SHIFT              0
825: /// @brief Raspberry Pi Mini SPI (SPI1/2) I/O register data bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
826: #define RPI_AUX_SPI_IO_DATA_MASK               BITS(RPI_AUX_SPI_IO_DATA_SHIFT, RPI_AUX_SPI_IO_DATA_SHIFT + 15)
827: /// @brief Raspberry Pi Mini SPI (SPI1/2) I/O register data value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
828: #define RPI_AUX_SPI_IO_DATA_VALUE(x)           static_cast<uint32>((x & RPI_AUX_SPI_IO_DATA_MASK) >> RPI_AUX_SPI_IO_DATA_SHIFT)
```

- Line 573-574: We define `RPI_AUX_IRQ_SPI2` to enable interrupts for SPI2
- Line 575-576: We define `RPI_AUX_IRQ_SPI1` to enable interrupts for SPI1
- Line 577-578: We define `RPI_AUX_IRQ_UART1` to enable interrupts for UART1
- Line 593-594: We define `RPI_AUX_MU_BASE` to be the base address of the Mini UART (UART1) registers. This is more in line with other devices
- Line 595-616: We use `RPI_AUX_MU_BASE` as the base address to define the registers for the Mini UART (UART1)
- Line 635-636: We add a definition `RPI_AUX_MU_IIR_INTERRUPT_PENDING` to check if an interrupt is pending for UART1, SPI1 or SPI2
- Line 676-689: We define `RPI_AUX_SPI1_BASE` to be the base address of the SPI1 registers, and we define the registers for SPI1
- Line 691-704: We define `RPI_AUX_SPI2_BASE` to be the base address of the SPI2 registers, and we define the registers for SPI2
- Line 706-764: We define the values for the CNTL0 register of SPI1 and SPI2.
 For multi-bit fields, we define the shift and mask, as well as a helper to read the value of the field
- Line 766-782: We define the values for the CNTL1 register of SPI1 and SPI2
- Line 784-812: We define the values for the STAT register of SPI1 and SPI2
- Line 814-820: We define the values for the PEEK register of SPI1 and SPI2
- Line 822-828: We define the values for the I/O register of SPI1 and SPI2

#### ISPIMaster.h {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2_ISPIMASTERH}

We'll declare the interface class `ISPIMaster` to represent the common interface of all SPI peripherals.

Add the file `code/libraries/baremetal/include/baremetal/ISPIMaster.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ISPIMaster.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : ISPIMaster.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SPIMaster
9: //
10: // Description : SPI Master abstract interface
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "stdlib/Types.h"
43: 
44: /// @file
45: /// SPI Master abstract interface
46: 
47: namespace baremetal {
48: 
49: #if BAREMETAL_RPI_TARGET <= 4
50: 
51: /// <summary>
52: /// Chip select / chip enable index
53: /// </summary>
54: enum class SPI_CEIndex : uint8
55: {
56:     /// @brief CE0 / CS0 signal
57:     CE0 = 0,
58:     /// @brief CE1 / CS1 signal
59:     CE1 = 1,
60:     /// @brief CE2 / CS2 signal
61:     CE2 = 2,
62:     /// @brief None selected
63:     None = 3
64: };
65: 
66: /// <summary>
67: /// SPIMaster abstract interface. Can be inherited for creating a mock
68: /// </summary>
69: class ISPIMaster
70: {
71: public:
72:     /// <summary>
73:     /// Default destructor needed for abstract class
74:     /// </summary>
75:     virtual ~ISPIMaster() = default;
76: 
77:     /// <summary>
78:     /// Set SPI clock rate in Hz. The actual clock rate may be different depending on the hardware capabilities and the core clock rate, but it should
79:     /// be the closest possible to the requested clock rate without exceeding it.
80:     /// </summary>
81:     /// <param name="clockRate">Requested clock rate</param>
82:     virtual void SetClock(uint32 clockRate) = 0;
83: 
84:     /// <summary>
85:     /// Read bytes into buffer
86:     /// </summary>
87:     /// <param name="ceIndex">Index of CE/CS signal to activate</param>
88:     /// <param name="buffer">Pointer to buffer to store data received</param>
89:     /// <param name="count">Requested byte count for read</param>
90:     /// <returns>Number of bytes actually read, or negative if an error occurs</returns>
91:     virtual size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) = 0;
92:     /// <summary>
93:     /// Write bytes to device
94:     /// </summary>
95:     /// <param name="ceIndex">Index of CE/CS signal to activate</param>
96:     /// <param name="buffer">Pointer to buffer containing data to be sent</param>
97:     /// <param name="count">Requested byte count for write</param>
98:     /// <returns>Number of bytes actually written, or negative if an error occurs</returns>
99:     virtual size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) = 0;
100:     /// <summary>
101:     /// Write then read from device
102:     /// </summary>
103:     /// <param name="ceIndex">Index of CE/CS signal to activate</param>
104:     /// <param name="writeBuffer">Pointer to buffer containing data to be sent</param>
105:     /// <param name="readBuffer">Pointer to buffer to store data received</param>
106:     /// <param name="count">Requested byte count to read / write</param>
107:     /// <returns>Number of bytes actually written / read, or negative if an error occurs</returns>
108:     virtual size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) = 0;
109: };
110: 
111: #else
112: 
113: #error RPI 5 not supported yet
114: 
115: #endif
116: 
117: } // namespace baremetal
```

- Line 51-64: We define the `SPI_CEIndex` enum to represent the chip select / chip enable signals for SPI devices. For Raspberry Pi 3 and 4, there are 3 CE signals (CE0, CE1, CE2) and a None option. Raspberry Pi 5 is not covered yet
- Line 66-109: We declare the abstract class `ISPIMaster` to represent the common interface for SPI master devices
    - Line 72-75: We need a virtual destructor for an abstract class
    - Line 77-82: The method `SetClock()` is used to set the SPI clock rate
    - Line 84-91: The method `Read() is used to read data from a SPI device. It needs a CE signal, and receives a buffer to place received data, as well as the number of bytes to receive
    - Line 92-99: The method `Write() is used to write data to a SPI device. It needs a CE signal, and receives a buffer holding the data to send, as well as the number of bytes to send
    - Line 100-108: The method `WriteRead() is used to write and read data to and from a SPI device. It needs a CE signal, and receives a buffer holsing the data to send, a buffer to place received data, as well as the number of bytes to send and receive

#### SPIMasterAux.h {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2_SPIMASTERAUXH}

We'll declare the class `SPIMasterAux` to represent the SPI1 and SPI2 peripherals. This implements the `ISPIMaster` interface.

Add the file `code/libraries/baremetal/include/baremetal/SPIMasterAux.h`

```cpp
File: code/libraries/baremetal/include/baremetal/SPIMasterAux.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : SPIMasterAux.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SPIMasterAux
9: //
10: // Description : SPI Master functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/ISPIMaster.h"
44: #include "baremetal/IMemoryAccess.h"
45: #include "baremetal/PhysicalGPIOPin.h"
46: 
47: /// @file
48: /// SPI Master
49: 
50: namespace baremetal {
51: 
52: /// <summary>
53: /// Driver for SPI master devices
54: ///
55: /// GPIO pin mapping (Raspberry Pi 3-4)
56: /// Device | CE2    CE1    CE0    MISO   MOSI   SCLK   | Boards
57: /// :----: | :---------------------------------------: | :-----
58: /// 1      | GPIO16 GPIO17 GPIO18 GPIO19 GPIO20 GPIO21 | Raspberry Pi 3 / 4
59: /// 2      |                                           | Not usable
60: ///
61: /// GPIO pin mapping (Raspberry Pi 5)
62: /// No Aux peripheral, no SPI devices
63: /// </summary>
64: class SPIMasterAux : public ISPIMaster
65: {
66: private:
67:     /// @brief Memory access interface reference for accessing registers.
68:     IMemoryAccess& m_memoryAccess;
69:     /// @brief SPI device index
70:     uint8 m_device;
71:     /// @brief SPI device base register address
72:     regaddr m_baseAddress;
73:     /// @brief SPI clock rate (Hz)
74:     uint32 m_clockRate;
75:     /// @brief SPI clock divider
76:     uint16 m_clockDivider;
77:     /// @brief Core clock rate used to determine SPI clock rate in Hz
78:     unsigned m_coreClockRate;
79:     /// @brief True if class is already initialized
80:     bool m_isInitialized;
81: 
82:     /// @brief GPIO pin for SCLK wire
83:     PhysicalGPIOPin m_sclkPin;
84:     /// @brief GPIO pin for MOSI wire
85:     PhysicalGPIOPin m_mosiPin;
86:     /// @brief GPIO pin for MISO wire
87:     PhysicalGPIOPin m_misoPin;
88:     /// @brief GPIO pin for CE0 wire
89:     PhysicalGPIOPin m_ce0Pin;
90:     /// @brief GPIO pin for CE1 wire
91:     PhysicalGPIOPin m_ce1Pin;
92:     /// @brief GPIO pin for CE2 wire
93:     PhysicalGPIOPin m_ce2Pin;
94: 
95: public:
96:     SPIMasterAux(IMemoryAccess& memoryAccess = GetMemoryAccess());
97: 
98:     ~SPIMasterAux();
99: 
100:     bool Initialize(uint8 device, uint32 clockRate = 500000);
101: 
102:     void SetClock(uint32 clockRate) override;
103:     size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) override;
104:     size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) override;
105:     size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) override;
106: 
107: private:
108: };
109: 
110: } // namespace baremetal
```

- Line 52-109: We declare the class `SPIMasterAux` deriving from `ISPIMaster`
    - Line 67-68: The member variable `m_memoryAccess` holds a reference to the memory access interface
    - Line 69-70: The member variable `m_device` holds the SPI device index (1 for SPI1, 2 for SPI2)
    - Line 71-72: The member variable `m_baseAddress` holds the register base address for the selected SPI master
    - Line 73-74: The member variable `m_clockRate` holds the SPI clock rate in Hz
    - Line 75-76: The member variable `m_clockDivider` holds the SPI clock divider value used to achieve the requested clock rate
    - Line 77-78: The member variable `m_coreClockRate` holds the Raspberry Pi core clock frequency in Hz, which is divided by the clock divider to achieve the requested clock rate
    - Line 79-80: The member variable `m_isInitialized` is true if the class has been initialized, to guard against multiple initialization
    - Line 82-85: The member variable `m_sclkPin` is the GPIO pin for the SCLK wire
    - Line 84-87: The member variable `m_mosiPin` is the GPIO pin for the MOSI wire
    - Line 86-85: The member variable `m_misoPin` is the GPIO pin for the MISO wire
    - Line 88-89: The member variable `m_ce0Pin` is the GPIO pin for the CE0 wire
    - Line 90-91: The member variable `m_ce1Pin` is the GPIO pin for the CE1 wire
    - Line 92-93: The member variable `m_ce2Pin` is the GPIO pin for the CE2 wire
    - Line 96: We declare the constructor, which takes a memory access interface reference
    - Line 98: We declare the destructor
    - Line 100: We declare the method `Initialize()` which initializes the SPI master
    - Line 102: We declare the method `SetClock()` to override the interface method
    - Line 103: We declare the method `Read()` to override the interface method
    - Line 104: We declare the method `Write()` to override the interface method
    - Line 105: We declare the method `WriteRead()` to override the interface method

#### SPIMasterAux.cpp {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2_SPIMASTERAUXCPP}

We'll implement the `SPIMasterAux` class.

Add the file `code/libraries/baremetal/src/SPIMasterAux.cpp`

```cpp
File: code/libraries/baremetal/src/SPIMasterAux.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : SPIMasterAux.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SPIMasterAux
9: //
10: // Description : SPI Master functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "baremetal/SPIMasterAux.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/MachineInfo.h"
45: #include "baremetal/String.h"
46: #include "baremetal/Timer.h"
47: 
48: /// @file
49: /// SPIMasterAux
50: 
51: using namespace baremetal;
52: 
53: /// @brief Define log name
54: LOG_MODULE("SPIMasterAux");
55: 
56: #if BAREMETAL_RPI_TARGET <= 4
57: /// @brief Number of SPI devices for RPI 3
58: #define SPI_DEVICES 2
59: #endif
60: 
61: /// @brief Total number of GPIO pins used for SPI (CE2 / CE1 / CE0 / MISO / MOSI / SCLK)
62: #define SPI_WIRES             6
63: /// @brief Index of SPI CE2 pin
64: #define SPI_GPIO_CE2          0
65: /// @brief Index of SPI CE1 pin
66: #define SPI_GPIO_CE1          1
67: /// @brief Index of SPI CE1 pin
68: #define SPI_GPIO_CE0          2
69: /// @brief Index of SPI CE1 pin
70: #define SPI_GPIO_MISO         3
71: /// @brief Index of SPI CE1 pin
72: #define SPI_GPIO_MOSI         4
73: /// @brief Index of SPI CE1 pin
74: #define SPI_GPIO_SCLK         5
75: 
76: /// @brief Value in configuration table to flag invalid combination
77: #define NONE                  10000
78: 
79: /// <summary>
80: /// Constructor for SPIMasterAux
81: ///
82: /// The default value for memoryAccess will use the singleton MemoryAccess instance. A different reference to a IMemoryAccess instance can be passed for testing
83: /// </summary>
84: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
85: SPIMasterAux::SPIMasterAux(IMemoryAccess& memoryAccess /* = GetMemoryAccess()*/)
86:     : m_memoryAccess{memoryAccess}
87:     , m_device{}
88:     , m_baseAddress{}
89:     , m_clockRate{}
90:     , m_clockDivider{}
91:     , m_coreClockRate{}
92:     , m_isInitialized{}
93:     , m_sclkPin{}
94:     , m_mosiPin{}
95:     , m_misoPin{}
96:     , m_ce0Pin{}
97:     , m_ce1Pin{}
98:     , m_ce2Pin{}
99: {
100: }
101: 
102: /// <summary>
103: /// Destructor for SPIMasterAux
104: /// </summary>
105: SPIMasterAux::~SPIMasterAux()
106: {
107:     if (m_isInitialized)
108:     {
109:         m_memoryAccess.Write32(RPI_AUX_ENABLES, m_memoryAccess.Read32(RPI_AUX_ENABLES) & ~RPI_AUX_ENABLES_SPI1);
110: 
111:         m_sclkPin.SetMode(GPIOMode::InputPullUp);
112:         m_mosiPin.SetMode(GPIOMode::InputPullUp);
113:         m_misoPin.SetMode(GPIOMode::InputPullUp);
114:         m_ce0Pin.SetMode(GPIOMode::InputPullUp);
115:         m_ce1Pin.SetMode(GPIOMode::InputPullUp);
116:         m_ce2Pin.SetMode(GPIOMode::InputPullUp);
117:     }
118:     m_isInitialized = false;
119:     m_baseAddress = nullptr;
120: }
121: 
122: /// <summary>
123: /// Initialize the SPIMasterAux for a specific device, setting the clock as specified.
124: /// </summary>
125: /// <param name="device">SPI device index</param>
126: /// <param name="clockRate">SPI clock rate to be used in Hz</param>
127: /// <returns>True on success, false on failure</returns>
128: bool SPIMasterAux::Initialize(uint8 device, uint32 clockRate /*= 500000*/)
129: {
130:     if (m_isInitialized)
131:         return true;
132: 
133:     LOG_INFO("Initialize SPI device %d", device);
134:     if ((device != 1))
135:         return false;
136: 
137:     m_device = device;
138:     m_baseAddress = RPI_AUX_SPI1_BASE;
139:     m_coreClockRate = GetMachineInfo().GetClockRate(ClockID::CORE);
140: 
141:     assert(m_baseAddress != 0);
142: 
143:     m_memoryAccess.Write32(RPI_AUX_ENABLES, m_memoryAccess.Read32(RPI_AUX_ENABLES) | RPI_AUX_ENABLES_SPI1);
144:     m_memoryAccess.Write32(AUX_SPI1_CNTL1_REG, 0);
145:     m_memoryAccess.Write32(AUX_SPI1_CNTL0_REG, RPI_AUX_SPI_CNTL0_CLEAR_FIFO);
146:     m_ce2Pin.AssignPin(16);
147:     m_ce2Pin.SetMode(GPIOMode::AlternateFunction4);
148:     m_ce1Pin.AssignPin(17);
149:     m_ce1Pin.SetMode(GPIOMode::AlternateFunction4);
150:     m_ce0Pin.AssignPin(18);
151:     m_ce0Pin.SetMode(GPIOMode::AlternateFunction4);
152:     m_sclkPin.AssignPin(19);
153:     m_sclkPin.SetMode(GPIOMode::AlternateFunction4);
154:     m_mosiPin.AssignPin(20);
155:     m_mosiPin.SetMode(GPIOMode::AlternateFunction4);
156:     m_misoPin.AssignPin(21);
157:     m_misoPin.SetMode(GPIOMode::AlternateFunction4);
158: 
159:     assert(m_coreClockRate > 0);
160: 
161:     m_isInitialized = true;
162: 
163:     SetClock(clockRate);
164: 
165:     LOG_INFO("Set up SPI device %d, clock rate %d, base address %08X", device, clockRate, m_baseAddress);
166:     return true;
167: }
168: 
169: /// <summary>
170: /// Set SPI clock rate
171: /// </summary>
172: /// <param name="clockRate">Clock rate in Hz</param>
173: void SPIMasterAux::SetClock(unsigned clockRate)
174: {
175:     assert(m_isInitialized);
176: 
177:     assert(clockRate > 0);
178:     m_clockRate = clockRate;
179: 
180:     m_clockDivider = MIN(static_cast<uint16>(((m_coreClockRate + clockRate - 1) / (2 * clockRate)) - 1), RPI_AUX_SPI_CNTL0_SPEED_MAX);
181: 
182:     LOG_INFO("Set clock %d", clockRate);
183: }
184: 
185: size_t SPIMasterAux::Read(SPI_CEIndex ceIndex, void* buffer, size_t count)
186: {
187:     return WriteRead(ceIndex, nullptr, buffer, count);
188: }
189: 
190: size_t SPIMasterAux::Write(SPI_CEIndex ceIndex, const void* buffer, size_t count)
191: {
192:     return WriteRead(ceIndex, buffer, nullptr, count);
193: }
194: 
195: size_t SPIMasterAux::WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count)
196: {
197:     assert(m_isInitialized);
198: 
199:     assert(writeBuffer != nullptr || readBuffer != nullptr);
200:     const uint8* writeData = reinterpret_cast<const uint8*>(writeBuffer);
201:     uint8* readData = reinterpret_cast<uint8*>(readBuffer);
202: 
203:     assert(count > 0);
204:     size_t writeCount = count;
205:     size_t readCount = count;
206: 
207:     uint32 control0Value = m_clockDivider << RPI_AUX_SPI_CNTL0_SPEED_SHIFT;
208: 
209:     switch (ceIndex)
210:     {
211:     case SPI_CEIndex::CE0:
212:         control0Value |= RPI_AUX_SPI_CNTL0_CS0_N;
213:         break;
214: 
215:     case SPI_CEIndex::CE1:
216:         control0Value |= RPI_AUX_SPI_CNTL0_CS1_N;
217:         break;
218: 
219:     case SPI_CEIndex::CE2:
220:         control0Value |= RPI_AUX_SPI_CNTL0_CS2_N;
221:         break;
222: 
223:     default:
224:         assert(false);
225:         break;
226:     }
227: 
228:     control0Value |= RPI_AUX_SPI_CNTL0_ENABLE;
229:     control0Value |= RPI_AUX_SPI_CNTL0_SHIFT_OUT_MSB_FIRST;
230:     control0Value |= RPI_AUX_SPI_CNTL0_VARIABLE_WIDTH;
231:     m_memoryAccess.Write32(AUX_SPI1_CNTL0_REG, control0Value);
232: 
233:     m_memoryAccess.Write32(AUX_SPI1_CNTL1_REG, RPI_AUX_SPI_CNTL1_SHIFT_IN_MSB_FIRST);
234: 
235:     while ((writeCount > 0) || (readCount > 0))
236:     {
237:         while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_TX_FULL) && (writeCount > 0))
238:         {
239:             uint32 numBytes = MIN(writeCount, 3);
240:             uint32 data = 0;
241: 
242:             for (unsigned i = 0; i < numBytes; i++)
243:             {
244:                 uint8 nByte = (writeData != 0) ? *writeData++ : 0;
245:                 data |= nByte << (8 * (2 - i));
246:             }
247: 
248:             data |= (numBytes * 8) << 24;
249:             writeCount -= numBytes;
250: 
251:             if (writeCount != 0)
252:             {
253:                 m_memoryAccess.Write32(AUX_SPI1_TXHOLD_REG, data);
254:             }
255:             else
256:             {
257:                 m_memoryAccess.Write32(AUX_SPI1_IO_REG, data);
258:             }
259:         }
260: 
261:         while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_RX_EMPTY) && (readCount > 0))
262:         {
263:             uint32 numBytes = MIN(readCount, 3);
264:             uint32 data = m_memoryAccess.Read32(AUX_SPI1_IO_REG);
265: 
266:             if (readBuffer != 0)
267:             {
268:                 switch (numBytes)
269:                 {
270:                 case 3:
271:                     *readData++ = (uint8)((data >> 16) & 0xFF);
272:                     // fall through
273: 
274:                 case 2:
275:                     *readData++ = (uint8)((data >> 8) & 0xFF);
276:                     // fall through
277: 
278:                 case 1:
279:                     *readData++ = (uint8)((data >> 0) & 0xFF);
280:                 }
281:             }
282: 
283:             readCount -= numBytes;
284:         }
285: 
286:         while (!(m_memoryAccess.Read32(AUX_SPI1_STAT_REG) & RPI_AUX_SPI_STAT_BUSY) && (readCount > 0))
287:         {
288:             uint32 numBytes = MIN(readCount, 3);
289:             uint32 data = m_memoryAccess.Read32(AUX_SPI1_STAT_REG);
290: 
291:             if (readBuffer != 0)
292:             {
293:                 switch (numBytes)
294:                 {
295:                 case 3:
296:                     *readData++ = (uint8) ((data >> 16) & 0xFF);
297:                     // fall through
298: 
299:                 case 2:
300:                     *readData++ = (uint8) ((data >> 8) & 0xFF);
301:                     // fall through
302: 
303:                 case 1:
304:                     *readData++ = (uint8) ((data >> 0) & 0xFF);
305:                 }
306:             }
307: 
308:             readCount -= numBytes;
309:         }
310:     }
311: 
312:     int result = 0;
313: 
314:     return result;
315: }
```

- Line 57-58: We define `SPI_DEVICES` as the number of available devices on Raspberry Pi 3 and 4, which is 2 (SPI1 and SPI2). Raspberry Pi 5 has no mini SPI devices
- Line 61-62: We define `SPI_WIRES` as the total number of GPIO pins used for SPI, which is 6 (CE2, CE1, CE0, MISO, MOSI, SCLK)
- Line 63-74: We define constants for the index of each SPI wire in the GPIO configuration table
- Line 76-77: We define `NONE` as a value int the configuration table to denote an invalid entry
- Line 79-100: We implement the constructor for `SPIMasterAux`, which initializes member variables
- Line 102-120: We implement the destructor, which resets the GPIO pins to inputs, and disables the SPI1 device
- Line 122-167: We implement the `Initialize()` method
  - Line 134-135: We verfy that the device is valid (only SPI1)
  - Line 137-139: We set the device index and its base address, then retrieve the core clock rate
  - Line 143-145: We enable the SPI1 device, and initialize the CNTL0 and CNTL1 registers (a.o. to clear the FIFO)
  - Line 146-157: We set up the GPIO pins
  - Line 163: We set the clock rate to be the requested value
- Line 169-183: We implement the `SetClock()` method, which calculates the clock divider value based on the core clock rate and the requested clock rate, and stores it in a member variable
- Line 185-188: We implement the `Read()` method as a call to `WriteRead()` with a null write buffer
- Line 190-193: We implement the `Write()` method as a call to `WriteRead()` with a null read buffer
- Line 195-315: We implement the `WriteRead()` method, which performs the actual SPI communication
  - Line 199-201: We cast the write and read buffers to byte pointers for easier access, and verify that at least one of them is not null
  - Line 207: We calculate the base value for the CNTL0 register using the clock divider
  - Line 209-226: We add the CE specific value for CNTL0 depending on the ceIndex passed
  - Line 227-231: We enable the SPI master, set the output to msb first, and set variable width in the CNTL0 value, then write it to the CNTL0 register
  - Line 233: We set the input to also shift in msb first in the CNTL1 register
  - Line 235-310: We enter a loop that continues until all data is written and read. Inside the loop, we have three inner loops:
    - Line 237-259: The first inner loop checks if the TX FIFO is not full and there is still data to write.
If so, it writes up to 3 bytes of data to the TX FIFO, and then adds the additional number of bytes to follow, and updates the write count.
It then writes the data to the IO register if this is the last data to write, or to the TXHOLD register if there is more data to write (as the IO register will trigger the transmission immediately)
    - Line 261-284: The second inner loop checks if the RX FIFO is not empty and there is still data to read.
If so, it reads up to 3 bytes of data from the RX FIFO, stores it in the read buffer, and updates the read count
    - Line 286-09: The third inner loop checks if the SPI master is not busy and there is still data to read.
If so, it reads up to 3 bytes of data from the STAT register (which holds received data when not busy), stores it in the read buffer, and updates the read count

As you can see, the way data is written and read is a bit complex, as the SPI master can only write or read up to 3 bytes at a time, and we need to check the status of the FIFOs and the busy flag to know when we can write or read data.

### Step 2 - SPI0 and SPI3/6 {#TUTORIAL_27_SPI_SPI_STEP_2___SPI0_AND_SPI36}

Again, let's start with an extension of `BCMRegisters.h` to add the registers for the other SPI master devices.

#### BCMRegisters.h {#TUTORIAL_27_SPI_SPI_STEP_2___SPI0_AND_SPI36_BCMREGISTERSH}

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
473: //---------------------------------------------
474: // Raspberry Pi SPI Master
475: //---------------------------------------------
476: 
477: /// @brief Raspberry Pi SPI bus 0 registers base address.
478: #define RPI_SPI0_BASE         reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00204000)
479: #if BAREMETAL_RPI_TARGET == 4
480: /// @brief Raspberry Pi SPI bus 3 registers base address.
481: #define RPI_SPI3_BASE         reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00204600)
482: /// @brief Raspberry Pi SPI bus 4 registers base address.
483: #define RPI_SPI4_BASE         reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00204800)
484: /// @brief Raspberry Pi SPI bus 5 registers base address.
485: #define RPI_SPI5_BASE         reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00204A00)
486: /// @brief Raspberry Pi SPI bus 6 registers base address.
487: #define RPI_SPI6_BASE         reinterpret_cast<regaddr>(RPI_BCM_IO_BASE + 0x00204C00)
488: #endif
489: 
490: /// @brief Raspberry Pi SPI Master Control and Status register (R/W) offset relative to RPI_SPIx_BASE
491: #define RPI_SPI_CS_OFFSET                      0x00000000
492: /// @brief Raspberry Pi SPI Master Control and Status register long data word in LOSSI mode for DMA
493: #define RPI_SPI_CS_LEN_LONG                    BIT1(25)
494: /// @brief Raspberry Pi SPI Master Control and Status register enable DMA in LOSSI mode
495: #define RPI_SPI_CS_DMA_LEN                     BIT1(24)
496: /// @brief Raspberry Pi SPI Master Control and Status register Chip select 2 polarity
497: #define RPI_SPI_CS_POL2                        BIT1(23)
498: /// @brief Raspberry Pi SPI Master Control and Status register Chip select 1 polarity
499: #define RPI_SPI_CS_POL1                        BIT1(22)
500: /// @brief Raspberry Pi SPI Master Control and Status register Chip select 0 polarity
501: #define RPI_SPI_CS_POL0                        BIT1(21)
502: /// @brief Raspberry Pi SPI Master Control and Status register RX FIFO full (RO)
503: #define RPI_SPI_CS_RXF                         BIT1(20)
504: /// @brief Raspberry Pi SPI Master Control and Status register RX FIFO need to read, 3/4 full (RO)
505: #define RPI_SPI_CS_RXR                         BIT1(19)
506: /// @brief Raspberry Pi SPI Master Control and Status register TX FIFO has space (RO)
507: #define RPI_SPI_CS_TXD                         BIT1(18)
508: /// @brief Raspberry Pi SPI Master Control and Status register RX FIFO has data (RO)
509: #define RPI_SPI_CS_RXD                         BIT1(17)
510: /// @brief Raspberry Pi SPI Master Control and Status register Done (RO)
511: #define RPI_SPI_CS_DONE                        BIT1(16)
512: /// @brief Raspberry Pi SPI Master Control and Status register LOSSI mode enable
513: #define RPI_SPI_CS_LEN                         BIT1(13)
514: /// @brief Raspberry Pi SPI Master Control and Status register read enable
515: #define RPI_SPI_CS_REN                         BIT1(12)
516: /// @brief Raspberry Pi SPI Master Control and Status register auto de-assert CS
517: #define RPI_SPI_CS_ADCS                        BIT1(11)
518: /// @brief Raspberry Pi SPI Master Control and Status register interrupt on RXRenable
519: #define RPI_SPI_CS_INTR                        BIT1(10)
520: /// @brief Raspberry Pi SPI Master Control and Status register interrupt on Done enable
521: #define RPI_SPI_CS_INTD                        BIT1(9)
522: /// @brief Raspberry Pi SPI Master Control and Status register DMA enable
523: #define RPI_SPI_CS_DMAEN                       BIT1(8)
524: /// @brief Raspberry Pi SPI Master Control and Status register transfer activity
525: #define RPI_SPI_CS_TA                          BIT1(7)
526: /// @brief Raspberry Pi SPI Master Control and Status register CS polarity
527: #define RPI_SPI_CS_POL                         BIT1(6)
528: /// @brief Raspberry Pi SPI Master Control and Status register clear FIFO
529: #define RPI_SPI_CS_CLEAR                       BITS(4, 5)
530: /// @brief Raspberry Pi SPI Master Control and Status register clear TX FIFO
531: #define RPI_SPI_CS_CLEAR_TX                    BIT1(4)
532: /// @brief Raspberry Pi SPI Master Control and Status register clear RX FIFO
533: #define RPI_SPI_CS_CLEAR_RX                    BIT1(5)
534: /// @brief Raspberry Pi SPI Master Control and Status register Clock polarity
535: #define RPI_SPI_CS_CPOL                        BIT1(3)
536: /// @brief Raspberry Pi SPI Master Control and Status register Clock polarity idle low
537: #define RPI_SPI_CS_CPOL_IDLE_LOW               BIT0(3)
538: /// @brief Raspberry Pi SPI Master Control and Status register Clock polarity idle high
539: #define RPI_SPI_CS_CPOL_IDLE_HIGH              BIT1(3)
540: /// @brief Raspberry Pi SPI Master Control and Status register Clock phase
541: #define RPI_SPI_CS_CPHA                        BIT1(2)
542: /// @brief Raspberry Pi SPI Master Control and Status register Clock phase middle of data bit
543: #define RPI_SPI_CS_CPHA_MIDDLE_BIT             BIT0(2)
544: /// @brief Raspberry Pi SPI Master Control and Status register Clock phase beginning of data bit
545: #define RPI_SPI_CS_CPHA_BEGIN_BIT              BIT1(2)
546: /// @brief Raspberry Pi SPI Master Control and Status register Chip select activation
547: #define RPI_SPI_CS_ACTIVATE_SHIFT              0
548: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed bit mask. See @ref RASPBERRY_PI_MINI_SPI_SPI12
549: #define RPI_SPI_CS_ACTIVATE_MASK               BITS(RPI_SPI_CS_ACTIVATE_SHIFT, RPI_SPI_CS_ACTIVATE_SHIFT + 1)
550: /// @brief Raspberry Pi Mini SPI (SPI1/2) Control register 0 speed value for reading. See @ref RASPBERRY_PI_MINI_SPI_SPI12
551: #define RPI_SPI_CS_ACTIVATE_VALUE(x)           static_cast<uint32>((x & RPI_SPI_CS_ACTIVATE_MASK) >> RPI_SPI_CS_ACTIVATE_SHIFT)
552: /// @brief Raspberry Pi SPI Master Control and Status register Chip select activate 0
553: #define RPI_SPI_CS_ACTIVATE_CS0                0b00
554: /// @brief Raspberry Pi SPI Master Control and Status register Chip select activate 1
555: #define RPI_SPI_CS_ACTIVATE_CS1                0b01
556: /// @brief Raspberry Pi SPI Master Control and Status register Chip select activate 2
557: #define RPI_SPI_CS_ACTIVATE_CS2                0b10
558: /// @brief Raspberry Pi SPI Master Control and Status register Chip select activate none
559: #define RPI_SPI_CS_ACTIVATE_NONE               0b11
560: 
561: /// @brief Raspberry Pi SPI Master RX/TX FIFO register (R/W) offset relative to RPI_SPIx_BASE
562: #define RPI_SPI_FIFO_OFFSET                    0x00000004
563: 
564: /// @brief Raspberry Pi SPI Master Clock Divider register (R/W) offset relative to RPI_SPIx_BASE
565: #define RPI_SPI_CLK_OFFSET                     0x00000008
566: /// @brief Raspberry Pi SPI Master Clock Divider register clock divider bit shift
567: #define RPI_SPI_CLK_CDIV_SHIFT                 0
568: /// @brief Raspberry Pi SPI Master Clock Divider register clock divider bit mask
569: #define RPI_SPI_CLK_CDIV_MASK                  BITS(RPI_SPI_CLK_CDIV_SHIFT, RPI_SPI_CLK_CDIV_SHIFT + 15)
570: /// @brief Raspberry Pi SPI Master Clock Divider register clock divider value for reading
571: #define RPI_SPI_CLK_CDIV_VALUE(x)              static_cast<uint32>((x & RPI_SPI_CLK_CDIV_MASK) >> RPI_SPI_CLK_CDIV_SHIFT)
572: 
573: /// @brief Raspberry Pi SPI Master Data Length register (R/W) offset relative to RPI_SPIx_BASE
574: #define RPI_SPI_DLEN_OFFSET                    0x0000000C
575: /// @brief Raspberry Pi SPI Master Data Length register length in bytes bit shift
576: #define RPI_SPI_DLEN_LEN_SHIFT                 0
577: /// @brief Raspberry Pi SPI Master Data Length register length in bytes bit mask
578: #define RPI_SPI_DLEN_LEN_MASK                  BITS(RPI_SPI_DLEN_LEN_SHIFT, RPI_SPI_DLEN_LEN_SHIFT + 15)
579: /// @brief Raspberry Pi SPI Master Data Length register length in bytes value for reading
580: #define RPI_SPI_DLEN_LEN_VALUE(x)              static_cast<uint32>((x & RPI_SPI_DLEN_LEN_MASK) >> RPI_SPI_DLEN_LEN_SHIFT)
581: 
582: /// @brief Raspberry Pi SPI LOSSI mode Time Output Hold (TOH) register (R/W) offset relative to RPI_SPIx_BASE
583: #define RPI_SPI_LTOH_OFFSET                    0x00000010
584: /// @brief Raspberry Pi SPI Master Data Length register length in bytes bit shift
585: #define RPI_SPI_LTOH_TOH_SHIFT                 0
586: /// @brief Raspberry Pi SPI Master Data Length register length in bytes bit mask
587: #define RPI_SPI_LTOH_TOH_MASK                  BITS(RPI_SPI_LTOH_TOH_SHIFT, RPI_SPI_LTOH_TOH_SHIFT + 3)
588: /// @brief Raspberry Pi SPI Master Data Length register length in bytes value for reading
589: #define RPI_SPI_LTOH_TOH_VALUE(x)              static_cast<uint32>((x & RPI_SPI_LTOH_TOH_MASK) >> RPI_SPI_LTOH_TOH_SHIFT)
590: 
591: /// @brief Raspberry Pi SPI DMA DREQ Control register (R/W) offset relative to RPI_SPIx_BASE
592: #define RPI_SPI_DC_OFFSET                      0x00000014
593: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read panic threshold bit shift
594: #define RPI_SPI_DC_RPANIC_SHIFT                24
595: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read panic threshold bit mask
596: #define RPI_SPI_DC_RPANIC_MASK                 BITS(RPI_SPI_DC_RPANIC_SHIFT, RPI_SPI_DC_RPANIC_SHIFT + 7)
597: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read panic threshold value for reading
598: #define RPI_SPI_DC_RPANIC_VALUE(x)             static_cast<uint32>((x & RPI_SPI_DC_RPANIC_MASK) >> RPI_SPI_DC_RPANIC_SHIFT)
599: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read request threshold bit shift
600: #define RPI_SPI_DC_RDREQ_SHIFT                 16
601: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read request threshold bit mask
602: #define RPI_SPI_DC_RDREQ_MASK                  BITS(RPI_SPI_DC_RDREQ_SHIFT, RPI_SPI_DC_RDREQ_SHIFT + 7)
603: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA read request threshold value for reading
604: #define RPI_SPI_DC_RDREQ_VALUE(x)              static_cast<uint32>((x & RPI_SPI_DC_RDREQ_MASK) >> RPI_SPI_DC_RDREQ_SHIFT)
605: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write panic threshold bit shift
606: #define RPI_SPI_DC_TPANIC_SHIFT                8
607: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write panic threshold bit mask
608: #define RPI_SPI_DC_TPANIC_MASK                 BITS(RPI_SPI_DC_TPANIC_SHIFT, RPI_SPI_DC_TPANIC_SHIFT + 7)
609: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write panic threshold value for reading
610: #define RPI_SPI_DC_TPANIC_VALUE(x)             static_cast<uint32>((x & RPI_SPI_DC_TPANIC_MASK) >> RPI_SPI_DC_TPANIC_SHIFT)
611: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write request threshold bit shift
612: #define RPI_SPI_DC_TDREQ_SHIFT                 0
613: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write request threshold bit mask
614: #define RPI_SPI_DC_TDREQ_MASK                  BITS(RPI_SPI_DC_TDREQ_SHIFT, RPI_SPI_DC_TDREQ_SHIFT + 7)
615: /// @brief Raspberry Pi SPI DMA DREQ Control register DMA write request threshold value for reading
616: #define RPI_SPI_DC_TDREQ_VALUE(x)              static_cast<uint32>((x & RPI_SPI_DC_TDREQ_MASK) >> RPI_SPI_DC_TDREQ_SHIFT)
617: 
618: /// @brief Raspberry Pi SPI register address from base address and offset
619: #define RPI_SPI_REG_ADDRESS(base, offset)      reinterpret_cast<regaddr>((base) + (offset))
620: 
...
```

- Line 477-478: We define `RPI_SPI0_BASE` as the base address for the SPI0 registers
- Line 480-481: We define `RPI_SPI3_BASE` as the base address for the SPI3 registers, for Raspberry Pi 4 only
- Line 482-483: We define `RPI_SPI4_BASE` as the base address for the SPI4 registers, for Raspberry Pi 4 only
- Line 484-485: We define `RPI_SPI5_BASE` as the base address for the SPI5 registers, for Raspberry Pi 4 only
- Line 486-487: We define `RPI_SPI6_BASE` as the base address for the SPI6 registers, for Raspberry Pi 4 only
- Line 490-491: We define `RPI_SPI_CS_OFFSET` as the offset of the CS (Control and Status) register relative to the base address
- Line 492-559: We define the register values for the CS register.
For multi-bit fields, we define the shift and mask, as well as a helper to read the value of the field
- Line 561-562: We define `RPI_SPI_FIFO_OFFSET` as the offset of the FIFO register relative to the base address
- Line 564-565: We define `RPI_SPI_CLK_OFFSET` as the offset of the CLK (Clock) register relative to the base address
- Line 566-571: We define the register values for the CLK register
- Line 573-574: We define `RPI_SPI_DLEN_OFFSET` as the offset of the DLEN (Data Length) register relative to the base address
- Line 575-580: We define the register values for the DLEN register
- Line 582-583: We define `RPI_SPI_LTOH_OFFSET` as the offset of the LTOH (LOSSI Time Output Hold) register relative to the base address
- Line 584-589: We define the register values for the LTOH register
- Line 591-592: We define `RPI_SPI_DC_OFFSET` as the offset of the DC (DMA Control) register relative to the base address
- Line 593-616: We define the register values for the DC register
- Line 618-619: We define `RPI_SPI_REG_ADDRESS` as a helper to calculate the address of a register given the base address and the offset

#### SPIMaster.h {#TUTORIAL_27_SPI_SPI_STEP_2___SPI0_AND_SPI36_SPIMASTERH}

We'll declare the class `SPIMaster` to represent the SPI0 and SPI3/6 peripherals. This implements the `ISPIMaster` interface.

Add the file `code/libraries/baremetal/include/baremetal/SPIMaster.h`

```cpp
File: code/libraries/baremetal/include/baremetal/SPIMaster.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : SPIMaster.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SPIMaster
9: //
10: // Description : SPI Master functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/ISPIMaster.h"
44: #include "baremetal/IMemoryAccess.h"
45: #include "baremetal/PhysicalGPIOPin.h"
46: 
47: /// @file
48: /// SPI Master
49: 
50: namespace baremetal {
51: 
52: /// <summary>
53: /// SPI clock polarity
54: /// </summary>
55: enum class SPIClockPolarity
56: {
57:     /// @brief Clock is low when idle
58:     IdleLow,
59:     /// @brief Clock is high when idle
60:     IdleHigh,
61: };
62: 
63: /// <summary>
64: /// SPI clock phase
65: /// </summary>
66: enum class SPIClockPhase
67: {
68:     /// @brief First SCLK transition at middle of data bit.
69:     Middle,
70:     /// @brief First SCLK transition at beginning of data bit.
71:     Beginning,
72: };
73: 
74: /// <summary>
75: /// Driver for SPI master devices
76: ///
77: /// GPIO pin mapping (Raspberry Pi 3-4)
78: /// Device | CE1    CE0    MISO   MOSI   SCLK   | Boards
79: /// :----: | :--------------------------------: | :-----
80: /// 0      | GPIO07 GPIO08 GPIO09 GPIO10 GPIO11 | Raspberry Pi 3 / 4
81: /// 1      |                                    | Defined in SPIMasterAux class, not usable here
82: /// 2      |                                    | Not usable
83: /// 3      | GPIO24 GPIO00 GPIO01 GPIO02 GPIO03 | Raspberry Pi 4 only
84: /// 4      | GPIO25 GPIO04 GPIO05 GPIO06 GPIO07 | Raspberry Pi 4 only
85: /// 5      | GPIO26 GPIO12 GPIO13 GPIO14 GPIO15 | Raspberry Pi 4 only
86: /// 4      | GPIO27 GPIO18 GPIO19 GPIO20 GPIO21 | Raspberry Pi 4 only
87: /// GPIO0/1 are normally reserved for ID EEPROM.
88: ///
89: /// GPIO pin mapping (Raspberry Pi 5)
90: /// Device |  CE1    CE0    MISO   MOSI   SCLK   | Boards
91: /// :----: | :---------------------------------: | :-----
92: /// 0      |  GPIO9  GPIO10 GPIO11 GPIO8  GPIO7  | Raspberry Pi 5 only
93: /// 1      |  GPIO19 GPIO20 GPIO21 GPIO18 GPIO17 | Raspberry Pi 5 only
94: /// 2      |  GPIO1  GPIO2  GPIO3  GPIO0  GPIO24 | Raspberry Pi 5 only
95: /// 3      |  GPIO5  GPIO6  GPIO7  GPIO4  GPIO25 | Raspberry Pi 5 only
96: /// 4      |                                     | None
97: /// 5      |  GPIO13 GPIO14 GPIO15 GPIO12 GPIO26 | Raspberry Pi 5 only
98: /// GPIO0/1 are normally reserved for ID EEPROM.
99: /// </summary>
100: class SPIMaster : public ISPIMaster
101: {
102: private:
103:     /// @brief Memory access interface reference for accessing registers.
104:     IMemoryAccess& m_memoryAccess;
105:     /// @brief SPI device index
106:     uint8 m_device;
107:     /// @brief SPI device base register address
108:     regaddr m_baseAddress;
109:     /// @brief SPI clock polarity
110:     SPIClockPolarity m_clockPolarity;
111:     /// @brief SPI clock phase
112:     SPIClockPhase m_clockPhase;
113:     /// @brief SPI output hold time in microseconds. This is the time to wait after the last SCLK transition before de-asserting CS and ending the
114:     /// transfer, to ensure the slave device has time to process the last bit.
115:     uint32 m_csHoldTimeMicroSeconds;
116:     /// @brief SPI clock rate (Hz)
117:     uint32 m_clockRate;
118:     /// @brief Core clock rate used to determine SPI clock rate in Hz
119:     unsigned m_coreClockRate;
120:     /// @brief True if class is already initialized
121:     bool m_isInitialized;
122: 
123:     /// @brief GPIO pin for SCLK wire
124:     PhysicalGPIOPin m_sclkPin;
125:     /// @brief GPIO pin for MOSI wire
126:     PhysicalGPIOPin m_mosiPin;
127:     /// @brief GPIO pin for MISO wire
128:     PhysicalGPIOPin m_misoPin;
129:     /// @brief GPIO pin for CE0 wire
130:     PhysicalGPIOPin m_ce0Pin;
131:     /// @brief GPIO pin for CE1 wire
132:     PhysicalGPIOPin m_ce1Pin;
133: 
134: public:
135:     SPIMaster(IMemoryAccess& memoryAccess = GetMemoryAccess());
136: 
137:     ~SPIMaster();
138: 
139:     bool Initialize(uint8 device, uint32 clockRate = 500000, SPIClockPolarity polarity = SPIClockPolarity::IdleLow, SPIClockPhase phase = SPIClockPhase::Middle);
140: 
141:     void SetClock(uint32 clockRate) override;
142:     void SetClockMode(SPIClockPolarity polarity, SPIClockPhase phase);
143:     void SetCSHoldTime(uint32 csHoldTimeMicroSeconds);
144:     size_t Read(SPI_CEIndex ceIndex, void* buffer, size_t count) override;
145:     size_t Write(SPI_CEIndex ceIndex, const void* buffer, size_t count) override;
146:     size_t WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count) override;
147: 
148: private:
149: };
150: 
151: } // namespace baremetal
```

- Line 52-61: We define the `SPIClockPolarity` enum to represent the clock polarity (idle low or idle high)
- Line 63-72: We define the `SPIClockPhase` enum to represent the clock phase (middle or beginning of bit)
- Line 74-150: We declare the `SPIMaster` class, which implements the `ISPIMaster` interface
  - Line 103-104: Member variable `m_memoryAccess` holds a reference to an `IMemoryAccess` instance for accessing registers
  - Line 105-106: Member variable `m_device` holds the device index
  - Line 107-108: Member variable `m_baseAddress` holds the device base address
  - Line 109-110: Member variable `m_clockPolarity` holds the clock polarity
  - Line 111-112: Member variable `m_clockPolarity` holds the clock phase
  - Line 113-115: Member variable `m_csHoldTimeMicroSeconds` holds the CS hold time
  - Line 116-117: Member variable `m_clockRate` holds the clock rate
  - Line 118-119: Member variable `m_coreClockRate` holds the Raspberry Pi core clock rate, which is used to calculate the divisor to achieve the desired SPI clock rate
  - Line 120-121: Member variable `m_clockPolarity` holds the initialization status, to prevent against multiple initialization
  - Line 123-132: We declare member variables for the GPIO pins used for SCLK, MOSI, MISO, CE0, and CE1
  - Line 135: We declare the constructor, which takes a memory access interface reference
  - Lien 137: We declare the destructor
  - Line 139: We declare the `Initialize()` method, which initializes the SPI master with the specified device index, and clock rate, polarity, and phase
  - Line 141: We declare the method `SetClock()` as an override of the interface
  - Line 142: We declare the method `SetClockMode()` to set the desired clock polarity and phase
  - Line 143: We declare the method `SetCSHoldTime()` to set the desired CS hold time
  - Line 144: We declare the method `Read()` as an override of the interface
  - Line 145: We declare the method `Write()` as an override of the interface
  - Line 146: We declare the method `WriteRead()` as an override of the interface

#### SPIMaster.cpp {#TUTORIAL_27_SPI_SPI_STEP_2___SPI0_AND_SPI36_SPIMASTERCPP}

We'll implement the `SPIMaster` class.

Add the file `code/libraries/baremetal/src/SPIMaster.cpp`

```cpp
File: code/libraries/baremetal/src/SPIMaster.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : SPIMaster.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SPIMaster
9: //
10: // Description : SPI Master functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "baremetal/SPIMaster.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/MachineInfo.h"
45: #include "baremetal/String.h"
46: #include "baremetal/Timer.h"
47: 
48: /// @file
49: /// SPIMaster
50: 
51: using namespace baremetal;
52: 
53: /// @brief Define log name
54: LOG_MODULE("SPIMaster");
55: 
56: #if BAREMETAL_RPI_TARGET == 3
57: /// @brief Number of SPI devices for RPI 3
58: #define SPI_DEVICES 1
59: #elif BAREMETAL_RPI_TARGET == 4
60: /// @brief Number of SPI devices for RPI 4 (of which 2 are not available)
61: #define SPI_DEVICES 7
62: #else
63: /// @brief RPI5 is not currently supported.
64: #endif
65: 
66: /// @brief Total number of GPIO pins used for SPI (CE1 / CE0 / MISO / MOSI / SCLK)
67: #define SPI_WIRES             5
68: /// @brief Index of SPI CE1 pin
69: #define SPI_GPIO_CE1          0
70: /// @brief Index of SPI CE1 pin
71: #define SPI_GPIO_CE0          1
72: /// @brief Index of SPI CE1 pin
73: #define SPI_GPIO_MISO         2
74: /// @brief Index of SPI CE1 pin
75: #define SPI_GPIO_MOSI         3
76: /// @brief Index of SPI CE1 pin
77: #define SPI_GPIO_SCLK         4
78: 
79: /// @brief Total number of different GPIO pin alternative functions
80: #define SPI_VALUES            2
81: /// @brief Index for pin number in configuration table
82: #define SPI_VALUE_PIN         0
83: /// @brief Index for alternative function in configuration table
84: #define SPI_VALUE_ALT         1
85: 
86: /// @brief Value in configuration table to flag invalid combination
87: #define NONE                  { 10000, 10000 }
88: 
89: /// @brief GPIO pin configurations for SPI pins. For every bus there are multiple configurations for the two SPI pins
90: static unsigned s_gpioConfig[SPI_DEVICES][SPI_WIRES][SPI_VALUES] = {
91:     // CE1,      CE0,       MISO,      MOSI,      SCLK
92:     { { 7, 0 },  { 8, 0 },  { 9, 0 },  { 10, 0 }, { 11, 0 } },
93: #if BAREMETAL_RPI_TARGET == 4
94:     { NONE,      NONE,      NONE,      NONE,      NONE      }, // unused
95:     { NONE,      NONE,      NONE,      NONE,      NONE      }, // unused
96:     { { 24, 5 }, { 0,  3 }, { 1,  3 }, { 2,  3 }, { 3 , 3 } }, // Alt3, Alt5 (CE1)
97:     { { 25, 5 }, { 4,  3 }, { 5,  3 }, { 6,  3 }, { 7 , 3 } }, // Alt3, Alt5 (CE1)
98:     { { 26, 5 }, { 12, 3 }, { 13, 3 }, { 14, 3 }, { 15, 3 } }, // Alt3, Alt5 (CE1)
99:     { { 27, 5 }, { 18, 3 }, { 19, 3 }, { 20, 3 }, { 21, 3 } }  // Alt3, Alt5 (CE1)
100: #endif
101: };
102: 
103: /// @brief Macro to determine GPIO function to be selected 
104: #define ALT_FUNC(alt) (((alt) == 0) ? GPIOMode::AlternateFunction0 : (((alt) == 3) ? GPIOMode::AlternateFunction3 : GPIOMode::AlternateFunction5))
105: 
106: /// @brief SPI register bases addresses for each bus, depening on the RPI model
107: static regaddr s_baseAddress[SPI_DEVICES] = {
108:     RPI_SPI0_BASE,
109: #if BAREMETAL_RPI_TARGET == 4
110:     0,
111:     0,
112:     RPI_SPI3_BASE,
113:     RPI_SPI4_BASE,
114:     RPI_SPI5_BASE,
115:     RPI_SPI6_BASE,
116: #endif
117: };
118: 
119: /// <summary>
120: /// Constructor for SPIMaster
121: ///
122: /// The default value for memoryAccess will use the singleton MemoryAccess instance. A different reference to a IMemoryAccess instance can be passed for testing
123: /// </summary>
124: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
125: SPIMaster::SPIMaster(IMemoryAccess& memoryAccess /* = GetMemoryAccess()*/)
126:     : m_memoryAccess{memoryAccess}
127:     , m_device{}
128:     , m_baseAddress{}
129:     , m_clockPolarity{}
130:     , m_clockPhase{}
131:     , m_csHoldTimeMicroSeconds{}
132:     , m_clockRate{}
133:     , m_coreClockRate{}
134:     , m_isInitialized{}
135:     , m_sclkPin{memoryAccess}
136:     , m_mosiPin{memoryAccess}
137:     , m_misoPin{memoryAccess}
138:     , m_ce0Pin{memoryAccess}
139:     , m_ce1Pin{memoryAccess}
140: {
141: }
142: 
143: /// <summary>
144: /// Destructor for SPIMaster
145: /// </summary>
146: SPIMaster::~SPIMaster()
147: {
148:     if (m_isInitialized)
149:     {
150:         m_ce1Pin.SetMode(GPIOMode::InputPullUp);
151:         m_ce0Pin.SetMode(GPIOMode::InputPullUp);
152:         m_misoPin.SetMode(GPIOMode::InputPullUp);
153:         m_mosiPin.SetMode(GPIOMode::InputPullUp);
154:         m_sclkPin.SetMode(GPIOMode::InputPullUp);
155:     }
156:     m_isInitialized = false;
157:     m_baseAddress = nullptr;
158: }
159: 
160: /// <summary>
161: /// Initialize the SPIMaster for a specific device, setting the clock as specified.
162: /// </summary>
163: /// <param name="device">SPI device index</param>
164: /// <param name="clockRate">SPI clock rate to be used in Hz</param>
165: /// <param name="polarity">SPI clock polarity</param>
166: /// <param name="phase">SPI clock phase</param>
167: /// <returns></returns>
168: bool SPIMaster::Initialize(uint8 device, uint32 clockRate /*= 500000*/, SPIClockPolarity polarity /*= SPIClockPolarity::IdleLow*/, SPIClockPhase phase /*= SPIClockPhase::Middle*/)
169: {
170:     if (m_isInitialized)
171:         return true;
172: 
173:     LOG_INFO("Initialize SPI device %d", device);
174:     if ((device >= SPI_DEVICES) || (s_gpioConfig[device][SPI_GPIO_SCLK][SPI_VALUE_PIN] >= NUM_GPIO))
175:         return false;
176: 
177:     m_device = device;
178:     m_baseAddress = s_baseAddress[m_device];
179:     assert(m_baseAddress != 0);
180: 
181:     m_ce1Pin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_CE1][SPI_VALUE_PIN]);
182:     m_ce1Pin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_CE1][SPI_VALUE_ALT]));
183:     m_ce0Pin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_CE0][SPI_VALUE_PIN]);
184:     m_ce0Pin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_CE0][SPI_VALUE_ALT]));
185:     m_misoPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_MISO][SPI_VALUE_PIN]);
186:     m_misoPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_MISO][SPI_VALUE_ALT]));
187:     m_mosiPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_MOSI][SPI_VALUE_PIN]);
188:     m_mosiPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_MOSI][SPI_VALUE_ALT]));
189:     m_sclkPin.AssignPin(s_gpioConfig[m_device][SPI_GPIO_SCLK][SPI_VALUE_PIN]);
190:     m_sclkPin.SetMode(ALT_FUNC(s_gpioConfig[m_device][SPI_GPIO_SCLK][SPI_VALUE_ALT]));
191: 
192:     m_coreClockRate = GetMachineInfo().GetClockRate(ClockID::CORE);
193:     assert(m_coreClockRate > 0);
194: 
195:     m_isInitialized = true;
196: 
197:     SetClock(clockRate);
198:     SetClockMode(polarity, phase);
199: 
200:     LOG_INFO("Set up SPI device %d, clock rate %d, base address %08X", device, clockRate, m_baseAddress);
201:     return true;
202: }
203: 
204: /// <summary>
205: /// Set SPI clock rate
206: /// </summary>
207: /// <param name="clockRate">Clock rate in Hz</param>
208: void SPIMaster::SetClock(unsigned clockRate)
209: {
210:     assert(m_isInitialized);
211: 
212:     assert(4000 <= clockRate && clockRate <= 125000000);
213:     m_clockRate = clockRate;
214: 
215:     uint32 divider = static_cast<uint32>(m_coreClockRate / clockRate);
216:     uint32 value = ((divider << RPI_SPI_CLK_CDIV_SHIFT) & RPI_SPI_CLK_CDIV_MASK);
217:     m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CLK_OFFSET), value);
218:     LOG_INFO("Set clock core %d, divider %d, clockrate %d", m_coreClockRate, divider, clockRate);
219: }
220: 
221: /// <summary>
222: /// Set SPI clock polarity and phase
223: /// </summary>
224: /// <param name="polarity">Clock polarity</param>
225: /// <param name="phase">Clock phase</param>
226: void SPIMaster::SetClockMode(SPIClockPolarity polarity, SPIClockPhase phase)
227: {
228:     assert(m_isInitialized);
229:     assert(m_baseAddress != nullptr);
230: 
231:     m_clockPolarity = polarity;
232:     m_clockPhase = phase;
233: 
234:     uint32 value = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET));
235:     value &= ~(RPI_SPI_CS_CPOL | RPI_SPI_CS_CPHA);
236:     value |= ((m_clockPolarity == SPIClockPolarity::IdleHigh) ? RPI_SPI_CS_CPOL_IDLE_HIGH : RPI_SPI_CS_CPOL_IDLE_LOW) | ((m_clockPhase == SPIClockPhase::Beginning) ? RPI_SPI_CS_CPHA_BEGIN_BIT : RPI_SPI_CS_CPHA_MIDDLE_BIT);
237:     m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);
238:     LOG_INFO("Set clock polarity %d, phase %d", m_clockPolarity, m_clockPhase);
239: }
240: 
241: /// <summary>
242: /// Set the time to hold the CS line active after the transfer is completed, in order to allow some devices to complete internal operations before the
243: /// line is de-asserted. The time is specified in microseconds. The default value is 0, which means that the CS line will be de-asserted immediately
244: /// after the transfer is completed.
245: /// </summary>
246: /// <param name="csHoldTimeMicroSeconds">CS hold time in microseconds</param>
247: void SPIMaster::SetCSHoldTime(uint32 csHoldTimeMicroSeconds)
248: {
249:     assert(m_isInitialized);
250:     assert(csHoldTimeMicroSeconds < 200);
251:     m_csHoldTimeMicroSeconds = csHoldTimeMicroSeconds;
252: }
253: 
254: /// <summary>
255: /// READ bytes FROM device
256: /// </summary>
257: /// <param name="ceIndex">CE / CS pin to activate</param>
258: /// <param name="buffer">Buffer for data to be received</param>
259: /// <param name="count">Number of bytes to receive</param>
260: /// <returns>Number of bytes transferred</returns>
261: size_t SPIMaster::Read(SPI_CEIndex ceIndex, void* buffer, size_t count)
262: {
263:     return WriteRead(ceIndex, nullptr, buffer, count);
264: }
265: 
266: /// <summary>
267: /// Write bytes to device
268: /// </summary>
269: /// <param name="ceIndex">CE / CS pin to activate</param>
270: /// <param name="buffer">Buffer containing data to send</param>
271: /// <param name="count">Number of bytes to send</param>
272: /// <returns>Number of bytes transferred</returns>
273: size_t SPIMaster::Write(SPI_CEIndex ceIndex, const void* buffer, size_t count)
274: {
275:     return WriteRead(ceIndex, buffer, nullptr, count);
276: }
277: 
278: /// <summary>
279: /// Read / Write bytes from / to device
280: /// Data on SPI is always transferred in both directions at the same time, so every byte written will also cause a byte to be read. If the caller is
281: /// only interested in writing or reading, the other buffer can be set to nullptr. In this case, the bytes read or written will be discarded.
282: /// </summary>
283: /// <param name="ceIndex">CE / CS pin to activate</param>
284: /// <param name="writeBuffer">Buffer containing data to send</param>
285: /// <param name="readBuffer">Buffer for data to be received</param>
286: /// <param name="count">Number of bytes to send / receive</param>
287: /// <returns>Number of bytes transferred</returns>
288: size_t SPIMaster::WriteRead(SPI_CEIndex ceIndex, const void* writeBuffer, void* readBuffer, size_t count)
289: {
290:     assert(m_isInitialized);
291:     assert(m_baseAddress != nullptr);
292:     assert(writeBuffer != nullptr || readBuffer != nullptr);
293: 
294:     const uint8* writeData = reinterpret_cast<const uint8*>(writeBuffer);
295:     uint8* readData = reinterpret_cast<uint8*>(readBuffer);
296: 
297:     assert(count > 0);
298:     assert(count <= 0xFFFF);
299:     TRACE_DEBUG("Set data size %d", count);
300:     m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_DLEN_OFFSET), count);
301: 
302:     TRACE_DEBUG("Start transfer");
303:     assert(ceIndex <= SPI_CEIndex::CE1 || ceIndex == SPI_CEIndex::None);
304:     uint32 value = (m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & ~RPI_SPI_CS_ACTIVATE_NONE)
305:                  | (static_cast<uint32>(ceIndex) << RPI_SPI_CS_ACTIVATE_SHIFT)
306:                  | RPI_SPI_CS_CLEAR | RPI_SPI_CS_TA;
307:     m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);
308: 
309:     TRACE_DEBUG("Read/Write");
310: 
311:     unsigned writeCount = 0;
312:     unsigned readCount = 0;
313: 
314:     while ((writeCount < count) || (readCount < count))
315:     {
316:         while (writeCount < count)
317:         {
318:             if (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_TXD))
319:                 break;
320: 
321:             uint32 data = 0;
322:             if (writeData != 0)
323:             {
324:                 data = *writeData++;
325:             }
326: 
327:             m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET), data);
328: 
329:             writeCount++;
330:         }
331: 
332:         while (readCount < count)
333:         {
334:             if (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_RXD))
335:                 break;
336: 
337:             uint32 data = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET));
338:             if (readData != 0)
339:             {
340:                 *readData++ = static_cast<uint8>(data);
341:             }
342: 
343:             readCount++;
344:         }
345:     }
346: 
347:     while (!(m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_DONE))
348:     {
349:         while (m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & RPI_SPI_CS_RXD)
350:         {
351:             m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_FIFO_OFFSET));
352:         }
353:     }
354: 
355:     if (m_csHoldTimeMicroSeconds > 0)
356:     {
357:         Timer::WaitMicroSeconds(m_csHoldTimeMicroSeconds);
358: 
359:         m_csHoldTimeMicroSeconds = 0;
360:     }
361: 
362:     value = m_memoryAccess.Read32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET)) & ~RPI_SPI_CS_TA;
363:     m_memoryAccess.Write32(RPI_SPI_REG_ADDRESS(m_baseAddress, RPI_SPI_CS_OFFSET), value);
364: 
365:     return static_cast<size_t>(count);
366: }
```

- Line 56-64: We define the number of SPI devices available depending on the Raspberry Pi model
- Line 66-67: We define `SPI_WIRES` as the total number of GPIO pins used for SPI, which is 5 (CE1, CE0, MISO, MOSI, SCLK)
- Line 68-77: We define constants for the index of each SPI wire in the GPIO configuration table
- Line 79-80: We define `SPI_VALUES` as the total number of values for each pin in the GPIO configuration table, which are the pin number and the alternative function to select
- Line 81-84: We define constants for the index of each value for a SPI wire in the GPIO configuration table
- Line 86-87: We define `NONE` as a value int the configuration table to denote an invalid entry
- Line 89-101: We define the static variable `s_gpioConfig` as the GPIO configuration. This holds for each SPI device, for each pin, which GPIO pin is used and using which GPIO function.
As you can see, there is a gap for SPI1 and 2, to enable continued numbering
- Line 103-104: We define the `ALT_FUNC` macro to determine the GPIO function to select based on the alternative function index in the configuration table
- Line 106-117: We define the static variable `s_baseAddress` to hold the base address for each SPI device, depending on the Raspberry Pi model
- Line 119-141: We implement the constructor for `SPIMaster`, which initializes member variables
- Line 143-158: We implement the destructor, which resets the GPIO pins to inputs
- Line 160-202: We implement the `Initialize()` method
  - Line 174-175: We verfy that the device is valid
  - Line 177-178: We set the device index and its base address
  - Line 181-190: We set up the GPIO pins
  - Line 192-193: We retrieve the core clock rate and check that it's valid
  - Line 197: We set the clock rate to be the requested value
  - Line 198: We set the clock mode to be the requested value
- Line 204-219: We implement the `SetClock()` method, which calculates the clock divider value based on the core clock rate and the requested clock rate, and stores it in a member variable
- Line 221-239: We implement the `SetClockMode()` method, which sets the correct clock polarity and phase
- Line 241-252: We implement the `SetCSHoldTime()` method, which stores the requested CS hold time in a member variable
- Line 254-264: We implement the `Read()` method as a call to `WriteRead()` with a null write buffer
- Line 266-276: We implement the `Write()` method as a call to `WriteRead()` with a null read buffer
- Line 278-366: We implement the `WriteRead()` method, which performs the actual SPI communication
  - Line 290-292: We verify than the SPI master is initialized, has a correct base address, and that at least one of the read / write buffers is not null
  - Line 294-295: We cast the write and read buffers to byte pointers for easier access
  - Line 297-300: We set the data length register to the number of bytes to transfer, after checking that it's within the valid range
  - Line 302-307: We set up de the device for transfer by activating the correct CE line, and setting the TA (transfer active) bit in the CS register
  - Line 314-345: We enter a loop that continues until all data is written and read. Inside the loop, we have two inner loops:
    - Line 316-330: The first inner loop checks if the TX FIFO is not full and there is still data to write.
If so, it writes a byte of data to the TX FIFO from the write buffer
    - Line 332-345: The second inner loop checks if the RX FIFO is not empty and there is still data to read.
If so, it reads a byte of data from the RX FIFO, stores it in the read buffer
  - Line 347-353: After the main loop, we wait until the transfer is done by checking the DONE bit in the CS register.
  While waiting, we also read and discard any remaining data in the RX FIFO to ensure it's empty before ending the transfer
  - Line 355-360: If a CS hold time was specified, we wait for that time before de-asserting the CS line
  - Line 362-363: We de-assert the TA (transfer active) bit in the CS register to end the transfer

### Step 3 - Using SPI {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI}

In order to use the `SPIMaster` class, we will be using another member of the 23017 family, specific for SPI.

The family consists of the following ICs:

| Type      | Interface | Number of ports |
|-----------|-----------|-----------------|
| MCP23008  | I2C       | 1 x 8 bits      |
| MCP23017  | I2C       | 2 x 8 bits      |
| MCP23S008 | SPI       | 1 x 8 bits      |
| MCP23S017 | SPI       | 2 x 8 bits      |

We have used MCP23017 in the previous tutorial to demonstrate the use of I2C, and now we will be using MCP23S008 to demonstrate the use of SPI.
This IC is very similar to the MCP23017, but it uses SPI instead of I2C for communication, and has only 1 8 bit port.
The register map is similar.

For this tutorial, we will again be using a dedicated electronic circuit, as depicted in [this diagram](pdf/MCP23S008_LEDs.pdf):

<img src="images/MCP23S008_LEDs.png" alt="MCP23S008 controlling LEDs" width="1400"/>

In order to use the similarity between MCP23017 and MCP23S017, and between MCP23008 and MCP23S008, we will be creating a base class for the each.
The I2C and SPI specific classes will then inherit from the base class and implement the specific communication protocol.

#### MCP23017.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017H}

We'll declare the partly abstract class `MCP23017` which will implement specific of the MCP23(S)017 IC, leaving the communication to a inheriting class.

Add the file `code/libraries/device/include/device/expander/MCP23017.h`

```cpp
File: code/libraries/device/include/device/expander/MCP23008.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017
9: //
10: // Description : MCP23017 expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "stdlib/Macros.h"
43: #include "stdlib/Types.h"
44: 
45: /// @file
46: /// MCP23017 expander declaration
47: 
48: namespace device {
49: 
50: /// @brief Select register bank for MCP23017.
51: /// MCP23017 has two register banks, which differ in the relative register numbers
52: #define BANK 0
53: 
54: #if BANK == 0
55: 
56: /// @brief MCP23017 registers when BANK = 0
57: enum MCP23017RegisterIndex
58: {
59:     /// @brief I/O Direction Register Port A
60:     IODIRA = 0,
61:     /// @brief I/O Direction Register Port B
62:     IODIRB = 1,
63:     /// @brief Input Polarity Register Port A
64:     IPOLA = 2,
65:     /// @brief Input Polarity Register Port B
66:     IPOLB = 3,
67:     /// @brief GPIO Interrupt Enable Register Port A
68:     GPINTENA = 4,
69:     /// @brief GPIO Interrupt Enable Register Port B
70:     GPINTENB = 5,
71:     /// @brief Default Compare Register Port A
72:     DEFVALA = 6,
73:     /// @brief Default Compare Register Port B
74:     DEFVALB = 7,
75:     /// @brief Interrupt Control Register Port A
76:     INTCONA = 8,
77:     /// @brief Interrupt Control Register Port B
78:     INTCONB = 9,
79:     /// @brief I/O Control Register Port A
80:     IOCONA = 10,
81:     /// @brief I/O Control Register Port B
82:     IOCONB = 11,
83:     /// @brief GPIO Pull-up Resistor Configuration Register Port A
84:     GPPUA = 12,
85:     /// @brief GPIO Pull-up Resistor Configuration Register Port B
86:     GPPUB = 13,
87:     /// @brief Interrupt Flag Register Port A
88:     INTFA = 14,
89:     /// @brief Interrupt Flag Register Port B
90:     INTFB = 15,
91:     /// @brief Interrupt Captured Register Port A
92:     INTCAPA = 16,
93:     /// @brief Interrupt Captured Register Port B
94:     INTCAPB = 17,
95:     /// @brief GPIO Port Register Port A
96:     GPIOA = 18,
97:     /// @brief GPIO Port Register Port b
98:     GPIOB = 19,
99:     /// @brief Output Latch Register Port A
100:     OLATA = 20,
101:     /// @brief Output Latch Register Port B
102:     OLATB = 21,
103: };
104: 
105: #else
106: 
107: /// @brief MCP23017 registers when BANK = 1
108: enum MCP23017RegisterIndex
109: {
110:     /// @brief I/O Direction Register Port A
111:     IODIRA = 0,
112:     /// @brief I/O Direction Register Port B
113:     IODIRB = 16,
114:     /// @brief Input Polarity Register Port A
115:     IPOLA = 1,
116:     /// @brief Input Polarity Register Port B
117:     IPOLB = 17,
118:     /// @brief GPIO Interrupt Enable Register Port A
119:     GPINTENA = 2,
120:     /// @brief GPIO Interrupt Enable Register Port B
121:     GPINTENB = 18,
122:     /// @brief Default Compare Register Port A
123:     DEFVALA = 3,
124:     /// @brief Default Compare Register Port B
125:     DEFVALB = 19,
126:     /// @brief Interrupt Control Register Port A
127:     INTCONA = 4,
128:     /// @brief Interrupt Control Register Port B
129:     INTCONB = 20,
130:     /// @brief I/O Control Register Port A
131:     IOCONA = 5,
132:     /// @brief I/O Control Register Port B
133:     IOCONB = 21,
134:     /// @brief GPIO Pull-up Resistor Configuration Register Port A
135:     GPPUA = 6,
136:     /// @brief GPIO Pull-up Resistor Configuration Register Port B
137:     GPPUB = 22,
138:     /// @brief Interrupt Flag Register Port A
139:     INTFA = 7,
140:     /// @brief Interrupt Flag Register Port B
141:     INTFB = 23,
142:     /// @brief Interrupt Captured Register Port A
143:     INTCAPA = 8,
144:     /// @brief Interrupt Captured Register Port B
145:     INTCAPB = 24,
146:     /// @brief GPIO Port Register Port A
147:     GPIOA = 9,
148:     /// @brief GPIO Port Register Port b
149:     GPIOB = 25,
150:     /// @brief Output Latch Register Port A
151:     OLATA = 10,
152:     /// @brief Output Latch Register Port B
153:     OLATB = 26,
154: };
155: 
156: #endif
157: 
158: /// @brief IO Configuration Register Bank 0 selection
159: #define IOCON_BANK0  BIT0(7)
160: /// @brief IO Configuration Register Bank 1 selection
161: #define IOCON_BANK1  BIT1(7)
162: /// @brief IO Configuration Register Interrupt A/B mirroring
163: #define IOCON_MIRROR BIT1(6)
164: /// @brief IO Configuration Register sequential operation (automatic register index increment, subsequent write will be to next register)
165: #define IOCON_SEQOP  BIT1(5)
166: /// @brief IO Configuration Register SDA slew rate
167: #define IOCON_DISSLW BIT1(4)
168: /// @brief IO Configuration Register Hardware Address Enable (for MCP23S017 only)
169: #define IOCON_HAEN   BIT1(3)
170: /// @brief IO Configuration Register Open Drain Interrupt pin
171: #define IOCON_ODR    BIT1(2)
172: /// @brief IO Configuration Register Interrupt polarity
173: #define IOCON_INTPOL BIT1(1)
174: 
175: /// @brief MCP23017 pin direction
176: enum class MCP23017PinDirection
177: {
178:     /// @brief Input pin
179:     In,
180:     /// @brief Output pin
181:     Out,
182: };
183: 
184: /// @brief MCP23017 pin index
185: enum class MCP23017Pin
186: {
187:     /// @brief Port A pin 0
188:     PinA0,
189:     /// @brief Port A pin 1
190:     PinA1,
191:     /// @brief Port A pin 2
192:     PinA2,
193:     /// @brief Port A pin 3
194:     PinA3,
195:     /// @brief Port A pin 4
196:     PinA4,
197:     /// @brief Port A pin 5
198:     PinA5,
199:     /// @brief Port A pin 6
200:     PinA6,
201:     /// @brief Port A pin 7
202:     PinA7,
203:     /// @brief Port B pin 0
204:     PinB0,
205:     /// @brief Port B pin 1
206:     PinB1,
207:     /// @brief Port B pin 2
208:     PinB2,
209:     /// @brief Port B pin 3
210:     PinB3,
211:     /// @brief Port B pin 4
212:     PinB4,
213:     /// @brief Port B pin 5
214:     PinB5,
215:     /// @brief Port B pin 6
216:     PinB6,
217:     /// @brief Port B pin 7
218:     PinB7,
219: };
220: 
221: /// <summary>
222: /// Driver for MCP23017 expander device
223: ///
224: /// This is a common driver for MCP23008 and MCP23S08 devices, which differ in the communication protocol (I2C vs SPI) and the register addresses, but
225: /// have the same functionality.
226: /// The device supports two 8 bit ports named port A and B.
227: /// Each pin can be either an input or output.
228: /// </summary>
229: class MCP23017
230: {
231: public:
232:     MCP23017();
233:     ~MCP23017();
234: 
235:     bool Initialize();
236:     void Uninitialize();
237: 
238:     /// <summary>
239:     /// Read MCP23017 register
240:     /// </summary>
241:     /// <param name="registerAddress">Register index</param>
242:     /// <returns>Value read</returns>
243:     virtual uint8 ReadRegister(MCP23017RegisterIndex registerAddress) = 0;
244:     /// <summary>
245:     /// Write MCP23017 register
246:     /// </summary>
247:     /// <param name="registerAddress">Register index</param>
248:     /// <param name="byte">Value to write</param>
249:     virtual void WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte) = 0;
250:     void GetPinDirection(MCP23017Pin pinNumber, MCP23017PinDirection& direction);
251:     void SetPinDirection(MCP23017Pin pinNumber, const MCP23017PinDirection& direction);
252:     bool GetPinValue(MCP23017Pin pinNumber);
253:     void SetPinValue(MCP23017Pin pinNumber, bool on);
254:     void SetPortADirections(const MCP23017PinDirection& direction);
255:     uint8 GetPortAValue();
256:     void SetPortAValue(uint8 data);
257:     void SetPortBDirections(const MCP23017PinDirection& direction);
258:     uint8 GetPortBValue();
259:     void SetPortBValue(uint8 data);
260: };
261: 
262: } // namespace device
```

You'll recognize a lot from the previous [tutorial](#TUTORIAL_22_I2C_USING_MCP23017___STEP_2_MCP23017H) on MCP23017 using I2C, so I'll not cover it in detail again.

- Line 42-43: Instead of including the I2CMaster header, we include more basic headers needed for this code
- Line 236: The constructor does not need to do memory access for I2C, so the parameter is dropped
- Line 235: The `Initialize()` method does not need the I2C bus or address, so these parameters are dropped
- Line 236: We add a method `Uninitialize()` to close down the device. We cannot do this in de destructor anymore as other classes will derive from it
- Line 238-243: The method `ReadRegister()` is now pure virtual
- Line 244-249: The method `WriteRegister()` is now pure virtual

#### MCP23017.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017CPP}

We'll implement the `MCP23017` class.

Add the file `code/libraries/device/src/expander/MCP23017.cpp`

```cpp
File: code/libraries/device/src/expander/MCP23017.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017
9: //
10: // Description : MCP23017 expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/expander/MCP23017.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23017 expander implementation
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23017");
52: 
53: /// <summary>
54: /// Constructor for MCP23017 class
55: /// </summary>
56: MCP23017::MCP23017()
57: {
58: }
59: 
60: /// <summary>
61: /// Destructor for MCP23017 class
62: /// </summary>
63: MCP23017::~MCP23017()
64: {
65: }
66: 
67: /// <summary>
68: /// Initialize the MCP23017 expander
69: /// </summary>
70: /// <returns>True on success, false otherwise</returns>
71: bool MCP23017::Initialize()
72: {
73:     LOG_INFO("Initialize");
74:     WriteRegister(IOCONA, IOCON_BANK0 | IOCON_SEQOP | IOCON_HAEN | IOCON_ODR);
75:     return true;
76: }
77: 
78: /// <summary>
79: /// Uninitialize MCP23008. Set pins to input mode.
80: /// </summary>
81: void MCP23017::Uninitialize()
82: {
83:     SetPortADirections(MCP23017PinDirection::In);
84:     SetPortBDirections(MCP23017PinDirection::In);
85: }
86: 
87: /// <summary>
88: /// Get the I/O pin direction for the specified pin on the MCP23017
89: /// </summary>
90: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
91: /// <param name="direction">Holds pin direction on return</param>
92: void MCP23017::GetPinDirection(MCP23017Pin pinNumber, MCP23017PinDirection& direction)
93: {
94:     MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? IODIRA : IODIRB;
95:     uint8 data = ReadRegister(registerAddress);
96:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
97:     direction = (data & (1 << pinShift)) ? MCP23017PinDirection::In : MCP23017PinDirection::In;
98: }
99: 
100: /// <summary>
101: /// Set the I/O pin direction for the specified pin on the MCP23017
102: /// </summary>
103: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
104: /// <param name="direction">Pin direction to set</param>
105: void MCP23017::SetPinDirection(MCP23017Pin pinNumber, const MCP23017PinDirection& direction)
106: {
107:     MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? IODIRA : IODIRB;
108:     uint8 data = ReadRegister(registerAddress);
109:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
110:     uint8 mask = 1 << pinShift;
111:     uint8 pinData = (direction == MCP23017PinDirection::In) ? mask : 0;
112:     WriteRegister(registerAddress, (data & ~mask) | pinData);
113: }
114: 
115: /// <summary>
116: /// Get the value of an input pin on the MCP23017
117: /// </summary>
118: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
119: /// <returns>Requested pin value</returns>
120: bool MCP23017::GetPinValue(MCP23017Pin pinNumber)
121: {
122:     MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? GPIOA : GPIOB;
123:     uint8 data = ReadRegister(registerAddress);
124:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
125:     return (data & (1 << pinShift));
126: }
127: 
128: /// <summary>
129: /// Set the value of an output pin on the MCP23017
130: /// </summary>
131: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
132: /// <param name="on">Value for output pin</param>
133: void MCP23017::SetPinValue(MCP23017Pin pinNumber, bool on)
134: {
135:     MCP23017RegisterIndex registerAddress = (pinNumber < MCP23017Pin::PinB0) ? GPIOA : GPIOB;
136:     uint8 data = ReadRegister(registerAddress);
137:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
138:     uint8 mask = 1 << pinShift;
139:     uint8 pinData = on ? mask : 0;
140:     WriteRegister(registerAddress, (data & ~mask) | pinData);
141: }
142: 
143: /// <summary>
144: /// Set I/O pin directions for all pins on Port A
145: /// </summary>
146: /// <param name="direction">Direction for pins</param>
147: void MCP23017::SetPortADirections(const MCP23017PinDirection& direction)
148: {
149:     WriteRegister(GPPUA, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
150:     WriteRegister(IODIRA, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
151: }
152: 
153: /// <summary>
154: /// Get value for all pins on Port A.
155: /// Bit 0 is the value for pin 0, etc.
156: /// </summary>
157: /// <returns></returns>
158: uint8 MCP23017::GetPortAValue()
159: {
160:     return ReadRegister(GPIOA);
161: }
162: 
163: /// <summary>
164: /// Set value for all pins on Port A
165: /// </summary>
166: /// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
167: void MCP23017::SetPortAValue(uint8 data)
168: {
169:     WriteRegister(GPIOA, data);
170: }
171: 
172: /// <summary>
173: /// Set I/O pin directions for all pins on Port B
174: /// </summary>
175: /// <param name="direction">Direction for pins</param>
176: void MCP23017::SetPortBDirections(const MCP23017PinDirection& direction)
177: {
178:     WriteRegister(GPPUB, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
179:     WriteRegister(IODIRB, direction == MCP23017PinDirection::In ? 0xFF : 0x00);
180: }
181: 
182: /// <summary>
183: /// Get value for all pins on Port B.
184: /// Bit 0 is the value for pin 0, etc.
185: /// </summary>
186: /// <returns></returns>
187: uint8 MCP23017::GetPortBValue()
188: {
189:     return ReadRegister(GPIOB);
190: }
191: 
192: /// <summary>
193: /// Set value for all pins on Port B
194: /// </summary>
195: /// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
196: void MCP23017::SetPortBValue(uint8 data)
197: {
198:     WriteRegister(GPIOB, data);
199: }
```

Again, this implementation is very similar to what we had before.

- Line 60-65: The destructor no longer puts the ports back in input mode
- Line 67-76: The method `Initialize()` has no parameters, so parameter checking and device initialization is no longer needed
- Line 78-85: Setting the ports to input mode is now done in `Uninitialize()`
- Line 87: We removed the implementation for `ReadRegister()` and `WriteRegister()` as they are now abstract

#### MCP23008.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008H}

We'll declare the partly abstract class `MCP23008` which will implement specific of the MCP23(S)008 IC, leaving the communication to a inheriting class.

Add the file `code/libraries/device/include/device/expander/MCP23008.h`

```cpp
File: code/libraries/device/include/device/expander/MCP23008.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008
9: //
10: // Description : MCP23008 expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "stdlib/Macros.h"
43: #include "stdlib/Types.h"
44: 
45: /// @file
46: /// MCP23008 expander declaration
47: 
48: namespace device {
49: 
50: /// @brief MCP23008 registers
51: enum MCP23008RegisterIndex
52: {
53:     /// @brief I/O Direction Register
54:     IODIR = 0,
55:     /// @brief Input Polarity Register
56:     IPOL = 1,
57:     /// @brief GPIO Interrupt Enable Register
58:     GPINTEN = 2,
59:     /// @brief Default Compare Register
60:     DEFVAL = 3,
61:     /// @brief Interrupt Control Register
62:     INTCON = 4,
63:     /// @brief I/O Control Register
64:     IOCON = 5,
65:     /// @brief GPIO Pull-up Resistor Configuration Register
66:     GPPU = 6,
67:     /// @brief Interrupt Flag Register
68:     INTF = 7,
69:     /// @brief Interrupt Captured Register
70:     INTCAP = 8,
71:     /// @brief GPIO Port Register
72:     GPIO = 9,
73:     /// @brief Output Latch Register
74:     OLAT = 10,
75: };
76: 
77: /// @brief IO Configuration Register sequential operation (automatic register index increment, subsequent write will be to next register)
78: #define IOCON_SEQOP  BIT1(5)
79: /// @brief IO Configuration Register SDA slew rate
80: #define IOCON_DISSLW BIT1(4)
81: /// @brief IO Configuration Register Hardware Address Enable (for MCP23S017 only)
82: #define IOCON_HAEN   BIT1(3)
83: /// @brief IO Configuration Register Open Drain Interrupt pin
84: #define IOCON_ODR    BIT1(2)
85: /// @brief IO Configuration Register Interrupt polarity
86: #define IOCON_INTPOL BIT1(1)
87: 
88: /// @brief MCP23008 pin direction
89: enum class MCP23008PinDirection
90: {
91:     /// @brief Input pin
92:     In,
93:     /// @brief Output pin
94:     Out,
95: };
96: 
97: /// @brief MCP23008 pin index
98: enum class MCP23008Pin
99: {
100:     /// @brief Pin 0
101:     Pin0,
102:     /// @brief Pin 1
103:     Pin1,
104:     /// @brief Pin 2
105:     Pin2,
106:     /// @brief Pin 3
107:     Pin3,
108:     /// @brief Pin 4
109:     Pin4,
110:     /// @brief Pin 5
111:     Pin5,
112:     /// @brief Pin 6
113:     Pin6,
114:     /// @brief Pin 7
115:     Pin7,
116: };
117: 
118: /// <summary>
119: /// Driver for MCP23008 expander device
120: ///
121: /// This is a common driver for MCP23008 and MCP23S08 devices, which differ in the communication protocol (I2C vs SPI) and the register addresses, but
122: /// have the same functionality.
123: /// The device supports one 8 bit port.
124: /// Each pin can be either an input or output.
125: /// </summary>
126: class MCP23008
127: {
128: public:
129:     MCP23008();
130:     ~MCP23008();
131: 
132:     bool Initialize();
133:     void Uninitialize();
134: 
135:     /// <summary>
136:     /// Read MCP23008 register
137:     /// </summary>
138:     /// <param name="registerAddress">Register index</param>
139:     /// <returns>Value read</returns>
140:     virtual uint8 ReadRegister(MCP23008RegisterIndex registerAddress) = 0;
141:     /// <summary>
142:     /// Write MCP23008 register
143:     /// </summary>
144:     /// <param name="registerAddress">Register index</param>
145:     /// <param name="byte">Value to write</param>
146:     virtual void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte) = 0;
147:     void GetPinDirection(MCP23008Pin pinNumber, MCP23008PinDirection& direction);
148:     void SetPinDirection(MCP23008Pin pinNumber, const MCP23008PinDirection& direction);
149:     bool GetPinValue(MCP23008Pin pinNumber);
150:     void SetPinValue(MCP23008Pin pinNumber, bool on);
151:     void SetPortDirections(const MCP23008PinDirection& direction);
152:     uint8 GetPortValue();
153:     void SetPortValue(uint8 data);
154: };
155: 
156: } // namespace device
```

You'll notice that this is very similar to [MCP23017.h](#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017H).

- Line 50-75: We now have fewer registers as there is only one port.
Also the concept of having different banks to access registers is no longer valid
- Line 77-86: There are fewer configuration bits (`IOCON_BANK`, `IOCON_MIRROR` do not exist)
- Line 88-95: Pin directions are the same, just using a new enum class
- Line 97-116: We only have 8 pins now
- Line 118-154: The class `MCP23008` is again very similar.
The only difference is that we only have one port

#### MCP23008.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008CPP}

We'll implement the `MCP23008` class.

Add the file `code/libraries/device/src/expander/MCP23008.cpp`

```cpp
File: code/libraries/device/src/expander/MCP23008.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008
9: //
10: // Description : MCP23008 expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/expander/MCP23008.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23008 expander implementation
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23008");
52: 
53: /// <summary>
54: /// Constructor for MCP23008 class
55: /// </summary>
56: MCP23008::MCP23008()
57: {
58: }
59: 
60: /// <summary>
61: /// Destructor for MCP23008 class
62: /// </summary>
63: MCP23008::~MCP23008()
64: {
65: }
66: 
67: /// <summary>
68: /// Initialize the MCP23008 expander
69: /// </summary>
70: /// <returns>True on success, false otherwise</returns>
71: bool MCP23008::Initialize()
72: {
73:     LOG_INFO("Initialize");
74: 
75:     WriteRegister(IOCON, IOCON_SEQOP | IOCON_HAEN | IOCON_ODR);
76:     return true;
77: }
78: 
79: /// <summary>
80: /// Uninitialize MCP23008. Set pins to input mode.
81: /// </summary>
82: void MCP23008::Uninitialize()
83: {
84:     SetPortDirections(MCP23008PinDirection::In);
85: }
86: 
87: /// <summary>
88: /// Get the I/O pin direction for the specified pin on the MCP23008
89: /// </summary>
90: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
91: /// <param name="direction">Holds pin direction on return</param>
92: void MCP23008::GetPinDirection(MCP23008Pin pinNumber, MCP23008PinDirection& direction)
93: {
94:     uint8 data = ReadRegister(IODIR);
95:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
96:     direction = (data & (1 << pinShift)) ? MCP23008PinDirection::In : MCP23008PinDirection::In;
97: }
98: 
99: /// <summary>
100: /// Set the I/O pin direction for the specified pin on the MCP23008
101: /// </summary>
102: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
103: /// <param name="direction">Pin direction to set</param>
104: void MCP23008::SetPinDirection(MCP23008Pin pinNumber, const MCP23008PinDirection& direction)
105: {
106:     uint8 data = ReadRegister(IODIR);
107:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
108:     uint8 mask = 1 << pinShift;
109:     uint8 pinData = (direction == MCP23008PinDirection::In) ? mask : 0;
110:     WriteRegister(IODIR, (data & ~mask) | pinData);
111: }
112: 
113: /// <summary>
114: /// Get the value of an input pin on the MCP23008
115: /// </summary>
116: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
117: /// <returns>Requested pin value</returns>
118: bool MCP23008::GetPinValue(MCP23008Pin pinNumber)
119: {
120:     uint8 data = ReadRegister(GPIO);
121:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
122:     return (data & (1 << pinShift));
123: }
124: 
125: /// <summary>
126: /// Set the value of an output pin on the MCP23008
127: /// </summary>
128: /// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
129: /// <param name="on">Value for output pin</param>
130: void MCP23008::SetPinValue(MCP23008Pin pinNumber, bool on)
131: {
132:     uint8 data = ReadRegister(GPIO);
133:     uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
134:     uint8 mask = 1 << pinShift;
135:     uint8 pinData = on ? mask : 0;
136:     WriteRegister(GPIO, (data & ~mask) | pinData);
137: }
138: 
139: /// <summary>
140: /// Set I/O pin directions for all pins
141: /// </summary>
142: /// <param name="direction">Direction for pins</param>
143: void MCP23008::SetPortDirections(const MCP23008PinDirection& direction)
144: {
145:     WriteRegister(GPPU, direction == MCP23008PinDirection::In ? 0xFF : 0x00);
146:     WriteRegister(IODIR, direction == MCP23008PinDirection::In ? 0xFF : 0x00);
147: }
148: 
149: /// <summary>
150: /// Get value for all pins.
151: /// Bit 0 is the value for pin 0, etc.
152: /// </summary>
153: /// <returns></returns>
154: uint8 MCP23008::GetPortValue()
155: {
156:     return ReadRegister(GPIO);
157: }
158: 
159: /// <summary>
160: /// Set value for all pins
161: /// </summary>
162: /// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
163: void MCP23008::SetPortValue(uint8 data)
164: {
165:     WriteRegister(GPIO, data);
166: }
```

This code should speak for itself.

#### MCP23017I2C.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017I2CH}

We'll change the class `MCP23017I2C` to inherit from `MCP23017`.

Rename the file `code/libraries/device/include/device/i2c/MCP23017.h` and update

```cpp
File: code/libraries/device/include/device/i2c/MCP23017I2C.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017I2C.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017I2C
9: //
10: // Description : MCP23017 I2C expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/I2CMaster.h"
43: #include "device/expander/MCP23017.h"
44: 
45: /// @file
46: /// MCP23017 I2C expander declaration
47: 
48: namespace device {
49: 
50: /// <summary>
51: /// Driver for MCP23017 I2C expander device
52: ///
53: /// This device is normally on I2C address 20-27 depending on how its A0-A2 pins are connected.
54: /// The device supports two 8 bit ports named port A and B.
55: /// Each pin can be either an input or output.
56: /// </summary>
57: class MCP23017I2C
58:     : public MCP23017
59: {
60: private:
61:     /// @brief I2C base device
62:     baremetal::I2CMaster m_device;
63:     /// @brief I2C device address
64:     uint8 m_address;
65: 
66: public:
67:     MCP23017I2C(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
68:     ~MCP23017I2C();
69: 
70:     bool Initialize(uint8 bus, uint8 address);
71: 
72:     uint8 ReadRegister(MCP23017RegisterIndex registerAddress) override;
73:     void WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte) override;
74: };
75: 
76: } // namespace device
```

This header is now much simpler, as the actual work is done in the `MCP23017` class.

- Line 42-43: Next to the `I2CMaster` header we also include the header for `MCP23017`
- Line 50-74: The new class `MCP23017I2C` now derives from `MCP23017`
  - Line 61-64: The `I2CMaster` and address are the only member variables left, as they are specific to I2C
  - Line 67: The constructor takes the memory access interface as before
  - Line 70: The `Initialize()` method takes the I2C bus and address as before
  - Line 72-73: All other methods are removed, except for `ReadRegister()` and `WriteRegister()`.
These are overrides of the abstract `MCP23017` class

#### MCP23017I2C.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017I2CCPP}

We'll update the implementation of the `MCP23017I2C` class.

Rename the file `code/libraries/device/src/i2c/MCP23017.cpp` and update

```cpp
File: code/libraries/device/src/i2c/MCP23017I2C.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017I2C.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017I2C
9: //
10: // Description : MCP23017 I2C expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/i2c/MCP23017I2C.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23017 I2C expander implementation
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23017I2C");
52: 
53: /// <summary>
54: /// Constructor for MCP23017 class
55: /// </summary>
56: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
57: MCP23017I2C::MCP23017I2C(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
58:     : MCP23017()
59:     , m_device{memoryAccess}
60: {
61: }
62: 
63: /// <summary>
64: /// Destructor for MCP23017 class
65: /// </summary>
66: MCP23017I2C::~MCP23017I2C()
67: {
68:     Uninitialize();
69: }
70: 
71: /// <summary>
72: /// Initialize the MCP23017 I2C expander
73: /// </summary>
74: /// <param name="bus">I2C bus index</param>
75: /// <param name="address">I2C slave address</param>
76: /// <returns>True on success, false otherwise</returns>
77: bool MCP23017I2C::Initialize(uint8 bus, uint8 address)
78: {
79:     LOG_INFO("Initialize %02x", address);
80:     m_address = address;
81:     if (!m_device.Initialize(bus, I2CClockMode::Normal, 0))
82:         return false;
83: 
84:     if (!MCP23017::Initialize())
85:         return false;
86:     return true;
87: }
88: 
89: /// <summary>
90: /// Read from the specified MCP23017 register
91: /// </summary>
92: /// <param name="registerAddress">Register index</param>
93: /// <returns>Value read</returns>
94: uint8 MCP23017I2C::ReadRegister(MCP23017RegisterIndex registerAddress)
95: {
96:     uint8 address = static_cast<uint8>(registerAddress);
97:     uint8 data{};
98:     auto bytesTransferred = m_device.WriteReadRepeatedStart(m_address, &address, 1, &data, 1);
99:     LOG_DEBUG("Read bytes from I2C %02x Register %02x: %02x, %d bytes transferred", m_address, address, data, bytesTransferred);
100:     return data;
101: }
102: 
103: /// <summary>
104: /// Write to the specified MCP23017 register
105: /// </summary>
106: /// <param name="registerAddress">Register index</param>
107: /// <param name="byte">Value to write</param>
108: void MCP23017I2C::WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte)
109: {
110:     const size_t BufferSize{2};
111:     uint8 buffer[BufferSize];
112:     buffer[0] = static_cast<uint8>(registerAddress);
113:     buffer[1] = byte;
114:     auto bytesWritten = m_device.Write(m_address, buffer, BufferSize);
115: 
116:     LOG_DEBUG("Write bytes to I2C %02x Register %02x: %02x, %d bytes written", m_address, buffer[0], buffer[1], bytesWritten);
117: }
```

- Line 40: We include the correct header
- Line 53-61: We call the constructor of `MCP23017`
- Line 63-69: We call the `Uninitialize()` method to set the pins to input mode
- Line 71-87: The `Initialize()` method is almost the same, except that we call the `Initialize()` method on `MCP23017`
- Line 89-101: We implement the method `ReadRegister()` as before
- Line 103-117: We implement the method `WriteRegister()` as before

#### MCP23008I2C.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008I2CH}

We'll add a class `MCP23008I2C` inheriting from `MCP23008`, which similar to `MCP23017I2C` implements the I2C interface to MCP23008.

Add the file `code/libraries/device/include/device/i2c/MCP23008I2C.h`

```cpp
File: code/libraries/device/include/device/i2c/MCP23008I2C.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008I2C.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008I2C
9: //
10: // Description : MCP23008 I2C expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/I2CMaster.h"
43: #include "device/expander/MCP23008.h"
44: 
45: /// @file
46: /// MCP23008 I2C expander declaration
47: 
48: namespace device {
49: 
50: /// <summary>
51: /// Driver for MCP23008 I2C expander device
52: ///
53: /// This device is normally on I2C address 20-27 depending on how its A0-A2 pins are connected.
54: /// The device supports one 8 bit ports.
55: /// Each pin can be either an input or output.
56: /// </summary>
57: class MCP23008I2C 
58:     : public MCP23008
59: {
60: private:
61:     /// @brief I2C base device
62:     baremetal::I2CMaster m_device;
63:     /// @brief I2C device address
64:     uint8 m_address;
65: 
66: public:
67:     MCP23008I2C(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
68:     ~MCP23008I2C();
69: 
70:     bool Initialize(uint8 bus, uint8 address);
71: 
72:     uint8 ReadRegister(MCP23008RegisterIndex registerAddress) override;
73:     void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte) override;
74: };
75: 
76: } // namespace device
```

This code should speak for itself. We simply declare the class MCP23008I2C based on MCP23008, similar to MCP23017I2C.

#### MCP23008I2C.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008I2CCPP}

We'll implement the `MCP23008I2C` class.

Add the file `code/libraries/device/src/i2c/MCP23008I2C.cpp`

```cpp
File: code/libraries/device/src/i2c/MCP23008I2C.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008I2C.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008I2C
9: //
10: // Description : MCP23008 I2C expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/i2c/MCP23008I2C.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23008 I2C expander implementation
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23008I2C");
52: 
53: /// <summary>
54: /// Constructor for MCP23008I2C class
55: /// </summary>
56: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
57: MCP23008I2C::MCP23008I2C(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
58:     : MCP23008()
59:     , m_device{memoryAccess}
60: {
61: }
62: 
63: /// <summary>
64: /// Destructor for MCP23008I2C class
65: /// </summary>
66: MCP23008I2C::~MCP23008I2C()
67: {
68:     Uninitialize();
69: }
70: 
71: /// <summary>
72: /// Initialize the MCP23008 I2C expander
73: /// </summary>
74: /// <param name="bus">I2C bus index</param>
75: /// <param name="address">I2C slave address</param>
76: /// <returns>True on success, false otherwise</returns>
77: bool MCP23008I2C::Initialize(uint8 bus, uint8 address)
78: {
79:     LOG_INFO("Initialize %02x", address);
80:     m_address = address;
81:     if (!m_device.Initialize(bus, I2CClockMode::Normal, 0))
82:         return false;
83: 
84:     if (!MCP23008::Initialize())
85:         return false;
86:     return true;
87: }
88: 
89: /// <summary>
90: /// Read from the specified MCP23008 register
91: /// </summary>
92: /// <param name="registerAddress">Register index</param>
93: /// <returns>Value read</returns>
94: uint8 MCP23008I2C::ReadRegister(MCP23008RegisterIndex registerAddress)
95: {
96:     uint8 address = static_cast<uint8>(registerAddress);
97:     uint8 data{};
98:     auto bytesTransferred = m_device.WriteReadRepeatedStart(m_address, &address, 1, &data, 1);
99:     LOG_DEBUG("Read bytes from I2C %02x Register %02x: %02x, %d bytes transferred", m_address, address, data, bytesTransferred);
100:     return data;
101: }
102: 
103: /// <summary>
104: /// Write to the specified MCP23008 register
105: /// </summary>
106: /// <param name="registerAddress">Register index</param>
107: /// <param name="byte">Value to write</param>
108: void MCP23008I2C::WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte)
109: {
110:     const size_t BufferSize{2};
111:     uint8 buffer[BufferSize];
112:     buffer[0] = static_cast<uint8>(registerAddress);
113:     buffer[1] = byte;
114:     auto bytesWritten = m_device.Write(m_address, buffer, BufferSize);
115: 
116:     LOG_DEBUG("Write bytes to I2C %02x Register %02x: %02x, %d bytes written", m_address, buffer[0], buffer[1], bytesWritten);
117: }
```

Again this code should speak for itself.

#### Update MCP23017SPI.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_UPDATE_MCP23017SPIH}

We'll add a class `MCP23017SPI` inheriting from `MCP23017`, which is similar to `MCP23017I2C`, but implements the SPI interface to MCP23017.

Update the file `code/libraries/device/include/device/spi/MCP23017SPI.h`

```cpp
File: code/libraries/device/include/device/spi/MCP23017SPI.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017SPI.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017SPI
9: //
10: // Description : MCP23017 SPI expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/SPIMaster.h"
43: #include "device/expander/MCP23017.h"
44: 
45: /// @file
46: /// MCP23017 SPI expander support declaration
47: 
48: namespace device {
49: 
50: /// <summary>
51: /// Driver for MCP23017 SPI expander device
52: ///
53: /// The device supports two 8 bit ports named port A and B.
54: /// Each pin can be either an input or output.
55: /// </summary>
56: class MCP23017SPI
57:     : public MCP23017
58: {
59: private:
60:     /// @brief I2C base device
61:     baremetal::SPIMaster m_device;
62:     /// @brief I2C device address
63:     baremetal::SPI_CEIndex m_ceIndex;
64: 
65: public:
66:     MCP23017SPI(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
67:     ~MCP23017SPI();
68: 
69:     bool Initialize(uint8 device, baremetal::SPI_CEIndex ceIndex);
70: 
71:     uint8 ReadRegister(MCP23017RegisterIndex registerAddress) override;
72:     void WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte) override;
73: };
74: 
75: } // namespace device
```

You will see a lot of similarity to `MCP23017I2C.h`.

- Line 42: We of course need to include the `SPIMaster` header
- Line 60-63: The member variables we use are now `SPIMaster` and a CE index
- Line 69: The `Initialize()` method now uses the device index (0 for SPI0, etc.) and the CE index (SPI_CEIndex::CE0 for CE0, etc.)

#### MCP23017SPI.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23017SPICPP}

We'll implement the `MCP23017SPI` class.

Add the file `code/libraries/device/src/spi/MCP23017SPI.cpp`

```cpp
File: code/libraries/device/src/spi/MCP23017SPI.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23017SPI.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23017SPI
9: //
10: // Description : MCP23017 SPI expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/spi/MCP23017SPI.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23017 I2C expander support declaration
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23017SPI");
52: 
53: /// <summary>
54: /// Constructor for MCP23017SPI class
55: /// </summary>
56: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
57: MCP23017SPI::MCP23017SPI(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
58:     : MCP23017()
59:     , m_device{memoryAccess}
60: {
61: }
62: 
63: /// <summary>
64: /// Destructor for MCP23017SPI class
65: /// </summary>
66: MCP23017SPI::~MCP23017SPI()
67: {
68:     Uninitialize();
69: }
70: 
71: /// <summary>
72: /// Initialize the MCP23017 SPI expander
73: /// </summary>
74: /// <param name="device">SPI device index</param>
75: /// <param name="ceIndex">SPI CE/CS to activate</param>
76: /// <returns>True on success, false otherwise</returns>
77: bool MCP23017SPI::Initialize(uint8 device, SPI_CEIndex ceIndex)
78: {
79:     LOG_INFO("Initialize SPI %02x CS %02x", device, ceIndex);
80:     m_ceIndex = ceIndex;
81:     if (!m_device.Initialize(device))
82:         return false;
83: 
84:     if (!MCP23017::Initialize())
85:         return false;
86:     return true;
87: }
88: 
89: /// <summary>
90: /// Read from the specified MCP23008 register
91: /// </summary>
92: /// <param name="registerAddress">Register index</param>
93: /// <returns>Value read</returns>
94: uint8 MCP23017SPI::ReadRegister(MCP23017RegisterIndex registerAddress)
95: {
96:     const size_t BufferSize{2};
97:     uint8 buffer[BufferSize];
98:     buffer[0] = 0b01000001;
99:     buffer[1] = static_cast<uint8>(registerAddress);
100:     uint8 data{};
101:     auto bytesTransferred = m_device.Write(m_ceIndex, buffer, BufferSize);
102:     bytesTransferred += m_device.Read(m_ceIndex, &data, 1);
103:     LOG_DEBUG("Read bytes from SPI CE %02x Register %02x: %02x, %d bytes transferred", m_ceIndex, buffer[1], data, bytesTransferred);
104:     return data;
105: }
106: 
107: /// <summary>
108: /// Write to the specified MCP23008 register
109: /// </summary>
110: /// <param name="registerAddress">Register index</param>
111: /// <param name="byte">Value to write</param>
112: void MCP23017SPI::WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte)
113: {
114:     const size_t BufferSize{3};
115:     uint8 buffer[BufferSize];
116:     buffer[0] = 0b01000000;
117:     buffer[1] = static_cast<uint8>(registerAddress);
118:     buffer[2] = byte;
119:     auto bytesWritten = m_device.Write(m_ceIndex, buffer, BufferSize);
120: 
121:     LOG_DEBUG("Write bytes to SPI CE %02x Register %02x: %02x, %d bytes written", m_ceIndex, buffer[1], buffer[2], bytesWritten);
122: }
```

Again this code is very similar to `MCP23017I2C.cpp`.

- Line 89-105: The implementation of `ReadRegister()` is different for SPI
  - Line 96-99: We need to prepend a control byte to the sequence, in this case 0x41 to signal the controller that we will perform a read action.
See [MCP23017/MCP23S17 16-Bit I/O Expander with Serial Interface](mcp23017.pdf) figure 3-4 for more information.
  - Line 101-102: We perform a seperate write and read operation to make sure the controller has time to perform the read
- Line 107-121: The implementation of `WriteRegister()` is different for SPI
  - Line 114-118: We again need to prepend a control byte to the sequence, in this case 0x40 to signal the controller that we will perform a write action.
See [MCP23017/MCP23S17 16-Bit I/O Expander with Serial Interface](mcp23017.pdf) figure 3-4 for more information

#### MCP23008SPI.h {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008SPIH}

We'll add a class `MCP23008SPI` inheriting from `MCP23008`, which similar to `MCP23017SPI` implement the SPI interface to MCP23008.

Add the file `code/libraries/device/include/device/spi/MCP23008SPI.h`

```cpp
File: code/libraries/device/include/device/spi/MCP23008SPI.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008SPI.h
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008SPI
9: //
10: // Description : MCP23008 SPI expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/SPIMaster.h"
43: #include "device/expander/MCP23008.h"
44: 
45: /// @file
46: /// MCP23008 SPI expander support declaration
47: 
48: namespace device {
49: 
50: /// <summary>
51: /// Driver for MCP23008 SPI expander device
52: ///
53: /// The device supports one 8 bit port.
54: /// Each pin can be either an input or output.
55: /// </summary>
56: class MCP23008SPI
57:     : public MCP23008
58: {
59: private:
60:     /// @brief I2C base device
61:     baremetal::SPIMaster m_device;
62:     /// @brief I2C device address
63:     baremetal::SPI_CEIndex m_ceIndex;
64: 
65: public:
66:     MCP23008SPI(baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
67:     ~MCP23008SPI();
68: 
69:     bool Initialize(uint8 device, baremetal::SPI_CEIndex ceIndex);
70: 
71:     uint8 ReadRegister(MCP23008RegisterIndex registerAddress) override;
72:     void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte) override;
73: };
74: 
75: } // namespace device
```

This code should speak for itself.

#### MCP23008SPI.cpp {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_MCP23008SPICPP}

We'll implement the `MCP23008SPI` class.

Add the file `code/libraries/device/src/spi/MCP23008SPI.cpp`

```cpp
File: code/libraries/device/src/spi/MCP23008SPI.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : MCP23008SPI.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : MCP23008SPI
9: //
10: // Description : MCP23008 SPI expander functionality
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------
39: 
40: #include "device/spi/MCP23008SPI.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: using namespace device;
45: using namespace baremetal;
46: 
47: /// @file
48: /// MCP 23008 I2C expander support declaration
49: 
50: /// @brief Define log name
51: LOG_MODULE("MCP23008SPI");
52: 
53: /// <summary>
54: /// Constructor for MCP23008SPI class
55: /// </summary>
56: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
57: MCP23008SPI::MCP23008SPI(baremetal::IMemoryAccess& memoryAccess /*= baremetal::GetMemoryAccess()*/)
58:     : MCP23008()
59:     , m_device{memoryAccess}
60: {
61: }
62: 
63: /// <summary>
64: /// Destructor for MCP23008SPI class
65: /// </summary>
66: MCP23008SPI::~MCP23008SPI()
67: {
68:     Uninitialize();
69: }
70: 
71: /// <summary>
72: /// Initialize the MCP23008 SPI expander
73: /// </summary>
74: /// <param name="device">SPI device index</param>
75: /// <param name="ceIndex">SPI CE/CS to activate</param>
76: /// <returns>True on success, false otherwise</returns>
77: bool MCP23008SPI::Initialize(uint8 device, SPI_CEIndex ceIndex)
78: {
79:     LOG_INFO("Initialize SPI %02x, CE %02x", device, static_cast<uint8>(ceIndex));
80:     m_ceIndex = ceIndex;
81:     if (!m_device.Initialize(device))
82:         return false;
83: 
84:     if (!MCP23008::Initialize())
85:         return false;
86:     return true;
87: }
88: 
89: /// <summary>
90: /// Read from the specified MCP23008 register
91: /// </summary>
92: /// <param name="registerAddress">Register index</param>
93: /// <returns>Value read</returns>
94: uint8 MCP23008SPI::ReadRegister(MCP23008RegisterIndex registerAddress)
95: {
96:     const size_t BufferSize{2};
97:     uint8 buffer[BufferSize];
98:     buffer[0] = 0b01000001;
99:     buffer[1] = static_cast<uint8>(registerAddress);
100:     uint8 data{};
101:     auto bytesTransferred = m_device.Write(m_ceIndex, buffer, BufferSize);
102:     bytesTransferred += m_device.Read(m_ceIndex, &data, 1);
103:     LOG_DEBUG("Read bytes from SPI CE %02x Register %02x: %02x, %d bytes transferred", m_ceIndex, buffer[1], data, bytesTransferred);
104:     return data;
105: }
106: 
107: /// <summary>
108: /// Write to the specified MCP23008 register
109: /// </summary>
110: /// <param name="registerAddress">Register index</param>
111: /// <param name="byte">Value to write</param>
112: void MCP23008SPI::WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte)
113: {
114:     const size_t BufferSize{3};
115:     uint8 buffer[BufferSize];
116:     buffer[0] = 0b01000000;
117:     buffer[1] = static_cast<uint8>(registerAddress);
118:     buffer[2] = byte;
119:     auto bytesWritten = m_device.Write(m_ceIndex, buffer, BufferSize);
120: 
121:     LOG_DEBUG("Write bytes to SPI CE %02x Register %02x: %02x, %d bytes written", m_ceIndex, buffer[1], buffer[2], bytesWritten);
122: }
```

This code should also speak for itself.

#### Update application code {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_UPDATE_APPLICATION_CODE}

Now let's use the actual device.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "stdlib/Util.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5: #include "device/spi/MCP23008SPI.h"
6: 
7: LOG_MODULE("main");
8: 
9: using namespace baremetal;
10: using namespace device;
11: 
12: int main()
13: {
14:     GetLogger().SetLogLevel(LogSeverity::Debug);
15: 
16:     uint8 deviceIndex = 0;
17:     SPI_CEIndex ceIndex{SPI_CEIndex::CE0};
18: 
19:     MCP23008SPI mcp;
20:     if (!mcp.Initialize(deviceIndex, ceIndex))
21:     {
22:         LOG_INFO("Cannot initialize device");
23:     }
24:     mcp.SetPortDirections(MCP23008PinDirection::Out);
25:     mcp.SetPortValue(0xFF);
26: 
27:     Timer::WaitMilliSeconds(2000);
28: 
29:     mcp.SetPortValue(0x00);
30: 
31:     LOG_INFO("Rebooting");
32: 
33:     return static_cast<int>(ReturnCode::ExitReboot);
34: }
```

- Line 5: We include the header for the SPI version of MCP23008
- Line 19: We instantiate MCP23008SPI
- Line 20: We call `Initialize()`, passing the device index for SPI0 and the used CS / CE pin CE0
- Line 24: We set the port to all outputs
- Line 25: We set the value of the pins to all on
- Line 29: We set the value of the pins to all off again

#### Configuring, building and debugging {#TUTORIAL_27_SPI_SPI_STEP_3___USING_SPI_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice that the setup of the GPIO pins is logged by the stub.

```text
1: Setting up UART0
2: Info   0.00:00:00.030 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
3: Info   0.00:00:00.050 Starting up (System:213)
4: Info   0.00:00:00.070 Initialize SPI 00, CE 00 (MCP23008SPI:79)
5: Info   0.00:00:00.090 Initialize SPI device 0 (SPIMaster:173)
6: Debug  0.00:00:00.110 GetClockRate (RPIProperties.cpp:307)
7: Debug  0.00:00:00.130 Clock ID:   00000004 (RPIProperties.cpp:308)
8: Debug  0.00:00:00.150 Result: OK (RPIProperties.cpp:309)
9: Debug  0.00:00:00.170 Rate:       0FEEACA0 (RPIProperties.cpp:314)
10: Info   0.00:00:00.200 Set clock core 267300000, divider 534, clockrate 500000 (SPIMaster:218)
11: Info   0.00:00:00.220 Set clock polarity 0, phase 0 (SPIMaster:238)
12: Info   0.00:00:00.230 Set up SPI device 0, clock rate 500000, base address FE204000 (SPIMaster:200)
13: Info   0.00:00:00.260 Initialize (MCP23008:73)
14: Debug  0.00:00:00.270 Set data size 3 (SPIMaster.cpp:299)
15: Debug  0.00:00:00.290 Start transfer (SPIMaster.cpp:302)
16: Debug  0.00:00:00.310 Read/Write (SPIMaster.cpp:309)
17: Debug  0.00:00:00.330 Write bytes to SPI CE 00 Register 05: 2C, 3 bytes written (MCP23008SPI:121)
18: Debug  0.00:00:00.350 Set data size 3 (SPIMaster.cpp:299)
19: Debug  0.00:00:00.370 Start transfer (SPIMaster.cpp:302)
20: Debug  0.00:00:00.400 Read/Write (SPIMaster.cpp:309)
21: Debug  0.00:00:00.420 Write bytes to SPI CE 00 Register 06: 00, 3 bytes written (MCP23008SPI:121)
22: Debug  0.00:00:00.440 Set data size 3 (SPIMaster.cpp:299)
23: Debug  0.00:00:00.460 Start transfer (SPIMaster.cpp:302)
24: Debug  0.00:00:00.480 Read/Write (SPIMaster.cpp:309)
25: Debug  0.00:00:00.500 Write bytes to SPI CE 00 Register 00: 00, 3 bytes written (MCP23008SPI:121)
26: Debug  0.00:00:00.520 Set data size 3 (SPIMaster.cpp:299)
27: Debug  0.00:00:00.540 Start transfer (SPIMaster.cpp:302)
28: Debug  0.00:00:00.560 Read/Write (SPIMaster.cpp:309)
29: Debug  0.00:00:00.580 Write bytes to SPI CE 00 Register 09: FF, 3 bytes written (MCP23008SPI:121)
30: Debug  0.00:00:02.600 Set data size 3 (SPIMaster.cpp:299)
31: Debug  0.00:00:02.620 Start transfer (SPIMaster.cpp:302)
32: Debug  0.00:00:02.640 Read/Write (SPIMaster.cpp:309)
33: Debug  0.00:00:02.660 Write bytes to SPI CE 00 Register 09: 00, 3 bytes written (MCP23008SPI:121)
34: Info   0.00:00:02.690 Rebooting (main:83)
35: Debug  0.00:00:02.700 Set data size 3 (SPIMaster.cpp:299)
36: Debug  0.00:00:02.720 Start transfer (SPIMaster.cpp:302)
37: Debug  0.00:00:02.730 Read/Write (SPIMaster.cpp:309)
38: Debug  0.00:00:02.750 Write bytes to SPI CE 00 Register 06: FF, 3 bytes written (MCP23008SPI:121)
39: Debug  0.00:00:02.780 Set data size 3 (SPIMaster.cpp:299)
40: Debug  0.00:00:02.790 Start transfer (SPIMaster.cpp:302)
41: Debug  0.00:00:02.810 Read/Write (SPIMaster.cpp:309)
42: Debug  0.00:00:02.830 Write bytes to SPI CE 00 Register 00: FF, 3 bytes written (MCP23008SPI:121)
43: Info   0.00:00:02.850 Reboot (System:144)
44: 
```

- Line 4: The `Initialize()` method of MCP23008SPI is called
- Line 5-12: This calls the `Initialize()` method of SPIMaster, which sets up the SPI clock and the correct GPIO pins
- Line 13: Subsequently, the `Initialize()` method of MCP23008 is called
- Line 14-17: The MCP23008 class `Initialize()` method writes 0x2C to the MCP123017 IOCON register
- Line 18-21: We write 0x00 to the MCP23008 GPPU register.
This disables the pull up resistors for the port
- Line 22-25: We write 0x00 to the MCP23008 IODIR register.
This sets all pins on the port to output
- Line 26-29: We write 0xFF to the MCP23008 GPIO register.
This sets the pin values on the output port.
This results in the first image below
- Line 71-76: We write 0x00 to the MCP23008 GPIO register.
This resets the pin values on the output port.
This results in the second image below
- Line 84-89: We write 0xFF to the MCP23008 GPPU register.
This enables the pull up resistors for the port
- Line 90-95: We write 0xFF to the MCP23008 IODIR register.
This sets all pins on the port to input

<img src="images/Tutorial_21-MCP23017-1.jpg"  alt="I2C bus" width="600"/>

<img src="images/Tutorial_21-MCP23017-2.jpg"  alt="I2C bus" width="600"/>

<img src="images/Tutorial_21-MCP23017-3.jpg"  alt="I2C bus" width="600"/>

Next: [28-i2s](28-i2s.md)
