# Tutorial 00: Raspberry Pi {#TUTORIAL_00_RASPBERRY_PI}

@tableofcontents

In this first tutorial we will explain a bit about Raspberry Pi.
If you already know all the ins and outs of this board, you can skip this tutorial.
However, you might just learn something :)

## What is Raspberry Pi {#TUTORIAL_00_RASPBERRY_PI_WHAT_IS_RASPBERRY_PI}

Raspberry Pi, RPI, or simply Pi is an embedded computing board initially intended for school students to learn more about computers.
The board was created by the Raspberry Pi Foundation, and has had multiple revisions of the board.
Due to its success also outside schools, also for a large part because of its pricing and the open source approach to supporting it, it has become one of the most sold computing platforms.
Nowadays it is used in school, by hobbyists, and even in industry, with a wide range of applications.

I'm going to focus only on Raspberry Pi 3 and later, as these boards allow for 64 bit code, which is what I'd like to limit myself to for simplicity.

<img src="images/rpi3b-board-layout.png" alt="Raspberry Pi 3B" width="400"/>

Raspberry Pi 3B

<img src="images/rpi4b-board-layout.png" alt="Raspberry Pi 4B" width="400"/>

Raspberry Pi 4B

<img src="images/rpi5b-board-layout.jpg" alt="Raspberry Pi 5B" width="400"/>

Raspberry Pi 5B

The family of Raspberry Pi boards is extensive (see also [here](#RASPBERRY_PI_PLATFORMS) as well as [Wikipedia](https://en.wikipedia.org/wiki/Raspberry_Pi)):

The first board was Raspberry Pi Model B, which was soon followed by a slimmed down version with no USB and no ethernet, model A.
As of 2016, there was a revision of Model 2, which had a Cortex-A53 multi-core processor on board.
All later versions have quad core CPUs, with gradually higher clock frequencies.

Each Raspberry Pi board consists of some common components:

- SoC (System on Chip) which contains the CPU as well as the VPU (also GPU or VideoCore) produced by Broadcom
- RAM: ranging from 256Mb in the first models, to 2, 4 or 8 Gb on model 5
- Power inlet: first micro USB, later USB-C to power the board
- Video output: first composite video as well as HDMI, later only HDMI, from model 4 onwards double HDMI
- Audio output: first analog as well as HDMI, since model 5 only HDMI
- USB: some models have none, nowadays board have 2x USB 2.0 and 2x USB 3.0
- Ethernet: all model B boards have ethernet, first 100 Mb, from model 3 onwards 1 Gb (only full speed since model 4)
- Wifi: all models from model 3A onwards contain Wifi support first only 2.4 GHz, nowadays dual band 2.4 / 5.0 GHz
- Bluetooth: all models from model 3A onwards contain Bluetooth support first only 2.4 GHz, nowadays dual band 2.4 / 5.0 GHz
- GPIO (General Purpose Input / Output) which contributed much to the popularity of the board.
This enables connection of e.g. I2C, I2S, SPI, UART, PWM, and also other hardware)
- Camera and display: through as so-called MIPI interface using CSI / DSI (Camera / Display Serial Interface)

It is quite hard to find more detailed information on what's inside the SoC (due the proprietary nature of the Broadcom SoC), however a while ago a block diagram of the SoC in Raspberry Pi 4 (BCM 2711) was included in the German C'T Magazin. I based the following picture on this (while translating to English):

<img src="images/rpi4b-block-diagram.png" alt="Raspberry Pi 4 block diagram" width="800"/>

The CPU consists of 4 independent cores. The building blocks of the CPU are shown in the image below:

<img src="images/rpi4b-cpu.jpg" alt="Raspberry Pi 4 CPU" width="800"/>

If you wish to read more about the different Raspberry Pi models, see [Introduction](#INTRODUCTION).

## Raspberry Pi peripherals {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS}

The peripherals mentioned in the previous section will be descibed in a bit more detail.

### Video {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_VIDEO}

Raspberry Pi 3 has a single HDMI output capable of Full HD, Raspberry Pi 4 has a dual output capable of 4K@30 fps. Raspberry Pi 5 is able to support up to 4K@60 fps on both outputs.
Next to video the HDMI ports also support audio output for 2 channels.
Video is output by the GPU or VPU, also named VideoCore or VC.
Raspberry Pi3 has a VideoCore IV built in, Raspberry Pi 4 has VideoCore VI and Raspberry Pi 5 has VideoCore VII.
Not much is known about these circuits inside the BCM chips, as the are proprietary.
The different models show support for video encoding and decoding.
Raspberry Pi 3 did not have hardware support for decoding H.264 (1080p60 decoding and 1080p30 encoding) or H.265 (4Kp60 decoding), however Raspberry Pi 4 has builtin support for both, and even (though not at full rate) encoding of these streams.
For Raspberry Pi 5, it was decided to drop hardware accelleration of video encoding.

### Audio {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_AUDIO}

Up to Raspberry Pi 4, the board had a combined analog audio / video output in a 4 pin 3.5 mm jack.
Audio quality was reasonable, though not great. Since Raspberry Pi 5, audio can only be output through HDMI.
As audio quality was an issue for quite some time, hardware vendors came up with extension board as a solution, using the I2S interface.
See also [I2S](#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_I2S) and [Other hardware - the HAT](#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_OTHER_HARDWARE___THE_HAT).

### USB {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_USB}

Raspberry Pi 3 did support USB 2.0 on 4 ports, but as the USB host and ethernet controller were connected to the same USB bus and implemented in a single chip (LAN9514), performance was not great, both for USB and ethernet.
In Raspberry Pi 4, a new USB host connected (VLI VL805) to the PCIe bus was introduced, and networking was handled separately, also directly connected to PCIe.
This enabled support for full speed USB 3.0.

### Ethernet {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_ETHERNET}

Up to Raspberry Pi 3B, the network interface was 10/100 Mbps. The chip used for this was the SMSC LAN9514 chip.
Raspberry Pi 3B+ was the first board equipped with a 1 Gbps ethernet port. However, as stated above in [USB](#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_USB), the USB host and ethernet interface shared the same hardware bus, performance suffered, leading to a maximum network speed of around 300 Mbit/s.
This was resolved in Raspberry Pi 4, which can support a full 1 Gbps, using the BCM54213 chip, USB is handled separately.

### Wifi / Bluetooth {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_WIFI__BLUETOOTH}

Wifi and Bluetooth were introduced in Raspbery Pi 3 with the addition of the BCM 43438 chip, which supports 2.4 GHz IEEE 802.11ac b/g/n Wifi and BT 4.1 BLE.
This chip uses a SPI channel for Wifi, and a UART channel for Bluetooth. The antenna is integrated, leading to limited signal quality, but Raspberry Pi can communicate quite well using radio.
In Raspberry Pi 4 this chip was replaced by Cypress CYW43455, which supports 2.4 / 5 GHz IEEE 802.11ac b/g/n Wifi and BT 5.0 BLE.
There is a possibility to add an external antenna, however the connection point is quite hard to find and attach to.

### GPIO {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_GPIO}

GPIO is one of the main reason for the popularity of Raspberry Pi. Since Raspberry Pi 2, there is a more or less standard 40 pin header with access to different interface signals:

<img src="images/rpi-gpio-pinout-only.png" alt="Raspberry Pi 4 GPIO pinout" width="400"/>

Each of the interfaces will be described below.
It is important to be aware that not all interfaces are available, or at the same pins, at all times.
This depends on how GPIO is configured. For each pin there is a normal GPIO (simple digital signal) function as input or output, but also up to 6 special functions, depending on this configuration.
Although the pinning is mostly the same for different versions of Raspberry Pi, there are differences. Please refer to [Alternative functions for GPIO, Raspberry Pi 3](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO_RASPBERRY_PI_3),  and [Alternative functions for GPIO, Raspberry Pi 4](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO_RASPBERRY_PI_4) and [Alternative functions for GPIO, Raspberry Pi 5](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO_RASPBERRY_PI_5)

This makes it quite complicated to figure out how to connected to a certain interface, and it also limits the number of parallel devices that can be connected.
Also, be aware that only part of the GPIO are actually available on the header (GPIO 0 through GPIO 27), even though a larger number of GPIO pins is mentioned in documentation (up to GPIO54).

### I2C {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_I2C}

The I2C (Inter IC) interfaces is used for short distance, low data rate interfacing between ICs.
Raspberry Pi 3 has 2 different I2C buses, Raspberry Pi 4 and later support 7 I2C buses (of which one not usable due to the Bluetooth interface using this).

A I2C bus uses two wires, and uses the following pins:
- SDA: Serial Data
- SCL: Serial Clock

| GPIO | Alternative function | Signal | Board |
|------|----------------------|--------|-------|
| 0    | 0                    | SDA0   | RPI3/4/5
| 0    | 5                    | SDA6   | RPI4/5
| 1    | 0                    | SCL0   | RPI3/4/5
| 1    | 5                    | SCL6   | RPI4/5
| 2    | 0                    | SDA1   | RPI3/4/5
| 2    | 5                    | SDA3   | RPI4/5
| 3    | 0                    | SCL1   | RPI3/4/5
| 3    | 5                    | SCL3   | RPI4/5
| 4    | 5                    | SDA3   | RPI4/5
| 5    | 5                    | SCL3   | RPI4/5
| 6    | 5                    | SDA4   | RPI4/5
| 7    | 5                    | SCL4   | RPI4/5
| 8    | 5                    | SDA4   | RPI4/5
| 9    | 5                    | SCL4   | RPI4/5
| 10   | 5                    | SDA5   | RPI4/5
| 11   | 5                    | SCL5   | RPI4/5
| 12   | 5                    | SDA5   | RPI4/5
| 13   | 5                    | SCL5   | RPI4/5
| 22   | 5                    | SDA6   | RPI4/5
| 23   | 5                    | SCL6   | RPI4/5

SDA0 and SCL0 should not be used except for HAT boards (see [Other hardware - the HAT](#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_OTHER_HARDWARE___THE_HAT)). It provides for HAT boards to identify themselves.
SDA2/7 and SCL2/7 do not on the GPIO header, they are used for HDMI control.
SDA1,3,4,5,6 and SCL1,3,4,5,6 are meant for connecting external I2C circuits, e.g. a text display such as the Joy-IT LCD-16x2 or LCD-20x4 displays.

### I2S {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_I2S}

For digital sound, Raspberry Pi supports the I2S interface. This 4-wire interface uses the following pins.
For I2S to be available, you need to use Alternate function 0 for these pins:

- GPIO 18: PCM_CLK. Sample clock signal
- GPIO 19: PCM_FS: Frame sync, selects channel (0 = left, 1 = right)
- GPIO 20: PCM_DIN: Input data
- GPIO 21: PCM_DOUT: Output data

### SPI {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_SPI}

The SPI (or Serial Peripheral Interface) is a high data rate bi-directional point to point serial interface.
This 3+-wire interface uses the following pins:
- MOSI (Main Out Sub In)
- MISO (Main In Sub Out)
- SCLK (Serial Clock) from main to sub
- CS (Chip Select (up to 3 signals), active low) from main to sub

Raspberry Pi 3 supports 2 SPI buses, Raspberry Pi 4 and later support 7 SPI buses (of which one not usable due to the Bluetooth interface using this)

| GPIO | Alternative function | Signal     |
|------|----------------------|------------|
| 0    | 3                    | SPI3_CEO_N |
| 1    | 3                    | SPI3_MISO  |
| 2    | 3                    | SPI3_MOSI  |
| 3    | 3                    | SPI3_SCLK  |
| 4    | 3                    | SPI4_CEO_N |
| 5    | 3                    | SPI4_MISO  |
| 6    | 3                    | SPI4_MOSI  |
| 7    | 3                    | SPI4_SCLK  |
| 7    | 0                    | SPI0_CE1_N |
| 8    | 0                    | SPI0_CE0_N |
| 9    | 0                    | SPI0_MISO  |
| 10   | 0                    | SPI0_MOSI  |
| 11   | 0                    | SPI0_SCLK  |
| 12   | 3                    | SPI5_CE0_N |
| 13   | 3                    | SPI5_MISO  |
| 14   | 3                    | SPI5_MOSI  |
| 15   | 3                    | SPI5_SCLK  |
| 16   | 4                    | SPI1_CE2_N |
| 17   | 4                    | SPI1_CE1_N |
| 18   | 3                    | SPI6_CE0_N |
| 18   | 4                    | SPI1_CE0_N |
| 19   | 3                    | SPI6_MISO  |
| 19   | 4                    | SPI1_MISO  |
| 20   | 3                    | SPI6_MOSI  |
| 20   | 4                    | SPI1_MOSI  |
| 21   | 3                    | SPI6_SCLK  |
| 21   | 4                    | SPI1_SCLK  |
| 24   | 5                    | SPI3_CE1_N |
| 25   | 5                    | SPI4_CE1_N |
| 26   | 5                    | SPI5_CE1_N |
| 27   | 5                    | SPI6_CE1_N |

### UART {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_UART}

A UART (Universal Asynchronous Receiver / Transmitter) is another type of serial interface using 2 to 4 wires:
- RxD: Receive data
- TxD: Transmit data
- CTS: Clear to send
- RTS: Request to send

Raspberry Pi 3 supports 2 UART channels, Raspberry Pi 4 and later support 6 UART channels

| GPIO | Alternative function | Signal     |
|------|----------------------|------------|
| 0    | 4                    | TxD2       |
| 1    | 4                    | RxD2       |
| 2    | 4                    | CTS2       |
| 3    | 4                    | RTS2       |
| 4    | 4                    | TxD3       |
| 5    | 4                    | RxD3       |
| 6    | 4                    | CTS3       |
| 7    | 4                    | RTS3       |
| 8    | 4                    | TxD4       |
| 9    | 4                    | RxD4       |
| 10   | 4                    | CTS4       |
| 11   | 4                    | RTS4       |
| 12   | 4                    | TxD5       |
| 13   | 4                    | RxD5       |
| 14   | 0                    | TxD0       |
| 14   | 4                    | CTS5       |
| 14   | 5                    | TxD1       |
| 15   | 0                    | RxD0       |
| 15   | 4                    | RTS5       |
| 15   | 5                    | RxD1       |
| 16   | 3                    | CTS0       |
| 16   | 5                    | CTS1       |
| 17   | 3                    | RTS0       |
| 17   | 5                    | RTS1       |

Some of the UART are full 1655O UART devices, some are limited in functionality (mini UART).

### PWM {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_PWM}

PWM (Pulse Width Modulation) can be used to drive electronics that require dimming, e.g. LEDs.
Using PWM is is possible to not only turn a LED on or off, but dim it by varying the dutycycle.

Raspberry Pi supports 4 PWM signals (of which two are not usable):
- PWM0_0: PWM 0 channel 0
- PWM0_1: PWM 0 channel 1
- PWM1_0: PWM 1 channel 0
- PWM1_1: PWM 1 channel 1

| GPIO | Alternative function | Signal     |
|------|----------------------|------------|
| 12   | 0                    | PWM0_0     |
| 13   | 0                    | PWM0_1     |
| 18   | 5                    | PWM0_0     |
| 19   | 5                    | PWM0_1     |

### JTAG {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_JTAG}

JTAG (Joint Test Action Group, which is the group that initiated the interface) is a special interface which can be used to debug microcontroller systems.
Raspberry Pi also supports a JTAG interface.

A JTAG interface has 5 or 6 wires:
- TDI: Test Data In (input for large shift register)
- TDO: Test Data Out (output for large shift register)
- TCLK: Test Clock (clock for shift register, rising edge for clock in, falling edge for valid data out)
- TMS: Test Mode Select ()
- RTCK: Return Test Clock
- TRST: Test Reset

Raspberry PI 3 has two possible ways to connect JTAG, Raspberry Pi 4 and later have only one:

| GPIO | Alternative function | Signal                |
|------|----------------------|-----------------------|
| 4    | 5                    | ARM_TDI (only RPI 3)  |
| 5    | 5                    | ARM_TDO (only RPI 3)  |
| 6    | 5                    | ARM_RTCK (only RPI 3) |
| 12   | 5                    | ARM_TMS (only RPI 3)  |
| 13   | 5                    | ARM_TCK (only RPI 3)  |
| 22   | 4                    | ARM_TRST              |
| 23   | 4                    | ARM_RTCK              |
| 24   | 4                    | ARM_TDO               |
| 25   | 4                    | ARM_TCK               |
| 26   | 4                    | ARM_TDI               |
| 27   | 4                    | ARM_TMS               |

### Other hardware - the HAT {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_OTHER_HARDWARE___THE_HAT}

Quite a few suppliers provide add-on boards for Raspberry Pi, that are plugged onto the GPIO header.
These boards are commonly called HAT (Hardware At the Top):
- Experimentation boards
- Sensor boards
- PoE (Power over Ethernet) boards
- Audio boards
- etc.

Care must be taken to not use the same GPIO pins that are used by the board. Very often it is no longer possible to connect to the GPIO header because the HAT board does not have pins exposed.

I will not go into a lot of detail, but mention one thing. Specifically for the HAT, there is a specific I2C channel reserved to communicate with the board and get its identification.
This is channel 2, as mentioned in [I2C](#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_PERIPHERALS_I2C).
Do not use these pins for any other purpose.

## Raspberry Pi SoC {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_SOC}

The different version of Raspberry Pi each have a different SoC. However, many of the functions are the same or similar.

The SoC used per board is:

| Raspberry Pi model | SoC       | ARM CPU    | Documentation |
|--------------------|-----------|------------|---------------|
| 3A/B               | BCM2837   | Cortex A53 | [documentation](pdf/bcm2837-peripherals.pdf)
| 3A+/B+             | BCM2837B0 | Cortex A53 | [documentation](pdf/bcm2837-peripherals.pdf)
| 4B                 | BCM2711   | Cortex A72 | [documentation](pdf/bcm2711-peripherals.pdf)
| 5B                 | BCM2712   | Cortex A76 | so far not documented

Important to know is how the VideoCore (the part responsible for 2D/3D video processing) and the ARM CPU communicate, and how the system starts up.

The VideoCore consists of a kind of GPU, but also an ARM v6 processor and firmware.
This processor is the initial part of the system that starts.

## Raspberry Pi startup {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_STARTUP}

There is a difference between how Raspberry Pi 3 starts compared to Raspberry Pi 4 and later. I'll try make it clear below.

### Boot sequence for Raspberry Pi 3 {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_STARTUP_BOOT_SEQUENCE_FOR_RASPBERRY_PI_3}

See also [Boot sequence](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#boot-sequence)

<img src="images/boot-rpi3.png" alt="Boot sequence Raspberry Pi 3" width="800"/>

__Raspberry Pi 3 boot sequence__

1. Raspberry Pi starts up in the GPU. At this point, RAM is disabled, and the CPU is off
2. The GPU runs startup code from ROM (inside SoC) and executes first stage bootloader
3. The first stage bootloader checks for boot modes (GPIO, SD, Flash, SPI, USB).
This will attempt to retrieve the second stage bootloader in the following order:
  - If GPIO boot enabled check GPIO pins for boot mode enable
  - If SD boot enabled, load `bootcode.bin` from the first SD card into level 2 cache, start second stage boot loader if possible
  - If SD boot enabled, load `bootcode.bin` from the second SD card into level 2 cache, start second stage boot loader if possible
  - If NAND / Flash boot enabled, load second stage bootloader from flash into level 2 cache, start second stage boot loader if possible
  - If SPI boot enabled, try to boot from SPI
  - If USB boot enabled, try to boot from a mass storage device, for each mass storage device, try to load `bootcode.bin`
  - If network boot enabled (through USB boot), try to boot from a mass storage of LAN951X (ethernet) device, for each device, try to perform DHCP / TFTP boot
4. If one of the above methods is successful, the first stage bootload loads it into level 2 cache
5. Second stage bootloader is executed
6. Second stage bootloader loads the GPU firmware from `start.elf`
7. GPU firmware is executed
8. GPU firmware loads configuration parameters (optional) from `config.txt`, kernel image from `kernel8.img` (64 bit) or `kernel8-32.img` (32 bit) and kernel parameters from `cmdline.txt`.
9. The CPU is started, and starts core 0 in EL2 (unless the firmware is changed)

Be aware that a large part of the bootup is performed by the GPU (bootcode.bin, start.elf), which has different assembly code than the CPU, as the GPU contains an ARMv6 processor.

### Boot sequence for Raspberry Pi 4 / 5 {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_STARTUP_BOOT_SEQUENCE_FOR_RASPBERRY_PI_4__5}

See also [Boot flow](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-4-boot-flow)

<img src="images/boot-rpi4.png" alt="Boot sequence Raspberry Pi 4" width="800"/>

__Raspberry Pi 4 boot sequence__

<img src="images/boot-rpi5.png" alt="Boot sequence Raspberry Pi 5" width="800"/>

__Raspberry Pi 5 boot sequence__

1. Raspberry Pi starts up in the GPU. At this point, RAM is disabled, and the CPU is off
2. The GPU runs startup code from ROM (inside SoC) and executes first stage bootloader
3. First stage bootloader checks for boot modes (GPIO, SD, Flash, SPI, USB).
This will attempt to retrieve the second stage bootloader in the following order:
  - If GPIO boot enabled check GPIO pins for boot mode enable
  - If SD boot enabled, load `recovery.bin` from the SD card into level 2 cache, and reflash the SPI flash
  - Try to load `recovery.bin` from USB boot device, and reflash the SPI flash
4. Load second stage bootloader from SPI flash, start second stage boot loader if possible
5. Initialize clocks and RAM
6. Read EEPROM configuration file
7. If PM_RSTS in configurations signals power off or wake on GPIO, sleep. This will wake us with a falling edge on GPIO (RPI 4) or the power button (RPI 5)
8. Check all boot modes in boot order configuration
    1. If Restart restart boot mode enumeration
    2. If Stop display error and wait forever
    3. If SD then load GPU firmware from `start.elf` from device, and start GPU firmware if possible
    4. If Network then load GPU firmware form `start.elf` from TFTP server, and start GPU firmware if possible
    5. If USB Master Storage Device then load GPU firmware form `start.elf` from device, and start GPU firmware if possible
    6. If NVME then load GPU firmware form `start.elf` from device, and start GPU firmware if possible
    4. If RPIBoot then boot from USB OTG boot
9. GPU firmware is executed
10. GPU firmware loads configuration parameters (optional) from `config.txt`, kernel image from `kernel8-rpi4.img` (Raspberry Pi 4, 64 bit), `kernel7l.img` (Raspberry Pi 4, 32 bit),  or `kernel8_2712.img` (Raspberry Pi 5, 64 bit) and kernel parameters from `cmdline.txt`.
11. The CPU is started, and starts core 0 in EL2 (unless the firmware is changed or the stub is changed)

For RPI 5, the GPU firmware is embedded into the kernel image, so the kernel image is loaded directly.

### config.txt {#TUTORIAL_00_RASPBERRY_PI_RASPBERRY_PI_STARTUP_CONFIGTXT}

For 64 bit systems, the mimimal contents of config.txt are similar to:

```text
#
# Enable 64-bit mode (AArch64)
#
# This file must be copied along with the generated kernel8[-rpi4].img
# onto a SDHC card with FAT file system, if 64-bit mode is used.
#
# This file also enables the possibility to use two displays with
# the Raspberry Pi 4 and selects the required mode for the USB host
# controller of the Compute Module 4.
#

arm_64bit=1

[pi02]
kernel=kernel8.img

[pi2]
kernel=kernel8.img

[pi3]
kernel=kernel8.img

[pi3+]
kernel=kernel8.img

[pi4]
armstub=armstub8-rpi4.bin
kernel=kernel8-rpi4.img
max_framebuffers=2

[cm4]
otg_mode=1
```

This does not yet contain the entries for RPI 5 (which would use `kernel_2712.img`)

For more information see [Configuring Your Raspberry Pi with /boot/firmware/config.txt](https://fleetstack.io/blog/raspberry-pi-boot-config-file)

## Startup kernel for ARM CPU {#TUTORIAL_00_RASPBERRY_PI_STARTUP_KERNEL_FOR_ARM_CPU}

In order to start the CPU, we will need a 'kernel' image. This is simply put a file containing the first application that is started on the CPU.
The name of the kernel image loaded depends on the Raspberry Pi model as well as the architecture, i.e. 32 or 64 bit mode.

kernel8.img is the default kernel to start for normal 64 bit Linux distributions.

For baremetal, the defaults are as follows:

| Board  | Architecture     | Image            |
|--------|------------------|------------------|
| RPI 1  | 32 bit (Arm)     | kernel.img       |
| RPI 2  | 32 bit (Arm)     | kernel7.img      |
| RPI 3  | 32 bit (Arm)     | kernel8-32.img   |
| RPI 3  | 64 bit (AArch64) | kernel8.img      |
| RPI 4  | 32 bit (Arm)     | kernel7l.img     |
| RPI 4  | 64 bit (AArch64) | kernel8-rpi4.img |
| RPI 5  | 64 bit (AArch64) | kernel_2712.img  |

Once the image is loaded, the GPU resets the ARM, which then starts executing. The start address depends on the architecture:

### CPU execution start address {#TUTORIAL_00_RASPBERRY_PI_STARTUP_KERNEL_FOR_ARM_CPU_CPU_EXECUTION_START_ADDRESS}

| Architecture     | Start address |
|------------------|---------------|
| 32 bit (Arm)     | 0x8000        |
| 64 bit (AArch64) | 0x80000       |

This is due to the GPU placing a jump opcode at address 0x0000, which is the initial starting point.

Every core will use a defined starting address, which is loaded into memory at specific locations.
For Core 0, this is done by the firmware, for the other cores, we need to program the correct starting address in memory.

| Core | Start address location |
|------|------------------------|
| 0    | 000000D8               |
| 1    | 000000E0               |
| 2    | 000000E8               |
| 3    | 000000F0               |

I know this was a lot of information, but we need to lay a foundation in order to start programming.
The next tutorial will be about setting up for development. Be patient, there is some grounwork to do before we can start coding.

Next: [01-setting-up-for-development](01-setting-up-for-development.md)
