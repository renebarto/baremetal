# Tutorial 20: GPIO {#TUTORIAL_20_GPIO}

@tableofcontents

## New tutorial setup {#TUTORIAL_20_GPIO_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-timer-extension`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_20_GPIO_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-20.a`
- a library `output/Debug/lib/stdlib-20.a`
- an application `output/Debug/bin/20-timer-extension.elf`
- an image in `deploy/Debug/20-timer-extension-image`

## Using hardware {#TUTORIAL_20_GPIO_EXTENDING_THE_TIMER}

In this tutorial, we'll be adding hardware. We'll connect the hardware to GPIO pins, so we can read out a rotary encoder.
We'll then trigger on changes of the GPIO pins to generate interrupts. We'll also be debouncing the GPIO inputs using kernel timers.

Now on using hardware, be careful when connecting devices to the GPIO header. A number of things to make sure of:

- Use good quality wiring, preferably buy jumper wires, do not create them yourself, as it is tricky work and may lead to failures
- Make sure which __voltage__ to use. Raspberry Pi has both 5V and 3.3V power, but not all pins are able to support 5V. If in doubt, first get the specifications for the device
- Be __absolutely sure__ to connect to the right pins on the header. Failing to connect correctly may break you Raspberry Pi board

### The KY-040 rotary encoder

The KY-040 rotary encoder is a relatively simple device to start with. It is sold as a input device for Arduino (Iduino), and is also sold by e.g. Joy-it.

<img src="images/KY-040.png"  alt="KY-040 rotary encoder" width="400"/>

It needs a supply voltage VCC (VCC or +), we can use 3.3V for this, and a ground (GND or -).
The outputs are a press switch `SW` as well as two rotary encoder outputs named `CLK` and `DT`.

We can conveniently use 5 pins close to each other on the Raspberry Pi for this, see the GPIO pinout below (this holds for all Raspberry PPi models since version 2):

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

Here the A to C siwtch corresponds to the `CLK` output, and the B to C switch to the `DT` output.

## Setting up GPIO and reading data - Step 1 {#TUTORIAL_20_GPIO_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1}

We'll add some functionality to the `Timer` class for enabling, handling and disabling interrupts on the timer.

### Timer.h {#TUTORIAL_20_GPIO_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
```

Next: [21-i2c](21-i2c.md)
