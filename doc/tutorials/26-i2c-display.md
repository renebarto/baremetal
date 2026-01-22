# Tutorial 26: I2C display {#TUTORIAL_26_I2C_DISPLAY}

@tableofcontents

## Tutorial setup {#TUTORIAL_26_I2C_DISPLAY_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/26-i2c-display`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_26_I2C_DISPLAY_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-26.a`
- a library `output/Debug/lib/device-26.a`
- a library `output/Debug/lib/stdlib-26.a`
- a library `output/Debug/lib/unittest-26.a`
- an application `output/Debug/bin/26-i2c-display.elf`
- an image in `deploy/Debug/26-i2c-display-image`

## Controlling a I2C LCD display {#TUTORIAL_26_I2C_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY}

I'd like to dive into using a I2C based LCD display module, for a couple of reasons.

- These modules are very common, your can buy them online almost everywhere
- The pricing is very reasonable
- It is a very simple way to give feedback from an application

Two very common display modules are a 2 x 16 character display and a 4 x 20 character display (also known as LCD1602 and LCD2004).
They come in different colors (orange, blue, green) and can be either controlled directly through GPIO, or through I2C using a piggyback board.
It also possible to add a piggyback board that has buttons, that can be used as GPIO pins next to the I2C controlled display. This however uses a full GPIO header for Raspberry Pi, so nothing else can be added.

The first display is 16 x 2 characters, and has a I2C piggyback module.

<img src="images/LCD16x2_I2C_Front.png" alt="16 x 2 character display front" width="400"/>

<img src="images/LCD16x2_I2C_Back.png" alt="16 x 2 character display back" width="400"/>

The second display is 20 x 4 characters, and has a I2C and GPIO piggyback module with 4 buttons. This uses a full GPIO header.

<img src="images/LCD20x4_Front.png" alt="20 x 4 character display front" width="400"/>

<img src="images/LCD20x4_Back.png" alt="20 x 4 character display back" width="400"/>

We'll be using the first display in this tutorial.

At you can see in the front image, the display board itself has 16 connections, the piggback board only has 4 (power + I2C).

The piggyback board schematics are shown below.

<img src="images/SBC-LCD16x2-I2C.png" alt="I2C piggyback board for 16x2 display" width="1000"/>

The board uses a [PF8574 I2C 8 bit I/O expander](pdf/PCF8574_PCF8574A.pdf). This is used to control the signals to the board.
As it is a 8 bit expander, it has 8 output signals:

| Output pin | Display connection | Function |
|------------|--------------------|----------|
| 0          | RS                 | Register Select (0 = instruction, 1 = data |
| 1          | RW                 | Read/Write (0 = write, 1 = read)           |
| 2          | E                  | Enable: Starts data read / write           |
| 3          | BL                 | Backlight (0 = off, 1 = on)                |
| 4          | D4                 | Data bit 4 (4 bit access only)             |
| 5          | D5                 | Data bit 5 (4 bit access only)             |
| 6          | D6                 | Data bit 6 (4 bit access only)             |
| 7          | D7                 | Data bit 7 (4 bit access only)             |

As you can see, only data bits 4/7 are connected. As we only have 8 lines to connect, we place the display controller in 4 bit mode, meaning we have to write twice to transfer a byte.
Even though we have a RW signal, the hardware is not set up to support reading from the display controller.

The LCD controller is a [Hitachi HD44780 device](pdf/HD44780.pdf).
This supports a 4 bit interaction, if initialized correctly.
We'll be using this, as we're going to use the I2C interface.
Using the 8 bit interface requires 12 signals, which is harder to support unless we use a device such as MCP23017.

### Initialization sequence {#TUTORIAL_26_I2C_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_INITIALIZATION_SEQUENCE}

The initialization sequence is as follows for a 4 bit interface (see [Hitachi HD44780 device](pdf/HD44780.pdf) Figure 24, Page 46).
These are all 4-bit transfers, the E signal is used as a clock:
- Wait >=15 milliseconds after power on
- Write RS=0, RW=0, E=0, D7-D4=0011
- Write RS=0, RW=0, E=1, D7-D4=0011
- Wait >=4.1 milliseconds
- Write RS=0, RW=0, E=0, D7-D4=0011
- Write RS=0, RW=0, E=1, D7-D4=0011
- Wait >=100 microseconds
- Write RS=0, RW=0, E=0, D7-D4=0011
- Write RS=0, RW=0, E=1, D7-D4=0011
- Write RS=0, RW=0, E=0, D7-D4=0010
- Write RS=0, RW=0, E=1, D7-D4=0010

### Mode setting {#TUTORIAL_26_I2C_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_MODE_SETTING}

Setting the correct mode, these are all 8-bit transfers, with MSB 4 bits first, then LSB 4 bits:
- Send command `Function set`: 0010NF** (N = 0 for single line display, 1 for multi line, F = 0 fpr 5x8 pixel characters, 1 for 5x10 pixel characters)
  - Write RS=0, RW=0, E=0, D7-D4=0010
  - Write RS=0, RW=0, E=1, D7-D4=0010
  - Write RS=0, RW=0, E=0, D7-D4=NF** = 10**
  - Write RS=0, RW=0, E=1, D7-D4=NF** = 10**
- Send command `Display control`: 00001DCB (D = 0 for display off, 1 for on, C = 0 for cursor off, 1 for on, B = 0 for steady cursor, 1 for blinking)
  - Write RS=0, RW=0, E=0, D7-D4=0000
  - Write RS=0, RW=0, E=1, D7-D4=0000
  - Write RS=0, RW=0, E=0, D7-D4=1DCB = 1000
  - Write RS=0, RW=0, E=1, D7-D4=1DCB = 1000
- Send command 'Clear display': 00000001
  - Write RS=0, RW=0, E=0, D7-D4=0000
  - Write RS=0, RW=0, E=1, D7-D4=0000
  - Write RS=0, RW=0, E=0, D7-D4=0001
  - Write RS=0, RW=0, E=1, D7-D4=0001
- Send command 'Entry mode set': 000001IS (I = 1 for incrementing cursor position (left to right), 0 for decrementing (right to left), S = 1 for shifting the display while writing, 0 for only moving cursor)
  - Write RS=0, RW=0, E=0, D7-D4=0000
  - Write RS=0, RW=0, E=1, D7-D4=0000
  - Write RS=0, RW=0, E=0, D7-D4=01IS = 0110
  - Write RS=0, RW=0, E=1, D7-D4=01IS = 0110

As you can see, there is quite a bit of work to be done. The main issue is that we use the Enable signal as a clock.

The one thing that is not related to the HD44780 chip is the backlight control.
Bit 3 of the I/O expander controls the BL signal, which switches the backlight on and off (1 = on, 0 = off).

We'll start with controlling the backlight, after which we will implement sending data to the display.
Here we'll start with the initialization sequences, as this is required to do anything useful with the display.

## LCD display interface and basic functionality - Step 1 - backlight {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT}

The first step we will take is controlling the display backlight, which can be switched on and off.

### ITextDisplay.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_ITEXTDISPLAYH}

Create the file `code/libraries/device/include/device/display/ITextDisplay.h`

```cpp
File: code/libraries/device/include/device/display/ITextDisplay.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : ITextDisplay.h
5: //
6: // Namespace   : device
7: //
8: // Class       : ITextDisplay
9: //
10: // Description : Generic LCD text display interface
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
44: namespace device
45: {
46: 
47: /// <summary>
48: /// Generic character matrix LCD display interface
49: /// </summary>
50: class ITextDisplay
51: {
52:   public:
53:     /// <summary>
54:     /// Destructor
55:     /// </summary>
56:     virtual ~ITextDisplay() = default;
57: 
58:     /// <summary>
59:     /// Set backlight on or off
60:     /// </summary>
61:     /// <param name="on">If true backlight is switched on, if false it is switched off</param>
62:     virtual void SetBacklight(bool on) = 0;
63:     /// <summary>
64:     /// Returns the current backlight status
65:     /// </summary>
66:     /// <returns>True if backlight is on, false otherwise</returns>
67:     virtual bool IsBacklightOn() const = 0;
68: };
69: 
70: } // namespace device
```

- Line 47-68: We create an abstract class `ITextDisplay`
  - Line 58-62: We add a method `SetBacklight()` to switch the backlight on and off
  - Line 63-67: We add a method `IsBacklightOn()` to retrieve the backlight status

### HD44780Display.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_HD44780DISPLAYH}

Now we'll create a class `HD44780Display` which implements the `ITextDisplay` interface.
This forms a generic interface for `HD44780` based devices.

Create the file `code/libraries/device/include/device/display/HD44780Display.h`

```cpp
File: code/libraries/device/include/device/display/HD44780Display.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : HD44780Display.h
5: //
6: // Namespace   : device
7: //
8: // Class       : HD44780Display
9: //
10: // Description : HD44780 based LCD generic display (max 40x4)
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
42: #include "device/display/ITextDisplay.h"
43: 
44: /// @file
45: /// HD44780 based I2C LCD character display (max 40x4 characters)
46: 
47: namespace device {
48: 
49: /// <summary>
50: /// Hitachi HD44780 based LCD text display controller
51: /// </summary>
52: class HD44780Display : public ITextDisplay
53: {
54: public:
55:     HD44780Display();
56: 
57:     virtual ~HD44780Display();
58: 
59:     void SetBacklight(bool on) override;
60:     bool IsBacklightOn() const override;
61: };
62: 
63: } // namespace device
```

This should speak for itself.
The class is relatively still abstract, as it does not use the I2C interface. We'll create a derived class for this later on.
This way we can concentrate the logic in the `HD44780Display` class, without the need to handle interface specifics.
The display could be connected by I2C, SPI, GPIO, etc.
As said, we're going to use I2C.

### HD44780Display.cpp {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_HD44780DISPLAYCPP}

Next we implement the `HD44780Display` class.

Create the file `code/libraries/device/src/display/HD44780Display.cpp`

```cpp
File: code/libraries/device/src/display/HD44780Display.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : HD44780Display.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : HD44780Display
9: //
10: // Description : HD44780 based LCD generic display (max 40x4)
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
40: #include "device/display/HD44780Display.h"
41: 
42: #include "baremetal/Logger.h"
43: #include "baremetal/Timer.h"
44: #include "stdlib/Util.h"
45: 
46: using namespace baremetal;
47: 
48: /// @file
49: /// HD44780 based I2C LCD character display (max 40x4 characters)
50: 
51: /// @brief Define log name
52: LOG_MODULE("HD44780Display");
53: 
54: namespace device {
55: 
56: /// <summary>
57: /// Constructor
58: /// </summary>
59: HD44780Display::HD44780Display()
60: {
61: }
62: 
63: /// <summary>
64: /// Destructor
65: /// </summary>
66: HD44780Display::~HD44780Display()
67: {
68:     // Don't write anymore, as derived class is already destroyed
69: }
70: 
71: /// <summary>
72: /// Switch backlight on or off
73: /// </summary>
74: /// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
75: void HD44780Display::SetBacklight(bool on)
76: {
77:     // Default implementation does not support backlight
78:     (void)on;
79: }
80: 
81: /// <summary>
82: /// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
83: /// </summary>
84: /// <returns>True if backlight is on, false otherwise</returns>
85: bool HD44780Display::IsBacklightOn() const
86: {
87:     // Default implementation does not support backlight
88:     return false;
89: }
90: 
91: } // namespace device
```

- Line 56-61: We implement the constructor, which is trivial
- Line 63-69: We implement the destructor, which is trivial
- Line 71-79: We implement `SetBacklight()` which does not do anything by default
- Line 81-89: We implement `IsBacklightOn()` which simply return false by default

### HD44780DisplayI2C.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_HD44780DISPLAYI2CH}

We'll create a derived version of `HD44780Display` named `HD44780DisplayI2C` which implements the I2C interface of the display.

Create the file `code/libraries/device/include/device/i2c/HD44780DisplayI2C.h`

```cpp
File: code/libraries/device/include/device/i2c/HD44780DisplayI2C.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : HD44780DisplayI2C.h
5: //
6: // Namespace   : device
7: //
8: // Class       : HD44780DisplayI2C
9: //
10: // Description : HD44780 based LCD generic display (max 40x4) with I2C piggyback
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
42: #include "device/display/HD44780Display.h"
43: 
44: /// @file
45: /// HD44780 based I2C LCD character display (max 40x4 characters) with I2C piggyback
46: 
47: namespace baremetal {
48: 
49: class II2CMaster;
50: 
51: } // namespace baremetal
52: 
53: namespace device {
54: 
55: /// <summary>
56: /// I2C controlled HD44780 based LCD character display
57: /// </summary>
58: class HD44780DisplayI2C : public HD44780Display
59: {
60: private:
61:     /// @brief I2C master interface
62:     baremetal::II2CMaster& m_i2cMaster;
63:     /// @brief I2C address of the LCD controller
64:     uint8                  m_address;
65:     /// @brief Backlight status
66:     bool                   m_backlightOn;
67: 
68: public:
69:     HD44780DisplayI2C(baremetal::II2CMaster& i2cMaster, uint8 address);
70: 
71:     ~HD44780DisplayI2C();
72: 
73:     void SetBacklight(bool on) override;
74:     bool IsBacklightOn() const override;
75: };
76: 
77: } // namespace device
```

- Line 55-75: We declare the class `HD44780DisplayI2C`
    - Line 61-62: We declare a member variable `m_i2cMaster` to hold a reference to the `II2CMaster` interface (an abstract interface to a I2C master)
    - Line 63-64: We declare a member variable `m_address` to hold the I2C address of the display
    - Line 65-66: We declare a member variable `m_backlightOn` to hold the backlight status of the display
    - Line 69: We declare the constructor, which takes the I2C interface reference as well as the I2C address
    - Line 71: We declare the destructor
    - Line 73-74: We override both `SetBacklight()` and `IsBacklightOn()`

### HD44780DisplayI2C.cpp {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_HD44780DISPLAYI2CCPP}

Next we implement the `HD44780DisplayI2C` class.

Create the file `code/libraries/device/src/i2c/HD44780DisplayI2C.cpp`

```cpp
File: code/libraries/device/src/i2c/HD44780DisplayI2C.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2026 Rene Barto
3: //
4: // File        : HD44780DisplayI2C.cpp
5: //
6: // Namespace   : device
7: //
8: // Class       : HD44780DisplayI2C
9: //
10: // Description : HD44780 based LCD generic display (max 40x4) with I2C piggyback
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
40: #include "device/i2c/HD44780DisplayI2C.h"
41: 
42: #include "baremetal/I2CMaster.h"
43: 
44: using namespace baremetal;
45: 
46: /// @file
47: /// HD44780 based I2C 16x2 LCD display
48: 
49: namespace device {
50: 
51: // HD44780Display via PCF8574 I2C port expander (for 16X2 LCD display
52: //  Pin mapping::
53: //  7  | 6  | 5  | 4  | 3  | 2  | 1  | 0
54: //  D3 | D2 | D1 | D0 | BK | EN | RW | RS
55: //
56: //  D3 : D7 (first write) and D3 (second write)
57: //  D2 : D6 (first write) and D2 (second write)
58: //  D1 : D5 (first write) and D1 (second write)
59: //  D0 : D4 (first write) and D0 (second write)
60: //  BK : Backlight off (0) or on (1)
61: //  EN : Enable. Needs to be strobed high to write
62: //  RW : Read (0) or Write (1)
63: //  RS : Instruction (0) or Data (1)
64: 
65: // PCF8574 I2C multiplexer signal mapping to HD44780 display
66: /// @brief RS pin is bit 0
67: static const uint8 PCF_RS = BIT1(0); // RS pin
68: /// @brief RW pin is bit 1
69: static const uint8 PCF_RW = BIT1(1); // RW pin
70: /// @brief EN pin is bit 2
71: static const uint8 PCF_EN = BIT1(2); // EN pin
72: /// @brief Backlight pin is bit 3
73: static const uint8 PCF_BK = BIT1(3); // Backlight pin
74: 
75: // Flags for RS pin modes
76: /// @brief Instruction register select (RS pin low)
77: static const uint8 RS_INSTRUCTION = (0x00);
78: /// @brief Data register select (RS pin high)
79: static const uint8 RS_DATA        = PCF_RS;
80: 
81: // Flags for backlight control
82: /// @brief Backlight on
83: const uint8 LCD_BACKLIGHT   = PCF_BK;
84: /// @brief Backlight off
85: const uint8 LCD_NOBACKLIGHT = 0x00;
86: 
87: /// <summary>
88: /// Constructor
89: ///
90: /// \note Driver uses 4-bit mode, pins D0-D3 are not used.
91: /// </summary>
92: /// <param name="i2cMaster">     I2C master interface</param>
93: /// <param name="address">       I2C device address</param>
94: HD44780DisplayI2C::HD44780DisplayI2C(II2CMaster& i2cMaster, uint8 address)
95:     : HD44780Display()
96:     , m_i2cMaster(i2cMaster)
97:     , m_address(address)
98:     , m_backlightOn{}
99: {
100: }
101: 
102: /// <summary>
103: /// Destructor
104: ///
105: /// Resets device back to 8 bit interface
106: /// </summary>
107: HD44780DisplayI2C::~HD44780DisplayI2C()
108: {
109:     SetBacklight(false);
110: }
111: 
112: /// <summary>
113: /// Switch backlight on or off
114: /// </summary>
115: /// <param name="on">If true, backlight is switched on, otherwise it is off</param>
116: void HD44780DisplayI2C::SetBacklight(bool on)
117: {
118:     if (on != m_backlightOn)
119:     {
120:         uint8 byte = (on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
121:         // We write a single byte with all other bits off. This will have no effect to the controller state, except for the backlight
122:         m_i2cMaster.Write(m_address, &byte, 1);
123:         m_backlightOn = on;
124:     }
125: }
126: 
127: /// <summary>
128: /// Returns backlight on / off status
129: /// </summary>
130: /// <returns>True if backlight is on, false otherwise</returns>
131: bool HD44780DisplayI2C::IsBacklightOn() const
132: {
133:     return m_backlightOn;
134: }
135: 
136: } // namespace device
```

- Line 66-67: We define a constant `PCF_RS` which is the RS pin of the display as seen from the PCF8574 port expander
- Line 68-69: We define a constant `PCF_RW` which is the RW pin of the display as seen from the PCF8574 port expander
- Line 70-71: We define a constant `PCF_EN` which is the E or EN pin of the display as seen from the PCF8574 port expander
- Line 72-73: We define a constant `PCF_BK` which is the backlight pin of the display as seen from the PCF8574 port expander.
As can be seen from the schematics in [Controlling a I2C LCD display](#TUTORIAL_26_I2C_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY), this actual controls the current through the K pin
- Line 76-77: We define a constant `RS_INSTRUCTION` to denote that we send an instruction to the display (setting the RS pin to 0)
- Line 78-79: We define a constant `RS_DATA` to denote that we send data to the display (setting the RS pin to 1)
- Line 82-83: We define a constant `LCD_BACKLIGHT` to set the backlight on (setting the backlight pin to 1)
- Line 84-85: We define a constant `LCD_NOBACKLIGHT` to set the backlight off (setting the backlight pin to 0)
- Line 87-100: We implement the constructor
- Line 102-110: We implement the destructor, which switches the backlight off
- Line 112-125: We implement `SetBacklight()` which simple writes the value for the backlight bit depending on the desired state of the backlight.
We can do this without disrupting the display interface state, as this clocks the EN pin to write to the display
- Line 127-134: We implement `IsBacklightOn()`, which simply returns the saved backlight state

### Application code {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_APPLICATION_CODE}

Let's try switching the backlight on and the off again.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/I2CMaster.h"
3: #include "baremetal/Logger.h"
4: #include "baremetal/System.h"
5: #include "baremetal/Timer.h"
6: #include "device/i2c/HD44780DisplayI2C.h"
7: #include "device/mocks/MemoryAccessHD44780DisplayI2CMock.h"
8: 
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: using namespace device;
13: 
14: int main()
15: {
16:     auto& console = GetConsole();
17: 
18:     uint8 busIndex = 1;
19:     uint8 address{0x27};
20:     I2CMaster i2cMaster;
21:     i2cMaster.Initialize(busIndex);
22:     HD44780DisplayI2C display(i2cMaster, address);
23:     display.SetBacklight(true);
24: 
25:     LOG_INFO("Wait 5 seconds");
26:     Timer::WaitMilliSeconds(5000);
27: 
28:     display.SetBacklight(false);
29: 
30:     console.Write("Press r to reboot, h to halt\n");
31:     char ch{};
32:     while ((ch != 'r') && (ch != 'h'))
33:     {
34:         ch = console.ReadChar();
35:         console.WriteChar(ch);
36:     }
37: 
38:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
39: }
```

- Line 18: We have the display connected to the I2C bus 1
- Line 19: As can be seen from the circuit diagram, lower 3 bits are all 1 by default.
The default address for PCF8574 is 0x20-0x27, which in our case is 0x27
- Line 20-21: We instantiate a `I2CMaster` and initialize it to bus 1
- Line 22: We instantiate a `HD44780DisplayI2C` and inject the `I2CMaster` instance, and set the I2C address
- Line 23: We switch the backlight on
- Line 28: We switch the backlight off again

### Configuring, building and debugging {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_1___BACKLIGHT_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
You should see the display backlight turn on and then off again after 5 seconds.
As the backlight is on after power on, you will need to run the application twice to see it turn on and off.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.090 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.110 Set up bus 1, config 0, base address 3F804000 (I2CMaster:190)
Info   0.00:00:00.130 Wait 5 seconds (main:25)
Press r to reboot, h to halt
rInfo   0.00:00:13.310 Reboot (System:144)
```

## LCD display interface and basic functionality - Step 2 - initialization {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION}

Now let's extend the functionality to also initialize the display and write some text to it.
This included the following steps:
- Sending the initialization sequence
- Configuring the display
- Writing text

### ITextDisplay.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_ITEXTDISPLAYH}

We'll extend the interface to start with.

Update the file `code/libraries/device/include/device/display/ITextDisplay.h`

```cpp
File: code/libraries/device/include/device/display/ITextDisplay.h
...
40: #pragma once
41: 
42: #include "stdlib/Types.h"
43: 
44: namespace device
45: {
46: 
47: /// <summary>
48: /// Generic character matrix LCD display interface
49: /// </summary>
50: class ITextDisplay
51: {
52:   public:
53:     /// <summary>
54:     /// Destructor
55:     /// </summary>
56:     virtual ~ITextDisplay() = default;
57: 
58:     /// <summary>
59:     /// Retrieve the number of display columns
60:     /// </summary>
61:     /// <returns>Number of display columns</returns>
62:     virtual uint8 GetNumColumns() const = 0;
63:     /// <summary>
64:     /// Retrieve the number of display rows
65:     /// </summary>
66:     /// <returns>Number of display rows</returns>
67:     virtual uint8 GetNumRows() const = 0;
68: 
69:     /// <summary>
70:     /// Initialize the device
71:     /// </summary>
72:     virtual void Initialize() = 0;
73: 
74:     /// <summary>
75:     /// Set backlight on or off
76:     /// </summary>
77:     /// <param name="on">If true backlight is switched on, if false it is switched off</param>
78:     virtual void SetBacklight(bool on) = 0;
79:     /// <summary>
80:     /// Returns the current backlight status
81:     /// </summary>
82:     /// <returns>True if backlight is on, false otherwise</returns>
83:     virtual bool IsBacklightOn() const = 0;
84: 
85:     /// <summary>
86:     /// Write a character to the display, and update the cursor location
87:     ///
88:     /// The character is written in the buffer, depending on the shift it may or may not be visible
89:     /// </summary>
90:     /// <param name="value">Character to write</param>
91:     virtual void Write(char value) = 0;
92:     /// <summary>
93:     /// Write text to display, and update the cursor location
94:     ///
95:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
96:     /// </summary>
97:     /// <param name="text">Text to write</param>
98:     /// <param name="count">Number of characters to write</param>
99:     virtual void Write(const char *text, size_t count) = 0;
100:     /// <summary>
101:     /// Write text to display, and update the cursor location
102:     ///
103:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
104:     /// </summary>
105:     /// <param name="text">Text to write</param>
106:     virtual void Write(const char *text) = 0;
107: };
108: 
109: } // namespace device
```

- Line 58-62: We declare a method `GetNumColumns()` to retrieve the number of columns the display supports
- Line 63-67: We declare a method `GetNumRows()` to retrieve the number of rows the display supports
- Line 69-72: We declare a method `Initialize()` to initialize the display, and configures it
- Line 85-91: We declare a method `Write()` to write a single character to the display
- Line 92-98: We declare a method `Write()` to write a string of characters with a set count to the display
- Line 99-106: We declare a method `Write()` to write a string of characters to the display

### HD44780Display.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_HD44780DISPLAYH}

We'll extend the `HD44780Display` class to implement the new `ITextDisplay` methods.

Update the file `code/libraries/device/include/device/display/HD44780Display.h`

```cpp
File: code/libraries/device/include/device/display/HD44780Display.h
...
40: #pragma once
41: 
42: #include "device/display/ITextDisplay.h"
43: 
44: /// @file
45: /// HD44780 based I2C LCD character display (max 40x4 characters)
46: 
47: namespace device {
48: 
49: /// <summary>
50: /// Hitachi HD44780 based LCD text display controller
51: /// </summary>
52: class HD44780Display : public ITextDisplay
53: {
54: public:
55:     /// <summary>
56:     /// Selection of data transfer mode. The HD44780 controller can be either used in 4 bit mode (using D7-D4) and 8 bit mode (using D7-D0)
57:     /// </summary>
58:     enum class DataMode : uint8
59:     {
60:         /// @brief 4 bit transfer mode
61:         Mode4Bit,
62:         /// @brief 8 bit transfer mode
63:         Mode8Bit,
64:     };
65: 
66:     /// <summary>
67:     /// Selection of the register to address in the LCD controller
68:     /// </summary>
69:     enum class RegisterSelect : uint8
70:     {
71:         /// @brief Instruction register
72:         Instruction,
73:         /// @brief Data register
74:         Data,
75:     };
76: 
77:     /// <summary>
78:     /// Display configuration
79:     /// </summary>
80:     enum class DisplayLines : uint8
81:     {
82:         /// @brief Single row display
83:         LinesSingle,
84:         /// @brief Multi row display
85:         LinesMulti,
86:     };
87: 
88:     /// <summary>
89:     /// Selects character size
90:     /// </summary>
91:     enum class CharacterSize : uint8
92:     {
93:         /// @brief Character size 5 x 10 pixels
94:         Size5x10,
95:         /// @brief Character size 5 x 8 pixels
96:         Size5x8,
97:     };
98: 
99: private:
100:     /// @brief Number of rows on the display
101:     uint8         m_numRows;
102:     /// @brief Number of columns on the display
103:     uint8         m_numColumns;
104:     /// @brief Data transfer mode (4 or 8 bit)
105:     DataMode      m_dataMode;
106:     /// @brief Whether display has single or multiple lines
107:     DisplayLines  m_lineMode;
108:     /// @brief Character size (5x8 or 5x10)
109:     CharacterSize m_characterSize;
110: 
111: public:
112:     HD44780Display(uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8, DataMode dataMode = DataMode::Mode4Bit);
113: 
114:     virtual ~HD44780Display();
115: 
116:     uint8 GetNumColumns() const override;
117:     uint8 GetNumRows() const override;
118:     void Initialize() override;
119:     bool IsDataMode4Bits() const;
120: 
121:     void SetBacklight(bool on) override;
122:     bool IsBacklightOn() const override;
123: 
124:     void Write(char value) override;
125:     void Write(const char *text, size_t count) override;
126:     void Write(const char *text) override;
127: 
128: protected:
129:     /// <summary>
130:     /// Write a half byte to the display
131:     ///
132:     /// This is used when using 4 bit mode. The upper 4 bits of the byte are used for data, the lower 4 bits contain the control bits (register select, read/write, enable, backlight) <br/>
133:     /// Bit 0: Instruction (0) or Data (1) <br/>
134:     /// Bit 1: Read (1) or write (0) <br/>
135:     /// Bit 2: Enable bit (will first be set on, then off) to latch data <br/>
136:     /// Bit 3: Backlight on (1) or off (0) <br/>
137:     /// Bit 4-7: 4 bits of the data to write (first the upper 4 bits, then the lower 4 bits are sent)
138:     /// </summary>
139:     /// <param name="data">Byte to be written (including control bits in lower 4 bits, data in upper 4 bits)</param>
140:     virtual void WriteHalfByte(uint8 data) = 0;
141: 
142: private:
143:     void         WriteInstruction(uint8 value);
144:     void         WriteData(uint8 value);
145:     void         WriteByte(uint8 data, RegisterSelect mode);
146: };
147: 
148: } // namespace device
```

- Line 55-64: We declare and enum class `DataMode` to set the data interface mode for the display (4 or 8 bits).
We'll only support the 4 bit mode
- Line 66-75: We declare and enum class `RegisterSelect` to select between a instruction or data register to write to
- Line 77-86: We declare and enum class `DisplayLines` to set the number of text lines in the display (either 1, or 2 or more)
- Line 88-97: We declare and enum class `CharacterSize` to set the display character size (either 5x8 or 5x10 pixels).
When multiple lines are used, this must always be 5x8 pixels
- Line 101-102: We declare a member variable `m_numRows` to hold the number of rows the display supports
- Line 103-104: We declare a member variable `m_numColumns` to hold the number of columns the display supports
- Line 105-106: We declare a member variable `m_dataMode` to hold the data interface mode
- Line 107-108: We declare a member variable `m_lineMode` to hold the display line mode
- Line 105-106: We declare a member variable `m_characterSize` to hold the display character size
- Line 112: We change the constructor to take the number of columns and rows, the character size, and the data interface mode
- Line 116: We override the method `GetNumColumns()`
- Line 117: We override the method `GetNumRows()`
- Line 118: We override the method `Initialize()`
- Line 119: We declare a method `IsDataMode4Bits()` to return the selected data interface mode
- Line 124: We override the method `Write()` for a single character
- Line 125: We override the method `Write()` for a string of characters with a count
- Line 126: We override the method `Write()` for a string of characters
- Line 129-140: We declare a protected abstract method `WriteHalfByte()` to write 4 bits to the device.
This make the class abstract again, an inheriting class will need to implement this
- Line 143: We declare a private method `WriteInstruction()` to write and instruction byte to the device
- Line 144: We declare a private method `WriteData()` to write and data byte to the device
- Line 145: We declare a private method `WriteByte()` to write an instruction or data byte to the device depending on the selected register

### HD44780Display.cpp {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_HD44780DISPLAYCPP}

We'll implement the new methods.

Update the file `code/libraries/device/src/display/HD44780Display.cpp`

```cpp
File: code/libraries/device/src/display/HD44780Display.cpp
...
40: #include "device/display/HD44780Display.h"
41: 
42: #include "baremetal/Logger.h"
43: #include "baremetal/Timer.h"
44: #include "stdlib/Util.h"
45: 
46: using namespace baremetal;
47: 
48: /// @file
49: /// HD44780 based I2C LCD character display (max 40x4 characters)
50: 
51: /// @brief Define log name
52: LOG_MODULE("HD44780Display");
53: 
54: namespace device {
55: 
56: /// @brief Bit pattern used for Command or Instruction. This refers to the value of the RS pin
57: static const uint8 MODE_CMD  = (0x00);
58: /// @brief Bit pattern used for Data. This refers to the value of the RS pin
59: static const uint8 MODE_DATA = (0x01);
60: 
61: /// @brief Display control
62: /// Sets cursor move direction and specifies display shift. These operations are performed during data write and read. <br/>
63: /// Display on/off control <br/>
64: /// 0 0 0 0 1 D C B <br/>
65: /// If D = 1, display is on, if D = 0, display is off <br/>
66: /// If C = 1, cursor is on, if C = 0, cursor is off <br/>
67: /// If B = 1, cursor is blinking, if B = 0, cursor is not blinking
68: static const uint8 LCD_DISPLAYCONTROL                 = (0x08);
69: /// @brief Display control, display on
70: static const uint8 LCD_DISPLAYCONTROL_ON              = (0x04);
71: /// @brief Display control, display off
72: static const uint8 LCD_DISPLAYCONTROL_OFF             = (0x00);
73: /// @brief Display control, cursor on
74: static const uint8 LCD_DISPLAYCONTROL_CURSOR_ON       = (0x02);
75: /// @brief Display control, cursor off
76: static const uint8 LCD_DISPLAYCONTROL_CURSOR_OFF      = (0x00);
77: /// @brief Display control, cursor blinking
78: static const uint8 LCD_DISPLAYCONTROL_CURSOR_BLINK    = (0x01);
79: /// @brief Display control, cursor not blinking
80: static const uint8 LCD_DISPLAYCONTROL_CURSOR_NO_BLINK = (0x00);
81: /// @brief Display control, cursor move or display shift
82: 
83: /// @brief Set display configuration
84: /// Write configuration to display <br/>
85: /// Function set <br/>
86: /// 0 0 1 DL N F * * <br/>
87: /// Sets interface data length (DL), number of display lines (N), and character font (F). <br/>
88: /// Needs to be done immediately after the initialization sequence and cannot be changed. <br/>
89: /// If DL = 1, set 8 bit mode, if DL = 0 set 4 bit mode (this is used for I2C devices and devices with only 4 data lanes) <br/>
90: /// If N = 1, set 2 (or more) line mode, if N = 0 set 1 line mode (this enables if desired 5x10 characters) <br/>
91: /// If F = 1, set 5x10 character size, if F = 0 set 5x8 character size. If N=1 only 5x8 character size is supported
92: static const uint8 LCD_FUNCTIONSET          = (0x20);
93: /// @brief Set 8 bit transfer mode
94: static const uint8 LCD_FUNCTIONSET_8BIT_MODE = (0x10);
95: /// @brief Set 4 bit transfer mode
96: static const uint8 LCD_FUNCTIONSET_4BIT_MODE = (0x00);
97: /// @brief Set display configuration to multiple lines
98: static const uint8 LCD_FUNCTIONSET_2_LINE    = (0x08);
99: /// @brief Set display configuration to single line
100: static const uint8 LCD_FUNCTIONSET_1_LINE    = (0x00);
101: /// @brief Set display font to 5x10 pixels (only for single line display)
102: static const uint8 LCD_FUNCTIONSET_5x10_DOTS = (0x04);
103: /// @brief Set display font to 5x8 pixels (default for multiple line display)
104: static const uint8 LCD_FUNCTIONSET_5x8_DOTS  = (0x00);
105: 
106: /// <summary>
107: /// Constructor
108: /// </summary>
109: /// <param name="numColumns">Number of text columns on display</param>
110: /// <param name="numRows">Number of text rows on display</param>
111: /// <param name="characterSize">Character size (5x8 or 5x10 pixels)</param>
112: /// <param name="dataMode">Data interface mode (4 bits or 8 bits)</param>
113: HD44780Display::HD44780Display(uint8 numColumns, uint8 numRows, CharacterSize characterSize /*= CharacterSize::Size5x8*/, DataMode dataMode /*= DataMode::Mode4Bit*/)
114:     : m_numRows{numRows}
115:     , m_numColumns{numColumns}
116:     , m_dataMode{dataMode}
117:     , m_lineMode{}
118:     , m_characterSize{characterSize}
119: {
120:     assert((numRows >= 1) && (numRows <= 4));
121:     assert((numColumns >= 1) && (numColumns <= 20));
122:     assert((characterSize == CharacterSize::Size5x8) || (characterSize == CharacterSize::Size5x10));
123:     assert((characterSize == CharacterSize::Size5x8) || (numRows == 1));
124: 
125:     m_lineMode = (numRows == 1) ? DisplayLines::LinesSingle : DisplayLines::LinesMulti;
126: }
127: 
128: /// <summary>
129: /// Destructor
130: /// </summary>
131: HD44780Display::~HD44780Display()
132: {
133:     // Don't write anymore, as derived class is already destroyed
134: }
135: 
136: /// <summary>
137: /// Return number of display columns
138: /// </summary>
139: /// <returns>Number of display columns</returns>
140: uint8 HD44780Display::GetNumColumns() const
141: {
142:     return m_numColumns;
143: }
144: 
145: /// <summary>
146: /// Return number of display rows
147: /// </summary>
148: /// <returns>Number of display rows</returns>
149: uint8 HD44780Display::GetNumRows() const
150: {
151:     return m_numRows;
152: }
153: 
154: /// <summary>
155: /// Initialize the display
156: /// </summary>
157: void HD44780Display::Initialize()
158: {
159:     // Initialization sequence (see page 45 / 46 of Hitachi HD44780 display controller datasheet
160:     // Choose 4 or 8 bit mode
161:     if (IsDataMode4Bits())
162:     {
163:         WriteHalfByte(0x30);
164:         Timer::WaitMicroSeconds(4500);
165:         WriteHalfByte(0x30);
166:         Timer::WaitMicroSeconds(100);
167:         WriteHalfByte(0x30);
168:         // See page 46 Hitachi HD44780 display controller datasheet
169:         WriteHalfByte(0x20);
170:     }
171:     else
172:     {
173:         WriteInstruction(0x03);
174:         Timer::WaitMicroSeconds(4500);
175:         WriteInstruction(0x03);
176:         Timer::WaitMicroSeconds(4500);
177:         WriteInstruction(0x03);
178:     }
179: 
180:     // Setup initial display configuration
181:     uint8 displayFunction = (m_dataMode == DataMode::Mode8Bit) ? LCD_FUNCTIONSET_8BIT_MODE : LCD_FUNCTIONSET_4BIT_MODE;
182:     displayFunction |= (m_lineMode == DisplayLines::LinesMulti) ? LCD_FUNCTIONSET_2_LINE : LCD_FUNCTIONSET_1_LINE;
183:     displayFunction |= (m_characterSize == CharacterSize::Size5x10) ? LCD_FUNCTIONSET_5x10_DOTS : LCD_FUNCTIONSET_5x8_DOTS;
184: 
185:     SetBacklight(true);
186:     WriteInstruction(LCD_FUNCTIONSET | displayFunction);
187:     Timer::WaitMicroSeconds(50);
188:     WriteInstruction(LCD_DISPLAYCONTROL | LCD_DISPLAYCONTROL_OFF);
189: }
190: 
191: /// <summary>
192: /// Switch backlight on or off
193: /// </summary>
194: /// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
195: void HD44780Display::SetBacklight(bool on)
196: {
197:     // Default implementation does not support backlight
198:     (void)on;
199: }
200: 
201: /// <summary>
202: /// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
203: /// </summary>
204: /// <returns>True if backlight is on, false otherwise</returns>
205: bool HD44780Display::IsBacklightOn() const
206: {
207:     // Default implementation does not support backlight
208:     return false;
209: }
210: 
211: /// <summary>
212: /// Write single character to display
213: /// </summary>
214: /// <param name="value"></param>
215: void HD44780Display::Write(char value)
216: {
217:     // Write a raw character byte to the HD44780 display.
218:     WriteData(static_cast<uint8>(value));
219: }
220: 
221: /// <summary>
222: /// Write multiple characters to display
223: /// </summary>
224: /// <param name="text">Point to string to write to display</param>
225: /// <param name="count">Count of characters to write to display</param>
226: void HD44780Display::Write(const char *text, size_t count)
227: {
228:     for (size_t i = 0; i < count; ++i)
229:     {
230:         Write(text[i]);
231:     }
232: }
233: 
234: /// <summary>
235: /// Write multiple characters to display
236: /// </summary>
237: /// <param name="text">Point to string to write to display</param>
238: void HD44780Display::Write(const char *text)
239: {
240:     Write(text, strlen(text));
241: }
242: 
243: /// <summary>
244: /// Return true if we configured the display to use 4 bit mode
245: /// </summary>
246: /// <returns>True if using 4 bit mode, false otherwise</returns>
247: bool HD44780Display::IsDataMode4Bits() const
248: {
249:     return m_dataMode == DataMode::Mode4Bit;
250: }
251: 
252: /// <summary>
253: /// Write instruction to HD44780 display controller
254: /// </summary>
255: /// <param name="value">Instruction byte (see LCD_x)</param>
256: void HD44780Display::WriteInstruction(uint8 value)
257: {
258:     WriteByte(value, RegisterSelect::Instruction);
259: }
260: 
261: /// <summary>
262: /// Write data to HD44780 display controller
263: /// </summary>
264: /// <param name="value">Data byte, character to be written</param>
265: void HD44780Display::WriteData(uint8 value)
266: {
267:     WriteByte(value, RegisterSelect::Data);
268: }
269: 
270: /// <summary>
271: /// Write a byte to the display controller
272: /// </summary>
273: /// <param name="data">Data byte to send</param>
274: /// <param name="mode">If equal to RegisterSelect::Cmd, this is an instruction, if equal to RegisterSelect::Data, this is data</param>
275: void HD44780Display::WriteByte(uint8 data, RegisterSelect mode)
276: {
277:     uint8 modeBits = (mode == RegisterSelect::Data) ? MODE_DATA : MODE_CMD;
278:     TRACE_DEBUG("Write %s byte %02x", (mode == RegisterSelect::Instruction ? "instruction" : "data"), data);
279:     if (m_dataMode == DataMode::Mode4Bit)
280:     {
281:         WriteHalfByte(modeBits | (data & 0xF0));
282:         WriteHalfByte(modeBits | ((data << 4) & 0xF0));
283:     }
284:     else
285:     {
286:         assert(false);
287:     }
288: }
289: 
290: } // namespace device
```

- Line 56-57: We define a constant `MODE_CMD` to denote sending an instruction byte
- Line 58-59: We define a constant `MODE_DATA` to denote sending a data byte
- Line 61-68: We define the display control command
- Line 69-70: We define the display control command value for setting the display to on
- Line 71-72: We define the display control command value for setting the display to off
- Line 73-74: We define the display control command value for setting the cursor to on
- Line 75-76: We define the display control command value for setting the cursor to off
- Line 77-78: We define the display control command value for setting the cursor to blinking
- Line 79-80: We define the display control command value for setting the cursor to steady
- Line 83-92: We define the function set or display configuration command
- Line 93-94: We define the function set value for setting the data interface mode to 8 bits
- Line 95-96: We define the function set value for setting the data interface mode to 4 bits
- Line 97-98: We define the function set value for setting the display configuration to 2 or more text lines
- Line 99-100: We define the function set value for setting the display configuration to 1 text line
- Line 101-102: We define the function set value for setting the display configuration to 5x10 pixels characters
- Line 103-104: We define the function set value for setting the display configuration to 5x8 pixels characters
- Line 106-126: We re-implement the constructor.
Next to initializing the member variables, this also asserts that the number of display columns and rows and the character size are valid
- Line 136-143: We implement the method `GetNumColumns()`
- Line 145-152: We implement the method `GetNumRows()`
- Line 154-189: We implement the method `Initialize()`
    - Line 161-170: For 4 bit mode, we write the initialization sequence as discussed in [Mode setting](#TUTORIAL_26_I2C_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_MODE_SETTING)
    - Line 172-178: Similarly, for 8 bit mode, we write the initialization sequence
    - Line 181-183: We calculate the display function set bits
    - Line 186: We write the display function set instruction
    - Line 187: We wait a very short time
    - Line 188: We write the display control instruction
- Line 211-219: We implement the method `Write()` for a single character
- Line 221-232: We implement the method `Write()` for a string with character count
- Line 234-241: We implement the method `Write()` for a string
- Line 243-250: We implement the method `IsDataMode4Bits()`
- Line 252-259: We implement the method `WriteInstruction()`
- Line 261-268: We implement the method `WriteData()`
- Line 270-288: We implement the method `WriteByte()` for a single character.
This will write the byte in two parts, first the lower 4 bits, then the higher 4 bits.
It uses the abstract method WrateHalfByte to do this.
As can be seen, we do not support 8 bit transfers

### HD44780DisplayI2C.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_HD44780DISPLAYI2CH}

We'll update `HD44780DisplayI2C` class, to add parameters to the constructor for configuring the display,
and implement the `WriteHalfByte()` method.

Create the file `code/libraries/device/include/device/i2c/HD44780DisplayI2C.h`

```cpp
File: code/libraries/device/include/device/i2c/HD44780DisplayI2C.h
...
40: #pragma once
41: 
42: #include "device/display/HD44780Display.h"
43: 
44: /// @file
45: /// HD44780 based I2C LCD character display (max 40x4 characters) with I2C piggyback
46: 
47: namespace baremetal {
48: 
49: class II2CMaster;
50: 
51: } // namespace baremetal
52: 
53: namespace device {
54: 
55: /// <summary>
56: /// I2C controlled HD44780 based LCD character display
57: /// </summary>
58: class HD44780DisplayI2C : public HD44780Display
59: {
60: private:
61:     /// @brief I2C master interface
62:     baremetal::II2CMaster& m_i2cMaster;
63:     /// @brief I2C address of the LCD controller
64:     uint8                  m_address;
65:     /// @brief Backlight status
66:     bool                   m_backlightOn;
67: 
68: public:
69:     HD44780DisplayI2C(baremetal::II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8);
70: 
71:     ~HD44780DisplayI2C();
72: 
73:     void SetBacklight(bool on) override;
74:     bool IsBacklightOn() const override;
75: 
76: protected:
77:     void WriteHalfByte(uint8 data) override;
78: };
79: 
80: } // namespace device
```

- Line 69: We add the number of columns and row, as well as the character size as parameters to the constructor
- Line 77: We override the abstract method `WriteHalfByte()`

### HD44780DisplayI2C.cpp {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_HD44780DISPLAYI2CCPP}

Next we implement the `HD44780DisplayI2C` class.

Create the file `code/libraries/device/src/i2c/HD44780DisplayI2C.cpp`

```cpp
File: code/libraries/device/src/i2c/HD44780DisplayI2C.cpp
...
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\device\src\i2c\HD44780DisplayI2C.cpp
40: #include "device/i2c/HD44780DisplayI2C.h"
41: 
42: #include "baremetal/I2CMaster.h"
43: #include "baremetal/Timer.h"
44: 
45: using namespace baremetal;
46: 
47: /// @file
48: /// HD44780 based I2C 16x2 LCD display
49: 
50: namespace device {
51: 
52: // HD44780Display via PCF8574 I2C port expander (for 16X2 LCD display
53: //  Pin mapping::
54: //  7  | 6  | 5  | 4  | 3  | 2  | 1  | 0
55: //  D3 | D2 | D1 | D0 | BK | EN | RW | RS
56: //
57: //  D3 : D7 (first write) and D3 (second write)
58: //  D2 : D6 (first write) and D2 (second write)
59: //  D1 : D5 (first write) and D1 (second write)
60: //  D0 : D4 (first write) and D0 (second write)
61: //  BK : Backlight off (0) or on (1)
62: //  EN : Enable. Needs to be strobed high to write
63: //  RW : Read (0) or Write (1)
64: //  RS : Instruction (0) or Data (1)
65: 
66: // PCF8574 I2C multiplexer signal mapping to HD44780 display
67: /// @brief RS pin is bit 0
68: static const uint8 PCF_RS = BIT1(0); // RS pin
69: /// @brief RW pin is bit 1
70: static const uint8 PCF_RW = BIT1(1); // RW pin
71: /// @brief EN pin is bit 2
72: static const uint8 PCF_EN = BIT1(2); // EN pin
73: /// @brief Backlight pin is bit 3
74: static const uint8 PCF_BK = BIT1(3); // Backlight pin
75: 
76: // Flags for RS pin modes
77: /// @brief Instruction register select (RS pin low)
78: static const uint8 RS_INSTRUCTION = (0x00);
79: /// @brief Data register select (RS pin high)
80: static const uint8 RS_DATA        = PCF_RS;
81: 
82: // Flags for backlight control
83: /// @brief Backlight on
84: const uint8 LCD_BACKLIGHT   = PCF_BK;
85: /// @brief Backlight off
86: const uint8 LCD_NOBACKLIGHT = 0x00;
87: 
88: /// <summary>
89: /// Constructor
90: ///
91: /// \note Driver uses 4-bit mode, pins D0-D3 are not used.
92: /// </summary>
93: /// <param name="i2cMaster">     I2C master interface</param>
94: /// <param name="address">       I2C device address</param>
95: /// <param name="numColumns">    Number of display text columns</param>
96: /// <param name="numRows">       Number of display text rows</param>
97: /// <param name="characterSize"> Display character size (5x8 or 5x10 pixels)</param>
98: HD44780DisplayI2C::HD44780DisplayI2C(II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows, CharacterSize characterSize /*= CharacterSize::Size5x8*/)
99:     : HD44780Display(numColumns, numRows, characterSize, HD44780Display::DataMode::Mode4Bit)
100:     , m_i2cMaster(i2cMaster)
101:     , m_address(address)
102:     , m_backlightOn{}
103: {
104: }
105: 
106: /// <summary>
107: /// Destructor
108: ///
109: /// Resets device back to 8 bit interface
110: /// </summary>
111: HD44780DisplayI2C::~HD44780DisplayI2C()
112: {
113:     SetBacklight(false);
114: }
115: 
116: /// <summary>
117: /// Switch backlight on or off
118: /// </summary>
119: /// <param name="on">If true, backlight is switched on, otherwise it is off</param>
120: void HD44780DisplayI2C::SetBacklight(bool on)
121: {
122:     if (on != m_backlightOn)
123:     {
124:         uint8 byte = (on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
125:         // We write a single byte with all other bits off. This will have no effect to the controller state, except for the backlight
126:         m_i2cMaster.Write(m_address, &byte, 1);
127:         m_backlightOn = on;
128:     }
129: }
130: 
131: /// <summary>
132: /// Returns backlight on / off status
133: /// </summary>
134: /// <returns>True if backlight is on, false otherwise</returns>
135: bool HD44780DisplayI2C::IsBacklightOn() const
136: {
137:     return m_backlightOn;
138: }
139: 
140: 
141: /// <summary>
142: /// Write a 4 bit value
143: ///
144: /// The most significant 4 bits are the data written, bit 0 acts as the register select bit (0 = instruction, 1 = data)
145: /// </summary>
146: /// <param name="data">Value to write</param>
147: void HD44780DisplayI2C::WriteHalfByte(uint8 data)
148: {
149:     uint8 byte = data | (m_backlightOn ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
150:     // Pulse the `enable` flag to process value.
151:     uint8 value = byte | PCF_EN;
152:     m_i2cMaster.Write(m_address, &value, 1);
153:     Timer::WaitMicroSeconds(1);
154:     value = byte & ~PCF_EN;
155:     m_i2cMaster.Write(m_address, &value, 1);
156:     // Wait for command to complete.
157:     Timer::WaitMicroSeconds(100);
158: }
159: 
160: } // namespace device
```

- Line 88-104: We update the constructor with the added parameters
- Line 141-158: We implement the method `WriteHalfByte()`.
Notice that we add the backlight bit to the data to be written to keep the backlight state.
Also we set the EN bit high first, then write again with the EN bit low.
This clocks the data into the controller.
We need to wait a short time to enable the controller to handle the data

### Application code {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_APPLICATION_CODE}

Let's now initialize the display and write some text.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/I2CMaster.h"
3: #include "baremetal/Logger.h"
4: #include "baremetal/System.h"
5: #include "baremetal/Timer.h"
6: #include "device/i2c/HD44780DisplayI2C.h"
7: 
8: LOG_MODULE("main");
9: 
10: using namespace baremetal;
11: using namespace device;
12: 
13: int main()
14: {
15:     auto& console = GetConsole();
16: 
17:     uint8 busIndex = 1;
18:     uint8 address{0x27};
19:     I2CMaster i2cMaster;
20:     i2cMaster.Initialize(busIndex);
21:     HD44780DisplayI2C display(i2cMaster, address, 16, 2);
22:     display.Initialize();
23:     display.Write("Hello");
24: 
25:     LOG_INFO("Wait 5 seconds");
26:     Timer::WaitMilliSeconds(5000);
27: 
28:     display.SetBacklight(false);
29: 
30:     console.Write("Press r to reboot, h to halt\n");
31:     char ch{};
32:     while ((ch != 'r') && (ch != 'h'))
33:     {
34:         ch = console.ReadChar();
35:         console.WriteChar(ch);
36:     }
37: 
38:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
39: }
```

- Line 18: We have the display connected to the I2C bus 1
- Line 19: As can be seen from the circuit diagram, lower 3 bits are all 1 by default.
The default address for PCF8574 is 0x20-0x27, which in our case is 0x27
- Line 20-21: We instantiate a `I2CMaster` and initialize it to bus 1
- Line 22: We instantiate a `HD44780DisplayI2C` and inject the `I2CMaster` instance, and set the I2C address
- Line 23: We switch the backlight on
- Line 28: We switch the backlight off again

### Configuring, building and debugging {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
You should see the display backlight turn on and then off again after 5 seconds.
As the backlight is on after power on, you will need to run the application twice to see it turn on and off.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.090 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.110 Set up bus 1, config 0, base address 3F804000 (I2CMaster:190)
Info   0.00:00:00.190 Wait 5 seconds (main:25)
Press r to reboot, h to halt
rInfo   0.00:00:13.520 Reboot (System:144)
```

The console does not show much, but the display will show the text "Hello" for 5 seconds, with the backlight on, and then switch off the backlight.

<img src="images/HD44780-display-on.png" alt="16 x 2 character display backlight on" width="400"/>

The text will still be visible, but hard to read without backlight.

<img src="images/HD44780-display-off.png" alt="16 x 2 character display backlight off" width="400"/>

## LCD display complete functionality {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_COMPLETE_FUNCTIONALITY}

Now that our display is working, let's complete the functionality of the HD44780 display.

### ITextDisplay.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_COMPLETE_FUNCTIONALITY_ITEXTDISPLAYH}

First we'll extend the `ITextDisplay` interface to include methods for controlling display, cursor and writing text.

Create the file `code/libraries/device/include/device/display/ITextDisplay.h`

```cpp
File: code/libraries/device/include/device/display/ITextDisplay.h
...
47: /// <summary>
48: /// Generic character matrix LCD display interface
49: /// </summary>
50: class ITextDisplay
51: {
52:   public:
53:     /// <summary>
54:     /// Cursur mode
55:     /// </summary>
56:     enum class CursorMode : uint8
57:     {
58:         /// @brief Cursor is hidden
59:         Hide,
60:         /// @brief Cursor is line
61:         Line,
62:         /// @brief Cursor is blinking block
63:         Blink,
64:         /// @brief Cursor is blinking line
65:         BlinkLine,
66:     };
67: 
68:     /// <summary>
69:     /// Destructor
70:     /// </summary>
71:     virtual ~ITextDisplay() = default;
72: 
73:     /// <summary>
74:     /// Retrieve the number of display columns
75:     /// </summary>
76:     /// <returns>Number of display columns</returns>
77:     virtual uint8 GetNumColumns() const = 0;
78:     /// <summary>
79:     /// Retrieve the number of display rows
80:     /// </summary>
81:     /// <returns>Number of display rows</returns>
82:     virtual uint8 GetNumRows() const = 0;
83: 
84:     /// <summary>
85:     /// Initialize the device
86:     /// </summary>
87:     virtual void Initialize() = 0;
88: 
89:     /// <summary>
90:     /// Set backlight on or off
91:     /// </summary>
92:     /// <param name="on">If true backlight is switched on, if false it is switched off</param>
93:     virtual void SetBacklight(bool on) = 0;
94:     /// <summary>
95:     /// Returns the current backlight status
96:     /// </summary>
97:     /// <returns>True if backlight is on, false otherwise</returns>
98:     virtual bool IsBacklightOn() const = 0;
99: 
100:     /// <summary>
101:     /// Write a character to the display, and update the cursor location
102:     ///
103:     /// The character is written in the buffer, depending on the shift it may or may not be visible
104:     /// </summary>
105:     /// <param name="value">Character to write</param>
106:     virtual void Write(char value) = 0;
107:     /// <summary>
108:     /// Write a character at the specified location
109:     ///
110:     /// The character is written in the buffer, depending on the shift it may or may not be visible
111:     /// </summary>
112:     /// <param name="posX">Horizontal position, 0..NumColumns-1</param>
113:     /// <param name="posY">Vertical position, 0..NumRows-1</param>
114:     /// <param name="ch"></param>
115:     virtual void Write(unsigned posX, unsigned posY, char ch) = 0;
116:     /// <summary>
117:     /// Write text to display, and update the cursor location
118:     ///
119:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
120:     /// </summary>
121:     /// <param name="text">Text to write</param>
122:     /// <param name="count">Number of characters to write</param>
123:     virtual void Write(const char *text, size_t count) = 0;
124:     /// <summary>
125:     /// Write text to display, and update the cursor location
126:     ///
127:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
128:     /// </summary>
129:     /// <param name="text">Text to write</param>
130:     virtual void Write(const char *text) = 0;
131:     /// <summary>
132:     /// Write text to display at the specified location
133:     ///
134:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
135:     /// </summary>
136:     /// <param name="posX">Horizontal position, 0..NumColumns-1</param>
137:     /// <param name="posY">Vertical position, 0..NumRows-1</param>
138:     /// <param name="text">Text to write</param>
139:     virtual void Write(unsigned posX, unsigned posY, const char *text) = 0;
140: 
141:     /// <summary>
142:     /// Clear the display contents
143:     /// </summary>
144:     virtual void ClearDisplay() = 0;
145:     /// <summary>
146:     /// Move the cursor to the home position
147:     /// </summary>
148:     virtual void Home() = 0;
149:     /// <summary>
150:     /// Shift the display horizontally
151:     /// </summary>
152:     /// <param name="amount">Number of characters to shift. Negative to shift left, positive to shift right</param>
153:     virtual void ShiftDisplay(int amount) = 0;
154:     /// <summary>
155:     /// Set display on or off
156:     /// </summary>
157:     /// <param name="on">If true characters are shown, if false they are hidden</param>
158:     virtual void SetDisplayEnabled(bool on) = 0;
159:     /// <summary>
160:     /// Set cursor mode
161:     /// </summary>
162:     /// <param name="mode">Mode to set for cursor</param>
163:     virtual void SetCursorMode(CursorMode mode) = 0;
164:     /// <summary>
165:     /// Retrieve the location of the cursor
166:     /// </summary>
167:     /// <param name="cursorX">Horizontal position, left = 0</param>
168:     /// <param name="cursorY">Vertical position, top = 0</param>
169:     virtual void GetCursorPosition(uint8& cursorX, uint8& cursorY) = 0;
170:     /// <summary>
171:     /// Set location of the cursor
172:     /// </summary>
173:     /// <param name="cursorX">Horizontal position, left = 0</param>
174:     /// <param name="cursorY">Vertical position, top = 0</param>
175:     virtual void SetCursorPosition(uint8 cursorX, uint8 cursorY) = 0;
176: };
```

- Line 53-66: We declare enum class `CursorMode` to denote the type of cursor shown on the display
- Line 107-115: We add a method `Write()` that writes a character at a specified row and column on the display
- Line 131-139: We add a method `Write()` that writes a string at a specified row and column on the display
- Line 141-144: We add a method `ClearDisplay()` to clear the display contents
- Line 145-148: We add a method `Home()` to move the cursor to the home position
- Line 149-153: We add a method `ShiftDisplay()` to shift the display horizontally to the left or right
- Line 154-158: We add a method `SetDisplayEnabled()` to show or hide the characters on the display
- Line 159-163: We add a method `SetCursorMode()` to set the cursor mode
- Line 164-169: We add a method `GetCursorPosition()` to get the cursor position
- Line 170-175: We add a method `SetCursorPosition()` to set the cursor position

### HD44780Display.h {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_COMPLETE_FUNCTIONALITY_HD44780DISPLAYH}

We'll extend the `HD44780Display` class again to implement the new `ITextDisplay` methods.

Update the file `code/libraries/device/include/device/display/HD44780Display.h`

```cpp
File: code/libraries/device/include/device/display/HD44780Display.h
...
47: namespace device {
48: 
49: /// <summary>
50: /// Hitachi HD44780 based LCD text display controller
51: /// </summary>
52: class HD44780Display : public ITextDisplay
53: {
54: public:
55:     /// <summary>
56:     /// Selection of data transfer mode. The HD44780 controller can be either used in 4 bit mode (using D7-D4) and 8 bit mode (using D7-D0)
57:     /// </summary>
58:     enum class DataMode : uint8
59:     {
60:         /// @brief 4 bit transfer mode
61:         Mode4Bit,
62:         /// @brief 8 bit transfer mode
63:         Mode8Bit,
64:     };
65: 
66:     /// <summary>
67:     /// Selection of the register to address in the LCD controller
68:     /// </summary>
69:     enum class RegisterSelect : uint8
70:     {
71:         /// @brief Instruction register
72:         Instruction,
73:         /// @brief Data register
74:         Data,
75:     };
76: 
77:     /// <summary>
78:     /// Display configuration
79:     /// </summary>
80:     enum class DisplayLines : uint8
81:     {
82:         /// @brief Single row display
83:         LinesSingle,
84:         /// @brief Multi row display
85:         LinesMulti,
86:     };
87: 
88:     /// <summary>
89:     /// Selects character size
90:     /// </summary>
91:     enum class CharacterSize : uint8
92:     {
93:         /// @brief Character size 5 x 10 pixels
94:         Size5x10,
95:         /// @brief Character size 5 x 8 pixels
96:         Size5x8,
97:     };
98: 
99:     /// <summary>
100:     /// Mode on writing
101:     /// </summary>
102:     enum class ShiftMode : uint8
103:     {
104:         /// @brief Shift display on write
105:         DisplayShift,
106:         /// @brief Move cursor on write
107:         CursorMove,
108:     };
109: 
110:     /// <summary>
111:     /// Display mode
112:     /// </summary>
113:     enum class DisplayMode : uint8
114:     {
115:         /// @brief Show characters
116:         Show,
117:         /// @brief Hide characters
118:         Hide,
119:     };
120: 
121: private:
122:     /// @brief Number of rows on the display
123:     uint8         m_numRows;
124:     /// @brief Number of columns on the display
125:     uint8         m_numColumns;
126:     /// @brief Data transfer mode (4 or 8 bit)
127:     DataMode      m_dataMode;
128:     /// @brief Whether display has single or multiple lines
129:     DisplayLines  m_lineMode;
130:     /// @brief Character size (5x8 or 5x10)
131:     CharacterSize m_characterSize;
132:     /// @brief Cursor mode (none, line, block)
133:     CursorMode    m_cursorMode;
134:     /// @brief Display mode (internal combination of display and cursor mode)
135:     uint8         m_displayMode;
136:     /// @brief Cursor position column
137:     uint8         m_cursorX;
138:     /// @brief Cursor position row
139:     uint8         m_cursorY;
140:     /// <summary>
141:     /// Row offsets for multi line display (max 4 lines)
142:     /// </summary>
143:     /// <returns>Row offsets for multi line display</returns>
144:     uint8         m_rowOffsets[4] __attribute__((aligned(8)));
145: 
146: public:
147:     HD44780Display(uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8, DataMode dataMode = DataMode::Mode4Bit);
148: 
149:     virtual ~HD44780Display();
150: 
151:     uint8 GetNumColumns() const override;
152:     uint8 GetNumRows() const override;
153:     void Initialize() override;
154:     bool IsDataMode4Bits() const;
155: 
156:     void SetBacklight(bool on) override;
157:     bool IsBacklightOn() const override;
158: 
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\device\include\device\display\HD44780Display.h
159:     void Write(char value) override;
160:     void Write(unsigned posX, unsigned posY, char ch) override;
161:     void Write(const char *text, size_t count) override;
162:     void Write(const char *text) override;
163:     void Write(unsigned posX, unsigned posY, const char *text) override;
164: 
165:     void ClearDisplay() override;
166:     void Home() override;
167:     void ShiftDisplay(int amount) override;
168:     void SetDisplayEnabled(bool on) override;
169:     void SetCursorMode(CursorMode mode) override;
170:     void GetCursorPosition(uint8& cursorX, uint8& cursorY) override;
171:     void SetCursorPosition(uint8 cursorX, uint8 cursorY) override;
172: 
173:     void DisplayControl(DisplayMode displayMode, CursorMode cursorMode);
174:     void DefineCharFont(char ch, const uint8 FontData[8]);
175: 
176:     protected:
177:     /// <summary>
178:     /// Write a half byte to the display
179:     ///
180:     /// This is used when using 4 bit mode. The upper 4 bits of the byte are used for data, the lower 4 bits contain the control bits (register select, read/write, enable, backlight) <br/>
181:     /// Bit 0: Instruction (0) or Data (1) <br/>
182:     /// Bit 1: Read (1) or write (0) <br/>
183:     /// Bit 2: Enable bit (will first be set on, then off) to latch data <br/>
184:     /// Bit 3: Backlight on (1) or off (0) <br/>
185:     /// Bit 4-7: 4 bits of the data to write (first the upper 4 bits, then the lower 4 bits are sent)
186:     /// </summary>
187:     /// <param name="data">Byte to be written (including control bits in lower 4 bits, data in upper 4 bits)</param>
188:     virtual void WriteHalfByte(uint8 data) = 0;
189: 
190: private:
191:     void WriteInstruction(uint8 value);
192:     void WriteData(uint8 value);
193:     void WriteByte(uint8 data, RegisterSelect mode);
194:     void CursorOrDisplayShift(ShiftMode mode, uint8 direction);
195:     void UpdateDisplayControl();
196:     uint8 ConvertCursorMode() const;
197:     void SetCGRAM_Address(uint8 address);
198:     void SetDDRAM_Address(uint8 address);
199: };
```

- Line 99-108: We add enum class `ShiftMode` to denote whether the display or the cursor is shifted on write
- Line 110-119: We add enum class `DisplayMode` to denote whether the characters are shown or hidden
- Line 132-133: We add a member variable `m_cursorMode` to store the current cursor mode
- Line 134-135: We add a member variable `m_displayMode` to store the current display mode
- Line 136-139: We add member variables `m_cursorX` and `m_cursorY` to store the current cursor position
- Line 140-144: We add a member variable `m_rowOffsets` to store the row offsets for multi line displays.
The memory organization of the HD44780 controller is such that the first line starts at address 0x00, the second line at 0x40, the third line at 0x00 + the number of columns, and the fourth line at 0x40 + the number of columns (all if existing of course)
- Line 160: We implement the method `Write()` that writes a character at a specified row and column on the display
- Line 163: We implement the method `Write()` that writes a string at a specified row and column on the display
- Line 165: We implement the method `ClearDisplay()`
- Line 166: We implement the method `Home()`
- Line 167: We implement the method `ShiftDisplay()`
- Line 168: We implement the method `SetDisplayEnabled()`
- Line 169: We implement the method `SetCursorMode()`
- Line 170: We implement the method `GetCursorPosition()`
- Line 171: We implement the method `SetCursorPosition()`
- Line 173: We add a method `DisplayControl()` to set the display and cursor mode
- Line 174: We add a method `DefineCharFont()` to define a custom character font
- Line 194: We add a method `CursorOrDisplayShift()` to shift the cursor or display depending on the mode specified
- Line 195: We add a method `UpdateDisplayControl()` to update the display control register
- Line 196: We add a method `ConvertCursorMode()` to convert the cursor mode enum to the corresponding bit value for the display control register
- Line 197: We add a method `SetCGRAM_Address()` to set the CGRAM address, which is used for defining custom character fonts
- Line 198: We add a method `SetDDRAM_Address()` to set the DDRAM address, which is used for setting the cursor position

### HD44780Display.cpp {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_COMPLETE_FUNCTIONALITY_HD44780DISPLAYCPP}

We'll implement the new methods.

Update the file `code/libraries/device/src/display/HD44780Display.cpp`

```cpp
File: code/libraries/device/src/display/HD44780Display.cpp
...
54: namespace device {
55: 
56: /// @brief Bit pattern used for Command or Instruction. This refers to the value of the RS pin
57: static const uint8 MODE_CMD  = (0x00);
58: /// @brief Bit pattern used for Data. This refers to the value of the RS pin
59: static const uint8 MODE_DATA = (0x01);
60: 
61: /// @brief Clear Display command
62: ///
63: /// See [HD44780 (Dot Matrix Liquid Crystal Display Controller/Driver) documentation](pdf/HD44780.pdf), page 24/25<br/>
64: /// Overwrite display with blank characters and reset cursor position.<br/>
65: /// Clear display<br/>
66: /// 0 0 0 0 0 0 0 1<br/>
67: /// Clears entire display and sets DDRAM address 0 in address counter.
68: static const uint8 LCD_CLEARDISPLAY = (0x01);
69: 
70: /// @brief Home command
71: ///
72: /// Set cursor to initial position and reset any shifting.<br/>
73: /// Return home<br/>
74: /// 0 0 0 0 0 0 1 *<br/>
75: /// Sets DDRAM address 0 in address counter. Also returns display from being shifted to original position. DDRAM contents remain unchanged.
76: static const uint8 LCD_RETURNHOME = (0x02);
77: 
78: /// @brief Entry mode set
79: ///
80: /// Sets cursor move direction and specifies display shift. These operations are performed during data write and read.<br/>
81: /// Entry mode set<br/>
82: /// 0 0 0 0 0 1 I/D S<br/>
83: /// If S = 1, shift display right (I/D = 0) or left (I/D = 1) on write.<br/>
84: /// If S = 0, move cursor left (I/D = 0) or right (I/D = 1) on write.
85: static const uint8 LCD_ENTRYMODESET         = (0x04);
86: /// @brief Entry mode set, cursor move right / shift right on write
87: static const uint8 LCD_ENTRYMODE_MOVE_RIGHT = (0x02);
88: /// @brief Entry mode set, cursor move left / shift left on write
89: static const uint8 LCD_ENTRYMODE_MOVE_LEFT  = (0x00);
90: /// @brief Entry mode set, shift mode
91: static const uint8 LCD_ENTRYMODE_SHIFT      = (0x01);
92: /// @brief Entry mode set, cursor move mode
93: static const uint8 LCD_ENTRYMODE_NO_SHIFT   = (0x00);
94: 
95: /// @brief Display control
96: /// Sets cursor move direction and specifies display shift. These operations are performed during data write and read.<br/>
97: /// Display on/off control<br/>
98: /// 0 0 0 0 1 D C B<br/>
99: /// If D = 1, display is on, if D = 0, display is off<br/>
100: /// If C = 1, cursor is on, if C = 0, cursor is off<br/>
101: /// If B = 1, cursor is blinking, if B = 0, cursor is not blinking
102: static const uint8 LCD_DISPLAYCONTROL                 = (0x08);
103: /// @brief Display control, display on
104: static const uint8 LCD_DISPLAYCONTROL_ON              = (0x04);
105: /// @brief Display control, display off
106: static const uint8 LCD_DISPLAYCONTROL_OFF             = (0x00);
107: /// @brief Display control, cursor on
108: static const uint8 LCD_DISPLAYCONTROL_CURSOR_ON       = (0x02);
109: /// @brief Display control, cursor off
110: static const uint8 LCD_DISPLAYCONTROL_CURSOR_OFF      = (0x00);
111: /// @brief Display control, cursor blinking
112: static const uint8 LCD_DISPLAYCONTROL_CURSOR_BLINK    = (0x01);
113: /// @brief Display control, cursor not blinking
114: static const uint8 LCD_DISPLAYCONTROL_CURSOR_NO_BLINK = (0x00);
115: 
116: /// @brief Display cursor move or display shift
117: ///
118: /// Cursor or display shift<br/>
119: /// 0 0 0 1 S/C R/L * *<br/>
120: /// Moves cursor and shifts display without changing DDRAM contents.<br/>
121: /// if S/C = 0, the cursor is moved to the left (R/L = 0) or right (R/L = 1)<br/>
122: /// if S/C = 1, the entire screen is moved to the left (R/L = 0) or right (R/L = 1)
123: static const uint8 LCD_CURSORSHIFT              = (0x10);
124: /// @brief Display shift
125: static const uint8 LCD_CURSORSHIFT_SCREEN       = (0x08);
126: /// @brief Display shift left
127: static const uint8 LCD_CURSORSHIFT_SCREEN_LEFT  = (0x08);
128: /// @brief Display shift right
129: static const uint8 LCD_CURSORSHIFT_SCREEN_RIGHT = (0x0C);
130: /// @brief Cursor move
131: static const uint8 LCD_CURSORSHIFT_CURSOR       = (0x00);
132: /// @brief Cursor move left
133: static const uint8 LCD_CURSORSHIFT_CURSOR_LEFT  = (0x00);
134: /// @brief Cursor move right
135: static const uint8 LCD_CURSORSHIFT_CURSOR_RIGHT = (0x04);
136: 
137: /// @brief Set display configuration
138: /// Write configuration to display<br/>
139: /// Function set<br/>
140: /// 0 0 1 DL N F * *<br/>
141: /// Sets interface data length (DL), number of display lines (N), and character font (F).<br/>
142: /// Needs to be done immediately after the initialization sequence and cannot be changed.<br/>
143: /// If DL = 1, set 8 bit mode, if DL = 0 set 4 bit mode (this is used for I2C devices and devices with only 4 data lanes)<br/>
144: /// If N = 1, set 2 (or more) line mode, if N = 0 set 1 line mode (this enables if desired 5x10 characters)<br/>
145: /// If F = 1, set 5x10 character size, if F = 0 set 5x8 character size. If N=1 only 5x8 character size is supported
146: static const uint8 LCD_FUNCTIONSET          = (0x20);
147: /// @brief Set 8 bit transfer mode
148: static const uint8 LCD_FUNCTIONSET_8BIT_MODE = (0x10);
149: /// @brief Set 4 bit transfer mode
150: static const uint8 LCD_FUNCTIONSET_4BIT_MODE = (0x00);
151: /// @brief Set display configuration to multiple lines
152: static const uint8 LCD_FUNCTIONSET_2_LINE    = (0x08);
153: /// @brief Set display configuration to single line
154: static const uint8 LCD_FUNCTIONSET_1_LINE    = (0x00);
155: /// @brief Set display font to 5x10 pixels (only for single line display)
156: static const uint8 LCD_FUNCTIONSET_5x10_DOTS = (0x04);
157: /// @brief Set display font to 5x8 pixels (default for multiple line display)
158: static const uint8 LCD_FUNCTIONSET_5x8_DOTS  = (0x00);
159: 
160: /// @brief Set CGRAM start address
161: ///
162: /// Set CGRAM address<br/>
163: /// 0 1 ACG ACG ACG ACG ACG ACG<br/>
164: /// Sets CGRAM address. CGRAM data is sent and received after this setting.<br/>
165: /// Sets the initial address for data write, the address is incremented after each write.
166: static const uint8 LCD_SETCGRAMADDR = (0x40);
167: 
168: /// @brief Set DDRAM start address
169: ///
170: /// Set DDRAM address<br/>
171: /// 1 ADD ADD ADD ADD ADD ADD ADD<br/>
172: /// Sets DDRAM address. DDRAM data is sent and received after this setting.<br/>
173: /// Sets the initial address for data write, the address is incremented after each write.
174: static const uint8 LCD_SETDDRAMADDR = (0x80);
175: 
176: /// <summary>
177: /// Constructor
178: /// </summary>
179: /// <param name="numColumns">Number of text columns on display</param>
180: /// <param name="numRows">Number of text rows on display</param>
181: /// <param name="characterSize">Character size (5x8 or 5x10 pixels)</param>
182: /// <param name="dataMode">Data interface mode (4 bits or 8 bits)</param>
183: HD44780Display::HD44780Display(uint8 numColumns, uint8 numRows, CharacterSize characterSize /*= CharacterSize::Size5x8*/, DataMode dataMode /*= DataMode::Mode4Bit*/)
184:     : m_numRows{numRows}
185:     , m_numColumns{numColumns}
186:     , m_dataMode{dataMode}
187:     , m_lineMode{}
188:     , m_characterSize{characterSize}
189:     , m_cursorMode{}
190:     , m_displayMode{}
191:     , m_cursorX{}
192:     , m_cursorY{}
193:     , m_rowOffsets{}
194: {
195:     assert((numRows >= 1) && (numRows <= 4));
196:     assert((numColumns >= 1) && (numColumns <= 20));
197:     assert((characterSize == CharacterSize::Size5x8) || (characterSize == CharacterSize::Size5x10));
198:     assert((characterSize == CharacterSize::Size5x8) || (numRows == 1));
199: 
200:     m_lineMode = (numRows == 1) ? DisplayLines::LinesSingle : DisplayLines::LinesMulti;
201:     for (uint8 i = 0; i < numRows; ++i)
202:     {
203:         if (i % 2 == 1)
204:             m_rowOffsets[i] = (i * 0x40); // 0x40 is the default offset for the next row when row is odd
205:         if (i >= 2)
206:             m_rowOffsets[i] += m_numColumns; // For rows 2 and 3, add the number of columns to the offset
207:     }
208: }
209: 
210: /// <summary>
211: /// Destructor
212: /// </summary>
213: HD44780Display::~HD44780Display()
214: {
215:     // Don't write anymore, as derived class is already destroyed
216: }
217: 
218: /// <summary>
219: /// Return number of display columns
220: /// </summary>
221: /// <returns>Number of display columns</returns>
222: uint8 HD44780Display::GetNumColumns() const
223: {
224:     return m_numColumns;
225: }
226: 
227: /// <summary>
228: /// Return number of display rows
229: /// </summary>
230: /// <returns>Number of display rows</returns>
231: uint8 HD44780Display::GetNumRows() const
232: {
233:     return m_numRows;
234: }
235: 
236: /// <summary>
237: /// Initialize the display
238: /// </summary>
239: void HD44780Display::Initialize()
240: {
241:     // Initialization sequence (see page 45 / 46 of Hitachi HD44780 display controller datasheet
242:     // Choose 4 or 8 bit mode
243:     if (IsDataMode4Bits())
244:     {
245:         WriteHalfByte(0x30);
246:         Timer::WaitMicroSeconds(4500);
247:         WriteHalfByte(0x30);
248:         Timer::WaitMicroSeconds(100);
249:         WriteHalfByte(0x30);
250:         // See page 46 Hitachi HD44780 display controller datasheet
251:         WriteHalfByte(0x20);
252:     }
253:     else
254:     {
255:         WriteInstruction(0x03);
256:         Timer::WaitMicroSeconds(4500);
257:         WriteInstruction(0x03);
258:         Timer::WaitMicroSeconds(4500);
259:         WriteInstruction(0x03);
260:     }
261: 
262:     // Setup initial display configuration
263:     uint8 displayFunction = (m_dataMode == DataMode::Mode8Bit) ? LCD_FUNCTIONSET_8BIT_MODE : LCD_FUNCTIONSET_4BIT_MODE;
264:     displayFunction |= (m_lineMode == DisplayLines::LinesMulti) ? LCD_FUNCTIONSET_2_LINE : LCD_FUNCTIONSET_1_LINE;
265:     displayFunction |= (m_characterSize == CharacterSize::Size5x10) ? LCD_FUNCTIONSET_5x10_DOTS : LCD_FUNCTIONSET_5x8_DOTS;
266: 
267:     SetBacklight(false);
268:     WriteInstruction(LCD_FUNCTIONSET | displayFunction);
269:     Timer::WaitMicroSeconds(50);
270:     WriteInstruction(LCD_DISPLAYCONTROL | LCD_DISPLAYCONTROL_OFF);
271:     WriteInstruction(LCD_CLEARDISPLAY);
272:     WriteInstruction(LCD_ENTRYMODESET | LCD_ENTRYMODE_MOVE_RIGHT | LCD_ENTRYMODE_NO_SHIFT); // set move cursor right, do not shift display
273: }
274: 
275: /// <summary>
276: /// Switch backlight on or off
277: /// </summary>
278: /// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
279: void HD44780Display::SetBacklight(bool on)
280: {
281:     // Default implementation does not support backlight
282:     (void)on;
283: }
284: 
285: /// <summary>
286: /// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
287: /// </summary>
288: /// <returns>True if backlight is on, false otherwise</returns>
289: bool HD44780Display::IsBacklightOn() const
290: {
291:     // Default implementation does not support backlight
292:     return false;
293: }
294: 
295: /// <summary>
296: /// Write single character to display
297: /// </summary>
298: /// <param name="value"></param>
299: void HD44780Display::Write(char value)
300: {
301:     // Write a raw character byte to the LCD16X2.
302:     WriteData(static_cast<uint8>(value));
303:     if (m_cursorX < m_numColumns - 1)
304:     {
305:         m_cursorX += 1;
306:     }
307:     else
308:     {
309:         // At end of line : go to left side next row. Wrap around to first row if on last row.
310:         m_cursorY = (m_cursorY + 1) % m_numRows;
311:         m_cursorX = 0;
312:     }
313: }
314: 
315: /// <summary>
316: /// Write a character at a specific position on the display
317: /// </summary>
318: /// <param name="posX"></param>
319: /// <param name="posY"></param>
320: /// <param name="ch"></param>
321: void HD44780Display::Write(unsigned posX, unsigned posY, char ch)
322: {
323:     SetCursorPosition(posX, posY);
324:     Write(ch);
325: }
326: 
327: /// <summary>
328: /// Write multiple characters to display
329: /// </summary>
330: /// <param name="text">Point to string to write to display</param>
331: /// <param name="count">Count of characters to write to display</param>
332: void HD44780Display::Write(const char *text, size_t count)
333: {
334:     for (size_t i = 0; i < count; ++i)
335:     {
336:         Write(text[i]);
337:     }
338: }
339: 
340: /// <summary>
341: /// Write multiple characters to display
342: /// </summary>
343: /// <param name="text">Point to string to write to display</param>
344: void HD44780Display::Write(const char *text)
345: {
346:     Write(text, strlen(text));
347: }
348: 
349: /// <summary>
350: /// Write multiple characters to display
351: /// </summary>
352: /// <param name="posX"></param>
353: /// <param name="posY"></param>
354: /// <param name="text">Point to string to write to display</param>
355: void HD44780Display::Write(unsigned posX, unsigned posY, const char *text)
356: {
357:     SetCursorPosition(posX, posY);
358:     Write(text, strlen(text));
359: }
360: 
361: /// <summary>
362: /// Clear the display contents, and position the cursor home
363: /// </summary>
364: void HD44780Display::ClearDisplay()
365: {
366:     WriteInstruction(LCD_CLEARDISPLAY);
367:     Timer::WaitMicroSeconds(2000);
368:     Home();
369: }
370: 
371: /// <summary>
372: /// Move the cursor to the home position
373: /// </summary>
374: void HD44780Display::Home()
375: {
376:     WriteInstruction(LCD_RETURNHOME);
377:     Timer::WaitMicroSeconds(2000);
378:     m_cursorX = 0;
379:     m_cursorY = 0;
380: }
381: 
382: /// <summary>
383: /// Shift display contents horizontally
384: /// </summary>
385: /// <param name="amount">Amound to shift by. If negative shifts left by -amount characters, if positive shifts right by amount characters. A value of 0 means no change.</param>
386: void HD44780Display::ShiftDisplay(int amount)
387: {
388:     if (amount == 0)
389:         return;
390: 
391:     auto direction = (amount > 0) ? LCD_CURSORSHIFT_CURSOR_RIGHT : LCD_CURSORSHIFT_CURSOR_LEFT;
392:     if (amount < 0)
393:         amount = -amount;
394:     for (int i = 0; i < amount; ++i)
395:     {
396:         CursorOrDisplayShift(ShiftMode::DisplayShift, direction);
397:         Timer::WaitMicroSeconds(50);
398:     }
399: }
400: 
401: /// <summary>
402: /// Set display on or off
403: /// </summary>
404: /// <param name="on">If true switch display to show mode, otherwise switch to hide mode</param>
405: void HD44780Display::SetDisplayEnabled(bool on)
406: {
407:     m_displayMode = (on) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;
408:     UpdateDisplayControl();
409: }
410: 
411: /// <summary>
412: /// Set cursor mode
413: /// </summary>
414: /// <param name="mode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
415: void HD44780Display::SetCursorMode(CursorMode mode)
416: {
417:     m_cursorMode = mode;
418:     UpdateDisplayControl();
419: }
420: 
421: /// <summary>
422: /// Retrieve the cursor position
423: /// </summary>
424: /// <param name="cursorX">Cursor column</param>
425: /// <param name="cursorY">Cursor row</param>
426: void HD44780Display::GetCursorPosition(uint8& cursorX, uint8& cursorY)
427: {
428:     SetDDRAM_Address(m_rowOffsets[cursorY] + cursorX);
429:     m_cursorX = cursorX;
430:     m_cursorY = cursorY;
431: }
432: 
433: /// <summary>
434: /// Change the cursor position
435: /// </summary>
436: /// <param name="cursorX">New cursor column</param>
437: /// <param name="cursorY">New cursor row</param>
438: void HD44780Display::SetCursorPosition(uint8 cursorX, uint8 cursorY)
439: {
440:     SetDDRAM_Address(m_rowOffsets[cursorY] + cursorX);
441:     m_cursorX = cursorX;
442:     m_cursorY = cursorY;
443: }
444: 
445: /// <summary>
446: /// Set display mode and cursor mode
447: /// </summary>
448: /// <param name="displayMode">Display on (DisplayMode::Show) or off (DisplayMode::Hide)</param>
449: /// <param name="cursorMode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
450: void HD44780Display::DisplayControl(DisplayMode displayMode, CursorMode cursorMode)
451: {
452:     m_cursorMode  = cursorMode;
453:     m_displayMode = (displayMode == DisplayMode::Show) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;
454:     
455:     UpdateDisplayControl();
456: }
457: 
458: /// <summary>
459: /// Define a user defined character
460: ///
461: /// Only characters 0x80-0x87 are user defined. The font data is 8 lines, each line is one byte. Only 5x8 pixel
462: /// characters are supported for now.
463: /// </summary>
464: /// <param name="ch">Character to define</param>
465: /// <param name="FontData">Font data. For every line there is one byte, only the low order 5 bits are used</param>
466: void HD44780Display::DefineCharFont(char ch, const uint8 FontData[8])
467: {
468:     uint8 data = static_cast<uint8>(ch);
469:     if (data < 0x08 || data > 0x0F)
470:     {
471:         return;
472:     }
473: 
474:     SetCGRAM_Address(data << 3);
475: 
476:     for (unsigned line = 0; line <= 7; line++)
477:     {
478:         WriteData(FontData[line] & 0x1F);
479:     }
480: }
481: 
482: /// <summary>
483: /// Return true if we configured the display to use 4 bit mode
484: /// </summary>
485: /// <returns>True if using 4 bit mode, false otherwise</returns>
486: bool HD44780Display::IsDataMode4Bits() const
487: {
488:     return m_dataMode == DataMode::Mode4Bit;
489: }
490: 
491: /// <summary>
492: /// Write instruction to HD44780 display controller
493: /// </summary>
494: /// <param name="value">Instruction byte (see LCD_x)</param>
495: void HD44780Display::WriteInstruction(uint8 value)
496: {
497:     WriteByte(value, RegisterSelect::Instruction);
498: }
499: 
500: /// <summary>
501: /// Write data to HD44780 display controller
502: /// </summary>
503: /// <param name="value">Data byte, character to be written</param>
504: void HD44780Display::WriteData(uint8 value)
505: {
506:     WriteByte(value, RegisterSelect::Data);
507: }
508: 
509: /// <summary>
510: /// Write a byte to the display controller
511: /// </summary>
512: /// <param name="data">Data byte to send</param>
513: /// <param name="mode">If equal to RegisterSelect::Cmd, this is an instruction, if equal to RegisterSelect::Data, this is data</param>
514: void HD44780Display::WriteByte(uint8 data, RegisterSelect mode)
515: {
516:     uint8 modeBits = (mode == RegisterSelect::Data) ? MODE_DATA : MODE_CMD;
517:     TRACE_DEBUG("Write %s byte %02x", (mode == RegisterSelect::Instruction ? "instruction" : "data"), data);
518:     if (m_dataMode == DataMode::Mode4Bit)
519:     {
520:         WriteHalfByte(modeBits | (data & 0xF0));
521:         WriteHalfByte(modeBits | ((data << 4) & 0xF0));
522:     }
523:     else
524:     {
525:         assert(false);
526:     }
527: }
528: 
529: /// <summary>
530: /// Shift cursor or display contents horizontally
531: /// </summary>
532: /// <param name="mode">If equal to ShiftMode::DisplayShift, the display contents are shifted, if equal to ShiftMode::CursorMove move the cursor to a new location</param>
533: /// <param name="direction">Direction to move. If equal to LCD_CURSORSHIFT_CURSOR_RIGHT moves to the right, otherwise moves to the left.</param>
534: void HD44780Display::CursorOrDisplayShift(ShiftMode mode, uint8 direction)
535: {
536:     WriteInstruction(LCD_CURSORSHIFT | ((mode == ShiftMode::DisplayShift) ? LCD_CURSORSHIFT_SCREEN : LCD_CURSORSHIFT_CURSOR) | direction);
537: }
538: 
539: /// <summary>
540: /// Update the display control register with the current display mode and cursor mode
541: /// </summary>
542: void HD44780Display::UpdateDisplayControl()
543: {
544:     WriteInstruction(LCD_DISPLAYCONTROL | m_displayMode | ConvertCursorMode());
545: }
546: 
547: /// <summary>
548: /// Convert the currently set cursor mode to display control register bits
549: /// </summary>
550: /// <returns>Cursor mode bits for display control register</returns>
551: uint8 HD44780Display::ConvertCursorMode() const
552: {
553:     return (m_cursorMode == CursorMode::BlinkLine)
554:         ? LCD_DISPLAYCONTROL_CURSOR_BLINK | LCD_DISPLAYCONTROL_CURSOR_ON
555:         : (m_cursorMode == CursorMode::Blink)
556:           ? LCD_DISPLAYCONTROL_CURSOR_BLINK
557:           : ((m_cursorMode == CursorMode::Line)
558:             ? LCD_DISPLAYCONTROL_CURSOR_ON 
559:             : LCD_DISPLAYCONTROL_CURSOR_OFF);
560: }
561: 
562: /// <summary>
563: /// Set dislay Character Generator RAM (CGRAM) address
564: ///
565: /// Data written subsequently is written to CGRAM starting at the specified address, incrementing by 1 for each byte
566: /// written.
567: /// </summary>
568: /// <param name="address">New CGRAM address (6 bits)</param>
569: void HD44780Display::SetCGRAM_Address(uint8 address)
570: {
571:     // Set CGRAM address
572:     // 0 1 ACG ACG ACG ACG ACG ACG
573:     // Sets CGRAM address.CGRAM data is sent and received after this setting.
574:     WriteInstruction(LCD_SETCGRAMADDR | (address & 0x3F));
575: }
576: 
577: /// <summary>
578: /// Set Display Data RAM (DDRAM) address
579: ///
580: /// Data written subsequently is written to DDRAM (visible display) starting at the specified address, incrementing by 1 for each byte
581: /// written.
582: /// </summary>
583: /// <param name="address">New display RAM address (7 bits)</param>
584: void HD44780Display::SetDDRAM_Address(uint8 address)
585: {
586:     // Set DDRAM address
587:     // 1 ADD ADD ADD ADD ADD ADD ADD
588:     // Sets DDRAM address. DDRAM data is sent and received after this setting.
589:     WriteInstruction(LCD_SETDDRAMADDR | (address & 0x7F));
590: }
591: 
592: } // namespace device
```

- Line 61-68: We define the clear display command
- Line 70-76: We define the home command
- Line 78-93: We define the entry mode set command
- Line 86-87: We define the entry mode set value for cursor move right / shift right on write
- Line 88-89: We define the entry mode set value for cursor move left / shift left on write
- Line 90-91: We define the entry mode set value for shift on write
- Line 92-93: We define the entry mode set value for cursor move on write
- Line 116-123: We define the display cursor move or display shift command
- Line 124-125: We define the display shift value for shifting the entire screen
- Line 126-127: We define the display shift value for shifting left
- Line 128-129: We define the display shift value for shifting right
- Line 130-131: We define the cursor move value for moving the cursor
- Line 132-133: We define the cursor move value for moving left
- Line 134-135: We define the cursor move value for moving right
- Line 160-166: We define the set CGRAM start address command in order to address the character generator RAM, i.e. to define user defined characters
- Line 168-174: We define the set DDRAM start address command in order to address the display data RAM, i.e. set the cursor position
- Line 176-208: We update the constructor to initialize the added member variables, and initialize the row offsets
- Line 236-273: We update the 'Initialize()' method to switch off the backlight and the display, set the line mode and character size, clear the display, 
and set the display to move the cursor right on write
- Line 295-313: We update the method `Write()` to write a character by updating the cursor position accordingly
- Line 315-325: We implement the method `Write()` to write a character at the specified cursor location
- Line 349-359: We implement the method `Write()` to write a string at the specified cursor location
- Line 361-369: We implement the method `ClearDisplay()` to clear the display and position the cursor home
- Line 371-380: We implement the method `Home()` to position the cursor home
- Line 382-399: We implement the method `ShiftDisplay()` to shift the display contents horizontally
- Line 401-409: We implement the method `SetDisplayEnabled()` to switch the display on or off
- Line 411-419: We implement the method `SetCursorMode()` to set the cursor mode
- Line 421-431: We implement the method `GetCursorPosition()` to retrieve the current cursor position
- Line 433-443: We implement the method `SetCursorPosition()` to set the cursor position
- Line 445-456: We implement the method `DisplayControl()` to set both display mode and cursor mode simultaneously
- Line 458-480: We implement the method `DefineCharFont()` to define a user defined character.
User defined characters are in the range 0x08-0x0F (even though the documentation says that also 0x00-0x07 work, only character codes 0x08-0x0F are correctly displayed
- Line 529-537: We implement the method `CursorOrDisplayShift()` to shift either the display contents or move the cursor.
This is used by `ShiftDisplay()`
- Line 539-545: We implement the method `UpdateDisplayControl()` to update the display control register with the current display mode and cursor mode.
This is used by `SetDisplayEnabled()` and `SetCursorMode()`
- Line 547-560: We implement the method `ConvertCursorMode()` to convert the currently set cursor mode to a display control register value
- Line 562-575: We implement the method `SetCGRAM_Address()` to set the character generator RAM address
- Line 577-590: We implement the method `SetDDRAM_Address()` to set the display data RAM address

### Application code {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_APPLICATION_CODE}

Let's now write some text, with user defined characters, and try some settings.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/I2CMaster.h"
3: #include "baremetal/Logger.h"
4: #include "baremetal/System.h"
5: #include "baremetal/Timer.h"
6: #include "device/i2c/HD44780DisplayI2C.h"
7: 
8: LOG_MODULE("main");
9: 
10: using namespace baremetal;
11: using namespace device;
12: 
13: int main()
14: {
15:     auto& console = GetConsole();
16: 
17:     uint8 busIndex = 1;
18:     uint8 address{0x27};
19:     I2CMaster i2cMaster;
20:     i2cMaster.Initialize(busIndex);
21:     HD44780DisplayI2C display(i2cMaster, address, 16, 2);
22:     display.Initialize();
23:     display.SetBacklight(true);
24:     display.SetDisplayEnabled(true);
25: 
26:     uint8 char08[8] = 
27:     {
28:         0b00000,
29:         0b00100,
30:         0b01010,
31:         0b10001,
32:         0b10001, 
33:         0b01010,
34:         0b00100,
35:         0b00000
36:     };
37:     uint8 char09[8] = 
38:     {
39:         0b00000, 
40:         0b01010, 
41:         0b11111, 
42:         0b11111, 
43:         0b11111, 
44:         0b01110, 
45:         0b00100, 
46:         0b00000
47:     };
48:     display.DefineCharFont(0x08, char08);
49:     display.DefineCharFont(0x09, char09);
50:     display.Write(0, 0, "Hello");
51:     display.Write(1, 1, "World\x08\x09");
52: 
53:     for (int i = 0; i < 3; ++i)
54:     {
55:         display.SetDisplayEnabled(false);
56:         Timer::WaitMilliSeconds(500);
57:         display.SetDisplayEnabled(true);
58:         Timer::WaitMilliSeconds(500);
59:     }
60: 
61:     display.SetCursorMode(ITextDisplay::CursorMode::BlinkLine);
62: 
63:     Timer::WaitMilliSeconds(5000);
64: 
65:     display.SetBacklight(false);
66: 
67:     Timer::WaitMilliSeconds(2000);
68: 
69:     LOG_INFO("Wait 5 seconds");
70:     Timer::WaitMilliSeconds(5000);
71: 
72:     display.SetDisplayEnabled(false);
73: 
74:     console.Write("Press r to reboot, h to halt\n");
75:     char ch{};
76:     while ((ch != 'r') && (ch != 'h'))
77:     {
78:         ch = console.ReadChar();
79:         console.WriteChar(ch);
80:     }
81: 
82:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
83: }
```

- Line 17: We have the display connected to the I2C bus 1 as before
- Line 18: The PCF8574 I2C is 0x27 as before
- Line 19-20: We instantiate a `I2CMaster` and initialize the bus index as before
- Line 21: We instantiate a `HD44780DisplayI2C`, inject the `I2CMaster` instance, set the I2C address, and set the display size
- Line 22: We initialize the display
- Line 23: We switch the backlight on
- Line 24: We switch the display on
- Line 26-36: We define a user defined character at code 0x08 (a kind of 'o' symbol)
- Line 37-47: We define a user defined character at code 0x09 (a heart symbol)
- Line 48: We define the user defined character 0x08 in the display
- Line 49: We define the user defined character 0x09 in the display
- Line 50: We write "Hello" at position (0,0)
- Line 51: We write "World" followed by the two user defined characters at position (1,1)
- Line 53-59: We blink the display 3 times by switching it off and on every 500 milliseconds
- Line 61-63: We set the cursor mode to blinking line and wait 5 seconds
- Line 65-67: We switch the backlight off and wait 2 seconds
- Line 69-72: We wait another 5 seconds and switch the display off

### Configuring, building and debugging {#TUTORIAL_26_I2C_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY___STEP_2___INITIALIZATION_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
You should see the display backlight turn on and then off again after 5 seconds.
As the backlight is on after power on, you will need to run the application twice to see it turn on and off.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.090 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.110 Set up bus 1, config 0, base address 3F804000 (I2CMaster:190)
Info   0.00:00:10.370 Wait 5 seconds (main:69)
Press r to reboot, h to halt
rInfo   0.00:00:42.610 Reboot (System:144)
```

Again, the console does not show much, but the display will show the text "Hello World", blink, show the cursor blinking with the backlight on, and then switch off the backlight, and later the display.

<img src="images/HD44780-display-text-on.png" alt="16 x 2 character display text with user characters on" width="400"/>

The text will still be visible, but hard to read without backlight.

<img src="images/HD44780-display-text-no-backlight.png" alt="16 x 2 character display text with user characters no backlight" width="400"/>

The text will completely disappear when off.

<img src="images/HD44780-display-text-off.png" alt="16 x 2 character display text with user characters off" width="400"/>

## Display in a real world example

### Update application code

We'll make the example slightly more entertaining.

We will show a choice of either `Reboot` or `Halt`, when the KY-040 rotary switch (see [The KY-040 rotary encoder](#TUTORIAL_21_GPIO_USING_HARDWARE_THE_KY_040_ROTARY_ENCODER)) is turned the option will toggle between these two.
When the switch is pressed, we will perform the selected action, e.g. either halt or reboot the system.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Format.h>
3: #include <baremetal/I2CMaster.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/System.h>
6: #include <device/gpio/KY-040.h>
7: #include <device/i2c/HD44780DisplayI2C.h>
8: 
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: using namespace device;
13: 
14: static int option{};
15: static bool select{};
16: static HD44780Display* lcdDevice{};
17: 
18: static void ShowOption();
19: 
20: static void OnEvent(KY040::Event event, void *param)
21: {
22:     LOG_INFO("Event %s", KY040::EventToString(event));
23:     switch (event)
24:     {
25:         case KY040::Event::SwitchDown:
26:             LOG_INFO("Value selected");
27:             select = true;
28:             break;
29:         case KY040::Event::RotateClockwise:
30:             option++;
31:             ShowOption();
32:             break;
33:         case KY040::Event::RotateCounterclockwise:
34:             option--;
35:             ShowOption();
36:             break;
37:         default:
38:             break;
39:     }
40: }
41: 
42: static bool ShouldReboot()
43: {
44:     return (option % 2 == 0);
45: }
46: 
47: static void ShowOption()
48: {
49:     lcdDevice->Write(0, 1, "      ");
50:     if (ShouldReboot())
51:     {
52:         LOG_INFO("Select Reboot");
53:         lcdDevice->Write(0, 1, "Reboot");
54:     }
55:     else
56:     {
57:         LOG_INFO("Select Halt");
58:         lcdDevice->Write(0, 1, "Halt");
59:     }
60: }
61: 
62: int main()
63: {
64:     auto& console = GetConsole();
65:     GetLogger().SetLogLevel(LogSeverity::Info);
66: 
67:     KY040 rotarySwitch(11, 9, 10);
68:     rotarySwitch.Initialize();
69:     rotarySwitch.RegisterEventHandler(OnEvent, nullptr);
70: 
71:     uint8 busIndex = 1;
72:     uint8 address{ 0x27 };
73:     const uint8 NumRows    = 2;
74:     const uint8 NumColumns = 16;
75:     I2CMaster i2cMaster;
76:     i2cMaster.Initialize(busIndex);
77: 
78:     HD44780DisplayI2C device(i2cMaster, address, NumColumns, NumRows);
79:     device.Initialize();
80:     lcdDevice = &device;
81: 
82:     console.Write("Select from menu by turning rotary switch and pressing to select option\n");
83: 
84:     device.SetBacklight(true);
85:     device.SetDisplayEnabled(true);
86:     device.SetCursorMode(HD44780Display::CursorMode::Hide);
87: 
88:     device.SetCursorPosition(0, 0);
89:     device.Write("Menu", 4);
90:     ShowOption();
91: 
92:     while (!select)
93:     {
94:         WaitForInterrupt();
95:     }
96: 
97:     device.SetBacklight(false);
98:     device.SetDisplayEnabled(false);
99: 
100:     if (ShouldReboot())
101:         LOG_INFO("Rebooting");
102:     else
103:         LOG_INFO("Halting");
104: 
105:     return static_cast<int>(ShouldReboot() ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
106: }
```

- Line 20-40: We handle the callbacks form the KY-040 switch.
When turned we increment or decrement the option, and call `ShowOption()` to update the display.
When pressed, we set the variable `select` to true
- Line 42-45: We create a utility function to see whether the `option` variable is even, meaning we should reboot, or odd, meaning we should halt
- Line 47-60: We update the display with the selected option value.
We only update the second line of the display
- Line 67-69: We instantiate and initialize the KY-040 rotary switch, and register the event handler
- Line 71-76: We instantiate a I2C bus master like before
- Line 78-80: We instantiate and initialize the HD44780 display like before, and store a pointer to it in the global variable `lcdDevice` so that the event handler can access it
- Line 88-90: We write the text "Menu" on the first line, and then show the current option on the second line
- Line 92-95: While no interrupts occur we wait, looping until the `select` variable is set to true
- Line 97-98: We switch off the backlight and the display
- Line 100-105: We log whether we are rebooting or halting, and return the appropriate return code

### Configuring, building and debugging {#TUTORIAL_26_I2C_DISPLAY_I2C_OPERATION_WITH_REAL_DEVICE_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

```text
Setting up UART0
Info   0.00:00:00.020 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.050 Starting up (System:213)
Info   0.00:00:00.070 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.090 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.120 Set up bus 1, config 0, base address 3F804000 (I2CMaster:190)
Select from menu by turning rotary switch and pressing to select option
Info   0.00:00:00.250 Select Reboot (main:52)
Info   0.00:00:04.540 Event RotateClockwise (main:22)
Info   0.00:00:04.540 Select Halt (main:57)
Info   0.00:00:04.770 Event RotateCounterclockwise (main:22)
Info   0.00:00:04.770 Select Reboot (main:52)
Info   0.00:00:06.430 Event RotateCounterclockwise (main:22)
Info   0.00:00:06.430 Select Halt (main:57)
Info   0.00:00:07.830 Event RotateCounterclockwise (main:22)
Info   0.00:00:07.830 Select Reboot (main:52)
Info   0.00:00:13.150 Event SwitchDown (main:22)
Info   0.00:00:13.150 Value selected (main:26)
Info   0.00:00:13.200 Rebooting (main:101)
Info   0.00:00:13.220 Reboot (System:144)
```

Next: [27-dma](27-dma.md)
