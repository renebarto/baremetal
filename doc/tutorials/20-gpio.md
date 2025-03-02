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
Not that we need to have the hardware installed, so we need to run the code on an actual board.

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

### Configuring, building and debugging {#TUTORIAL_20_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## General approach for GPIO interrupts - Step 3 {#TUTORIAL_20_GPIO_SETTING_UP_GPIO_AND_READING_DATA___STEP_1}

As we can see in the previous step, we need to set up administration to pass to the interrupt handler.
That is impractical, it would be easier if we could register a handler for a specific GPIO pin interrupt.

We'll introduce a GPIO manager that will register to the GPIO interrupts, and have each pin register itself with the manager.
The GPIO manager will then check for each registered pin whether an event occurred, and call the interrupt handler on that pin.

### IGPIOManager.h {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINH}

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
56:     virtual ~IGPIOManager() = default;
57: 
58:     virtual void Initialize() = 0;
59: 
60:     virtual void ConnectInterrupt(IGPIOPin *pin)          = 0;
61:     virtual void DisconnectInterrupt(const IGPIOPin *pin) = 0;
62: 
63:     virtual void InterruptHandler() = 0;
64: 
65:     /// @brief Switch all GPIO pins to input mode, without pull-up or pull-down
66:     /// @return True if successful, false otherwise
67:     virtual bool AllOff() = 0;
68: };
69: 
70: } // namespace baremetal
```

- Line 53-68: We declare the abstract class `IGPIOManager`
  - Line 58: We declare an abstract method `Initialize()` to initialize the manager
  - Line 60: We declare an abstract method `ConnectInterrupt()` to register a GPIO pin for interrupts
  - Line 61: We declare an abstract method `DisconnectInterrupt()` to unregister a GPIO pin for interrupts
  - Line 63: We declare an abstract method `InterruptHandler()` as the interrupt handler function
  - Line 67: We declare an abstract method `AllOff()` to unregister all GPIO pins and switch them to default mode

### GPIOManager.h {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINH}

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
67:     /// @brief True if the GPIO IRQ is connected to
68:     bool m_irqConnected;
69:     /// @brief Array of all registered GPIO pins (nullptr if the GPIO is not registered)
70:     IGPIOPin* m_pins[NUM_GPIO];
71:     /// @brief Memory access interface
72:     IMemoryAccess& m_memoryAccess;
73: 
74:     GPIOManager();
75: 
76: public:
77:     explicit GPIOManager(IMemoryAccess &memoryAccess);
78:     ~GPIOManager();
79: 
80:     void Initialize() override;
81: 
82:     void ConnectInterrupt(IGPIOPin *pin) override;
83:     void DisconnectInterrupt(const IGPIOPin *pin) override;
84: 
85:     void InterruptHandler() override;
86: 
87:     bool AllOff() override;
88: 
89:     void DisableAllInterrupts(uint8 pinNumber);
90: 
91: private:
92: };
93: 
94: GPIOManager &GetGPIOManager();
95: 
96: } // namespace baremetal
```

- Line 55-92: We declare the class `GPIOManager`
  - Line 62: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager
  - Line 66: We declare a member variable `m_isInitialized` to guard against multiple initialization
  - Line 68: We declare a member variable `m_irqConnected` to flag whether we have registered to the GPIO interrupt
  - Line 70: We declare a member variable `m_pins` to keep track of the registered GPIO pins
  - Line 72: We declare a member variable `m_memoryAccess` to hold the `MemoryAccess` instance
  - Line 74: We declare the default constructor as private, as we did before
  - Line 77: We declare a non-default constructor taking a `MemoryAccess` instance
  - Line 78: We declare a virtual destructor as we inherit from an abstract interface
  - Line 80: We declare a method `Initialize()` to override the abstract interface
  - Line 82: We declare a method `ConnectInterrupt()` to override the abstract interface
  - Line 83: We declare a method `DisconnectInterrupt()` to override the abstract interface
  - Line 85: We declare a method `InterruptHandler()` to override the abstract interface
  - Line 87: We declare a method `AllOff()` to override the abstract interface
  - Line 89: We declare a method `DisableAllInterrupts()` to disable all GPIO interrupts by clearing the respective registers
- Line 94: As before, we declare a friend function to retrieve the singleton instance of the GPIO manager

### PhysicalGPIOPin.cpp {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINCPP}

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
```
### PhysicalGPIOPin.h {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINH}

Update the file `code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h
```

### PhysicalGPIOPin.cpp {#TUTORIAL_20_GPIO_ADDING_GPIO_INTERRUPTS___STEP_2_PHYSICALGPIOPINCPP}

Update the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
```
Next: [21-i2c](21-i2c.md)
