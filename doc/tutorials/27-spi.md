# Tutorial 27: SPI {#TUTORIAL_27_SPI}

@tableofcontents

## Tutorial setup {#TUTORIAL_27_SPI_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-timer-extension`.
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

SPI stands for Serial Peripheral Interface and is a synchronous serial communication protocol.
It is commonly used for communication between microcontrollers and peripheral devices such as sensors, displays, DAC / ADC and EEPROM / flash memory chips.

SPI interfaces consist of a master device and one or more slave devices. The master device initiates communication and controls the clock signal, while the slave devices respond to the master's commands.
Raspberry Pi support both master and slave mode.

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

### Step 1 - SPI1 and SPI2 {#TUTORIAL_27_SPI_SPI_STEP_1___SPI1_AND_SPI2}

Let's start with an extension of `BCMRegisters.h` to add the registers for SPI1 and SPI2.

### BCMRegisters.h {#TUTORIAL_27_SPI_SPI_BCMREGISTERSH}

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
```

Next: [28-i2s](28-i2s.md)
