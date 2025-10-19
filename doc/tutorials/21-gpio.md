# Tutorial 21: GPIO {#TUTORIAL_21_GPIO}

@tableofcontents

## New tutorial setup {#TUTORIAL_21_GPIO_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/21-gpio`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_21_GPIO_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-21.a`
- a library `output/Debug/lib/stdlib-21.a`
- an application `output/Debug/bin/21-gpio.elf`
- an image in `deploy/Debug/21-gpio-image`

## Using hardware {#TUTORIAL_21_GPIO_USING_HARDWARE}

In this tutorial, we'll be adding hardware. We'll connect the hardware to GPIO pins, so we can read out a rotary encoder.
We'll then trigger on changes of the GPIO pins to generate interrupts. We'll also be debouncing the GPIO inputs using kernel timers.

Now on using hardware, be careful when connecting devices to the GPIO header. A number of things to make sure of:

- Use good quality wiring, preferably buy jumper wires, do not create them yourself, as it is tricky work and may lead to failures
- Make sure which __voltage__ to use. Raspberry Pi has both 5V and 3.3V power, but not all pins are able to support 5V. If in doubt, first get the specifications for the device
- Be __absolutely sure__ to connect to the right pins on the header. Failing to connect correctly may break your Raspberry Pi board

### The KY-040 rotary encoder {#TUTORIAL_21_GPIO_USING_HARDWARE_THE_KY_040_ROTARY_ENCODER}

The KY-040 rotary encoder is a relatively simple device to start with. It is sold as a input device for Arduino (Iduino), and is also sold by e.g. Joy-it.

<img src="images/KY-040.png"  alt="KY-040 rotary encoder" width="400"/>

It needs a supply voltage VCC (VCC or +), we can use 3.3V for this, and a ground (GND or -).
The outputs are a press switch `SW` as well as two rotary encoder outputs named `CLK` and `DT`.

We can conveniently use 5 pins close to each other on the Raspberry Pi for this, see the GPIO pinout below (this holds for all Raspberry Raspberry Pi models since version 2):

<img src="images/rpi-gpio-pinout-only.png"  alt="Raspberry Pi GPIO pinout" width="400"/>

- VCC or + is connected to pin 17 on the header) (red wire)
- SW is connected to pin 19 on the header (GPIO 10) (orange wire)
- DT is connected to pin 21 on the header (GPIO 9) (yellow wire)
- CLK is connected to pin 23 on the header (GPIO 11) (green wire)
- GND is connected to pin 25 on the header (brown wire)

<img src="images/KY-040-connected.jpg"  alt="KY-040 rotary encoder connected" width="400"/>
<img src="images/KY-040-RPI-connection.jpg"  alt="KY-040 rotary encoder connection Raspberry Pi" width="400"/>

When the rotary encoder is connected, we can expect the following signal patterns (see also [datasheet](pdf/KY-040-Rotary-Encoder-Datasheet.pdf)):

<img src="images/KY-040-signals.png"  alt="KY-040 rotary encoder" width="400"/>

Here the A to C switch corresponds to the `CLK` output, and the B to C switch to the `DT` output.

## Faking GPIO  - Step 1 {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1}

We'll be using the GPIO pins, and sometimes, as we don't have actual HW available, or simply for debugging purposes, it's handy to have a fake implementation available.
So let's try to create that to start with.

We'll do this by creating a IMemoryAccess instance that handles the access to GPIO registers.
As we'll see, due to the fact that we also need to program GPIO to correctly assign the pins, we'll also be creating a fake IMemoryAccess instance for GPIO.
For this, we'll take the following steps:
- We'll update BCMRegisters.h a bit so that we can calculate a GPIO register adress from the register offset and vice versa
- We'll generalize IMemoryAccess and the MemoryAccess implementation
- We'll declare and implement a GPIO MemoryAccess stub
- Then we'll use this stub and get the results

### BCMRegisters.h {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_BCMREGISTERSH}

First let's revisit the GPIO registers, so we can understand how to deal with setting up interrupts.

In summary, the `BCMRegisters.h` header contains the following information:

| Register           | Address         | Bits  | Name                                  | Meaning |
|--------------------|-----------------|-------|---------------------------------------|---------|
| RPI_GPIO_GPFSEL0   | Base+0x00200000 | 29:0  | FSEL9:0                               | Function select GPIO9:0
| RPI_GPIO_GPFSEL1   | Base+0x00200004 | 29:0  | FSEL19:10                             | Function select GPIO19:10
| RPI_GPIO_GPFSEL2   | Base+0x00200008 | 29:0  | FSEL29:20                             | Function select GPIO29:20
| RPI_GPIO_GPFSEL3   | Base+0x0020000C | 29:0  | FSEL39:30                             | Function select GPIO39:30
| RPI_GPIO_GPFSEL4   | Base+0x00200010 | 29:0  | FSEL49:40                             | Function select GPIO49:40
| RPI_GPIO_GPFSEL5   | Base+0x00200014 | 11:0  | FSEL53:50                             | Function select GPIO53
| -                  | Base+0x00200018 |       | -                                     | Reserved
| RPI_GPIO_GPSET0    | Base+0x0020001C | 31:0  | SET31:0                               | Set GPIO31:0
| RPI_GPIO_GPSET1    | Base+0x00200020 | 21:0  | SET53:32                              | Set GPIO53:32
| -                  | Base+0x00200024 |       | -                                     | Reserved
| RPI_GPIO_GPCLR0    | Base+0x00200028 | 31:0  | CLR31:0                               | Clear GPIO31:0
| RPI_GPIO_GPCLR1    | Base+0x0020002C | 21:0  | CLR53:32                              | Clear GPIO53:32
| -                  | Base+0x00200030 |       | -                                     | Reserved
| RPI_GPIO_GPLEV0    | Base+0x00200034 | 31:0  | LEV31:0                               | Level GPIO31:0
| RPI_GPIO_GPLEV1    | Base+0x00200038 | 21:0  | LEV53:32                              | Level GPIO53:32
| -                  | Base+0x0020003C |       | -                                     | Reserved
| RPI_GPIO_GPEDS0    | Base+0x00200040 | 31:0  | EDS31:0                               | Event detected GPIO31:0
| RPI_GPIO_GPEDS1    | Base+0x00200044 | 21:0  | EDS53:32                              | Event detected GPIO53:32
| -                  | Base+0x00200048 |       | -                                     | Reserved
| RPI_GPIO_GPREN0    | Base+0x0020004C | 31:0  | GPREN31:0                             | Rising edge detect enable GPIO31:0
| RPI_GPIO_GPREN1    | Base+0x00200050 | 21:0  | GPREN53:32                            | Rising edge detect enable GPIO53:32
| -                  | Base+0x00200054 |       | -                                     | Reserved
| RPI_GPIO_GPFEN0    | Base+0x00200058 | 31:0  | GPFEN31:0                             | Falling edge detect enable GPIO31:0
| RPI_GPIO_GPFEN1    | Base+0x0020005C | 21:0  | GPFEN53:32                            | Falling edge detect enable GPIO53:32
| -                  | Base+0x00200060 |       | -                                     | Reserved
| RPI_GPIO_GPHEN0    | Base+0x00200064 | 31:0  | GPHEN31:0                             | High level detect enable GPIO31:0
| RPI_GPIO_GPHEN1    | Base+0x00200068 | 21:0  | GPHEN53:32                            | High level detect enable GPIO53:32
| -                  | Base+0x0020006C |       | -                                     | Reserved
| RPI_GPIO_GPLEN0    | Base+0x00200070 | 31:0  | GPLEN31:0                             | Low level detect enable GPIO31:0
| RPI_GPIO_GPLEN1    | Base+0x00200074 | 21:0  | GPLEN53:32                            | Low level detect enable GPIO53:32
| -                  | Base+0x00200078 |       | -                                     | Reserved
| RPI_GPIO_GPAREN0   | Base+0x0020007C | 31:0  | GPAREN31                              | Asynchronous rising edge detect enable GPIO31:0
| RPI_GPIO_GPAREN1   | Base+0x00200080 | 21:0  | GPAREN53                              | Asynchronous rising edge detect enable GPIO53:32
| -                  | Base+0x00200084 |       | -                                     | Reserved
| RPI_GPIO_GPAFEN0   | Base+0x00200088 | 31:0  | GPAFEN31                              | Asynchronous falling edge detect enable GPIO31:0
| RPI_GPIO_GPAFEN1   | Base+0x0020008C | 21:0  | GPAFEN53                              | Asynchronous falling edge detect enable GPIO53:32
| -                  | Base+0x00200090 |       | -                                     | Reserved
| RPI_GPIO_GPPUD     | Base+0x00200094 | 1:0   | PUD      (RPI3)                       | Pull up/down mode
| RPI_GPIO_GPPUDCLK0 | Base+0x00200098 | 31:0  | PUDCLK31 (RPI3)                       | Pull up/down set GPIO31:0
| RPI_GPIO_GPPUDCLK1 | Base+0x0020009C | 21:0  | PUDCLK53 (RPI3)                       | Pull up/down set GPIO53:32
| -                  | Base+0x002000A0 |       | -                                     | Reserved
| -                  | Base+0x002000A4 |       | -                                     | Reserved
| -                  | Base+0x002000A8 |       | -                                     | Reserved
| -                  | Base+0x002000AC |       | -                                     | Reserved
| -                  | Base+0x002000B0 |       | -                                     | Test
| RPI_GPIO_GPPINMUXSD| Base+0x002000D0 | -     | Undocumented                          | GPIO pull up/down pin multiplexer register
| RPI_GPIO_GPPUPPDN0 | Base+0x002000E4 | 31:0  | PUD15:0  (RPI 4 and later only)       | Pull up/down mode GPIO15:0
| RPI_GPIO_GPPUPPDN1 | Base+0x002000E8 | 31:0  | PUD31:16 (RPI 4 and later only)       | Pull up/down mode GPIO31:16
| RPI_GPIO_GPPUPPDN2 | Base+0x002000EC | 31:0  | PUD47:32 (RPI 4 and later only)       | Pull up/down mode GPIO47:32
| RPI_GPIO_GPPUPPDN3 | Base+0x002000F0 | 17:0  | PUD56:48 (RPI 4 and later only)       | Pull up/down mode GPIO56:48

Taking away all the details, we see a pattern:

- Starting from RPI_GPIO_GPSET0, we see RPI_GPIO_GPSET1, and then an unused space
- Again for RPI_GPIO_GPCLR0, we see RPI_GPIO_GPCLR1, and then an unused space
- etc.

So all these registers are in blocks of 3.
Looking at the groups, we see:
- RPI_GPIO_GPSETn to set a GPIO
- RPI_GPIO_GPCLRn to clear a GPIO
- RPI_GPIO_GPLEVn to get a GPIO level
- RPI_GPIO_GPEDSn to get the event flag for GPIO
- RPI_GPIO_GPRENn to enable an interrupt on a rising edge for a GPIO
- RPI_GPIO_GPFENn to enable an interrupt on a falling edge for a GPIO
- RPI_GPIO_GPHENn to enable an interrupt on a high level for a GPIO
- RPI_GPIO_GPLENn to enable an interrupt on a low level for a GPIO
- RPI_GPIO_GPARENn to enable an interrupt on a asynchronous rising edge for a GPIO
- RPI_GPIO_GPAFENn to enable an interrupt on a asynchronous falling edge for a GPIO

Each of these blocks is 12 bytes (3 times 4) in size.

We'll be adding definitions for the offset of each GPIO register relative to the GPIO base address.

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
286: //---------------------------------------------
287: // Raspberry Pi GPIO
288: //---------------------------------------------
289: 
290: /// @brief Raspberry Pi GPIO registers base address. See @ref RASPBERRY_PI_GPIO
291: #define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
292: /// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
293: #define RPI_GPIO_GPFSEL0_OFFSET         0x00000000
294: /// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
295: #define RPI_GPIO_GPFSEL0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL0_OFFSET)
296: /// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
297: #define RPI_GPIO_GPFSEL1_OFFSET         0x00000004
298: /// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
299: #define RPI_GPIO_GPFSEL1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL1_OFFSET)
300: /// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
301: #define RPI_GPIO_GPFSEL2_OFFSET         0x00000008
302: /// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
303: #define RPI_GPIO_GPFSEL2                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL2_OFFSET)
304: /// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
305: #define RPI_GPIO_GPFSEL3_OFFSET         0x0000000C
306: /// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
307: #define RPI_GPIO_GPFSEL3                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL3_OFFSET)
308: /// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
309: #define RPI_GPIO_GPFSEL4_OFFSET         0x00000010
310: /// @brief Raspberry Pi GPIO function select register 3 (GPIO 40..49) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
311: #define RPI_GPIO_GPFSEL4                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL4_OFFSET)
312: /// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
313: #define RPI_GPIO_GPFSEL5_OFFSET         0x00000014
314: /// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..59) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
315: #define RPI_GPIO_GPFSEL5                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFSEL5_OFFSET)
316: /// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
317: #define RPI_GPIO_GPSET0_OFFSET          0x0000001C
318: /// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
319: #define RPI_GPIO_GPSET0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPSET0_OFFSET)
320: /// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
321: #define RPI_GPIO_GPSET1_OFFSET          0x00000020
322: /// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
323: #define RPI_GPIO_GPSET1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPSET1_OFFSET)
324: /// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
325: #define RPI_GPIO_GPCLR0_OFFSET          0x00000028
326: /// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
327: #define RPI_GPIO_GPCLR0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPCLR0_OFFSET)
328: /// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
329: #define RPI_GPIO_GPCLR1_OFFSET          0x0000002C
330: /// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
331: #define RPI_GPIO_GPCLR1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPCLR1_OFFSET)
332: /// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
333: #define RPI_GPIO_GPLEV0_OFFSET          0x00000034
334: /// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
335: #define RPI_GPIO_GPLEV0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEV0_OFFSET)
336: /// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
337: #define RPI_GPIO_GPLEV1_OFFSET          0x00000038
338: /// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
339: #define RPI_GPIO_GPLEV1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEV1_OFFSET)
340: /// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
341: #define RPI_GPIO_GPEDS0_OFFSET          0x00000040
342: /// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
343: #define RPI_GPIO_GPEDS0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPEDS0_OFFSET)
344: /// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R) offset. See @ref RASPBERRY_PI_GPIO
345: #define RPI_GPIO_GPEDS1_OFFSET          0x00000044
346: /// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
347: #define RPI_GPIO_GPEDS1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPEDS1_OFFSET)
348: /// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
349: #define RPI_GPIO_GPREN0_OFFSET          0x0000004C
350: /// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
351: #define RPI_GPIO_GPREN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPREN0_OFFSET)
352: /// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
353: #define RPI_GPIO_GPREN1_OFFSET          0x00000050
354: /// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
355: #define RPI_GPIO_GPREN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPREN1_OFFSET)
356: /// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
357: #define RPI_GPIO_GPFEN0_OFFSET          0x00000058
358: /// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
359: #define RPI_GPIO_GPFEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFEN0_OFFSET)
360: /// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
361: #define RPI_GPIO_GPFEN1_OFFSET          0x0000005C
362: /// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
363: #define RPI_GPIO_GPFEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPFEN1_OFFSET)
364: /// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
365: #define RPI_GPIO_GPHEN0_OFFSET          0x00000064
366: /// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
367: #define RPI_GPIO_GPHEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPHEN0_OFFSET)
368: /// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
369: #define RPI_GPIO_GPHEN1_OFFSET          0x00000068
370: /// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
371: #define RPI_GPIO_GPHEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPHEN1_OFFSET)
372: /// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
373: #define RPI_GPIO_GPLEN0_OFFSET          0x00000070
374: /// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
375: #define RPI_GPIO_GPLEN0                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEN0_OFFSET)
376: /// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
377: #define RPI_GPIO_GPLEN1_OFFSET          0x00000074
378: /// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
379: #define RPI_GPIO_GPLEN1                 reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPLEN1_OFFSET)
380: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
381: #define RPI_GPIO_GPAREN0_OFFSET         0x0000007C
382: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
383: #define RPI_GPIO_GPAREN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAREN0_OFFSET)
384: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
385: #define RPI_GPIO_GPAREN1_OFFSET         0x00000080
386: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
387: #define RPI_GPIO_GPAREN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAREN1_OFFSET)
388: /// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
389: #define RPI_GPIO_GPAFEN0_OFFSET         0x00000088
390: /// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
391: #define RPI_GPIO_GPAFEN0                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAFEN0_OFFSET)
392: /// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. See @ref RASPBERRY_PI_GPIO
393: #define RPI_GPIO_GPAFEN1_OFFSET         0x0000008C
394: /// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
395: #define RPI_GPIO_GPAFEN1                reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPAFEN1_OFFSET)
396: #if BAREMETAL_RPI_TARGET == 3
397: /// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
398: #define RPI_GPIO_GPPUD_OFFSET     0x00000094
399: /// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
400: #define RPI_GPIO_GPPUD            reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUD_OFFSET)
401: /// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
402: #define RPI_GPIO_GPPUDCLK0_OFFSET 0x00000098
403: /// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
404: #define RPI_GPIO_GPPUDCLK0        reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUDCLK0_OFFSET)
405: /// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W) offset. Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
406: #define RPI_GPIO_GPPUDCLK1_OFFSET 0x0000009C
407: /// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
408: #define RPI_GPIO_GPPUDCLK1        reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUDCLK1_OFFSET)
409: #elif BAREMETAL_RPI_TARGET == 4
410: /// @brief Raspberry Pi GPIO pull up/down pin multiplexer register offset. Undocumented
411: #define RPI_GPIO_GPPINMUXSD_OFFSET 0x000000D0
412: /// @brief Raspberry Pi GPIO pull up/down pin multiplexer register. Undocumented
413: #define RPI_GPIO_GPPINMUXSD        reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPINMUXSD_OFFSET)
414: /// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
415: #define RPI_GPIO_GPPUPPDN0_OFFSET  0x000000E4
416: /// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
417: #define RPI_GPIO_GPPUPPDN0         reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN0_OFFSET)
418: /// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
419: #define RPI_GPIO_GPPUPPDN1_OFFSET  0x000000E8
420: /// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
421: #define RPI_GPIO_GPPUPPDN1         reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN1_OFFSET)
422: /// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
423: #define RPI_GPIO_GPPUPPDN2_OFFSET  0x000000EC
424: /// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
425: #define RPI_GPIO_GPPUPPDN2         reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN2_OFFSET)
426: /// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W) offset. Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
427: #define RPI_GPIO_GPPUPPDN3_OFFSET  0x000000F0
428: /// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
429: #define RPI_GPIO_GPPUPPDN3         reinterpret_cast<regaddr>(RPI_GPIO_BASE + RPI_GPIO_GPPUPPDN3_OFFSET)
430: #else // RPI target 5
431: // Not supported yet
432: #endif
433: /// @brief End of GPIO register region
434: #define RPI_GPIO_END                           RPI_GPIO_BASE + 0x00000100
```

- Line 290-432: We add a definition for the relative offset of each GPIO register, and use this to calculate the actual address.
We can later use the offset
- Line 431: We add a definition `RPI_GPIO_END` to denote the end of the GPIO registers.
We'll use this later to distinguish between I2C and GPIO registers

### IMemoryAccess.h {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_IMEMORYACCESSH}

As we'll be implementing the same kind of functions multiple times, we should see if there is any potential code duplication we can get around.
The methods `ReadModifyWrite8`, `ReadModifyWrite16` and `ReadModifyWrite32` all read a value, manipulate it and then write the result back.
This will be the same for all derived classes, so we'll implement them as part of the interface.

Update the file `code/libraries/baremetal/include/baremetal/IMemoryAccess.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IMemoryAccess.h
...
49: /// <summary>
50: /// Abstract memory access interface
51: /// </summary>
52: class IMemoryAccess
53: {
54: public:
55:     /// <summary>
56:     /// Default destructor needed for abstract interface
57:     /// </summary>
58:     virtual ~IMemoryAccess() = default;
59: 
60:     /// <summary>
61:     /// Read a 8 bit value from register at address
62:     /// </summary>
63:     /// <param name="address">Address of register</param>
64:     /// <returns>8 bit register value</returns>
65:     virtual uint8 Read8(regaddr address) = 0;
66:     /// <summary>
67:     /// Write a 8 bit value to register at address
68:     /// </summary>
69:     /// <param name="address">Address of register</param>
70:     /// <param name="data">Data to write</param>
71:     virtual void Write8(regaddr address, uint8 data) = 0;
72:     /// <summary>
73:     /// Read, modify and write a 8 bit value to register at address
74:     ///
75:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (8 bits) provided
76:     /// The data provided (8 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
77:     /// The result will then be written back to the register
78:     /// </summary>
79:     /// <param name="address">Address of register</param>
80:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
81:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
82:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
83:     void ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)
84:     {
85:         auto value = Read8(address);
86:         value &= ~mask;
87:         value |= ((data << shift) & mask);
88:         Write8(address, value);
89:     }
90: 
91:     /// <summary>
92:     /// Read a 16 bit value from register at address
93:     /// </summary>
94:     /// <param name="address">Address of register</param>
95:     /// <returns>16 bit register value</returns>
96:     virtual uint16 Read16(regaddr address) = 0;
97:     /// <summary>
98:     /// Write a 16 bit value to register at address
99:     /// </summary>
100:     /// <param name="address">Address of register</param>
101:     /// <param name="data">Data to write</param>
102:     virtual void Write16(regaddr address, uint16 data) = 0;
103:     /// <summary>
104:     /// Read, modify and write a 16 bit value to register at address
105:     ///
106:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (16 bits) provided
107:     /// The data provided (16 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
108:     /// The result will then be written back to the register
109:     /// </summary>
110:     /// <param name="address">Address of register</param>
111:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
112:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
113:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
114:     void ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift)
115:     {
116:         auto value = Read16(address);
117:         value &= ~mask;
118:         value |= ((data << shift) & mask);
119:         Write16(address, value);
120:     }
121: 
122:     /// <summary>
123:     /// Read a 32 bit value from register at address
124:     /// </summary>
125:     /// <param name="address">Address of register</param>
126:     /// <returns>32 bit register value</returns>
127:     virtual uint32 Read32(regaddr address) = 0;
128:     /// <summary>
129:     /// Write a 32 bit value to register at address
130:     /// </summary>
131:     /// <param name="address">Address of register</param>
132:     /// <param name="data">Data to write</param>
133:     virtual void Write32(regaddr address, uint32 data) = 0;
134:     /// <summary>
135:     /// Read, modify and write a 32 bit value to register at address
136:     ///
137:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (32 bits) provided
138:     /// The data provided (32 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
139:     /// The result will then be written back to the register
140:     /// </summary>
141:     /// <param name="address">Address of register</param>
142:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
143:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
144:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
145:     void ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift)
146:     {
147:         auto value = Read32(address);
148:         value &= ~mask;
149:         value |= ((data << shift) & mask);
150:         Write32(address, value);
151:     }
152: };
...
```

- Line 83-89: Instead of declaring a pure virtual method `ReadModifyWrite8()`, we implement it directly in the interface
- Line 114-120: Instead of declaring a pure virtual method `ReadModifyWrite16()`, we implement it directly in the interface
- Line 145-151: Instead of declaring a pure virtual method `ReadModifyWrite32()`, we implement it directly in the interface

### MemoryAccess.h {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_MEMORYACCESSH}

We'll remove the three methods mentioned above from `MemoryAccess`.

Update the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryAccess.h
...
49: /// <summary>
50: /// Memory access interface
51: /// </summary>
52: class MemoryAccess : public IMemoryAccess
53: {
54: public:
55:     uint8 Read8(regaddr address) override;
56:     void Write8(regaddr address, uint8 data) override;
57: 
58:     uint16 Read16(regaddr address) override;
59:     void Write16(regaddr address, uint16 data) override;
60: 
61:     uint32 Read32(regaddr address) override;
62:     void Write32(regaddr address, uint32 data) override;
63: };
...
```

### MemoryAccess.cpp {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_MEMORYACCESSCPP}

We'll also remove the three methods mentioned above from the `MemoryAccess` implementation.

Update the file `code/libraries/baremetal/src/MemoryAccess.cpp`

```cpp
File: code/libraries/baremetal/src/MemoryAccess.cpp
...
47: /// <summary>
48: /// Read a 8 bit value from register at address
49: /// </summary>
50: /// <param name="address">Address of register</param>
51: /// <returns>8 bit register value</returns>
52: uint8 MemoryAccess::Read8(regaddr address)
53: {
54:     return *reinterpret_cast<uint8 volatile*>(address);
55: }
56: 
57: /// <summary>
58: /// Write a 8 bit value to register at address
59: /// </summary>
60: /// <param name="address">Address of register</param>
61: /// <param name="data">Data to write</param>
62: void MemoryAccess::Write8(regaddr address, uint8 data)
63: {
64:     *reinterpret_cast<uint8 volatile*>(address) = data;
65: }
66: 
67: /// <summary>
68: /// Read a 16 bit value from register at address
69: /// </summary>
70: /// <param name="address">Address of register</param>
71: /// <returns>16 bit register value</returns>
72: uint16 MemoryAccess::Read16(regaddr address)
73: {
74:     return *reinterpret_cast<uint16 volatile*>(address);
75: }
76: 
77: /// <summary>
78: /// Write a 16 bit value to register at address
79: /// </summary>
80: /// <param name="address">Address of register</param>
81: /// <param name="data">Data to write</param>
82: void MemoryAccess::Write16(regaddr address, uint16 data)
83: {
84:     *reinterpret_cast<uint16 volatile*>(address) = data;
85: }
86: 
87: /// <summary>
88: /// Read a 32 bit value from register at address
89: /// </summary>
90: /// <param name="address">Address of register</param>
91: /// <returns>32 bit register value</returns>
92: uint32 MemoryAccess::Read32(regaddr address)
93: {
94:     return *reinterpret_cast<uint32 volatile*>(address);
95: }
96: 
97: /// <summary>
98: /// Write a 32 bit value to register at address
99: /// </summary>
100: /// <param name="address">Address of register</param>
101: /// <param name="data">Data to write</param>
102: void MemoryAccess::Write32(regaddr address, uint32 data)
103: {
104:     *reinterpret_cast<uint32 volatile*>(address) = data;
105: }
106: 
107: /// <summary>
108: /// Construct the singleton memory access interface if needed, and return a reference to the instance
109: /// </summary>
110: /// <returns>Reference to the singleton memory access interface</returns>
111: MemoryAccess& baremetal::GetMemoryAccess()
112: {
113:     static MemoryAccess value;
114:     return value;
115: }
```

### MemoryAccessStubGPIO.h {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_MEMORYACCESSSTUBGPIOH}

We'll define a `MemoryAccess` implementation that will fake the GPIO register access.
We'll place this in a separate folder underneath the baremetal include directory.

Create the file `code/libraries/baremetal/include/baremetal/stubs/MemoryAccessStubGPIO.h`

```cpp
File: code/libraries/baremetal/include/baremetal/stubs/MemoryAccessStubGPIO.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessStubGPIO.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessStubGPIO
9: //
10: // Description : GPIO register memory access stub
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
42: #include "baremetal/IMemoryAccess.h"
43: #include "stdlib/Macros.h"
44: 
45: /// @file
46: /// MemoryAccessStubGPIO
47: 
48: namespace baremetal {
49: 
50: /// @brief GPIO registers storage
51: struct GPIORegisters
52: {
53:     /// @brief RPI_GPIO_GPFSEL0 value
54:     uint32 FunctionSelect0; // 0x00
55:     /// @brief RPI_GPIO_GPFSEL1 value
56:     uint32 FunctionSelect1; // 0x04
57:     /// @brief RPI_GPIO_GPFSEL2 value
58:     uint32 FunctionSelect2; // 0x08
59:     /// @brief RPI_GPIO_GPFSEL3 value
60:     uint32 FunctionSelect3; // 0x0C
61:     /// @brief RPI_GPIO_GPFSEL4 value
62:     uint32 FunctionSelect4; // 0x10
63:     /// @brief RPI_GPIO_GPFSEL5 value
64:     uint32 FunctionSelect5; // 0x14
65:     /// @brief Reserved value
66:     uint32 Reserved_1; // 0x18
67:     /// @brief RPI_GPIO_GPSET0 value
68:     uint32 Set0; // 0x1C
69:     /// @brief RPI_GPIO_GPSET1 value
70:     uint32 Set1; // 0x20
71:     /// @brief Reserved value
72:     uint32 Reserved_2; // 0x24
73:     /// @brief RPI_GPIO_GPCLR0 value
74:     uint32 Clear0; // 0x28
75:     /// @brief RPI_GPIO_GPCLR1 value
76:     uint32 Clear1; // 0x2C
77:     /// @brief Reserved value
78:     uint32 Reserved_3; // 0x30
79:     /// @brief RPI_GPIO_GPLEV0 value
80:     uint32 PinLevel0; // 0x34
81:     /// @brief RPI_GPIO_GPLEV1 value
82:     uint32 PinLevel1; // 0x38
83:     /// @brief Reserved value
84:     uint32 Reserved_4; // 0x3C
85:     /// @brief RPI_GPIO_GPEDS0 value
86:     uint32 EventDetectStatus0; // 0x40
87:     /// @brief RPI_GPIO_GPEDS1 value
88:     uint32 EventDetectStatus1; // 0x44
89:     /// @brief Reserved value
90:     uint32 Reserved_5; // 0x48
91:     /// @brief RPI_GPIO_GPREN0 value
92:     uint32 RisingEdgeDetectEn0; // 0x4C
93:     /// @brief RPI_GPIO_GPREN1 value
94:     uint32 RisingEdgeDetectEn1; // 0x50
95:     /// @brief Reserved value
96:     uint32 Reserved_6; // 0x54
97:     /// @brief RPI_GPIO_GPFEN0 value
98:     uint32 FallingEdgeDetectEn0; // 0x58
99:     /// @brief RPI_GPIO_GPFEN1 value
100:     uint32 FallingEdgeDetectEn1; // 0x5C
101:     /// @brief Reserved value
102:     uint32 Reserved_7; // 0x60
103:     /// @brief RPI_GPIO_GPHEN0 value
104:     uint32 HighDetectEn0; // 0x64
105:     /// @brief RPI_GPIO_GPHEN1 value
106:     uint32 HighDetectEn1; // 0x68
107:     /// @brief Reserved value
108:     uint32 Reserved_8; // 0x6C
109:     /// @brief RPI_GPIO_GPLEN0 value
110:     uint32 LowDetectEn0; // 0x70
111:     /// @brief RPI_GPIO_GPLEN1 value
112:     uint32 LowDetectEn1; // 0x74
113:     /// @brief Reserved value
114:     uint32 Reserved_9; // 0x78
115:     /// @brief RPI_GPIO_GPAREN0 value
116:     uint32 AsyncRisingEdgeDetectEn0; // 0x7C
117:     /// @brief RPI_GPIO_GPAREN1 value
118:     uint32 AsyncRisingEdgeDetectEn1; // 0x80
119:     /// @brief Reserved value
120:     uint32 Reserved_10; // 0x84
121:     /// @brief RPI_GPIO_GPAFEN0 value
122:     uint32 AsyncFallingEdgeDetectEn0; // 0x88
123:     /// @brief RPI_GPIO_GPAFEN1 value
124:     uint32 AsyncFallingEdgeDetectEn1; // 0x8C
125:     /// @brief Reserved value
126:     uint32 Reserved_11; // 0x90
127:     /// @brief RPI_GPIO_GPPUD value
128:     uint32 PullUpDownEnable; // 0x94
129:     /// @brief RPI_GPIO_GPPUDCLK0 value
130:     uint32 PullUpDownEnableClock0; // 0x98
131:     /// @brief RPI_GPIO_GPPUDCLK1 value
132:     uint32 PullUpDownEnableClock1; // 0x9C
133:     /// @brief Reserved value
134:     uint32 Reserved_12[4]; // 0xA0-AC
135:     /// @brief Test register value
136:     uint32 Test; // 0xB0
137: #if BAREMETAL_RPI_TARGET == 4
138:     /// @brief Reserved value
139:     uint32 Reserved_13;     // 0xB4
140:     uint32 Reserved_14[10]; // 0xB8-DC
141:     uint32 Reserved_15;     // 0xE0
142:     /// @brief RPI_GPIO_GPPUPPDN0 value
143:     uint32 PullUpDown0; // 0xE4
144:     /// @brief RPI_GPIO_GPPUPPDN1 value
145:     uint32 PullUpDown1; // 0xE8
146:     /// @brief RPI_GPIO_GPPUPPDN2 value
147:     uint32 PullUpDown2; // 0xEC
148:     /// @brief RPI_GPIO_GPPUPPDN3 value
149:     uint32 PullUpDown3; // 0xF0
150:     /// @brief Reserved value
151:     uint32 Reserved_16;    // 0xF4: Alignment
152:     uint32 Reserved_17[2]; // 0xF8-FC: Alignment
153: #endif
154: 
155:     /// <summary>
156:     /// Constructor for GPIORegisters
157:     ///
158:     /// Sets default register values
159:     /// </summary>
160:     GPIORegisters()
161:         : FunctionSelect0{}
162:         , FunctionSelect1{}
163:         , FunctionSelect2{}
164:         , FunctionSelect3{}
165:         , FunctionSelect4{}
166:         , FunctionSelect5{}
167:         , Reserved_1{}
168:         , Set0{}
169:         , Set1{}
170:         , Reserved_2{}
171:         , Clear0{}
172:         , Clear1{}
173:         , Reserved_3{}
174:         , PinLevel0{}
175:         , PinLevel1{}
176:         , Reserved_4{}
177:         , EventDetectStatus0{}
178:         , EventDetectStatus1{}
179:         , Reserved_5{}
180:         , RisingEdgeDetectEn0{}
181:         , RisingEdgeDetectEn1{}
182:         , Reserved_6{}
183:         , FallingEdgeDetectEn0{}
184:         , FallingEdgeDetectEn1{}
185:         , Reserved_7{}
186:         , HighDetectEn0{}
187:         , HighDetectEn1{}
188:         , Reserved_8{}
189:         , LowDetectEn0{}
190:         , LowDetectEn1{}
191:         , Reserved_9{}
192:         , AsyncRisingEdgeDetectEn0{}
193:         , AsyncRisingEdgeDetectEn1{}
194:         , Reserved_10{}
195:         , AsyncFallingEdgeDetectEn0{}
196:         , AsyncFallingEdgeDetectEn1{}
197:         , Reserved_11{}
198:         , PullUpDownEnable{}
199:         , PullUpDownEnableClock0{}
200:         , PullUpDownEnableClock1{}
201:         , Reserved_12{}
202:         , Test{}
203: #if BAREMETAL_RPI_TARGET == 4
204:         , Reserved_13{}
205:         , Reserved_14{}
206:         , Reserved_15{}
207:         , PullUpDown0{0xAAA95555}
208:         , PullUpDown1{0xA0AAAAAA}
209:         , PullUpDown2{0x50AAA95A}
210:         , PullUpDown3{0x00055555}
211:         , Reserved_16{}
212:         , Reserved_17{}
213: #endif
214:     {
215:     }
216: } PACKED;
217: 
218: /// @brief MemoryAccess implementation for GPIO stub
219: class MemoryAccessStubGPIO : public IMemoryAccess
220: {
221: private:
222:     /// @brief Saved GPIO register values
223:     GPIORegisters m_registers;
224: 
225: public:
226:     MemoryAccessStubGPIO();
227:     uint8 Read8(regaddr address) override;
228:     void Write8(regaddr address, uint8 data) override;
229: 
230:     uint16 Read16(regaddr address) override;
231:     void Write16(regaddr address, uint16 data) override;
232: 
233:     uint32 Read32(regaddr address) override;
234:     void Write32(regaddr address, uint32 data) override;
235: 
236: private:
237:     uint32 GetRegisterOffset(regaddr address);
238: };
239: 
240: } // namespace baremetal
```

- Line 50-219: We declare a struct `GPIORegisters` to hold the values of the different GPIO registers.
  - Line 53-153: We declare all the different register fields.
For Raspberry Pi 3 and 4 there are some specific registers, for Raspberry Pi none of these are supported yet.
Note that there are also some `Reserved_` registers which are needed for correct addressing, but are not used.
Due to alignment requirements you will see multiple reserved fields after another. Combining these will result in a Data Abort exception.
You'll notice that I added an offset number to each field, and that whenever arrays are used, they start at an 8 byte boundary.
This is due to the alignment requirement
  - Line 155-215: We define the constructor for `GPIORegisters` to initialize to the default (power-on) values
- Line 218-238: We declare the class `MemoryAccessStubGPIO` which is the GPIO MemoryAccess stub, and implements the `IMemoryAccess` interface
  - Line 222-223: We declare a member variable `m_registers` that hold the GPIO register values
  - Line 237: We declare a method `GetRegisterOffset()` to convert a GPIO register address to a GPIO register offset

### MemoryAccessStubGPIO.cpp {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_MEMORYACCESSSTUBGPIOCPP}

We will implement the stub.

Create the file `code/libraries/baremetal/src/stubs/MemoryAccessStubGPIO.cpp`

```cpp
File: code/libraries/baremetal/src/stubs/MemoryAccessStubGPIO.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessStubGPIO.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessStubGPIO
9: //
10: // Description : GPIO register memory access stub implementation
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
40: #include "baremetal/stubs/MemoryAccessStubGPIO.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Format.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/String.h"
47: 
48: /// @file
49: /// MemoryAccessStubGPIO
50: 
51: /// @brief Define log name
52: LOG_MODULE("MemoryAccessStubGPIO");
53: 
54: using namespace baremetal;
55: 
56: /// @brief GPIO base address
57: static uintptr GPIOBaseAddress{RPI_GPIO_BASE};
58: /// @brief Mask used to check whether an address is in the GPIO register range
59: static uintptr GPIOBaseAddressMask{0xFFFFFFFFFFFFFF00};
60: 
61: /// <summary>
62: /// MemoryAccessStubGPIO constructor
63: /// </summary>
64: MemoryAccessStubGPIO::MemoryAccessStubGPIO()
65:     : m_registers{}
66: {
67: }
68: 
69: /// <summary>
70: /// Read a 8 bit value from register at address
71: /// </summary>
72: /// <param name="address">Address of register</param>
73: /// <returns>8 bit register value</returns>
74: uint8 MemoryAccessStubGPIO::Read8(regaddr address)
75: {
76:     LOG_PANIC("Call to Read8 should not happen");
77:     return {};
78: }
79: 
80: /// <summary>
81: /// Write a 8 bit value to register at address
82: /// </summary>
83: /// <param name="address">Address of register</param>
84: /// <param name="data">Data to write</param>
85: void MemoryAccessStubGPIO::Write8(regaddr address, uint8 data)
86: {
87:     LOG_PANIC("Call to Write8 should not happen");
88: }
89: 
90: /// <summary>
91: /// Read a 16 bit value from register at address
92: /// </summary>
93: /// <param name="address">Address of register</param>
94: /// <returns>16 bit register value</returns>
95: uint16 MemoryAccessStubGPIO::Read16(regaddr address)
96: {
97:     LOG_PANIC("Call to Read16 should not happen");
98:     return {};
99: }
100: 
101: /// <summary>
102: /// Write a 16 bit value to register at address
103: /// </summary>
104: /// <param name="address">Address of register</param>
105: /// <param name="data">Data to write</param>
106: void MemoryAccessStubGPIO::Write16(regaddr address, uint16 data)
107: {
108:     LOG_PANIC("Call to Write16 should not happen");
109: }
110: 
111: /// <summary>
112: /// Convert pin mode to string
113: /// </summary>
114: /// <param name="mode">Pin mode</param>
115: /// <returns>String representing pin mode</returns>
116: static String PinModeToString(uint32 mode)
117: {
118:     String result{};
119:     switch (mode & 0x07)
120:     {
121:     case 0:
122:         result = "Input";
123:         break;
124:     case 1:
125:         result = "Output";
126:         break;
127:     case 2:
128:         result = "Alt5";
129:         break;
130:     case 3:
131:         result = "Alt4";
132:         break;
133:     case 4:
134:         result = "Alt0";
135:         break;
136:     case 5:
137:         result = "Alt1";
138:         break;
139:     case 6:
140:         result = "Alt2";
141:         break;
142:     case 7:
143:         result = "Alt3";
144:         break;
145:     }
146:     return result;
147: }
148: 
149: /// <summary>
150: /// Convert pull up/down mode to string
151: /// </summary>
152: /// <param name="mode">Pull up/down mode</param>
153: /// <returns>String representing pull up/down mode</returns>
154: static String PullUpDownModeToString(uint32 mode)
155: {
156:     String result{};
157:     switch (mode & 0x03)
158:     {
159:     case 0:
160:         result = "None";
161:         break;
162:     case 1:
163: #if BAREMETAL_RPI_TARGET == 3
164:         result = "PullDown";
165: #else
166:         result = "PullUp";
167: #endif
168:         break;
169:     case 2:
170: #if BAREMETAL_RPI_TARGET == 3
171:         result = "PullUp";
172: #else
173:         result = "PullDown";
174: #endif
175:         break;
176:     case 3:
177:         result = "Reserved";
178:         break;
179:     }
180:     return result;
181: }
182: 
183: /// <summary>
184: /// Read a 32 bit value from register at address
185: /// </summary>
186: /// <param name="address">Address of register</param>
187: /// <returns>32 bit register value</returns>
188: uint32 MemoryAccessStubGPIO::Read32(regaddr address)
189: {
190:     uintptr offset = GetRegisterOffset(address);
191:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
192:     //    TRACE_DEBUG("GPIO read register %016x = %08x", offset, *registerField);
193:     switch (offset)
194:     {
195:     case RPI_GPIO_GPFSEL0_OFFSET:
196:     case RPI_GPIO_GPFSEL1_OFFSET:
197:     case RPI_GPIO_GPFSEL2_OFFSET:
198:     case RPI_GPIO_GPFSEL3_OFFSET:
199:     case RPI_GPIO_GPFSEL4_OFFSET:
200:     case RPI_GPIO_GPFSEL5_OFFSET:
201:         {
202:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
203:             String line{"GPIO Read Pin Mode "};
204:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
205:             {
206:                 int shift = pinIndex * 3;
207:                 uint8 pin = pinBase + pinIndex;
208:                 uint8 pinMode = (*registerField >> shift) & 0x00000007;
209:                 line += Format(" - Pin %d mode %s", pin, PinModeToString(pinMode).c_str());
210:             }
211:             TRACE_DEBUG(line.c_str());
212:             break;
213:         }
214:     case RPI_GPIO_GPLEV0_OFFSET:
215:     case RPI_GPIO_GPLEV1_OFFSET:
216:         {
217:             uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET) / 4 * 32;
218:             String line{"GPIO Read Pin Level "};
219:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
220:             {
221:                 int shift = pinIndex;
222:                 uint8 pin = pinBase + pinIndex;
223:                 uint8 value = (*registerField >> shift) & 0x00000001;
224:                 if (value)
225:                     line += " - Pin %d ON ";
226:                 else
227:                     line += " - Pin %d OFF";
228:             }
229:             TRACE_DEBUG(line.c_str());
230:             break;
231:         }
232:     case RPI_GPIO_GPEDS0_OFFSET:
233:     case RPI_GPIO_GPEDS1_OFFSET:
234:         {
235:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
236:             String line{"GPIO Read Pin Event Detect Status "};
237:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
238:             {
239:                 int shift = pinIndex;
240:                 uint8 pin = pinBase + pinIndex;
241:                 uint8 value = (*registerField >> shift) & 0x00000001;
242:                 if (value)
243:                     line += " - Pin %d ON ";
244:                 else
245:                     line += " - Pin %d OFF";
246:             }
247:             TRACE_DEBUG(line.c_str());
248:             break;
249:         }
250:     case RPI_GPIO_GPREN0_OFFSET:
251:     case RPI_GPIO_GPREN1_OFFSET:
252:         {
253:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET) / 4 * 32;
254:             String line{"GPIO Read Pin Rising Edge Detect Enable "};
255:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
256:             {
257:                 int shift = pinIndex;
258:                 uint8 pin = pinBase + pinIndex;
259:                 uint8 value = (*registerField >> shift) & 0x00000001;
260:                 if (value)
261:                     line += Format(" - Pin %d ON ", pin);
262:                 else
263:                     line += Format(" - Pin %d OFF", pin);
264:             }
265:             TRACE_DEBUG(line.c_str());
266:             break;
267:         }
268:     case RPI_GPIO_GPFEN0_OFFSET:
269:     case RPI_GPIO_GPFEN1_OFFSET:
270:         {
271:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET) / 4 * 32;
272:             String line{"GPIO Read Pin Falling Edge Detect Enable "};
273:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
274:             {
275:                 int shift = pinIndex;
276:                 uint8 pin = pinBase + pinIndex;
277:                 uint8 value = (*registerField >> shift) & 0x00000001;
278:                 if (value)
279:                     line += Format(" - Pin %d ON ", pin);
280:                 else
281:                     line += Format(" - Pin %d OFF", pin);
282:             }
283:             TRACE_DEBUG(line.c_str());
284:             break;
285:         }
286:     case RPI_GPIO_GPHEN0_OFFSET:
287:     case RPI_GPIO_GPHEN1_OFFSET:
288:         {
289:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET) / 4 * 32;
290:             String line{"GPIO Read Pin High Level Detect Enable "};
291:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
292:             {
293:                 int shift = pinIndex;
294:                 uint8 pin = pinBase + pinIndex;
295:                 uint8 value = (*registerField >> shift) & 0x00000001;
296:                 if (value)
297:                     line += Format(" - Pin %d ON ", pin);
298:                 else
299:                     line += Format(" - Pin %d OFF", pin);
300:             }
301:             TRACE_DEBUG(line.c_str());
302:             break;
303:         }
304:     case RPI_GPIO_GPLEN0_OFFSET:
305:     case RPI_GPIO_GPLEN1_OFFSET:
306:         {
307:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET) / 4 * 32;
308:             String line{"GPIO Read Pin Low Level Detect Enable "};
309:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
310:             {
311:                 int shift = pinIndex;
312:                 uint8 pin = pinBase + pinIndex;
313:                 uint8 value = (*registerField >> shift) & 0x00000001;
314:                 if (value)
315:                     line += Format(" - Pin %d ON ", pin);
316:                 else
317:                     line += Format(" - Pin %d OFF", pin);
318:             }
319:             TRACE_DEBUG(line.c_str());
320:             break;
321:         }
322:     case RPI_GPIO_GPAREN0_OFFSET:
323:     case RPI_GPIO_GPAREN1_OFFSET:
324:         {
325:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET) / 4 * 32;
326:             String line{"GPIO Read Pin Async Rising Edge Detect Enable "};
327:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
328:             {
329:                 int shift = pinIndex;
330:                 uint8 pin = pinBase + pinIndex;
331:                 uint8 value = (*registerField >> shift) & 0x00000001;
332:                 if (value)
333:                     line += Format(" - Pin %d ON ", pin);
334:                 else
335:                     line += Format(" - Pin %d OFF", pin);
336:             }
337:             TRACE_DEBUG(line.c_str());
338:             break;
339:         }
340:     case RPI_GPIO_GPAFEN0_OFFSET:
341:     case RPI_GPIO_GPAFEN1_OFFSET:
342:         {
343:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET) / 4 * 32;
344:             String line{"GPIO Read Pin Async Falling Edge Detect Enable "};
345:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
346:             {
347:                 int shift = pinIndex;
348:                 uint8 pin = pinBase + pinIndex;
349:                 uint8 value = (*registerField >> shift) & 0x00000001;
350:                 if (value)
351:                     line += Format(" - Pin %d ON ", pin);
352:                 else
353:                     line += Format(" - Pin %d OFF", pin);
354:             }
355:             TRACE_DEBUG(line.c_str());
356:             break;
357:         }
358: #if BAREMETAL_RPI_TARGET == 3
359:     case RPI_GPIO_GPPUD_OFFSET:
360:         {
361:             uint8 value = *registerField & 0x00000003;
362:             TRACE_DEBUG("GPIO Read Pull Up/Down Mode %s", PullUpDownModeToString(value).c_str());
363:             break;
364:         }
365:     case RPI_GPIO_GPPUDCLK0_OFFSET:
366:     case RPI_GPIO_GPPUDCLK1_OFFSET:
367:         {
368:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
369:             String line{"GPIO Read Pin Pull Up/Down Enable Clock "};
370:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
371:             {
372:                 int shift = pinIndex;
373:                 uint8 pin = pinBase + pinIndex;
374:                 uint8 value = (*registerField >> shift) & 0x00000001;
375:                 if (value)
376:                     line += Format(" - Pin %d ON ", pin);
377:                 else
378:                     line += Format(" - Pin %d OFF", pin);
379:             }
380:             TRACE_DEBUG(line.c_str());
381:             break;
382:         }
383: #elif BAREMETAL_RPI_TARGET == 4
384:     case RPI_GPIO_GPPINMUXSD_OFFSET:
385:         {
386:             uint32 value = *registerField;
387:             TRACE_DEBUG("GPIO Read Pin Mux Mode %x", value);
388:             break;
389:         }
390:     case RPI_GPIO_GPPUPPDN0_OFFSET:
391:     case RPI_GPIO_GPPUPPDN1_OFFSET:
392:     case RPI_GPIO_GPPUPPDN2_OFFSET:
393:     case RPI_GPIO_GPPUPPDN3_OFFSET:
394:         {
395:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
396:             String line{"GPIO Read Pin Pull Up/Down Mode "};
397:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
398:             {
399:                 int shift = pinIndex * 2;
400:                 uint8 pin = pinBase + pinIndex;
401:                 uint8 value = (*registerField >> shift) & 0x00000003;
402:                 line += Format(" - Pin %d Pull up/down mode %s", pin, PullUpDownModeToString(value).c_str());
403:             }
404:             TRACE_DEBUG(line.c_str());
405:             break;
406:         }
407: #endif
408:     default:
409:         LOG_ERROR("Invalid register access for reading: offset %d", offset);
410:         break;
411:     }
412:     return *registerField;
413: }
414: 
415: /// <summary>
416: /// Write a 32 bit value to register at address
417: /// </summary>
418: /// <param name="address">Address of register</param>
419: /// <param name="data">Data to write</param>
420: void MemoryAccessStubGPIO::Write32(regaddr address, uint32 data)
421: {
422:     uintptr offset = GetRegisterOffset(address);
423:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
424:     //    TRACE_DEBUG("GPIO write register %016x = %08x", offset, data);
425:     switch (offset)
426:     {
427:     case RPI_GPIO_GPFSEL0_OFFSET:
428:     case RPI_GPIO_GPFSEL1_OFFSET:
429:     case RPI_GPIO_GPFSEL2_OFFSET:
430:     case RPI_GPIO_GPFSEL3_OFFSET:
431:     case RPI_GPIO_GPFSEL4_OFFSET:
432:     case RPI_GPIO_GPFSEL5_OFFSET:
433:         {
434:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
435:             uint32 diff = data ^ *registerField;
436:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
437:             {
438:                 int shift = pinIndex * 3;
439:                 if (((diff >> shift) & 0x00000007) != 0)
440:                 {
441:                     uint8 pin = pinBase + pinIndex;
442:                     uint8 pinMode = (data >> shift) & 0x00000007;
443:                     String modeName = PinModeToString(pinMode);
444:                     TRACE_DEBUG("GPIO Set Pin %d Mode %s", pin, modeName.c_str());
445:                 }
446:             }
447:             break;
448:         }
449:     case RPI_GPIO_GPSET0_OFFSET:
450:     case RPI_GPIO_GPSET1_OFFSET:
451:         {
452:             uint8 pinBase = (offset - RPI_GPIO_GPSET0_OFFSET) / 4 * 32;
453:             uint32 diff = data ^ *registerField;
454:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
455:             {
456:                 int shift = pinIndex;
457:                 if (((diff >> shift) & 0x00000001) != 0)
458:                 {
459:                     uint8 pin = pinBase + pinIndex;
460:                     uint8 value = (data >> shift) & 0x00000001;
461:                     if (value != 0)
462:                         TRACE_DEBUG("GPIO Set Pin %d ON", pin);
463:                 }
464:             }
465:             break;
466:         }
467:     case RPI_GPIO_GPCLR0_OFFSET:
468:     case RPI_GPIO_GPCLR1_OFFSET:
469:         {
470:             uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET) / 4 * 32;
471:             uint32 diff = data ^ *registerField;
472:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
473:             {
474:                 int shift = pinIndex;
475:                 if (((diff >> shift) & 0x00000001) != 0)
476:                 {
477:                     uint8 pin = pinBase + pinIndex;
478:                     uint8 value = (data >> shift) & 0x00000001;
479:                     if (value != 0)
480:                         TRACE_DEBUG("GPIO Set Pin %d OFF", pin);
481:                 }
482:             }
483:             break;
484:         }
485:     case RPI_GPIO_GPEDS0_OFFSET:
486:     case RPI_GPIO_GPEDS1_OFFSET:
487:         {
488:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
489:             uint32 diff = data ^ *registerField;
490:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
491:             {
492:                 int shift = pinIndex;
493:                 if (((diff >> shift) & 0x00000001) != 0)
494:                 {
495:                     uint8 pin = pinBase + pinIndex;
496:                     uint8 value = (data >> shift) & 0x00000001;
497:                     if (value != 0)
498:                         TRACE_DEBUG("GPIO Clear Pin %d Event Status", pin);
499:                 }
500:             }
501:             break;
502:         }
503:     case RPI_GPIO_GPREN0_OFFSET:
504:     case RPI_GPIO_GPREN1_OFFSET:
505:         {
506:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET) / 4 * 32;
507:             uint32 diff = data ^ *registerField;
508:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
509:             {
510:                 int shift = pinIndex;
511:                 if (((diff >> shift) & 0x00000001) != 0)
512:                 {
513:                     uint8 pin = pinBase + pinIndex;
514:                     uint8 value = (data >> shift) & 0x00000001;
515:                     if (value != 0)
516:                         TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect ON", pin);
517:                     else
518:                         TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect OFF", pin);
519:                 }
520:             }
521:             break;
522:         }
523:     case RPI_GPIO_GPFEN0_OFFSET:
524:     case RPI_GPIO_GPFEN1_OFFSET:
525:         {
526:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET) / 4 * 32;
527:             uint32 diff = data ^ *registerField;
528:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
529:             {
530:                 int shift = pinIndex;
531:                 if (((diff >> shift) & 0x00000001) != 0)
532:                 {
533:                     uint8 pin = pinBase + pinIndex;
534:                     uint8 value = (data >> shift) & 0x00000001;
535:                     if (value != 0)
536:                         TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect ON", pin);
537:                     else
538:                         TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect OFF", pin);
539:                 }
540:             }
541:             break;
542:         }
543:     case RPI_GPIO_GPHEN0_OFFSET:
544:     case RPI_GPIO_GPHEN1_OFFSET:
545:         {
546:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET) / 4 * 32;
547:             uint32 diff = data ^ *registerField;
548:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
549:             {
550:                 int shift = pinIndex;
551:                 if (((diff >> shift) & 0x00000001) != 0)
552:                 {
553:                     uint8 pin = pinBase + pinIndex;
554:                     uint8 value = (data >> shift) & 0x00000001;
555:                     if (value != 0)
556:                         TRACE_DEBUG("GPIO Set Pin %d High Level Detect ON", pin);
557:                     else
558:                         TRACE_DEBUG("GPIO Set Pin %d High Level Detect OFF", pin);
559:                 }
560:             }
561:             break;
562:         }
563:     case RPI_GPIO_GPLEN0_OFFSET:
564:     case RPI_GPIO_GPLEN1_OFFSET:
565:         {
566:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET) / 4 * 32;
567:             uint32 diff = data ^ *registerField;
568:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
569:             {
570:                 int shift = pinIndex;
571:                 if (((diff >> shift) & 0x00000001) != 0)
572:                 {
573:                     uint8 pin = pinBase + pinIndex;
574:                     uint8 value = (data >> shift) & 0x00000001;
575:                     if (value != 0)
576:                         TRACE_DEBUG("GPIO Set Pin %d Low Level Detect ON", pin);
577:                     else
578:                         TRACE_DEBUG("GPIO Set Pin %d Low Level Detect OFF", pin);
579:                 }
580:             }
581:             break;
582:         }
583:     case RPI_GPIO_GPAREN0_OFFSET:
584:     case RPI_GPIO_GPAREN1_OFFSET:
585:         {
586:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET) / 4 * 32;
587:             uint32 diff = data ^ *registerField;
588:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
589:             {
590:                 int shift = pinIndex;
591:                 if (((diff >> shift) & 0x00000001) != 0)
592:                 {
593:                     uint8 pin = pinBase + pinIndex;
594:                     uint8 value = (data >> shift) & 0x00000001;
595:                     if (value != 0)
596:                         TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect ON", pin);
597:                     else
598:                         TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect OFF", pin);
599:                 }
600:             }
601:             break;
602:         }
603:     case RPI_GPIO_GPAFEN0_OFFSET:
604:     case RPI_GPIO_GPAFEN1_OFFSET:
605:         {
606:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET) / 4 * 32;
607:             uint32 diff = data ^ *registerField;
608:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
609:             {
610:                 int shift = pinIndex;
611:                 if (((diff >> shift) & 0x00000001) != 0)
612:                 {
613:                     uint8 pin = pinBase + pinIndex;
614:                     uint8 value = (data >> shift) & 0x00000001;
615:                     if (value != 0)
616:                         TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect ON", pin);
617:                     else
618:                         TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect OFF", pin);
619:                 }
620:             }
621:             break;
622:         }
623: #if BAREMETAL_RPI_TARGET == 3
624:     case RPI_GPIO_GPPUD_OFFSET:
625:         {
626:             uint32 diff = data ^ *registerField;
627:             if ((diff & 0x00000003) != 0)
628:             {
629:                 uint8 value = data & 0x00000003;
630:                 String modeName = PullUpDownModeToString(value);
631:                 TRACE_DEBUG("GPIO Set Pin Pull Up/Down Mode %s", modeName.c_str());
632:             }
633:             break;
634:         }
635:     case RPI_GPIO_GPPUDCLK0_OFFSET:
636:     case RPI_GPIO_GPPUDCLK1_OFFSET:
637:         {
638:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
639:             uint32 diff = data ^ *registerField;
640:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
641:             {
642:                 int shift = pinIndex;
643:                 if (((diff >> shift) & 0x00000001) != 0)
644:                 {
645:                     uint8 pin = pinBase + pinIndex;
646:                     uint8 value = (data >> shift) & 0x00000001;
647:                     if (value != 0)
648:                         TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock ON", pin);
649:                     else
650:                         TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock OFF", pin);
651:                 }
652:             }
653:             break;
654:         }
655: #elif BAREMETAL_RPI_TARGET == 4
656:     case RPI_GPIO_GPPINMUXSD_OFFSET:
657:         {
658:             uint32 value = *registerField;
659:             TRACE_DEBUG("GPIO Set Pin Mux Mode %x", value);
660:             break;
661:         }
662:     case RPI_GPIO_GPPUPPDN0_OFFSET:
663:     case RPI_GPIO_GPPUPPDN1_OFFSET:
664:     case RPI_GPIO_GPPUPPDN2_OFFSET:
665:     case RPI_GPIO_GPPUPPDN3_OFFSET:
666:         {
667:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
668:             uint32 diff = data ^ *registerField;
669:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
670:             {
671:                 int shift = pinIndex * 2;
672:                 if (((diff >> shift) & 0x00000003) != 0)
673:                 {
674:                     uint8 pin = pinBase + pinIndex;
675:                     uint8 value = (data >> shift) & 0x00000003;
676:                     String modeName = PullUpDownModeToString(value);
677:                     TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Mode %s", pin, modeName.c_str());
678:                 }
679:             }
680:             break;
681:         }
682: #endif
683:     default:
684:         LOG_ERROR("Invalid GPIO register access for writing: offset %d", offset);
685:         break;
686:     }
687:     *registerField = data;
688: }
689: 
690: /// <summary>
691: /// Determine register address offset relative to GPIO base address
692: ///
693: /// If the address is not in the correct range, an assert is fired
694: /// </summary>
695: /// <param name="address">Address to check</param>
696: /// <returns>Offset relative to GPIO base address</returns>
697: uint32 MemoryAccessStubGPIO::GetRegisterOffset(regaddr address)
698: {
699:     assert((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) == GPIOBaseAddress);
700:     return reinterpret_cast<uintptr>(address) - GPIOBaseAddress;
701: }
```

- Line 57: We define a variable `GPIOBaseAddress` to hold the base address of the GPIO registers
- Line 59: We define a variable `GPIOBaseAddressMask` to check whether an address is in the GPIO register range
- Line 61-67: We implement the constructor
- Line 69-78: We implement the method `Read8()` as a panic, as we will always do 32 bit read / write access
- Line 80-88: We implement the method `Write8()` as a panic, as we will always do 32 bit read / write access
- Line 90-99: We implement the method `Read16()` as a panic, as we will always do 32 bit read / write access
- Line 101-109: We implement the method `Write16()` as a panic, as we will always do 32 bit read / write access
- Line 111-147: We define a function `PinModeToString()` to convert a GPIO pin mode to a string
This is used for debug tracing
- Line 149-181: We define a function `PullUpDownModeToString()` to convert a GPIO pin pull up/down mode to a string.
This is used for debug tracing
- Line 183-413: We implement the method `Read32()`
  - Line 190: We calculate the relative register offset from the address
  - Line 191: We determine the address of the field to read in the saved GPIO registers
  - Line 195-213: We handle the GPIO Function Select Registers, to trace the access
  - Line 214-231: We handle the GPIO Level Registers, to trace the access
  - Line 232-249: We handle the GPIO Event Detect Status Registers, to trace the access
  - Line 250-267: We handle the GPIO Rising Edge Detect Enable Registers, to trace the access
  - Line 268-285: We handle the GPIO Falling Edge Detect Enable Registers, to trace the access
  - Line 286-303: We handle the GPIO High Level Detect Enable Registers, to trace the access
  - Line 304-321: We handle the GPIO Low Level Detect Enable Registers, to trace the access
  - Line 322-339: We handle the GPIO Asynchronous Rising Edge Detect Enable Registers, to trace the access
  - Line 340-357: We handle the GPIO Asynchronous Falling Edge Detect Enable Registers, to trace the access
  - Line 359-364: We handle the GPIO Pull up/down Mode Register for Raspberry Pi 3, to trace the access
  - Line 365-282: We handle the GPIO Pull up/down Clock Registers for Raspberry Pi 3, to trace the access
  - Line 384-389: We handle the GPIO Pull up/down Pin Mux Register for Raspberry Pi 4, to trace the access
  - Line 390-406: We handle the GPIO Pull up/down Mode Registers for Raspberry Pi 4, to trace the access
- Line 415-688: We implement the method `Write32()`
  - Line 422: We calculate the relative register offset from the address
  - Line 423: We determine the address of the field to write in the save register values
  - Line 427-448: We handle the GPIO Function Select Registers, to trace the access
  - Line 449-466: We handle the GPIO Set Registers, to trace the access
  - Line 467-484: We handle the GPIO Clear Registers, to trace the access
  - Line 485-502: We handle the GPIO Event Detect Status Registers, to trace the access
  - Line 503-522: We handle the GPIO Rising Edge Detect Enable Registers, to trace the access
  - Line 523-542: We handle the GPIO Falling Edge Detect Enable Registers, to trace the access
  - Line 543-562: We handle the GPIO High Level Detect Enable Registers, to trace the access
  - Line 563-582: We handle the GPIO Low Level Detect Enable Registers, to trace the access
  - Line 583-602: We handle the GPIO Asynchronous Rising Edge Detect Enable Registers, to trace the access
  - Line 603-622: We handle the GPIO Asynchronous Falling Edge Detect Enable Registers, to trace the access
  - Line 624-634: We handle the GPIO Pull up/down Mode Register for Raspberry Pi 3, to trace the access
  - Line 635-654: We handle the GPIO Pull up/down Clock Registers for Raspberry Pi 3, to trace the access
  - Line 656-661: We handle the GPIO Pull up/down Pin Mux Register for Raspberry Pi 4, to trace the access
  - Line 662-681: We handle the GPIO Pull up/down Mode Registers for Raspberry Pi 4, to trace the access
  - Line 687: We write the value to the saved GPIO registers
- Line 690-701: We implement the method `GetRegisterOffset()`

### Logger.h {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_LOGGERH}

Even though we protected against early logging and tracing, we want to make sure this happens everywhere, so whenever we use the `LOG` or `TRACE` macros, we want to only output when a logger was actually created.

So we'll change the singleton instance to a pointer, and check whether this pointer is initialized.
Also, whenever we use the macros, we check whether the static `HaveLogger()` method returns true.
For this, we'll introduce static `Log()`, `LogNoAlloc()`, `Trace()` and `TraceNoAlloc()` methods.

Update the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
70: /// <summary>
71: /// Logger class
72: /// </summary>
73: class Logger
74: {
75:     /// <summary>
76:     /// Construct the singleton Logger instance if needed, and return a reference to the instance. This is a friend function of class Logger
77:     /// </summary>
78:     /// <returns>Reference to the singleton logger instance</returns>
79:     friend Logger& GetLogger();
80: 
81: private:
82:     /// @brief True if class is already initialized
83:     bool m_isInitialized;
84:     /// @brief Pointer to timer instance
85:     Timer* m_timer;
86:     /// @brief Currently set logging severity level
87:     LogSeverity m_level;
88:     /// @brief Singleton console instance
89:     static Console s_console;
90:     /// @brief Singleton logger instance
91:     static Logger* s_logger;
92: 
93:     explicit Logger(LogSeverity logLevel, Timer* timer = nullptr);
94: 
95: public:
96:     static bool HaveLogger();
97: 
98:     bool Initialize();
99:     void SetLogLevel(LogSeverity logLevel);
100: 
101:     void Log(const char* from, int line, LogSeverity severity, const char* message, ...);
102:     void LogV(const char* from, int line, LogSeverity severity, const char* message, va_list args);
103: 
104:     void LogNoAlloc(const char* from, int line, LogSeverity severity, const char* message, ...);
105:     void LogNoAllocV(const char* from, int line, LogSeverity severity, const char* message, va_list args);
106: 
107:     void Trace(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
108:     void TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
109: 
110:     void TraceNoAlloc(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
111:     void TraceNoAllocV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
112: 
113:     static void LogEntry(const char* from, int line, LogSeverity severity, const char* message, ...);
114:     static void LogEntryNoAlloc(const char* from, int line, LogSeverity severity, const char* message, ...);
115:     static void TraceEntry(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
116:     static void TraceEntryNoAlloc(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
117: };
118: 
119: Logger& GetLogger();
120: 
121: /// @brief Define the static variable From to the specified name, to support printing a different file specification in LOG_* macros
122: #define LOG_MODULE(name)                  static const char From[] = name
123: 
124: /// @brief Log a panic message
125: #define LOG_PANIC(...)                    Logger::LogEntry(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
126: /// @brief Log an error message
127: #define LOG_ERROR(...)                    Logger::LogEntry(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
128: /// @brief Log a warning message
129: #define LOG_WARNING(...)                  Logger::LogEntry(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
130: /// @brief Log a info message
131: #define LOG_INFO(...)                     Logger::LogEntry(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
132: /// @brief Log a debug message
133: #define LOG_DEBUG(...)                    Logger::LogEntry(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
134: 
135: /// @brief Log a message with specified severity and message string
136: #define LOG(severity, message)            Logger::LogEntry(From, __LINE__, severity, message)
137: 
138: /// @brief Log a panic message
139: #define LOG_NO_ALLOC_PANIC(...)           Logger::LogEntryNoAlloc(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
140: /// @brief Log an error message
141: #define LOG_NO_ALLOC_ERROR(...)           Logger::LogEntryNoAlloc(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
142: /// @brief Log a warning message
143: #define LOG_NO_ALLOC_WARNING(...)         Logger::LogEntryNoAlloc(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
144: /// @brief Log a info message
145: #define LOG_NO_ALLOC_INFO(...)            Logger::LogEntryNoAlloc(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
146: /// @brief Log a debug message
147: #define LOG_NO_ALLOC_DEBUG(...)           Logger::LogEntryNoAlloc(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
148: 
149: /// @brief Log a message with specified severity and message string
150: #define LOG_NO_ALLOC(severity, message)   Logger::LogEntryNoAlloc(From, __LINE__, severity, message)
151: 
152: /// @brief Log a warning message
153: #define TRACE_WARNING(...)                Logger::TraceEntry(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
154: /// @brief Log a info message
155: #define TRACE_INFO(...)                   Logger::TraceEntry(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
156: /// @brief Log a debug message
157: #define TRACE_DEBUG(...)                  Logger::TraceEntry(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)
158: 
159: /// @brief Log a message with specified severity and message string
160: #define TRACE(severity, message)          Logger::TraceEntry(__FILE_NAME__, __LINE__, __func__, severity, message)
161: 
162: /// @brief Log a warning message
163: #define TRACE_NO_ALLOC_WARNING(...)       Logger::TraceEntryNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
164: /// @brief Log a info message
165: #define TRACE_NO_ALLOC_INFO(...)          Logger::TraceEntryNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
166: /// @brief Log a debug message
167: #define TRACE_NO_ALLOC_DEBUG(...)         Logger::TraceEntryNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)
168: 
169: /// @brief Log a message with specified severity and message string
170: #define TRACE_NO_ALLOC(severity, message) Logger::TraceEntryNoAlloc(__FILE_NAME__, __LINE__, __func__, severity, message)
```

- Line 90-91: We declare a static class variable `s_logger` to point to the singleton instance
- Line 113-116: We declare the new static methods
- Line 124-136: We now call the static method `LogEntry()` instead of retrieving the singleton instance and calling `Log()` on it
- Line 138-150: We now call the static method `LogEntryNoAlloc()` instead of retrieving the singleton instance and calling `LogNoAlloc()` on it
- Line 152-160: We now call the static method `TraceEntry()` instead of retrieving the singleton instance and calling `Trace()` on it
- Line 162-170: We now call the static method `TraceEntryNoAlloc()` instead of retrieving the singleton instance and calling `TraceNoAlloc()` on it

### Logger.cpp {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_LOGGERCPP}

Let's update the code to reflect the changes in the header.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
59: Console Logger::s_console(nullptr);
60: Logger* Logger::s_logger{};
61: 
62: /// <summary>
63: /// Construct a logger
64: /// </summary>
65: /// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
66: /// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
67: Logger::Logger(LogSeverity logLevel, Timer* timer /*= nullptr*/)
68:     : m_isInitialized{}
69:     , m_timer{timer}
70:     , m_level{logLevel}
71: {
72: }
73: 
74: /// <summary>
75: /// Check whether the singleton logger was instantiated and initialized
76: /// </summary>
77: /// <returns>Returns true if the singleton logger instance is created and initialized, false otherwise</returns>
78: bool Logger::HaveLogger()
79: {
80:     return (s_logger != nullptr) && (s_logger->m_isInitialized);
81: }
...
491: /// <summary>
492: /// Write a string with variable arguments to the logger. Static entry point for Log() method
493: /// </summary>
494: /// <param name="source">Source name or file name</param>
495: /// <param name="line">Source line number</param>
496: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
497: /// <param name="message">Formatted message string, with variable arguments</param>
498: void Logger::LogEntry(const char *from, int line, LogSeverity severity, const char *message, ...)
499: {
500:     if (HaveLogger())
501:     {
502:         va_list args;
503:         va_start(args, message);
504:         GetLogger().LogV(from, line, severity, message, args);
505:         va_end(args);
506:     }
507: }
508: 
509: /// <summary>
510: /// Write a string with variable arguments to the logger, not using memory allocation. Static entry point for LogNoAlloc() method
511: /// </summary>
512: /// <param name="source">Source name or file name</param>
513: /// <param name="line">Source line number</param>
514: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
515: /// <param name="message">Formatted message string, with variable arguments</param>
516: void Logger::LogEntryNoAlloc(const char *from, int line, LogSeverity severity, const char *message, ...)
517: {
518:     if (HaveLogger())
519:     {
520:         va_list args;
521:         va_start(args, message);
522:         GetLogger().LogNoAllocV(from, line, severity, message, args);
523:         va_end(args);
524:     }
525: }
526: 
527: /// <summary>
528: /// Write a trace string with variable arguments to the logger. Static entry point for Trace() method
529: /// </summary>
530: /// <param name="filename">File name</param>
531: /// <param name="line">Source line number</param>
532: /// <param name="function">Function name</param>
533: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
534: /// <param name="message">Formatted message string, with variable arguments</param>
535: void Logger::TraceEntry(const char *filename, int line, const char *function, LogSeverity severity, const char *message, ...)
536: {
537:     if (HaveLogger())
538:     {
539:         va_list args;
540:         va_start(args, message);
541:         GetLogger().TraceV(filename, line, function, severity, message, args);
542:         va_end(args);
543:     }
544: }
545: 
546: /// <summary>
547: /// Write a trace string with variable arguments to the logger, not using memory allocation. Static entry point for TraceNoAlloc() method
548: /// </summary>
549: /// <param name="filename">File name</param>
550: /// <param name="line">Source line number</param>
551: /// <param name="function">Function name</param>
552: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
553: /// <param name="message">Formatted message string, with variable arguments</param>
554: void Logger::TraceEntryNoAlloc(const char *filename, int line, const char *function, LogSeverity severity, const char *message, ...)
555: {
556:     if (HaveLogger())
557:     {
558:         va_list args;
559:         va_start(args, message);
560:         GetLogger().TraceNoAllocV(filename, line, function, severity, message, args);
561:         va_end(args);
562:     }
563: }
564: 
565: /// <summary>
566: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
567: /// </summary>
568: /// <returns>Reference to the singleton logger instance</returns>
569: Logger& baremetal::GetLogger()
570: {
571:     if (Logger::s_logger == nullptr)
572:     {
573:         Logger::s_logger = new Logger(LogSeverity::Info, &GetTimer());
574:         Logger::s_logger->Initialize();
575: 
576:     }
577:     return *Logger::s_logger;
578: }
```

- Line 60: We define and initialize the class variable `s_logger`
- Line 80: We now use `s_logger` and check that it is not null, and if not null that the class is initialized
- Line 491-507: We implement `LogEntry()`, which first checks whether `HaveLogger()` returns true, then builds the variable argument list and calls `LogV()`
- Line 509-525: We implement `LogEntryNoAlloc)`, which first checks whether `HaveLogger()` returns true, then builds the variable argument list and calls `LogNoAllocV()`
- Line 527-544: We implement `TraceEntry()`, which first checks whether `HaveLogger()` returns true, then builds the variable argument list and calls `TraceV()`
- Line 546-563: We implement `TraceEntryNoAlloc()`, which first checks whether `HaveLogger()` returns true, then builds the variable argument list and calls `TraceNoAllocV()`
- Line 565-578: We update the implementation of `GetLogger()` to create a new object if the pointer is still null

### Update application code : Use GPIO stub {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_UPDATE_APPLICATION_CODE__USE_GPIO_STUB}

We will first use the GPIO stub to verify it works correctly.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Logger.h"
2: #include "baremetal/PhysicalGPIOPin.h"
3: #include "baremetal/System.h"
4: #include "baremetal/stubs/MemoryAccessStubGPIO.h"
5:
6: LOG_MODULE("main");
7:
8: using namespace baremetal;
9:
10: int main()
11: {
12:     auto& console = GetConsole();
13:     GetLogger().SetLogLevel(LogSeverity::Debug);
14:
15:     MemoryAccessStubGPIO fakeMemoryAccess;
16:
17:     uint8 CLKPinNumber{ 11 };
18:     uint8 DTPinNumber{ 9 };
19:     uint8 SWPinNumber{ 10 };
20:     PhysicalGPIOPin clkPin(CLKPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);
21:     PhysicalGPIOPin dtPin(DTPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);
22:     PhysicalGPIOPin swPin(SWPinNumber, GPIOMode::InputPullUp, fakeMemoryAccess);
23:
24:     LOG_INFO("Rebooting");
25: 
26:     return static_cast<int>(ReturnCode::ExitReboot);
27: }
```

### Configuring, building and debugging : Use GPIO stub {#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING__USE_GPIO_STUB}

We can now configure and build our code, and test.
Notice that the setup of the GPIO pins is logged by the stub.

```text
1: Setting up UART0
2: Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
3: Info   0.00:00:00.050 Starting up (System:213)
4: Debug  0.00:00:00.070 GPIO Set Pin Pull Up/Down Mode PullUp (MemoryAccessStubGPIO.cpp:631)
5: Debug  0.00:00:00.090 GPIO Set Pin 11 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO.cpp:648)
6: Debug  0.00:00:00.120 GPIO Set Pin 11 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO.cpp:650)
7: Debug  0.00:00:00.150 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Input - Pin 12 mode Input - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO.cpp:211)
8: Debug  0.00:00:00.190 GPIO Set Pin 9 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO.cpp:648)
9: Debug  0.00:00:00.210 GPIO Set Pin 9 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO.cpp:650)
10: Debug  0.00:00:00.250 GPIO Read Pin Mode  - Pin 0 mode Input - Pin 1 mode Input - Pin 2 mode Input - Pin 3 mode Input - Pin 4 mode Input - Pin 5 mode Input - Pin 6 mode Input - Pin 7 mode Input - Pin 8 mode Input - Pin 9 mode Input (MemoryAccessStubGPIO.cpp:211)
11: Debug  0.00:00:00.280 GPIO Set Pin 10 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO.cpp:648)
12: Debug  0.00:00:00.310 GPIO Set Pin 10 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO.cpp:650)
13: Debug  0.00:00:00.340 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Input - Pin 12 mode Input - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO.cpp:211)
14: Info   0.00:00:00.380 Rebooting (main:24)
15: Info   0.00:00:00.400 Reboot (System:144)
```

- Line 5-7 We set pin 11 to Pull-up mode, and set the pin to input
- Line 8-10 We set pin 9 to Pull-up mode, and set the pin to input
- Line 11-13 We set pin 10 to Pull-up mode, and set the pin to input

## Setting up GPIO and reading data - Step 2 {#TUTORIAL_21_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_2}

Now let's start using the actual GPIO pins.
We'll start by reading the GPIO values for the signals we just introduced.

### Update application code {#TUTORIAL_21_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_2_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Logger.h"
2: #include "baremetal/PhysicalGPIOPin.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5:
6: LOG_MODULE("main");
7:
8: using namespace baremetal;
9:
10: int main()
11: {
12:     auto& console = GetConsole();
13:     GetLogger().SetLogLevel(LogSeverity::Debug);
14:
15:     uint8 CLKPinNumber{ 11 };
16:     uint8 DTPinNumber{ 9 };
17:     uint8 SWPinNumber{ 10 };
18:     PhysicalGPIOPin pinCLK(CLKPinNumber, GPIOMode::InputPullUp);
19:     PhysicalGPIOPin pinDT(DTPinNumber, GPIOMode::InputPullUp);
20:     PhysicalGPIOPin pinSW(SWPinNumber, GPIOMode::InputPullUp);
21:
22:     auto valueCLK = pinCLK.Get();
23:     auto valueDT = pinDT.Get();
24:     auto valueSW = pinSW.Get();
25:     int waitTimeMS = 10000;
26:     LOG_DEBUG("CLK=%d, DT=%d, SW=%d", valueCLK, valueDT, valueSW);
27:     while (waitTimeMS > 0)
28:     {
29:         auto newValueCLK = pinCLK.Get();
30:         auto newValueDT = pinDT.Get();
31:         auto newValueSW = pinSW.Get();
32:         if (newValueCLK != valueCLK)
33:         {
34:             LOG_DEBUG("CLK=%d", newValueCLK);
35:             valueCLK = newValueCLK;
36:         }
37:         if (newValueDT != valueDT)
38:         {
39:             LOG_DEBUG("DT=%d", newValueDT);
40:             valueDT = newValueDT;
41:         }
42:         if (newValueSW != valueSW)
43:         {
44:             LOG_DEBUG("SW=%d", newValueSW);
45:             valueSW = newValueSW;
46:         }
47:
48:         Timer::WaitMilliSeconds(10);
49:         waitTimeMS -= 10;
50:     }
51:
52:     console.Write("Press r to reboot, h to halt\n");
53:     char ch{};
54:     while ((ch != 'r') && (ch != 'h'))
55:     {
56:         ch = console.ReadChar();
57:         console.WriteChar(ch);
58:     }
59:
60:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
61: }
```

- Line 4: We add the include file for the timer
- Line 22-24: We read the current values for each pin
- Line 25: We initialize a counter to count down time. We set it to 10 seconds
- Line 26: We print the current values for each pin
- Line 27: We loop as long as the counter is still not 0
- Line 29-31: We read the current values for each pin
- Line 32-36: If the value of the CLK input changed, we print its value
- Line 37-41: If the value of the CLK input changed, we print its value
- Line 42-46: If the value of the CLK input changed, we print its value
- Line 48-49: We wait can decrement the counter

### Configuring, building and debugging {#TUTORIAL_21_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Note that we need to have the hardware installed, so we need to run the code on an actual board.

When we press the switch:

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Debug  0.00:00:00.070 CLK=1, DT=1, SW=1 (main:26)
Debug  0.00:00:01.860 SW=0 (main:44)
Debug  0.00:00:02.050 SW=1 (main:44)
Debug  0.00:00:02.550 SW=0 (main:44)
Debug  0.00:00:02.710 SW=1 (main:44)
Debug  0.00:00:03.300 SW=0 (main:44)
Debug  0.00:00:03.490 SW=1 (main:44)
Debug  0.00:00:04.000 SW=0 (main:44)
Debug  0.00:00:04.120 SW=1 (main:44)
Debug  0.00:00:04.330 SW=0 (main:44)
Debug  0.00:00:04.650 SW=1 (main:44)
Debug  0.00:00:05.310 SW=0 (main:44)
Debug  0.00:00:05.440 SW=1 (main:44)
Debug  0.00:00:05.610 SW=0 (main:44)
Debug  0.00:00:05.700 SW=1 (main:44)
Debug  0.00:00:05.830 SW=0 (main:44)
Debug  0.00:00:05.990 SW=1 (main:44)
Press r to reboot, h to halt
```

It is quite simple to see the values change when the switch is pressed.
Notice however, that the initial value is 1, meaning that pressing the switch actually makes the value go to 0.

When we turn the switch clockwise:

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Debug  0.00:00:00.070 CLK=1, DT=1, SW=1 (main:26)
Debug  0.00:00:02.990 CLK=0 (main:34)
Debug  0.00:00:03.020 DT=0 (main:39)
Debug  0.00:00:03.050 CLK=1 (main:34)
Debug  0.00:00:03.170 DT=1 (main:39)
Debug  0.00:00:04.510 CLK=0 (main:34)
Debug  0.00:00:04.550 DT=0 (main:39)
Debug  0.00:00:04.580 CLK=1 (main:34)
Debug  0.00:00:04.620 DT=1 (main:39)
Debug  0.00:00:05.490 CLK=0 (main:34)
Debug  0.00:00:05.590 DT=0 (main:39)
Debug  0.00:00:05.620 CLK=1 (main:34)
Debug  0.00:00:05.660 DT=1 (main:39)
Debug  0.00:00:06.930 CLK=0 (main:34)
Debug  0.00:00:06.980 DT=0 (main:39)
Debug  0.00:00:07.070 CLK=1 (main:34)
Debug  0.00:00:07.140 DT=1 (main:39)
Press r to reboot, h to halt
```

Again we see that both values are initially 1, and turning the switch clockwise results in the following pattern:

- First CLK goes low
- Then DT goes low
- Then CLK goes high
- And finally DT goes high

When we turn the switch anticlockwise:

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Debug  0.00:00:00.070 CLK=1, DT=1, SW=1 (main:26)
Debug  0.00:00:00.870 DT=0 (main:39)
Debug  0.00:00:00.900 CLK=0 (main:34)
Debug  0.00:00:00.930 DT=1 (main:39)
Debug  0.00:00:00.960 CLK=1 (main:34)
Debug  0.00:00:02.180 DT=0 (main:39)
Debug  0.00:00:02.280 CLK=0 (main:34)
Debug  0.00:00:02.320 DT=1 (main:39)
Debug  0.00:00:02.380 CLK=1 (main:34)
Debug  0.00:00:03.620 DT=0 (main:39)
Debug  0.00:00:03.670 CLK=0 (main:34)
Debug  0.00:00:03.750 DT=1 (main:39)
Debug  0.00:00:03.860 CLK=1 (main:34)
Debug  0.00:00:05.160 DT=0 (main:39)
Debug  0.00:00:05.190 CLK=0 (main:34)
Debug  0.00:00:05.220 DT=1 (main:39)
Debug  0.00:00:05.260 CLK=1 (main:34)
Press r to reboot, h to halt
```

Again we see that both values are initially 1, however turning the switch anti-clockwise results in a different pattern:

- First DT goes low
- Then CLK goes low
- Then DT goes high
- And finally CLK goes high

One thing worthwhile however happens, when we turn the switch quickly (clockwise in this case):

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Debug  0.00:00:00.070 CLK=1, DT=1, SW=1 (main:26)
Debug  0.00:00:01.150 CLK=0 (main:34)
Debug  0.00:00:01.190 DT=0 (main:39)
Debug  0.00:00:01.220 DT=1 (main:39)
Debug  0.00:00:01.260 CLK=1 (main:34)
Debug  0.00:00:01.280 DT=0 (main:39)
Debug  0.00:00:01.310 CLK=0 (main:34)
Debug  0.00:00:01.340 CLK=1 (main:34)
Debug  0.00:00:01.360 DT=1 (main:39)
Debug  0.00:00:01.390 CLK=0 (main:34)
Debug  0.00:00:01.400 DT=0 (main:39)
Debug  0.00:00:01.430 CLK=1 (main:34)
Debug  0.00:00:01.460 DT=1 (main:39)
Debug  0.00:00:03.160 CLK=0 (main:34)
Debug  0.00:00:03.180 DT=0 (main:39)
Debug  0.00:00:03.210 DT=1 (main:39)
Debug  0.00:00:03.240 CLK=1 (main:34)
Debug  0.00:00:03.260 DT=0 (main:39)
Debug  0.00:00:03.300 CLK=0 (main:34)
Debug  0.00:00:03.320 DT=1 (main:39)
Debug  0.00:00:03.370 CLK=1 (main:34)
Debug  0.00:00:03.420 CLK=0 (main:34)
Debug  0.00:00:03.460 DT=0 (main:39)
Debug  0.00:00:03.490 CLK=1 (main:34)
Debug  0.00:00:03.500 DT=1 (main:39)
Press r to reboot, h to halt
```

Here we see that the pattern is disturbed. If we want to reliably handle turning the switch, we need to this into account as well.

## Adding GPIO interrupts - Step 3 {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3}

Now that we can read GPIO data for the rotary switch, let's set up for generating interrupts for these signals.
We'd like to get an interrupt on each change, so both on rising and falling edges.

We'll first add some methods to `PhysicalGPIOPin` to enable setting up for interrupts.

### IGPIOPin.h {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3_IGPIOPINH}

Update the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IGPIOPin.h
...
76: /// <summary>
77: /// Abstraction of a GPIO pin
78: /// </summary>
79: class IGPIOPin
80: {
81: public:
82:     /// <summary>
83:     /// Default destructor needed for abstract interface
84:     /// </summary>
85:     virtual ~IGPIOPin() = default;
86: 
87:     /// <summary>
88:     /// Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
89:     /// </summary>
90:     /// <returns>Pin number</returns>
91:     virtual uint8 GetPinNumber() const = 0;
92:     /// <summary>
93:     /// Assign a GPIO pin
94:     /// </summary>
95:     /// <param name="pinNumber">Pin number</param>
96:     /// <returns>true if successful, false otherwise</returns>
97:     virtual bool AssignPin(uint8 pinNumber) = 0;
98: 
99:     /// <summary>
100:     /// Switch GPIO on
101:     /// </summary>
102:     virtual void On() = 0;
103:     /// <summary>
104:     /// Switch GPIO off
105:     /// </summary>
106:     virtual void Off() = 0;
107:     /// <summary>
108:     /// Get GPIO value
109:     /// </summary>
110:     /// <returns>GPIO value, true if on, false if off</returns>
111:     virtual bool Get() = 0;
112:     /// <summary>
113:     /// Set GPIO on (true) or off (false)
114:     /// </summary>
115:     /// <param name="on">Value to set, on (true) or off (false)</param>
116:     virtual void Set(bool on) = 0;
117:     /// <summary>
118:     /// Invert GPIO value on->off off->on
119:     /// </summary>
120:     virtual void Invert() = 0;
121:     /// <summary>
122:     /// Get GPIO event status
123:     /// </summary>
124:     /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
125:     virtual bool GetEvent() = 0;
126:     /// <summary>
127:     /// Clear GPIO event status
128:     /// </summary>
129:     virtual void ClearEvent() = 0;
130: };
...
```

- Line 125: We add an abstract method `GetEvent()` to check whether an event happened on the GPIO pin
- Line 129: We add an abstract method `ClearEvent()` to clear any event conditions on the GPIO pin

### PhysicalGPIOPin.h {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3_PHYSICALGPIOPINH}

Update the file `code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h
...
48: namespace baremetal {
49: 
50: /// @brief GPIO function
51: enum class GPIOFunction;
52: 
53: /// @brief GPIO pull mode
54: enum class GPIOPullMode;
55: 
56: #if BAREMETAL_RPI_TARGET == 3
57: /// @brief Total count of GPIO pins, numbered from 0 through 53
58: #define NUM_GPIO 54
59: #else
60: /// @brief Total count of GPIO pins, numbered from 0 through 56
61: #define NUM_GPIO 57
62: #endif
63: 
64: /// @brief Interrupt type to enable
65: enum class GPIOInterruptType
66: {
67:     /// @brief Interrupt on rising edge
68:     RisingEdge,
69:     /// @brief Interrupt on falling edge
70:     FallingEdge,
71:     /// @brief Interrupt on low level
72:     HighLevel,
73:     /// @brief Interrupt on high level
74:     LowLevel,
75:     /// @brief Interrupt on asynchronous rising edge
76:     AsyncRisingEdge,
77:     /// @brief Interrupt on asynchronous falling edge
78:     AsyncFallingEdge,
79:     /// @brief Invalid / unknown
80:     Unknown,
81: };
82: 
83: /// <summary>
84: /// Physical GPIO pin (i.e. available on GPIO header)
85: /// </summary>
86: class PhysicalGPIOPin : public IGPIOPin
87: {
88: private:
89:     /// @brief Configured GPIO pin number (0..53)
90:     uint8 m_pinNumber;
91:     /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
92:     GPIOMode m_mode;
93:     /// @brief Configured GPIO function.
94:     GPIOFunction m_function;
95:     /// @brief Configured GPIO pull mode (only for input function).
96:     GPIOPullMode m_pullMode;
97:     /// @brief Current value of the GPIO pin (true for on, false for off).
98:     bool m_value;
99:     /// @brief Memory access interface reference for accessing registers.
100:     IMemoryAccess& m_memoryAccess;
101:     /// @brief Register offset for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
102:     unsigned m_regOffset;
103:     /// @brief Register mask for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
104:     uint32 m_regMask;
105:     /// @brief GPIO interrupt types enabled
106:     bool m_interruptEnabled[static_cast<size_t>(GPIOInterruptType::Unknown)];
107: 
108: public:
109:     PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());
110: 
111:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());
112: 
113:     uint8 GetPinNumber() const override;
114:     bool AssignPin(uint8 pinNumber) override;
115: 
116:     void On() override;
117:     void Off() override;
118:     bool Get() override;
119:     void Set(bool on) override;
120:     void Invert() override;
121:     bool GetEvent() override;
122:     void ClearEvent() override;
123: 
124:     GPIOMode GetMode();
125:     bool SetMode(GPIOMode mode);
126:     GPIOFunction GetFunction();
127:     GPIOPullMode GetPullMode();
128:     void SetPullMode(GPIOPullMode pullMode);
129: 
130:     void EnableInterrupt(GPIOInterruptType interruptType);
131:     void DisableInterrupt(GPIOInterruptType interruptType);
132:     void DisableAllInterrupts();
133: 
134: private:
135:     void SetFunction(GPIOFunction function);
136: };
137: 
138: } // namespace baremetal
```

- Line 64-81: We add a enum type `GPIOInterruptType` to distiguish between the different types of events that can cause an interrupt.
They relate directly to the registers mentioned above in [BCMRegisters.h](#TUTORIAL_21_GPIO_FAKING_GPIO____STEP_1_BCMREGISTERSH)
- Line 101-102: We add a member variable `m_regOffset` to indicate the byte offset of the register to address for the GPIO.
This is relative to the group, and as there are 32 GPIO event bits in each register, this will thus be (m_pinNumber / 32) * 4
- Line 103-104: We add a member variable `m_regMask` to indicate the register mask to be used for the GPIO.
As there are 32 GPIO event bits in each register, this will thus be 1 << (m_pinNumber % 32)
- Line 105-106: We add a member variable `m_interruptEnabled` to indicate for each interrupt type whether it is enabled
- Line 121-122: We override the methods `GetEvent()` and `ClearEvent()` in the abstract class `IGPIOPin`
- Line 130: We add a method `EnableInterrupt()` to enable interrupts of the specified type
- Line 131: We add a method `DisableInterrupt()` to disable interrupts of the specified type
- Line 132: We add a method `DisableAllInterrupts()` to disable interrupts of all different types

### PhysicalGPIOPin.cpp {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3_PHYSICALGPIOPINCPP}

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
...
93: /// <summary>
94: /// Creates a virtual GPIO pin
95: /// </summary>
96: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
97: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
98:     : m_pinNumber{NUM_GPIO}
99:     , m_mode{GPIOMode::Unknown}
100:     , m_function{GPIOFunction::Unknown}
101:     , m_pullMode{GPIOPullMode::Unknown}
102:     , m_value{}
103:     , m_memoryAccess{memoryAccess}
104:     , m_regOffset{}
105:     , m_regMask{}
106:     , m_interruptEnabled{}
107: {
108: }
109: 
110: /// <summary>
111: /// Creates a virtual GPIO pin
112: /// </summary>
113: /// <param name="pinNumber">GPIO pin number (0..53)</param>
114: /// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
115: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
116: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
117:     : m_pinNumber{NUM_GPIO}
118:     , m_mode{GPIOMode::Unknown}
119:     , m_value{}
120:     , m_memoryAccess{memoryAccess}
121:     , m_regOffset{}
122:     , m_regMask{}
123:     , m_interruptEnabled{}
124: {
125:     AssignPin(pinNumber);
126:     SetMode(mode);
127: }
...
138: /// <summary>
139: /// Assign a GPIO pin
140: /// </summary>
141: /// <param name="pinNumber">GPIO pin number to set (0..53)</param>
142: /// <returns>Return true on success, false on failure</returns>
143: bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
144: {
145:     // Check if pin already assigned
146:     if (m_pinNumber != NUM_GPIO)
147:         return false;
148:     m_pinNumber = pinNumber;
149: 
150:     m_regOffset = (m_pinNumber / 32) * 4;
151:     m_regMask = 1 << (m_pinNumber % 32);
152: 
153:     return true;
154: }
...
222: /// <summary>
223: /// Get GPIO event status
224: /// </summary>
225: /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
226: bool PhysicalGPIOPin::GetEvent()
227: {
228:     return (m_memoryAccess.Read32(RPI_GPIO_GPEDS0 + m_regOffset) & m_regMask) != 0;
229: }
230: 
231: /// <summary>
232: /// Clear GPIO event status
233: /// </summary>
234: void PhysicalGPIOPin::ClearEvent()
235: {
236:     m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + m_regOffset, m_regMask);
237: }
238: 
...
361: /// <summary>
362: /// Enable interrupts for the specified type
363: /// </summary>
364: /// <param name="interruptType">Interrupt type to enable</param>
365: void PhysicalGPIOPin::EnableInterrupt(GPIOInterruptType interruptType)
366: {
367:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
368: 
369:     assert(interruptType < GPIOInterruptType::Unknown);
370:     if (!m_interruptEnabled[static_cast<size_t>(interruptType)])
371:     {
372:         m_interruptEnabled[static_cast<size_t>(interruptType)] = true;
373: 
374:         regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + (static_cast<unsigned>(interruptType) - static_cast<unsigned>(GPIOInterruptType::RisingEdge)) * 12;
375: 
376:         m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) | m_regMask);
377:     }
378: }
379: 
380: /// <summary>
381: /// Disable interrupts for the specified type
382: /// </summary>
383: /// <param name="interruptType">Interrupt type to disable</param>
384: void PhysicalGPIOPin::DisableInterrupt(GPIOInterruptType interruptType)
385: {
386:     assert(interruptType < GPIOInterruptType::Unknown);
387:     if (m_interruptEnabled[static_cast<size_t>(interruptType)])
388:     {
389:         m_interruptEnabled[static_cast<size_t>(interruptType)] = false;
390: 
391:         regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + (static_cast<unsigned>(interruptType) - static_cast<unsigned>(GPIOInterruptType::RisingEdge)) * 12;
392: 
393:         m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) & ~m_regMask);
394:     }
395: }
396: 
397: /// <summary>
398: /// Disable all interrupts
399: /// </summary>
400: void PhysicalGPIOPin::DisableAllInterrupts()
401: {
402:     DisableInterrupt(GPIOInterruptType::RisingEdge);
403:     DisableInterrupt(GPIOInterruptType::FallingEdge);
404:     DisableInterrupt(GPIOInterruptType::HighLevel);
405:     DisableInterrupt(GPIOInterruptType::LowLevel);
406:     DisableInterrupt(GPIOInterruptType::AsyncRisingEdge);
407:     DisableInterrupt(GPIOInterruptType::AsyncFallingEdge);
408: }
409: 
...
```

- Line 104-106: We initialize the new member variables in the 'default' constructor only taking the memory access instance
- Line 121-123: We initialize the new member variables in the 'specific' constructor also taking the pin number and mode
- Line 150-151: We set up the values for `m_regOffset` and `m_regMask` when a GPIO pin is assigned
- Line 222-229: We implement the `GetEvent()` method, by reading the correct event detection status register `RPI_GPIO_GPEDSn`, and checking whether the bit for this GPIO pin is a `1`, using the calculated mask
- Line 231-237: We implement the `ClearEvent()` method by writing the mask (containing only a `1` bit for this GPIO pin) and write to the event detection status register.
This has the effect of resetting the selected status bit
- Line 361-378: We implement `EnableInterrupt()`
  - Line 367: We verify that the GPIO pin is set to one of the input modes
  - Line 369: We verify that the interrupt type requested is valid
  - Line 370: We check that the interrupt type is not already enabled
  - Line 372: We set the registered interrupt flag to true
  - Line 374: We calculate the register address (remember that the registers are in groups of 3, with each using 4 bytes)
  - Line 376: We read the register, and the set the bit for the GPIO pin to `1` and write back
- Line 380-395: We implement `DisableInterrupt()`
  - Line 386: We verify that the interrupt type requested is valid
  - Line 387: We check that the interrupt type is enabled
  - Line 389: We set the registered interrupt flag to false
  - Line 391: We calculate the register address
  - Line 393: We read the register, and the set the bit for the GPIO pin to `0` and write back
- Line 397-408: We implement `DisableAllInterrupts()` by calling `DisableInterrupt()` for all types of events

### Update application code {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/InterruptHandler.h"
5: #include "baremetal/Interrupts.h"
6: #include "baremetal/Logger.h"
7: #include "baremetal/PhysicalGPIOPin.h"
8: #include "baremetal/System.h"
9: #include "baremetal/Timer.h"
10: 
11: LOG_MODULE("main");
12: 
13: using namespace baremetal;
14: 
15: struct GPIOPins
16: {
17:     IGPIOPin& pinCLK;
18:     IGPIOPin& pinDT;
19:     IGPIOPin& pinSW;
20: };
21: 
22: void InterruptHandler(void *param)
23: {
24:     TRACE_DEBUG("GPIO3");
25:     GPIOPins* pins = reinterpret_cast<GPIOPins*>(param);
26:     if (pins->pinCLK.GetEvent())
27:     {
28:         auto value = pins->pinCLK.Get();
29:         TRACE_DEBUG("CLK=%d", value);
30:         pins->pinCLK.ClearEvent();
31:     }
32:     if (pins->pinDT.GetEvent())
33:     {
34:         auto value = pins->pinDT.Get();
35:         TRACE_DEBUG("DT=%d", value);
36:         pins->pinDT.ClearEvent();
37:     }
38:     if (pins->pinSW.GetEvent())
39:     {
40:         auto value = pins->pinSW.Get();
41:         TRACE_DEBUG("SW=%d", value);
42:         pins->pinSW.ClearEvent();
43:     }
44: }
45: 
46: int main()
47: {
48:     auto& console = GetConsole();
49:     GetLogger().SetLogLevel(LogSeverity::Debug);
50: 
51:     auto exceptionLevel = CurrentEL();
52:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
53: 
54:     PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
55:     PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
56:     PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
57:     GPIOPins pins { pinCLK, pinDT, pinSW };
58: 
59:     GetInterruptSystem().RegisterIRQHandler(IRQ_ID::IRQ_GPIO3, InterruptHandler, &pins);
60: 
61:     pinCLK.EnableInterrupt(GPIOInterruptType::RisingEdge);
62:     pinCLK.EnableInterrupt(GPIOInterruptType::FallingEdge);
63:     pinDT.EnableInterrupt(GPIOInterruptType::RisingEdge);
64:     pinDT.EnableInterrupt(GPIOInterruptType::FallingEdge);
65:     pinSW.EnableInterrupt(GPIOInterruptType::RisingEdge);
66:     pinSW.EnableInterrupt(GPIOInterruptType::FallingEdge);
67: 
68:     LOG_INFO("Wait 5 seconds");
69:     Timer::WaitMilliSeconds(5000);
70: 
71:     GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);
72: 
73:     pinCLK.DisableAllInterrupts();
74:     pinDT.DisableAllInterrupts();
75:     pinSW.DisableAllInterrupts();
76: 
77:     console.Write("Press r to reboot, h to halt\n");
78:     char ch{};
79:     while ((ch != 'r') && (ch != 'h'))
80:     {
81:         ch = console.ReadChar();
82:         console.WriteChar(ch);
83:     }
84: 
85:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
86: }
```

- Line 7: We include the header for `PhysicalGPIOPin`
- Line 15-20: We set up a struct `GPIOPins` to access the different pins we are handling.
This is needed as the interrupt handler will not have any context, so we need to provide it through its parameter
- Line 22-44: We define the interrupt handler
  - Line 25: We convert the parameter to a pointer to the struct `GPIOPins`
  - Line 26-31: If the CLK pin had an event, we get its value and print it, then clear the event
  - Line 32-37: If the DT pin had an event, we get its value and print it, then clear the event
  - Line 38-43: If the SW pin had an event, we get its value and print it, then clear the event
- Line 57: We create and instance of the struct `GPIOPins` and fill it with pointers to the respective pins
- Line 59: We register to the interrupt `IRQ_GPIO3`. This is a shared interrupt, which is triggered when any of the pins generate an event
- Line 61-62: We enable interrupts for rising and falling edges on the CLK pin
- Line 63-64: We enable interrupts for rising and falling edges on the DT pin
- Line 65-66: We enable interrupts for rising and falling edges on the SW pin
- Line 68-69: We wait again as we did before, so we no longer actively monitor the pins
- Line 71: We unregister to the interrupt `IRQ_GPIO3`
- Line 73: We disable all interrupts on the CLK pin
- Line 74: We disable all interrupts on the DT pin
- Line 75: We disable all interrupts on the SW pin

### Configuring, building and debugging {#TUTORIAL_21_GPIO_ADDING_GPIO_INTERRUPTS___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

When we press the switch or turn it, you can see the interrupts coming in.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Current EL: 1 (main:52)
Debug  0.00:00:00.090 GPIO3 (main.cpp:24)
Debug  0.00:00:00.090 CLK=1 (main.cpp:29)
Debug  0.00:00:00.130 GPIO3 (main.cpp:24)
Debug  0.00:00:00.130 DT=1 (main.cpp:35)
Debug  0.00:00:00.160 GPIO3 (main.cpp:24)
Debug  0.00:00:00.160 SW=1 (main.cpp:41)
Info   0.00:00:00.200 Wait 5 seconds (main:68)
Debug  0.00:00:01.180 GPIO3 (main.cpp:24)
Debug  0.00:00:01.180 CLK=0 (main.cpp:29)
Debug  0.00:00:01.180 DT=1 (main.cpp:35)
Debug  0.00:00:01.230 GPIO3 (main.cpp:24)
Debug  0.00:00:01.230 CLK=1 (main.cpp:29)
Debug  0.00:00:01.940 GPIO3 (main.cpp:24)
Debug  0.00:00:01.940 CLK=0 (main.cpp:29)
Debug  0.00:00:01.980 GPIO3 (main.cpp:24)
Debug  0.00:00:01.980 CLK=0 (main.cpp:29)
Debug  0.00:00:01.980 DT=0 (main.cpp:35)
Debug  0.00:00:02.610 GPIO3 (main.cpp:24)
Debug  0.00:00:02.610 CLK=0 (main.cpp:29)
Debug  0.00:00:02.610 DT=1 (main.cpp:35)
Debug  0.00:00:02.670 GPIO3 (main.cpp:24)
Debug  0.00:00:02.670 CLK=1 (main.cpp:29)
Debug  0.00:00:03.150 GPIO3 (main.cpp:24)
Debug  0.00:00:03.150 DT=0 (main.cpp:35)
Debug  0.00:00:03.190 GPIO3 (main.cpp:24)
Debug  0.00:00:03.190 CLK=1 (main.cpp:29)
Debug  0.00:00:04.240 GPIO3 (main.cpp:24)
Debug  0.00:00:04.240 SW=0 (main.cpp:41)
Debug  0.00:00:04.410 GPIO3 (main.cpp:24)
Debug  0.00:00:04.410 SW=1 (main.cpp:41)
Debug  0.00:00:04.580 GPIO3 (main.cpp:24)
Debug  0.00:00:04.580 SW=0 (main.cpp:41)
Debug  0.00:00:04.710 GPIO3 (main.cpp:24)
Debug  0.00:00:04.710 SW=1 (main.cpp:41)
Press r to reboot, h to halt
```

## Generic approach for GPIO interrupts - Step 4 {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4}

As we can see in the previous step, we need to set up administration to pass to the interrupt handler.
That is impractical, it would be easier if we could register a handler for a specific GPIO pin interrupt.

We'll introduce a GPIO manager that will register to the GPIO interrupts, and have each pin register itself with the manager.
The GPIO manager will then check for each registered pin whether an event occurred, and call the interrupt handler on that pin.

### IGPIOManager.h {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_IGPIOMANAGERH}

First we'll create an abstract interface for the GPIO manager.

Create the file `code/libraries/baremetal/include/baremetal/IGPIOManager.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IGPIOManager.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : IGPIOManager.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : IGPIOManager
9: //
10: // Description : GPIO control abstract interface
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
45: /// GPIO configuration and control
46: 
47: namespace baremetal {
48: 
49: class IGPIOPin;
50: 
51: /// @brief Handles configuration, setting and getting GPIO controls
52: /// This is a singleton class, created as soon as GetIGPIOManager() is called
53: class IGPIOManager
54: {
55: public:
56:     /// <summary>
57:     /// Default destructor needed for abstract interface
58:     /// </summary>
59:     virtual ~IGPIOManager() = default;
60: 
61:     /// <summary>
62:     /// Initialize GPIO manager
63:     /// </summary>
64:     virtual void Initialize() = 0;
65: 
66:     /// <summary>
67:     /// Connect the GPIO pin interrupt for the specified pin
68:     /// </summary>
69:     /// <param name="pin">GPIO pin to connect interrupt for</param>
70:     virtual void ConnectInterrupt(IGPIOPin* pin) = 0;
71:     /// <summary>
72:     /// Disconnect the GPIO pin interrupt for the specified pin
73:     /// </summary>
74:     /// <param name="pin">GPIO pin to disconnect interrupt for</param>
75:     virtual void DisconnectInterrupt(const IGPIOPin* pin) = 0;
76: 
77:     /// <summary>
78:     /// GPIO pin interrupt handler, called by the static entry point GPIOInterruptHandler()
79:     /// </summary>
80:     virtual void InterruptHandler() = 0;
81: 
82:     /// <summary>
83:     /// Switch all GPIO pins to input mode, without pull-up or pull-down
84:     /// </summary>
85:     virtual void AllOff() = 0;
86: };
87: 
88: } // namespace baremetal
```

- Line 51-86: We declare the abstract class `IGPIOManager`
  - Line 61-64: We declare an abstract method `Initialize()` to initialize the manager
  - Line 66-70: We declare an abstract method `ConnectInterrupt()` to register a GPIO pin for interrupts
  - Line 71-75: We declare an abstract method `DisconnectInterrupt()` to unregister a GPIO pin for interrupts
  - Line 77-80: We declare an abstract method `InterruptHandler()` as the interrupt handler function
  - Line 82-85: We declare an abstract method `AllOff()` to unregister all GPIO pins and switch them to default mode

### GPIOManager.h {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_GPIOMANAGERH}

We'll now declare the GPIO manager class `GPIOManager`.

Create the file `code/libraries/baremetal/include/baremetal/GPIOManager.h`

```cpp
File: code/libraries/baremetal/include/baremetal/GPIOManager.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : GPIOManager.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : GPIOManager
9: //
10: // Description : GPIO control
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
42: #include "baremetal/IGPIOManager.h"
43: #include "baremetal/PhysicalGPIOPin.h"
44: #include "stdlib/Types.h"
45: 
46: /// @file
47: /// GPIO configuration and control
48: 
49: namespace baremetal {
50: 
51: class IMemoryAccess;
52: 
53: /// @brief Handles configuration, setting and getting GPIO controls
54: /// This is a singleton class, created as soon as GetGPIOManager() is called
55: class GPIOManager : public IGPIOManager
56: {
57:     /// <summary>
58:     /// Construct the singleton GPIOManager instance if needed, and return a reference to the instance. This is a friend function of class GPIOManager
59:     /// </summary>
60:     /// <returns>Reference to the singleton GPIOManager instance</returns>
61:     friend GPIOManager& GetGPIOManager();
62: 
63: private:
64:     /// @brief True if class is already initialized
65:     bool m_isInitialized;
66:     /// @brief Array of all registered GPIO pins (nullptr if the GPIO is not registered)
67:     IGPIOPin* m_pins[NUM_GPIO];
68:     /// @brief Memory access interface
69:     IMemoryAccess& m_memoryAccess;
70: 
71:     GPIOManager();
72: 
73: public:
74:     explicit GPIOManager(IMemoryAccess& memoryAccess);
75:     ~GPIOManager();
76: 
77:     void Initialize() override;
78: 
79:     void ConnectInterrupt(IGPIOPin* pin) override;
80:     void DisconnectInterrupt(const IGPIOPin* pin) override;
81: 
82:     void InterruptHandler() override;
83: 
84:     void AllOff() override;
85: 
86:     void DisableAllInterrupts(uint8 pinNumber);
87: 
88: private:
89: };
90: 
91: GPIOManager& GetGPIOManager();
92: 
93: } // namespace baremetal
```

- Line 53-89: We declare the class `GPIOManager`
  - Line 57-61: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager
  - Line 64-64: We declare a member variable `m_isInitialized` to guard against multiple initialization
  - Line 66-67: We declare a member variable `m_pins` to keep track of the registered GPIO pins
  - Line 68-69: We declare a member variable `m_memoryAccess` to hold the `IMemoryAccess` instance
  - Line 71: We declare the default constructor as private, as we did before
  - Line 74: We declare a non-default constructor taking a `IUMemoryAccess` instance
  - Line 75: We declare a destructor
  - Line 77: We declare a method `Initialize()` to override the abstract interface
  - Line 79: We declare a method `ConnectInterrupt()` to override the abstract interface
  - Line 80: We declare a method `DisconnectInterrupt()` to override the abstract interface
  - Line 82: We declare a method `InterruptHandler()` to override the abstract interface
  - Line 84: We declare a method `AllOff()` to override the abstract interface
  - Line 86: We declare a method `DisableAllInterrupts()` to disable all GPIO interrupts by clearing the respective registers
- Line 91: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager

### GPIOManager.cpp {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_GPIOMANAGERCPP}

Update the file `code/libraries/baremetal/src/GPIOManager.cpp`

```cpp
File: code/libraries/baremetal/src/GPIOManager.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : GPIOManager.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : GPIOManager
9: //
10: // Description : GPIO control
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
40: #include "baremetal/GPIOManager.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/InterruptHandler.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "baremetal/PhysicalGPIOPin.h"
48: #include "baremetal/Timer.h"
49: 
50: using namespace baremetal;
51: 
52: static const int WaitCycles = 150;
53: 
54: static const IRQ_ID GPIO_IRQ{IRQ_ID::IRQ_GPIO3}; // shared IRQ line for all GPIOs
55: 
56: static void GPIOInterruptHandler(void* param);
57: 
58: /// @brief Define log name
59: LOG_MODULE("GPIOManager");
60: 
61: /// <summary>
62: /// Create a GPIO control. Note that the constructor is private, so GetGPIOManager() is needed to instantiate the GPIO control
63: /// </summary>
64: GPIOManager::GPIOManager()
65:     : m_isInitialized{}
66:     , m_pins{}
67:     , m_memoryAccess{GetMemoryAccess()}
68: {
69: }
70: 
71: /// <summary>
72: /// Create a GPIO control with a custom memory access instance (for testing)
73: /// </summary>
74: /// <param name="memoryAccess">MemoryAccess instance to use for the GPIOManager</param>
75: GPIOManager::GPIOManager(IMemoryAccess& memoryAccess)
76:     : m_isInitialized{}
77:     , m_pins{}
78:     , m_memoryAccess{memoryAccess}
79: {
80: }
81: 
82: /// <summary>
83: /// GPIO manager destructor
84: /// </summary>
85: GPIOManager::~GPIOManager()
86: {
87: #ifndef NDEBUG
88:     for (uint8 pin = 0; pin < NUM_GPIO; pin++)
89:     {
90:         assert(m_pins[pin] == nullptr);
91:     }
92: #endif
93: 
94:     if (m_isInitialized)
95:     {
96:         GetInterruptSystem().UnregisterIRQHandler(GPIO_IRQ);
97:     }
98: }
99: 
100: /// <summary>
101: /// Initialize GPIO manager
102: /// </summary>
103: void GPIOManager::Initialize()
104: {
105:     if (m_isInitialized)
106:         return;
107:     GetInterruptSystem().RegisterIRQHandler(GPIO_IRQ, GPIOInterruptHandler, this);
108:     m_isInitialized = true;
109: }
110: 
111: /// <summary>
112: /// Connect the GPIO pin interrupt for the specified pin
113: /// </summary>
114: /// <param name="pin">GPIO pin to connect interrupt for</param>
115: void GPIOManager::ConnectInterrupt(IGPIOPin* pin)
116: {
117:     assert(m_isInitialized);
118: 
119:     assert(pin != nullptr);
120:     auto pinNumber = pin->GetPinNumber();
121:     assert(pinNumber < NUM_GPIO);
122: 
123:     assert(m_pins[pinNumber] == nullptr);
124:     m_pins[pinNumber] = pin;
125: }
126: 
127: /// <summary>
128: /// Disconnect the GPIO pin interrupt for the specified pin
129: /// </summary>
130: /// <param name="pin">GPIO pin to disconnect interrupt for</param>
131: void GPIOManager::DisconnectInterrupt(const IGPIOPin* pin)
132: {
133:     assert(m_isInitialized);
134: 
135:     assert(pin != nullptr);
136:     auto pinNumber = pin->GetPinNumber();
137:     assert(pinNumber < NUM_GPIO);
138: 
139:     assert(m_pins[pinNumber] != nullptr);
140:     m_pins[pinNumber] = nullptr;
141: }
142: 
143: /// <summary>
144: /// Switch all GPIO pins to input mode, without pull-up or pull-down
145: /// </summary>
146: void GPIOManager::AllOff()
147: {
148:     // power off gpio pins (but not VCC pins)
149:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL0, 0); // All pins input
150:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL1, 0);
151:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL2, 0);
152:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL3, 0);
153:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL4, 0);
154:     m_memoryAccess.Write32(RPI_GPIO_GPFSEL5, 0);
155: #if BAREMETAL_RPI_TARGET == 3
156:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, 0); // All mode Off
157:     Timer::WaitCycles(WaitCycles);
158:     m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK0, 0xFFFFFFFF); // Trigger clock
159:     m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK1, 0xFFFFFFFF);
160:     Timer::WaitCycles(WaitCycles);
161:     m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK0, 0); // Release clock
162:     m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK1, 0);
163: #else
164:     m_memoryAccess.Write32(RPI_GPIO_GPPUPPDN0, 0); // All mode Off
165:     m_memoryAccess.Write32(RPI_GPIO_GPPUPPDN1, 0);
166: #endif
167: }
168: 
169: /// <summary>
170: /// GPIO pin interrupt handler, called by the static entry point GPIOInterruptHandler()
171: /// </summary>
172: void GPIOManager::InterruptHandler()
173: {
174:     assert(m_isInitialized);
175: 
176:     uint32 eventStatus = m_memoryAccess.Read32(RPI_GPIO_GPEDS0);
177: 
178:     uint8 pinNumber = 0;
179:     while (pinNumber < NUM_GPIO)
180:     {
181:         if (eventStatus & 1)
182:         {
183:             break;
184:         }
185:         eventStatus >>= 1;
186: 
187:         if (++pinNumber % 32 == 0)
188:         {
189:             eventStatus = m_memoryAccess.Read32(RPI_GPIO_GPEDS1);
190:         }
191:     }
192: 
193:     if (pinNumber < NUM_GPIO)
194:     {
195:         auto pin = m_pins[pinNumber];
196:         if (pin != nullptr)
197:         {
198:             pin->InterruptHandler();
199: 
200:             if (pin->GetAutoAcknowledgeInterrupt())
201:             {
202:                 pin->AcknowledgeInterrupt();
203:             }
204:         }
205:         else
206:         {
207:             LOG_ERROR("No pin found for interrupt");
208: 
209:             uint32 regOffset{static_cast<uint32>((pinNumber / 32) * 4)};
210:             uint32 regMask{static_cast<uint32>(1 << (pinNumber % 32))};
211:             m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + regOffset, regMask);
212:         }
213:     }
214: }
215: 
216: /// <summary>
217: /// Disable all GPIO interrupt types for the specified in number
218: /// </summary>
219: /// <param name="pinNumber">Pin number of GPIO to disable interrupts for</param>
220: void GPIOManager::DisableAllInterrupts(uint8 pinNumber)
221: {
222:     assert(pinNumber < NUM_GPIO);
223: 
224:     uint32 mask = 1 << (pinNumber % 32);
225: 
226:     for (auto reg = RPI_GPIO_GPREN0 + (pinNumber / 32) * 4; reg < RPI_GPIO_GPAFEN0 + 4; reg += 12)
227:     {
228:         m_memoryAccess.Write32(reg, m_memoryAccess.Read32(reg) & ~mask);
229:     }
230: }
231: 
232: /// <summary>
233: /// GPIO IRQ entry pointer, calls Interrupt handle on the GPIOManager instance passed through param
234: /// </summary>
235: /// <param name="param">Pointer to GPIOManager instance</param>
236: void GPIOInterruptHandler(void* param)
237: {
238:     GPIOManager* pThis = reinterpret_cast<GPIOManager*>(param);
239:     assert(pThis != nullptr);
240: 
241:     pThis->InterruptHandler();
242: }
243: 
244: /// <summary>
245: /// Create a singleton GPIOManager if neededm and return the singleton instance
246: /// </summary>
247: /// <returns>A reference to the singleton GPIOManager.</returns>
248: GPIOManager& baremetal::GetGPIOManager()
249: {
250:     static GPIOManager control;
251:     control.Initialize();
252:     return control;
253: }
```

- Line 56: We forward declare the interrupt handler function `GPIOInterruptHandler()`
- Line 61-69: We implement the default constructor. This is straightforward
- Line 71-80: We implement the specialized constructor. This is straightforward
- Line 82-98: We implement the destructor
  - Line 88-91: We verify there are no lingering pins connected
  - Line 94-97: We unregister from the GPIO IRQ
- Line 100-109: We implement the method `Initialize()`
  - Line 105-106: We protect against multiple initialization, as we did before
  - Line 107-108: We register to the GPIO IRQ, and set the initialized flag.
When an interrupt occurs, the local function `GPIOInterruptHandler()` is called
- Line 111-125: We implement the method `ConnectInterrupt()`
  - Line 117: We verify that the `GPIOManager` was initialized
  - Line 119: We verify that the pin pointer is valid
  - Line 120-121: We extract the pin number and verify that it is valid
  - Line 123-124: We verify that no interrupt handler was installed yet for the requested pin, and then register the pin
- Line 127-141: We implement the method `DisconnectInterrupt()`
  - Line 133: We verify that the `GPIOManager` was initialized
  - Line 135: We verify that the pin pointer is valid
  - Line 136-137: We extract the pin number and verify that it is valid
  - Line 139-140: We verify that a interrupt handler was installed for the requested pin, and then set the registration for the pin to nullptr
- Line 143-167: We implement the method `AllOff()`
  - Line 149-154: We set all pins to input mode
  - Line 156-162: We set all pins to pullmode off for Raspberry Pi 3
  - Line 164-165: We set all pins to pullmode off for Raspberry Pi 4
- Line 169-214: We implement the method `InterruptHandler()` in the class
  - Line 174: We verify that the `GPIOManager` was initialized
  - Line 176: We read the event status for GPIO 0..31
  - Line 178-191: We check for each GPIO number whether an event was set. When going beyond GPIO 31, we pick up the event status for interrupts 32..53/56.
For the first `1` bit we step out of this loop, as we only handle one GPIO per call.
The interrupt routine will be called again for other GPIO if needed
  - Line 195: We retrieve the pin
  - Line 196-204: If the pin is registered for interrupts, we call its `InterruptHandler`, and if auto acklowledge is request, perform an acknowledgement.
The latter resets the event status bit for the specified GPIO
  - Line 207-211: If the pin is not registered, we print an error, and reset the event bit for the pin
- Line 216-230: We implement the method `DisableAllInterrupts()`.
This removes all interrupts for the specified pin, by resetting the interrupt bit for each GPIO interrupt type
- Line 232-242: We implement the function `GPIOInterruptHandler()`.
This is the static entry point to the `GPIOManager` for interrupts, and relays to the `InterruptHandler()` method of the singleton instance
- Line 244-253: We implement the `GetGPIOManager()` function to return the singleton instance of the `GPIOManager`

### IGPIOPin.h {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_IGPIOPINH}

We need to add the methods `InterruptHandler()`, `GetAutoAcknowledgeInterrupt()` and `AcknowledgeInterrupt()` to the GPIO pin.

Update the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IGPIOPin.h
...
76: /// <summary>
77: /// Abstraction of a GPIO pin
78: /// </summary>
79: class IGPIOPin
80: {
81: public:
82:     /// <summary>
83:     /// Default destructor needed for abstract interface
84:     /// </summary>
85:     virtual ~IGPIOPin() = default;
86: 
87:     /// <summary>
88:     /// Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
89:     /// </summary>
90:     /// <returns>Pin number</returns>
91:     virtual uint8 GetPinNumber() const = 0;
92:     /// <summary>
93:     /// Assign a GPIO pin
94:     /// </summary>
95:     /// <param name="pinNumber">Pin number</param>
96:     /// <returns>true if successful, false otherwise</returns>
97:     virtual bool AssignPin(uint8 pinNumber) = 0;
98: 
99:     /// <summary>
100:     /// Switch GPIO on
101:     /// </summary>
102:     virtual void On() = 0;
103:     /// <summary>
104:     /// Switch GPIO off
105:     /// </summary>
106:     virtual void Off() = 0;
107:     /// <summary>
108:     /// Get GPIO value
109:     /// </summary>
110:     /// <returns>GPIO value, true if on, false if off</returns>
111:     virtual bool Get() = 0;
112:     /// <summary>
113:     /// Set GPIO on (true) or off (false)
114:     /// </summary>
115:     /// <param name="on">Value to set, on (true) or off (false)</param>
116:     virtual void Set(bool on) = 0;
117:     /// <summary>
118:     /// Invert GPIO value on->off off->on
119:     /// </summary>
120:     virtual void Invert() = 0;
121:     /// <summary>
122:     /// Get GPIO event status
123:     /// </summary>
124:     /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
125:     virtual bool GetEvent() = 0;
126:     /// <summary>
127:     /// Clear GPIO event status
128:     /// </summary>
129:     virtual void ClearEvent() = 0;
130:     /// <summary>
131:     /// Determine whether interrupts for the GPIO pin are automatically acknowledged (by reseting the event bit)
132:     /// </summary>
133:     /// <returns>Returns true if a GPIO interrupt is automatically acknowledged, false if not</returns>
134:     virtual bool GetAutoAcknowledgeInterrupt() const = 0;
135:     /// <summary>
136:     /// Acknowledge interrupts for the GPIO pin, by reseting the event bit
137:     /// </summary>
138:     virtual void AcknowledgeInterrupt() = 0;
139:     /// <summary>
140:     /// Handler for an interrupt on the GPIO pin
141:     /// </summary>
142:     virtual void InterruptHandler() = 0;
143: };
...
```

- Line 130-134: We declare a method `GetAutoAcknowledgeInterrupt` to retrieve the auto acknowledge setting
- Line 135-138: We declare a method `AcknowledgeInterrupt()` to perform GPIO acknowledgement, by resetting the event bit for the GPIO
- Line 139-142: We declare a method `InterruptHandler()` as the GPIO pin interrupt handler

### PhysicalGPIOPin.h {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_PHYSICALGPIOPINH}

We need to add the new methods in `IGPIOPin`, as well as allow for registering and unregistering and interrupt handler with the `GPIOManager`.
We'll also change the way we deal with the different interrupts sources, by using bit patterns to allow for enabling and disabling multiple interrupt types at once.

Update the file `code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h
...
64: /// @brief Interrupt type to enable
65: enum class GPIOInterruptTypes : uint8
66: {
67:     /// @brief Interrupt on rising edge
68:     RisingEdge = 0x01,
69:     /// @brief Interrupt on falling edge
70:     FallingEdge = 0x02,
71:     /// @brief Interrupt on low level
72:     HighLevel = 0x04,
73:     /// @brief Interrupt on high level
74:     LowLevel = 0x08,
75:     /// @brief Interrupt on asynchronous rising edge
76:     AsyncRisingEdge = 0x10,
77:     /// @brief Interrupt on asynchronous falling edge
78:     AsyncFallingEdge = 0x20,
79:     /// @brief No interrupts
80:     None = 0x00,
81:     /// @brief All interrupts
82:     All = 0x3F,
83: };
84: /// <summary>
85: /// Combine two GPIO pin interrupt types, by performing a binary or on the two bit patterns
86: /// </summary>
87: /// <param name="lhs">First GPIO pin interrupt type</param>
88: /// <param name="rhs">Second GPIO pin interrupt type</param>
89: /// <returns>Combined GPIO pin interrupt type</returns>
90: inline GPIOInterruptTypes operator|(GPIOInterruptTypes lhs, GPIOInterruptTypes rhs)
91: {
92:     return static_cast<GPIOInterruptTypes>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs));
93: }
94: 
95: /// <summary>
96: /// GPIO pin interrupt handler
97: /// </summary>
98: using GPIOPinInterruptHandler = void(IGPIOPin* pin, void* param);
99: 
100: /// <summary>
101: /// Physical GPIO pin (i.e. available on GPIO header)
102: /// </summary>
103: class PhysicalGPIOPin : public IGPIOPin
104: {
105: private:
106:     /// @brief Configured GPIO pin number (0..53)
107:     uint8 m_pinNumber;
108:     /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
109:     GPIOMode m_mode;
110:     /// @brief Configured GPIO function.
111:     GPIOFunction m_function;
112:     /// @brief Configured GPIO pull mode (only for input function).
113:     GPIOPullMode m_pullMode;
114:     /// @brief Current value of the GPIO pin (true for on, false for off).
115:     bool m_value;
116:     /// @brief Memory access interface reference for accessing registers.
117:     IMemoryAccess& m_memoryAccess;
118:     /// @brief Register offset for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
119:     unsigned m_regOffset;
120:     /// @brief Register mask for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
121:     uint32 m_regMask;
122:     /// @brief Interrupt handler for the pin
123:     GPIOPinInterruptHandler* m_handler;
124:     /// @brief Interrupt handler parameter for the pin
125:     void* m_handlerParam;
126:     /// @brief Auto acknowledge interrupt for the pin. If true, the interrupt handler will of the GPIOManager will automatically reset the event state
127:     bool m_autoAcknowledge;
128:     /// @brief GPIO interrupt types enabled
129:     uint8 m_interruptMask;
130: 
131: public:
132:     PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());
133: 
134:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());
135: 
136:     uint8 GetPinNumber() const override;
137:     bool AssignPin(uint8 pinNumber) override;
138: 
139:     void On() override;
140:     void Off() override;
141:     bool Get() override;
142:     void Set(bool on) override;
143:     void Invert() override;
144:     bool GetEvent() override;
145:     void ClearEvent() override;
146: 
147:     GPIOMode GetMode();
148:     bool SetMode(GPIOMode mode);
149:     GPIOFunction GetFunction();
150:     GPIOPullMode GetPullMode();
151:     void SetPullMode(GPIOPullMode pullMode);
152: 
153:     bool GetAutoAcknowledgeInterrupt() const override;
154:     void AcknowledgeInterrupt() override;
155:     void InterruptHandler() override;
156: 
157:     void ConnectInterrupt(GPIOPinInterruptHandler* handler, void* param, bool autoAcknowledge = true);
158:     void DisconnectInterrupt();
159: 
160:     void EnableInterrupt(GPIOInterruptTypes interruptTypes);
161:     void DisableInterrupt(GPIOInterruptTypes interruptTypes);
162:     void DisableAllInterrupts();
163: 
164: private:
165:     void SetFunction(GPIOFunction function);
166: };
167: 
168: } // namespace baremetal
```

- Line 64-83: We change the GPIO interrupt types from a regular enum to a set of bit fields, so they can be combined
- Line 84-93: We define an operator `|` to combine interrupt types
- Line 95-98: We declare the GPIO pin interrupt handler type. This will not only receive a parameter, but also the pin instance itself
- Line 122-123: We add a member variable `m_handler` for the GPIO pin interrupt handler
- Line 124-125: We add a member variable `m_handlerParam` for the GPIO pin interrupt handler parameter
- Line 126-127: We add a member variable `m_autoAcknowledge` to enable auto acknowledge for the GPIO pin
- Line 128-129: We change the `m_interruptEnabled` field to a simple bit flag `m_interruptMask`
- Line 153: We declare a method `GetAutoAcknowledgeInterrupt()` to retrieve the auto acknowledge setting
- Line 154: We declare a method `AcknowledgeInterrupt()` to perform GPIO acknowledgement, by resetting the event bit for the GPIO
- Line 155: We declare a method `InterruptHandler()` as the GPIO pin interrupt handler
- Line 157: We declare a method `ConnectInterrupt()` to connect the GPIO pint interrupt with its parameter, and set the auto acknowledge status
- Line 158: We declare a method `DisconnectInterrupt()` to disconnect the GPIO pint interrupt

### PhysicalGPIOPin.cpp {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_PHYSICALGPIOPINCPP}

We need to implement the new methods, and change the way we deal with the interrupt type registration.

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
...
40: #include "baremetal/PhysicalGPIOPin.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/GPIOManager.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "baremetal/Timer.h"
48: 
49: /// @file
50: /// Physical GPIO pin implementation
51: 
52: /// @brief Define log name
53: LOG_MODULE("PhysicalGPIOPin");
54: 
55: namespace baremetal {
56: 
57: /// @brief Interrupt type register offset
58: enum GPIOInterruptTypeOffset
59: {
60:     /// @brief Interrupt on rising edge
61:     OffsetRisingEdge = 0,
62:     /// @brief Interrupt on falling edge
63:     OffsetFallingEdge = 12,
64:     /// @brief Interrupt on low level
65:     OffsetHighLevel = 24,
66:     /// @brief Interrupt on high level
67:     OffsetLowLevel = 36,
68:     /// @brief Interrupt on asynchronous rising edge
69:     OffsetAsyncRisingEdge = 48,
70:     /// @brief Interrupt on asynchronous falling edge
71:     OffsetAsyncFallingEdge = 60,
72: };
73: 
74: /// @brief GPIO function
75: enum class GPIOFunction
76: {
77:     /// @brief GPIO used as input
78:     Input,
79:     /// @brief GPIO used as output
80:     Output,
81:     /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
82:     AlternateFunction0,
83:     /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
84:     AlternateFunction1,
85:     /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
86:     AlternateFunction2,
87:     /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
88:     AlternateFunction3,
89:     /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
90:     AlternateFunction4,
91:     /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
92:     AlternateFunction5,
93:     /// @brief GPIO function unknown / not set / invalid
94:     Unknown,
95: };
96: 
97: /// @brief GPIO pull mode
98: enum class GPIOPullMode
99: {
100:     /// @brief GPIO pull mode off (no pull-up or pull-down)
101:     Off,
102:     /// @brief GPIO pull mode pull-down
103:     PullDown,
104:     /// @brief GPIO pull mode pull-up
105:     PullUp,
106:     /// @brief GPIO pull mode unknown / not set / invalid
107:     Unknown,
108: };
109: 
110: #if BAREMETAL_RPI_TARGET == 3
111: /// @brief Number of cycles to wait when setting pull mode for GPIO pin (Raspberry Pi 3 only)
112: static const int NumWaitCycles = 150;
113: #endif // BAREMETAL_RPI_TARGET == 3
114: 
115: /// <summary>
116: /// Creates a virtual GPIO pin
117: /// </summary>
118: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
119: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
120:     : m_pinNumber{NUM_GPIO}
121:     , m_mode{GPIOMode::Unknown}
122:     , m_function{GPIOFunction::Unknown}
123:     , m_pullMode{GPIOPullMode::Unknown}
124:     , m_value{}
125:     , m_memoryAccess{memoryAccess}
126:     , m_regOffset{}
127:     , m_regMask{}
128:     , m_handler{}
129:     , m_handlerParam{}
130:     , m_autoAcknowledge{}
131:     , m_interruptMask{}
132: {
133: }
134: 
135: /// <summary>
136: /// Creates a virtual GPIO pin
137: /// </summary>
138: /// <param name="pinNumber">GPIO pin number (0..53)</param>
139: /// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
140: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
141: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
142:     : m_pinNumber{NUM_GPIO}
143:     , m_mode{GPIOMode::Unknown}
144:     , m_value{}
145:     , m_memoryAccess{memoryAccess}
146:     , m_regOffset{}
147:     , m_regMask{}
148:     , m_handler{}
149:     , m_handlerParam{}
150:     , m_autoAcknowledge{}
151:     , m_interruptMask{}
152: {
153:     AssignPin(pinNumber);
154:     SetMode(mode);
155: }
156: 
157: /// <summary>
158: /// Return the configured GPIO pin number
159: /// </summary>
160: /// <returns>GPIO pin number (0..53)</returns>
161: uint8 PhysicalGPIOPin::GetPinNumber() const
162: {
163:     return m_pinNumber;
164: }
165: 
166: /// <summary>
167: /// Assign a GPIO pin
168: /// </summary>
169: /// <param name="pinNumber">GPIO pin number to set (0..53)</param>
170: /// <returns>Return true on success, false on failure</returns>
171: bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
172: {
173:     // Check if pin already assigned
174:     if (m_pinNumber != NUM_GPIO)
175:         return false;
176:     m_pinNumber = pinNumber;
177: 
178:     m_regOffset = (m_pinNumber / 32) * 4;
179:     m_regMask = 1 << (m_pinNumber % 32);
180: 
181:     return true;
182: }
183: 
184: /// <summary>
185: /// Switch GPIO on
186: /// </summary>
187: void PhysicalGPIOPin::On()
188: {
189:     Set(true);
190: }
191: 
192: /// <summary>
193: /// Switch GPIO off
194: /// </summary>
195: void PhysicalGPIOPin::Off()
196: {
197:     Set(false);
198: }
199: 
200: /// <summary>
201: /// Get GPIO value
202: /// </summary>
203: /// <returns>The status of the configured GPIO pin. Returns true if on, false if off</returns>
204: bool PhysicalGPIOPin::Get()
205: {
206:     // Check if pin is assigned
207:     if (m_pinNumber >= NUM_GPIO)
208:         return false;
209: 
210:     if ((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown))
211:     {
212:         uint32 regOffset = (m_pinNumber / 32);
213:         uint32 regMask = 1 << (m_pinNumber % 32);
214:         return (m_memoryAccess.Read32(RPI_GPIO_GPLEV0 + regOffset * 4) & regMask) ? true : false;
215:     }
216:     return m_value;
217: }
218: 
219: /// <summary>
220: /// Set GPIO on (true) or off (false)
221: /// </summary>
222: /// <param name="on">Value to set GPIO pin to (true for on, false for off).</param>
223: void PhysicalGPIOPin::Set(bool on)
224: {
225:     // Check if pin is assigned
226:     if (m_pinNumber >= NUM_GPIO)
227:         return;
228: 
229:     // Check if mode is output
230:     if (m_mode != GPIOMode::Output)
231:         return;
232: 
233:     m_value = on;
234: 
235:     uint32 regOffset = (m_pinNumber / 32);
236:     uint32 regMask = 1 << (m_pinNumber % 32);
237:     regaddr regAddress = (m_value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset * 4;
238: 
239:     m_memoryAccess.Write32(regAddress, regMask);
240: }
241: 
242: /// <summary>
243: /// Invert GPIO value on->off off->on
244: /// </summary>
245: void PhysicalGPIOPin::Invert()
246: {
247:     Set(!Get());
248: }
249: 
250: /// <summary>
251: /// Get GPIO event status
252: /// </summary>
253: /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
254: bool PhysicalGPIOPin::GetEvent()
255: {
256:     return (m_memoryAccess.Read32(RPI_GPIO_GPEDS0 + m_regOffset) & m_regMask) != 0;
257: }
258: 
259: /// <summary>
260: /// Clear GPIO event status
261: /// </summary>
262: void PhysicalGPIOPin::ClearEvent()
263: {
264:     m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + m_regOffset, m_regMask);
265: }
266: 
267: /// /// <summary>
268: /// Get the mode for the GPIO pin
269: /// </summary>
270: /// <returns>Currently set mode for the configured GPIO pin</returns>
271: GPIOMode PhysicalGPIOPin::GetMode()
272: {
273:     return m_mode;
274: }
275: 
276: /// <summary>
277: /// Convert GPIO mode to GPIO function. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
278: /// </summary>
279: /// <param name="mode">GPIO mode</param>
280: /// <returns>GPIO function</returns>
281: static GPIOFunction ConvertGPIOModeToFunction(GPIOMode mode)
282: {
283:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
284:     {
285:         unsigned alternateFunctionIndex = static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0);
286:         return static_cast<GPIOFunction>(static_cast<unsigned>(GPIOFunction::AlternateFunction0) + alternateFunctionIndex);
287:     }
288:     else if (GPIOMode::Output == mode)
289:     {
290:         return GPIOFunction::Output;
291:     }
292:     return GPIOFunction::Input;
293: }
294: 
295: /// <summary>
296: /// Set the mode for the GPIO pin
297: /// </summary>
298: /// <param name="mode">Mode to be set for the configured GPIO. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
299: /// <returns>Return true on success, false on failure</returns>
300: bool PhysicalGPIOPin::SetMode(GPIOMode mode)
301: {
302:     // Check if pin is assigned
303:     if (m_pinNumber >= NUM_GPIO)
304:         return false;
305: 
306:     // Check if mode is valid
307:     if (mode >= GPIOMode::Unknown)
308:         return false;
309: 
310:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
311:     {
312:         SetPullMode(GPIOPullMode::Off);
313: 
314:         SetFunction(ConvertGPIOModeToFunction(mode));
315:     }
316:     else if (GPIOMode::Output == mode)
317:     {
318:         SetPullMode(GPIOPullMode::Off);
319: 
320:         SetFunction(ConvertGPIOModeToFunction(mode));
321:     }
322:     else
323:     {
324:         SetPullMode((mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off);
325:         SetFunction(ConvertGPIOModeToFunction(mode));
326:     }
327:     m_mode = mode;
328:     if (m_mode == GPIOMode::Output)
329:         Off();
330:     return true;
331: }
332: 
333: /// <summary>
334: /// Get GPIO pin function
335: /// </summary>
336: /// <returns>Function set for the configured GPIO pin</returns>
337: GPIOFunction PhysicalGPIOPin::GetFunction()
338: {
339:     return m_function;
340: }
341: 
342: /// <summary>
343: /// Get GPIO pin pull mode
344: /// </summary>
345: /// <returns>Pull mode set for the configured GPIO pin</returns>
346: GPIOPullMode PhysicalGPIOPin::GetPullMode()
347: {
348:     return m_pullMode;
349: }
350: 
351: /// <summary>
352: /// Set GPIO pin pull mode
353: /// </summary>
354: /// <param name="pullMode">Pull mode to be set for the configured GPIO pin</param>
355: void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
356: {
357:     // Check if pin is assigned
358:     if (m_pinNumber >= NUM_GPIO)
359:         return;
360: 
361:     // Check if mode is valid
362:     if (pullMode >= GPIOPullMode::Unknown)
363:         return;
364: 
365: #if BAREMETAL_RPI_TARGET == 3
366:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32) * 4;
367:     uint32 shift = m_pinNumber % 32;
368: 
369:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
370:     Timer::WaitCycles(NumWaitCycles);
371:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
372:     Timer::WaitCycles(NumWaitCycles);
373:     m_memoryAccess.Write32(clkRegister, 0);
374: #else
375:     regaddr modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16) * 4;
376:     unsigned shift = (m_pinNumber % 16) * 2;
377: 
378:     static const unsigned ModeMap[3] = {0, 2, 1};
379: 
380:     uint32 value = m_memoryAccess.Read32(modeReg);
381:     value &= ~(3 << shift);
382:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
383:     m_memoryAccess.Write32(modeReg, value);
384: #endif
385: 
386:     m_pullMode = pullMode;
387: }
388: 
389: /// <summary>
390: /// Set up an interrupt handler for the GPIO pin
391: /// </summary>
392: /// <param name="handler">Interrupt handler function</param>
393: /// <param name="param">Interrupt handler parameter</param>
394: /// <param name="autoAcknowledge">Perform auto acknowledge on the GPIO pin event status</param>
395: void PhysicalGPIOPin::ConnectInterrupt(GPIOPinInterruptHandler* handler, void* param, bool autoAcknowledge /*= true*/)
396: {
397:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
398: 
399:     assert(m_interruptMask == static_cast<uint8>(GPIOInterruptTypes::None));
400: 
401:     assert(handler != nullptr);
402:     assert(m_handler == nullptr);
403:     m_handler = handler;
404:     m_handlerParam = param;
405:     m_autoAcknowledge = autoAcknowledge;
406: 
407:     GetGPIOManager().ConnectInterrupt(this);
408: }
409: 
410: /// <summary>
411: /// Remove the interrupt handler for the GPIO pin.
412: /// </summary>
413: void PhysicalGPIOPin::DisconnectInterrupt()
414: {
415:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
416: 
417:     DisableAllInterrupts();
418: 
419:     m_handler = nullptr;
420: 
421:     GetGPIOManager().DisconnectInterrupt(this);
422: }
423: 
424: /// <summary>
425: /// Return the interrupt auto acknowledge setting
426: /// </summary>
427: /// <returns>Returns the interrupt auto acknowledge setting</returns>
428: bool PhysicalGPIOPin::GetAutoAcknowledgeInterrupt() const
429: {
430:     return m_autoAcknowledge;
431: }
432: 
433: /// <summary>
434: /// Acknowledge the GPIO pin interrupt by resetting the event status for this GPIO
435: /// </summary>
436: void PhysicalGPIOPin::AcknowledgeInterrupt()
437: {
438:     assert(m_handler != nullptr);
439: 
440:     m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + m_regOffset, m_regMask);
441: }
442: 
443: /// <summary>
444: /// GPIO pin interrupt handler
445: /// </summary>
446: void PhysicalGPIOPin::InterruptHandler()
447: {
448:     LOG_DEBUG("Interrupt for GPIO pin %d handled", m_pinNumber);
449:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
450: 
451:     assert(m_handler != nullptr);
452:     if (m_interruptMask != static_cast<uint8>(GPIOInterruptTypes::None))
453:         (*m_handler)(this, m_handlerParam);
454: }
455: 
456: /// <summary>
457: /// Convert an interrupt type to a register offset
458: /// </summary>
459: /// <param name="interruptTypes">Interrupt type (only a single bit set)</param>
460: /// <returns>Register offset from the rising edge interrupt register</returns>
461: static GPIOInterruptTypeOffset ConvertToOffset(GPIOInterruptTypes interruptTypes)
462: {
463:     switch (interruptTypes)
464:     {
465:     case GPIOInterruptTypes::RisingEdge:
466:         return GPIOInterruptTypeOffset::OffsetRisingEdge;
467:     case GPIOInterruptTypes::FallingEdge:
468:         return GPIOInterruptTypeOffset::OffsetFallingEdge;
469:     case GPIOInterruptTypes::HighLevel:
470:         return GPIOInterruptTypeOffset::OffsetHighLevel;
471:     case GPIOInterruptTypes::LowLevel:
472:         return GPIOInterruptTypeOffset::OffsetLowLevel;
473:     case GPIOInterruptTypes::AsyncRisingEdge:
474:         return GPIOInterruptTypeOffset::OffsetAsyncRisingEdge;
475:     case GPIOInterruptTypes::AsyncFallingEdge:
476:         return GPIOInterruptTypeOffset::OffsetAsyncFallingEdge;
477:     default:
478:         return GPIOInterruptTypeOffset::OffsetRisingEdge;
479:     }
480: }
481: 
482: /// <summary>
483: /// Enable interrupts for the specified type
484: /// </summary>
485: /// <param name="interruptTypes">Set of interrupt types to enable</param>
486: void PhysicalGPIOPin::EnableInterrupt(GPIOInterruptTypes interruptTypes)
487: {
488:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
489: 
490:     uint8 interruptMask = static_cast<uint8>(interruptTypes);
491:     assert((interruptMask & ~static_cast<uint8>(GPIOInterruptTypes::All)) == 0);
492:     uint8 pattern = static_cast<uint8>(GPIOInterruptTypes::AsyncFallingEdge);
493:     while (pattern != 0)
494:     {
495:         if ((interruptMask & pattern) != 0)
496:         {
497:             m_interruptMask |= pattern;
498:             regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + ConvertToOffset(static_cast<GPIOInterruptTypes>(pattern));
499:             m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) | m_regMask);
500:         }
501:         pattern >>= 1;
502:     }
503: }
504: 
505: /// <summary>
506: /// Disable interrupts for the specified type
507: /// </summary>
508: /// <param name="interruptTypes">Set of interrupt types to disable</param>
509: void PhysicalGPIOPin::DisableInterrupt(GPIOInterruptTypes interruptTypes)
510: {
511:     uint8 interruptMask = static_cast<uint8>(interruptTypes);
512:     assert((interruptMask & ~static_cast<uint8>(GPIOInterruptTypes::All)) == 0);
513:     uint8 pattern = static_cast<uint8>(GPIOInterruptTypes::AsyncFallingEdge);
514:     while (pattern != 0)
515:     {
516:         if ((interruptMask & pattern) != 0)
517:         {
518:             m_interruptMask &= ~pattern;
519:             regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + ConvertToOffset(static_cast<GPIOInterruptTypes>(pattern));
520:             m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) & ~m_regMask);
521:         }
522:         pattern >>= 1;
523:     }
524: }
525: 
526: /// <summary>
527: /// Disable all interrupts
528: /// </summary>
529: void PhysicalGPIOPin::DisableAllInterrupts()
530: {
531:     DisableInterrupt(GPIOInterruptTypes::All);
532: }
533: 
534: /// <summary>
535: /// Set GPIO pin function
536: /// </summary>
537: /// <param name="function">Function to be set for the configured GPIO pin</param>
538: void PhysicalGPIOPin::SetFunction(GPIOFunction function)
539: {
540:     // Check if pin is assigned
541:     if (m_pinNumber >= NUM_GPIO)
542:         return;
543: 
544:     // Check if mode is valid
545:     if (function >= GPIOFunction::Unknown)
546:         return;
547: 
548:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (m_pinNumber / 10) * 4;
549:     uint32 shift = (m_pinNumber % 10) * 3;
550: 
551:     static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};
552: 
553:     uint32 value = m_memoryAccess.Read32(selectRegister);
554:     value &= ~(7 << shift);
555:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
556:     m_memoryAccess.Write32(selectRegister, value);
557:     m_function = function;
558: }
559: 
560: } // namespace baremetal
```

- Line 44: We need to include the header for the `GPIOManager`.
- Line 45: As we want to do logging we also add the include for that
- Line 52-53: We define the log name
- Line 57-72: We add an enum for the offset of the register for all the GPIO interrupt types.
As mentioned before the registers come in groups of 3, each taking 32 bits or 4 bytes
- Line 115-133: We update the 'default' constructor to initialize the new member variables
- Line 135-155: We also update the specific constructor to initialize the new member variables
- Line 389-408: We implement the method `ConnectInterrupt()`
  - Line 397: We verify that the GPIO pin is set to input mode
  - Line 399: We verify that no interrupt types are registered yet
  - Line 401-402: We verify that the handler passed is valid, and that no handler is set yet
  - Line 403-405: We save the handler function, its parameter and the auto acknowledge setting
  - Line 407: We connect the interrupt for the GPIO pin
- Line 410-422: We implement the method `DisconnectInterrupt()`
  - Line 415: We verify that the GPIO pin is set to input mode
  - Line 417: We remove all interrupt types for the GPIO
  - Line 419: We reset the handler
  - Line 421: We disconnect the interrupt for the GPIO pin
- Line 424-431: We implement the method `GetAutoAcknowledgeInterrupt()`
- Line 433-441: We implement the method `AcknowledgeInterrupt()`, resetting the event status bit for the GPIO
- Line 443-454: We implement the method `InterruptHandler()`
  - Line 449: We verify that the GPIO is set to input mode
  - Line 451: We verify there is a interrupt handler set
  - Line 452-453: We call the interrupthandler if on of the interrupt types is enabled
- Line 456-480: We convert a GPIO interrupt type to a register offset (relative to the rising edge interrupt register)
- Line 482-503: We update the method `EnableInterrupt()` to take a combination of GPIO interrupt types
  - Line 490-491: We convert the mask to byte and verify that only allowed bits are set
  - Line 492-502: We start with a pattern for `AsyncFallingEdge` (the highest), and for each pattern check whether the bit is set.
If so, we add it to the interrupt mask, calculate the register address, and set the corresponding bit in the interrupt enable register
- Line 505-524: We update the method `DisableInterrupt()` to take a combination of GPIO interrupt types
  - Line 511-512: We convert the mask to byte and verify that only allowed bits are set
  - Line 513-523: We start with a pattern for `AsyncFallingEdge` (the highest), and for each pattern check whether the bit is set.
If so, we remove it from the interrupt mask, calculate the register address, and reset the corresponding bit in the interrupt enable register
- Line 526-532: We update the method `DisableAllInterrupts()` to simply call `DisableInterrupt()` with all interrupt types combined

### InterruptHandler.cpp {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_INTERRUPTHANDLERCPP}

We'd like to see what is happening concerning registration and unregistration of IRQ and FIQ handlers.
Let's add this. We need to take into account however, that the `Logger` class needs the `Timer` class, which already registers for an IRQ while staring up.
So we need a way to check whether the `Logger` instance is already created.
We'll also add logging to the `Shutdown()` method, but we'll skip the `Initialize()` method, as that will definitely be called before the `Logger` is actually initialized.

Update the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
...
142: /// <summary>
143: /// Shutdown interrupt system, disable all
144: /// </summary>
145: void InterruptSystem::Shutdown()
146: {
147:     if (Logger::HaveLogger())
148:         LOG_INFO("InterruptSystem::Shutdown");
149:     DisableIRQs();
150: 
151:     DisableInterrupts();
152:     m_isInitialized = false;
153: }
154: 
155: /// <summary>
156: /// Disable all IRQ interrupts
157: /// </summary>
158: void InterruptSystem::DisableInterrupts()
159: {
160:     if (Logger::HaveLogger())
161:         LOG_DEBUG("InterruptSystem::DisableInterrupts");
162: #if BAREMETAL_RPI_TARGET == 3
163:     m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
164: 
165:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
166:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
167:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
168:     m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
169: #else
170:     // initialize distributor:
171: 
172:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
173:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_DISABLE);
174:     // disable, acknowledge and deactivate all interrupts
175:     for (unsigned n = 0; n < IRQ_LINES / 32; n++)
176:     {
177:         m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
178:         m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
179:         m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
180:     }
181: #endif
182: }
183: 
184: /// <summary>
185: /// Enable IRQ interrupts
186: /// </summary>
187: void InterruptSystem::EnableInterrupts()
188: {
189:     if (Logger::HaveLogger())
190:         LOG_DEBUG("InterruptSystem::EnableInterrupts");
191: #if BAREMETAL_RPI_TARGET == 3
192: #else
193:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
194:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
195: #endif
196: }
197: 
198: /// <summary>
199: /// Enable and register an IRQ handler
200: ///
201: /// Enable the IRQ with specified index, and register its handler.
202: /// </summary>
203: /// <param name="irqID">IRQ ID</param>
04: /// <param name="handler">Handler to register for this IRQ</param>
205: /// <param name="param">Parameter to pass to IRQ handler</param>
206: void InterruptSystem::RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param)
207: {
208:     uint32 irq = static_cast<int>(irqID);
209:     assert(irq < IRQ_LINES);
210:     if (Logger::HaveLogger())
211:         LOG_DEBUG("InterruptSystem::RegisterIRQHandler IRQ=%d", irq);
212:     assert(m_irqHandlers[irq] == nullptr);
213: 
214:     EnableIRQ(irqID);
215: 
216:     m_irqHandlers[irq] = handler;
217:     m_irqHandlersParam[irq] = param;
218: }
219: 
220: /// <summary>
221: /// Disable and unregister an IRQ handler
222: ///
223: /// Disable the IRQ with specified index, and unregister its handler.
224: /// </summary>
225: /// <param name="irqID">IRQ ID</param>
226: void InterruptSystem::UnregisterIRQHandler(IRQ_ID irqID)
227: {
228:     uint32 irq = static_cast<int>(irqID);
229:     assert(irq < IRQ_LINES);
230:     if (Logger::HaveLogger())
231:         LOG_DEBUG("InterruptSystem::UnregisterIRQHandler IRQ=%d", irq);
232:     assert(m_irqHandlers[irq] != nullptr);
233: 
234:     m_irqHandlers[irq] = nullptr;
235:     m_irqHandlersParam[irq] = nullptr;
236: 
237:     DisableIRQ(irqID);
238: }
239: 
240: /// <summary>
241: /// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
242: /// </summary>
243: /// <param name="fiqID">FIQ interrupt number</param>
244: /// <param name="handler">FIQ interrupt handler</param>
245: /// <param name="param">FIQ interrupt data</param>
246: // cppcheck-suppress unusedFunction
247: void InterruptSystem::RegisterFIQHandler(FIQ_ID fiqID, FIQHandler *handler, void *param)
248: {
249:     uint32 fiq = static_cast<int>(fiqID);
250:     assert(fiq <= IRQ_LINES);
251:     if (Logger::HaveLogger())
252:         LOG_DEBUG("InterruptSystem::RegisterFIQHandler IRQ=%d", fiq);
253:     assert(handler != nullptr);
254:     assert(s_fiqData.handler == nullptr);
255: 
256:     s_fiqData.handler = handler;
257:     s_fiqData.param   = param;
258:     s_fiqData.fiqID   = fiq;
259: 
260:     EnableFIQ(fiqID);
261: }
262: 
263: /// <summary>
264: /// Disable and unregister a FIQ interrupt handler
265: /// </summary>
266: /// <param name="fiqID">FIQ interrupt number</param>
267: void InterruptSystem::UnregisterFIQHandler(FIQ_ID fiqID)
268: {
269:     uint32 fiq = static_cast<int>(fiqID);
270:     assert(s_fiqData.handler != nullptr);
271:     assert(s_fiqData.fiqID == fiq);
272:     if (Logger::HaveLogger())
273:         LOG_DEBUG("InterruptSystem::UnregisterFIQHandler IRQ=%d", fiq);
274:     DisableFIQ(fiqID);
275: 
276:     s_fiqData.handler = nullptr;
277:     s_fiqData.param   = nullptr;
278: }
...
```

- Line 147-148: We update the method `Shutdown()` to log, but only when a logger is instantiated and initialized
- Line 160-161: We update the method `DisableInterrupts()` to log, but only when a logger is instantiated and initialized
- Line 189-190: We update the method `EnableInterrupts()` to log, but only when a logger is instantiated and initialized
- Line 210-211: We update the method `RegisterIRQHandler()` to log, but only when a logger is instantiated and initialized
- Line 230-231: We update the method `UnregisterIRQHandler()` to log, but only when a logger is instantiated and initialized
- Line 251-252: We update the method `RegisterFIQHandler()` to log, but only when a logger is instantiated and initialized
- Line 272-273: We update the method `UnregisterFIQHandler()` to log, but only when a logger is instantiated and initialized

### Update application code {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/InterruptHandler.h"
5: #include "baremetal/Interrupts.h"
6: #include "baremetal/Logger.h"
7: #include "baremetal/PhysicalGPIOPin.h"
8: #include "baremetal/System.h"
9: #include "baremetal/Timer.h"
10: 
11: LOG_MODULE("main");
12: 
13: using namespace baremetal;
14: 
15: void InterruptHandlerCLK(IGPIOPin* pin, void *param)
16: {
17:     LOG_DEBUG("GPIO CLK");
18:     if (pin->GetEvent())
19:     {
20:         auto value = pin->Get();
21:         LOG_DEBUG("CLK=%d", value);
22:         pin->ClearEvent();
23:     }
24: }
25: 
26: void InterruptHandlerDT(IGPIOPin* pin, void *param)
27: {
28:     LOG_DEBUG("GPIO DT");
29:     if (pin->GetEvent())
30:     {
31:         auto value = pin->Get();
32:         LOG_DEBUG("DT=%d", value);
33:         pin->ClearEvent();
34:     }
35: }
36: 
37: void InterruptHandlerSW(IGPIOPin* pin, void *param)
38: {
39:     LOG_DEBUG("GPIO SW");
40:     if (pin->GetEvent())
41:     {
42:         auto value = pin->Get();
43:         LOG_DEBUG("SW=%d", value);
44:         pin->ClearEvent();
45:     }
46: }
47: 
48: int main()
49: {
50:     auto& console = GetConsole();
51:     GetLogger().SetLogLevel(LogSeverity::Debug);
52: 
53:     auto exceptionLevel = CurrentEL();
54:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
55: 
56:     PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
57:     PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
58:     PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
59: 
60:     pinCLK.ConnectInterrupt(InterruptHandlerCLK, nullptr);
61:     pinCLK.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
62:     pinDT.ConnectInterrupt(InterruptHandlerDT, nullptr);
63:     pinDT.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
64:     pinSW.ConnectInterrupt(InterruptHandlerSW, nullptr);
65:     pinSW.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
66: 
67:     LOG_INFO("Wait 5 seconds");
68:     Timer::WaitMilliSeconds(5000);
69: 
70:     GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);
71: 
72:     pinCLK.DisconnectInterrupt();
73:     pinDT.DisconnectInterrupt();
74:     pinSW.DisconnectInterrupt();
75: 
76:     console.Write("Press r to reboot, h to halt\n");
77:     char ch{};
78:     while ((ch != 'r') && (ch != 'h'))
79:     {
80:         ch = console.ReadChar();
81:         console.WriteChar(ch);
82:     }
83: 
84:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
85: }
```

- Line 15-24: We add a separate GPIO interrupt handler for the CLK pin
- Line 26-35: We add a separate GPIO interrupt handler for the DT pin
- Line 37-46: We add a separate GPIO interrupt handler for the SW pin
- Line 60-61: We set up the interrupt handler for the CLK pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 62-63: We set up the interrupt handler for the DT pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 64-65: We set up the interrupt handler for the SW pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 72-74: We disconnect the interrupts using the new method, which goes through the `GPIOManager`

### Configuring, building and debugging {#TUTORIAL_21_GPIO_GENERIC_APPROACH_FOR_GPIO_INTERRUPTS___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

When we press the switch or turn it, you can see the interrupts coming in.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Current EL: 1 (main:54)
Debug  0.00:00:00.090 InterruptSystem::RegisterIRQHandler IRQ=52 (InterruptHandler:211)
Info   0.00:00:00.110 Wait 5 seconds (main:67)
Debug  0.00:00:00.700 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:448)
Debug  0.00:00:00.700 GPIO CLK (main:17)
Debug  0.00:00:00.700 CLK=0 (main:21)
Debug  0.00:00:00.760 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:448)
Debug  0.00:00:00.760 GPIO CLK (main:17)
Debug  0.00:00:00.760 CLK=1 (main:21)
Debug  0.00:00:00.820 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:448)
Debug  0.00:00:00.820 GPIO DT (main:28)
Debug  0.00:00:00.820 DT=1 (main:32)
Debug  0.00:00:01.170 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:448)
Debug  0.00:00:01.170 GPIO DT (main:28)
Debug  0.00:00:01.170 DT=1 (main:32)
Debug  0.00:00:01.230 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:448)
Debug  0.00:00:01.230 GPIO CLK (main:17)
Debug  0.00:00:01.230 CLK=1 (main:21)
Debug  0.00:00:01.670 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:448)
Debug  0.00:00:01.670 GPIO DT (main:28)
Debug  0.00:00:01.670 DT=0 (main:32)
Debug  0.00:00:01.730 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:448)
Debug  0.00:00:01.730 GPIO CLK (main:17)
Debug  0.00:00:01.730 CLK=1 (main:21)
Debug  0.00:00:01.790 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:448)
Debug  0.00:00:01.790 GPIO DT (main:28)
Debug  0.00:00:01.790 DT=1 (main:32)
Debug  0.00:00:04.010 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:448)
Debug  0.00:00:04.010 GPIO SW (main:39)
Debug  0.00:00:04.010 SW=0 (main:43)
Debug  0.00:00:04.190 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:448)
Debug  0.00:00:04.190 GPIO SW (main:39)
Debug  0.00:00:04.190 SW=1 (main:43)
Debug  0.00:00:04.610 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:448)
Debug  0.00:00:04.610 GPIO SW (main:39)
Debug  0.00:00:04.610 SW=0 (main:43)
Debug  0.00:00:04.790 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:448)
Debug  0.00:00:04.790 GPIO SW (main:39)
Debug  0.00:00:04.790 SW=1 (main:43)
Debug  0.00:00:05.130 InterruptSystem::UnregisterIRQHandler IRQ=52 (InterruptHandler:231)
Press r to reboot, h to halt
```

## Adding intelligence to the switch button - Step 5 {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5}

So, now we can read the rotary switch signals, we attached interrupts so we can handle changes.
Let's make the rotare switch a bit smarter.

We'll add a new class in a new library device `KY040` for this, and as a first step, we'll add the possibility to distinguish between several event of the switch button:

- Single press
- Press and hold
- Double press
- Tripple press

### Adding a new library {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_ADDING_A_NEW_LIBRARY}

Let's add a new library for specific hardware devices.
We'll create a new library `device`, and add an extra layer of directories to distignuish between e.g. GPIO and I2C devices.
For now we'll be adding a device underneath gpio:
- We create a directory libraries/device
- We create a directory libraries/device/include/device/gpio
- We create a directory libraries/device/src/gpio

The new library wil be next to `baremetal` and `stdlib`, and have the following structure:

<img src="images/treeview-device-library.png" alt="Initial project structure" width="300"/>

### CMake file for device library {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_CMAKE_FILE_FOR_DEVICE_LIBRARY}

We'll have to set up the CMake file for the new library.

Create the file `code/libraries/device/CMakeLists.txt`

```cmake
File: code/libraries/device/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: project(device
5:     DESCRIPTION "Bare metal device library"
6:     LANGUAGES CXX ASM)
7: 
8: set(PROJECT_TARGET_NAME ${PROJECT_NAME})
9: 
10: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
11: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
12: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
13: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
14: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
15: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
16: set(PROJECT_INCLUDE_DIRS_PRIVATE )
17: set(PROJECT_INCLUDE_DIRS_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
18: 
19: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
20: 
21: set(PROJECT_DEPENDENCIES
22:     baremetal
23:     )
24: 
25: set(PROJECT_LIBS
26:     ${LINKER_LIBRARIES}
27:     ${PROJECT_DEPENDENCIES}
28:     )
29: 
30: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp src/*.S)
31: set(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/*.h)
32: set(PROJECT_INCLUDES_PRIVATE )
33: 
34: set(PROJECT_INCLUDES_PRIVATE )
35: 
36: if (CMAKE_VERBOSE_MAKEFILE)
37:     display_list("Package                           : " ${PROJECT_NAME} )
38:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
39:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
40:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
41:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
42:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
43:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
44:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
45:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
46:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
47:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
48:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
49:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
50:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
51:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
52:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
53:     display_list("Link libs                         : " ${PROJECT_LIBS} )
54:     display_list("Source files                      : " ${PROJECT_SOURCES} )
55:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
56:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
57: endif()
58: 
59: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
60: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
61: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
62: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
63: target_compile_definitions(${PROJECT_NAME} PRIVATE
64:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
65:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
66:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
67:     )
68: target_compile_definitions(${PROJECT_NAME} PUBLIC
69:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
70:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
71:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
72:     )
73: target_compile_options(${PROJECT_NAME} PRIVATE
74:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
75:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
76:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
77:     )
78: target_compile_options(${PROJECT_NAME} PUBLIC
79:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
80:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
81:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
82:     )
83: 
84: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
85: 
86: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
87: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
88:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
89: endif()
90: 
91: link_directories(${LINK_DIRECTORIES})
92: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
93: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
94: 
95: show_target_properties(${PROJECT_NAME})
```

### Update CMake file for libraries {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_UPDATE_CMAKE_FILE_FOR_LIBRARIES}

We'll add the new library in the libraries CMake file.

Update the file `code/libraries/CMakeLists.txt`

```cmake
File: code/libraries/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: add_subdirectory(baremetal)
5: add_subdirectory(device)
6: add_subdirectory(stdlib)
```

### KY-040.h {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_KY_040H}

We'll add a class KY040 for the rotary switch.

Create the file `code/libraries/device/include/device/gpio/KY-040.h`

```cpp
File: code/libraries/device/include/device/gpio/KY-040.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : KY-040.h
5: //
6: // Namespace   : device
7: //
8: // Class       : KY040
9: //
10: // Description : KY-040 rotary encoder support
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
42: #include "baremetal/PhysicalGPIOPin.h"
43: #include "baremetal/Timer.h"
44: #include "stdlib/Types.h"
45: 
46: /// @file
47: /// KY-040 rotary switch support declaration.
48: 
49: /// @brief Device library namespace
50: namespace device {
51: 
52: enum class SwitchButtonEvent;
53: enum class SwitchButtonState;
54: 
55: /// <summary>
56: /// KY-040 rotary switch device
57: /// </summary>
58: class KY040
59: {
60: public:
61:     /// <summary>
62:     /// Events generated by the rotary switch
63:     /// </summary>
64:     enum class Event
65:     {
66:         /// @brief Switch is pressed
67:         SwitchDown,
68:         /// @brief Switch is released
69:         SwitchUp,
70:         /// @brief Switch is clicked (short press / release cycle)
71:         SwitchClick,
72:         /// @brief Switch is clicked twice in a short time
73:         SwitchDoubleClick,
74:         /// @brief Switch is clicked three time in a short time
75:         SwitchTripleClick,
76:         /// @brief Switch is held down for a longer time
77:         SwitchHold, ///< generated each second
78:         /// @brief Unknown event
79:         Unknown
80:     };
81: 
82:     /// <summary>
83:     /// Pointer to event handler function to be registered by an application
84:     /// </summary>
85:     using EventHandler = void(Event event, void* param);
86: 
87: private:
88:     /// @brief True if the rotary switch was initialized
89:     bool m_isInitialized;
90:     /// @brief GPIO pin for CLK input
91:     baremetal::PhysicalGPIOPin m_clkPin;
92:     /// @brief GPIO pin for DT input
93:     baremetal::PhysicalGPIOPin m_dtPin;
94:     /// @brief GPIO pin for SW input (switch button)
95:     baremetal::PhysicalGPIOPin m_swPin;
96:     /// @brief Internal state of the switch button (to tracking single, double, triple clicking and hold
97:     SwitchButtonState m_switchButtonState;
98:     /// @brief Handle to timer for debouncing the switch button
99:     baremetal::KernelTimerHandle m_debounceTimerHandle;
100:     /// @brief Handle to timer for handling button press ticks (for hold)
101:     baremetal::KernelTimerHandle m_tickTimerHandle;
102:     /// @brief Time at which the current button press occurred
103:     unsigned m_currentPressTicks;
104:     /// @brief Time at which the current button release occurred
105:     unsigned m_currentReleaseTicks;
106:     /// @brief Time at which the last button press occurred
107:     unsigned m_lastPressTicks;
108:     /// @brief Time at which the last button release occurred
109:     unsigned m_lastReleaseTicks;
110: 
111:     /// @brief Registered event handler
112:     EventHandler* m_eventHandler;
113:     /// @brief Parameter for registered event handler
114:     void* m_eventHandlerParam;
115: 
116: public:
117:     KY040(uint8 clkPin, uint8 dtPin, uint8 swPin);
118:     virtual ~KY040();
119: 
120:     void Initialize();
121:     void Uninitialize();
122: 
123:     void RegisterEventHandler(EventHandler* handler, void* param);
124:     void UnregisterEventHandler(EventHandler* handler);
125:     static const char* EventToString(Event event);
126: 
127: private:
128:     static void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin, void* param);
129:     void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin);
130:     static void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void* param, void* context);
131:     void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void* param);
132:     static void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void* param, void* context);
133:     void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void* param);
134:     void HandleSwitchButtonEvent(SwitchButtonEvent switchEvent);
135: };
136: 
137: } // namespace device
```

Notice that the class is declared in the `device` namespace, which we will use for all classes in the device library.

- Line 52: We forward declare an enum `SwitchButtonEvent`, which will hold the internal switch button event (up, down, click, doubleclick, tick in case of holding the button down)
- Line 53: We forward declare an enum `SwitchButtonState`, which will hold the internal switch button state (which keeps track of the state machine for the button switch)
- Line 55-135: We declare the class `KY040`
  - Line 61-80: We declare the enum `Event` which will hold the event actually sent to a registered event handler (down, up, click, double click, triple click, hold)
  - Line 82-85: We declare the type for the event handler that can be registered
  - Line 88-89: We declare the member variable `m_isInitialized` to guard against multiple initialization
  - Line 90-91: We declare the member variable `m_clkPin` which is the GPIO pin for the CLK GPIO
  - Line 92-93: We declare the member variable `m_dtPin` which is the GPIO pin for the DT GPIO
  - Line 94-95: We declare the member variable `m_swPin` which is the GPIO pin for the SW GPIO
  - Line 96-97: We declare the member variable `m_switchButtonState` which is the internal switch button state
  - Line 98-99: We declare the member variable `m_debounceTimerHandle` which is a handle to the time to perform debouncing of the switch button
  - Line 100-101: We declare the member variable `m_tickTimerHandle` which is a handle to the hold tick timer
  - Line 102-103: We declare the member variable `m_currentPressTicks` which is the time the current switch button press happened (in timer ticks)
  - Line 104-105: We declare the member variable `m_currentReleaseTicks` which is the time the current switch button release happened (in timer ticks).
This is used to detect clicks
  - Line 106-107: We declare the member variable `m_lastPressTicks` which is the time the last switch button press happened (in timer ticks).
This is used to detect double and triple clicks
  - Line 108-109: We declare the member variable `m_lastReleaseTicks` which is the time the last switch button release happened (in timer ticks)
  - Line 111-112: We declare the member variable `m_eventHandler` which is the registered event handler
  - Line 113-114: We declare the member variable `m_eventHandlerParam` which is the parameter to be passed to the event handler
  - Line 117: We declare the constructor, which takes GPIO pin numbers for the CLK, DT and SW pin respectively
  - Line 118: We declare the destructor
  - Line 120: We declare the method `Initialize()` to initialize the rotary switch
  - Line 121: We declare the method `Uninitialize()` to uninitialize the rotary switch
  - Line 123: We declare the method `RegisterEventHandler()` to register an event handler
  - Line 124: We declare the method `UnregisterEventHandler()` to unregister an event handler
  - Line 125: We declare the method `EventToString()` to convert an event to a string
  - Line 128: We declare the static private method `SwitchButtonInterruptHandler()` which is the GPIO pin interrupt handler.
As the parameter will point to the class instance, it will call the method `SwitchButtonInterruptHandler()`
  - Line 129: We declare the private method `SwitchButtonInterruptHandler()` to handle debouncing of the switch button
  - Line 130: We declare the static private method `SwitchButtonDebounceHandler()` which is the timer timeout handler for switch button debouncing.
As the context parameter will point to the class instance, it will call the method `SwitchButtonDebounceHandler()`
  - Line 131: We declare the private method `SwitchButtonDebounceHandler()` to handle debouncing of the switch button
  - Line 132: We declare the static private method `SwitchButtonTickHandler()` which is the timer timeout handler for switch button holding.
As the context parameter will point to the class instance, it will call the method `SwitchButtonTickHandler()`
  - Line 133: We declare the private method `SwitchButtonTickHandler()` to handle holding down the switch button
  - Line 134: We declare the private method `HandleSwitchButtonEvent()` which deals with internal state keeping

### KY-040.cpp {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_KY_040CPP}

We'll implement the class KY040.

Create the file `code/libraries/device/src/gpio/KY-040.cpp`

```cpp
File: code/libraries/device/src/gpio/KY-040.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : KY-040.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : KY040
9: //
10: // Description : KY-040 rotary encoder support
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
40: #include <device/gpio/KY-040.h>
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: 
45: /// @brief Define log name
46: LOG_MODULE("KY-040");
47: 
48: using namespace baremetal;
49: 
50: namespace device {
51: 
52: /// @brief Time delay for debounding switch button
53: static const unsigned SwitchDebounceDelayMilliseconds = 50;
54: /// @brief Tick delay for determining if switch button was held down
55: static const unsigned SwitchTickDelayMilliseconds = 1000;
56: /// @brief Maximum delay between press and release for a click
57: static const unsigned SwitchClickMaxDelayMilliseconds = 400;
58: /// @brief Maximum delay between two presses for a double click (or triple click)
59: static const unsigned SwitchDoubleClickMaxDelayMilliseconds = 800;
60: 
61: /// <summary>
62: /// Switch button internal event
63: /// </summary>
64: enum class SwitchButtonEvent
65: {
66:     /// @brief Switch button is down
67:     Down,
68:     /// @brief Switch button is up
69:     Up,
70:     /// @brief Switch button is clicked
71:     Click,
72:     /// @brief Switch button is double clicked
73:     DblClick,
74:     /// @brief Switch button is held down for at least SwitchTickDelayMilliseconds milliseconds
75:     Tick,
76:     /// @brief Unknown event
77:     Unknown
78: };
79: 
80: /// <summary>
81: /// Switch button internal state
82: /// </summary>
83: enum class SwitchButtonState
84: {
85:     /// @brief Initial state
86:     Start,
87:     /// @brief Switch button is down
88:     Down,
89:     /// @brief Switch button is clicked
90:     Click,
91:     /// @brief Switch button is down for the second time within SwitchDoubleClickMaxDelayMilliseconds milliseconds
92:     Click2,
93:     /// @brief Switch button is down for the third time, since second time within SwitchDoubleClickMaxDelayMilliseconds milliseconds
94:     Click3,
95:     /// @brief Switch button is held down
96:     Hold,
97:     /// @brief Invalid state
98:     Invalid,
99:     /// @brief Unknown state
100:     Unknown
101: };
102: 
103: /// <summary>
104: /// Convert rotary switch event to a string
105: /// </summary>
106: /// <param name="event">Event type</param>
107: /// <returns>String representing event</returns>
108: const char* KY040::EventToString(KY040::Event event)
109: {
110:     switch (event)
111:     {
112:     case KY040::Event::SwitchDown:
113:         return "SwitchDown";
114:     case KY040::Event::SwitchUp:
115:         return "SwitchUp";
116:     case KY040::Event::SwitchClick:
117:         return "SwitchClick";
118:     case KY040::Event::SwitchDoubleClick:
119:         return "SwitchDoubleClick";
120:     case KY040::Event::SwitchTripleClick:
121:         return "SwitchTripleClick";
122:     case KY040::Event::SwitchHold:
123:         return "SwitchHold";
124:     case KY040::Event::Unknown:
125:     default:
126:         break;
127:     }
128:     return "Unknown";
129: }
130: 
131: /// <summary>
132: /// Convert internal switch button event to a string
133: /// </summary>
134: /// <param name="event">Event button event</param>
135: /// <returns>String representing event</returns>
136: static const char* SwitchButtonEventToString(SwitchButtonEvent event)
137: {
138:     switch (event)
139:     {
140:     case SwitchButtonEvent::Down:
141:         return "Down";
142:     case SwitchButtonEvent::Up:
143:         return "Up";
144:     case SwitchButtonEvent::Click:
145:         return "Click";
146:     case SwitchButtonEvent::DblClick:
147:         return "DblClick";
148:     case SwitchButtonEvent::Tick:
149:         return "Tick";
150:     case SwitchButtonEvent::Unknown:
151:     default:
152:         break;
153:     }
154:     return "Unknown";
155: }
156: 
157: /// <summary>
158: /// Convert internal switch button state to a string
159: /// </summary>
160: /// <param name="event">Event button state</param>
161: /// <returns>String representing state</returns>
162: static const char* SwitchButtonStateToString(SwitchButtonState state)
163: {
164:     switch (state)
165:     {
166:     case SwitchButtonState::Start:
167:         return "Start";
168:     case SwitchButtonState::Down:
169:         return "Down";
170:     case SwitchButtonState::Click:
171:         return "Click";
172:     case SwitchButtonState::Click2:
173:         return "Click2";
174:     case SwitchButtonState::Click3:
175:         return "Click3";
176:     case SwitchButtonState::Hold:
177:         return "Hold";
178:     case SwitchButtonState::Invalid:
179:         return "Invalid";
180:     case SwitchButtonState::Unknown:
181:     default:
182:         break;
183:     }
184:     return "Unknown";
185: }
186: 
187: /// <summary>
188: /// Event lookup for handling switch button state versus switch button event
189: ///
190: /// Every row signifies a beginning internalstate, every column signifies an internal event, values in the table determine the resulting event
191: /// </summary>
192: static const KY040::Event s_switchOutput[static_cast<size_t>(SwitchButtonState::Unknown)][static_cast<size_t>(SwitchButtonEvent::Unknown)] = {
193:     // {Down,               Up,                    Click,                           DoubleClick,                     Tick}
194: 
195:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::Unknown          }, // SwitchButtonState::Start
196:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchHold       }, // SwitchButtonState::Down
197:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchClick      }, // SwitchButtonState::Click
198:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick}, // SwitchButtonState::Click2
199:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick}, // SwitchButtonState::Click3
200:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::SwitchHold       }, // SwitchButtonState::Hold
201:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::Unknown          }  // SwitchButtonState::Invalid
202: };
203: /// <summary>
204: /// Determine the event for the current internal state and internal event
205: /// </summary>
206: /// <param name="state">Current internal state</param>
207: /// <param name="event">Current internal event</param>
208: /// <returns>Resulting event</returns>
209: static KY040::Event GetSwitchOutput(SwitchButtonState state, SwitchButtonEvent event)
210: {
211:     return s_switchOutput[static_cast<size_t>(state)][static_cast<size_t>(event)];
212: }
213: 
214: /// <summary>
215: /// State machine for handling switch button state
216: ///
217: /// Every row signifies a beginning internalstate, every column signifies an internal event, values in the table determine the resulting internal state
218: /// </summary>
219: static const SwitchButtonState s_nextSwitchState[static_cast<size_t>(SwitchButtonState::Unknown)][static_cast<size_t>(SwitchButtonEvent::Unknown)] = {
220:     // {Down,              Up,                  Click,               DoubleClick,          Tick}
221: 
222:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Start  }, // SwitchButtonState::Start
223:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Hold   }, // SwitchButtonState::Down
224:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Invalid}, // SwitchButtonState::Click
225:     {SwitchButtonState::Down, SwitchButtonState::Click2, SwitchButtonState::Click2, SwitchButtonState::Click3, SwitchButtonState::Hold   }, // SwitchButtonState::Click2
226:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click3, SwitchButtonState::Click3, SwitchButtonState::Hold   }, // SwitchButtonState::Click3
227:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Hold   }, // SwitchButtonState::Hold
228:     {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Invalid}  // SwitchButtonState::Invalid
229: };
230: /// <summary>
231: /// Determine the next internal state for the current internal state and internal event
232: /// </summary>
233: /// <param name="state">Current internal state</param>
234: /// <param name="event">Current internal event</param>
235: /// <returns>Resulting state</returns>
236: static SwitchButtonState GetSwitchNextState(SwitchButtonState state, SwitchButtonEvent event)
237: {
238:     return s_nextSwitchState[static_cast<size_t>(state)][static_cast<size_t>(event)];
239: }
240: 
241: /// <summary>
242: /// Constructor for KY040 class
243: /// </summary>
244: /// <param name="clkPin">GPIO pin number for CLK input</param>
245: /// <param name="dtPin">GPIO pin number for DT input</param>
246: /// <param name="swPin">GPIO pin number for SW input</param>
247: KY040::KY040(uint8 clkPin, uint8 dtPin, uint8 swPin)
248:     : m_isInitialized{}
249:     , m_clkPin(clkPin, GPIOMode::InputPullUp)
250:     , m_dtPin(dtPin, GPIOMode::InputPullUp)
251:     , m_swPin(swPin, GPIOMode::InputPullUp)
252:     , m_switchButtonState{SwitchButtonState::Start}
253:     , m_debounceTimerHandle{}
254:     , m_tickTimerHandle{}
255:     , m_currentPressTicks{}
256:     , m_currentReleaseTicks{}
257:     , m_lastPressTicks{}
258:     , m_lastReleaseTicks{}
259: 
260:     , m_eventHandler{}
261:     , m_eventHandlerParam{}
262: {
263:     LOG_DEBUG("KY040 constructor");
264: }
265: 
266: /// <summary>
267: /// Destructor for KY040 class
268: /// </summary>
269: KY040::~KY040()
270: {
271:     LOG_DEBUG("KY040 destructor");
272:     Uninitialize();
273: }
274: 
275: /// <summary>
276: /// Initialize the KY040 rotary switch
277: /// </summary>
278: void KY040::Initialize()
279: {
280:     if (m_isInitialized)
281:         return;
282: 
283:     LOG_DEBUG("KY040 Initialize");
284:     m_swPin.ConnectInterrupt(SwitchButtonInterruptHandler, this);
285: 
286:     m_swPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
287: 
288:     m_isInitialized = true;
289: }
290: 
291: /// <summary>
292: /// Uninitialize the KY040 rotary switch
293: /// </summary>
294: void KY040::Uninitialize()
295: {
296:     if (m_isInitialized)
297:     {
298:         LOG_DEBUG("Disconnect SW pin");
299:         m_swPin.DisableAllInterrupts();
300:         m_swPin.DisconnectInterrupt();
301:         m_isInitialized = false;
302:     }
303:     if (m_debounceTimerHandle)
304:     {
305:         GetTimer().CancelKernelTimer(m_debounceTimerHandle);
306:     }
307:     if (m_tickTimerHandle)
308:     {
309:         GetTimer().CancelKernelTimer(m_tickTimerHandle);
310:     }
311: }
312: 
313: /// <summary>
314: /// Register an event handler for the rotary switch
315: /// </summary>
316: /// <param name="handler">Handler function</param>
317: /// <param name="param">Parameter to pass to handler function</param>
318: void KY040::RegisterEventHandler(EventHandler* handler, void* param)
319: {
320:     assert(!m_eventHandler);
321:     m_eventHandler = handler;
322:     assert(m_eventHandler);
323:     m_eventHandlerParam = param;
324: }
325: 
326: /// <summary>
327: /// Unregister event handler for the rotary switch
328: /// </summary>
329: /// <param name="handler">Handler function</param>
330: void KY040::UnregisterEventHandler(EventHandler* handler)
331: {
332:     assert(m_eventHandler = handler);
333:     m_eventHandler = nullptr;
334:     m_eventHandlerParam = nullptr;
335: }
336: 
337: /// <summary>
338: /// Global GPIO pin interrupt handler for the switch button
339: /// </summary>
340: /// <param name="pin">GPIO pin for the button switch</param>
341: /// <param name="param">Parameter for the interrupt handler, which is a pointer to the class instance</param>
342: void KY040::SwitchButtonInterruptHandler(IGPIOPin* pin, void* param)
343: {
344:     KY040* pThis = reinterpret_cast<KY040*>(param);
345:     assert(pThis != nullptr);
346:     pThis->SwitchButtonInterruptHandler(pin);
347: }
348: 
349: /// <summary>
350: /// GPIO pin interrupt handlerthe switch button
351: /// </summary>
352: /// <param name="pin">GPIO pin for the button switch</param>
353: void KY040::SwitchButtonInterruptHandler(IGPIOPin* pin)
354: {
355:     LOG_DEBUG("KY040 SwitchButtonInterruptHandler");
356:     assert(pin != nullptr);
357: 
358:     /// Get Switch state (false = pressed, true = released)
359:     bool swValue = pin->Get();
360:     if (swValue)
361:     {
362:         m_currentReleaseTicks = GetTimer().GetTicks();
363:     }
364:     else
365:     {
366:         m_currentPressTicks = GetTimer().GetTicks();
367:     }
368: 
369:     if (m_debounceTimerHandle)
370:     {
371:         LOG_DEBUG("KY040 Cancel debounce timer");
372:         GetTimer().CancelKernelTimer(m_debounceTimerHandle);
373:     }
374: 
375:     LOG_DEBUG("KY040 Start debounce timer");
376:     m_debounceTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchDebounceDelayMilliseconds), SwitchButtonDebounceHandler, nullptr, this);
377: }
378: 
379: /// <summary>
380: /// Global switch button debounce handler, called by the switch button debounce timer on timeout
381: ///
382: /// Will call the class internal switch button debounce handler
383: /// </summary>
384: /// <param name="handle">Kernel timer handle</param>
385: /// <param name="param">Timer handler parameter</param>
386: /// <param name="context">Timer handler context</param>
387: void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void* param, void* context)
388: {
389:     KY040* pThis = reinterpret_cast<KY040*>(context);
390:     assert(pThis != nullptr);
391:     pThis->SwitchButtonDebounceHandler(handle, param);
392: }
393: 
394: /// <summary>
395: /// Switch button debounce handler, called by the global switch button debounce handler on timeout
396: /// </summary>
397: /// <param name="handle">Kernel timer handle</param>
398: /// <param name="param">Timer handler parameter</param>
399: void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void* param)
400: {
401:     LOG_DEBUG("KY040 Timeout debounce timer");
402:     m_debounceTimerHandle = 0;
403: 
404:     bool swValue = m_swPin.Get();
405:     auto event = swValue ? Event::SwitchUp : Event::SwitchDown;
406:     auto switchButtonEvent = swValue ? SwitchButtonEvent::Up : SwitchButtonEvent::Down;
407:     if (swValue)
408:     {
409:         if (m_currentReleaseTicks - m_lastPressTicks < MSEC2TICKS(SwitchClickMaxDelayMilliseconds))
410:         {
411:             switchButtonEvent = SwitchButtonEvent::Click;
412:         }
413:     }
414:     else
415:     {
416:         if (m_currentPressTicks - m_lastPressTicks < MSEC2TICKS(SwitchDoubleClickMaxDelayMilliseconds))
417:         {
418:             switchButtonEvent = SwitchButtonEvent::DblClick;
419:         }
420:     }
421:     LOG_DEBUG("KY040 SW                : %d", swValue);
422:     LOG_DEBUG("KY040 SW LastDown       : %d", m_lastPressTicks);
423:     LOG_DEBUG("KY040 SW LastUp         : %d", m_lastReleaseTicks);
424:     LOG_DEBUG("KY040 SW CurrentDown    : %d", m_currentPressTicks);
425:     LOG_DEBUG("KY040 SW CurrentUp      : %d", m_currentReleaseTicks);
426:     if (swValue)
427:     {
428:         m_lastReleaseTicks = m_currentReleaseTicks;
429:     }
430:     else
431:     {
432:         m_lastPressTicks = m_currentPressTicks;
433:     }
434: 
435:     LOG_DEBUG("KY040 Event             : %s", EventToString(event));
436:     LOG_DEBUG("KY040 Switch Event      : %s", SwitchButtonEventToString(switchButtonEvent));
437:     if (m_eventHandler)
438:     {
439:         (*m_eventHandler)(event, m_eventHandlerParam);
440:     }
441: 
442:     if (m_tickTimerHandle)
443:     {
444:         GetTimer().CancelKernelTimer(m_tickTimerHandle);
445:     }
446: 
447:     if (!swValue) // If pressed, check for hold
448:         m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchButtonTickHandler, nullptr, this);
449: 
450:     HandleSwitchButtonEvent(switchButtonEvent);
451: }
452: 
453: /// <summary>
454: /// Global switch button tick handler, called by the switch button tick timer on timeout
455: ///
456: /// Will call the class internal switch button tick handler
457: /// </summary>
458: /// <param name="handle">Kernel timer handle</param>
459: /// <param name="param">Timer handler parameter</param>
460: /// <param name="context">Timer handler context</param>
461: void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void* param, void* context)
462: {
463:     KY040* pThis = reinterpret_cast<KY040*>(context);
464:     assert(pThis != nullptr);
465: 
466:     pThis->SwitchButtonTickHandler(handle, param);
467: }
468: 
469: /// <summary>
470: /// Switch button tick handler, called by the global switch button tick handler on timeout
471: /// </summary>
472: /// <param name="handle">Kernel timer handle</param>
473: /// <param name="param">Timer handler parameter</param>
474: void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void* param)
475: {
476:     LOG_DEBUG("KY040 Timeout tick timer");
477:     // Timer timed out, so we need to generate a tick
478:     m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchButtonTickHandler, nullptr, this);
479: 
480:     HandleSwitchButtonEvent(SwitchButtonEvent::Tick);
481: }
482: 
483: /// <summary>
484: /// Handle a switch button event
485: ///
486: /// Updates the internal state of switch button, and generates the proper event
487: /// </summary>
488: /// <param name="switchButtonEvent">Internal switch button event</param>
489: void KY040::HandleSwitchButtonEvent(SwitchButtonEvent switchButtonEvent)
490: {
491:     assert(switchButtonEvent < SwitchButtonEvent::Unknown);
492: 
493:     LOG_DEBUG("KY040 Current state     : %s", SwitchButtonStateToString(m_switchButtonState));
494:     LOG_DEBUG("KY040 Switch Event      : %s", SwitchButtonEventToString(switchButtonEvent));
495:     Event event = GetSwitchOutput(m_switchButtonState, switchButtonEvent);
496:     SwitchButtonState nextState = GetSwitchNextState(m_switchButtonState, switchButtonEvent);
497: 
498:     LOG_DEBUG("KY040 Event             : %s", EventToString(event));
499:     LOG_DEBUG("KY040 Next state        : %s", SwitchButtonStateToString(nextState));
500: 
501:     m_switchButtonState = nextState;
502: 
503:     if ((event != Event::Unknown) && (m_eventHandler != nullptr))
504:     {
505:         (*m_eventHandler)(event, m_eventHandlerParam);
506:     }
507: }
508: 
509: } // namespace device
```

- Line 52-53: We define the timeout value to be used for debouncing in milliseconds
- Line 54-55: We define the timeout value to be used for detecting holding down the switch button in milliseconds
- Line 56-57: We define the timeout value to be used for detecting clicks (short push/release cycle) in milliseconds
- Line 58-59: We define the timeout value to be used for detecting double and triple clicks in milliseconds, as the time between two consecutive switch push downs
- Line 61-78: We declare the `SwitchButtonEvent` enum which is used to keep track of events internally
- Line 80-101: We declare the `SwitchButtonState` enum which is used to keep track of the switch state internally
- Line 103-129: We implement the method `EventToString()`
- Line 131-155: We implement a local function `SwitchButtonEventToString()` to convert the internal event to a string, for debugging
- Line 157-185: We implement a local function `SwitchButtonStateToString()` to convert the internal switch button state to a string, for debugging
- Line 187-202: We define a matrix `s_switchOutput` to determine the event to be generated when an event happens in a certain state. This is part of the state machine.
I'll not go into details here, I'll leave it to you to figure this out
- Line 203-212: We implement a local function `GetSwitchOutput()` which uses the `s_switchOutput` variable to determine the event to generate
- Line 214-229: We define a matrix `s_nextSwitchState` to determine the next state when an event happens in a certain state. This is part of the state machine.
Again, I'll not go into details here, I'll leave it to you to figure this out
- Line 230-239: We implement a local function `GetSwitchNextState()` which uses the `s_nextSwitchState` variable to determine the next state
- Line 241-264: We implement the constructor. This is quite straightforward
- Line 266-273: We implement the desctructor. This is quite straightforward
- Line 275-289: We implement the `Initialize()` method.
For now it only connects the interrupt for the SW GPIO, for both rising and falling edges
- Line 291-311: We implement the `Uninitialize()` method.
This disconnects the interrupt, and cancels any running timers
- Line 313-324: We implement the `RegisterEventHandler` method. This is quite straightforward
- Line 326-335: We implement the `UnregisterEventHandler` method. This is quite straightforward
- Line 337-347: We implement the global `SwitchButtonInterruptHandler()` method, which is the global interrupt handler for the SW GPIO pin.
It converts the parameter to a class pointer, and then calls the class method
- Line 349-377: We implement the `SwitchButtonInterruptHandler()` method, which is the class interrupt handler for the SW GPIO pin
  - Line 359: It reads the GPIO pin value (down is false, up is true)
  - Line 361-363: If the switch button is up, we set the release time
  - Line 365-367: If the switch button is down, we set the press time
  - Line 369-373: If there was already a debounce timer running, we cancel it
  - Line 375-376: We start a new debounce timer
- Line 379-392: We implement the global `SwitchButtonDebounceHandler()` method, which is the global debounce timer timeout handler.
It converts the parameter to a class pointer, and then calls the class method
- Line 394-451: We implement the `SwitchButtonDebounceHandler()` method, which is the class debounce timer timeout handler
  - Line 404-406: We determin the GPIO pin value, set the event to either `SwitchUp` or `SwitchDown`, and similarly set the internal event to `Up` or `Down`
  - Line 408-413: If the button is up, we check the time elapsed between the press and the release, if this is within the click time, we change the internal event to `Click`
  - Line 415-420: If the button is down, we check the time elapsed between the press and the previous press, if this is within the double click time, we change the internal event to `DblClick`
  - Line 421-425: We print debug output on switch button state and timing
  - Line 427-429: If the button is up, we save the last release time
  - Line 431-433: Else we save the last press time
  - Line 435-436: We print debug output on switch button state and timing
  - Line 437-440: If an event handler is registered, we call it
  - Line 442-445: If a tick timer was running, we cancel it
  - Line 447-448: if the button is down, we start a time to check for holding down the button
  - Line 450: We call the method `HandleSwitchButtonEvent()` to handle the internal state and generate a possible event
- Line 453-467: We implement the global `SwitchButtonTickHandler()` method, which is the global tick timer timeout handler.
It converts the parameter to a class pointer, and then calls the class method
- Line 469-481: We implement the `SwitchButtonTickHandler()` method, which is the class tick timer timeout handler
  - Line 478: We start another tick timer as we will repeat the event as long as the button is held down
  - Line 480: We call the method `HandleSwitchButtonEvent()` to handle the internal state and generate a possible event
- Line 483-507: We implement the `HandleSwitchButtonEvent()` method, which handles the state machine
  - Line 491: We verify we have a valid event
  - Line 493-494: We print debug info
  - Line 495: We determine the output event from the current internal event and the internal state
  - Line 496: We determine the new internal state from the current internal event and the internal state
  - Line 498-499: We print debug info
  - Line 501: We save the next state
  - Line 503-506: If the output event is valid, and a event handler is registered, we call the event handler

### Update application code {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_UPDATE_APPLICATION_CODE}

We'll create a rotary switch in the application and display its event callback.
Due to the amount of debug output, we'll change the log level to `Info`. We'll also keep waiting a bit longer to allow for interaction with the rotary switch.
Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/Logger.h"
5: #include "baremetal/System.h"
6: #include "baremetal/Timer.h"
7: #include <device/gpio/KY-040.h>
8:
9: LOG_MODULE("main");
10:
11: using namespace baremetal;
12: using namespace device;
13:
14: void OnEvent(KY040::Event event, void *param)
15: {
16:     LOG_INFO("Event %s", KY040::EventToString(event));
17: }
18:
19: int main()
20: {
21:     auto& console = GetConsole();
22:     GetLogger().SetLogLevel(LogSeverity::Info);
23:
24:     auto exceptionLevel = CurrentEL();
25:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
26:
27:     KY040 rotarySwitch(11, 9, 10);
28:     rotarySwitch.Initialize();
29:     rotarySwitch.RegisterEventHandler(OnEvent, nullptr);
30:
31:     LOG_INFO("Wait 20 seconds");
32:     Timer::WaitMilliSeconds(20000);
33:
34:     rotarySwitch.UnregisterEventHandler(OnEvent);
35:
36:     console.Write("Press r to reboot, h to halt\n");
37:     char ch{};
38:     while ((ch != 'r') && (ch != 'h'))
39:     {
40:         ch = console.ReadChar();
41:         console.WriteChar(ch);
42:     }
43:
44:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
45: }
```

### Update application CMake file {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_UPDATE_APPLICATION_CMAKE_FILE}

As we are now using the `device` library, we need to add it to the application dependencies.

Update the file `code/applications/demo/CMakeLists.txt`

```cmake
File: code/applications/demo/CMakeLists.txt
27: set(PROJECT_DEPENDENCIES
28:     device
29:     baremetal
30:     )
31:
```

### Configuring, building and debugging {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice the click, double click, triple click and hold events

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Current EL: 1 (main:25)
Info   0.00:00:00.090 Wait 20 seconds (main:31)
Info   0.00:00:00.670 Event SwitchDown (main:16)
Info   0.00:00:00.670 Event SwitchDoubleClick (main:16)
Info   0.00:00:00.910 Event SwitchUp (main:16)
Info   0.00:00:00.910 Event SwitchDoubleClick (main:16)
Info   0.00:00:01.480 Event SwitchDown (main:16)
Info   0.00:00:01.640 Event SwitchUp (main:16)
Info   0.00:00:01.640 Event SwitchClick (main:16)
Info   0.00:00:01.800 Event SwitchDown (main:16)
Info   0.00:00:01.800 Event SwitchDoubleClick (main:16)
Info   0.00:00:01.950 Event SwitchUp (main:16)
Info   0.00:00:01.950 Event SwitchDoubleClick (main:16)
Info   0.00:00:02.490 Event SwitchDown (main:16)
Info   0.00:00:02.490 Event SwitchDoubleClick (main:16)
Info   0.00:00:02.640 Event SwitchUp (main:16)
Info   0.00:00:02.640 Event SwitchTripleClick (main:16)
Info   0.00:00:02.760 Event SwitchDown (main:16)
Info   0.00:00:02.760 Event SwitchTripleClick (main:16)
Info   0.00:00:03.080 Event SwitchUp (main:16)
Info   0.00:00:03.190 Event SwitchDown (main:16)
Info   0.00:00:03.190 Event SwitchDoubleClick (main:16)
Info   0.00:00:03.310 Event SwitchUp (main:16)
Info   0.00:00:03.310 Event SwitchDoubleClick (main:16)
Info   0.00:00:03.430 Event SwitchDown (main:16)
Info   0.00:00:03.430 Event SwitchDoubleClick (main:16)
Info   0.00:00:03.540 Event SwitchUp (main:16)
Info   0.00:00:03.540 Event SwitchTripleClick (main:16)
Info   0.00:00:03.630 Event SwitchDown (main:16)
Info   0.00:00:03.630 Event SwitchTripleClick (main:16)
Info   0.00:00:03.760 Event SwitchUp (main:16)
Info   0.00:00:03.760 Event SwitchTripleClick (main:16)
Info   0.00:00:04.880 Event SwitchDown (main:16)
Info   0.00:00:05.030 Event SwitchUp (main:16)
Info   0.00:00:05.030 Event SwitchClick (main:16)
Info   0.00:00:05.560 Event SwitchDown (main:16)
Info   0.00:00:05.560 Event SwitchDoubleClick (main:16)
Info   0.00:00:06.560 Event SwitchDoubleClick (main:16)
Info   0.00:00:07.560 Event SwitchHold (main:16)
Info   0.00:00:08.560 Event SwitchHold (main:16)
Info   0.00:00:09.560 Event SwitchHold (main:16)
Info   0.00:00:10.560 Event SwitchHold (main:16)
Info   0.00:00:11.250 Event SwitchUp (main:16)
Press r to reboot, h to halt
```

## Adding intelligence to the rotary switch - Step 6 {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6}

Now let's also make the rotate part a bit smarter.

We'll want to distinguish between a clockwise and a counter-clockwise rotation, and we will need to handle fast turning which may skip an event here and there.

We'll start each pattern with the default situation, where both signals are high.

When turning clockwise, the expected pattern is:

| CLK | DT |
|-----|----|
| 1   | 1  |
| 0   | 1  |
| 0   | 0  |
| 1   | 0  |
| 1   | 1  |

When turning counter-clockwise, the expected pattern is:

| CLK | DT |
|-----|----|
| 1   | 1  |
| 1   | 0  |
| 0   | 0  |
| 0   | 1  |
| 1   | 1  |

We'll use another state machine to keep track of the rotary status:

| State          | CLK | DT | New state         | Event            |
|----------------|-----|----|-------------------|------------------|
| Start          | 0   | 0  | Invalid           | -                |
| Start          | 0   | 1  | CW Start          | -                |
| Start          | 1   | 0  | CCW Start         | -                |
| Start          | 1   | 1  | Start             | -                |
| CW Start       | 0   | 0  | CW Data Fall      | -                |
| CW Start       | 0   | 1  | CW Start          | -                |
| CW Start       | 1   | 0  | CW Clock Rise (*) | -                |
| CW Start       | 1   | 1  | Start (*)         | -                |
| CW Data Fall   | 0   | 0  | CW Data Fall      | -                |
| CW Data Fall   | 0   | 1  | CW Start (*)      | -                |
| CW Data Fall   | 1   | 0  | CW Clock Rise     | -                |
| CW Data Fall   | 1   | 1  | Invalid           | -                |
| CW Clock Rise  | 0   | 0  | CW Data Fall (*)  | -                |
| CW Clock Rise  | 0   | 1  | Invalid           | -                |
| CW Clock Rise  | 1   | 0  | CW Clock Rise     | -                |
| CW Clock Rise  | 1   | 1  | Start (**)        | Clockwise        |
| CCW Start      | 0   | 0  | CCW Clock Fall    | -                |
| CCW Start      | 0   | 1  | CCW Data Rise (*) | -                |
| CCW Start      | 1   | 0  | CCW Start         | -                |
| CCW Start      | 1   | 1  | Start (*)         | -                |
| CCW Clock Fall | 0   | 0  | CCW Clock Fall    | -                |
| CCW Clock Fall | 0   | 1  | CCW Data Rise     | -                |
| CCW Clock Fall | 1   | 0  | CCW Start (*)     | -                |
| CCW Clock Fall | 1   | 1  | Invalid           | -                |
| CCW Data Rise  | 0   | 0  | CCW Clock Fall    | -                |
| CCW Data Rise  | 0   | 1  | CCW Data Rise     | -                |
| CCW Data Rise  | 1   | 0  | Invalid           | -                |
| CCW Data Rise  | 1   | 1  | Start (**)        | CounterClockwise |
| Invalid        | 0   | 0  | Invalid           | -                |
| Invalid        | 0   | 1  | Invalid           | -                |
| Invalid        | 1   | 0  | Invalid           | -                |
| Invalid        | 1   | 1  | Start             | -                |

Here, a single asterisk '*' indicates a fallback situation that is not expected, but we try to handle it correctly.
A double asterisk '**' means it is the end of a cycle, and we generate an event an restart the cycle

### KY-040.h {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6_KY_040H}

So, lets update the KY040 class to add rotary behaviour.

Update the file `code/libraries/device/include/device/gpio/KY-040.h`

```cpp
File: code/libraries/device/include/device/gpio/KY-040.h
...
49: /// @brief Device library namespace
50: namespace device {
51: 
52: enum class SwitchEncoderState;
53: enum class SwitchButtonEvent;
54: enum class SwitchButtonState;
55: 
56: /// <summary>
57: /// KY-040 rotary switch device
58: /// </summary>
59: class KY040
60: {
61: public:
62:     /// <summary>
63:     /// Events generated by the rotary switch
64:     /// </summary>
65:     enum class Event
66:     {
67:         /// @brief Switch is rotated clockwise
68:         RotateClockwise,
69:         /// @brief Switch is rotated counter clockwise
70:         RotateCounterclockwise,
71:         /// @brief Switch is pressed
72:         SwitchDown,
73:         /// @brief Switch is released
74:         SwitchUp,
75:         /// @brief Switch is clicked (short press / release cycle)
76:         SwitchClick,
77:         /// @brief Switch is clicked twice in a short time
78:         SwitchDoubleClick,
79:         /// @brief Switch is clicked three time in a short time
80:         SwitchTripleClick,
81:         /// @brief Switch is held down for a longer time
82:         SwitchHold, ///< generated each second
83:         /// @brief Unknown event
84:         Unknown
85:     };
86: 
87:     /// <summary>
88:     /// Pointer to event handler function to be registered by an application
89:     /// </summary>
90:     using EventHandler = void(Event event, void* param);
91: 
92: private:
93:     /// @brief True if the rotary switch was initialized
94:     bool m_isInitialized;
95:     /// @brief GPIO pin for CLK input
96:     baremetal::PhysicalGPIOPin m_clkPin;
97:     /// @brief GPIO pin for DT input
98:     baremetal::PhysicalGPIOPin m_dtPin;
99:     /// @brief GPIO pin for SW input (switch button)
100:     baremetal::PhysicalGPIOPin m_swPin;
101:     /// @brief Internal state of the rotary encoder
102:     SwitchEncoderState m_switchEncoderState;
103:     /// @brief Internal state of the switch button (to tracking single, double, triple clicking and hold
104:     SwitchButtonState m_switchButtonState;
105:     /// @brief Handle to timer for debouncing the switch button
106:     baremetal::KernelTimerHandle m_debounceTimerHandle;
107:     /// @brief Handle to timer for handling button press ticks (for hold)
108:     baremetal::KernelTimerHandle m_tickTimerHandle;
109:     /// @brief Time at which the current button press occurred
110:     unsigned m_currentPressTicks;
111:     /// @brief Time at which the current button release occurred
112:     unsigned m_currentReleaseTicks;
113:     /// @brief Time at which the last button press occurred
114:     unsigned m_lastPressTicks;
115:     /// @brief Time at which the last button release occurred
116:     unsigned m_lastReleaseTicks;
117: 
118:     /// @brief Registered event handler
119:     EventHandler* m_eventHandler;
120:     /// @brief Parameter for registered event handler
121:     void* m_eventHandlerParam;
122: 
123: public:
124:     KY040(uint8 clkPin, uint8 dtPin, uint8 swPin, baremetal::IMemoryAccess& memoryAccess = baremetal::GetMemoryAccess());
125:     virtual ~KY040();
126: 
127:     void Initialize();
128:     void Uninitialize();
129: 
130:     void RegisterEventHandler(EventHandler* handler, void* param);
131:     void UnregisterEventHandler(EventHandler* handler);
132:     static const char* EventToString(Event event);
133: 
134: private:
135:     static void SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin, void* param);
136:     void SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin);
137:     static void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin, void* param);
138:     void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin);
139:     static void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void* param, void* context);
140:     void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void* param);
141:     static void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void* param, void* context);
142:     void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void* param);
143:     void HandleSwitchButtonEvent(SwitchButtonEvent switchEvent);
144: };
145: 
146: } // namespace device
```

- Line 52: We forward declare the enum `SwitchEncoderState` which will hold the internal rotary encoder state
- Line 67-70: We add the clockwise and counter clockwise rotation as events
- Line 101-102: We add the member variable `m_switchEncoderState` to hold the rotary encoder internal state
- Line 135-136: We add the GPIO pin interrupt handler for the CLK and DT GPIO pins.
One is global and receives a pointer to the KY040 instance, the other is called by the global one

### KY-040.cpp {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6_KY_040CPP}

We'll implement the changes.

Update the file `code/libraries/device/src/gpio/KY-040.cpp`

```cpp
File: code/libraries/device/src/gpio/KY-040.cpp
...
45: /// @file
46: /// KY-040 rotary switch support imlementation.
47: 
48: /// @brief Define log name
49: LOG_MODULE("KY-040");
50: 
51: using namespace baremetal;
52: 
53: namespace device {
54: 
55: /// @brief Time delay for debounding switch button
56: static const unsigned SwitchDebounceDelayMilliseconds = 50;
57: /// @brief Tick delay for determining if switch button was held down
58: static const unsigned SwitchTickDelayMilliseconds = 1000;
59: /// @brief Maximum delay between press and release for a click
60: static const unsigned SwitchClickMaxDelayMilliseconds = 300;
61: /// @brief Maximum delay between two presses for a double click (or triple click)
62: static const unsigned SwitchDoubleClickMaxDelayMilliseconds = 800;
63: 
64: /// <summary>
65: /// Switch encoder internal state
66: /// </summary>
67: enum class SwitchEncoderState
68: {
69:     /// @brief CLK high, DT high
70:     Start,
71:     /// @brief CLK high, DT down
72:     CWStart,
73:     /// @brief CLK down, DT low
74:     CWDataFall,
75:     /// @brief CLK low, DT up
76:     CWClockRise,
77:     /// @brief CLK down, DT high
78:     CCWStart,
79:     /// @brief CLK up DT down
80:     CCWClockFall,
81:     /// @brief CLK up, DT low
82:     CCWDataRise,
83:     /// @brief Invalid state
84:     Invalid,
85:     /// @brief Unknown
86:     Unknown
87: };
88: 
...
131: /// <summary>
132: /// Convert rotary switch encoder state to a string
133: /// </summary>
134: /// <param name="state">Switch encode state</param>
135: /// <returns>String representing state</returns>
136: static const char* EncoderStateToString(SwitchEncoderState state)
137: {
138:     switch (state)
139:     {
140:     case SwitchEncoderState::Start:
141:         return "Start";
142:     case SwitchEncoderState::CWStart:
143:         return "CWStart";
144:     case SwitchEncoderState::CWDataFall:
145:         return "CWDataFall";
146:     case SwitchEncoderState::CWClockRise:
147:         return "CWClockRise";
148:     case SwitchEncoderState::CCWStart:
149:         return "CCWStart";
150:     case SwitchEncoderState::CCWClockFall:
151:         return "CCWClockFall";
152:     case SwitchEncoderState::CCWDataRise:
153:         return "CCWDataRise";
154:     case SwitchEncoderState::Invalid:
155:         return "Invalid";
156:     case SwitchEncoderState::Unknown:
157:     default:
158:         break;
159:     }
160:     return "Unknown";
161: }
162: 
163: /// <summary>
164: /// Convert rotary switch event to a string
165: /// </summary>
166: /// <param name="event">Event type</param>
167: /// <returns>String representing event</returns>
168: const char* KY040::EventToString(KY040::Event event)
169: {
170:     switch (event)
171:     {
172:     case KY040::Event::RotateClockwise:
173:         return "RotateClockwise";
174:     case KY040::Event::RotateCounterclockwise:
175:         return "RotateCounterclockwise";
176:     case KY040::Event::SwitchDown:
177:         return "SwitchDown";
178:     case KY040::Event::SwitchUp:
179:         return "SwitchUp";
180:     case KY040::Event::SwitchClick:
181:         return "SwitchClick";
182:     case KY040::Event::SwitchDoubleClick:
183:         return "SwitchDoubleClick";
184:     case KY040::Event::SwitchTripleClick:
185:         return "SwitchTripleClick";
186:     case KY040::Event::SwitchHold:
187:         return "SwitchHold";
188:     case KY040::Event::Unknown:
189:     default:
190:         break;
191:     }
192:     return "Unknown";
193: }
...
251: /// <summary>
252: /// Lookup table for rotary switch to create an event from an the status of the CLK and DT GPIO inputs when in a certain internal state
253: /// </summary>
254: static const KY040::Event s_encoderOutput[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
255:     //  {{CLK=0/DT=0,            CLK=0/DT=1},            {CLK=1/DT=0,            CLK=1/DT=1}}
256: 
257:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // Start
258: 
259:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CWStart
260:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CWDataFall
261:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateClockwise}       }, // CWClockRise
262: 
263:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CCWStart
264:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CCWClockFall
265:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateCounterclockwise}}, // CCWDataRise
266: 
267:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }  // Invalid
268: };
269: /// <summary>
270: /// Get an event for the rotary switch
271: /// </summary>
272: /// <param name="state">Rotary switch internal state</param>
273: /// <param name="clkValue">Value of CLK GPIO input</param>
274: /// <param name="dtValue">Value of DT GPIO input</param>
275: /// <returns>Resulting event</returns>
276: static KY040::Event GetEncoderOutput(SwitchEncoderState state, bool clkValue, bool dtValue)
277: {
278:     return s_encoderOutput[static_cast<size_t>(state)][clkValue][dtValue];
279: }
280: 
281: /// <summary>
282: /// Lookup table for rotary switch to create an new internal state from an the status of the CLK and DT GPIO inputs when in a certain internal state
283: /// </summary>
284: static const SwitchEncoderState s_encoderNextState[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
285:     //  {{CLK=0/DT=0,                       CLK=0/DT=1},                      {CLK=1/DT=0,                      CLK=1/DT=1}}
286: 
287:     {{SwitchEncoderState::Invalid, SwitchEncoderState::CWStart},          {SwitchEncoderState::CCWStart, SwitchEncoderState::Start}     }, // Start (1, 1), this is the default state between two clicks
288: 
289:     {{SwitchEncoderState::CWDataFall, SwitchEncoderState::CWStart},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}  }, // CWStart (1, 0)
290:     {{SwitchEncoderState::CWDataFall, SwitchEncoderState::CWStart},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Invalid}}, // CWDataFall (0, 0)
291:     {{SwitchEncoderState::CWDataFall, SwitchEncoderState::Invalid},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}  }, // CWClockRise (0, 1)
292: 
293:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart, SwitchEncoderState::Start}     }, // CCWStart (0, 1)
294:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart, SwitchEncoderState::Invalid}   }, // CCWClockFall (0, 0)
295:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::Invalid, SwitchEncoderState::Start}      }, // CCWDataRise (1, 0)
296: 
297:     {{SwitchEncoderState::Invalid, SwitchEncoderState::Invalid},          {SwitchEncoderState::Invalid, SwitchEncoderState::Start}      }  // Invalid
298: };
299: /// <summary>
300: /// Get new internal state for the rotary switch
301: /// </summary>
302: /// <param name="state">Rotary switch internal state</param>
303: /// <param name="clkValue">Value of CLK GPIO input</param>
304: /// <param name="dtValue">Value of DT GPIO input</param>
305: /// <returns>New internal state</returns>
306: static SwitchEncoderState GetEncoderNextState(SwitchEncoderState state, bool clkValue, bool dtValue)
307: {
308:     return s_encoderNextState[static_cast<size_t>(state)][clkValue][dtValue];
309: }
310: 
...
365: /// <summary>
366: /// Constructor for KY040 class
367: /// </summary>
368: /// <param name="clkPin">GPIO pin number for CLK input</param>
369: /// <param name="dtPin">GPIO pin number for DT input</param>
370: /// <param name="swPin">GPIO pin number for SW input</param>
371: /// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
372: KY040::KY040(uint8 clkPin, uint8 dtPin, uint8 swPin, IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
373:     : m_isInitialized{}
374:     , m_clkPin(clkPin, GPIOMode::InputPullUp, memoryAccess)
375:     , m_dtPin(dtPin, GPIOMode::InputPullUp, memoryAccess)
376:     , m_swPin(swPin, GPIOMode::InputPullUp, memoryAccess)
377:     , m_switchEncoderState{SwitchEncoderState::Start}
378:     , m_switchButtonState{SwitchButtonState::Start}
379:     , m_debounceTimerHandle{}
380:     , m_tickTimerHandle{}
381:     , m_currentPressTicks{}
382:     , m_currentReleaseTicks{}
383:     , m_lastPressTicks{}
384:     , m_lastReleaseTicks{}
385:     , m_eventHandler{}
386:     , m_eventHandlerParam{}
387: {
388:     TRACE_DEBUG("KY040 constructor");
389: }
...
400: /// <summary>
401: /// Initialized the KY040 rotary switch
402: /// </summary>
403: void KY040::Initialize()
404: {
405:     if (m_isInitialized)
406:         return;
407: 
408:     TRACE_DEBUG("KY040 Initialize");
409:     m_clkPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
410:     m_dtPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
411:     m_swPin.ConnectInterrupt(SwitchButtonInterruptHandler, this);
412: 
413:     m_clkPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
414:     m_dtPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
415:     m_swPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
416: 
417:     m_isInitialized = true;
418: }
419: 
420: /// <summary>
421: /// Uninitialize the KY040 rotary switch
422: /// </summary>
423: void KY040::Uninitialize()
424: {
425:     if (m_isInitialized)
426:     {
427:         TRACE_DEBUG("Disconnect CLK pin");
428:         m_clkPin.DisableAllInterrupts();
429:         TRACE_DEBUG("Disconnect DT pin");
430:         m_dtPin.DisableAllInterrupts();
431:         TRACE_DEBUG("Disconnect SW pin");
432:         m_swPin.DisableAllInterrupts();
433:         m_swPin.DisconnectInterrupt();
434:         m_isInitialized = false;
435:     }
436:     if (m_debounceTimerHandle)
437:     {
438:         GetTimer().CancelKernelTimer(m_debounceTimerHandle);
439:     }
440:     if (m_tickTimerHandle)
441:     {
442:         GetTimer().CancelKernelTimer(m_tickTimerHandle);
443:     }
444: }
...
470: /// <summary>
471: /// Global GPIO pin interrupt handler for switch encoder
472: /// </summary>
473: /// <param name="pin">GPIO pin for the button encoder inputs</param>
474: /// <param name="param">Parameter for the interrupt handler, which is a pointer to the class instance</param>
475: void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin, void* param)
476: {
477:     KY040* pThis = reinterpret_cast<KY040*>(param);
478:     assert(pThis != nullptr);
479:     pThis->SwitchEncoderInterruptHandler(pin);
480: }
481: 
482: /// <summary>
483: /// GPIO pin interrupt handler for switch encoder
484: /// </summary>
485: /// <param name="pin">GPIO pin for the button encoder inputs</param>
486: void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin)
487: {
488:     auto clkValue = m_clkPin.Get();
489:     auto dtValue = m_dtPin.Get();
490:     TRACE_DEBUG("KY040 CLK: %d", clkValue);
491:     TRACE_DEBUG("KY040 DT:  %d", dtValue);
492:     assert(m_switchEncoderState < SwitchEncoderState::Unknown);
493: 
494:     TRACE_DEBUG("KY040 Current state: %s", EncoderStateToString(m_switchEncoderState));
495:     Event event = GetEncoderOutput(m_switchEncoderState, clkValue, dtValue);
496:     m_switchEncoderState = GetEncoderNextState(m_switchEncoderState, clkValue, dtValue);
497:     TRACE_DEBUG("KY040 Event: %s", EventToString(event));
498:     TRACE_DEBUG("KY040 Next state: %s", EncoderStateToString(m_switchEncoderState));
499: 
500:     if ((event != Event::Unknown) && (m_eventHandler != nullptr))
501:     {
502:         (*m_eventHandler)(event, m_eventHandlerParam);
503:     }
504: }
505: 
```

- Line 64-87: We declare the `SwitchEncoderState` enum values.
You can recognize the values from the table we showed in [Adding intelligence to the rotary switch - Step 6](#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6)
- Line 131-161: We define a function `EncoderStateToString` to convert a rotary encoder state to a string for debugging
- Line 172-175: We add the `Clockwise` and `CounterClockwise` enum values to the conversion of an event to string
- Line 251-268: We define a matrix `s_encoderOutput` to determine the event to be generated when an event happens in a certain state.
This is part of the state machine
- Line 269-279: We implement a local function `GetEncoderOutput()` which uses the `s_encoderOutput` variable to determine the event to generate
- Line 281-298: We define a matrix `s_encoderNextState` to determine the next state when an event happens in a certain state.
This is part of the state machine
- Line 299-309: We implement a local function `GetEncoderNextState()` which uses the `s_encoderNextState` variable to determine the next state
- Line 377: We update the constructor to initialize `m_switchEncoderState`
- Line 409-410: We update the `Initialize()` method to also connect and interrupt handler for the CLK and DT GPIO pins
- Line 413-414: We update the `Initialize()` method to also set up interrupt for both rising and falling edges for the CLK and DT GPIO pins
- Line 427-430: We update the `Uninitialize()` method to also disable interrupts for the CLK and DT pins
- Line 470-480: We implement the global `SwitchEncoderInterruptHandler()` method, which is the global interrupt handler for the CLK and DT GPIO pins.
It converts the parameter to a class pointer, and then calls the class method
- Line 482-504: We implement the `SwitchEncoderInterruptHandler()` method, which is the class interrupt handler for the CLK and DT GPIO pins
  - Line 488-489: It reads the GPIO pin values
  - Line 495: We determine the event to be generated from the current state and the GPIO pin values
  - Line 496: We determine the next state from the current state and the GPIO pin values
  - Line 500-503: If the event is not `Unknown` and there is an event handler function installed, we call it

### Update application code {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6_UPDATE_APPLICATION_CODE}

Let's try to do something useful with the rotary switch.
We use a value which starts at 0, if we rotate clockwise we increment the value, if we rotate counter clockwise we decrement, and if the switch is push down, we print the value.

In fact, let's make it even more fancy. If we press and hold the switch button for 2 seconds, let's automatically perform a reboot.
This means the main loop will simply be waiting for an interrupt to happen, and check whether we need to reboot.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/Logger.h"
5: #include "baremetal/System.h"
6: #include "baremetal/Timer.h"
7: #include <device/gpio/KY-040.h>
8:
9: LOG_MODULE("main");
10:
11: using namespace baremetal;
12: using namespace device;
13:
14: static int value = 0;
15: static int holdCounter = 0;
16: static const int HoldThreshold = 2;
17: static bool reboot = false;
18:
19: void OnEvent(KY040::Event event, void *param)
20: {
21:     LOG_INFO("Event %s", KY040::EventToString(event));
22:     switch (event)
23:     {
24:         case KY040::Event::SwitchDown:
25:             LOG_INFO("Value %d", value);
26:             break;
27:         case KY040::Event::RotateClockwise:
28:             value++;
29:             break;
30:         case KY040::Event::RotateCounterclockwise:
31:             value--;
32:             break;
33:         case KY040::Event::SwitchHold:
34:             holdCounter++;
35:             if (holdCounter >= HoldThreshold)
36:             {
37:                 reboot = true;
38:                 LOG_INFO("Reboot triggered");
39:             }
40:             break;
41:         default:
42:             break;
43:     }
44: }
45:
46: int main()
47: {
48:     auto& console = GetConsole();
49:     GetLogger().SetLogLevel(LogSeverity::Info);
50:
51:     auto exceptionLevel = CurrentEL();
52:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
53:
54:     KY040 rotarySwitch(11, 9, 10);
55:     rotarySwitch.Initialize();
56:     rotarySwitch.RegisterEventHandler(OnEvent, nullptr);
57:
58:     LOG_INFO("Hold down switch button for %d seconds to reboot", HoldThreshold);
59:     while (!reboot)
60:     {
61:         WaitForInterrupt();
62:     }
63:
64:     rotarySwitch.UnregisterEventHandler(OnEvent);
65:     rotarySwitch.Uninitialize();
66:
67:     LOG_INFO("Rebooting");
68:
69:     return static_cast<int>(ReturnCode::ExitReboot);
70: }
```

- Line 14: We introduce a variable to be incremented / decremented by the rotary switch
- Line 19-44: We handle the event callback from the rotary switch
  - Line 24-26: If the switch button is down, we print the current value of the variable we introduced
  - Line 27-29: If the switch is rotated clockwise, we increment the value
  - Line 30-32: If the switch is rotated counter clockwise, we decrement the value
  - Line 33-40: If the switch button is held down, we count the number of times we get a `SwitchHold` event.
If this reached `HoldThreshold`, we flag a reboot
- Line 58: We print a message to hold down the button for `HoldThreshold` seconds to reboot
- Line 59-62: As long as the reboot flag is not set, we wait for an interrupt.
This effectively shuts down the core until an interrupt happens
- Line 69: We return `ReturnCode::ExitReboot` to enforce a reboot

### Configuring, building and debugging {#TUTORIAL_21_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_6_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice the rotate events, as well as the value being printed when with push the switch down.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Current EL: 1 (main:52)
Info   0.00:00:00.090 Hold down switch button for 2 seconds to reboot (main:58)
Info   0.00:00:00.820 Event RotateClockwise (main:21)
Info   0.00:00:01.370 Event RotateCounterclockwise (main:21)
Info   0.00:00:02.750 Event RotateClockwise (main:21)
Info   0.00:00:03.220 Event RotateClockwise (main:21)
Info   0.00:00:03.390 Event RotateClockwise (main:21)
Info   0.00:00:03.780 Event RotateCounterclockwise (main:21)
Info   0.00:00:04.780 Event SwitchDown (main:21)
Info   0.00:00:04.780 Value 2 (main:25)
Info   0.00:00:05.010 Event SwitchUp (main:21)
Info   0.00:00:05.010 Event SwitchClick (main:21)
Info   0.00:00:05.780 Event SwitchDown (main:21)
Info   0.00:00:05.780 Value 2 (main:25)
Info   0.00:00:05.950 Event SwitchUp (main:21)
Info   0.00:00:05.950 Event SwitchClick (main:21)
Info   0.00:00:07.850 Event RotateCounterclockwise (main:21)
Info   0.00:00:08.020 Event RotateCounterclockwise (main:21)
Info   0.00:00:09.200 Event SwitchDown (main:21)
Info   0.00:00:09.200 Value 0 (main:25)
Info   0.00:00:09.400 Event SwitchUp (main:21)
Info   0.00:00:09.400 Event SwitchClick (main:21)
Info   0.00:00:10.790 Event RotateClockwise (main:21)
Info   0.00:00:12.090 Event SwitchDown (main:21)
Info   0.00:00:12.090 Value 1 (main:25)
Info   0.00:00:12.240 Event SwitchUp (main:21)
Info   0.00:00:13.890 Event SwitchDown (main:21)
Info   0.00:00:13.890 Value 1 (main:25)
Info   0.00:00:14.890 Event SwitchHold (main:21)
Info   0.00:00:15.890 Event SwitchHold (main:21)
Info   0.00:00:15.890 Reboot triggered (main:38)
Info   0.00:00:15.920 Rebooting (main:67)
Info   0.00:00:15.940 Reboot (System:144)
```

Next: [22-i2c](22-i2c.md)
