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

## Using hardware {#TUTORIAL_20_GPIO_USING_HARDWARE}

In this tutorial, we'll be adding hardware. We'll connect the hardware to GPIO pins, so we can read out a rotary encoder.
We'll then trigger on changes of the GPIO pins to generate interrupts. We'll also be debouncing the GPIO inputs using kernel timers.

Now on using hardware, be careful when connecting devices to the GPIO header. A number of things to make sure of:

- Use good quality wiring, preferably buy jumper wires, do not create them yourself, as it is tricky work and may lead to failures
- Make sure which __voltage__ to use. Raspberry Pi has both 5V and 3.3V power, but not all pins are able to support 5V. If in doubt, first get the specifications for the device
- Be __absolutely sure__ to connect to the right pins on the header. Failing to connect correctly may break you Raspberry Pi board

### The KY-040 rotary encoder {#TUTORIAL_20_GPIO_USING_HARDWARE_THE_KY_040_ROTARY_ENCODER}

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

## Setting up GPIO and reading data - Step 1 {#TUTORIAL_20_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_1}

We'll start by reading the GPIO values for the signals we just introduced.

### Update application code {#TUTORIAL_20_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_1_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/PhysicalGPIOPin.h>
6: #include <baremetal/System.h>
7: #include <baremetal/Timer.h>
8: 
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: 
13: int main()
14: {
15:     auto& console = GetConsole();
16: 
17:     auto exceptionLevel = CurrentEL();
18:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
19: 
20:     PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
21:     PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
22:     PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
23: 
24:     auto valueCLK = pinCLK.Get();
25:     auto valueDT = pinDT.Get();
26:     auto valueSW = pinSW.Get();
27:     int waitTimeMS = 10000;
28:     LOG_DEBUG("CLK=%d, DT=%d, SW=%d", valueCLK, valueDT, valueSW);
29:     while (waitTimeMS > 0)
30:     {
31:         auto newValueCLK = pinCLK.Get();
32:         auto newValueDT = pinDT.Get();
33:         auto newValueSW = pinSW.Get();
34:         if (newValueCLK != valueCLK)
35:         {
36:             LOG_DEBUG("CLK=%d", newValueCLK);
37:             valueCLK = newValueCLK;
38:         }
39:         if (newValueDT != valueDT)
40:         {
41:             LOG_DEBUG("DT=%d", newValueDT);
42:             valueDT = newValueDT;
43:         }
44:         if (newValueSW != valueSW)
45:         {
46:             LOG_DEBUG("SW=%d", newValueSW);
47:             valueSW = newValueSW;
48:         }
49: 
50:         Timer::WaitMilliSeconds(10);
51:         waitTimeMS -= 10;
52:     }
53: 
54:     console.Write("Press r to reboot, h to halt\n");
55:     char ch{};
56:     while ((ch != 'r') && (ch != 'h'))
57:     {
58:         ch = console.ReadChar();
59:         console.WriteChar(ch);
60:     }
61: 
62:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
63: }
```

- Line 5: We add the include file for the physical GPIO pin
- Line 20: We set up a physical GPIO pin for the CLK input on pin 11.
Notice that we use `InputPullUp` as this is a GPIO input, and it is internally pulled up by the rotary switch hardware
- Line 21: We set up a physical GPIO pin for the DT input on pin 9
- Line 22: We set up a physical GPIO pin for the SW input on pin 10
- Line 24-26: We read the current values for each pin
- Line 27: We initialize a counter to count down time. We set it to 10 seconds
- Line 28: We print the current values for each pin
- Line 29: We loop as long as the counter is still not 0
- Line 31-33: We read the current values for each pin
- Line 34-38: If the value of the CLK input changed, we print its value
- Line 39-43: If the value of the CLK input changed, we print its value
- Line 44-48: If the value of the CLK input changed, we print its value
- Line 50-51: We wait can decrement the counter

### Configuring, building and debugging {#TUTORIAL_20_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Note that we need to have the hardware installed, so we need to run the code on an actual board.

When we press the switch:

```text
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:20)
Debug  0.00:00:00.090 CLK=1, DT=1, SW=1 (main:30)
Debug  0.00:00:01.880 SW=0 (main:48)
Debug  0.00:00:02.140 SW=1 (main:48)
Debug  0.00:00:02.730 SW=0 (main:48)
Debug  0.00:00:02.970 SW=1 (main:48)
Debug  0.00:00:03.460 SW=0 (main:48)
Debug  0.00:00:03.640 SW=1 (main:48)
Debug  0.00:00:04.260 SW=0 (main:48)
Debug  0.00:00:04.450 SW=1 (main:48)
Press r to reboot, h to halt
```

It is quite simple to see the values change when the switch is pressed.
Notice however, that the initial value is 1, meaning that pressing the switch actually makes the value go to 0.

When we turn the switch clockwise:

```text
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:20)
Debug  0.00:00:00.090 CLK=1, DT=1, SW=1 (main:30)
Debug  0.00:00:01.030 CLK=0 (main:38)
Debug  0.00:00:01.090 DT=0 (main:43)
Debug  0.00:00:01.120 CLK=1 (main:38)
Debug  0.00:00:01.150 DT=1 (main:43)
Debug  0.00:00:01.560 CLK=0 (main:38)
Debug  0.00:00:01.590 DT=0 (main:43)
Debug  0.00:00:01.630 CLK=1 (main:38)
Debug  0.00:00:01.680 DT=1 (main:43)
Debug  0.00:00:02.120 CLK=0 (main:38)
Debug  0.00:00:02.170 DT=0 (main:43)
Debug  0.00:00:02.250 CLK=1 (main:38)
Debug  0.00:00:02.340 DT=1 (main:43)
Debug  0.00:00:02.900 CLK=0 (main:38)
Debug  0.00:00:02.930 DT=0 (main:43)
Debug  0.00:00:02.970 CLK=1 (main:38)
Debug  0.00:00:03.110 DT=1 (main:43)
Press r to reboot, h to halt
```

Again we see that both values are initially 1, and turning the switch clockwise results in the following pattern:

- First CLK goes low
- Then DT goes low
- Then CLK goes high
- And finally DT goes high

When we turn the switch anticlockwise:

```text
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:20)
Debug  0.00:00:00.090 CLK=1, DT=1, SW=1 (main:30)
Debug  0.00:00:00.730 DT=0 (main:43)
Debug  0.00:00:00.760 CLK=0 (main:38)
Debug  0.00:00:00.780 DT=1 (main:43)
Debug  0.00:00:00.810 CLK=1 (main:38)
Debug  0.00:00:00.900 DT=0 (main:43)
Debug  0.00:00:00.950 CLK=0 (main:38)
Debug  0.00:00:02.040 DT=1 (main:43)
Debug  0.00:00:02.060 CLK=1 (main:38)
Debug  0.00:00:02.980 DT=0 (main:43)
Debug  0.00:00:03.010 CLK=0 (main:38)
Debug  0.00:00:03.050 DT=1 (main:43)
Debug  0.00:00:03.080 CLK=1 (main:38)
Debug  0.00:00:04.520 DT=0 (main:43)
Debug  0.00:00:04.550 CLK=0 (main:38)
Debug  0.00:00:04.580 DT=1 (main:43)
Debug  0.00:00:04.610 CLK=1 (main:38)
Press r to reboot, h to halt
```

Again we see that both values are initially 1, however turning the switch anti-clockwise results in a different pattern:

- First DT goes low
- Then CLK goes low
- Then DT goes high
- And finally CLK goes high

One thing worthwhile however happens, when we turn the switch quickly (clockwise in this case):

```text
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:20)
Debug  0.00:00:00.090 CLK=1, DT=1, SW=1 (main:30)
Debug  0.00:00:00.810 CLK=0 (main:38)
Debug  0.00:00:00.840 CLK=1 (main:38)
Debug  0.00:00:00.860 CLK=0 (main:38)
Debug  0.00:00:00.890 CLK=1 (main:38)
Debug  0.00:00:00.920 CLK=0 (main:38)
Debug  0.00:00:00.960 DT=0 (main:43)
Debug  0.00:00:00.990 CLK=1 (main:38)
Debug  0.00:00:01.000 DT=1 (main:43)
Debug  0.00:00:01.040 CLK=0 (main:38)
Debug  0.00:00:01.070 CLK=1 (main:38)
Debug  0.00:00:01.110 CLK=0 (main:38)
Debug  0.00:00:01.140 DT=0 (main:43)
Debug  0.00:00:01.160 CLK=1 (main:38)
Debug  0.00:00:01.180 DT=1 (main:43)
Debug  0.00:00:01.220 DT=0 (main:43)
Debug  0.00:00:01.250 CLK=0 (main:38)
Debug  0.00:00:01.280 DT=1 (main:43)
Debug  0.00:00:01.310 CLK=1 (main:38)
Press r to reboot, h to halt
```

Here we see that the pattern is disturbed. If we want to reliably handle turning the switch, we need to this into account as well.

## Adding GPIO interrupts - Step 2 {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2}

Now that we can read GPIO data for the rotary switch, let's set up for generating interrupts for these signals.
We'd like to get an interrupt on each change, so both on rising and falling edges.

We'll first add some methods to `PhysicalGPIOPin` to enable setting up for interrupts.

### Revisiting GPIO registers {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_REVISITING_GPIO_REGISTERS}

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

Each of these blocks is 12 byte (3 times 4) in size.

### IGPIOPin.h {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_IGPIOPINH}

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

- Line 125: We add an abstract method to check whether an event happened on the GPIO pin
- Line 129: We add an abstract method to clear any event conditions on the GPIO pin

### PhysicalGPIOPin.h {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINH}

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
90:     uint8               m_pinNumber;
91:     /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
92:     GPIOMode            m_mode;
93:     /// @brief Configured GPIO function.
94:     GPIOFunction        m_function;
95:     /// @brief Configured GPIO pull mode (only for input function).
96:     GPIOPullMode        m_pullMode;
97:     /// @brief Current value of the GPIO pin (true for on, false for off).
98:     bool                m_value;
99:     /// @brief Memory access interface reference for accessing registers.
100:     IMemoryAccess&      m_memoryAccess;
101:     /// @brief Register offset for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
102:     unsigned            m_regOffset;
103:     /// @brief Register mask for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
104:     uint32              m_regMask;
105:     /// @brief GPIO interrupt types enabled
106:     bool                m_interruptEnabled[static_cast<size_t>(GPIOInterruptType::Unknown)];
107: 
108: public:
109:     PhysicalGPIOPin(IMemoryAccess &memoryAccess = GetMemoryAccess());
110: 
111:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess = GetMemoryAccess());
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

- Line 65-81: We add a enum type `GPIOInterruptType` to distiguish between the different types of events that can cause an interrupt.
There relate directly to the registered mentioned above in [Revisiting GPIO registers](#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_REVISITING_GPIO_REGISTERS)
- Line 102: We add a member variable `m_regOffset` to indicate the byte offset of the register to address for the GPIO.
This is relative to the group, and as there are 32 GPIO event bits in each register, this will thus be (m_pinNumber / 32) * 4
- Line 104: We add a member variable `m_regMask` to indicate the register mask to be used for the GPIO.
As there are 32 GPIO event bits in each register, this will thus be 1 << (m_pinNumber % 32)
- Line 106: We add a member variable `m_interruptEnabled` to indicate for each interrupt type whether it is enabled
- Line 121-122: We override the methods `GetEvent()` and `ClearEvent()` in the abstract class `IGPIOPin`
- Line 130: We add a method `EnableInterrupt()` to enable interrupts of the specified type
- Line 131: We add a method `DisableInterrupt()` to disable interrupts of the specified type
- Line 132: We add a method `DisableAllInterrupts()` to disable interrupts of all different types

### PhysicalGPIOPin.cpp {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINCPP}

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
...
94: /// <summary>
95: /// Creates a virtual GPIO pin 
96: /// </summary>
97: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
98: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess &memoryAccess /*= GetMemoryAccess()*/)
99:     : m_pinNumber{ NUM_GPIO }
100:     , m_mode{ GPIOMode::Unknown }
101:     , m_function{ GPIOFunction::Unknown }
102:     , m_pullMode{ GPIOPullMode::Unknown }
103:     , m_value{}
104:     , m_memoryAccess{ memoryAccess }
105:     , m_regOffset{}
106:     , m_regMask{}
107:     , m_interruptEnabled{}
108: {
109: }
110: 
111: /// <summary>
112: /// Creates a virtual GPIO pin 
113: /// </summary>
114: /// <param name="pinNumber">GPIO pin number (0..53)</param>
115: /// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
116: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
117: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess /*= m_memoryAccess*/)
118:     : m_pinNumber{ NUM_GPIO }
119:     , m_mode{ GPIOMode::Unknown }
120:     , m_value{}
121:     , m_memoryAccess{ memoryAccess }
122:     , m_regOffset{}
123:     , m_regMask{}
124:     , m_interruptEnabled{}
125: {
126:     AssignPin(pinNumber);
127:     SetMode(mode);
128: }
...
139: /// <summary>
140: /// Assign a GPIO pin
141: /// </summary>
142: /// <param name="pinNumber">GPIO pin number to set (0..53)</param>
143: /// <returns>Return true on success, false on failure</returns>
144: bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
145: {
146:     // Check if pin already assigned
147:     if (m_pinNumber != NUM_GPIO)
148:         return false;
149:     m_pinNumber = pinNumber;
150: 
151:     m_regOffset = (m_pinNumber / 32) * 4;
152:     m_regMask   = 1 << (m_pinNumber % 32);
153: 
154:     return true;
155: }
156: 
...
223: /// <summary>
224: /// Get GPIO event status
225: /// </summary>
226: /// <returns>GPIO event status, true if an event is flagged, false if not</returns>
227: bool PhysicalGPIOPin::GetEvent()
228: {
229:     return (m_memoryAccess.Read32(RPI_GPIO_GPEDS0 + m_regOffset) & m_regMask) != 0;
230: }
231: 
232: /// <summary>
233: /// Clear GPIO event status
234: /// </summary>
235: void PhysicalGPIOPin::ClearEvent()
236: {
237:     m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + m_regOffset, m_regMask);
238: }
239: 
...
362: /// <summary>
363: /// Enable interrupts for the specified type
364: /// </summary>
365: /// <param name="interruptType">Interrupt type to enable</param>
366: void PhysicalGPIOPin::EnableInterrupt(GPIOInterruptType interruptType)
367: {
368:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
369: 
370:     assert(interruptType < GPIOInterruptType::Unknown);
371:     if (!m_interruptEnabled[static_cast<size_t>(interruptType)])
372:     {
373:         m_interruptEnabled[static_cast<size_t>(interruptType)] = true;
374: 
375:         regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + (static_cast<unsigned>(interruptType) - static_cast<unsigned>(GPIOInterruptType::RisingEdge)) * 12;
376: 
377:         m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) | m_regMask);
378:     }
379: }
380: 
381: /// <summary>
382: /// Disable interrupts for the specified type
383: /// </summary>
384: /// <param name="interruptType">Interrupt type to disable</param>
385: void PhysicalGPIOPin::DisableInterrupt(GPIOInterruptType interruptType)
386: {
387:     assert(interruptType < GPIOInterruptType::Unknown);
388:     if (m_interruptEnabled[static_cast<size_t>(interruptType)])
389:     {
390:         m_interruptEnabled[static_cast<size_t>(interruptType)] = false;
391: 
392:         regaddr regAddress = RPI_GPIO_GPREN0 + m_regOffset + (static_cast<unsigned>(interruptType) - static_cast<unsigned>(GPIOInterruptType::RisingEdge)) * 12;
393: 
394:         m_memoryAccess.Write32(regAddress, m_memoryAccess.Read32(regAddress) & ~m_regMask);
395:     }
396: }
397: 
398: /// <summary>
399: /// Disable all interrupts
400: /// </summary>
401: void PhysicalGPIOPin::DisableAllInterrupts()
402: {
403:     DisableInterrupt(GPIOInterruptType::RisingEdge);
404:     DisableInterrupt(GPIOInterruptType::FallingEdge);
405:     DisableInterrupt(GPIOInterruptType::HighLevel);
406:     DisableInterrupt(GPIOInterruptType::LowLevel);
407:     DisableInterrupt(GPIOInterruptType::AsyncRisingEdge);
408:     DisableInterrupt(GPIOInterruptType::AsyncFallingEdge);
409: }
410: 
...
```

- Line 105-107: We initialize the new member variables in the 'default' constructor only taking the memory access instance
- Line 122-124: We initialize the new member variables in the 'specific' constructor also taking the pin number and mode
- Line 151-152: We set up the values for `m_regOffset` and `m_regMask` when a GPIO pin is assigned
- Line 227-230: We implement the `GetEvent()` method, by reading the correct event detection status register `RPI_GPIO_GPEDSn`, and checking whether the bit for this GPIO pin is a `1`
- Line 235-238: We implement the `ClearEvent()` method by writing the mask (containing only a `1` bit for this GPIO pin) and write to the event detection status register.
This has the effect of resetting the selected status bit
- Line 366-379: We implement `EnableInterrupt()`
  - Line 368: We verify that the GPIO pin is set to one of the input modes
  - Line 370: We verify that the interrupt type requested is valid
  - Line 371: We check that the interrupt type is not already enabled
  - Line 373: We set the flag to true
  - Line 375: We calculate the register address
  - Line 377: We read the register, and the set the bit for the GPIO pin to `1` and write
- Line 385-396: We implement `DisableInterrupt()`
  - Line 387: We verify that the interrupt type requested is valid
  - Line 388: We check that the interrupt type is enabled
  - Line 390: We set the flag to false
  - Line 392: We calculate the register address
  - Line 394: We read the register, and the set the bit for the GPIO pin to `0` and write
- Line 401-409: We implement `DisableAllInterrupts()` by calling `DisableInterrupt()` for all types of events

### Update application code {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/InterruptHandler.h>
5: #include <baremetal/Interrupts.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/PhysicalGPIOPin.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
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
24:     LOG_DEBUG("GPIO3");
25:     GPIOPins* pins = reinterpret_cast<GPIOPins*>(param);
26:     if (pins->pinCLK.GetEvent())
27:     {
28:         auto value = pins->pinCLK.Get();
29:         LOG_DEBUG("CLK=%d", value);
30:         pins->pinCLK.ClearEvent();
31:     }
32:     if (pins->pinDT.GetEvent())
33:     {
34:         auto value = pins->pinDT.Get();
35:         LOG_DEBUG("DT=%d", value);
36:         pins->pinDT.ClearEvent();
37:     }
38:     if (pins->pinSW.GetEvent())
39:     {
40:         auto value = pins->pinSW.Get();
41:         LOG_DEBUG("SW=%d", value);
42:         pins->pinSW.ClearEvent();
43:     }
44: }
45: 
46: int main()
47: {
48:     auto& console = GetConsole();
49: 
50:     auto exceptionLevel = CurrentEL();
51:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
52: 
53:     PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
54:     PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
55:     PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
56:     GPIOPins pins { pinCLK, pinDT, pinSW };
57: 
58:     GetInterruptSystem().RegisterIRQHandler(IRQ_ID::IRQ_GPIO3, InterruptHandler, &pins);
59: 
60:     pinCLK.EnableInterrupt(GPIOInterruptType::RisingEdge);
61:     pinCLK.EnableInterrupt(GPIOInterruptType::FallingEdge);
62:     pinDT.EnableInterrupt(GPIOInterruptType::RisingEdge);
63:     pinDT.EnableInterrupt(GPIOInterruptType::FallingEdge);
64:     pinSW.EnableInterrupt(GPIOInterruptType::RisingEdge);
65:     pinSW.EnableInterrupt(GPIOInterruptType::FallingEdge);
66: 
67:     LOG_INFO("Wait 5 seconds");
68:     Timer::WaitMilliSeconds(5000);
69: 
70:     GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);
71: 
72:     pinCLK.DisableAllInterrupts();
73:     pinDT.DisableAllInterrupts();
74:     pinSW.DisableAllInterrupts();
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

- Line 7: We include the header for `PhysicalGPIOPin`
- Line 15-20: We set up a struct `GPIOPins` to access the different pins we are handling.
This is needed as the interrupt handler will not have any context, so we need to provide it through its parameter
- Line 22-44: We define the interrupt handler
  - Line 25: We convert the parameter to a pointer to the struct `GPIOPins`
  - Line 26-31: If the CLK pin had an event, we get its value and print it, then clear the event
  - Line 32-37: If the DT pin had an event, we get its value and print it, then clear the event
  - Line 38-43: If the SW pin had an event, we get its value and print it, then clear the event
- Line 56: We create and instance of the struct `GPIOPins` and fill it with pointers to the respective pins
- Line 58: We register to the interrupt `IRQ_GPIO3`. This is a shared interrupt, which is triggered when any of the pins generate and event
- Line 60-61: We enable interrupts for rising and falling edges on the CLK pin
- Line 62-63: We enable interrupts for rising and falling edges on the DT pin
- Line 64-65: We enable interrupts for rising and falling edges on the SW pin
- Line 67-68: We wait again as we did before, so we no longer actively monitor the pins
- Line 70: We unregister to the interrupt `IRQ_GPIO3`
- Line 72: We disable all interrupts on the CLK pin
- Line 73: We disable all interrupts on the DT pin
- Line 74: We disable all interrupts on the SW pin

### Configuring, building and debugging {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

When we press the switch or turn it, you can see the interrupts coming in.

```text
Info   0.00:00:00.030 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:51)
Debug  0.00:00:00.100 GPIO3 (main:24)
Debug  0.00:00:00.100 CLK=1 (main:29)
Debug  0.00:00:00.130 GPIO3 (main:24)
Debug  0.00:00:00.130 DT=1 (main:35)
Debug  0.00:00:00.170 GPIO3 (main:24)
Debug  0.00:00:00.170 SW=1 (main:41)
Info   0.00:00:00.210 Wait 5 seconds (main:67)
Debug  0.00:00:01.140 GPIO3 (main:24)
Debug  0.00:00:01.140 SW=0 (main:41)
Debug  0.00:00:01.320 GPIO3 (main:24)
Debug  0.00:00:01.320 SW=1 (main:41)
Debug  0.00:00:01.940 GPIO3 (main:24)
Debug  0.00:00:01.940 SW=0 (main:41)
Debug  0.00:00:02.120 GPIO3 (main:24)
Debug  0.00:00:02.120 SW=1 (main:41)
Debug  0.00:00:02.740 GPIO3 (main:24)
Debug  0.00:00:02.740 SW=0 (main:41)
Debug  0.00:00:02.910 GPIO3 (main:24)
Debug  0.00:00:02.910 SW=1 (main:41)
Debug  0.00:00:03.730 GPIO3 (main:24)
Debug  0.00:00:03.730 DT=0 (main:35)
Debug  0.00:00:03.770 GPIO3 (main:24)
Debug  0.00:00:03.770 CLK=0 (main:29)
Debug  0.00:00:03.770 DT=1 (main:35)
Debug  0.00:00:03.830 GPIO3 (main:24)
Debug  0.00:00:03.830 CLK=1 (main:29)
Debug  0.00:00:03.890 GPIO3 (main:24)
Debug  0.00:00:03.890 DT=0 (main:35)
Debug  0.00:00:03.920 GPIO3 (main:24)
Debug  0.00:00:03.920 CLK=0 (main:29)
Debug  0.00:00:03.920 DT=1 (main:35)
Debug  0.00:00:04.010 GPIO3 (main:24)
Debug  0.00:00:04.010 DT=0 (main:35)
Debug  0.00:00:04.050 GPIO3 (main:24)
Debug  0.00:00:04.050 CLK=0 (main:29)
Debug  0.00:00:04.050 DT=1 (main:35)
Debug  0.00:00:04.110 GPIO3 (main:24)
Debug  0.00:00:04.110 CLK=1 (main:29)
Debug  0.00:00:04.270 GPIO3 (main:24)
Debug  0.00:00:04.270 DT=0 (main:35)
Debug  0.00:00:04.310 GPIO3 (main:24)
Debug  0.00:00:04.310 CLK=0 (main:29)
Debug  0.00:00:04.310 DT=1 (main:35)
Debug  0.00:00:04.360 GPIO3 (main:24)
Debug  0.00:00:04.360 CLK=1 (main:29)
Debug  0.00:00:04.800 GPIO3 (main:24)
Debug  0.00:00:04.800 CLK=0 (main:29)
Debug  0.00:00:04.840 GPIO3 (main:24)
Debug  0.00:00:04.840 DT=0 (main:35)
Debug  0.00:00:04.880 GPIO3 (main:24)
Debug  0.00:00:04.880 CLK=1 (main:29)
Debug  0.00:00:04.880 DT=1 (main:35)
Debug  0.00:00:04.950 GPIO3 (main:24)
Debug  0.00:00:04.950 CLK=0 (main:29)
Debug  0.00:00:04.950 DT=1 (main:35)
Debug  0.00:00:05.060 GPIO3 (main:24)
Debug  0.00:00:05.060 CLK=1 (main:29)
Debug  0.00:00:05.110 GPIO3 (main:24)
Debug  0.00:00:05.110 DT=1 (main:35)
Press r to reboot, h to halt
```

## General approach for GPIO interrupts - Step 3 {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3}

As we can see in the previous step, we need to set up administration to pass to the interrupt handler.
That is impractical, it would be easier if we could register a handler for a specific GPIO pin interrupt.

We'll introduce a GPIO manager that will register to the GPIO interrupts, and have each pin register itself with the manager.
The GPIO manager will then check for each registered pin whether an event occurred, and call the interrupt handler on that pin.

### IGPIOManager.h {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_IGPIOMANAGERH}

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
42: #include <stdlib/Types.h>
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
70:     virtual void ConnectInterrupt(IGPIOPin *pin) = 0;
71:     /// <summary>
72:     /// Disconnect the GPIO pin interrupt for the specified pin
73:     /// </summary>
74:     /// <param name="pin">GPIO pin to disconnect interrupt for</param>
75:     virtual void DisconnectInterrupt(const IGPIOPin *pin) = 0;
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

- Line 53-86: We declare the abstract class `IGPIOManager`
  - Line 64: We declare an abstract method `Initialize()` to initialize the manager
  - Line 70: We declare an abstract method `ConnectInterrupt()` to register a GPIO pin for interrupts
  - Line 75: We declare an abstract method `DisconnectInterrupt()` to unregister a GPIO pin for interrupts
  - Line 80: We declare an abstract method `InterruptHandler()` as the interrupt handler function
  - Line 85: We declare an abstract method `AllOff()` to unregister all GPIO pins and switch them to default mode

### GPIOManager.h {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_GPIOMANAGERH}

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
42: #include <stdlib/Types.h>
43: #include <baremetal/IGPIOManager.h>
44: #include <baremetal/PhysicalGPIOPin.h>
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
55: class GPIOManager
56:     : public IGPIOManager
57: {
58:     /// <summary>
59:     /// Construct the singleton GPIOManager instance if needed, and return a reference to the instance. This is a friend function of class GPIOManager
60:     /// </summary>
61:     /// <returns>Reference to the singleton GPIOManager instance</returns>
62:     friend GPIOManager &GetGPIOManager();
63: 
64: private:
65:     /// @brief True if class is already initialized
66:     bool m_isInitialized;
67:     /// @brief Array of all registered GPIO pins (nullptr if the GPIO is not registered)
68:     IGPIOPin* m_pins[NUM_GPIO];
69:     /// @brief Memory access interface
70:     IMemoryAccess& m_memoryAccess;
71: 
72:     GPIOManager();
73: 
74: public:
75:     explicit GPIOManager(IMemoryAccess &memoryAccess);
76:     ~GPIOManager();
77: 
78:     void Initialize() override;
79: 
80:     void ConnectInterrupt(IGPIOPin *pin) override;
81:     void DisconnectInterrupt(const IGPIOPin *pin) override;
82: 
83:     void InterruptHandler() override;
84: 
85:     void AllOff() override;
86: 
87:     void DisableAllInterrupts(uint8 pinNumber);
88: 
89: private:
90: };
91: 
92: GPIOManager &GetGPIOManager();
93: 
94: } // namespace baremetal
```

- Line 55-92: We declare the class `GPIOManager`
  - Line 62: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager
  - Line 66: We declare a member variable `m_isInitialized` to guard against multiple initialization
  - Line 68: We declare a member variable `m_pins` to keep track of the registered GPIO pins
  - Line 70: We declare a member variable `m_memoryAccess` to hold the `MemoryAccess` instance
  - Line 72: We declare the default constructor as private, as we did before
  - Line 75: We declare a non-default constructor taking a `MemoryAccess` instance
  - Line 76: We declare a virtual destructor as we inherit from an abstract interface
  - Line 78: We declare a method `Initialize()` to override the abstract interface
  - Line 80: We declare a method `ConnectInterrupt()` to override the abstract interface
  - Line 81: We declare a method `DisconnectInterrupt()` to override the abstract interface
  - Line 83: We declare a method `InterruptHandler()` to override the abstract interface
  - Line 85: We declare a method `AllOff()` to override the abstract interface
  - Line 87: We declare a method `DisableAllInterrupts()` to disable all GPIO interrupts by clearing the respective registers
- Line 92: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager

### GPIOManager.cpp {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_GPIOMANAGERCPP}

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
40: #include <baremetal/GPIOManager.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/InterruptHandler.h>
45: #include <baremetal/Logger.h>
46: #include <baremetal/MemoryAccess.h>
47: #include <baremetal/PhysicalGPIOPin.h>
48: #include <baremetal/Timer.h>
49: 
50: using namespace baremetal;
51: 
52: static const int WaitCycles = 150;
53: 
54: static const IRQ_ID GPIO_IRQ{IRQ_ID::IRQ_GPIO3}; // shared IRQ line for all GPIOs
55: 
56: static void GPIOInterruptHandler(void *param);
57: 
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\baremetal\src\GPIOManager.cpp
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
75: GPIOManager::GPIOManager(IMemoryAccess &memoryAccess)
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
108:     m_isInitialized  = true;
109: }
110: 
111: /// <summary>
112: /// Connect the GPIO pin interrupt for the specified pin
113: /// </summary>
114: /// <param name="pin">GPIO pin to connect interrupt for</param>
115: void GPIOManager::ConnectInterrupt(IGPIOPin *pin)
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
131: void GPIOManager::DisconnectInterrupt(const IGPIOPin *pin)
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
178:     uint8  pinNumber = 0;
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
236: void GPIOInterruptHandler(void *param)
237: {
238:     GPIOManager *pThis = reinterpret_cast<GPIOManager *>(param);
239:     assert(pThis != nullptr);
240: 
241:     pThis->InterruptHandler();
242: }
243: 
244: /// <summary>
245: /// Create a singleton GPIOManager if neededm and return the singleton instance
246: /// </summary>
247: /// <returns>A reference to the singleton GPIOManager.</returns>
248: GPIOManager &baremetal::GetGPIOManager()
249: {
250:     static GPIOManager control;
251:     control.Initialize();
252:     return control;
253: }
```

- Line 56: We forward declare the interrupt handler function
- Line 64-69: We implement the default constructor. This is straightforward
- Line 75-80: We implement the specialized constructor. This is straightforward
- Line 85-98: We implement the destructor
  - Line 88-91: We verify there are no lingering pins connected
  - Line 94-97: We unregister from the GPIO IRQ
- Line 103-109: We implement the method `Initialize()`
  - Line 105-106: We protect against multiple initialization, as we did before
  - Line 107-108: We register to the GPIO IRQ, and set the initialized flag
- Line 115-125: We implement the method `ConnectInterrupt()`
  - Line 117: We verify that the `GPIOManager` was initialized
  - Line 119: We verify that the pin pointer is valid
  - Line 120-121: We extract the pin number and verify that it is valid
  - Line 123-124: We verify that no interrupt handler was installed yet for the requested pin, and then save the pin
- Line 131-141: We implement the method `DisconnectInterrupt()`
  - Line 133: We verify that the `GPIOManager` was initialized
  - Line 135: We verify that the pin pointer is valid
  - Line 136-137: We extract the pin number and verify that it is valid
  - Line 139-140: We verify that a interrupt handler was installed for the requested pin, and then set the registration for the pin to nullptr
- Line 146-167: We implement the method `AllOff()`
  - Line 149-154: We set all pins to input mode
  - Line 156-162: We set all pins to pullmode off for Raspberry Pi 3
  - Line 164-165: We set all pins to pullmode off for Raspberry Pi 4 / 5
- Line 172-214: We implement the method `InterruptHandler()` in the class
  - Line 174: We verify that the `GPIOManager` was initialized
  - Line 176: We read the event status for GPIO 0..31
  - Line 178-191: We check for each GPIO number whether an event was set. When going beyond GPIO 31, we pick up the event status for interrupts 32..53/56.
For the first `1` bit we step out of this loop, as we only handle one GPIO per call.
The interrupt routine will be called again for other GPIO if needed
  - Line 195: We retrieve the pin
  - Line 196-204: If the pin is registered for interrupts, we call its `InterruptHandler`, and if auto acklowledge is request, perform an acknowledgement.
The latter resets the event status bit for the specified GPIO
  - Line 207-211: If the pin is not registered, we print an error, and reset the event bit for the pin
- Line 220-230: We implement the method `DisableAllInterrupts()`.
This removes all interrupts for the specified pins, by resetting the interrupt bits for each GPIO interrupt type
- Line 236-242: We implement the method `GPIOInterruptHandler()`.
This is the static entry point to the `GPIOManager` for interrupts, and relays to the `InterruptHandler()` method of the singleton instance
- Line 248-253: We implement the `GetGPIOManager()` function to return the singleton instance of the `GPIOManager`

### IGPIOPin.h {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_IGPIOPINH}

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

- Line 134: We declare a method `GetAutoAcknowledgeInterrupt` to retrieve the auto acknowledge setting
- Line 138: We declare a method `AcknowledgeInterrupt()` to perform GPIO acknowledgement, by resetting the event bit for the GPIO
- Line 142: We declare a method `InterruptHandler()` as the GPIO pin interrupt handler

### PhysicalGPIOPin.h {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_PHYSICALGPIOPINH}

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
90: inline GPIOInterruptTypes operator | (GPIOInterruptTypes lhs, GPIOInterruptTypes rhs)
91: {
92:     return static_cast<GPIOInterruptTypes>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs));
93: }
94: 
95: /// <summary>
96: /// GPIO pin interrupt handler
97: /// </summary>
98: using GPIOPinInterruptHandler = void(IGPIOPin* pin, void *param);
99: 
100: /// <summary>
101: /// Physical GPIO pin (i.e. available on GPIO header) 
102: /// </summary>
103: class PhysicalGPIOPin
104:     : public IGPIOPin
105: {
106: private:
107:     /// @brief Configured GPIO pin number (0..53)
108:     uint8                    m_pinNumber;
109:     /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
110:     GPIOMode                 m_mode;
111:     /// @brief Configured GPIO function.
112:     GPIOFunction             m_function;
113:     /// @brief Configured GPIO pull mode (only for input function).
114:     GPIOPullMode             m_pullMode;
115:     /// @brief Current value of the GPIO pin (true for on, false for off).
116:     bool                     m_value;
117:     /// @brief Memory access interface reference for accessing registers.
118:     IMemoryAccess&           m_memoryAccess;
119:     /// @brief Register offset for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
120:     unsigned                 m_regOffset;
121:     /// @brief Register mask for enabling interrupts, setting / clearing GPIO levels and checking GPIO level and interrupt events
122:     uint32                   m_regMask;
123:     /// @brief Interrupt handler for the pin
124:     GPIOPinInterruptHandler* m_handler;
125:     /// @brief Interrupt handler parameter for the pin
126:     void*                    m_handlerParam;
127:     /// @brief Auto acknowledge interrupt for the pin. If true, the interrupt handler will of the GPIOManager will automatically reset the event state
128:     bool                     m_autoAcknowledge;
129:     /// @brief GPIO interrupt types enabled
130:     uint8                    m_interruptMask;
131: 
132: public:
133:     PhysicalGPIOPin(IMemoryAccess &memoryAccess = GetMemoryAccess());
134: 
135:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess = GetMemoryAccess());
136: 
137:     uint8 GetPinNumber() const override;
138:     bool AssignPin(uint8 pinNumber) override;
139: 
140:     void On() override;
141:     void Off() override;
142:     bool Get() override;
143:     void Set(bool on) override;
144:     void Invert() override;
145:     bool GetEvent() override;
146:     void ClearEvent() override;
147: 
148:     GPIOMode GetMode();
149:     bool SetMode(GPIOMode mode);
150:     GPIOFunction GetFunction();
151:     GPIOPullMode GetPullMode();
152:     void SetPullMode(GPIOPullMode pullMode);
153: 
154:     bool GetAutoAcknowledgeInterrupt() const override;
155:     void AcknowledgeInterrupt() override;
156:     void InterruptHandler() override;
157: 
158:     void ConnectInterrupt(GPIOPinInterruptHandler *handler, void *param, bool autoAcknowledge = true);
159:     void DisconnectInterrupt();
160: 
161:     void EnableInterrupt(GPIOInterruptTypes interruptTypes);
162:     void DisableInterrupt(GPIOInterruptTypes interruptTypes);
163:     void DisableAllInterrupts();
164: 
165: private:
166:     void SetFunction(GPIOFunction function);
167: };
168: 
169: } // namespace baremetal
```

- Line 65-83: We change the GPIO interrupt types from a regular enum to a set of bit fields, so they can be combined
- Line 90-93: We define an operator `|` to combine interrupt types
- Line 98: We declare the GPIO pin interrupt handler type. This will not only receive a parameter, but also the pin instance itself
- Line 124: We add a member variable `m_handler` for the GPIO pin interrupt handler
- Line 126: We add a member variable `m_handlerParam` for the GPIO pin interrupt handler parameter
- Line 128: We add a member variable `m_autoAcknowledge` to enable auto acknowledge for the GPIO pin
- Line 130; We change the `m_interruptEnabled` field to a simple bit flag `m_interruptMask`
- Line 154: We declare a method `GetAutoAcknowledgeInterrupt()` to retrieve the auto acknowledge setting
- Line 155: We declare a method `AcknowledgeInterrupt()` to perform GPIO acknowledgement, by resetting the event bit for the GPIO
- Line 156: We declare a method `InterruptHandler()` as the GPIO pin interrupt handler
- Line 158: We declare a method `ConnectInterrupt()` to connect the GPIO pint interrupt with its parameter, and set the auto acknowledge status
- Line 159: We declare a method `DisconnectInterrupt()` to disconnect the GPIO pint interrupt

### PhysicalGPIOPin.cpp {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_PHYSICALGPIOPINCPP}

We need to implement the new methods, and change the way we deal with the interrupt type registration.

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
...
40: #include <baremetal/PhysicalGPIOPin.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/GPIOManager.h>
45: #include <baremetal/Logger.h>
46: #include <baremetal/MemoryAccess.h>
47: #include <baremetal/Timer.h>
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
119: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess &memoryAccess /*= GetMemoryAccess()*/)
120:     : m_pinNumber{ NUM_GPIO }
121:     , m_mode{ GPIOMode::Unknown }
122:     , m_function{ GPIOFunction::Unknown }
123:     , m_pullMode{ GPIOPullMode::Unknown }
124:     , m_value{}
125:     , m_memoryAccess{ memoryAccess }
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
141: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess /*= m_memoryAccess*/)
142:     : m_pinNumber{ NUM_GPIO }
143:     , m_mode{ GPIOMode::Unknown }
144:     , m_value{}
145:     , m_memoryAccess{ memoryAccess }
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
179:     m_regMask   = 1 << (m_pinNumber % 32);
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
230:     if (m_mode == GPIOMode::Output)
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
367:     uint32  shift = m_pinNumber % 32;
368: 
369:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
370:     Timer::WaitCycles(NumWaitCycles);
371:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
372:     Timer::WaitCycles(NumWaitCycles);
373:     m_memoryAccess.Write32(clkRegister, 0);
374: #else
375:     regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16) * 4;
376:     unsigned              shift = (m_pinNumber % 16) * 2;
377: 
378:     static const unsigned ModeMap[3] = { 0, 2, 1 };
379: 
380:     uint32                value = m_memoryAccess.Read32(modeReg);
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
395: void PhysicalGPIOPin::ConnectInterrupt(GPIOPinInterruptHandler *handler, void *param, bool autoAcknowledge /*= true*/)
396: {
397:     assert((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown));
398: 
399:     assert(m_interruptMask == static_cast<uint8>(GPIOInterruptTypes::None));
400: 
401:     assert(handler != nullptr);
402:     assert(m_handler == nullptr);
403:     m_handler = handler;
404:     m_handlerParam   = param;
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
465:         case GPIOInterruptTypes::RisingEdge:
466:             return GPIOInterruptTypeOffset::OffsetRisingEdge;
467:         case GPIOInterruptTypes::FallingEdge:
468:             return GPIOInterruptTypeOffset::OffsetFallingEdge;
469:         case GPIOInterruptTypes::HighLevel:
470:             return GPIOInterruptTypeOffset::OffsetHighLevel;
471:         case GPIOInterruptTypes::LowLevel:
472:             return GPIOInterruptTypeOffset::OffsetLowLevel;
473:         case GPIOInterruptTypes::AsyncRisingEdge:
474:             return GPIOInterruptTypeOffset::OffsetAsyncRisingEdge;
475:         case GPIOInterruptTypes::AsyncFallingEdge:
476:             return GPIOInterruptTypeOffset::OffsetAsyncFallingEdge;
477:         default:
478:             return GPIOInterruptTypeOffset::OffsetRisingEdge;
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
549:     uint32  shift = (m_pinNumber % 10) * 3;
550: 
551:     static const unsigned FunctionMap[] = { 0, 1, 4, 5, 6, 7, 3, 2 };
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
- Line 45: As we want to do logging we also include for that
- Line 53: We define the log name
- Line 58-72: We add an enum for the offset of the register for all the GPIO interrupt types
- Line 119-133: We update the 'default' constructor to initialize the new member variables
- Line 141-155: We also update the specific constructor to initialize the new member variables
- Line 395-408: We implement the method `ConnectInterrupt()`
  - Line 397: We verify that the GPIO pin is set to input mode
  - Line 399: We verify that no interrupt types are registered yet
  - Line 401-402: We verify that the handler passed is valid, and that no handler is set yet
  - Line 403-405: We save the handler function, its parameter and the auto acknowledge setting
  - Line 407: We connect the interrupt for the GPIO pin
- Line 413-422: We implement the method `DisconnectInterrupt()`
  - Line 415: We verify that the GPIO pin is set to input mode
  - Line 417: We remove all interrupt types for the GPIO
  - Line 419: We verify reset the handler
  - Line 421: We disconnect the interrupt for the GPIO pin
- Line 428-431: We implement the method `GetAutoAcknowledgeInterrupt()`
- Line 436-441: We implement the method `AcknowledgeInterrupt()`, resetting the event status bit for the GPIO
- Line 446-454: We implement the method `InterruptHandler()`
- Line 461-480: We convert a GPIO interrupt type to a register offset (relative to the rising edge interrupt register)
- Line 486-503: We update the method `EnableInterrupt()` to take a combination of GPIO interrupt types
  - Line 490-491: We convert the mask to byte and verify that only allowed bits are set
  - Line 492-502: We with a pattern for `AsyncFallingEdge` (the highest), and for each pattern check whether the bit is set.
If so, we add it to the interrupt mask, calculate the register address, and set the corresponding bit in the interrupt enable register
- Line 509-524: We update the method `DisableInterrupt()` to take a combination of GPIO interrupt types
  - Line 511-512: We convert the mask to byte and verify that only allowed bits are set
  - Line 513-523: We with a pattern for `AsyncFallingEdge` (the highest), and for each pattern check whether the bit is set.
If so, we remove it from the interrupt mask, calculate the register address, and reset the corresponding bit in the interrupt enable register
- Line 529-532: We update the method `DisableAllInterrupts()` to simply call `DisableInterrupt()` with all interrupt types combined

### InterruptHandler.cpp {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_INTERRUPTHANDLERCPP}

We'd like to see what is happening concerning registration and unregistration of IRQ and FIQ handlers.
Let's add this. We need to take into account however, that the `Logger` class needs the `Timer` class, which already registers for an IRQ while staring up.
So we need a way to check whether the `Logger` instance is already created.
We'll also add logging to the `Shutdown()` method, but we'll skip the `Initialize()` method, as that will definitely be called before the `Logger` is actually initialized.

Update the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
...
147: /// <summary>
148: /// Shutdown interrupt system, disable all
149: /// </summary>
150: void InterruptSystem::Shutdown()
151: {
152:     if (Logger::HaveLogger())
153:         LOG_INFO("InterruptSystem::Shutdown");
154:     DisableIRQs();
155: 
156:     DisableInterrupts();
157:     m_isInitialized = false;
158: }
159: 
160: /// <summary>
161: /// Disable all IRQ interrupts
162: /// </summary>
163: void InterruptSystem::DisableInterrupts()
164: {
165:     if (Logger::HaveLogger())
166:         LOG_DEBUG("InterruptSystem::DisableInterrupts");
167: #if BAREMETAL_RPI_TARGET == 3
168:     m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
169: 
170:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
171:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
172:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
173:     m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
174: #else
175:     // initialize distributor:
176: 
177:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
178:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_DISABLE);
179:     // disable, acknowledge and deactivate all interrupts
180:     for (unsigned n = 0; n < IRQ_LINES / 32; n++)
181:     {
182:         m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
183:         m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
184:         m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
185:     }
186: #endif
187: }
188: 
189: /// <summary>
190: /// Enable IRQ interrupts
191: /// </summary>
192: void InterruptSystem::EnableInterrupts()
193: {
194:     if (Logger::HaveLogger())
195:         LOG_DEBUG("InterruptSystem::EnableInterrupts");
196: #if BAREMETAL_RPI_TARGET == 3
197: #else
198:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
199:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
200: #endif
201: }
202: 
203: /// <summary>
204: /// Enable and register an IRQ handler
205: ///
206: /// Enable the IRQ with specified index, and register its handler.
207: /// </summary>
208: /// <param name="irqID">IRQ ID</param>
209: /// <param name="handler">Handler to register for this IRQ</param>
210: /// <param name="param">Parameter to pass to IRQ handler</param>
211: void InterruptSystem::RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param)
212: {
213:     uint32 irq = static_cast<int>(irqID);
214:     assert(irq < IRQ_LINES);
215:     if (Logger::HaveLogger())
216:         LOG_DEBUG("InterruptSystem::RegisterIRQHandler IRQ=%d", irq);
217:     assert(m_irqHandlers[irq] == nullptr);
218: 
219:     EnableIRQ(irqID);
220: 
221:     m_irqHandlers[irq] = handler;
222:     m_irqHandlersParam[irq] = param;
223: }
224: 
225: /// <summary>
226: /// Disable and unregister an IRQ handler
227: ///
228: /// Disable the IRQ with specified index, and unregister its handler.
229: /// </summary>
230: /// <param name="irqID">IRQ ID</param>
231: void InterruptSystem::UnregisterIRQHandler(IRQ_ID irqID)
232: {
233:     uint32 irq = static_cast<int>(irqID);
234:     assert(irq < IRQ_LINES);
235:     if (Logger::HaveLogger())
236:         LOG_DEBUG("InterruptSystem::UnregisterIRQHandler IRQ=%d", irq);
237:     assert(m_irqHandlers[irq] != nullptr);
238: 
239:     m_irqHandlers[irq] = nullptr;
240:     m_irqHandlersParam[irq] = nullptr;
241: 
242:     DisableIRQ(irqID);
243: }
244: 
245: /// <summary>
246: /// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
247: /// </summary>
248: /// <param name="fiqID">FIQ interrupt number</param>
249: /// <param name="handler">FIQ interrupt handler</param>
250: /// <param name="param">FIQ interrupt data</param>
251: // cppcheck-suppress unusedFunction
252: void InterruptSystem::RegisterFIQHandler(FIQ_ID fiqID, FIQHandler *handler, void *param)
253: {
254:     uint32 fiq = static_cast<int>(fiqID);
255:     assert(fiq <= IRQ_LINES);
256:     if (Logger::HaveLogger())
257:         LOG_DEBUG("InterruptSystem::RegisterFIQHandler IRQ=%d", fiq);
258:     assert(handler != nullptr);
259:     assert(s_fiqData.handler == nullptr);
260: 
261:     s_fiqData.handler = handler;
262:     s_fiqData.param   = param;
263:     s_fiqData.fiqID   = fiq;
264: 
265:     EnableFIQ(fiqID);
266: }
267: 
268: /// <summary>
269: /// Disable and unregister a FIQ interrupt handler
270: /// </summary>
271: /// <param name="fiqID">FIQ interrupt number</param>
272: void InterruptSystem::UnregisterFIQHandler(FIQ_ID fiqID)
273: {
274:     uint32 fiq = static_cast<int>(fiqID);
275:     assert(s_fiqData.handler != nullptr);
276:     assert(s_fiqData.fiqID == fiq);
277:     if (Logger::HaveLogger())
278:         LOG_DEBUG("InterruptSystem::UnregisterFIQHandler IRQ=%d", fiq);
279:     DisableFIQ(fiqID);
280: 
281:     s_fiqData.handler = nullptr;
282:     s_fiqData.param   = nullptr;
283: }
...
```

- Line 152-153: We update the method `Shutdown()` to log, but only when a logger is instantiated and initialized
- Line 165-166: We update the method `DisableInterrupts()` to log, but only when a logger is instantiated and initialized
- Line 1194-195: We update the method `EnableInterrupts()` to log, but only when a logger is instantiated and initialized
- Line 215-216: We update the method `RegisterIRQHandler()` to log, but only when a logger is instantiated and initialized
- Line 235-236: We update the method `UnregisterIRQHandler()` to log, but only when a logger is instantiated and initialized
- Line 256-257: We update the method `RegisterFIQHandler()` to log, but only when a logger is instantiated and initialized
- Line 277-278: We update the method `UnregisterFIQHandler()` to log, but only when a logger is instantiated and initialized

### Logger.h {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_LOGGERH}

The changed usage of the `Logger` class means we need to be able to access the singleton instance. We therefore change it into a static pointer.

Update the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
...
73: class Logger
74: {
75:     /// <summary>
76:     /// Construct the singleton Logger instance if needed, and return a reference to the instance. This is a friend function of class Logger
77:     /// </summary>
78:     /// <returns>Reference to the singleton logger instance</returns>
79:     friend Logger &GetLogger();
80: 
81: private:
82:     /// @brief True if class is already initialized
83:     bool        m_isInitialized;
84:     /// @brief Pointer to timer instance
85:     Timer      *m_timer;
86:     /// @brief Reference to console instance
87:     Console    &m_console;
88:     /// @brief Currently set logging severity level
89:     LogSeverity m_level;
90:     /// @brief Pointer to singleton logger. To be use to check whether Logger was already instantiated
91:     static Logger* s_logger;
92: 
93:     explicit Logger(LogSeverity logLevel, Timer *timer = nullptr, Console &console = GetConsole());
94: 
95: public:
96:     static bool HaveLogger();
97: 
98:     bool Initialize();
99:     void SetLogLevel(LogSeverity logLevel);
100: 
101:     void Write(const char *source, int line, LogSeverity severity, const char *message, ...);
102:     void WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args);
103: 
104:     void WriteNoAlloc(const char* source, int line, LogSeverity severity, const char* message, ...);
105:     void WriteNoAllocV(const char* source, int line, LogSeverity severity, const char* message, va_list args);
106: };
...
```

- Line 91: We add a static `Logger` pointer to hold the singleton instance
- Line 96: We declare a static method `HaveLogger()` to determine if we have an initialized `Logger` instance

### Logger.cpp {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_LOGGERCPP}

We need a way to check whether the `Logger` instance is already created.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/InterrupLoggertHandler.cpp
...
56: /// @brief Define log name
57: LOG_MODULE("Logger");
58: 
59: Logger* Logger::s_logger{};
60: 
61: /// <summary>
62: /// Construct a logger
63: /// </summary>
64: /// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
65: /// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
66: /// <param name="console">Console to print to, defaults to the singleton console instance</param>
67: Logger::Logger(LogSeverity logLevel, Timer *timer /*= nullptr*/, Console &console /*= GetConsole()*/)
68:     : m_isInitialized{}
69:     , m_timer{timer}
70:     , m_console{console}
71:     , m_level{logLevel}
72: {
73: }
74: 
75: /// <summary>
76: /// Check whether the singleton logger was instantiated and initialized
77: /// </summary>
78: /// <returns>Returns true if the singleton logger instance is created and initialized, false otherwise</returns>
79: bool Logger::HaveLogger()
80: {
81:     if (s_logger == nullptr)
82:         return false;
83:     return s_logger->m_isInitialized;
84: }
85: 
...
315: /// <summary>
316: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
317: /// </summary>
318: /// <returns>Reference to the singleton logger instance</returns>
319: Logger &baremetal::GetLogger()
320: {
321:     if (Logger::s_logger == nullptr)
322:     {
323:         Logger::s_logger = new Logger(LogSeverity::Debug, &GetTimer());
324:         Logger::s_logger->Initialize();
325: 
326:     }
327:     return *Logger::s_logger;
328: }
```

- Line 59: We initialize the static class variable `s_logger`
- Line 79-84: We implement the static method `HaveLogger()`
- Line 319-328: We change the implement of `GetLogger()` to create a `Logger` instance through the new operator

### Update application code {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_UPDATE_APPLICATION_CODE}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/InterruptHandler.h>
5: #include <baremetal/Interrupts.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/PhysicalGPIOPin.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
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
51: 
52:     auto exceptionLevel = CurrentEL();
53:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
54: 
55:     PhysicalGPIOPin pinCLK(11, GPIOMode::InputPullUp);
56:     PhysicalGPIOPin pinDT(9, GPIOMode::InputPullUp);
57:     PhysicalGPIOPin pinSW(10, GPIOMode::InputPullUp);
58: 
59:     pinCLK.ConnectInterrupt(InterruptHandlerCLK, nullptr);
60:     pinCLK.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
61:     pinDT.ConnectInterrupt(InterruptHandlerDT, nullptr);
62:     pinDT.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
63:     pinSW.ConnectInterrupt(InterruptHandlerSW, nullptr);
64:     pinSW.EnableInterrupt(GPIOInterruptTypes::RisingEdge | GPIOInterruptTypes::FallingEdge);
65: 
66:     LOG_INFO("Wait 5 seconds");
67:     Timer::WaitMilliSeconds(5000);
68: 
69:     GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_GPIO3);
70: 
71:     pinCLK.DisconnectInterrupt();
72:     pinDT.DisconnectInterrupt();
73:     pinSW.DisconnectInterrupt();
74: 
75:     console.Write("Press r to reboot, h to halt\n");
76:     char ch{};
77:     while ((ch != 'r') && (ch != 'h'))
78:     {
79:         ch = console.ReadChar();
80:         console.WriteChar(ch);
81:     }
82: 
83:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
84: }
```

- Line 15-24: We add a separate GPIO interrupt handler for the CLK pin
- Line 26-35: We add a separate GPIO interrupt handler for the DT pin
- Line 37-46: We add a separate GPIO interrupt handler for the SW pin
- Line 59-60: We set up the interrupt handler for the CLK pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 61-62: We set up the interrupt handler for the DT pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 63-64: We set up the interrupt handler for the SW pin in a new way through the `GPIOManager`, using the combined `RisingEdge` and `FallingEdge` types
- Line 71-73: We disconnect the interrupts using the new method, which goes through the `GPIOManager`

### Update CMake file {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_UPDATE_CMAKE_FILE}

As we have added some source files to the `baremetal` library, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`
```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionStub.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/GPIOManager.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/InterruptHandler.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Interrupts.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Malloc.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
53:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
55:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
56:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
57:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
58:     )
59: 
60: set(PROJECT_INCLUDES_PUBLIC
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMRegisters.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Exception.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/GPIOManager.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOManager.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/InterruptHandler.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Interrupts.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Iterator.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Malloc.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
91:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
92:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Synchronization.h
93:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
94:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
95:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
96:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
97:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
98:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
99:     )
100: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_20_GPIO_GENERAL_APPROACH_FOR_GPIO_INTERRUPTS___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

When we press the switch or turn it, you can see the interrupts coming in.

```text
Info   0.00:00:00.030 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
Info   0.00:00:00.060 Starting up (System:209)
Info   0.00:00:00.080 Current EL: 1 (main:53)
Debug  0.00:00:00.100 InterruptSystem::RegisterIRQHandler IRQ=52 (InterruptHandler:216)
Info   0.00:00:00.130 Wait 5 seconds (main:66)
Debug  0.00:00:01.250 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.250 GPIO CLK (main:17)
Debug  0.00:00:01.250 CLK=1 (main:21)
Debug  0.00:00:01.320 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.320 GPIO DT (main:28)
Debug  0.00:00:01.320 DT=1 (main:32)
Debug  0.00:00:01.620 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.620 GPIO CLK (main:17)
Debug  0.00:00:01.620 CLK=0 (main:21)
Debug  0.00:00:01.690 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.690 GPIO DT (main:28)
Debug  0.00:00:01.690 DT=1 (main:32)
Debug  0.00:00:01.790 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.790 GPIO CLK (main:17)
Debug  0.00:00:01.790 CLK=1 (main:21)
Debug  0.00:00:01.850 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:01.850 GPIO DT (main:28)
Debug  0.00:00:01.850 DT=1 (main:32)
Debug  0.00:00:02.150 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.150 GPIO DT (main:28)
Debug  0.00:00:02.150 DT=0 (main:32)
Debug  0.00:00:02.220 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.220 GPIO CLK (main:17)
Debug  0.00:00:02.220 CLK=0 (main:21)
Debug  0.00:00:02.400 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.400 GPIO CLK (main:17)
Debug  0.00:00:02.400 CLK=1 (main:21)
Debug  0.00:00:02.470 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.470 GPIO DT (main:28)
Debug  0.00:00:02.470 DT=1 (main:32)
Debug  0.00:00:02.680 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.680 GPIO DT (main:28)
Debug  0.00:00:02.680 DT=1 (main:32)
Debug  0.00:00:02.750 Interrupt for GPIO pin 11 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.750 GPIO CLK (main:17)
Debug  0.00:00:02.750 CLK=0 (main:21)
Debug  0.00:00:02.820 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:02.820 GPIO DT (main:28)
Debug  0.00:00:02.820 DT=1 (main:32)
Debug  0.00:00:03.860 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:03.860 GPIO DT (main:28)
Debug  0.00:00:03.860 DT=0 (main:32)
Debug  0.00:00:03.920 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:449)
Debug  0.00:00:03.920 GPIO SW (main:39)
Debug  0.00:00:03.920 SW=0 (main:43)
Debug  0.00:00:04.090 Interrupt for GPIO pin 10 handled (PhysicalGPIOPin:449)
Debug  0.00:00:04.090 GPIO SW (main:39)
Debug  0.00:00:04.090 SW=1 (main:43)
Debug  0.00:00:04.170 Interrupt for GPIO pin 9 handled (PhysicalGPIOPin:449)
Debug  0.00:00:04.170 GPIO DT (main:28)
Debug  0.00:00:04.170 DT=1 (main:32)
Debug  0.00:00:05.100 InterruptSystem::UnregisterIRQHandler IRQ=52 (InterruptHandler:236)
Press r to reboot, h to halt
```

## Adding intelligence to the switch button - Step 4 {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4}

So, now we can read the rotary switch signals, we attached interrupts so we can handle changes.
Let's make the rotare switch a bit smarter.

We'll add a new class in a new library device `KY040` for this, and as a first step, we'll add the possibility to distinguish between several event of the switch button:

- Single press
- Press and hold
- Double press
- Tripple press

### Adding a new library {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_ADDING_A_NEW_LIBRARY}

Let's add a new library for specific hardware devices.
We'll create a new library `device`, and add an extra layer of directories to distignuish between e.g. GPIO and I2C devices.
For now we'll be adding a device underneath gpio:
- We create a directory libraries/device
- We create a directory libraries/device/include/device/gpio
- We create a directory libraries/device/src/gpio

The new library wil be next to `baremetal` and `stdlib`, and have the following structure:

<img src="images/treeview-device-library.png" alt="Initial project structure" width="300"/>

### CMake file for device library {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_CMAKE_FILE_FOR_DEVICE_LIBRARY}

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
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/KY-040.cpp
32:     )
33: 
34: set(PROJECT_INCLUDES_PUBLIC
35:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/gpio/KY-040.h
36:     )
37: set(PROJECT_INCLUDES_PRIVATE )
38: 
39: if (CMAKE_VERBOSE_MAKEFILE)
40:     display_list("Package                           : " ${PROJECT_NAME} )
41:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
42:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
43:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
44:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
45:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
46:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
47:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
48:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
49:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
50:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
51:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
52:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
53:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
54:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
55:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
56:     display_list("Link libs                         : " ${PROJECT_LIBS} )
57:     display_list("Source files                      : " ${PROJECT_SOURCES} )
58:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
59:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
60: endif()
61: 
62: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
63: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
64: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
65: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
66: target_compile_definitions(${PROJECT_NAME} PRIVATE
67:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
68:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
69:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
70:     )
71: target_compile_definitions(${PROJECT_NAME} PUBLIC
72:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
73:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
74:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
75:     )
76: target_compile_options(${PROJECT_NAME} PRIVATE
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
80:     )
81: target_compile_options(${PROJECT_NAME} PUBLIC
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
85:     )
86: 
87: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
88: 
89: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
90: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
91:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
92: endif()
93: 
94: link_directories(${LINK_DIRECTORIES})
95: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
96: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
97: 
98: show_target_properties(${PROJECT_NAME})
```

As you can see we added a header and a source file, which we'll have to implement.

### Update CMake file for libraries {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_UPDATE_CMAKE_FILE_FOR_LIBRARIES}

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

### KY-040.h {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_KY_040H}

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
42: #include <stdlib/Types.h>
43: #include <baremetal/PhysicalGPIOPin.h>
44: #include <baremetal/Timer.h>
45: 
46: /// @file
47: /// Abstract GPIO pin. Could be either a virtual or physical pin
48: 
49: namespace device {
50: 
51: enum class SwitchButtonEvent;
52: enum class SwitchButtonState;
53: 
54: /// <summary>
55: /// KY-040 rotary switch device
56: /// </summary>
57: class KY040
58: {
59: public:
60:     /// <summary>
61:     /// Events generated by the rotary switch
62:     /// </summary>
63:     enum class Event
64:     {
65:         /// @brief Switch is pressed
66:         SwitchDown,
67:         /// @brief Switch is released
68:         SwitchUp,
69:         /// @brief Switch is clicked (short press / release cycle)
70:         SwitchClick,
71:         /// @brief Switch is clicked twice in a short time
72:         SwitchDoubleClick,
73:         /// @brief Switch is clicked three time in a short time
74:         SwitchTripleClick,
75:         /// @brief Switch is held down for a longer time
76:         SwitchHold, ///< generated each second
77:         /// @brief Unknown event
78:         Unknown
79:     };
80: 
81:     /// <summary>
82:     /// Pointer to event handler function to be registered by an application
83:     /// </summary>
84:     using EventHandler = void(Event event, void *param);
85: 
86: private:
87:     /// @brief True if the rotary switch was initialized
88:     bool                            m_isInitialized;
89:     /// @brief GPIO pin for CLK input
90:     baremetal::PhysicalGPIOPin      m_clkPin;
91:     /// @brief GPIO pin for DT input
92:     baremetal::PhysicalGPIOPin      m_dtPin;
93:     /// @brief GPIO pin for SW input (switch button)
94:     baremetal::PhysicalGPIOPin      m_swPin;
95:     /// @brief Internal state of the switch button (to tracking single, double, triple clicking and hold
96:     SwitchButtonState               m_switchButtonState;
97:     /// @brief Handle to timer for debouncing the switch button
98:     baremetal::KernelTimerHandle    m_debounceTimerHandle;
99:     /// @brief Handle to timer for handling button press ticks (for hold)
100:     baremetal::KernelTimerHandle    m_tickTimerHandle;
101:     /// @brief Time at which the current button press occurred
102:     unsigned                        m_currentPressTicks;
103:     /// @brief Time at which the current button release occurred
104:     unsigned                        m_currentReleaseTicks;
105:     /// @brief Time at which the last button press occurred
106:     unsigned                        m_lastPressTicks;
107:     /// @brief Time at which the last button release occurred
108:     unsigned                        m_lastReleaseTicks;
109: 
110:     /// @brief Registered event handler
111:     EventHandler*                   m_eventHandler;
112:     /// @brief Parameter for registered event handler
113:     void*                           m_eventHandlerParam;
114: 
115: public:
116:     KY040(uint8 clkPin, uint8 dtPin, uint8 swPin);
117:     virtual ~KY040();
118: 
119:     void               Initialize();
120:     void               Uninitialize();
121: 
122:     void               RegisterEventHandler(EventHandler *handler, void *param);
123:     void               UnregisterEventHandler(EventHandler *handler);
124:     static const char *EventToString(Event event);
125: 
126: private:
127:     static void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin, void *param);
128:     void        SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin);
129:     static void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void *param, void *context);
130:     void        SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void *param);
131:     static void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void *param, void *context);
132:     void        SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void *param);
133:     void        HandleSwitchButtonEvent(SwitchButtonEvent switchEvent);
134: };
135: 
136: } // namespace device
```

Notice that the class is declared in the `device` namespace, which we will use for all classes in the device library.

- Line 51: We forward declare an enum `SwitchButtonEvent`, which will hold the internal switch button event (up, down, click, doubleclick, tick in case of holding the button down)
- Line 52: We forward declare an enum `SwitchButtonState`, which will hold the internal switch button state (which keeps track of the state machine for the button switch)
- Line 57-135: We declare the class `KY040`
  - Line 63-79: We declare the enum `Event` which will hold the event actually sent to a registered event handler (down, up, click, double click, triple click, hold)
  - Line 84: We declare the type for the event handler that can registered to
  - Line 88: We declare the member variable `m_isInitialized` to guard against multiple initialization
  - Line 90: We declare the member variable `m_clkPin` which is the GPIO pin for the CLK GPIO
  - Line 92: We declare the member variable `m_dtPin` which is the GPIO pin for the DT GPIO
  - Line 94: We declare the member variable `m_swPin` which is the GPIO pin for the SW GPIO
  - Line 96: We declare the member variable `m_switchButtonState` which is the internal switch button state
  - Line 98: We declare the member variable `m_debounceTimerHandle` which is a handle to the time to perform debouncing of the switch button
  - Line 100: We declare the member variable `m_tickTimerHandle` which is a handle to the hold tick timer
  - Line 102: We declare the member variable `m_currentPressTicks` which is the time the current switch button press happened (in timer ticks)
  - Line 104: We declare the member variable `m_currentReleaseTicks` which is the time the current switch button release happened (in timer ticks).
This is used to detect clicks
  - Line 106: We declare the member variable `m_lastPressTicks` which is the time the last switch button press happened (in timer ticks).
This is used to detect double and triple clicks
  - Line 108: We declare the member variable `m_lastReleaseTicks` which is the time the last switch button release happened (in timer ticks)
  - Line 111: We declare the member variable `m_eventHandler` which is the time the registered event handler
  - Line 113: We declare the member variable `m_eventHandlerParam` which is the parameter to be passed to the event handler
  - Line 116: We declare the constructor, which takes GPIO pin numbers for the CLK, DT and SW pin respectively
  - Line 117: We declare the desctructor
  - Line 119: We declare the method `Initialize()` to initialize the rotary switch
  - Line 120: We declare the method `Uninitialize()` to uninitialize the rotary switch
  - Line 122: We declare the method `RegisterEventHandler()` to register an event handler
  - Line 123: We declare the method `UnregisterEventHandler()` to unregister an event handler
  - Line 124: We declare the method `EventToString()` to convert an event to a string
  - Line 127: We declare the static private method `SwitchButtonInterruptHandler()` which is the GPIO pin interrupt handler
As the parameter will point to the class instance, it will call the class method `SwitchButtonInterruptHandler()`
  - Line 128: We declare the private method `SwitchButtonInterruptHandler()` to handle debouncing of the switch button
  - Line 129: We declare the static private method `SwitchButtonDebounceHandler()` which is the timer timeout handler for switch button debouncing.
As the context parameter will point to the class instance, it will call the class method `SwitchButtonDebounceHandler()`
  - Line 130: We declare the private method `SwitchButtonDebounceHandler()` to handle debouncing of the switch button
  - Line 131: We declare the static private method `SwitchButtonTickHandler()` which is the timer timeout handler for switch button holding.
As the context parameter will point to the class instance, it will call the class method `SwitchButtonTickHandler()`
  - Line 132: We declare the private method `SwitchButtonTickHandler()` to handle holding down the switch button
  - Line 133: We declare the private method `HandleSwitchButtonEvent()` which deals with internal state keeping

### KY-040.cpp {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_KY_040CPP}

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
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: 
45: /// @brief Define log name
46: LOG_MODULE("KY-040");
47: 
48: using namespace baremetal;
49: 
50: namespace device {
51: 
52: /// @brief Time delay for debounding switch button
53: static const unsigned SwitchDebounceDelayMilliseconds       = 50;
54: /// @brief Tick delay for determining if switch button was held down
55: static const unsigned SwitchTickDelayMilliseconds           = 1000;
56: /// @brief Maximum delay between press and release for a click
57: static const unsigned SwitchClickMaxDelayMilliseconds       = 400;
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
108: const char *KY040::EventToString(KY040::Event event)
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
136: static const char *SwitchButtonEventToString(SwitchButtonEvent event)
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
162: static const char *SwitchButtonStateToString(SwitchButtonState state)
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
195:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::Unknown},            // SwitchButtonState::Start
196:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchHold},         // SwitchButtonState::Down
197:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchClick},        // SwitchButtonState::Click
198:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick},  // SwitchButtonState::Click2
199:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick},  // SwitchButtonState::Click3
200:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::SwitchHold},         // SwitchButtonState::Hold
201:     {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::Unknown}             // SwitchButtonState::Invalid
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
222:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click, SwitchButtonState::Click2, SwitchButtonState::Start},   // SwitchButtonState::Start
223:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click, SwitchButtonState::Click2, SwitchButtonState::Hold},    // SwitchButtonState::Down
224:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click, SwitchButtonState::Click2, SwitchButtonState::Invalid}, // SwitchButtonState::Click
225:     {SwitchButtonState::Down, SwitchButtonState::Click2, SwitchButtonState::Click2, SwitchButtonState::Click3, SwitchButtonState::Hold},  // SwitchButtonState::Click2
226:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click3, SwitchButtonState::Click3, SwitchButtonState::Hold},   // SwitchButtonState::Click3
227:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click, SwitchButtonState::Click2, SwitchButtonState::Hold},    // SwitchButtonState::Hold
228:     {SwitchButtonState::Down, SwitchButtonState::Start, SwitchButtonState::Click, SwitchButtonState::Click2, SwitchButtonState::Invalid}  // SwitchButtonState::Invalid
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
267: /// KY040 class destructor
268: /// </summary>
269: KY040::~KY040()
270: {
271:     LOG_DEBUG("KY040 destructor");
272:     Uninitialize();
273: }
274: 
275: /// <summary>
276: /// Initialized the KY040 rotary switch
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
318: void KY040::RegisterEventHandler(EventHandler *handler, void *param)
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
330: void KY040::UnregisterEventHandler(EventHandler *handler)
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
342: void KY040::SwitchButtonInterruptHandler(IGPIOPin* pin, void *param)
343: {
344:     KY040 *pThis = reinterpret_cast<KY040 *>(param);
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
387: void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void *param, void *context)
388: {
389:     KY040 *pThis = reinterpret_cast<KY040 *>(context);
390:     assert(pThis != nullptr);
391:     pThis->SwitchButtonDebounceHandler(handle, param);
392: }
393: 
394: /// <summary>
395: /// Switch button debounce handler, called by the global switch button debounce handler on timeout
396: /// </summary>
397: /// <param name="handle">Kernel timer handle</param>
398: /// <param name="param">Timer handler parameter</param>
399: void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void *param)
400: {
401:     LOG_DEBUG("KY040 Timeout debounce timer");
402:     m_debounceTimerHandle = 0;
403: 
404:     bool swValue     = m_swPin.Get();
405:     auto event       = swValue ? Event::SwitchUp : Event::SwitchDown;
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
461: void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void *param, void *context)
462: {
463:     KY040 *pThis = reinterpret_cast<KY040 *>(context);
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
474: void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void *param)
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

- Line 53: We define the timeout value to be used for debouncing in milliseconds
- Line 55: We define the timeout value to be used for detecting holding down the switch button in milliseconds
- Line 57: We define the timeout value to be used for detecting clicks (short push/release cycle) in milliseconds
- Line 59: We define the timeout value to be used for detecting double and triple clicks in milliseconds
- Line 64-78: We declare the `SwitchButtonEvent` enum which is used to keep track of events internally
- Line 83-101: We declare the `SwitchButtonState` enum which is used to keep track of the switch state internally
- Line 108-129: We implement the method `EventToString()`
- Line 136-155: We implement a local function `SwitchButtonEventToString()` to convert the internal event to a string, for debugging
- Line 162-185: We implement a local function `SwitchButtonStateToString()` to convert the internal switch button state to a string, for debugging
- Line 192-202: We define a matrix `s_switchOutput` to determine the event to be generated when an event happens in a certain state. This is part of the state machine.
I'll not go into details here, I'll leave it to you to figure this out
- Line 209-212: We implement a local function `GetSwitchOutput()` which uses the `s_switchOutput` variable to determine the event to generate
- Line 219-229: We define a matrix `s_nextSwitchState` to determine the next state when an event happens in a certain state. This is part of the state machine.
Again, I'll not go into details here, I'll leave it to you to figure this out
- Line 236-239: We implement a local function `GetSwitchNextState()` which uses the `s_nextSwitchState` variable to determine the next state
- Line 247-264: We implement the constructor. This is quite straightforward
- Line 269-273: We implement the desctructor. This is quite straightforward
- Line 278-289: We implement the `Initialize()` method.
For now it only connects the interrupt for the SW GPIO, for both rising and falling edges
- Line 294-311: We implement the `Uninitialize()` method.
This disconnects the interrupt, and cancels any running timers
- Line 318-324: We implement the `RegisterEventHandler` method. This is quite straightforward
- Line 330-335: We implement the `UnregisterEventHandler` method. This is quite straightforward
- Line 342-347: We implement the global `SwitchButtonInterruptHandler()` method, which is the global interrupt handler for the SW GPIO pin.
It converts the parameter to a class pointer, and then calls the class method
- Line 353-377: We implement the `SwitchButtonInterruptHandler()` method, which is the class interrupt handler for the GPIO pins
  - Line 359: It reads the GPIO pin value (down is false, up is true)
  - Line 361-363: If the switch button is up, we set the release time
  - Line 365-367: If the switch button is down, we set the press time
  - Line 369-373: If there was already a debounce timer running, we cancel it
  - Line 375-376: We start a new debounce timer
- Line 387-392: We implement the global `SwitchButtonDebounceHandler()` method, which is the global debounce timer timeout handler.
It converts the parameter to a class pointer, and then calls the class method
- Line 399-451: We implement the `SwitchButtonDebounceHandler()` method, which is the class debounce timer timeout handler
  - Line 404-406: We determin the GPIO pin value, set the event to either `SwitchUp` or `SwitchDown`, and similarly set the internal event to `Up` or `Down`
  - Line 408-413: If the button is up, we check the time elapsed between the press and the release, if this is within the click time, we change the internal event to `Click`
  - Line 415-420: If the button is down, we check the time elapsed between the press and the previous press, if this is within the double click time, we change the internal event to `DblClick`
  - Line 421-425: We print debug output on switch button state and timing
  - Line 428: If the button is up, we save the last release time
  - Line 432: Else we save the last press time
  - Line 435-436: We print debug output on switch button state and timing
  - Line 437-440: If an event handler is registered, we call it
  - Line 442-445: If a tick timer was running, we cancel it
  - Line 447-448: if the button is down, we start a time to check for holding down the button
  - Line 450: We call the method `HandleSwitchButtonEvent()` to handle the internal state and generate a possible event
- Line 461-467: We implement the global `SwitchButtonTickHandler()` method, which is the global tick timer timeout handler.
It converts the parameter to a class pointer, and then calls the class method
- Line 474-481: We implement the `SwitchButtonTickHandler()` method, which is the class tick timer timeout handler
  - Line 478: We start another tick timer as we will repeat the event as long as the button is held down
  - Line 480: We call the method `HandleSwitchButtonEvent()` to handle the internal state and generate a possible event
- Line 489-507: We implement the `HandleSwitchButtonEvent()` method, which handles the state machine
  - Line 491: We verify we have a valid event
  - Line 493-494: We print debug info
  - Line 495: We determine the output event from the current internal event and the internal state
  - Line 496: We determine the new internal state from the current internal event and the internal state
  - Line 498-499: We print debug info
  - Line 501: We save the next state
  - Line 503-5506: If the output event is valid, and a event handler is registered, we call the event handler
  
### Update application code {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_UPDATE_APPLICATION_CODE}

We'll create a rotary switch in the application and display its event callback.
Due to the amount of debug output, we'll change the log level to `Info`. We'll also keep waiting a bit longer to allow for interaction with the rotary switch.
Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/System.h>
6: #include <baremetal/Timer.h>
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
46: 
```

### Update application CMake file {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_UPDATE_APPLICATION_CMAKE_FILE}

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

### Configuring, building and debugging {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_SWITCH_BUTTON___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice the click, double click, triple click and hold events

```text
Info   0.00:00:21.820 InterruptSystem::Shutdown (InterruptHandler:153)
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:25)
Info   0.00:00:00.090 Wait 20 seconds (main:31)
Info   0.00:00:00.840 Event SwitchDown (main:16)
Info   0.00:00:00.840 Event SwitchDoubleClick (main:16)
Info   0.00:00:01.220 Event SwitchUp (main:16)
Info   0.00:00:02.780 Event SwitchDown (main:16)
Info   0.00:00:03.010 Event SwitchUp (main:16)
Info   0.00:00:03.010 Event SwitchClick (main:16)
Info   0.00:00:04.850 Event SwitchDown (main:16)
Info   0.00:00:05.000 Event SwitchUp (main:16)
Info   0.00:00:05.000 Event SwitchClick (main:16)
Info   0.00:00:05.110 Event SwitchDown (main:16)
Info   0.00:00:05.110 Event SwitchDoubleClick (main:16)
Info   0.00:00:05.290 Event SwitchUp (main:16)
Info   0.00:00:05.290 Event SwitchDoubleClick (main:16)
Info   0.00:00:06.500 Event SwitchDown (main:16)
Info   0.00:00:06.650 Event SwitchUp (main:16)
Info   0.00:00:06.650 Event SwitchClick (main:16)
Info   0.00:00:06.790 Event SwitchDown (main:16)
Info   0.00:00:06.790 Event SwitchDoubleClick (main:16)
Info   0.00:00:06.930 Event SwitchUp (main:16)
Info   0.00:00:06.930 Event SwitchDoubleClick (main:16)
Info   0.00:00:07.930 Event SwitchDown (main:16)
Info   0.00:00:08.080 Event SwitchUp (main:16)
Info   0.00:00:08.080 Event SwitchClick (main:16)
Info   0.00:00:08.220 Event SwitchDown (main:16)
Info   0.00:00:08.220 Event SwitchDoubleClick (main:16)
Info   0.00:00:08.330 Event SwitchUp (main:16)
Info   0.00:00:08.330 Event SwitchDoubleClick (main:16)
Info   0.00:00:08.440 Event SwitchDown (main:16)
Info   0.00:00:08.440 Event SwitchDoubleClick (main:16)
Info   0.00:00:08.590 Event SwitchUp (main:16)
Info   0.00:00:08.590 Event SwitchTripleClick (main:16)
Info   0.00:00:09.880 Event SwitchDown (main:16)
Info   0.00:00:10.880 Event SwitchHold (main:16)
Info   0.00:00:11.880 Event SwitchHold (main:16)
Info   0.00:00:12.880 Event SwitchHold (main:16)
Info   0.00:00:13.340 Event SwitchUp (main:16)
Press r to reboot, h to halt
rInfo   0.00:00:21.620 Reboot (System:154)
Info   0.00:00:21.640 InterruptSystem::Shutdown (InterruptHandler:153)
```

## Adding intelligence to the rotary switch - Step 5 {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5}
          
Now let's also make the rotate part a bit smarter.

We'll want to distinguish between a clockwise and a counter-clockwise tick, and we will need to handle fast turning which may skip an event here and there.

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
| CCW Data Rise  | 1   | 0  | Invalud           | -                |
| CCW Data Rise  | 1   | 1  | Start (**)        | CounterClockwise |
| Invalid        | 0   | 0  | Invalud           | -                |
| Invalid        | 0   | 1  | Invalud           | -                |
| Invalid        | 1   | 0  | Invalud           | -                |
| Invalid        | 1   | 1  | Start             | -                |

Here, sa single asterisk '*' indicates a fallback situation that is not expected, but we try to handle it correctly.
A double asterisk '**' means it is the end of a cycle, and we generate an event an restart the cycle

### KY-040.h {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5_KY_040H}

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
67:     	/// @brief Switch is rotated clockwise
68:         RotateClockwise,
69:     	/// @brief Switch is rotated counter clockwise
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
90:     using EventHandler = void(Event event, void *param);
91: 
92: private:
93:     /// @brief True if the rotary switch was initialized
94:     bool                            m_isInitialized;
95:     /// @brief GPIO pin for CLK input
96:     baremetal::PhysicalGPIOPin      m_clkPin;
97:     /// @brief GPIO pin for DT input
98:     baremetal::PhysicalGPIOPin      m_dtPin;
99:     /// @brief GPIO pin for SW input (switch button)
100:     baremetal::PhysicalGPIOPin      m_swPin;
101:     /// @brief Internal state of the rotary encoder
102:     SwitchEncoderState              m_switchEncoderState;
103:     /// @brief Internal state of the switch button (to tracking single, double, triple clicking and hold
104:     SwitchButtonState               m_switchButtonState;
105:     /// @brief Handle to timer for debouncing the switch button
106:     baremetal::KernelTimerHandle    m_debounceTimerHandle;
107:     /// @brief Handle to timer for handling button press ticks (for hold)
108:     baremetal::KernelTimerHandle    m_tickTimerHandle;
109:     /// @brief Time at which the current button press occurred
110:     unsigned                        m_currentPressTicks;
111:     /// @brief Time at which the current button release occurred
112:     unsigned                        m_currentReleaseTicks;
113:     /// @brief Time at which the last button press occurred
114:     unsigned                        m_lastPressTicks;
115:     /// @brief Time at which the last button release occurred
116:     unsigned                        m_lastReleaseTicks;
117: 
118:     /// @brief Registered event handler
119:     EventHandler*                   m_eventHandler;
120:     /// @brief Parameter for registered event handler
121:     void*                           m_eventHandlerParam;
122: 
123: public:
124:     KY040(uint8 clkPin, uint8 dtPin, uint8 swPin);
125:     virtual ~KY040();
126: 
127:     void               Initialize();
128:     void               Uninitialize();
129: 
130:     void               RegisterEventHandler(EventHandler *handler, void *param);
131:     void               UnregisterEventHandler(EventHandler *handler);
132:     static const char *EventToString(Event event);
133: 
134: private:
135:     static void SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin, void *param);
136:     void        SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin);
137:     static void SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin, void *param);
138:     void        SwitchButtonInterruptHandler(baremetal::IGPIOPin* pin);
139:     static void SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void *param, void *context);
140:     void        SwitchButtonDebounceHandler(baremetal::KernelTimerHandle handle, void *param);
141:     static void SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void *param, void *context);
142:     void        SwitchButtonTickHandler(baremetal::KernelTimerHandle handle, void *param);
143:     void        HandleSwitchButtonEvent(SwitchButtonEvent switchEvent);
144: };
145: 
146: } // namespace device
```

- Line 52: We forward declare the enum `SwitchEncoderState` which will hold the internal rotary encoder state
- Line 68-70: We add the clockwise and counter clockwise rotation as events
- Line 102: We add the member variable `m_switchEncoderState` to hold the rotary encoder internal state
- Line 135-136: We add the GPIO pin interrupt handler for the CLK and DT GPIO pins.
One is global and receives a pointer to the KY040 instance, the other is called by the global one

### KY-040.cpp {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5_KY_040CPP}

We'll implement the changes.

Update the file `code/libraries/device/src/gpio/KY-040.cpp`

```cpp
File: code/libraries/device/src/gpio/KY-040.cpp
...
61: /// <summary>
62: /// Switch encoder internal state
63: /// </summary>
64: enum class SwitchEncoderState
65: {
66: 	/// @brief CLK high, DT high
67:     Start,
68:     /// @brief CLK high, DT down
69:     CWStart,
70:     /// @brief CLK down, DT low
71:     CWDataFall,
72:     /// @brief CLK low, DT up
73:     CWClockRise,
74:     /// @brief CLK down, DT high
75:     CCWStart,
76:     /// @brief CLK up DT down
77:     CCWClockFall,
78:     /// @brief CLK up, DT low
79:     CCWDataRise,
80:     /// @brief Invalid state
81:     Invalid,
82:     /// @brief Unknown
83:     Unknown
84: };
85: 
...
128: /// <summary>
129: /// Convert rotary switch encoder state to a string
130: /// </summary>
131: /// <param name="state">Switch encode state</param>
132: /// <returns>String representing state</returns>
133: static const char *EncoderStateToString(SwitchEncoderState state)
134: {
135:     switch (state)
136:     {
137:     case SwitchEncoderState::Start:
138:         return "Start";
139:     case SwitchEncoderState::CWStart:
140:         return "CWStart";
141:     case SwitchEncoderState::CWDataFall:
142:         return "CWDataFall";
143:     case SwitchEncoderState::CWClockRise:
144:         return "CWClockRise";
145:     case SwitchEncoderState::CCWStart:
146:         return "CCWStart";
147:     case SwitchEncoderState::CCWClockFall:
148:         return "CCWClockFall";
149:     case SwitchEncoderState::CCWDataRise:
150:         return "CCWDataRise";
151:     case SwitchEncoderState::Invalid:
152:         return "Invalid";
153:     case SwitchEncoderState::Unknown:
154:     default:
155:         break;
156:     }
157:     return "Unknown";
158: }
159: 
...
165: const char *KY040::EventToString(KY040::Event event)
166: {
167:     switch (event)
168:     {
169:     case KY040::Event::RotateClockwise:
170:         return "RotateClockwise";
171:     case KY040::Event::RotateCounterclockwise:
172:         return "RotateCounterclockwise";
173:     case KY040::Event::SwitchDown:
174:         return "SwitchDown";
175:     case KY040::Event::SwitchUp:
176:         return "SwitchUp";
177:     case KY040::Event::SwitchClick:
178:         return "SwitchClick";
179:     case KY040::Event::SwitchDoubleClick:
180:         return "SwitchDoubleClick";
181:     case KY040::Event::SwitchTripleClick:
182:         return "SwitchTripleClick";
183:     case KY040::Event::SwitchHold:
184:         return "SwitchHold";
185:     case KY040::Event::Unknown:
186:     default:
187:         break;
188:     }
189:     return "Unknown";
190: }
...
248: static const KY040::Event s_encoderOutput[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
249: //  {{CLK=0/DT=0,            CLK=0/DT=1},            {CLK=1/DT=0,            CLK=1/DT=1}}
250: 
251:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                   // Start
252: 
253:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                   // CWStart
254:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                   // CWDataFall
255:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateClockwise}},           // CWClockRise
256: 
257:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                   // CCWStart
258:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                   // CCWClockFall
259:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateCounterclockwise}},    // CCWDataRise
260: 
261:     {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}}                    // Invalid
262: };
263: static KY040::Event GetEncoderOutput(SwitchEncoderState state, bool clkValue, bool dtValue)
264: {
265:     return s_encoderOutput[static_cast<size_t>(state)][clkValue][dtValue];
266: }
267: 
268: static const SwitchEncoderState s_encoderNextState[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
269: //  {{CLK=0/DT=0,                       CLK=0/DT=1},                      {CLK=1/DT=0,                      CLK=1/DT=1}}
270: 
271:     {{SwitchEncoderState::Invalid,      SwitchEncoderState::CWStart},     {SwitchEncoderState::CCWStart,    SwitchEncoderState::Start}},     // Start (1, 1), this is the default state between two clicks
272: 
273:     {{SwitchEncoderState::CWDataFall,   SwitchEncoderState::CWStart},     {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}},     // CWStart (1, 0)
274:     {{SwitchEncoderState::CWDataFall,   SwitchEncoderState::CWStart},     {SwitchEncoderState::CWClockRise, SwitchEncoderState::Invalid}},   // CWDataFall (0, 0)
275:     {{SwitchEncoderState::CWDataFall,   SwitchEncoderState::Invalid},     {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}},     // CWClockRise (0, 1)
276: 
277:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart,    SwitchEncoderState::Start}},     // CCWStart (0, 1)
278:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart,    SwitchEncoderState::Invalid}},   // CCWClockFall (0, 0)
279:     {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::Invalid,     SwitchEncoderState::Start}},     // CCWDataRise (1, 0)
280: 
281:     {{SwitchEncoderState::Invalid,      SwitchEncoderState::Invalid},     {SwitchEncoderState::Invalid,     SwitchEncoderState::Start}}      // Invalid
282: };
283: static SwitchEncoderState GetEncoderNextState(SwitchEncoderState state, bool clkValue, bool dtValue)
284: {
285:     return s_encoderNextState[static_cast<size_t>(state)][clkValue][dtValue];
286: }
287: 
...
348: KY040::KY040(uint8 clkPin, uint8 dtPin, uint8 swPin)
349:     : m_isInitialized{}
350:     , m_clkPin(clkPin, GPIOMode::InputPullUp)
351:     , m_dtPin(dtPin, GPIOMode::InputPullUp)
352:     , m_swPin(swPin, GPIOMode::InputPullUp)
353:     , m_switchEncoderState{SwitchEncoderState::Start}
354:     , m_switchButtonState{SwitchButtonState::Start}
355:     , m_debounceTimerHandle{}
356:     , m_tickTimerHandle{}
357:     , m_currentPressTicks{}
358:     , m_currentReleaseTicks{}
359:     , m_lastPressTicks{}
360:     , m_lastReleaseTicks{}
361: 
362:     , m_eventHandler{}
363:     , m_eventHandlerParam{}
364: {
365:     LOG_DEBUG("KY040 constructor");
366: }
...
380: void KY040::Initialize()
381: {
382:     if (m_isInitialized)
383:         return;
384: 
385:     LOG_DEBUG("KY040 Initialize");
386:     m_clkPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
387:     m_dtPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
388:     m_swPin.ConnectInterrupt(SwitchButtonInterruptHandler, this);
389: 
390:     m_clkPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
391:     m_dtPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
392:     m_swPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
393: 
394:     m_isInitialized = true;
395: }
...
447: /// <summary>
448: /// Global GPIO pin interrupt handler for switch encoder
449: /// </summary>
450: /// <param name="pin">GPIO pin for the button encoder inputs</param>
451: /// <param name="param">Parameter for the interrupt handler, which is a pointer to the class instance</param>
452: void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin *pin, void *param)
453: {
454:     KY040 *pThis = reinterpret_cast<KY040 *>(param);
455:     assert(pThis != nullptr);
456:     pThis->SwitchEncoderInterruptHandler(pin);
457: }
458: 
459: /// <summary>
460: /// GPIO pin interrupt handler for switch encoder
461: /// </summary>
462: /// <param name="pin">GPIO pin for the button encoder inputs</param>
463: void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin)
464: {
465:     auto clkValue = m_clkPin.Get();
466:     auto dtValue  = m_dtPin.Get();
467:     LOG_DEBUG("KY040 CLK: %d", clkValue);
468:     LOG_DEBUG("KY040 DT:  %d", dtValue);
469:     assert(m_switchEncoderState < SwitchEncoderState::Unknown);
470: 
471:     LOG_DEBUG("KY040 Current state: %s", EncoderStateToString(m_switchEncoderState));
472:     Event event = GetEncoderOutput(m_switchEncoderState, clkValue, dtValue);
473:     m_switchEncoderState = GetEncoderNextState(m_switchEncoderState, clkValue, dtValue);
474:     LOG_DEBUG("KY040 Event: %s", EventToString(event));
475:     LOG_DEBUG("KY040 Next state: %s", EncoderStateToString(m_switchEncoderState));
476: 
477:     if ((event != Event::Unknown) && (m_eventHandler != nullptr))
478:     {
479:         (*m_eventHandler)(event, m_eventHandlerParam);
480:     }
481: }
482: 
```

- Line 64-84: We declare the `SwitchEncoderState` enum values.
You can recognize the values from the table we showed in [Adding intelligence to the rotary switch - Step 5](#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5)
- Line 133-158: We define a function `EncoderStateToString` to convert a rotary encoder state to a string for debugging
- Line 169-172: We add the `Clockwise` and `CounterClockwise` enum values to the conversion to string
- Line 248-262: We define a matrix `s_encoderOutput` to determine the event to be generated when an event happens in a certain state.
This is part of the state machine
- Line 263-266: We implement a local function `GetEncoderOutput()` which uses the `s_encoderOutput` variable to determine the event to generate
- Line 268-282: We define a matrix `s_encoderNextState` to determine the next state when an event happens in a certain state.
This is part of the state machine
- Line 283-286: We implement a local function `GetEncoderNextState()` which uses the `s_encoderNextState` variable to determine the next state
- Line 353: We update the constructor to initialize `m_switchEncoderState`
- Line 386-387: We update the `Initialize()` method to also connect and interrupt handler for the CLK and DT GPIO pins
- Line 390-391: We update the `Initialize()` method to also set up interrupt for both rising and falling edges for the CLK and DT GPIO pins
- Line 452-457: We implement the global `SwitchEncoderInterruptHandler()` method, which is the global interrupt handler for the CLK and DT GPIO pins.
It converts the parameter to a class pointer, and then calls the class method
- Line 463-481: We implement the `SwitchEncoderInterruptHandler()` method, which is the class interrupt handler for the CLK and DT GPIO pins
  - Line 465-466: It reads the GPIO pin values
  - Line 472: We determine the event to be generated from the current state and the GPIO pin values
  - Line 473: We determine the next state from the current state and the GPIO pin values
  - Line 477-480: If the event is not `Unknown` and there is an event handler function installed, we call it

### Update application code {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5_UPDATE_APPLICATION_CODE}

Let's try to do something useful with the rotary switch.
We use a value which starts at 0, if we rotate clockwise we increment the value, if we rotate counter clockwise we decrement, and if the switch is push down, we print the value.

In fact, let's make it even more fancy. If we press and hold the switch button for 2 seconds, let's automatically perform a reboot.
This means the main loop will simply be waiting for an interrupt to happen, and check whether we need to reboot.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/System.h>
6: #include <baremetal/Timer.h>
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

- Line 19-44: We handle the event callback from the rotarty switch
  - Line 24-26: If the switch button is down, we print the current value
  - Line 27-29: If the switch is rotated clockwise, we increment the value
  - Line 30-32: If the switch is rotated counter clockwise, we decrement the value
  - Line 33-40: If the switch button is held down, we count the number of times we get a `SwitchHold` event.
If this reached `HolsThreshold`, we flag a reboot
- Line 58: We print a message to hold down the button for `HoldThreshold` seconds to reboot
- Line 59-62: As long as the reboot flag is not set, we wait for an interrupt.
This effectively shuts down the core until an interrupt happens
- Line: 69: We return `ReturnCode::ExitReboot` to enforce a reboot

### Configuring, building and debugging {#TUTORIAL_20_GPIO_ADDING_INTELLIGENCE_TO_THE_ROTARY_SWITCH___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice the rotate events, as well as the value being printed when with push the switch down.

```text
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:52)
Info   0.00:00:00.090 Hold down switch button for 2 seconds to reboot (main:58)
Info   0.00:00:03.540 Event RotateClockwise (main:21)
Info   0.00:00:03.900 Event RotateClockwise (main:21)
Info   0.00:00:04.020 Event RotateClockwise (main:21)
Info   0.00:00:04.180 Event RotateClockwise (main:21)
Info   0.00:00:04.320 Event RotateClockwise (main:21)
Info   0.00:00:04.680 Event RotateCounterclockwise (main:21)
Info   0.00:00:05.000 Event RotateCounterclockwise (main:21)
Info   0.00:00:05.100 Event RotateCounterclockwise (main:21)
Info   0.00:00:05.390 Event RotateCounterclockwise (main:21)
Info   0.00:00:05.480 Event RotateCounterclockwise (main:21)
Info   0.00:00:06.900 Event SwitchDown (main:21)
Info   0.00:00:06.900 Value 0 (main:25)
Info   0.00:00:07.360 Event SwitchUp (main:21)
Info   0.00:00:09.460 Event SwitchDown (main:21)
Info   0.00:00:09.460 Value 0 (main:25)
Info   0.00:00:10.460 Event SwitchHold (main:21)
Info   0.00:00:10.990 Event SwitchUp (main:21)
Info   0.00:00:12.450 Event SwitchDown (main:21)
Info   0.00:00:12.450 Value 0 (main:25)
Info   0.00:00:13.450 Event SwitchHold (main:21)
Info   0.00:00:13.450 Reboot triggered (main:38)
Info   0.00:00:13.480 Rebooting (main:67)
Info   0.00:00:13.500 Reboot (System:154)
Info   0.00:00:13.520 InterruptSystem::Shutdown (InterruptHandler:153)
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
Info   0.00:00:00.050 Starting up (System:209)
Info   0.00:00:00.070 Current EL: 1 (main:52)
Info   0.00:00:00.090 Hold down switch button for 2 seconds to reboot (main:58)
Info   0.00:00:01.320 Event RotateClockwise (main:21)
Info   0.00:00:02.230 Event RotateClockwise (main:21)
Info   0.00:00:02.550 Event RotateClockwise (main:21)
Info   0.00:00:02.780 Event RotateClockwise (main:21)
Info   0.00:00:03.050 Event RotateClockwise (main:21)
Info   0.00:00:04.170 Event RotateCounterclockwise (main:21)
Info   0.00:00:04.770 Event RotateCounterclockwise (main:21)
Info   0.00:00:05.190 Event RotateCounterclockwise (main:21)
Info   0.00:00:06.210 Event RotateCounterclockwise (main:21)
Info   0.00:00:08.470 Event SwitchDown (main:21)
Info   0.00:00:08.470 Value 1 (main:25)
Info   0.00:00:08.620 Event SwitchUp (main:21)
Info   0.00:00:08.620 Event SwitchClick (main:21)
Info   0.00:00:10.320 Event SwitchDown (main:21)
Info   0.00:00:10.320 Value 1 (main:25)
Info   0.00:00:11.320 Event SwitchHold (main:21)
Info   0.00:00:12.320 Event SwitchHold (main:21)
Info   0.00:00:12.320 Reboot triggered (main:38)
Info   0.00:00:12.350 Rebooting (main:67)
Info   0.00:00:12.370 Reboot (System:154)
Info   0.00:00:12.390 InterruptSystem::Shutdown (InterruptHandler:153)
```

Next: [21-i2c](21-i2c.md)
