# Tutorial 22: LCD display {#TUTORIAL_22_LCD_DISPLAY}

@tableofcontents

## New tutorial setup {#TUTORIAL_22_LCD_DISPLAY_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/22-i2c-lcd`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_22_LCD_DISPLAY_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-22.a`
- a library `output/Debug/lib/device-22.a`
- a library `output/Debug/lib/stdlib-22.a`
- an application `output/Debug/bin/22-i2c-lcd.elf`
- an image in `deploy/Debug/22-i2c-lcd-image`

## Controlling a I2C LCD display {#TUTORIAL_22_LCD_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY}

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

### Initialization sequence {#TUTORIAL_22_LCD_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_INITIALIZATION_SEQUENCE}

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

### Mode setting {#TUTORIAL_22_LCD_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_MODE_SETTING}
    
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
Bit 4 of the I/O expander controls the BL signal, which switches the backlight on and off (1 = on, 0 = off).

So let's start simple. We'll first abstract the I2CMaster class to an interface, which we'll then implement for a fake.
Then well start with controlling the backlight, after which we will implement sending data to the display.
We'll start with the initialization sequences, as this is required to do anything useful with the display.

## Fake I2C implementation {#TUTORIAL_22_LCD_DISPLAY_FAKE_I2C_IMPLEMENTATION}

### Abstracting I2CMaster - II2CMaster.h {#TUTORIAL_22_LCD_DISPLAY_FAKE_I2C_IMPLEMENTATION_ABSTRACTING_I2CMASTER___II2CMASTERH}

First, we'll abstract the I2CMaster class to an interface.

Create the file `code/libraries/baremetal/include/baremetal/II2CMaster.h`

```cpp
File: code/libraries/baremetal/include/baremetal/II2CMaster.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : II2CMaster.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : I2CMaster
9: //
10: // Description : I2C Master abstract interface
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
43: 
44: /// @file
45: /// I2C Master abstract interface
46: 
47: namespace baremetal
48: {
49: 
50: #if BAREMETAL_RPI_TARGET <= 4
51: 
52: // Return codes returned by Read/Write as negative value
53: /// @brief Invalid parameter
54: #define I2C_MASTER_INVALID_PARM  1
55: /// @brief Received a NACK
56: #define I2C_MASTER_ERROR_NACK   2
57: /// @brief Received clock stretch timeout
58: #define I2C_MASTER_ERROR_CLKT   3
59: /// @brief Not all data has been sent/received
60: #define I2C_MASTER_DATA_LEFT    4
61: /// @brief Transfer timed out
62: #define I2C_MASTER_TIMEOUT      5
63: /// @brief Bus did not become ready
64: #define I2C_MASTER_BUS_NOT_BUSY 6
65: 
66: /// <summary>
67: /// I2C speed selection
68: /// </summary>
69: enum class I2CClockMode
70: {
71:     /// @brief I2C @ 100 KHz
72:     Normal,
73:     /// @brief I2C @ 400 KHz
74:     Fast,
75:     /// @brief I2C @ 1 MHz
76:     FastPlus,
77: };
78: 
79: /// <summary>
80: /// I2CMaster abstract interface. Can be inherited for creating a mock
81: /// </summary>
82: class II2CMaster
83: {
84: public:
85:     /// <summary>
86:     /// Default destructor needed for abstract class
87:     /// </summary>
88:     virtual ~II2CMaster() = default;
89: 
90:     /// <summary>
91:     /// Read a single byte
92:     /// </summary>
93:     /// <param name="address">I2C address</param>
94:     /// <param name="data">Data read</param>
95:     /// <returns>Number of bytes actually read. Should be 1 for successful read, 0 if failed, negative if an error occurs</returns>
96:     virtual size_t Read(uint16 address, uint8 &data) = 0;
97:     /// <summary>
98:     /// Read multiple bytes into buffer
99:     /// </summary>
100:     /// <param name="address">I2C address</param>
101:     /// <param name="buffer">Pointer to buffer to store data received</param>
102:     /// <param name="count">Requested byte count for read</param>
103:     /// <returns>Number of bytes actually read, or negative if an error occurs</returns>
104:     virtual size_t Read(uint16 address, void *buffer, size_t count) = 0;
105:     /// <summary>
106:     /// Write a single byte
107:     /// </summary>
108:     /// <param name="address">I2C address</param>
109:     /// <param name="data">Data byte to write</param>
110:     /// <returns>Number of bytes actually written. Should be 1 for successful write, 0 if failed, negative if an error occurs</returns>
111:     virtual size_t Write(uint16 address, uint8 data) = 0;
112:     /// <summary>
113:     /// Write multiple bytes to device
114:     /// </summary>
115:     /// <param name="address">I2C address</param>
116:     /// <param name="buffer">Pointer to buffer containing data to be sent</param>
117:     /// <param name="count">Requested byte count for write</param>
118:     /// <returns>Number of bytes actually written, or negative if an error occurs</returns>
119:     virtual size_t Write(uint16 address, const void *buffer, size_t count) = 0;
120:     /// <summary>
121:     /// Write then read from device
122:     /// </summary>
123:     /// <param name="address">I2C address</param>
124:     /// <param name="writeBuffer">Pointer to buffer containing data to be sent</param>
125:     /// <param name="writeCount">Requested byte count for write</param>
126:     /// <param name="readBuffer">Pointer to buffer to store data received</param>
127:     /// <param name="readCount">Requested byte count for read</param>
128:     /// <returns>Number of bytes actually written and read (cumulated), or negative if an error occurs</returns>
129:     virtual size_t WriteReadRepeatedStart(uint16 address, const void *writeBuffer, size_t writeCount, void *readBuffer, size_t readCount) = 0;
130: };
131: 
132: #else
133: 
134: #error RPI 5 not supported yet
135: 
136: #endif
137: 
138: } // namespace baremetal
```

- Line 50-77: This is directly copied from `I2CMaster.h`
- Line 79-130: We declare the abstract class `II2CMaster`
  - Line 85-88: As needed by any abstract class, we need a virtual destructor, which is the default in this case
  - Line 90-96: We declare the virtual `Read()` method to read a single byte
  - Line 97-104: We declare the virtual `Read()` method to read multiple bytes
  - Line 105-111: We declare the virtual `Write()` method to write a single byte
  - Line 112-119: We declare the virtual `Write()` method to write multiple bytes
  - Line 120-129: We declare the virtual `WriteReadRepeatedStart()` method to perform a write - read cycle

### Update I2CMaster - I2CMaster.h {#TUTORIAL_22_LCD_DISPLAY_FAKE_I2C_IMPLEMENTATION_UPDATE_I2CMASTER___I2CMASTERH}

We'll derive I2CMaster from the newly created interface.

Update the file `code/libraries/baremetal/include/baremetal/I2CMaster.h`

```cpp
File: code/libraries/baremetal/include/baremetal/I2CMaster.h
50: namespace baremetal {
51: 
52: /// <summary>
53: /// Driver for I2C master devices
54: ///
55: /// GPIO pin mapping (Raspberry Pi 3-4)
56: /// bus       | config 0      | config 1      | config 2      | Boards
57: /// :-------: | :-----------: | :-----------: | :-----------: | :-----
58: /// ^         | SDA    SCL    | SDA    SCL    | SDA    SCL    | ^
59: /// 0         | GPIO0  GPIO1  | GPIO28 GPIO29 | GPIO44 GPIO45 | Raspberry Pi 3 / 4
60: /// 1         | GPIO2  GPIO3  |               |               | Raspberry Pi 3 only
61: /// 2         |               |               |               | None
62: /// 3         | GPIO2  GPIO3  | GPIO4  GPIO5  |               | Raspberry Pi 4 only
63: /// 4         | GPIO6  GPIO7  | GPIO8  GPIO9  |               | Raspberry Pi 4 only
64: /// 5         | GPIO10 GPIO11 | GPIO12 GPIO13 |               | Raspberry Pi 4 only
65: /// 6         | GPIO22 GPIO23 |               |               | Raspberry Pi 4 only
66: ///
67: /// GPIO pin mapping (Raspberry Pi 5)
68: /// bus       | config 0      | config 1      | config 2      | Boards
69: /// :-------: | :-----------: | :-----------: | :-----------: | :-----
70: /// ^         | SDA    SCL    | SDA    SCL    | SDA    SCL    | ^
71: /// 0         | GPIO0  GPIO1  | GPIO8  GPIO9  |               | Raspberry Pi 5 only
72: /// 1         | GPIO2  GPIO3  | GPIO10 GPIO11 |               | Raspberry Pi 5 only
73: /// 2         | GPIO4  GPIO5  | GPIO12 GPIO13 |               | Raspberry Pi 5 only
74: /// 3         | GPIO6  GPIO7  | GPIO14 GPIO15 | GPIO22 GPIO23 | Raspberry Pi 5 only
75: /// </summary>
76: class I2CMaster
77:     : public II2CMaster
78: {
79: private:
80:     /// @brief Memory access interface reference for accessing registers.
81:     IMemoryAccess&  m_memoryAccess;
82:     /// @brief I2C bus index
83:     uint8           m_bus;
84:     /// @brief I2C bus base register address
85:     regaddr         m_baseAddress;
86:     /// @brief I2C bus clock rate
87:     I2CClockMode    m_clockMode;
88:     /// @brief I2C bus GPIO configuration index used
89:     uint32          m_config;
90:     /// @brief True if class is already initialized
91:     bool            m_isInitialized;
92: 
93:     /// @brief GPIO pin for SDA wire
94:     PhysicalGPIOPin m_sdaPin;
95:     /// @brief GPIO pin for SCL wire
96:     PhysicalGPIOPin m_sclPin;
97: 
98:     /// @brief Core clock rate used to determine I2C clock rate in Hz
99:     unsigned        m_coreClockRate;
100:     /// @brief I2C clock rate in Hz
101:     unsigned        m_clockSpeed;
102: 
103: public:
104:     I2CMaster(IMemoryAccess &memoryAccess = GetMemoryAccess());
105: 
106:     virtual ~I2CMaster();
107: 
108:     bool Initialize(uint8 bus, I2CClockMode mode = I2CClockMode::Normal, uint32 config = 0);
109: 
110:     void SetClock(unsigned clockRate);
111:     bool Scan(uint16 address);
112:     size_t Read(uint16 address, uint8 &data) override;
113:     size_t Read(uint16 address, void *buffer, size_t count) override;
114:     size_t Write(uint16 address, uint8 data) override;
115:     size_t Write(uint16 address, const void *buffer, size_t count) override;
116:     size_t WriteReadRepeatedStart(uint16 address, const void *writeBuffer, size_t writeCount, void *readBuffer, size_t readCount) override;
117: 
118: private:
119:     uint32 ReadControlRegister();
120:     void WriteControlRegister(uint32 data);
121:     void StartReadTransfer();
122:     void StartWriteTransfer();
123:     void ClearFIFO();
124:     void WriteAddressRegister(uint8 data);
125:     void WriteDataLengthRegister(uint8 data);
126:     uint32 ReadStatusRegister();
127:     void WriteStatusRegister(uint32 data);
128:     bool HasClockStretchTimeout();
129:     bool HasAck();
130:     bool HasNAck();
131:     bool ReceiveFIFOFull();
132:     bool ReceiveFIFOHasData();
133:     bool ReceiveFIFONeedsReading();
134:     bool TransmitFIFOEmpty();
135:     bool TransmitFIFOHasSpace();
136:     bool TransmitFIFONeedsWriting();
137:     bool TransferDone();
138:     bool TransferActive();
139:     void ClearClockStretchTimeout();
140:     void ClearNAck();
141:     void ClearDone();
142:     void ClearAllStatus();
143:     uint8 ReadFIFORegister();
144:     void WriteFIFORegister(uint8 data);
145: };
146: 
147: } // namespace baremetal
```

- Line 42: We include the header for `II2CMaster`
- Line 52-79: We remove these as there are now part of `II2CMaster.h`
- Line 76-77: We now derive from `II2CMaster`
- Line 112: We now override the `Read()` method for a single byte
- Line 113: We now override the `Read()` method for multiple bytes
- Line 114: We now override the `Write()` method for a single byte
- Line 115: We now override the `Write()` method for multiple bytes
- Line 116: We now override the `WriteReadRepeatedStart()` method for a write - read cycle

## LCD display interface and basic functionality {#TUTORIAL_22_LCD_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY}

### ILCDDevice.h {#TUTORIAL_22_LCD_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY_ILCDDEVICEH}

We'll first declare a generic interface for a LCD controller.

Create the file `code/libraries/device/include/device/display/ILCDDevice.h`

```cpp
File: code/libraries/device/include/device/display/ILCDDevice.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : ILCDDevice.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : ILCDDevice
9: //
10: // Description : Generic LCD device interface, used by CharLCDDevice
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
43: 
44: namespace device
45: {
46: 
47: /// <summary>
48: /// Generic character matrix LCD device interface
49: /// </summary>
50: class ILCDDevice
51: {
52:   public:
53:     /// <summary>
54:     /// Selects character size
55:     /// </summary>
56:     enum class CharacterSize : uint8
57:     {
58:         /// @brief Character size 5 x 10 pixels
59:         Size5x10,
60:         /// @brief Character size 5 x 8 pixels
61:         Size5x8,
62:     };
63:     /// <summary>
64:     /// Selects cursor movement direction on write
65:     /// </summary>
66:     enum class CursorMoveDirection : uint8
67:     {
68:         /// @brief Move right after writing character
69:         MoveRight,
70:         /// @brief Move left after writing character
71:         MoveLeft,
72:     };
73:     /// <summary>
74:     /// Display mode
75:     /// </summary>
76:     enum class DisplayMode : uint8
77:     {
78:         /// @brief Show characters
79:         Show,
80:         /// @brief Hide characters
81:         Hide,
82:     };
83:     /// <summary>
84:     /// Cursur mode
85:     /// </summary>
86:     enum class CursorMode : uint8
87:     {
88:         /// @brief Cursor is hidden
89:         Hide,
90:         /// @brief Cursor is line
91:         Line,
92:         /// @brief Cursor is blinking block
93:         Blink,
94:     };
95: 
96:   public:
97:     /// <summary>
98:     /// Destructor
99:     /// </summary>
100:     virtual ~ILCDDevice() = default;
101: 
102:     /// <summary>
103:     /// Retrieve the number of display columns
104:     /// </summary>
105:     /// <returns>Number of display columns</returns>
106:     virtual uint8 GetNumColumns() const = 0;
107:     /// <summary>
108:     /// Retrieve the number of display rows
109:     /// </summary>
110:     /// <returns>Number of display rows</returns>
111:     virtual uint8 GetNumRows() const = 0;
112: 
113:     /// <summary>
114:     /// Initialize the device
115:     /// </summary>
116:     virtual void Initialize() = 0;
117: 
118:     /// <summary>
119:     /// Set backlight on or off
120:     /// </summary>
121:     /// <param name="on">If true backlight is switched on, if false it is switched off</param>
122:     virtual void SetBacklight(bool on) = 0;
123:     /// <summary>
124:     /// Returns the current backlight status
125:     /// </summary>
126:     /// <returns>True if backlight is on, false otherwise</returns>
127:     virtual bool IsBacklightOn() const = 0;
128: 
129:     /// <summary>
130:     /// Write a character to the display, and update the cursor location
131:     ///
132:     /// The character is written in the buffer, depending on the shift it may or may not be visible
133:     /// </summary>
134:     /// <param name="value">Character to write</param>
135:     virtual void Write(char value) = 0;
136:     /// <summary>
137:     /// Write a character at the specified location
138:     ///
139:     /// The character is written in the buffer, depending on the shift it may or may not be visible
140:     /// </summary>
141:     /// <param name="posX">Horizontal position, 0..NumColumns-1</param>
142:     /// <param name="posY">Vertical position, 0..NumRows-1</param>
143:     /// <param name="ch"></param>
144:     virtual void Write(unsigned posX, unsigned posY, char ch) = 0;
145:     /// <summary>
146:     /// Write text to display, and update the cursor location
147:     ///
148:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
149:     /// </summary>
150:     /// <param name="text">Text to write</param>
151:     /// <param name="count">Number of characters to write</param>
152:     virtual void Write(const char *text, size_t count) = 0;
153:     /// <summary>
154:     /// Write text to display, and update the cursor location
155:     ///
156:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
157:     /// </summary>
158:     /// <param name="text">Text to write</param>
159:     virtual void Write(const char *text) = 0;
160:     /// <summary>
161:     /// Write text to display at the specified location
162:     ///
163:     /// The characters are written in the buffer, depending on the shift they may or may not be visible
164:     /// </summary>
165:     /// <param name="posX">Horizontal position, 0..NumColumns-1</param>
166:     /// <param name="posY">Vertical position, 0..NumRows-1</param>
167:     /// <param name="text">Text to write</param>
168:     virtual void Write(unsigned posX, unsigned posY, const char *text) = 0;
169: 
170:     /// <summary>
171:     /// Clear the display contents
172:     /// </summary>
173:     virtual void ClearDisplay() = 0;
174:     /// <summary>
175:     /// Move the cursor to the home position
176:     /// </summary>
177:     virtual void Home() = 0;
178:     /// <summary>
179:     /// Shift the display horizontally
180:     /// </summary>
181:     /// <param name="amount">Number of characters to shift. Negative to shift left, positive to shift right</param>
182:     virtual void ShiftDisplay(int amount) = 0;
183:     /// <summary>
184:     /// Set display on or off
185:     /// </summary>
186:     /// <param name="on">If true characters are shown, if false they are hidden</param>
187:     virtual void SetDisplayEnabled(bool on) = 0;
188:     /// <summary>
189:     /// Set cursor mode
190:     /// </summary>
191:     /// <param name="mode">Mode to set for cursor</param>
192:     virtual void SetCursorMode(CursorMode mode) = 0;
193:     /// <summary>
194:     /// Set location of the cursor
195:     /// </summary>
196:     /// <param name="cursorX">Horizontal position, left = 0</param>
197:     /// <param name="cursorY">Vertical position, top = 0</param>
198:     virtual void SetCursorPosition(unsigned cursorX, unsigned cursorY) = 0;
199:     /// <summary>
200:     /// Define a user character font
201:     ///
202:     /// Only 5 x 8 pixel characters can be defined
203:     /// </summary>
204:     /// <param name="ch">Character to define the font for. Only characters 0x80-0x87 are user defined</param>
205:     /// <param name="FontData">Font data (8 lines, one byte per line, only bits 4..0 are used)</param>
206:     virtual void DefineCharFont(char ch, const uint8 FontData[8]) = 0;
207: };
208: 
209: } // namespace device
```

- Line 50-192: We declare the abstract class `ILCDDevice`
  - Line 53-62: We declare the enum class `CharacterSize` to denote the character size on the display (5x8 or 5x10 pixels)
  - Line 63-72: We declare the enum class `CursorMoveDirection` to denote the cursor movement when writing (left or right)
  - Line 73-82: We declare the enum class `DisplayMode` to denote whether the display is on or off (show / hide)
  - Line 83-94: We declare the enum class `CursorMode` to denote the cursor shape (hide / line / blinking block)
  - Line 97-100: We declare a default virtual destructor as needed for a abstract class
  - Line 102-106: We declare a method `GetNumColumns()` to retrieve the number of columns on the display
  - Line 107-111: We declare a method `GetNumRows()` to retrieve the number of rows on the display
  - Line 113-116: We declare a method `Initialize()` to initialize the display controller
  - Line 118-122: We declare a method `SetBacklight()` to switch the backlight on or off
  - Line 123-127: We declare a method `IsBacklightOn()` to retrieve backlight on/of state
  - Line 129-135: We declare a method `Write()` to write a single character to the display at the current cursor position, and update the cursor position
  - Line 136-144: We declare a method `Write()` to write a single character to the display at the specified cursor position, and then update the cursor position
  - Line 145-152: We declare a method `Write()` to write a string of characters of specified length to the display at the current cursor position, and update the cursor position
  - Line 153-159: We declare a method `Write()` to write a string of characters to the display at the current cursor position, and update the cursor position
  - Line 160-168: We declare a method `Write()` to write a string of characters to the display at the specified cursor position, and then update the cursor position
  - Line 170-173: We declare a method `ClearDisplay()` to clear the display contents
  - Line 174-177: We declare a method `Home()` to move the cursor to the home position (upper left)
  - Line 178-182: We declare a method `ShiftDisplay()` to shift the visible characters to the left or right by the specified amount
  - Line 183-187: We declare a method `SetDisplayEnabled()` to hide or show the display contents
  - Line 188-192: We declare a method `SetCursorMode()` to set the cursor mode (hide, line, blink)
  - Line 193-198: We declare a method `SetCursorPosition()` to move the cursor to the specific location
  - Line 199-206: We declare a method `DefineCharFont()` to define one of 8 user defined characters (0x80-0x87)

### HD44780Device.h {#TUTORIAL_22_LCD_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY_HD44780DEVICEH}

We declare the LCD controller HD44780 based on the interface just defined.

Create the file `code/libraries/device/include/device/display/HD44780Device.h`

```cpp
File: code/libraries/device/include/device/display/HD44780Device.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780Device.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780Device
9: //
10: // Description : HD44780 based 16x2 LCD generic display
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
42: #include <device/display/ILCDDevice.h>
43: 
44: /// @file
45: /// HD44780 based LCD character display (max 40x4 characters)
46: 
47: namespace device {
48: 
49: /// <summary>
50: /// Hitachi HD44780 based LCD controller
51: /// </summary>
52: class HD44780Device : public ILCDDevice
53: {
54: public:
55:     /// <summary>
56:     /// Selection of the register to address in the LCD controller
57:     /// </summary>
58:     enum class RegisterSelect
59:     {
60:         /// @brief Command register
61:         Cmd,
62:         /// @brief Data register
63:         Data,
64:     };
65:     /// <summary>
66:     /// Selection of data transfer mode. The HD44780 controller can be either used in 4 bit mode (using D7-D4) and 8 bit mode (using D7-D0)
67:     /// </summary>
68:     enum class DataMode : uint8
69:     {
70:         /// @brief 4 bit transfer mode
71:         Mode4Bit,
72:         /// @brief 8 bit transfer mode
73:         Mode8Bit,
74:     };
75:     /// <summary>
76:     /// Mode on writing
77:     /// </summary>
78:     enum class ShiftMode : uint8
79:     {
80:         /// @brief Shift display on write
81:         DisplayShift,
82:         /// @brief Move cursor on write
83:         CursorMove,
84:     };
85: 
86: private:
87:     /// <summary>
88:     /// Display configuration
89:     /// </summary>
90:     enum class LCDLines : uint8
91:     {
92:         /// @brief Single row display
93:         LinesSingle,
94:         /// @brief Multi row display
95:         LinesMulti,
96:     };
97:     /// @brief Number of rows on the display
98:     uint8         m_numRows;
99:     /// @brief Number of columns on the display
100:     uint8         m_numColumns;
101:     /// @brief Data transfer mode (4 or 8 bit)
102:     DataMode      m_dataMode;
103:     /// @brief Character size (5x8 or 5x10)
104:     CharacterSize m_characterSize;
105:     /// @brief Whether display has single or multiple lines
106:     LCDLines      m_lineMode;
107:     /// @brief Cursor mode (none, line, block)
108:     CursorMode    m_cursorMode;
109:     /// @brief Display mode (internal combination of display and cursor mode)
110:     uint8         m_displayMode;
111:     /// @brief Cursor position column
112:     uint8         m_cursorX;
113:     /// @brief Cursor position row
114:     uint8         m_cursorY;
115: 
116:     /// <summary>
117:     /// Row offsets for multi line display (max 4 lines)
118:     /// </summary>
119:     /// <returns>Row offsets for multi line display</returns>
120:     uint8         m_rowOffsets[4] __attribute__((aligned(8)));
121: 
122: public:
123:     HD44780Device(uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8,
124:                   DataMode dataMode = DataMode::Mode4Bit);
125: 
126:     virtual ~HD44780Device();
127: 
128:     uint8 GetNumColumns() const override;
129:     uint8 GetNumRows() const override;
130: 
131:     void Initialize() override;
132:     void SetBacklight(bool on) override;
133:     bool IsBacklightOn() const override;
134:     bool IsDataMode4Bits() const;
135: 
136:     void  Write(char value) override;
137:     void  Write(unsigned posX, unsigned posY, char ch) override;
138:     void  Write(const char *text, size_t count) override;
139:     void  Write(const char *text) override;
140:     void  Write(unsigned posX, unsigned posY, const char *text) override;
141: 
142:     void  ClearDisplay() override;
143:     void  Home() override;
144:     void  ShiftDisplay(int amount) override;
145:     void  SetDisplayEnabled(bool on) override;
146:     void  SetCursorMode(CursorMode mode) override;
147:     void  SetCursorPosition(unsigned cursorX, unsigned cursorY) override;
148: 
149:     void  DefineCharFont(char ch, const uint8 FontData[8]) override;
150: 
151:     void  DisplayControl(DisplayMode displayMode, CursorMode cursorMode);
152: 
153: protected:
154:     /// <summary>
155:     /// Write a half byte to the display
156:     ///
157:     /// This is used when using 4 bit mode. The upper 4 bits of the byte are used for data, the lower 4 bits contain the control bits (register select, read/write, enable, backlight) <br/>
158:     /// Bit 0: Instruction (0) or Data (1) <br/>
159:     /// Bit 1: Read (1) or write (0) <br/>
160:     /// Bit 2: Enable bit (will first be set on, then off) to latch data <br/>
161:     /// Bit 3: Backlight on (1) or off (0) <br/>
162:     /// Bit 4-7: 4 bits of the data to write (first the upper 4 bits, then the lower 4 bits are sent)
163:     /// </summary>
164:     /// <param name="data">Byte to be written (including control bits in lower 4 bits, data in upper 4 bits)</param>
165:     virtual void WriteHalfByte(uint8 data) = 0;
166: 
167: private:
168:     void         WriteInstruction(uint8 value);
169:     void         WriteData(uint8 value);
170:     void         WriteByte(uint8 data, RegisterSelect mode);
171:     void         UpdateDisplayControl();
172:     void         CursorOrDisplayShift(ShiftMode mode, uint8 direction);
173:     void         SetCGRAM_Address(uint8 address);
174:     void         SetDDRAM_Address(uint8 address);
175:     uint8        ConvertCursorMode() const;
176: };
177: 
178: } // namespace device
```

- Line 49-182: We declare the HD44780 display controller class
  - Line 55-64: We declare an enum class `RegisterSelect` to selected between the Command register and Data register.
The Command register is used to configure the display, the Data register is use to write text to the display
  - Line 65-74: We declare an enum class `DataMode` to selected between 4-bit and 8-bit configured displays.
We will only be using the 4-bit mode, but will also implement support for 8-bit mode
  - Line 75-84: We declare an enum class `ShiftMode` to selected between moving the cursor on writing, or shifting the display contents
  - Line 87-96: We declare a private enum class `LCDLines` to selected between single line or multi line displays
  - Line 97-98: We declare a member variable `m_numRows` to hold the number of rows on the display
  - Line 99-100: We declare a member variable` m_numColumns` to hold the number of columns on the display
  - Line 101-102: We declare a member variable `m_dataMode` to hold the data transfer mode (4 or 8 bits)
  - Line 103-104: We declare a member variable `m_characterSize` to hold the selected character size (5x8 or 5x10 pixels)
  - Line 105-106: We declare a member variable `m_lineMode` to hold the line configuration for the display (single or multiple lines)
  - Line 107-108: We declare a member variable `m_cursorMode` to hold the cursor mode (l)
  - Line 109-110: We declare a member variable `m_displayMode` to hold the line configuration for the display (single or multiple lines)
  - Line 111-112: We declare a member variable `m_cursorX` to hold the line configuration for the display (single or multiple lines)
  - Line 113-114: We declare a member variable `m_cursorY` to hold the line configuration for the display (single or multiple lines)
  - Line 116-120: We declare a member variable `m_rowOffsets` to hold row offsets for each row on the display
  - Line 123-124: We declare the constructor
  - Line 126: We declare the virtual destructor
  - Line 128: We declare the method `GetNumColumns()` as an override
  - Line 129: We declare the method `GetNumRows()` as an override
  - Line 131: We declare the method `Initialize()` as an override
  - Line 132: We declare the method `SetBacklight()` as an override
  - Line 133: We declare the method `IsBacklightOn()` as an override
  - Line 134: We declare a method `IsDataMode4Bits()` to to retrieve the data transfer configuration settings
  - Line 136: We declare the method `Write()` for a single character as an override
  - Line 137: We declare the method `Write()` for a single character at a specified location as an override
  - Line 138: We declare the method `Write()` for multiple characters as an override
  - Line 139: We declare the method `Write()` for a string as an override
  - Line 140: We declare the method `Write()` for a string at a specified locationas an override
  - Line 142: We declare the method `ClearDisplay()` as an override
  - Line 143: We declare the method `Home()` as an override
  - Line 144: We declare the method `ShiftDisplay()` as an override
  - Line 145: We declare the method `SetDisplayEnabled()` as an override
  - Line 146: We declare the method `SetCursorMode()` as an override
  - Line 147: We declare the method `SetCursorPosition()` as an override
  - Line 149: We declare the method `DefineCharFont()` as an override
  - Line 151: We declare a method `DisplayControl()` to specify the display and cursor settings
  - Line 154-165: We declare an abstract method `WriteHalfByte()` to send 4 bits in 4 bit transfer mode.
This will have a hardware dependent implementation whether connected through GPIO or I2C
  - Line 168: We declare a private method `WriteInstruction()` to write an instruction byte to the device.
This will call `WriteByte()`
  - Line 169: We declare a private method `WriteData()` to write an data byte to the device.
This will call `WriteByte()`
  - Line 170: We declare a private method `WriteByte()` to write a instruction byte or data byte to the device.
This will call `WriteHalfByte()` for 4 bit transfers.
The 8 bit version is not implemented
  - Line 171: We declare a private method `UpdateDisplayControl()` to write display and cursor settings to the device
  - Line 172: We declare a private method `CursorOrDisplayShift()` to write cursor move or shift settings to the device
  - Line 173: We declare a private method `SetCGRAM_Address()` to set the current Character Generator address.
This is needed for setting user defined characters
  - Line 174: We declare a private method `SetDDRAM_Address()` to set the current Display Data address.
This will determine the current location for a new character
  - Line 185: We declare a private method `ConvertCursorMode()` to convert the current cursor mode to the correct bit pattern

### HD44780Device.cpp {#TUTORIAL_22_LCD_DISPLAY_LCD_DISPLAY_INTERFACE_AND_BASIC_FUNCTIONALITY_HD44780DEVICECPP}

We will now implement the `HD44780Device` class.

Create the file `code/libraries/device/src/display/HD44780Device.cpp`

```cpp
File: code/libraries/device/src/display/HD44780Device.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780Device.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780Device
9: //
10: // Description : HD44780 based 16x2 LCD generic display
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
40: #include <device/display/HD44780Device.h>
41: 
42: #include <baremetal/Logger.h>
43: #include <baremetal/Timer.h>
44: #include <stdlib/Util.h>
45: 
46: using namespace baremetal;
47: 
48: /// @file
49: /// HD44780 based 16x2 LCD generic display
50: 
51: /// @brief Define log name
52: LOG_MODULE("HD44780Device");
53: 
54: namespace device {
55: 
56: /// @brief Bit pattern used for Command or Instruction. This refers to the value of the RS pin
57: static const uint8 MODE_CMD  = (0x00);
58: /// @brief Bit pattern used for Data. This refers to the value of the RS pin
59: static const uint8 MODE_DATA = (0x01);
60: 
61: /// @brief Clear Display command
62: ///
63: /// See [HD44780 (Dot Matrix Liquid Crystal Display Controller/Driver) documentation](pdf/HD44780.pdf), page 24/25 <br/>
64: /// Overwrite display with blank characters and reset cursor position. <br/>
65: /// Clear display <br/>
66: /// 0 0 0 0 0 0 0 1 <br/>
67: /// Clears entire display and sets DDRAM address 0 in address counter.
68: static const uint8 LCD_CLEARDISPLAY = (0x01);
69: /// @brief Home command
70: ///
71: /// Set cursor to initial position and reset any shifting. <br/>
72: /// Return home <br/>
73: /// 0 0 0 0 0 0 1 * <br/>
74: /// Sets DDRAM address 0 in address counter. Also returns display from being shifted to original position. DDRAM contents remain unchanged.
75: static const uint8 LCD_RETURNHOME = (0x02);
76: /// @brief Entry mode set
77: ///
78: /// Sets cursor move direction and specifies display shift. These operations are performed during data write and read. <br/>
79: /// Entry mode set <br/>
80: /// 0 0 0 0 0 1 I/D S <br/>
81: /// If S = 1, shift display right (I/D = 0) or left (I/D = 1) on write. <br/>
82: /// If S = 0, move cursor left (I/D = 0) or right (I/D = 1) on write.
83: static const uint8 LCD_ENTRYMODESET         = (0x04);
84: /// @brief Entry mode set, cursor move right / shift right on write
85: static const uint8 LCD_ENTRYMODE_MOVE_RIGHT = (0x02);
86: /// @brief Entry mode set, cursor move left / shift left on write
87: static const uint8 LCD_ENTRYMODE_MOVE_LEFT  = (0x00);
88: /// @brief Entry mode set, shift mode
89: static const uint8 LCD_ENTRYMODE_SHIFT      = (0x01);
90: /// @brief Entry mode set, cursor move mode
91: static const uint8 LCD_ENTRYMODE_NO_SHIFT   = (0x00);
92: /// @brief Display control
93: ///
94: /// Sets cursor move direction and specifies display shift. These operations are performed during data write and read. <br/>
95: /// Display on/off control <br/>
96: /// 0 0 0 0 1 D C B <br/>
97: /// If D = 1, display is on, if D = 0, display is off <br/>
98: /// If C = 1, cursor is on, if C = 0, cursor is off <br/>
99: /// If B = 1, cursor is blinking, if B = 0, cursor is not blinking
100: static const uint8 LCD_DISPLAYCONTROL                 = (0x08);
101: /// @brief Display control, display on
102: static const uint8 LCD_DISPLAYCONTROL_ON              = (0x04);
103: /// @brief Display control, display off
104: static const uint8 LCD_DISPLAYCONTROL_OFF             = (0x00);
105: /// @brief Display control, cursor on
106: static const uint8 LCD_DISPLAYCONTROL_CURSOR_ON       = (0x02);
107: /// @brief Display control, cursor off
108: static const uint8 LCD_DISPLAYCONTROL_CURSOR_OFF      = (0x00);
109: /// @brief Display control, cursor blinking
110: static const uint8 LCD_DISPLAYCONTROL_CURSOR_BLINK    = (0x01);
111: /// @brief Display control, cursor not blinking
112: static const uint8 LCD_DISPLAYCONTROL_CURSOR_NO_BLINK = (0x00);
113: /// @brief Display control, cursor move or display shift
114: ///
115: /// Cursor or display shift <br/>
116: /// 0 0 0 1 S/C R/L * * <br/>
117: /// Moves cursor and shifts display without changing DDRAM contents. <br/>
118: /// if S/C = 0, the cursor is moved to the left (R/L = 0) or right (R/L = 1) <br/>
119: /// if S/C = 1, the entire screen is moved to the left (R/L = 0) or right (R/L = 1)
120: static const uint8 LCD_CURSORSHIFT              = (0x10);
121: /// @brief Display shift
122: static const uint8 LCD_CURSORSHIFT_SCREEN       = (0x08);
123: /// @brief Display shift left
124: static const uint8 LCD_CURSORSHIFT_SCREEN_LEFT  = (0x08);
125: /// @brief Display shift right
126: static const uint8 LCD_CURSORSHIFT_SCREEN_RIGHT = (0x0C);
127: /// @brief Cursor move
128: static const uint8 LCD_CURSORSHIFT_CURSOR       = (0x00);
129: /// @brief Cursor move left
130: static const uint8 LCD_CURSORSHIFT_CURSOR_LEFT  = (0x00);
131: /// @brief Cursor move right
132: static const uint8 LCD_CURSORSHIFT_CURSOR_RIGHT = (0x04);
133: /// @brief Set display configuration
134: ///
135: /// Write configuration to display <br/>
136: /// Function set <br/>
137: /// 0 0 1 DL N F * * <br/>
138: /// Sets interface data length (DL), number of display lines (N), and character font (F). <br/>
139: /// Needs to be done immediately after the initialization sequence and cannot be changed. <br/>
140: /// If DL = 1, set 8 bit mode, if DL = 0 set 4 bit mode (this is used for I2C devices and devices with only 4 data lanes) <br/>
141: /// If N = 1, set 2 (or more) line mode, if N = 0 set 1 line mode (this enables if desired 5x10 characters) <br/>
142: /// If F = 1, set 5x10 character size, if F = 0 set 5x8 character size. If N=1 only 5x8 character size is supported
143: static const uint8 LCD_FUNCTIONSET          = (0x20);
144: /// @brief Set 8 bit transfer mode
145: static const uint8 LCD_FUNCTIONSET_8BITMODE = (0x10);
146: /// @brief Set 4 bit transfer mode
147: static const uint8 LCD_FUNCTIONSET_4BITMODE = (0x00);
148: /// @brief Set display configuration to multiple lines
149: static const uint8 LCD_FUNCTIONSET_2LINE    = (0x08);
150: /// @brief Set display configuration to single line
151: static const uint8 LCD_FUNCTIONSET_1LINE    = (0x00);
152: /// @brief Set display font to 5x10 pixels (only for single line display)
153: static const uint8 LCD_FUNCTIONSET_5x10DOTS = (0x04);
154: /// @brief Set display font to 5x8 pixels (default for multiple line display)
155: static const uint8 LCD_FUNCTIONSET_5x8DOTS  = (0x00);
156: /// @brief Set CGRAM start address
157: ///
158: /// Set CGRAM address <br/>
159: /// 0 1 ACG ACG ACG ACG ACG ACG <br/>
160: /// Sets CGRAM address. CGRAM data is sent and received after this setting. <br/>
161: /// Sets the initial address for data write, the address is incremented after each write.
162: static const uint8 LCD_SETCGRAMADDR = (0x40);
163: /// @brief Set DDRAM start address
164: ///
165: /// Set DDRAM address <br/>
166: /// 1 ADD ADD ADD ADD ADD ADD ADD <br/>
167: /// Sets DDRAM address. DDRAM data is sent and received after this setting. <br/>
168: /// Sets the initial address for data write, the address is incremented after each write.
169: static const uint8 LCD_SETDDRAMADDR = (0x80);
170: 
171: /// <summary>
172: /// Constructor
173: /// </summary>
174: /// <param name="numColumns">Display size in number of columns (max. 40)</param>
175: /// <param name="numRows">Display size in number of rows (max. 4). Note that the total number of characters is limited to 80 (so e.g. max 4x20)</param>
176: /// <param name="characterSize">Size of characters (5x8 or 5x10 pixels, must be 5x8 pixels for multiline displays)</param>
177: /// <param name="dataMode">Specifies 4 or 8 bit mode</param>
178: HD44780Device::HD44780Device(uint8 numColumns, uint8 numRows, CharacterSize characterSize /*= CharacterSize::Size5x8*/,
179:                              DataMode dataMode /*= DataMode::LCD_4BITMODE*/)
180:     : m_numRows{numRows}
181:     , m_numColumns{numColumns}
182:     , m_dataMode{dataMode}
183:     , m_characterSize{characterSize}
184:     , m_lineMode{}
185:     , m_cursorMode{}
186:     , m_displayMode{}
187:     , m_cursorX{}
188:     , m_cursorY{}
189:     , m_rowOffsets{}
190: {
191:     if (numRows == 1)
192:     {
193:         m_lineMode = LCDLines::LinesSingle;
194:     }
195:     else if ((numRows >= 2) && (numRows <= 4))
196:     {
197:         m_lineMode = LCDLines::LinesMulti;
198:         // Force to 5x8 characters, 5x10 is not supported for more than 1 line.
199:         m_characterSize = CharacterSize::Size5x8;
200:     }
201:     assert((m_characterSize == CharacterSize::Size5x8) || (m_characterSize == CharacterSize::Size5x10));
202:     assert((numRows >= 1) && (numRows <= 4));
203:     for (uint8 i = 0; i < numRows; ++i)
204:     {
205:         if (i % 2 == 1)
206:             m_rowOffsets[i] = (i * 0x40); // 0x40 is the default offset for the next row when row is odd
207:         if (i >= 2)
208:             m_rowOffsets[i] += m_numColumns; // For rows 2 and 3, add the number of columns to the offset
209:     }
210:     assert((m_characterSize == CharacterSize::Size5x8) || (numRows == 1));
211: }
212: 
213: /// <summary>
214: /// Destructor
215: /// </summary>
216: HD44780Device::~HD44780Device()
217: {
218:     // Don't write anymore, as derived class is already destroyed
219: }
220: 
221: /// <summary>
222: /// Return number of display columns
223: /// </summary>
224: /// <returns>Number of display columns</returns>
225: uint8 HD44780Device::GetNumColumns() const
226: {
227:     return m_numColumns;
228: }
229: 
230: /// <summary>
231: /// Return number of display rows
232: /// </summary>
233: /// <returns>Number of display rows</returns>
234: uint8 HD44780Device::GetNumRows() const
235: {
236:     return m_numRows;
237: }
238: 
239: /// <summary>
240: /// Initializat the display
241: /// </summary>
242: void HD44780Device::Initialize()
243: {
244:     // Setup initial display configuration
245:     uint8 displayFunction = (m_dataMode == DataMode::Mode8Bit) ? LCD_FUNCTIONSET_8BITMODE : LCD_FUNCTIONSET_4BITMODE;
246:     displayFunction |= (m_lineMode == LCDLines::LinesMulti) ? LCD_FUNCTIONSET_2LINE : LCD_FUNCTIONSET_1LINE;
247:     displayFunction |= (m_characterSize == CharacterSize::Size5x10) ? LCD_FUNCTIONSET_5x10DOTS : LCD_FUNCTIONSET_5x8DOTS;
248: 
249:     // Initialization sequence (see page 45 / 46 of Hitachi HD44780 display controller datasheet
250:     // Choose 4 or 8 bit mode
251:     if (IsDataMode4Bits())
252:     {
253:         WriteHalfByte(0x30);
254:         Timer::WaitMicroSeconds(4500);
255:         WriteHalfByte(0x30);
256:         Timer::WaitMicroSeconds(100);
257:         WriteHalfByte(0x30);
258:         // See page 46 Hitachi HD44780 display controller datasheet
259:         WriteHalfByte(0x20);
260:     }
261:     else
262:     {
263:         WriteInstruction(0x03);
264:         Timer::WaitMicroSeconds(4500);
265:         WriteInstruction(0x03);
266:         Timer::WaitMicroSeconds(4500);
267:         WriteInstruction(0x03);
268:     }
269: 
270:     SetBacklight(true);
271:     WriteInstruction(LCD_FUNCTIONSET | displayFunction);
272:     Timer::WaitMicroSeconds(50);
273:     WriteInstruction(LCD_DISPLAYCONTROL | LCD_DISPLAYCONTROL_OFF);
274:     WriteInstruction(LCD_CLEARDISPLAY);
275:     WriteInstruction(LCD_ENTRYMODESET | LCD_ENTRYMODE_MOVE_RIGHT); // set move cursor right, do not shift display
276: }
277: 
278: /// <summary>
279: /// Switch backlight on or off
280: /// </summary>
281: /// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
282: void HD44780Device::SetBacklight(bool on)
283: {
284:     // Default implementation does not support backlight
285:     (void)on;
286: }
287: 
288: /// <summary>
289: /// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
290: /// </summary>
291: /// <returns>True if backlight is on, false otherwise</returns>
292: bool HD44780Device::IsBacklightOn() const
293: {
294:     // Default implementation does not support backlight
295:     return false;
296: }
297: 
298: /// <summary>
299: /// Return true if we configured the display to use 4 bit mode
300: /// </summary>
301: /// <returns>True if using 4 bit mode, false otherwise</returns>
302: bool HD44780Device::IsDataMode4Bits() const
303: {
304:     return m_dataMode == DataMode::Mode4Bit;
305: }
306: 
307: /// <summary>
308: /// Write single character to display
309: /// </summary>
310: /// <param name="value"></param>
311: void HD44780Device::Write(char value)
312: {
313:     // Write a raw character byte to the LCD16X2.
314:     WriteData(static_cast<uint8>(value));
315:     if (m_cursorX < m_numColumns - 1)
316:     {
317:         m_cursorX += 1;
318:     }
319:     else
320:     {
321:         // At end of line : go to left side next row. Wrap around to first row if on last row.
322:         m_cursorY = (m_cursorY + 1) % m_numRows;
323:         m_cursorX = 0;
324:     }
325: }
326: 
327: /// <summary>
328: /// Write a character at a specific position on the display
329: /// </summary>
330: /// <param name="posX"></param>
331: /// <param name="posY"></param>
332: /// <param name="ch"></param>
333: void HD44780Device::Write(unsigned posX, unsigned posY, char ch)
334: {
335:     SetCursorPosition(posX, posY);
336:     Write(ch);
337: }
338: 
339: /// <summary>
340: /// Write multiple characters to display
341: /// </summary>
342: /// <param name="text">Point to string to write to display</param>
343: /// <param name="count">Count of characters to write to display</param>
344: void HD44780Device::Write(const char *text, size_t count)
345: {
346:     for (size_t i = 0; i < count; ++i)
347:     {
348:         Write(text[i]);
349:     }
350: }
351: 
352: /// <summary>
353: /// Write multiple characters to display
354: /// </summary>
355: /// <param name="text">Point to string to write to display</param>
356: void HD44780Device::Write(const char *text)
357: {
358:     Write(text, strlen(text));
359: }
360: 
361: /// <summary>
362: /// Write multiple characters to display
363: /// </summary>
364: /// <param name="posX"></param>
365: /// <param name="posY"></param>
366: /// <param name="text">Point to string to write to display</param>
367: void HD44780Device::Write(unsigned posX, unsigned posY, const char *text)
368: {
369:     Write(text, strlen(text));
370: }
371: 
372: /// <summary>
373: /// Clear the display contents, and position the cursor home
374: /// </summary>
375: void HD44780Device::ClearDisplay()
376: {
377:     WriteInstruction(LCD_CLEARDISPLAY);
378:     Timer::WaitMicroSeconds(2000);
379:     Home();
380: }
381: 
382: /// <summary>
383: /// Move the cursor to the home position
384: /// </summary>
385: void HD44780Device::Home()
386: {
387:     WriteInstruction(LCD_RETURNHOME);
388:     Timer::WaitMicroSeconds(2000);
389:     m_cursorX = 0;
390:     m_cursorY = 0;
391: }
392: 
393: /// <summary>
394: /// Shift display contents horizontally
395: /// </summary>
396: /// <param name="amount">Amound to shift by. If negative shifts left by -amount characters, if positive shifts right by amount characters. A value of 0 means no change.</param>
397: void HD44780Device::ShiftDisplay(int amount)
398: {
399:     if (amount == 0)
400:         return;
401: 
402:     auto direction = (amount > 0) ? LCD_CURSORSHIFT_CURSOR_RIGHT : LCD_CURSORSHIFT_CURSOR_LEFT;
403:     if (amount < 0)
404:         amount = -amount;
405:     for (int i = 0; i < amount; ++i)
406:     {
407:         CursorOrDisplayShift(ShiftMode::DisplayShift, direction);
408:         Timer::WaitMicroSeconds(50);
409:     }
410: }
411: 
412: /// <summary>
413: /// Set display on or off
414: /// </summary>
415: /// <param name="on">If true switch display to show mode, otherwise switch to hide mode</param>
416: void HD44780Device::SetDisplayEnabled(bool on)
417: {
418:     m_displayMode = (on) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;
419:     UpdateDisplayControl();
420: }
421: 
422: /// <summary>
423: /// Set cursor mode
424: /// </summary>
425: /// <param name="mode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
426: void HD44780Device::SetCursorMode(CursorMode mode)
427: {
428:     m_cursorMode = mode;
429:     UpdateDisplayControl();
430: }
431: 
432: /// <summary>
433: /// Change the cursor position
434: /// </summary>
435: /// <param name="cursorX">New cursor column</param>
436: /// <param name="cursorY">New cursor row</param>
437: void HD44780Device::SetCursorPosition(unsigned cursorX, unsigned cursorY)
438: {
439:     SetDDRAM_Address(m_rowOffsets[cursorY] + cursorX);
440:     m_cursorX = cursorX;
441:     m_cursorY = cursorY;
442: }
443: 
444: /// <summary>
445: /// Define a user defined character
446: ///
447: /// Only characters 0x80-0x87 are user defined. The font data is 8 lines, each line is one byte. Only 5x8 pixel
448: /// characters are supported for now.
449: /// </summary>
450: /// <param name="ch">Character to define</param>
451: /// <param name="FontData">Font data. For every line there is one byte, only the low order 5 bits are used</param>
452: void HD44780Device::DefineCharFont(char ch, const uint8 FontData[8])
453: {
454:     uint8 data = static_cast<uint8>(ch);
455:     if (data < 0x80 || data > 0x87)
456:     {
457:         return;
458:     }
459:     data -= 0x80;
460: 
461:     SetCGRAM_Address(data << 3);
462: 
463:     for (unsigned line = 0; line <= 7; line++)
464:     {
465:         WriteData(FontData[line] & 0x1F);
466:     }
467: }
468: 
469: /// <summary>
470: /// Set display mode and cursor mode
471: /// </summary>
472: /// <param name="displayMode">Display on (DisplayMode::Show) or off (DisplayMode::Hide)</param>
473: /// <param name="cursorMode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
474: void HD44780Device::DisplayControl(DisplayMode displayMode, CursorMode cursorMode)
475: {
476:     m_cursorMode  = cursorMode;
477:     m_displayMode = (displayMode == DisplayMode::Show) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;
478: 
479:     UpdateDisplayControl();
480: }
481: 
482: /// <summary>
483: /// Write instruction to HD44780 display controller
484: /// </summary>
485: /// <param name="value">Instruction byte (see LCD_x)</param>
486: void HD44780Device::WriteInstruction(uint8 value)
487: {
488:     WriteByte(value, RegisterSelect::Cmd);
489: }
490: 
491: /// <summary>
492: /// Write data to HD44780 display controller
493: /// </summary>
494: /// <param name="value">Data byte, character to be written</param>
495: void HD44780Device::WriteData(uint8 value)
496: {
497:     WriteByte(value, RegisterSelect::Data);
498: }
499: 
500: /// <summary>
501: /// Write a byte to the display controller
502: /// </summary>
503: /// <param name="data">Data byte to send</param>
504: /// <param name="mode">If equal to RegisterSelect::Cmd, this is an instruction, if equal to RegisterSelect::Data, this is data</param>
505: void HD44780Device::WriteByte(uint8 data, RegisterSelect mode)
506: {
507:     uint8 modeBits = (mode == RegisterSelect::Data) ? MODE_DATA : MODE_CMD;
508:     //LOG_INFO("Write %s byte %02x", (mode == RegisterSelect::Cmd ? "instruction" : "data"), data);
509:     if (m_dataMode == DataMode::Mode4Bit)
510:     {
511:         WriteHalfByte(modeBits | (data & 0xF0));
512:         WriteHalfByte(modeBits | ((data << 4) & 0xF0));
513:     }
514:     else
515:     {
516:         assert(false);
517:     }
518: }
519: 
520: /// <summary>
521: /// Update the display control register with the current display mode and cursor mode
522: /// </summary>
523: void HD44780Device::UpdateDisplayControl()
524: {
525:     WriteInstruction(LCD_DISPLAYCONTROL | m_displayMode | ConvertCursorMode());
526: }
527: 
528: /// <summary>
529: /// Shift cursor or display contents horizontally
530: /// </summary>
531: /// <param name="mode">If equal to ShiftMode::DisplayShift, the display contents are shifted, if equal to ShiftMode::CursorMove move the cursor to a new location</param>
532: /// <param name="direction">Direction to move. If equal to LCD_CURSORSHIFT_CURSOR_RIGHT moves to the right, otherwise moves to the left.</param>
533: void HD44780Device::CursorOrDisplayShift(ShiftMode mode, uint8 direction)
534: {
535:     WriteInstruction(LCD_CURSORSHIFT | ((mode == ShiftMode::DisplayShift) ? LCD_CURSORSHIFT_SCREEN : LCD_CURSORSHIFT_CURSOR) | direction);
536: }
537: 
538: /// <summary>
539: /// Set dislay Character Generator RAM (CGRAM) address
540: ///
541: /// Data written subsequently is written to CGRAM starting at the specified address, incrementing by 1 for each byte
542: /// written.
543: /// </summary>
544: /// <param name="address">New CGRAM address (6 bits)</param>
545: void HD44780Device::SetCGRAM_Address(uint8 address)
546: {
547:     // Set CGRAM address
548:     // 0 1 ACG ACG ACG ACG ACG ACG
549:     // Sets CGRAM address.CGRAM data is sent and received after this setting.
550:     WriteInstruction(LCD_SETCGRAMADDR | (address & 0x3F));
551: }
552: 
553: /// <summary>
554: /// Set Display Data RAM (DDRAM) address
555: ///
556: /// Data written subsequently is written to DDRAM (visible display) starting at the specified address, incrementing by 1 for each byte
557: /// written.
558: /// </summary>
559: /// <param name="address">New display RAM address (7 bits)</param>
560: void HD44780Device::SetDDRAM_Address(uint8 address)
561: {
562:     // Set DDRAM address
563:     // 1 ADD ADD ADD ADD ADD ADD ADD
564:     // Sets DDRAM address. DDRAM data is sent and received after this setting.
565:     WriteInstruction(LCD_SETDDRAMADDR | (address & 0x7F));
566: }
567: 
568: /// <summary>
569: /// Convert the currently set cursor mode to display control register bits
570: /// </summary>
571: /// <returns>Cursor mode bits for display control register</returns>
572: uint8 HD44780Device::ConvertCursorMode() const
573: {
574:     return (m_cursorMode == CursorMode::Blink) ? LCD_DISPLAYCONTROL_CURSOR_BLINK
575:                                                : ((m_cursorMode == CursorMode::Line) ? LCD_DISPLAYCONTROL_CURSOR_ON : LCD_DISPLAYCONTROL_CURSOR_OFF);
576: }
577: 
578: } // namespace device
```

- Line 56-57: We define the value for the RS pin for commands
- Line 58-59: We define the value for the RS pin for data
- Line 61-68: We define the command value for the Clear Display command
- Line 69-75: We define the command value for the Return Home command
- Line 76-83: We define the command value for the Entry Mode Set command
- Line 84-85: We define the entry mode value for moving right after write
- Line 86-87: We define the entry mode value for moving left after write
- Line 88-89: We define the entry mode value for performing a shift instead of a cursor move
- Line 90-91: We define the entry mode value for moving the cursor instead of performing a shift instead
- Line 92-100: We define the command value for the Display Control command
- Line 101-102: We define the display control value for switching the display on
- Line 103-104: We define the display control value for switching the display off
- Line 105-106: We define the display control value for switching the cursor on
- Line 107-108: We define the display control value for switching the cursor off
- Line 109-110: We define the display control value for switching the cursor to a blinking block
- Line 111-112: We define the display control value for switching the cursor to a line
- Line 113-120: We define the command value for the Cursor Or Display Shift command
- Line 121-122: We define the cursor/display shift value for shifting the display after a write
- Line 123-124: We define the cursor/display shift value for shifting the display left after a write
- Line 125-126: We define the cursor/display shift value for shifting the display right after a write
- Line 127-128: We define the cursor/display shift value for moving the cursor after a write
- Line 129-130: We define the cursor/display shift value for moving the cursor left after a write
- Line 131-132: We define the cursor/display shift value for moving the cursor right after a write
- Line 133-143: We define the command value for the Set Display Configuration command
- Line 144-145: We define the display configuration value for 8 bit transfer mode
- Line 146-147: We define the display configuration value for 4 bit transfer mode
- Line 148-149: We define the display configuration value for a multi line display
- Line 150-151: We define the display configuration value for a single line display
- Line 152-153: We define the display configuration value for character size 5 x 10 pixels
- Line 154-155: We define the display configuration value for character size 5 x 8 pixels
- Line 156-162: We define the command value for the Set CGRAM Address command
- Line 163-169: We define the command value for the Set DDRAM Address command
- Line 171-211: We implement the constructor
  - Line 203-209: We calculate the row offsets here as well.
The order is quite peculiar.
- Line 213-219: We implement the destructor
- Line 221-228: We implement the method GetNumColumns()
- Line 230-237: We implement the method GetNumRows()
- Line 239-285: We implement the method Initialize()
  - Line 245-247: We determine the display configuration value from the value passed to the constructor
  - Line 249-268: We perform initialization as described in [Initialization sequence](#TUTORIAL_22_LCD_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_INITIALIZATION_SEQUENCE)
  - Line 270-275: We perform the initial mode settings as described in [Mode settings](#TUTORIAL_22_LCD_DISPLAY_CONTROLLING_A_I2C_LCD_DISPLAY_MODE_SETTING)
- Line 278-286: We implement the method SetBacklight().
As not all devices support this, the default implementation does nothing
- Line 288-296: We implement the method IsBacklightOn().
Again the default implementation acts as if backlight is not supported, always returning false
- Line 298-305: We implement the method IsDataMode4Bits()
- Line 307-325: We implement the method Write() for a single character.
This writes the character at the current location, and advances the position.
If the position reaches beyond the last column, the line is incremented (wrapping around to the top if needed) and the column is set to the left side again
- Line 327-337: We implement the method Write() for a single character at a specific location.
This will set the cursor location, and then write the character as above
- Line 339-350: We implement the method Write() for a character string with specified number of characters.
This will simply write each character up to the specified count, meanwhile updating the cursor location
- Line 352-359: We implement the method Write() for a character string.
This will simply write each character up to the first null character, meanwhile updating the cursor location
- Line 361-370: We implement the method Write() for a character string at a specific location.
This will set the cursor location, and then write the string as above
- Line 372-380: We implement the method ClearDisplay().
This clears the display contents and sets the cursor to the home position (top left) using the command Clear Display
- Line 382-391: We implement the method Home().
This will set the cursor location to the home position using the command Return Home, and set the internal cursor location to (0,0)
- Line 393-410: We implement the method ShiftDisplay().
This move the display contents to the left (negative amount) or right (positive amount).
The amount is the amount of a character to shift (-amount for left, +amount for right).
The cursor location is not changed
- Line 412-420: We implement the method SetDisplayEnabled().
This switches the display on or off.
This settings is not related to any backlight setting, it simply switches controller output on or off.
This sets the display mode, and calls UpdateDisplayControl() to perform the actual display setting
- Line 422-430: We implement the method SetCursorMode().
This sets the cursor mode, and calls UpdateDisplayControl() to perform the actual display setting
- Line 432-442: We implement the method SetCursorPosition().
This sets the cursor location to (cursorX,cursorY).
For the display controller, this means setting the Display Data RAM offset to cursorX + m_rowOffsets[cursorY], where m_rowOffsets is the array of offsets calculated in the constructor
- Line 444-467: We implement the method DefineCharFont().
This defines one of the 8 user defined characters.
The character codes are expected to be 0x80-0x87.
The effect is that the Character Generator RAM address is set to the start address for the specified character, and the pixel patterns (only least significant 5 bits) are written to this location)
- Line 469-480: We implement the method DisplayControl().
This sets both the display mode and cursor mode specified, and calls UpdateDisplayControl() to perform the actual display setting
- Line 482-489: We implement the method WriteInstruction().
This writes a instruction byte to the display controller (RS pin low).
The WriteByte() method is called with RegisterSelect::Cmd
- Line 491-498: We implement the method WriteData().
This writes a data byte to the display controller (RS pin high).
The WriteByte() method is called with RegisterSelect::Data
- Line 500-518: We implement the method WriteByte().
This writes a byte to the display controller.
Data is written in two 4 bit parts. 8 bit transfer is not supported yet.
The data to be written is placed in the most significant 4 bits, the instruction / data bit specifies the RS pin value in bit 0
- Line 520-526: We implement the method UpdateDisplayControl().
This writes a command Display On/Off Control specifying the display on/off status as well as the cursor on/off and mode status
- Line 528-536: We implement the method CursorOrDisplayShift().
This writes a command Cursor Or Display Shift specifying the shift mode and the move direction
- Line 538-551: We implement the method SetCGRAM_Address().
This writes a command Set CGRAM Address specifying the new start offset for CGRAM read or write operations
- Line 553-566: We implement the method SetDDRAM_Address().
This writes a command Set DDRAM Address specifying the new start offset for DDRAM read or write operations
- Line 568-576: We implement the method ConvertCursorMode().
This converts the cursor mode to the correct Display Control bit pattern

## GPIO operation {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION}

For displays that do not have a piggyback board, we will need to control the different signals using GPIO.

### HD44780DeviceRaw.h {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_HD44780DEVICERAWH}

We'll declare a new class `HD44780DeviceRaw` which derives from `HD44780Device` for implementation using GPIO pins.
As this is GPIO related functionality, we'll place the header and source file under the `gpio` folder.

Create the file `code/libraries/device/include/device/gpio/HD44780DeviceRaw.h`

```cpp
File: code/libraries/device/include/device/gpio/HD44780DeviceRaw.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780DeviceRaw.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780DeviceRaw
9: //
10: // Description : HD44780 based 16x2 LCD display with direct (raw) interface (no I2C piggyback)
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
42: #include <device/display/HD44780Device.h>
43: 
44: #include <baremetal/PhysicalGPIOPin.h>
45: 
46: /// @file
47: /// HD44780 based GPIO LCD character display (max 40x4 characters)
48: 
49: namespace device {
50: 
51: /// <summary>
52: /// GPIO controlled HD44780 based LCD display
53: /// </summary>
54: class HD44780DeviceRaw : public HD44780Device
55: {
56: private:
57:     /// @brief GPIO pin for D4
58:     baremetal::PhysicalGPIOPin m_d4Pin;
59:     /// @brief GPIO pin for D5
60:     baremetal::PhysicalGPIOPin m_d5Pin;
61:     /// @brief GPIO pin for D6
62:     baremetal::PhysicalGPIOPin m_d6Pin;
63:     /// @brief GPIO pin for D7
64:     baremetal::PhysicalGPIOPin m_d7Pin;
65:     /// @brief GPIO pin for E (Enable / Clock)
66:     baremetal::PhysicalGPIOPin m_enPin;
67:     /// @brief GPIO pin for RS (Register Select)
68:     baremetal::PhysicalGPIOPin m_rsPin;
69: 
70: public:
71:     HD44780DeviceRaw(baremetal::IMemoryAccess &memoryAccess, uint8 numColumns, uint8 numRows, uint8 d4Pin, uint8 d5Pin, uint8 d6Pin, uint8 d7Pin, uint8 enPin, uint8 rsPin,
72:                      CharacterSize characterSize = CharacterSize::Size5x8);
73:     HD44780DeviceRaw(const HD44780DeviceRaw&) = delete;
74:     ~HD44780DeviceRaw();
75: 
76:     HD44780DeviceRaw &operator=(const HD44780DeviceRaw &other) = delete;
77: 
78: protected:
79:     void WriteHalfByte(uint8 data) override;
80: };
81: 
82: } // namespace device
```

- Line 51-78: We declare the class `HD44780DeviceRaw` which derives from `HD44780Device`.
  - Line 57-58: We declare the member variable m_d4Pin for the GPIO pin connected to the display controller's D4 pin
  - Line 59-60: We declare the member variable m_d5Pin for the GPIO pin connected to the display controller's D5 pin
  - Line 61-62: We declare the member variable m_d6Pin for the GPIO pin connected to the display controller's D6 pin
  - Line 63-64: We declare the member variable m_d7Pin for the GPIO pin connected to the display controller's D7 pin
  - Line 65-66: We declare the member variable m_enPin for the GPIO pin connected to the display controller's E pin
  - Line 67-68: We declare the member variable m_rsPin for the GPIO pin connected to the display controller's RS pin
  - Line 71-72: We declare the constructor
  - Line 73: We remove the copy constructor
  - Line 74: We declare the destructor
  - Line 76: We remove the assignment operator
  - Line 79: We override the method `WriteHalfByte()`

### HD44780DeviceRaw.cpp {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_HD44780DEVICERAWCPP}

We'll implement the class `HD44780DeviceRaw`.

Create the file `code/libraries/device/src/gpio/HD44780DeviceRaw.cpp`

```cpp
File: code/libraries/device/src/gpio/HD44780DeviceRaw.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780DeviceRaw.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780DeviceRaw
9: //
10: // Description : HD44780 based 16x2 LCD display with direct (raw) interface (no I2C piggyback)
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
40: #include <device/gpio/HD44780DeviceRaw.h>
41: 
42: #include <baremetal/Timer.h>
43: 
44: using namespace baremetal;
45: 
46: /// @file
47: /// HD44780 based GPIO 16x2 LCD display
48: 
49: namespace device {
50: 
51: /// <summary>
52: /// Constructor
53: ///
54: /// \note Driver uses 4-bit mode, pins D0-D3 are not used.
55: /// \note We don't support reading from GPIO controlled displays
56: /// </summary>
57: /// <param name="memoryAccess">  Memory access interface for GPIO pins</param>
58: /// <param name="numColumns">    Display size in number of columns (max. 40)</param>
59: /// <param name="numRows">       Display size in number of rows (max. 4)</param>
60: /// <param name="d4Pin">         GPIO pin number of Data 4 pin (Broadcom GPIO number)</param>
61: /// <param name="d5Pin">         GPIO pin number of Data 5 pin (Broadcom GPIO number)</param>
62: /// <param name="d6Pin">         GPIO pin number of Data 6 pin (Broadcom GPIO number)</param>
63: /// <param name="d7Pin">         GPIO pin number of Data 7 pin (Broadcom GPIO number)</param>
64: /// <param name="enPin">         GPIO pin number of Enable pin (Broadcom GPIO number)</param>
65: /// <param name="rsPin">         GPIO pin number of Register Select pin (Broadcom GPIO number)</param>
66: /// <param name="characterSize"> Character size, either 5x8 pixels (default) or 5x10 pixels</param>
67: HD44780DeviceRaw::HD44780DeviceRaw(IMemoryAccess &memoryAccess, uint8 numColumns, uint8 numRows, uint8 d4Pin, uint8 d5Pin, uint8 d6Pin, uint8 d7Pin, uint8 enPin, uint8 rsPin,
68:                                    CharacterSize characterSize /*= CharacterSize::CharacterSize::Size5x8*/)
69:     : HD44780Device{numColumns, numRows, characterSize, DataMode::Mode4Bit}
70:     , m_d4Pin{d4Pin, GPIOMode::Output, memoryAccess}
71:     , m_d5Pin{d5Pin, GPIOMode::Output, memoryAccess}
72:     , m_d6Pin{d6Pin, GPIOMode::Output, memoryAccess}
73:     , m_d7Pin{d7Pin, GPIOMode::Output, memoryAccess}
74:     , m_enPin{enPin, GPIOMode::Output, memoryAccess}
75:     , m_rsPin{rsPin, GPIOMode::Output, memoryAccess}
76: {
77:     m_enPin.Off();
78:     m_rsPin.Off();
79: }
80: 
81: /// <summary>
82: /// Destructor
83: ///
84: /// Switches all pins back to input mode
85: /// </summary>
86: HD44780DeviceRaw::~HD44780DeviceRaw()
87: {
88:     DisplayControl(DisplayMode::Hide, CursorMode::Hide);
89: 
90:     m_d4Pin.SetMode(GPIOMode::Input);
91:     m_d5Pin.SetMode(GPIOMode::Input);
92:     m_d6Pin.SetMode(GPIOMode::Input);
93:     m_d7Pin.SetMode(GPIOMode::Input);
94:     m_enPin.SetMode(GPIOMode::Input);
95:     m_rsPin.SetMode(GPIOMode::Input);
96: }
97: 
98: /// <summary>
99: /// Write a 4 bit value
100: ///
101: /// The top 4 bits are the data written, bit 0 acts as the register select bit (0 = instruction, 1 = data)
102: /// | Output pin | Display connection | Function |
103: /// |------------|--------------------|----------|
104: /// | 0          | RS                 | Register Select (0 = instruction, 1 = data |
105: /// | 1          | RW                 | Read/Write (0 = write, 1 = read)           |
106: /// | 2          | E                  | Enable: Starts data read / write           |
107: /// | 3          | BL                 | Backlight (0 = off, 1 = on)                |
108: /// | 4          | D4                 | Data bit 4 (4 bit access only)             |
109: /// | 5          | D5                 | Data bit 5 (4 bit access only)             |
110: /// | 6          | D6                 | Data bit 6 (4 bit access only)             |
111: /// | 7          | D7                 | Data bit 7 (4 bit access only)             |
112: ///
113: /// </summary>
114: /// <param name="data">Value to write</param>
115: void HD44780DeviceRaw::WriteHalfByte(uint8 data)
116: {
117:     // Set Register Select bit
118:     m_rsPin.Set((data & 0x01) ? true : false);
119:     // Set Bit D7-D4
120:     m_d4Pin.Set((data & 0x10) ? true : false);
121:     m_d5Pin.Set((data & 0x20) ? true : false);
122:     m_d6Pin.Set((data & 0x40) ? true : false);
123:     m_d7Pin.Set((data & 0x80) ? true : false);
124: 
125:     // Enable high: start data write
126:     m_enPin.On();
127:     Timer::WaitMicroSeconds(1);
128:     // Enable high: finish data write
129:     m_enPin.Off();
130:     Timer::WaitMicroSeconds(50);
131:     // Fall back to instruction
132:     m_rsPin.Off();
133: }
134: 
135: } // namespace device
```

- Line 51-79: We define the constructor.
We configure all specified GPIO pins as outputs
- Line 81-96: We define the destructor
We configure all used GPIO pins as inputs
- Line 98-133: We implement the method `WriteHalfByte()`.
This sets the RS pin depending on the data bit 0, D4-D& corresponding to data bits 4..7.
Then it switches the E pin on, and after some time off again.
This will clock the data into the display controller (falling edge of E).
Finally we switch the RS back to off

### MemoryAccessStubGPIO.cpp {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_MEMORYACCESSSTUBGPIOCPP}

As we don't have an actual GPIO controlled display, we'll use the GPIO MemoryAccess stub.
However, we'd like to be able to configure at build time whether or not to log output.
So let's update the source to use a defininition we'll then set in the main CMake file.

Update the file `code/libraries/baremetal/src/stubs/MemoryAccessStubGPIO.cpp`.

```cpp
File: d:\Projects\RaspberryPi\baremetal.test\code\libraries\baremetal\src\stubs\MemoryAccessStubGPIO.cpp
...
56: /// @brief GPIO base address
57: static uintptr GPIOBaseAddress{ RPI_GPIO_BASE };
58: /// @brief Mask used to check whether an address is in the GPIO register range
59: static uintptr GPIOBaseAddressMask{ 0xFFFFFFFFFFFFFF00 };
...
111: #if BAREMETAL_MEMORY_ACCESS_TRACING
112: /// <summary>
113: /// Convert pin mode to string
114: /// </summary>
115: /// <param name="mode">Pin mode</param>
116: /// <returns>String representing pin mode</returns>
117: static string PinModeToString(uint32 mode)
118: {
119:     string result{};
120:     switch (mode & 0x07)
121:     {
122:     case 0:
123:         result = "Input";
124:         break;
125:     case 1:
126:         result = "Output";
127:         break;
128:     case 2:
129:         result = "Alt5";
130:         break;
131:     case 3:
132:         result = "Alt4";
133:         break;
134:     case 4:
135:         result = "Alt0";
136:         break;
137:     case 5:
138:         result = "Alt1";
139:         break;
140:     case 6:
141:         result = "Alt2";
142:         break;
143:     case 7:
144:         result = "Alt3";
145:         break;
146:     }
147:     return result;
148: }
149: 
150: /// <summary>
151: /// Convert pull up/down mode to string
152: /// </summary>
153: /// <param name="mode">Pull up/down mode</param>
154: /// <returns>String representing pull up/down mode</returns>
155: static string PullUpDownModeToString(uint32 mode)
156: {
157:     string result{};
158:     switch (mode & 0x03)
159:     {
160:     case 0:
161:         result = "None";
162:         break;
163:     case 1:
164: #if BAREMETAL_RPI_TARGET == 3
165:         result = "PullDown";
166: #else
167:         result = "PullUp";
168: #endif
169:         break;
170:     case 2:
171: #if BAREMETAL_RPI_TARGET == 3
172:         result = "PullUp";
173: #else
174:         result = "PullDown";
175: #endif
176:         break;
177:     case 3:
178:         result = "Reserved";
179:         break;
180:     }
181:     return result;
182: }
183: #endif
184: 
185: /// <summary>
186: /// Read a 32 bit value from register at address
187: /// </summary>
188: /// <param name="address">Address of register</param>
189: /// <returns>32 bit register value</returns>
190: uint32 MemoryAccessStubGPIO::Read32(regaddr address)
191: {
192:     uintptr offset = GetRegisterOffset(address);
193:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
194: #if BAREMETAL_MEMORY_ACCESS_TRACING
195: #if BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL
196:     LOG_DEBUG("GPIO read register %016x = %08x", offset, *registerField);
197: #endif
198:     switch (offset)
199:     {
200:         case RPI_GPIO_GPFSEL0_OFFSET:
201:         case RPI_GPIO_GPFSEL1_OFFSET:
202:         case RPI_GPIO_GPFSEL2_OFFSET:
203:         case RPI_GPIO_GPFSEL3_OFFSET:
204:         case RPI_GPIO_GPFSEL4_OFFSET:
205:         case RPI_GPIO_GPFSEL5_OFFSET:
206:         {
207:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
208:             string line{ "GPIO Read Pin Mode "};
209:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
210:             {
211:                 int shift = pinIndex * 3;
212:                 uint8 pin = pinBase + pinIndex;
213:                 uint8 pinMode = (*registerField >> shift) & 0x00000007;
214:                 line += Format(" - Pin %d mode %s", pin, PinModeToString(pinMode).c_str());
215:             }
216:             LOG_DEBUG(line.c_str());
217:             break;
218:         }
219:         case RPI_GPIO_GPLEV0_OFFSET:
220:         case RPI_GPIO_GPLEV1_OFFSET:
221:         {
222:             uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET)  / 4 * 32;
223:             string line{ "GPIO Read Pin Level "};
224:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
225:             {
226:                 int shift = pinIndex;
227:                 uint8 pin = pinBase + pinIndex;
228:                 uint8 value = (*registerField >> shift) & 0x00000001;
229:                 if (value)
230:                 line += " - Pin %d ON ";
231:                 else
232:                 line += " - Pin %d OFF";
233:             }
234:             LOG_DEBUG(line.c_str());
235:             break;
236:         }
237:         case RPI_GPIO_GPEDS0_OFFSET:
238:         case RPI_GPIO_GPEDS1_OFFSET:
239:         {
240:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET)  / 4 * 32;
241:             string line{ "GPIO Read Pin Event Detect Status "};
242:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
243:             {
244:                 int shift = pinIndex;
245:                 uint8 pin = pinBase + pinIndex;
246:                 uint8 value = (*registerField >> shift) & 0x00000001;
247:                 if (value)
248:                 line += " - Pin %d ON ";
249:                 else
250:                 line += " - Pin %d OFF";
251:             }
252:             LOG_DEBUG(line.c_str());
253:             break;
254:         }
255:         case RPI_GPIO_GPREN0_OFFSET:
256:         case RPI_GPIO_GPREN1_OFFSET:
257:         {
258:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET)  / 4 * 32;
259:             string line{ "GPIO Read Pin Rising Edge Detect Enable "};
260:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
261:             {
262:                 int shift = pinIndex;
263:                 uint8 pin = pinBase + pinIndex;
264:                 uint8 value = (*registerField >> shift) & 0x00000001;
265:                 if (value)
266:                 line += Format(" - Pin %d ON ", pin);
267:                 else
268:                 line += Format(" - Pin %d OFF", pin);
269:             }
270:             LOG_DEBUG(line.c_str());
271:             break;
272:         }
273:         case RPI_GPIO_GPFEN0_OFFSET:
274:         case RPI_GPIO_GPFEN1_OFFSET:
275:         {
276:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET)  / 4 * 32;
277:             string line{ "GPIO Read Pin Falling Edge Detect Enable "};
278:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
279:             {
280:                 int shift = pinIndex;
281:                 uint8 pin = pinBase + pinIndex;
282:                 uint8 value = (*registerField >> shift) & 0x00000001;
283:                 if (value)
284:                 line += Format(" - Pin %d ON ", pin);
285:                 else
286:                 line += Format(" - Pin %d OFF", pin);
287:             }
288:             LOG_DEBUG(line.c_str());
289:             break;
290:         }
291:         case RPI_GPIO_GPHEN0_OFFSET:
292:         case RPI_GPIO_GPHEN1_OFFSET:
293:         {
294:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET)  / 4 * 32;
295:             string line{ "GPIO Read Pin High Level Detect Enable "};
296:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
297:             {
298:                 int shift = pinIndex;
299:                 uint8 pin = pinBase + pinIndex;
300:                 uint8 value = (*registerField >> shift) & 0x00000001;
301:                 if (value)
302:                 line += Format(" - Pin %d ON ", pin);
303:                 else
304:                 line += Format(" - Pin %d OFF", pin);
305:             }
306:             LOG_DEBUG(line.c_str());
307:             break;
308:         }
309:         case RPI_GPIO_GPLEN0_OFFSET:
310:         case RPI_GPIO_GPLEN1_OFFSET:
311:         {
312:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET)  / 4 * 32;
313:             string line{ "GPIO Read Pin Low Level Detect Enable "};
314:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
315:             {
316:                 int shift = pinIndex;
317:                 uint8 pin = pinBase + pinIndex;
318:                 uint8 value = (*registerField >> shift) & 0x00000001;
319:                 if (value)
320:                 line += Format(" - Pin %d ON ", pin);
321:                 else
322:                 line += Format(" - Pin %d OFF", pin);
323:             }
324:             LOG_DEBUG(line.c_str());
325:             break;
326:         }
327:         case RPI_GPIO_GPAREN0_OFFSET:
328:         case RPI_GPIO_GPAREN1_OFFSET:
329:         {
330:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET)  / 4 * 32;
331:             string line{ "GPIO Read Pin Async Rising Edge Detect Enable "};
332:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
333:             {
334:                 int shift = pinIndex;
335:                 uint8 pin = pinBase + pinIndex;
336:                 uint8 value = (*registerField >> shift) & 0x00000001;
337:                 if (value)
338:                 line += Format(" - Pin %d ON ", pin);
339:                 else
340:                 line += Format(" - Pin %d OFF", pin);
341:             }
342:             LOG_DEBUG(line.c_str());
343:             break;
344:         }
345:         case RPI_GPIO_GPAFEN0_OFFSET:
346:         case RPI_GPIO_GPAFEN1_OFFSET:
347:         {
348:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET)  / 4 * 32;
349:             string line{ "GPIO Read Pin Async Falling Edge Detect Enable "};
350:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
351:             {
352:                 int shift = pinIndex;
353:                 uint8 pin = pinBase + pinIndex;
354:                 uint8 value = (*registerField >> shift) & 0x00000001;
355:                 if (value)
356:                 line += Format(" - Pin %d ON ", pin);
357:                 else
358:                 line += Format(" - Pin %d OFF", pin);
359:             }
360:             LOG_DEBUG(line.c_str());
361:             break;
362:         }
363: #if BAREMETAL_RPI_TARGET == 3
364:         case RPI_GPIO_GPPUD_OFFSET:
365:         {
366:             uint8 value = *registerField & 0x00000003;
367:             LOG_DEBUG("GPIO Read Pull Up/Down Mode %s", PullUpDownModeToString(value).c_str());
368:             break;
369:         }
370:         case RPI_GPIO_GPPUDCLK0_OFFSET:
371:         case RPI_GPIO_GPPUDCLK1_OFFSET:
372:         {
373:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
374:             string line{ "GPIO Read Pin Pull Up/Down Enable Clock "};
375:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
376:             {
377:                 int shift = pinIndex;
378:                 uint8 pin = pinBase + pinIndex;
379:                 uint8 value = (*registerField >> shift) & 0x00000001;
380:                 if (value)
381:                 line += Format(" - Pin %d ON ", pin);
382:                 else
383:                 line += Format(" - Pin %d OFF", pin);
384:             }
385:             LOG_DEBUG(line.c_str());
386:             break;
387:         }
388: #elif BAREMETAL_RPI_TARGET == 4
389:         case RPI_GPIO_GPPINMUXSD_OFFSET:
390:         {
391:             uint32 value = *registerField;
392:             LOG_DEBUG("GPIO Read Pin Mux Mode %x", value);
393:             break;
394:         }
395:         case RPI_GPIO_GPPUPPDN0_OFFSET:
396:         case RPI_GPIO_GPPUPPDN1_OFFSET:
397:         case RPI_GPIO_GPPUPPDN2_OFFSET:
398:         case RPI_GPIO_GPPUPPDN3_OFFSET:
399:         {
400:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
401:             string line{ "GPIO Read Pin Pull Up/Down Mode "};
402:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
403:             {
404:                 int shift = pinIndex * 2;
405:                 uint8 pin = pinBase + pinIndex;
406:                 uint8 value = (*registerField >> shift) & 0x00000003;
407:                 line += Format(" - Pin %d Pull up/down mode %s", pin, PullUpDownModeToString(value).c_str());
408:             }
409:             LOG_DEBUG(line.c_str());
410:             break;
411:         }
412: #endif
413:         default:
414:             LOG_ERROR("Invalid register access for reading: offset %d", offset);
415:             break;
416:     }
417: #endif
418:     return *registerField;
419: }
420: 
421: /// <summary>
422: /// Write a 32 bit value to register at address
423: /// </summary>
424: /// <param name="address">Address of register</param>
425: /// <param name="data">Data to write</param>
426: void MemoryAccessStubGPIO::Write32(regaddr address, uint32 data)
427: {
428:     uintptr offset = GetRegisterOffset(address);
429:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
430: #if BAREMETAL_MEMORY_ACCESS_TRACING
431: #if BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL
432:     LOG_DEBUG("GPIO write register %016x = %08x", offset, data);
433: #endif
434:     switch (offset)
435:     {
436:         case RPI_GPIO_GPFSEL0_OFFSET:
437:         case RPI_GPIO_GPFSEL1_OFFSET:
438:         case RPI_GPIO_GPFSEL2_OFFSET:
439:         case RPI_GPIO_GPFSEL3_OFFSET:
440:         case RPI_GPIO_GPFSEL4_OFFSET:
441:         case RPI_GPIO_GPFSEL5_OFFSET:
442:         {
443:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
444:             uint32 diff = data ^ *registerField;
445:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
446:             {
447:                 int shift = pinIndex * 3;
448:                 if (((diff >> shift) & 0x00000007) != 0)
449:                 {
450:                     uint8 pin = pinBase + pinIndex;
451:                     uint8 pinMode = (data >> shift) & 0x00000007;
452:                     string modeName = PinModeToString(pinMode);
453:                     LOG_DEBUG("GPIO Set Pin %d Mode %s", pin, modeName.c_str());
454:                 }
455:             }
456:             break;
457:         }
458:         case RPI_GPIO_GPSET0_OFFSET:
459:         case RPI_GPIO_GPSET1_OFFSET:
460:         {
461:             uint8 pinBase = (offset - RPI_GPIO_GPSET0_OFFSET) / 4 * 32;
462:             uint32 diff = data ^ *registerField;
463:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
464:             {
465:                 int shift = pinIndex;
466:                 if (((diff >> shift) & 0x00000001) != 0)
467:                 {
468:                     uint8 pin = pinBase + pinIndex;
469:                     uint8 value = (data >> shift) & 0x00000001;
470:                     if (value != 0)
471:                         LOG_DEBUG("GPIO Set Pin %d ON", pin);
472:                 }
473:             }
474:             break;
475:         }
476:         case RPI_GPIO_GPCLR0_OFFSET:
477:         case RPI_GPIO_GPCLR1_OFFSET:
478:         {
479:             uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET)  / 4 * 32;
480:             uint32 diff = data ^ *registerField;
481:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
482:             {
483:                 int shift = pinIndex;
484:                 if (((diff >> shift) & 0x00000001) != 0)
485:                 {
486:                     uint8 pin = pinBase + pinIndex;
487:                     uint8 value = (data >> shift) & 0x00000001;
488:                     if (value != 0)
489:                         LOG_DEBUG("GPIO Set Pin %d OFF", pin);
490:                 }
491:             }
492:             break;
493:         }
494:         case RPI_GPIO_GPEDS0_OFFSET:
495:         case RPI_GPIO_GPEDS1_OFFSET:
496:         {
497:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET)  / 4 * 32;
498:             uint32 diff = data ^ *registerField;
499:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
500:             {
501:                 int shift = pinIndex;
502:                 if (((diff >> shift) & 0x00000001) != 0)
503:                 {
504:                     uint8 pin = pinBase + pinIndex;
505:                     uint8 value = (data >> shift) & 0x00000001;
506:                     if (value != 0)
507:                         LOG_DEBUG("GPIO Clear Pin %d Event Status", pin);
508:                 }
509:             }
510:             break;
511:         }
512:         case RPI_GPIO_GPREN0_OFFSET:
513:         case RPI_GPIO_GPREN1_OFFSET:
514:         {
515:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET)  / 4 * 32;
516:             uint32 diff = data ^ *registerField;
517:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
518:             {
519:                 int shift = pinIndex;
520:                 if (((diff >> shift) & 0x00000001) != 0)
521:                 {
522:                     uint8 pin = pinBase + pinIndex;
523:                     uint8 value = (data >> shift) & 0x00000001;
524:                     if (value != 0)
525:                         LOG_DEBUG("GPIO Set Pin %d Rising Edge Detect ON", pin);
526:                     else
527:                         LOG_DEBUG("GPIO Set Pin %d Rising Edge Detect OFF", pin);
528:                 }
529:             }
530:             break;
531:         }
532:         case RPI_GPIO_GPFEN0_OFFSET:
533:         case RPI_GPIO_GPFEN1_OFFSET:
534:         {
535:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET)  / 4 * 32;
536:             uint32 diff = data ^ *registerField;
537:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
538:             {
539:                 int shift = pinIndex;
540:                 if (((diff >> shift) & 0x00000001) != 0)
541:                 {
542:                     uint8 pin = pinBase + pinIndex;
543:                     uint8 value = (data >> shift) & 0x00000001;
544:                     if (value != 0)
545:                         LOG_DEBUG("GPIO Set Pin %d Falling Edge Detect ON", pin);
546:                     else
547:                         LOG_DEBUG("GPIO Set Pin %d Falling Edge Detect OFF", pin);
548:                 }
549:             }
550:             break;
551:         }
552:         case RPI_GPIO_GPHEN0_OFFSET:
553:         case RPI_GPIO_GPHEN1_OFFSET:
554:         {
555:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET)  / 4 * 32;
556:             uint32 diff = data ^ *registerField;
557:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
558:             {
559:                 int shift = pinIndex;
560:                 if (((diff >> shift) & 0x00000001) != 0)
561:                 {
562:                     uint8 pin = pinBase + pinIndex;
563:                     uint8 value = (data >> shift) & 0x00000001;
564:                     if (value != 0)
565:                         LOG_DEBUG("GPIO Set Pin %d High Level Detect ON", pin);
566:                     else
567:                         LOG_DEBUG("GPIO Set Pin %d High Level Detect OFF", pin);
568:                 }
569:             }
570:             break;
571:         }
572:         case RPI_GPIO_GPLEN0_OFFSET:
573:         case RPI_GPIO_GPLEN1_OFFSET:
574:         {
575:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET)  / 4 * 32;
576:             uint32 diff = data ^ *registerField;
577:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
578:             {
579:                 int shift = pinIndex;
580:                 if (((diff >> shift) & 0x00000001) != 0)
581:                 {
582:                     uint8 pin = pinBase + pinIndex;
583:                     uint8 value = (data >> shift) & 0x00000001;
584:                     if (value != 0)
585:                         LOG_DEBUG("GPIO Set Pin %d Low Level Detect ON", pin);
586:                     else
587:                         LOG_DEBUG("GPIO Set Pin %d Low Level Detect OFF", pin);
588:                 }
589:             }
590:             break;
591:         }
592:         case RPI_GPIO_GPAREN0_OFFSET:
593:         case RPI_GPIO_GPAREN1_OFFSET:
594:         {
595:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET)  / 4 * 32;
596:             uint32 diff = data ^ *registerField;
597:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
598:             {
599:                 int shift = pinIndex;
600:                 if (((diff >> shift) & 0x00000001) != 0)
601:                 {
602:                     uint8 pin = pinBase + pinIndex;
603:                     uint8 value = (data >> shift) & 0x00000001;
604:                     if (value != 0)
605:                         LOG_DEBUG("GPIO Set Pin %d Async Rising Edge Detect ON", pin);
606:                     else
607:                         LOG_DEBUG("GPIO Set Pin %d Async Rising Edge Detect OFF", pin);
608:                 }
609:             }
610:             break;
611:         }
612:         case RPI_GPIO_GPAFEN0_OFFSET:
613:         case RPI_GPIO_GPAFEN1_OFFSET:
614:         {
615:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET)  / 4 * 32;
616:             uint32 diff = data ^ *registerField;
617:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
618:             {
619:                 int shift = pinIndex;
620:                 if (((diff >> shift) & 0x00000001) != 0)
621:                 {
622:                     uint8 pin = pinBase + pinIndex;
623:                     uint8 value = (data >> shift) & 0x00000001;
624:                     if (value != 0)
625:                         LOG_DEBUG("GPIO Set Pin %d Async Falling Edge Detect ON", pin);
626:                     else
627:                         LOG_DEBUG("GPIO Set Pin %d Async Falling Edge Detect OFF", pin);
628:                 }
629:             }
630:             break;
631:         }
632: #if BAREMETAL_RPI_TARGET == 3
633:         case RPI_GPIO_GPPUD_OFFSET:
634:         {
635:             uint32 diff = data ^ *registerField;
636:             if ((diff & 0x00000003) != 0)
637:             {
638:                 uint8 value = data & 0x00000003;
639:                 string modeName = PullUpDownModeToString(value);
640:                 LOG_DEBUG("GPIO Set Pin Pull Up/Down Mode %s", modeName.c_str());
641:             }
642:             break;
643:         }
644:         case RPI_GPIO_GPPUDCLK0_OFFSET:
645:         case RPI_GPIO_GPPUDCLK1_OFFSET:
646:         {
647:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
648:             uint32 diff = data ^ *registerField;
649:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
650:             {
651:                 int shift = pinIndex;
652:                 if (((diff >> shift) & 0x00000001) != 0)
653:                 {
654:                     uint8 pin = pinBase + pinIndex;
655:                     uint8 value = (data >> shift) & 0x00000001;
656:                     if (value != 0)
657:                         LOG_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock ON", pin);
658:                     else
659:                         LOG_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock OFF", pin);
660:                 }
661:             }
662:             break;
663:         }
664: #elif BAREMETAL_RPI_TARGET == 4
665:         case RPI_GPIO_GPPINMUXSD_OFFSET:
666:         {
667:             uint32 value = *registerField;
668:             LOG_DEBUG("GPIO Set Pin Mux Mode %x", value);
669:             break;
670:         }
671:         case RPI_GPIO_GPPUPPDN0_OFFSET:
672:         case RPI_GPIO_GPPUPPDN1_OFFSET:
673:         case RPI_GPIO_GPPUPPDN2_OFFSET:
674:         case RPI_GPIO_GPPUPPDN3_OFFSET:
675:         {
676:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
677:             uint32 diff = data ^ *registerField;
678:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
679:             {
680:                 int shift = pinIndex * 2;
681:                 if (((diff >> shift) & 0x00000003) != 0)
682:                 {
683:                     uint8 pin = pinBase + pinIndex;
684:                     uint8 value = (data >> shift) & 0x00000003;
685:                     string modeName = PullUpDownModeToString(value);
686:                     LOG_DEBUG("GPIO Set Pin %d Pull Up/Down Mode %s", pin, modeName.c_str());
687:                 }
688:             }
689:             break;
690:         }
691: #endif
692:         default:
693:             LOG_ERROR("Invalid GPIO register access for writing: offset %d", offset);
694:             break;
695:     }
696: #endif
697:     *registerField = data;
698: }
...
```

- Line 56-57: We remove the definition for `TRACE`
- Line 111: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 194: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 195-197: We surround the register read logging with a `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` definition, to limit the logging output a little
- Line 432: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 433-435: We surround the register write logging with a `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` definition, to limit the logging output a little

### Main CMake File {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_MAIN_CMAKE_FILE}

Now let's update the main CMake file to define `BAREMETAL_MEMORY_ACCESS_TRACING` and `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` when desired.

Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
...
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
69: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
70: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
71: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
72: option(BAREMETAL_TRACE_MEMORY_ACCESS_STUBS "Enable memory access tracing output on stubs" OFF)
73: option(BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL "Enable detailed memory access tracing output on stubs" OFF)
...
111: if (BAREMETAL_TRACE_MEMORY_DETAIL)
112:     set(BAREMETAL_MEMORY_TRACING 1)
113:     set(BAREMETAL_MEMORY_TRACING_DETAIL 1)
114: else ()
115:     set(BAREMETAL_MEMORY_TRACING_DETAIL 0)
116: endif()
117: if (BAREMETAL_TRACE_MEMORY_ACCESS_STUBS)
118:     set(BAREMETAL_MEMORY_ACCESS_TRACING 1)
119: else ()
120:     set(BAREMETAL_MEMORY_ACCESS_TRACING 0)
121: endif()
122: if (BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL)
123:     set(BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL 1)
124: else ()
125:     set(BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL 0)
126: endif()
127: set(BAREMETAL_LOAD_ADDRESS 0x80000)
128: 
129: set(DEFINES_C
130:     PLATFORM_BAREMETAL
131:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
132:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
133:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
134:     BAREMETAL_MEMORY_TRACING=${BAREMETAL_MEMORY_TRACING}
135:     BAREMETAL_MEMORY_TRACING_DETAIL=${BAREMETAL_MEMORY_TRACING_DETAIL}
136:     BAREMETAL_MEMORY_ACCESS_TRACING=${BAREMETAL_MEMORY_ACCESS_TRACING}
137:     BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL=${BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL}
138:     BAREMETAL_MAJOR=${VERSION_MAJOR}
139:     BAREMETAL_MINOR=${VERSION_MINOR}
140:     BAREMETAL_LEVEL=${VERSION_LEVEL}
141:     BAREMETAL_BUILD=${VERSION_BUILD}
142:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
143:     )
...
306: message(STATUS "Baremetal settings:")
307: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
308: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
309: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
310: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
311: message(STATUS "-- Debug output to UART0:           ${BAREMETAL_CONSOLE_UART0}")
312: message(STATUS "-- Debug output to UART1:           ${BAREMETAL_CONSOLE_UART1}")
313: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
314: message(STATUS "-- Debug tracing output:            ${BAREMETAL_TRACE_DEBUG}")
315: message(STATUS "-- Memory tracing output:           ${BAREMETAL_TRACE_MEMORY}")
316: message(STATUS "-- Detailed memory tracing output:  ${BAREMETAL_TRACE_MEMORY_DETAIL}")
317: message(STATUS "-- Memory access tracing output:    ${BAREMETAL_TRACE_MEMORY_ACCESS_STUBS}")
318: message(STATUS "-- Detailed Memory access tracing:  ${BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL}")
319: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
320: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
321: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
322: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
323: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
...
```

- Line 72: We add an option `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` which is off by default
- Line 73: We add an option `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL` which is off by default
- Line 117-121: Depending on the value of `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` we set the variable `BAREMETAL_MEMORY_ACCESS_TRACING` to either 0 or 1
- Line 122-126: Depending on the value of `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL` we set the variable `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` to either 0 or 1
- Line 136: We add the compiler definition `BAREMETAL_MEMORY_ACCESS_TRACING` as the value of the CMake variable `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 137: We add the compiler definition `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` as the value of the CMake variable `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL`
- Line 317-318: We print the actual values for `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` and `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL`

### Update build configuration {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_UPDATE_BUILD_CONFIGURATION}

We'll now make sure to set the value of `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` to `ON` when building.

Update the file `CMakeSettings.json`.

```text
File: CMakeSettings.json
...
4:         {
5:             "name": "BareMetal-RPI3-Debug",
6:             "generator": "Ninja",
7:             "configurationType": "Debug",
8:             "buildRoot": "${projectDir}\\cmake-${name}",
9:             "installRoot": "${projectDir}\\output\\install\\${name}",
10:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=ON -DBAREMETAL_CONSOLE_UART0=ON -DBAREMETAL_TRACE_MEMORY_ACCESS_STUBS=ON -DBAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL=OFF",
11:             "buildCommandArgs": "",
12:             "ctestCommandArgs": "",
13:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
14:             "inheritEnvironments": [ "gcc-arm" ]
15:         },
...
```

- Line 10: We set the value of variable `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` to `ON` and `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL` to `OFF`.

### Update application code {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_UPDATE_APPLICATION_CODE}

Let's try working with a GPIO display, as we don't have a real display with GPIO pins, we'll use a stub.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Format.h>
2: #include <baremetal/Logger.h>
3: #include <baremetal/PhysicalGPIOPin.h>
4: #include <baremetal/String.h>
5: #include <baremetal/System.h>
6: #include <baremetal/Timer.h>
7: #include <baremetal/stubs/MemoryAccessStubGPIO.h>
8: #include <device/gpio/HD44780DeviceRaw.h>
9: 
10: LOG_MODULE("main");
11: 
12: using namespace baremetal;
13: using namespace device;
14: 
15: int main()
16: {
17:     auto& console = GetConsole();
18:     GetLogger().SetLogLevel(LogSeverity::Debug);
19: 
20:     MemoryAccessStubGPIO memoryAccess;
21:     HD44780DeviceRaw device(memoryAccess, 16, 2, 8, 9, 10, 11, 12, 13);
22: 
23:     device.Initialize();
24: 
25:     device.SetDisplayEnabled(true);
26:     device.SetCursorMode(device::ILCDDevice::CursorMode::Hide);
27: 
28:     device.ClearDisplay();
29:     device.Write("Hello World");
30: 
31:     Timer::WaitMilliSeconds(2000);
32: 
33:     device.SetDisplayEnabled(false);
34: 
35:     LOG_INFO("Rebooting");
36: 
37:     return static_cast<int>(ReturnCode::ExitReboot);
38: }
```

- Line 20: We instantiate a MemoryAccess stub for GPIO
- Line 21: We create an instance of HD44780DeviceRaw injecting the stub.
We define the display as 16x2 characters, and assign GPIO pins 8-11 for D4-D7, 12 for the E pin, and 13 for the RS pin
- Line 23: We initialize the device
- Line 25: We switch the display on
- Line 26: We set the cursor to hidden
- Line 28: We clear the display
- Line 29: We write the text "Hello World"
- Line 33: We switch the display off again

### Update CMake file for device library {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_UPDATE_CMAKE_FILE_FOR_DEVICE_LIBRARY}

We need to add the newly added files to the `device` library.

Update the file `code/libraries/device/CMakeLists.txt`

```cmake
File: code/libraries/device/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/display/HD44780Device.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/KY-040.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/HD44780DeviceRaw.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/MCP23017.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/MemoryAccessStubMCP23017LEDs.cpp
36:     )
37: 
38: set(PROJECT_INCLUDES_PUBLIC
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/display/HD44780Device.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/display/ILCDDevice.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/gpio/HD44780DeviceRaw.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/gpio/KY-040.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/i2c/MCP23017.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/i2c/MemoryAccessStubMCP23017LEDs.h
45:     )
46: set(PROJECT_INCLUDES_PRIVATE )
47: 
```

### Configuring, building and debugging {#TUTORIAL_22_LCD_DISPLAY_GPIO_OPERATION_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.
Notice that the setup of the GPIO pins is logged by the stub.

```text
1: Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
2: Info   0.00:00:00.010 Starting up (System:209)
3: Debug  0.00:00:00.010 GPIO Set Pin 8 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
4: Debug  0.00:00:00.020 GPIO Set Pin 8 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
5: Debug  0.00:00:00.020 GPIO Read Pin Mode  - Pin 0 mode Input - Pin 1 mode Input - Pin 2 mode Input - Pin 3 mode Input - Pin 4 mode Input - Pin 5 mode Input - Pin 6 mode Input - Pin 7 mode Input - Pin 8 mode Input - Pin 9 mode Input (MemoryAccessStubGPIO:216)
6: Debug  0.00:00:00.030 GPIO Set Pin 8 Mode Output (MemoryAccessStubGPIO:453)
7: Debug  0.00:00:00.040 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
8: Debug  0.00:00:00.040 GPIO Set Pin 9 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
9: Debug  0.00:00:00.040 GPIO Set Pin 9 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
10: Debug  0.00:00:00.050 GPIO Read Pin Mode  - Pin 0 mode Input - Pin 1 mode Input - Pin 2 mode Input - Pin 3 mode Input - Pin 4 mode Input - Pin 5 mode Input - Pin 6 mode Input - Pin 7 mode Input - Pin 8 mode Output - Pin 9 mode Input (MemoryAccessStubGPIO:216)
11: Debug  0.00:00:00.060 GPIO Set Pin 9 Mode Output (MemoryAccessStubGPIO:453)
12: Debug  0.00:00:00.060 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
13: Debug  0.00:00:00.060 GPIO Set Pin 10 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
14: Debug  0.00:00:00.070 GPIO Set Pin 10 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
15: Debug  0.00:00:00.070 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Input - Pin 12 mode Input - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
16: Debug  0.00:00:00.080 GPIO Set Pin 10 Mode Output (MemoryAccessStubGPIO:453)
17: Debug  0.00:00:00.080 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
18: Debug  0.00:00:00.090 GPIO Set Pin 11 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
19: Debug  0.00:00:00.090 GPIO Set Pin 11 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
20: Debug  0.00:00:00.100 GPIO Read Pin Mode  - Pin 10 mode Output - Pin 11 mode Input - Pin 12 mode Input - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
21: Debug  0.00:00:00.110 GPIO Set Pin 11 Mode Output (MemoryAccessStubGPIO:453)
22: Debug  0.00:00:00.110 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
23: Debug  0.00:00:00.110 GPIO Set Pin 12 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
24: Debug  0.00:00:00.120 GPIO Set Pin 12 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
25: Debug  0.00:00:00.120 GPIO Read Pin Mode  - Pin 10 mode Output - Pin 11 mode Output - Pin 12 mode Input - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
26: Debug  0.00:00:00.130 GPIO Set Pin 12 Mode Output (MemoryAccessStubGPIO:453)
27: Debug  0.00:00:00.130 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
28: Debug  0.00:00:00.130 GPIO Set Pin 13 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
29: Debug  0.00:00:00.140 GPIO Set Pin 13 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
30: Debug  0.00:00:00.140 GPIO Read Pin Mode  - Pin 10 mode Output - Pin 11 mode Output - Pin 12 mode Output - Pin 13 mode Input - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
31: Debug  0.00:00:00.150 GPIO Set Pin 13 Mode Output (MemoryAccessStubGPIO:453)
32: Debug  0.00:00:00.160 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
33: Debug  0.00:00:00.160 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
34: Debug  0.00:00:00.170 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
35: Debug  0.00:00:00.180 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
36: Debug  0.00:00:00.190 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
37: Debug  0.00:00:00.190 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
38: Debug  0.00:00:00.190 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
39: Debug  0.00:00:00.200 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
40: Debug  0.00:00:00.200 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
41: Debug  0.00:00:00.200 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
42: Debug  0.00:00:00.210 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
43: Debug  0.00:00:00.210 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
44: Debug  0.00:00:00.220 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
45: Debug  0.00:00:00.220 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
46: Debug  0.00:00:00.220 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
47: Debug  0.00:00:00.230 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
48: Debug  0.00:00:00.230 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
49: Debug  0.00:00:00.230 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
50: Debug  0.00:00:00.240 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
51: Debug  0.00:00:00.240 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
52: Debug  0.00:00:00.240 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
53: Debug  0.00:00:00.250 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
54: Debug  0.00:00:00.250 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
55: Debug  0.00:00:00.250 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
56: Debug  0.00:00:00.260 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
57: Debug  0.00:00:00.260 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
58: Debug  0.00:00:00.270 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
59: Debug  0.00:00:00.270 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
60: Debug  0.00:00:00.270 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
61: Debug  0.00:00:00.270 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
62: Debug  0.00:00:00.280 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
63: Debug  0.00:00:00.280 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
64: Debug  0.00:00:00.290 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
65: Debug  0.00:00:00.290 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
66: Debug  0.00:00:00.290 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
67: Debug  0.00:00:00.290 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
68: Debug  0.00:00:00.300 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
69: Debug  0.00:00:00.300 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
70: Debug  0.00:00:00.300 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
71: Debug  0.00:00:00.310 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
72: Debug  0.00:00:00.310 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
73: Debug  0.00:00:00.310 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
74: Debug  0.00:00:00.310 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
75: Debug  0.00:00:00.320 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
76: Debug  0.00:00:00.320 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
77: Debug  0.00:00:00.320 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
78: Debug  0.00:00:00.330 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
79: Debug  0.00:00:00.330 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
80: Debug  0.00:00:00.330 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
81: Debug  0.00:00:00.340 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
82: Debug  0.00:00:00.340 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
83: Debug  0.00:00:00.340 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
84: Debug  0.00:00:00.340 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
85: Debug  0.00:00:00.350 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
86: Debug  0.00:00:00.350 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
87: Debug  0.00:00:00.350 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
88: Debug  0.00:00:00.350 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
89: Debug  0.00:00:00.360 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
90: Debug  0.00:00:00.360 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
91: Debug  0.00:00:00.360 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
92: Debug  0.00:00:00.370 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
93: Debug  0.00:00:00.370 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
94: Debug  0.00:00:00.370 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
95: Debug  0.00:00:00.380 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
96: Debug  0.00:00:00.380 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
97: Debug  0.00:00:00.380 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
98: Debug  0.00:00:00.380 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
99: Debug  0.00:00:00.390 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
100: Debug  0.00:00:00.390 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
101: Debug  0.00:00:00.390 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
102: Debug  0.00:00:00.400 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
103: Debug  0.00:00:00.400 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
104: Debug  0.00:00:00.400 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
105: Debug  0.00:00:00.400 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
106: Debug  0.00:00:00.410 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
107: Debug  0.00:00:00.410 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
108: Debug  0.00:00:00.410 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
109: Debug  0.00:00:00.410 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
110: Debug  0.00:00:00.420 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
111: Debug  0.00:00:00.420 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
112: Debug  0.00:00:00.420 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
113: Debug  0.00:00:00.430 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
114: Debug  0.00:00:00.430 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
115: Debug  0.00:00:00.430 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
116: Debug  0.00:00:00.440 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
117: Debug  0.00:00:00.440 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
118: Debug  0.00:00:00.440 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
119: Debug  0.00:00:00.450 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
120: Debug  0.00:00:00.450 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
121: Debug  0.00:00:00.450 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
122: Debug  0.00:00:00.450 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
123: Debug  0.00:00:00.460 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
124: Debug  0.00:00:00.460 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
125: Debug  0.00:00:00.460 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
126: Debug  0.00:00:00.470 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
127: Debug  0.00:00:00.470 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
128: Debug  0.00:00:00.470 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
129: Debug  0.00:00:00.470 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
130: Debug  0.00:00:00.480 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
131: Debug  0.00:00:00.480 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
132: Debug  0.00:00:00.490 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
133: Debug  0.00:00:00.490 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
134: Debug  0.00:00:00.500 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
135: Debug  0.00:00:00.500 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
136: Debug  0.00:00:00.500 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
137: Debug  0.00:00:00.510 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
138: Debug  0.00:00:00.510 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
139: Debug  0.00:00:00.510 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
140: Debug  0.00:00:00.510 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
141: Debug  0.00:00:00.510 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
142: Debug  0.00:00:00.520 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
143: Debug  0.00:00:00.520 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
144: Debug  0.00:00:00.530 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
145: Debug  0.00:00:00.530 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
146: Debug  0.00:00:00.530 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
147: Debug  0.00:00:00.540 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
148: Debug  0.00:00:00.550 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
149: Debug  0.00:00:00.550 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
150: Debug  0.00:00:00.560 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
151: Debug  0.00:00:00.560 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
152: Debug  0.00:00:00.570 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
153: Debug  0.00:00:00.570 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
154: Debug  0.00:00:00.570 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
155: Debug  0.00:00:00.570 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
156: Debug  0.00:00:00.580 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
157: Debug  0.00:00:00.580 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
158: Debug  0.00:00:00.580 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
159: Debug  0.00:00:00.590 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
160: Debug  0.00:00:00.590 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
161: Debug  0.00:00:00.590 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
162: Debug  0.00:00:00.590 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
163: Debug  0.00:00:00.600 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
164: Debug  0.00:00:00.610 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
165: Debug  0.00:00:00.610 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
166: Debug  0.00:00:00.620 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
167: Debug  0.00:00:00.620 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
168: Debug  0.00:00:00.620 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
169: Debug  0.00:00:00.620 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
170: Debug  0.00:00:00.630 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
171: Debug  0.00:00:00.630 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
172: Debug  0.00:00:00.630 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
173: Debug  0.00:00:00.630 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
174: Debug  0.00:00:00.640 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
175: Debug  0.00:00:00.640 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
176: Debug  0.00:00:00.640 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
177: Debug  0.00:00:00.650 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
178: Debug  0.00:00:00.650 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
179: Debug  0.00:00:00.650 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
180: Debug  0.00:00:00.660 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
181: Debug  0.00:00:00.660 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
182: Debug  0.00:00:00.660 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
183: Debug  0.00:00:00.670 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
184: Debug  0.00:00:00.670 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
185: Debug  0.00:00:00.670 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
186: Debug  0.00:00:00.670 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
187: Debug  0.00:00:00.680 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
188: Debug  0.00:00:00.680 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
189: Debug  0.00:00:00.680 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
190: Debug  0.00:00:00.690 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
191: Debug  0.00:00:00.690 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
192: Debug  0.00:00:00.690 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
193: Debug  0.00:00:00.700 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
194: Debug  0.00:00:00.700 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
195: Debug  0.00:00:00.700 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
196: Debug  0.00:00:00.710 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
197: Debug  0.00:00:00.710 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
198: Debug  0.00:00:00.710 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
199: Debug  0.00:00:00.720 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
200: Debug  0.00:00:00.720 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
201: Debug  0.00:00:00.720 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
202: Debug  0.00:00:00.730 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
203: Debug  0.00:00:00.730 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
204: Debug  0.00:00:00.730 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
205: Debug  0.00:00:00.730 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
206: Debug  0.00:00:00.740 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
207: Debug  0.00:00:00.740 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
208: Debug  0.00:00:00.740 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
209: Debug  0.00:00:00.750 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
210: Debug  0.00:00:00.750 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
211: Debug  0.00:00:00.750 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
212: Debug  0.00:00:00.760 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
213: Debug  0.00:00:00.760 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
214: Debug  0.00:00:00.760 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
215: Debug  0.00:00:00.770 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
216: Debug  0.00:00:00.770 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
217: Debug  0.00:00:00.770 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
218: Debug  0.00:00:00.780 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
219: Debug  0.00:00:00.780 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
220: Debug  0.00:00:00.780 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
221: Debug  0.00:00:00.790 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
222: Debug  0.00:00:00.790 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
223: Debug  0.00:00:00.790 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
224: Debug  0.00:00:00.790 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
225: Debug  0.00:00:00.800 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
226: Debug  0.00:00:00.800 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
227: Debug  0.00:00:00.800 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
228: Debug  0.00:00:00.810 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
229: Debug  0.00:00:00.810 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
230: Debug  0.00:00:00.810 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
231: Debug  0.00:00:00.820 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
232: Debug  0.00:00:00.820 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
233: Debug  0.00:00:00.820 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
234: Debug  0.00:00:00.830 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
235: Debug  0.00:00:00.830 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
236: Debug  0.00:00:00.830 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
237: Debug  0.00:00:00.840 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
238: Debug  0.00:00:00.840 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
239: Debug  0.00:00:00.840 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
240: Debug  0.00:00:00.840 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
241: Debug  0.00:00:00.850 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
242: Debug  0.00:00:00.850 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
243: Debug  0.00:00:00.850 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
244: Debug  0.00:00:00.860 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
245: Debug  0.00:00:00.860 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
246: Debug  0.00:00:00.860 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
247: Debug  0.00:00:00.870 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
248: Debug  0.00:00:00.870 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
249: Debug  0.00:00:00.870 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
250: Debug  0.00:00:00.880 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
251: Debug  0.00:00:00.880 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
252: Debug  0.00:00:00.880 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
253: Debug  0.00:00:00.890 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
254: Debug  0.00:00:00.890 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
255: Debug  0.00:00:00.890 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
256: Debug  0.00:00:00.900 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
257: Debug  0.00:00:00.900 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
258: Debug  0.00:00:00.900 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
259: Debug  0.00:00:00.910 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
260: Debug  0.00:00:00.910 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
261: Debug  0.00:00:00.910 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
262: Debug  0.00:00:00.920 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
263: Debug  0.00:00:00.920 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
264: Debug  0.00:00:00.920 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
265: Debug  0.00:00:00.930 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
266: Debug  0.00:00:00.930 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
267: Debug  0.00:00:00.930 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
268: Debug  0.00:00:00.930 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
269: Debug  0.00:00:00.940 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
270: Debug  0.00:00:00.940 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
271: Debug  0.00:00:00.940 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
272: Debug  0.00:00:00.950 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
273: Debug  0.00:00:00.950 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
274: Debug  0.00:00:00.950 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
275: Debug  0.00:00:00.960 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
276: Debug  0.00:00:00.960 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
277: Debug  0.00:00:00.960 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
278: Debug  0.00:00:00.970 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
279: Debug  0.00:00:00.970 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
280: Debug  0.00:00:00.970 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
281: Debug  0.00:00:00.970 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
282: Debug  0.00:00:00.980 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
283: Debug  0.00:00:00.980 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
284: Debug  0.00:00:00.980 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
285: Debug  0.00:00:00.990 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
286: Debug  0.00:00:00.990 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
287: Debug  0.00:00:00.990 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
288: Debug  0.00:00:01.000 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
289: Debug  0.00:00:01.000 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
290: Debug  0.00:00:01.000 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
291: Debug  0.00:00:01.010 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
292: Debug  0.00:00:01.010 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
293: Debug  0.00:00:01.010 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
294: Debug  0.00:00:01.010 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
295: Debug  0.00:00:01.020 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
296: Debug  0.00:00:01.020 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
297: Debug  0.00:00:01.020 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
298: Debug  0.00:00:01.030 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
299: Debug  0.00:00:01.030 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
300: Debug  0.00:00:01.030 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
301: Debug  0.00:00:01.040 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
302: Debug  0.00:00:01.040 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
303: Debug  0.00:00:01.040 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
304: Debug  0.00:00:01.040 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
305: Debug  0.00:00:01.050 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
306: Debug  0.00:00:01.050 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
307: Debug  0.00:00:01.050 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
308: Debug  0.00:00:01.060 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
309: Debug  0.00:00:01.060 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
310: Debug  0.00:00:01.060 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
311: Debug  0.00:00:01.070 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
312: Debug  0.00:00:01.070 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
313: Debug  0.00:00:01.070 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
314: Debug  0.00:00:01.080 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
315: Debug  0.00:00:01.080 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
316: Debug  0.00:00:01.080 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
317: Debug  0.00:00:01.080 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
318: Debug  0.00:00:01.090 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
319: Debug  0.00:00:01.090 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
320: Debug  0.00:00:01.090 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
321: Debug  0.00:00:01.100 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
322: Debug  0.00:00:01.100 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
323: Debug  0.00:00:01.100 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
324: Debug  0.00:00:01.110 GPIO Set Pin 8 ON (MemoryAccessStubGPIO:468)
325: Debug  0.00:00:01.110 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
326: Debug  0.00:00:01.110 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
327: Debug  0.00:00:01.110 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
328: Debug  0.00:00:01.120 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
329: Debug  0.00:00:01.120 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
330: Debug  0.00:00:01.120 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
331: Debug  0.00:00:01.130 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
332: Debug  0.00:00:01.130 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
333: Debug  0.00:00:01.130 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
334: Debug  0.00:00:01.140 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
335: Debug  0.00:00:01.140 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
336: Debug  0.00:00:01.140 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
337: Debug  0.00:00:01.150 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
338: Debug  0.00:00:01.150 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
339: Debug  0.00:00:01.150 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
340: Debug  0.00:00:01.160 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
341: Debug  0.00:00:01.160 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
342: Debug  0.00:00:01.160 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
343: Debug  0.00:00:01.160 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
344: Debug  0.00:00:01.170 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
345: Debug  0.00:00:01.170 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
346: Debug  0.00:00:01.170 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
347: Debug  0.00:00:01.180 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
348: Debug  0.00:00:01.180 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
349: Debug  0.00:00:01.180 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
350: Debug  0.00:00:01.190 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
351: Debug  0.00:00:01.190 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
352: Debug  0.00:00:01.190 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
353: Debug  0.00:00:01.190 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
354: Debug  0.00:00:01.200 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
355: Debug  0.00:00:01.200 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
356: Debug  0.00:00:01.210 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
357: Debug  0.00:00:01.210 GPIO Set Pin 9 ON (MemoryAccessStubGPIO:468)
358: Debug  0.00:00:01.210 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
359: Debug  0.00:00:01.210 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
360: Debug  0.00:00:01.220 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
361: Debug  0.00:00:01.220 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
362: Debug  0.00:00:01.220 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
363: Debug  0.00:00:01.230 GPIO Set Pin 13 ON (MemoryAccessStubGPIO:468)
364: Debug  0.00:00:01.230 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
365: Debug  0.00:00:01.230 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
366: Debug  0.00:00:01.240 GPIO Set Pin 10 ON (MemoryAccessStubGPIO:468)
367: Debug  0.00:00:01.240 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
368: Debug  0.00:00:01.240 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
369: Debug  0.00:00:01.250 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
370: Debug  0.00:00:01.250 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
371: Debug  0.00:00:03.240 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
372: Debug  0.00:00:03.240 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
373: Debug  0.00:00:03.240 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
374: Debug  0.00:00:03.250 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
375: Debug  0.00:00:03.250 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
376: Debug  0.00:00:03.250 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
377: Debug  0.00:00:03.250 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
378: Debug  0.00:00:03.260 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
379: Debug  0.00:00:03.260 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
380: Debug  0.00:00:03.260 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
381: Debug  0.00:00:03.270 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
382: Debug  0.00:00:03.270 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
383: Debug  0.00:00:03.270 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
384: Debug  0.00:00:03.270 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
385: Debug  0.00:00:03.280 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
386: Debug  0.00:00:03.280 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
387: Info   0.00:00:03.280 Rebooting (main:35)
388: Debug  0.00:00:03.290 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
389: Debug  0.00:00:03.290 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
390: Debug  0.00:00:03.290 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
391: Debug  0.00:00:03.290 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
392: Debug  0.00:00:03.290 GPIO Set Pin 11 OFF (MemoryAccessStubGPIO:484)
393: Debug  0.00:00:03.300 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
394: Debug  0.00:00:03.300 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
395: Debug  0.00:00:03.300 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
396: Debug  0.00:00:03.310 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
397: Debug  0.00:00:03.310 GPIO Set Pin 8 OFF (MemoryAccessStubGPIO:484)
398: Debug  0.00:00:03.310 GPIO Set Pin 9 OFF (MemoryAccessStubGPIO:484)
399: Debug  0.00:00:03.320 GPIO Set Pin 10 OFF (MemoryAccessStubGPIO:484)
400: Debug  0.00:00:03.320 GPIO Set Pin 11 ON (MemoryAccessStubGPIO:468)
401: Debug  0.00:00:03.320 GPIO Set Pin 12 ON (MemoryAccessStubGPIO:468)
402: Debug  0.00:00:03.330 GPIO Set Pin 12 OFF (MemoryAccessStubGPIO:484)
403: Debug  0.00:00:03.330 GPIO Set Pin 13 OFF (MemoryAccessStubGPIO:484)
404: Debug  0.00:00:03.330 GPIO Set Pin 8 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
405: Debug  0.00:00:03.330 GPIO Set Pin 8 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
406: Debug  0.00:00:03.340 GPIO Read Pin Mode  - Pin 0 mode Input - Pin 1 mode Input - Pin 2 mode Input - Pin 3 mode Input - Pin 4 mode Input - Pin 5 mode Input - Pin 6 mode Input - Pin 7 mode Input - Pin 8 mode Output - Pin 9 mode Output (MemoryAccessStubGPIO:216)
407: Debug  0.00:00:03.350 GPIO Set Pin 8 Mode Input (MemoryAccessStubGPIO:453)
408: Debug  0.00:00:03.350 GPIO Set Pin 9 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
409: Debug  0.00:00:03.350 GPIO Set Pin 9 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
410: Debug  0.00:00:03.360 GPIO Read Pin Mode  - Pin 0 mode Input - Pin 1 mode Input - Pin 2 mode Input - Pin 3 mode Input - Pin 4 mode Input - Pin 5 mode Input - Pin 6 mode Input - Pin 7 mode Input - Pin 8 mode Input - Pin 9 mode Output (MemoryAccessStubGPIO:216)
411: Debug  0.00:00:03.370 GPIO Set Pin 9 Mode Input (MemoryAccessStubGPIO:453)
412: Debug  0.00:00:03.370 GPIO Set Pin 10 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
413: Debug  0.00:00:03.370 GPIO Set Pin 10 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
414: Debug  0.00:00:03.380 GPIO Read Pin Mode  - Pin 10 mode Output - Pin 11 mode Output - Pin 12 mode Output - Pin 13 mode Output - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
415: Debug  0.00:00:03.390 GPIO Set Pin 10 Mode Input (MemoryAccessStubGPIO:453)
416: Debug  0.00:00:03.390 GPIO Set Pin 11 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
417: Debug  0.00:00:03.400 GPIO Set Pin 11 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
418: Debug  0.00:00:03.400 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Output - Pin 12 mode Output - Pin 13 mode Output - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
419: Debug  0.00:00:03.410 GPIO Set Pin 11 Mode Input (MemoryAccessStubGPIO:453)
420: Debug  0.00:00:03.410 GPIO Set Pin 12 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
421: Debug  0.00:00:03.410 GPIO Set Pin 12 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
422: Debug  0.00:00:03.420 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Input - Pin 12 mode Output - Pin 13 mode Output - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
423: Debug  0.00:00:03.430 GPIO Set Pin 12 Mode Input (MemoryAccessStubGPIO:453)
424: Debug  0.00:00:03.430 GPIO Set Pin 13 Pull Up/Down Enable Clock ON (MemoryAccessStubGPIO:652)
425: Debug  0.00:00:03.430 GPIO Set Pin 13 Pull Up/Down Enable Clock OFF (MemoryAccessStubGPIO:654)
426: Debug  0.00:00:03.440 GPIO Read Pin Mode  - Pin 10 mode Input - Pin 11 mode Input - Pin 12 mode Input - Pin 13 mode Output - Pin 14 mode Input - Pin 15 mode Input - Pin 16 mode Input - Pin 17 mode Input - Pin 18 mode Input - Pin 19 mode Input (MemoryAccessStubGPIO:216)
427: Debug  0.00:00:03.450 GPIO Set Pin 13 Mode Input (MemoryAccessStubGPIO:453)
428: Info   0.00:00:03.450 Reboot (System:154)
429: Info   0.00:00:03.450 InterruptSystem::Shutdown (InterruptHandler:153)
430: Debug  0.00:00:03.460 InterruptSystem::DisableInterrupts (InterruptHandler:166)
```

- Line 3-32: The GPIO pins are assigned and set to output mode, then set to off
- Line 33-34: Both GPIO 12 (E pin) and GPIO 13 (RS pin) are set to OFF in the constructor of `HD44780DeviceRaw`
- Line 35-42: The first write of 4 bits is done, resulting from the initialization sequence in `HD44780Device::Initialize()`.
This writes 4 bits 0x03 to the display controller, which is passed as data 0x30.
First GPIO 13 (RS pin) is made low.
Then the 4 data bits are set with GPIO8 = ON, GPIO9 = ON, GPIO10 = OFF, GPIO11 = OFF.
Then GPIO 12 (E pin) is made high for some time, then low again.
Subsequently GPIO 13 (RS pin) is made low
- Line 43-50: Again the 4 bits 0x03 are written
- Line 51-58: The 4 bits 0x03 are written for the third time
- Line 59-66: The 4 bits 0x02 are written
- Line 67-82: The 4 bits 0x02 are written followed by 0x08, making for the 8 bits 0x28.
This is the first command sent which is Function Set, with the number of display lines being more than 1
- Line 83-98: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF
- Line 99-114: The 8 bits 0x01 are written, meaning Clear Display
- Line 115-130: The 8 bits 0x06 are written, meaning Entry Mode Set with incrementing position, and moving cursor instead of shifting display.
This is the last command sent by the `Initialize()` method
- Line 131-146: The 8 bits 0x0C are written, meaning Display On/Off Control with the display set to ON
This is caused the line in the application code switching the display on
- Line 147-162: The 8 bits 0x0C are written, meaning again Display On/Off Control with the display set to ON, this is caused by the line setting the cursor to hidden
- Line 163-178: The 8 bits 0x01 are written, meaning again Clear Display
- Line 179-194: The 8 bits 0x02 are written, meaning Return Home
- Line 195-210: The 8 bits 0x48 are written, but this time as data, as GPIO 13 (RS pin) is made high. This means the character 0x48='H' is written
- Line 211-226: The 8 bits 0x65 are written, which means the character 'e' is written
- Line 227-370: The other characters are written
- Line 371-386: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF
- Line 388-403: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF.
This is caused by the destructor of `HD44780DeviceRaw`
- Line 404-427: The GPIO pins are switched back to input mode

## I2C operation {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION}

For displays with a I2C piggyback board, we will be using the I2C interface.

### HD44780DeviceI2C.h {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_HD44780DEVICEI2CH}

We'll declare a new class `HD44780DeviceI2C` which derives from `HD44780Device` for implementation using I2C control.
As this is I2C related functionality, we'll place the header and source file under the `i2c` folder.

Create the file `code/libraries/device/include/device/i2c/HD44780DeviceI2C.h`

```cpp
File: code/libraries/device/include/device/i2c/HD44780DeviceI2C.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780DeviceI2C.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780DeviceI2C
9: //
10: // Description : HD44780 based 16x2 LCD display with I2C piggyback
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
42: #include <device/display/HD44780Device.h>
43: 
44: /// @file
45: /// HD44780 based I2C LCD character display (max 40x4 characters)
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
56: /// I2C controlled HD44780 based LCD display
57: /// </summary>
58: class HD44780DeviceI2C
59:     : public HD44780Device
60: {
61: private:
62:     /// @brief I2C master interface
63:     baremetal::II2CMaster& m_i2cMaster;
64:     /// @brief I2C address of the LCD controller
65:     uint8                  m_address;
66:     /// @brief Backlight status
67:     bool                   m_backlightOn;
68: 
69: public:
70:     HD44780DeviceI2C(baremetal::II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows,
71:                      CharacterSize characterSize = CharacterSize::Size5x8);
72: 
73:     ~HD44780DeviceI2C();
74: 
75:     void SetBacklight(bool on) override;
76:     bool IsBacklightOn() const override;
77: 
78: protected:
79:     void WriteHalfByte(uint8 data) override;
80: };
81: 
82: } // namespace device
```

- Line 58-80: We delcare the class `HD44780DeviceI2C`
  - Line 62-63: We declare a member variable `m_i2cMaster` to hold a reference to the II2CMaster interface injected through the constructor
  - Line 64-65: We declare a member variable `m_address` to hold the I2C address of the display controller
  - Line 66-67: We declare a member variable `m_backlightOn` to keep the status of the backlight
  - Line 70-71: We declare the constructor
  - Line 73: We declare the destructor
  - Line 75: We override the method `SetBacklight()`
  - Line 76: We override the method `IsBacklightOn()`
  - Line 79: We override the method `WriteHalfByte()`

### HD44780DeviceI2C.cpp {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_HD44780DEVICEI2CCPP}

We'll implement the class `HD44780DeviceI2C`.

Create the file `code/libraries/device/src/i2c/HD44780DeviceI2C.cpp`

```cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\device\src\i2c\HD44780DeviceI2C.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : HD44780DeviceI2C.h
5: //
6: // Namespace   : baremetal::display
7: //
8: // Class       : HD44780DeviceI2C
9: //
10: // Description : HD44780 based 16x2 LCD display with I2C piggyback
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
40: #include <device/i2c/HD44780DeviceI2C.h>
41: 
42: #include <baremetal/I2CMaster.h>
43: #include <baremetal/Timer.h>
44: 
45: using namespace baremetal;
46: 
47: /// @file
48: /// HD44780 based I2C 16x2 LCD display
49: 
50: namespace device {
51: 
52: // CharLCD via PCF8574 I2C port expander (for 16X2 LCD display
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
95: /// <param name="numColumns">    Display size in number of columns (max. 40)</param>
96: /// <param name="numRows">       Display size in number of rows (max. 4)</param>
97: /// <param name="characterSize">Character size, either 5x8 pixels (default) or 5x10 pixels</param>
98: HD44780DeviceI2C::HD44780DeviceI2C(II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows,
99:                                    CharacterSize characterSize /*= CharacterSize::Size5x8*/)
100:     : HD44780Device(numColumns, numRows, characterSize, DataMode::Mode4Bit)
101:     , m_i2cMaster(i2cMaster)
102:     , m_address(address)
103:     , m_backlightOn{}
104: {
105: }
106: 
107: /// <summary>
108: /// Destructor
109: ///
110: /// Resets device back to 8 bit interface
111: /// </summary>
112: HD44780DeviceI2C::~HD44780DeviceI2C()
113: {
114:     SetBacklight(false);
115:     DisplayControl(DisplayMode::Hide, CursorMode::Hide);
116: }
117: 
118: /// <summary>
119: /// Switch backlight on or off
120: /// </summary>
121: /// <param name="on">If true, backlight is switched on, otherwise it is off</param>
122: void HD44780DeviceI2C::SetBacklight(bool on)
123: {
124:     if (on != m_backlightOn)
125:     {
126:         uint8 byte = (on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
127:         // We write a single byte with all other bits off. This will have no effect to the controller state, except for the backlight
128:         m_i2cMaster.Write(m_address, &byte, 1);
129:         m_backlightOn = on;
130:     }
131: }
132: 
133: /// <summary>
134: /// Returns backlight on / off status
135: /// </summary>
136: /// <returns>True if backlight is on, false otherwise</returns>
137: bool HD44780DeviceI2C::IsBacklightOn() const
138: {
139:     return m_backlightOn;
140: }
141: 
142: /// <summary>
143: /// Write a 4 bit value
144: ///
145: /// The most significant 4 bits are the data written, bit 0 acts as the register select bit (0 = instruction, 1 = data)
146: /// </summary>
147: /// <param name="data">Value to write</param>
148: void HD44780DeviceI2C::WriteHalfByte(uint8 data)
149: {
150:     uint8 byte = data | (m_backlightOn ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
151:     // Pulse the `enable` flag to process value.
152:     uint8 value = byte | PCF_EN;
153:     m_i2cMaster.Write(m_address, &value, 1);
154:     Timer::WaitMicroSeconds(1);
155:     value = byte & ~PCF_EN;
156:     m_i2cMaster.Write(m_address, &value, 1);
157:     // Wait for command to complete.
158:     Timer::WaitMicroSeconds(100);
159: }
160: 
161: } // namespace device
```

- Line 67-68: We create a definition of the RS bit as part of the byte to be written
- Line 69-70: We create a definition of the RW bit as part of the byte to be written
- Line 71-72: We create a definition of the E/EN bit as part of the byte to be written
- Line 73-74: We create a definition of the BK (backlight) bit as part of the byte to be written
- Line 77-78: We create a definition of the value `RS_INSTRUCTION` for a command or instruction for the display controller (this has RS bit 0)
- Line 79-80: We create a definition of the value `RS_DATA` for data for the display controller (this has RS bit 1)
- Line 83-84: We create a definition of the value `LCD_BACKLIGHT` for switching the backlight on (this has BK bit 1)
- Line 85-86: We create a definition of the value `LCD_NOBACKLIGHT` for switching the backlight off (this has BK bit 0)
- Line 88-105: We implement the constructor
- Line 107-116: We implement the destructor.
This will switch off the display and the backlight
- Line 118-131: We implement `SetBacklight()`.
This writes the value of the backlight bit (BK). As we do not change the value of the E/EN bit, this other bits are don't care
- Line 133-140: We implement `IsBacklightOn()`
- Line 142-159: We implement `WriteHalfByte()`.
This sets the backlight bit if backlight should be on, and then sets the E/EN bit to 1, then back to 0

### MemoryAccessStubI2C.cpp {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_MEMORYACCESSSTUBI2CCPP}

Similar to what we did in the GPIO MemoryAccess stub, we will use externally set definitions to control logging.

Update the file `code/libraries/baremetal/src/stubs/MemoryAccessStubI2C.cpp`.

```cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\baremetal\src\stubs\MemoryAccessStubI2C.cpp
...
56: /// @brief I2C bus 0 register base address
57: static regaddr I2CBaseAddress0{ RPI_I2C0_BASE };
...
169: /// <summary>
170: /// Read a 32 bit value from register at address
171: /// </summary>
172: /// <param name="address">Address of register</param>
173: /// <returns>32 bit register value</returns>
174: uint32 MemoryAccessStubI2C::Read32(regaddr address)
175: {
176:     uintptr addr = reinterpret_cast<uintptr>(address);
177:     if ((addr >= RPI_GPIO_BASE) && (addr < RPI_GPIO_END))
178:     {
179:         return MemoryAccessStubGPIO::Read32(address);
180:     }
181: 
182:     uintptr offset = GetRegisterOffset(address);
183:     uint32 *registerField = reinterpret_cast<uint32 *>(reinterpret_cast<uint8 *>(&m_registers) + offset);
184: #if BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL
185:     LOG_DEBUG("I2C Read register %016x = %08x", offset, *registerField);
186: #endif
187:     switch (offset)
188:     {
189:         case RPI_I2C_C_OFFSET:
190:         {
191: #if BAREMETAL_MEMORY_ACCESS_TRACING
192:             string line{ "I2C Read Control Register "};
193:             if (*registerField & RPI_I2C_C_ENABLE)
194:                 line += "Enable ON  ";
195:             else
196:                 line += "Enable OFF ";
197:             if (*registerField & RPI_I2C_C_INTR_ENABLE)
198:                 line += "RX Interrupt ON  ";
199:             else
200:                 line += "RX Interrupt OFF ";
201:             if (*registerField & RPI_I2C_C_INTT_ENABLE)
202:                 line += "TX Interrupt ON  ";
203:             else
204:                 line += "TX Interrupt OFF ";
205:             if (*registerField & RPI_I2C_C_INTD_ENABLE)
206:                 line += "DONE Interrupt ON  ";
207:             else
208:                 line += "DONE Interrupt OFF ";
209:             if (*registerField & RPI_I2C_C_READ)
210:                 line += "Read ";
211:             else
212:                 line += "Write ";
213:             LOG_DEBUG(line.c_str());
214: #endif
215:             break;
216:         }
217:         case RPI_I2C_S_OFFSET:
218:         {
219: #if BAREMETAL_MEMORY_ACCESS_TRACING
220:             string line{ "I2C Read Status Register "};
221:             if (*registerField & RPI_I2C_S_CLKT)
222:                 line += "CLKT ";
223:             else
224:                 line += "     ";
225:             if (*registerField & RPI_I2C_S_ERR)
226:                 line += "NACK ";
227:             else
228:                 line += "ACK  ";
229:             if (*registerField & RPI_I2C_S_RXF)
230:                 line += "RXF ";
231:             else
232:                 line += "    ";
233:             if (*registerField & RPI_I2C_S_TXE)
234:                 line += "TXE ";
235:             else
236:                 line += "    ";
237:             if (*registerField & RPI_I2C_S_RXD)
238:                 line += "RXD ";
239:             else
240:                 line += "    ";
241:             if (*registerField & RPI_I2C_S_TXD)
242:                 line += "TXD ";
243:             else
244:                 line += "    ";
245:             if (*registerField & RPI_I2C_S_RXR)
246:                 line += "RXR ";
247:             else
248:                 line += "    ";
249:             if (*registerField & RPI_I2C_S_TXW)
250:                 line += "TXW ";
251:             else
252:                 line += "    ";
253:             if (*registerField & RPI_I2C_S_DONE)
254:                 line += "DONE ";
255:             else
256:                 line += "     ";
257:             if (*registerField & RPI_I2C_S_TA)
258:                 line += "TA ";
259:             else
260:                 line += "   ";
261:             LOG_DEBUG(line.c_str());
262: #endif
263:             break;
264:         }
265:         case RPI_I2C_DLEN_OFFSET:
266:         {
267: #if BAREMETAL_MEMORY_ACCESS_TRACING
268:             LOG_DEBUG("I2C Get Data Length %d", *registerField);
269: #endif
270:             break;
271:         }
272:         case RPI_I2C_A_OFFSET:
273:         {
274: #if BAREMETAL_MEMORY_ACCESS_TRACING
275:             LOG_DEBUG("I2C Get Address %02x", *registerField);
276: #endif
277:             break;
278:         }
279:         case RPI_I2C_FIFO_OFFSET:
280:         {
281: #if BAREMETAL_MEMORY_ACCESS_TRACING
282:             LOG_DEBUG("I2C Read FIFO %02x", *registerField);
283: #endif
284:             *registerField = HandleReadFIFORegister();
285:             break;
286:         }
287:         case RPI_I2C_DIV_OFFSET:
288:         {
289: #if BAREMETAL_MEMORY_ACCESS_TRACING
290:             LOG_DEBUG("I2C Set Clock Divider %d", *registerField);
291: #endif
292:             break;
293:         }
294:         case RPI_I2C_DEL_OFFSET:
295:         {
296: #if BAREMETAL_MEMORY_ACCESS_TRACING
297:             if (*registerField & 0xFFFF0000)
298:             {
299:                 LOG_DEBUG("I2C Get Falling Edge Delay %d", (*registerField >> 16));
300:             }
301:             if (*registerField & 0x0000FFFF)
302:             {
303:                 LOG_DEBUG("I2C Get Rising Edge Delay %d", (*registerField & 0x0000FFFF));
304:             }
305: #endif
306:             break;
307:         }
308:         case RPI_I2C_CLKT_OFFSET:
309:         {
310: #if BAREMETAL_MEMORY_ACCESS_TRACING
311:             LOG_DEBUG("I2C Get Clock Stretch Timeout %d", (*registerField & 0x0000FFFF));
312: #endif
313:             break;
314:         }
315:         default:
316:             LOG_ERROR("Invalid I2C register access for reading: offset %d", offset);
317:             break;
318:     }
319: 
320:     return *registerField;
321: }
322: 
323: /// <summary>
324: /// Write a 32 bit value to register at address
325: /// </summary>
326: /// <param name="address">Address of register</param>
327: /// <param name="data">Data to write</param>
328: void MemoryAccessStubI2C::Write32(regaddr address, uint32 data)
329: {
330:     uintptr addr = reinterpret_cast<uintptr>(address);
331:     if ((addr >= RPI_GPIO_BASE) && (addr < RPI_GPIO_END))
332:     {
333:         MemoryAccessStubGPIO::Write32(address, data);
334:         return;
335:     }
336: 
337:     uintptr offset = GetRegisterOffset(address);
338:     uint32 *registerField = reinterpret_cast<uint32 *>(reinterpret_cast<uint8 *>(&m_registers) + offset);
339: #if BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL
340:     LOG_DEBUG("I2C Write register %016x = %08x", offset, data);
341: #endif
342:     switch (offset)
343:     {
344:         case RPI_I2C_C_OFFSET:
345:         {
346: #if BAREMETAL_MEMORY_ACCESS_TRACING
347:             uint32 diff = (data ^ *registerField) | 0x00B0;
348:             if (diff & RPI_I2C_C_ENABLE)
349:             {
350:                 if (data & RPI_I2C_C_ENABLE)
351:                     LOG_DEBUG("I2C Enable Controller");
352:                 else
353:                     LOG_DEBUG("I2C Disable Controller");
354:             }
355:             if (diff & RPI_I2C_C_INTR_ENABLE)
356:             {
357:                 if (data & RPI_I2C_C_INTR_ENABLE)
358:                     LOG_DEBUG("I2C Enable RX Interrupt");
359:                 else
360:                     LOG_DEBUG("I2C Disable RX Interrupt");
361:             }
362:             if (diff & RPI_I2C_C_INTT_ENABLE)
363:             {
364:                 if (data & RPI_I2C_C_INTT_ENABLE)
365:                     LOG_DEBUG("I2C Enable TX Interrupt");
366:                 else
367:                     LOG_DEBUG("I2C Disable TX Interrupt");
368:             }
369:             if (diff & RPI_I2C_C_INTD_ENABLE)
370:             {
371:                 if (data & RPI_I2C_C_INTD_ENABLE)
372:                     LOG_DEBUG("I2C Enable Done Interrupt");
373:                 else
374:                     LOG_DEBUG("I2C Disable Done Interrupt");
375:             }
376:             if (diff & RPI_I2C_C_READ)
377:             {
378:                 if (data & RPI_I2C_C_READ)
379:                     LOG_DEBUG("I2C Read Mode");
380:                 else
381:                     LOG_DEBUG("I2C Write Mode");
382:             }
383:             if (data & RPI_I2C_C_CLEAR)
384:                 LOG_DEBUG("I2C Clear FIFO");
385:             if (data & RPI_I2C_C_ST)
386:                 LOG_DEBUG("I2C Start Transfer");
387: #endif
388:             HandleWriteControlRegister(data);
389:             break;
390:         }
391:         case RPI_I2C_S_OFFSET:
392:         {
393: #if BAREMETAL_MEMORY_ACCESS_TRACING
394:             if (data & RPI_I2C_S_CLKT)
395:                 LOG_DEBUG("I2C Reset Clock Stretch Timeout");
396:             if (data & RPI_I2C_S_ERR)
397:                 LOG_DEBUG("I2C Reset Ack Error");
398:             if (data & RPI_I2C_S_DONE)
399:                 LOG_DEBUG("I2C Reset Done");
400: #endif
401:             HandleWriteStatusRegister(data);
402:             break;
403:         }
404:         case RPI_I2C_DLEN_OFFSET:
405:         {
406: #if BAREMETAL_MEMORY_ACCESS_TRACING
407:             LOG_DEBUG("I2C Set Data Length %d", data);
408: #endif
409:             *registerField = data;
410:             break;
411:         }
412:         case RPI_I2C_A_OFFSET:
413:         {
414: #if BAREMETAL_MEMORY_ACCESS_TRACING
415:             LOG_DEBUG("I2C Set Address %02x", data);
416: #endif
417:             *registerField = data;
418:             break;
419:         }
420:         case RPI_I2C_FIFO_OFFSET:
421:         {
422: #if BAREMETAL_MEMORY_ACCESS_TRACING
423:             LOG_DEBUG("I2C Write FIFO %02x", data);
424: #endif
425:             HandleWriteFIFORegister(data);
426:             break;
427:         }
428:         case RPI_I2C_DIV_OFFSET:
429:         {
430: #if BAREMETAL_MEMORY_ACCESS_TRACING
431:             if (data != *registerField)
432:             {
433:                 LOG_DEBUG("I2C Set Clock Divider %d", data);
434:             }
435: #endif
436:             *registerField = data;
437:             break;
438:         }
439:         case RPI_I2C_DEL_OFFSET:
440:         {
441: #if BAREMETAL_MEMORY_ACCESS_TRACING
442:             uint32 diff = data & *registerField;
443:             if (diff & 0xFFFF0000)
444:             {
445:                 LOG_DEBUG("I2C Set Falling Edge Delay %d", (data >> 16));
446:             }
447:             if (diff & 0x0000FFFF)
448:             {
449:                 LOG_DEBUG("I2C Set Rising Edge Delay %d", (data & 0x0000FFFF));
450:             }
451: #endif
452:             *registerField = data;
453:             break;
454:         }
455:         case RPI_I2C_CLKT_OFFSET:
456:         {
457: #if BAREMETAL_MEMORY_ACCESS_TRACING
458:             uint32 diff = data & *registerField;
459:             if (diff & 0x0000FFFF)
460:             {
461:                 LOG_DEBUG("I2C Set Clock Stretch Timeout %d", (data & 0x0000FFFF));
462:             }
463: #endif
464:             *registerField = data;
465:             break;
466:         }
467:         default:
468:             LOG_ERROR("Invalid I2C register access for writing: offset %d", offset);
469:             break;
470:     }
471: }
472: 
473: /// <summary>
474: /// Set callback function for address send
475: /// </summary>
476: /// <param name="callback">Address of callback function</param>
477: void MemoryAccessStubI2C::SetSendAddressByteCallback(SendAddressByteCallback callback)
478: {
479:     m_sendAddressByteCallback = callback;
480: }
...
```

- Line 56-57: We remove the definition for `TRACE`
- Line 184: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` for logging the register value
- Line 191: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 219: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 267: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 274: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 281: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 289: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 296: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 310: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 339: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL` for logging the register value
- Line 346: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 393: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 406: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 414: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 422: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 430: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 441: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 457: We replace usage of `TRACE` by `BAREMETAL_MEMORY_ACCESS_TRACING`

### I2CMaster.cpp {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_I2CMASTERCPP}

Similar to what we just did in the I2C MemoryAccess stub, we will use a define to control tracing in `I2CMaster`.

Update the file `code/libraries/baremetal/src/I2CMaster.cpp`.

```cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\baremetal\src\I2CMaster.cpp
...
56: #if BAREMETAL_RPI_TARGET == 3
57: /// @brief Number of I2C buses for RPI 3
58: #define I2C_BUSES           2
59: #else
60: /// @brief Number of I2C buses for RPI 4
61: #define I2C_BUSES           7
62: #endif
...
240: /// <summary>
241: /// Read a requested number of bytes from the I2C device with specified address
242: /// </summary>
243: /// <param name="address">Device address</param>
244: /// <param name="buffer">Pointer to buffer to store data read</param>
245: /// <param name="count">Requested number of bytes to read</param>
246: /// <returns>Returns number of bytes read, or errorcode < 0 on failure</returns>
247: size_t I2CMaster::Read(uint16 address, void *buffer, size_t count)
248: {
249:     assert(m_isInitialized);
250: 
251:     if ((address >= ADDRESS_10BIT_HIGH) || (count == 0))
252:     {
253:         return -I2C_MASTER_INVALID_PARM;
254:     }
255:     if (address >= RESERVED_ADDRESS_HIGH)
256:     {
257:         // We need to start a write / read cycle
258:         // The address part contains bits 8 and 9 of the address, the first write data byte the bits 0 through 7 of the address.
259:         // After this we read the device
260:         uint8 secondAddressByte = address & 0xFF;
261:         uint8 firstAddressByte = static_cast<uint8>(0x78 | ((address >> 8) & 0x03));
262:         return WriteReadRepeatedStart(firstAddressByte, &secondAddressByte, 1, buffer, count);
263:     }
264:     uint8 addressByte{ static_cast<uint8>(address & 0x7F) };
265: 
266:     uint8 *data = reinterpret_cast<uint8 *>(buffer);
267:     assert(data != 0);
268: 
269:     size_t result = 0;
270: 
271:     WriteAddressRegister(addressByte);
272: 
273:     ClearFIFO();
274:     ClearAllStatus();
275:     WriteDataLengthRegister(count);
276:     StartReadTransfer();
277: 
278:     // Transfer active
279:     while (!TransferDone())
280:     {
281:         while (ReceiveFIFOHasData())
282:         {
283:             *data++ = ReadFIFORegister();
284:             //LOG_INFO("Read byte from %02x, data %02x", address, *(data - 1));
285: 
286:             count--;
287:             result++;
288:         }
289:     }
290: 
291:     // Transfer has finished, grab any remaining stuff from FIFO
292:     while ((count > 0) && ReceiveFIFOHasData())
293:     {
294:         *data++ = ReadFIFORegister();
295:         //LOG_INFO("Read extra byte from %02x, data %02x", address, *(data - 1));
296: 
297:         count--;
298:         result++;
299:     }
300: 
301: #if BAREMETAL_I2C_TRACING_DETAIL
302:     LOG_INFO("Read result = %d", result);
303: #endif
304: 
305:     uint32 status = ReadStatusRegister();
306:     if (status & RPI_I2C_S_ERR)
307:     {
308:         // Clear error bit
309:         ClearNAck();
310: 
311:         result = -I2C_MASTER_ERROR_NACK;
312:     }
313:     else if (status & RPI_I2C_S_CLKT)
314:     {
315:         ClearClockStretchTimeout();
316:         result = -I2C_MASTER_ERROR_CLKT;
317:     }
318:     else if (count > 0)
319:     {
320:         result = -I2C_MASTER_DATA_LEFT;
321:     }
322: 
323:     // Clear done bit
324:     ClearDone();
325: 
326:     return result;
327: }
... 
340: /// <summary>
341: /// Write a requested number of bytes to the I2C device with specified address
342: /// </summary>
343: /// <param name="address">Device address</param>
344: /// <param name="buffer">Pointer to buffer containing data to sebd</param>
345: /// <param name="count">Requested number of bytes to write</param>
346: /// <returns>Returns number of bytes written, or errorcode < 0 on failure</returns>
347: size_t I2CMaster::Write(uint16 address, const void *buffer, size_t count)
348: {
349:     assert(m_isInitialized);
350: 
351:     if (address >= ADDRESS_10BIT_HIGH)
352:     {
353:         return -I2C_MASTER_INVALID_PARM;
354:     }
355: 
356:     if ((count != 0) && (buffer == nullptr))
357:     {
358:         return -I2C_MASTER_INVALID_PARM;
359:     }
360: 
361:     const uint8 *data = reinterpret_cast<const uint8 *>(buffer);
362:     assert(data != 0);
363: 
364:     int result = 0;
365: 
366:     ClearFIFO();
367: 
368:     unsigned bytesWritten{};
369:     uint8 addressByte{ static_cast<uint8>(address & 0x7F) };
370:     if (address >= RESERVED_ADDRESS_HIGH)
371:     {
372:         // We need to write the low 8 bits of the address first
373:         // The address part contains bits 8 and 9 of the address, the first write data byte the bits 0 through 7 of the address.
374:         uint8 secondAddressByte = address & 0xFF;
375:         addressByte = static_cast<uint8>(0x78 | ((address >> 8) & 0x03));
376:         WriteFIFORegister(secondAddressByte);
377:         bytesWritten++;
378:     }
379: 
380:     WriteAddressRegister(addressByte);
381:     ClearAllStatus();
382:     WriteDataLengthRegister(static_cast<uint8>(bytesWritten + count));
383: 
384:     // Fill FIFO
385:     for (; count > 0 && bytesWritten < RPI_I2C_FIFO_SIZE; bytesWritten++)
386:     {
387:         WriteFIFORegister(*data++);
388:         count--;
389:         result++;
390:     }
391: 
392:     StartWriteTransfer();
393: 
394:     // Transfer active
395:     while (!TransferDone())
396:     {
397:         while ((count > 0) && TransmitFIFOHasSpace())
398:         {
399:             //LOG_INFO("Write extra byte to %02x, data %02x", address, *data);
400:             WriteFIFORegister(*data++);
401:             count--;
402:             result++;
403:         }
404:         Timer::WaitMilliSeconds(1);
405:     }
406: 
407: #if BAREMETAL_I2C_TRACING_DETAIL
408:     LOG_INFO("Write result = %d", result);
409: #endif
410: 
411:     // Check status
412:     uint32 status = ReadStatusRegister();
413:     if (status & RPI_I2C_S_ERR)
414:     {
415:         // Clear error bit
416:         ClearNAck();
417: 
418:         result = -I2C_MASTER_ERROR_NACK;
419:     }
420:     else if (status & RPI_I2C_S_CLKT)
421:     {
422:         ClearClockStretchTimeout();
423:         result = -I2C_MASTER_ERROR_CLKT;
424:     }
425:     else if (count > 0)
426:     {
427:         result = -I2C_MASTER_DATA_LEFT;
428:     }
429: 
430:     // Clear done bit
431:     ClearDone();
432: 
433:     while (ReceiveFIFOHasData())
434:     {
435:         uint8 data = ReadFIFORegister();
436: #if BAREMETAL_I2C_TRACING
437:         LOG_INFO("Read byte = %02x", data);
438: #endif
439:     }
440: 
441:     return result;
442: }
...
565: /// <summary>
566: /// Read the I2C Control Register
567: /// </summary>
568: /// <returns>Value read from I2C Control Register</returns>
569: uint32 I2CMaster::ReadControlRegister()
570: {
571:     auto result = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_C_OFFSET));
572: #if BAREMETAL_I2C_TRACING_DETAIL
573:     string text;
574:     text += (result & RPI_I2C_C_ENABLE) ? "EN " : "   ";
575:     text += (result & RPI_I2C_C_INTR_ENABLE) ? "IR " : "  ";
576:     text += (result & RPI_I2C_C_INTT_ENABLE) ? "IT " : "  ";
577:     text += (result & RPI_I2C_C_INTD_ENABLE) ? "ID " : "  ";
578:     text += (result & RPI_I2C_C_ST) ? "ST " : "  ";
579:     text += (result & RPI_I2C_C_CLEAR) ? "CL " : "  ";
580:     text += (result & RPI_I2C_C_READ) ? "RD " : "WR ";
581:     LOG_INFO("Read I2C Control, %s", text.c_str());
582: #endif
583:     return result;
584: }
585: 
586: /// <summary>
587: /// Write to the I2C Control Register
588: /// </summary>
589: /// <param name="data">Value to write</param>
590: void I2CMaster::WriteControlRegister(uint32 data)
591: {
592:     m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_C_OFFSET), data);
593: #if BAREMETAL_I2C_TRACING_DETAIL
594:     string text;
595:     text += (data & RPI_I2C_C_ENABLE) ? "EN " : "   ";
596:     text += (data & RPI_I2C_C_INTR_ENABLE) ? "IR " : "  ";
597:     text += (data & RPI_I2C_C_INTT_ENABLE) ? "IT " : "  ";
598:     text += (data & RPI_I2C_C_INTD_ENABLE) ? "ID " : "  ";
599:     text += (data & RPI_I2C_C_ST) ? "ST " : "  ";
600:     text += (data & RPI_I2C_C_CLEAR) ? "CL " : "  ";
601:     text += (data & RPI_I2C_C_READ) ? "RD " : "WR ";
602:     LOG_INFO("Write I2C Control, %s", text.c_str());
603: #endif
604: }
...
630: /// <summary>
631: /// Write to the I2C Address Register
632: /// </summary>
633: /// <param name="data">Value to write</param>
634: void I2CMaster::WriteAddressRegister(uint8 data)
635: {
636:     m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_A_OFFSET), data);
637: #if BAREMETAL_I2C_TRACING_DETAIL
638:     LOG_INFO("Write I2C Address, %02x", data);
639: #endif
640: }
641: 
642: /// <summary>
643: /// Write to the I2C Data Length Register
644: /// </summary>
645: /// <param name="data">Value to write</param>
646: void I2CMaster::WriteDataLengthRegister(uint8 data)
647: {
648:     m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_DLEN_OFFSET), data);
649: #if BAREMETAL_I2C_TRACING_DETAIL
650:     LOG_INFO("Write I2C Length, %08x", data);
651: #endif
652: }
653: 
654: /// <summary>
655: /// Read the I2C Status Register
656: /// </summary>
657: /// <returns>Value read from I2C Status Register</returns>
658: uint32 I2CMaster::ReadStatusRegister()
659: {
660:     auto data = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_S_OFFSET));
661: #if BAREMETAL_I2C_TRACING_DETAIL
662:     string text;
663:     text += (data & RPI_I2C_S_CLKT) ? "CLKT " : "     ";
664:     text += (data & RPI_I2C_S_ERR) ? "ERR " : "    ";
665:     text += (data & RPI_I2C_S_RXF) ? "RXF " : "    ";
666:     text += (data & RPI_I2C_S_TXE) ? "TXE " : "    ";
667:     text += (data & RPI_I2C_S_RXD) ? "RXD " : "    ";
668:     text += (data & RPI_I2C_S_TXD) ? "TXD " : "    ";
669:     text += (data & RPI_I2C_S_RXR) ? "RXR " : "    ";
670:     text += (data & RPI_I2C_S_TXW) ? "TXW " : "    ";
671:     text += (data & RPI_I2C_S_DONE) ? "DONE " : "     ";
672:     text += (data & RPI_I2C_S_TA) ? "TA " : "     ";
673:     LOG_INFO("Read I2C Status, %s", text.c_str());
674: #endif
675:     return data;
676: }
677: 
678: /// <summary>
679: /// Write to the I2C Status Register
680: /// </summary>
681: /// <param name="data">Value to write</param>
682: void I2CMaster::WriteStatusRegister(uint32 data)
683: {
684:     m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_S_OFFSET), data);
685: #if BAREMETAL_I2C_TRACING_DETAIL
686:     string text;
687:     text += (data & RPI_I2C_S_CLKT) ? "CLKT " : "     ";
688:     text += (data & RPI_I2C_S_ERR) ? "ERR " : "    ";
689:     text += (data & RPI_I2C_S_DONE) ? "DONE " : "     ";
690:     LOG_INFO("Write I2C Status, %s", text.c_str());
691: #endif
692: }
...
825: /// <summary>
826: /// Read the I2C FIFO Register
827: /// </summary>
828: /// <returns>Value read from I2C FIFO Register</returns>
829: uint8 I2CMaster::ReadFIFORegister()
830: {
831:     uint8 data = m_memoryAccess.Read32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_FIFO_OFFSET)) & RPI_I2C_FIFO_MASK;
832: #if BAREMETAL_I2C_TRACING
833:     LOG_INFO("Read FIFO, data %08x", data);
834: #endif
835:     return data;
836: }
837: 
838: /// <summary>
839: /// Write to the I2C FIFO Register
840: /// </summary>
841: /// <param name="data">Value to write</param>
842: void I2CMaster::WriteFIFORegister(uint8 data)
843: {
844:     m_memoryAccess.Write32(RPI_I2C_REG_ADDRESS(m_baseAddress, RPI_I2C_FIFO_OFFSET), data);
845: #if BAREMETAL_I2C_TRACING
846:     LOG_INFO("Write FIFO, data %08x", data);
847: #endif
848: }
```

### Main CMake File {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_MAIN_CMAKE_FILE}

Now let's update the main CMake file to define `BAREMETAL_I2C_TRACING` and `BAREMETAL_I2C_TRACING_DETAIL` when desired.

Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
69: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
70: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
71: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
72: option(BAREMETAL_TRACE_MEMORY_ACCESS_STUBS "Enable memory access tracing output on stubs" OFF)
73: option(BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL "Enable detailed memory access tracing output on stubs" OFF)
74: option(BAREMETAL_TRACE_I2C "Enable I2C tracing output" OFF)
75: option(BAREMETAL_TRACE_I2C_DETAIL "Enable detailed I2C tracing output" OFF)
...
124: if (BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL)
125:     set(BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL 1)
126: else ()
127:     set(BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL 0)
128: endif()
129: if (BAREMETAL_TRACE_I2C)
130:     set(BAREMETAL_I2C_TRACING 1)
131: else ()
132:     set(BAREMETAL_I2C_TRACING 0)
133: endif()
134: if (BAREMETAL_TRACE_I2C_DETAIL)
135:     set(BAREMETAL_I2C_TRACING_DETAIL 1)
136: else ()
137:     set(BAREMETAL_I2C_TRACING_DETAIL 0)
138: endif()
139: 
140: set(BAREMETAL_LOAD_ADDRESS 0x80000)
141: 
142: set(DEFINES_C
143:     PLATFORM_BAREMETAL
144:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
145:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
146:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
147:     BAREMETAL_MEMORY_TRACING=${BAREMETAL_MEMORY_TRACING}
148:     BAREMETAL_MEMORY_TRACING_DETAIL=${BAREMETAL_MEMORY_TRACING_DETAIL}
149:     BAREMETAL_MEMORY_ACCESS_TRACING=${BAREMETAL_MEMORY_ACCESS_TRACING}
150:     BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL=${BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL}
151:     BAREMETAL_I2C_TRACING=${BAREMETAL_I2C_TRACING}
152:     BAREMETAL_I2C_TRACING_DETAIL=${BAREMETAL_I2C_TRACING_DETAIL}
153:     BAREMETAL_MAJOR=${VERSION_MAJOR}
154:     BAREMETAL_MINOR=${VERSION_MINOR}
155:     BAREMETAL_LEVEL=${VERSION_LEVEL}
156:     BAREMETAL_BUILD=${VERSION_BUILD}
157:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
158:     )
...
321: message(STATUS "Baremetal settings:")
322: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
323: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
324: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
325: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
326: message(STATUS "-- Debug output to UART0:           ${BAREMETAL_CONSOLE_UART0}")
327: message(STATUS "-- Debug output to UART1:           ${BAREMETAL_CONSOLE_UART1}")
328: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
329: message(STATUS "-- Debug tracing output:            ${BAREMETAL_TRACE_DEBUG}")
330: message(STATUS "-- Memory tracing output:           ${BAREMETAL_TRACE_MEMORY}")
331: message(STATUS "-- Detailed memory tracing output:  ${BAREMETAL_TRACE_MEMORY_DETAIL}")
332: message(STATUS "-- Memory access tracing output:    ${BAREMETAL_TRACE_MEMORY_ACCESS_STUBS}")
333: message(STATUS "-- Detailed Memory access tracing:  ${BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL}")
334: message(STATUS "-- I2C tracing output:              ${BAREMETAL_TRACE_I2C}")
335: message(STATUS "-- Detailed I2C tracing output:     ${BAREMETAL_TRACE_I2C_DETAIL}")
336: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
337: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
338: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
339: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
340: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
...
```

- Line 74: We add an option `BAREMETAL_TRACE_I2C` which is off by default
- Line 75: We add an option `BAREMETAL_TRACE_I2C_DETAIL` which is off by default
- Line 129-133: Depending on the value of `BAREMETAL_TRACE_I2C` we set the variable `BAREMETAL_I2C_TRACING` to either 0 or 1
- Line 134-138: Depending on the value of `BAREMETAL_TRACE_I2C_DETAIL` we set the variable `BAREMETAL_I2C_TRACING_DETAIL` to either 0 or 1
- Line 151: We add the compiler definition `BAREMETAL_I2C_TRACING` as the value of the CMake variable `BAREMETAL_MEMORY_ACCESS_TRACING`
- Line 152: We add the compiler definition `BAREMETAL_I2C_TRACING_DETAIL` as the value of the CMake variable `BAREMETAL_MEMORY_ACCESS_TRACING_DETAIL`
- Line 317-318: We print the actual values for `BAREMETAL_TRACE_I2C` and `BAREMETAL_TRACE_I2C_DETAIL`

### Update build configuration {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_UPDATE_BUILD_CONFIGURATION}

We'll now make sure to set the value of `BAREMETAL_TRACE_I2C` to `ON` when building.
Meanwhile we will also switch off tracing memory access to keep the amount of logging limited.

Update the file `CMakeSettings.json`.

```text
File: CMakeSettings.json
...
4:         {
5:             "name": "BareMetal-RPI3-Debug",
6:             "generator": "Ninja",
7:             "configurationType": "Debug",
8:             "buildRoot": "${projectDir}\\cmake-${name}",
9:             "installRoot": "${projectDir}\\output\\install\\${name}",
10:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=ON -DBAREMETAL_CONSOLE_UART0=ON -DBAREMETAL_TRACE_MEMORY_ACCESS_STUBS=OFF -DBAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL=OFF -DBAREMETAL_TRACE_I2C=ON",
11:             "buildCommandArgs": "",
12:             "ctestCommandArgs": "",
13:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
14:             "inheritEnvironments": [ "gcc-arm" ]
15:         },
...
```

- Line 10: We set the value of variable `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS` to `ON` and `BAREMETAL_TRACE_MEMORY_ACCESS_STUBS_DETAIL` to `OFF`.

### Update application code {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_UPDATE_APPLICATION_CODE}

Let's try working with a I2C display, for now using a stub.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Format.h>
2: #include <baremetal/I2CMaster.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/PhysicalGPIOPin.h>
5: #include <baremetal/String.h>
6: #include <baremetal/System.h>
7: #include <baremetal/Timer.h>
8: #include <baremetal/stubs/MemoryAccessStubI2C.h>
9: #include <device/i2c/HD44780DeviceI2C.h>
10: 
11: LOG_MODULE("main");
12: 
13: using namespace baremetal;
14: using namespace device;
15: 
16: int main()
17: {
18:     auto& console = GetConsole();
19:     GetLogger().SetLogLevel(LogSeverity::Debug);
20: 
21:     uint8 busIndex = 1;
22:     uint8 address{ 0x27 };
23:     const uint8 NumRows    = 2;
24:     const uint8 NumColumns = 16;
25:     MemoryAccessStubI2C memoryAccess;
26:     memoryAccess.SetBus(busIndex);
27:     I2CMaster i2cMaster(memoryAccess);
28:     i2cMaster.Initialize(busIndex);
29:     HD44780DeviceI2C device(i2cMaster, address, NumColumns, NumRows);
30: 
31:     device.Initialize();
32: 
33:     device.SetDisplayEnabled(true);
34:     device.SetCursorMode(device::ILCDDevice::CursorMode::Hide);
35: 
36:     device.ClearDisplay();
37:     device.Write("Hello World");
38: 
39:     Timer::WaitMilliSeconds(2000);
40: 
41:     device.SetDisplayEnabled(false);
42: 
43:     LOG_INFO("Rebooting");
44: 
45:     return static_cast<int>(ReturnCode::ExitReboot);
46: }
```

- Line 25-26: We instiate a memory access stub for I2C, and link it to the correct bus
- Line 27: We instantiate a I2CMaster and inject the stub
- Line 28: We initialize the I2CMaster
- Line 29: We set up the LCD display controller with I2C, and inject the I2CMaster

### Update CMake file for device library {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_UPDATE_CMAKE_FILE_FOR_DEVICE_LIBRARY}

We need to add the newly added files to the `device` library.

Update the file `code/libraries/device/CMakeLists.txt`

```cmake
File: code/libraries/device/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/display/HD44780Device.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/KY-040.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio/HD44780DeviceRaw.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/HD44780DeviceI2C.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/MCP23017.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/i2c/MemoryAccessStubMCP23017LEDs.cpp
37:     )
38: 
39: set(PROJECT_INCLUDES_PUBLIC
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/display/HD44780Device.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/display/ILCDDevice.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/gpio/HD44780DeviceRaw.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/gpio/KY-040.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/i2c/HD44780DeviceI2C.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/i2c/MCP23017.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/device/i2c/MemoryAccessStubMCP23017LEDs.h
47:     )
48: set(PROJECT_INCLUDES_PRIVATE )
```

### Configuring, building and debugging {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

```text
1: Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
2: Info   0.00:00:00.010 Starting up (System:209)
3: Info   0.00:00:00.020 Initialize bus 1, mode 0, config 0 (I2CMaster:172)
4: Info   0.00:00:00.020 Set clock 100000 (I2CMaster:215)
5: Info   0.00:00:00.030 Set up bus 1, config 0, base address 3F804000 (I2CMaster:198)
6: Info   0.00:00:00.040 Write FIFO, data 00000034 (I2CMaster:846)
7: Info   0.00:00:00.050 Write FIFO, data 00000030 (I2CMaster:846)
8: Info   0.00:00:00.050 Write FIFO, data 00000034 (I2CMaster:846)
9: Info   0.00:00:00.060 Write FIFO, data 00000030 (I2CMaster:846)
10: Info   0.00:00:00.060 Write FIFO, data 00000034 (I2CMaster:846)
11: Info   0.00:00:00.060 Write FIFO, data 00000030 (I2CMaster:846)
12: Info   0.00:00:00.060 Write FIFO, data 00000024 (I2CMaster:846)
13: Info   0.00:00:00.070 Write FIFO, data 00000020 (I2CMaster:846)
14: Info   0.00:00:00.070 Write FIFO, data 00000008 (I2CMaster:846)
15: Info   0.00:00:00.070 Write FIFO, data 0000002C (I2CMaster:846)
16: Info   0.00:00:00.080 Write FIFO, data 00000028 (I2CMaster:846)
17: Info   0.00:00:00.080 Write FIFO, data 0000008C (I2CMaster:846)
18: Info   0.00:00:00.080 Write FIFO, data 00000088 (I2CMaster:846)
19: Info   0.00:00:00.090 Write FIFO, data 0000000C (I2CMaster:846)
20: Info   0.00:00:00.090 Write FIFO, data 00000008 (I2CMaster:846)
21: Info   0.00:00:00.090 Write FIFO, data 0000008C (I2CMaster:846)
22: Info   0.00:00:00.090 Write FIFO, data 00000088 (I2CMaster:846)
23: Info   0.00:00:00.100 Write FIFO, data 0000000C (I2CMaster:846)
24: Info   0.00:00:00.100 Write FIFO, data 00000008 (I2CMaster:846)
25: Info   0.00:00:00.100 Write FIFO, data 0000001C (I2CMaster:846)
26: Info   0.00:00:00.100 Write FIFO, data 00000018 (I2CMaster:846)
27: Info   0.00:00:00.110 Write FIFO, data 0000000C (I2CMaster:846)
28: Info   0.00:00:00.110 Write FIFO, data 00000008 (I2CMaster:846)
29: Info   0.00:00:00.110 Write FIFO, data 0000006C (I2CMaster:846)
30: Info   0.00:00:00.120 Write FIFO, data 00000068 (I2CMaster:846)
31: Info   0.00:00:00.120 Write FIFO, data 0000000C (I2CMaster:846)
32: Info   0.00:00:00.120 Write FIFO, data 00000008 (I2CMaster:846)
33: Info   0.00:00:00.130 Write FIFO, data 000000CC (I2CMaster:846)
34: Info   0.00:00:00.130 Write FIFO, data 000000C8 (I2CMaster:846)
35: Info   0.00:00:00.130 Write FIFO, data 0000000C (I2CMaster:846)
36: Info   0.00:00:00.130 Write FIFO, data 00000008 (I2CMaster:846)
37: Info   0.00:00:00.140 Write FIFO, data 000000CC (I2CMaster:846)
38: Info   0.00:00:00.140 Write FIFO, data 000000C8 (I2CMaster:846)
39: Info   0.00:00:00.150 Write FIFO, data 0000000C (I2CMaster:846)
40: Info   0.00:00:00.150 Write FIFO, data 00000008 (I2CMaster:846)
41: Info   0.00:00:00.160 Write FIFO, data 0000001C (I2CMaster:846)
42: Info   0.00:00:00.160 Write FIFO, data 00000018 (I2CMaster:846)
43: Info   0.00:00:00.170 Write FIFO, data 0000000C (I2CMaster:846)
44: Info   0.00:00:00.170 Write FIFO, data 00000008 (I2CMaster:846)
45: Info   0.00:00:00.170 Write FIFO, data 0000002C (I2CMaster:846)
46: Info   0.00:00:00.170 Write FIFO, data 00000028 (I2CMaster:846)
47: Info   0.00:00:00.180 Write FIFO, data 0000004D (I2CMaster:846)
48: Info   0.00:00:00.180 Write FIFO, data 00000049 (I2CMaster:846)
49: Info   0.00:00:00.180 Write FIFO, data 0000008D (I2CMaster:846)
50: Info   0.00:00:00.190 Write FIFO, data 00000089 (I2CMaster:846)
51: Info   0.00:00:00.190 Write FIFO, data 0000006D (I2CMaster:846)
52: Info   0.00:00:00.190 Write FIFO, data 00000069 (I2CMaster:846)
53: Info   0.00:00:00.200 Write FIFO, data 0000005D (I2CMaster:846)
54: Info   0.00:00:00.200 Write FIFO, data 00000059 (I2CMaster:846)
55: Info   0.00:00:00.200 Write FIFO, data 0000006D (I2CMaster:846)
56: Info   0.00:00:00.200 Write FIFO, data 00000069 (I2CMaster:846)
57: Info   0.00:00:00.210 Write FIFO, data 000000CD (I2CMaster:846)
58: Info   0.00:00:00.210 Write FIFO, data 000000C9 (I2CMaster:846)
59: Info   0.00:00:00.210 Write FIFO, data 0000006D (I2CMaster:846)
60: Info   0.00:00:00.220 Write FIFO, data 00000069 (I2CMaster:846)
61: Info   0.00:00:00.220 Write FIFO, data 000000CD (I2CMaster:846)
62: Info   0.00:00:00.220 Write FIFO, data 000000C9 (I2CMaster:846)
63: Info   0.00:00:00.220 Write FIFO, data 0000006D (I2CMaster:846)
64: Info   0.00:00:00.230 Write FIFO, data 00000069 (I2CMaster:846)
65: Info   0.00:00:00.230 Write FIFO, data 000000FD (I2CMaster:846)
66: Info   0.00:00:00.230 Write FIFO, data 000000F9 (I2CMaster:846)
67: Info   0.00:00:00.240 Write FIFO, data 0000002D (I2CMaster:846)
68: Info   0.00:00:00.240 Write FIFO, data 00000029 (I2CMaster:846)
69: Info   0.00:00:00.240 Write FIFO, data 0000000D (I2CMaster:846)
70: Info   0.00:00:00.250 Write FIFO, data 00000009 (I2CMaster:846)
71: Info   0.00:00:00.250 Write FIFO, data 0000005D (I2CMaster:846)
72: Info   0.00:00:00.250 Write FIFO, data 00000059 (I2CMaster:846)
73: Info   0.00:00:00.250 Write FIFO, data 0000007D (I2CMaster:846)
74: Info   0.00:00:00.260 Write FIFO, data 00000079 (I2CMaster:846)
75: Info   0.00:00:00.260 Write FIFO, data 0000006D (I2CMaster:846)
76: Info   0.00:00:00.260 Write FIFO, data 00000069 (I2CMaster:846)
77: Info   0.00:00:00.260 Write FIFO, data 000000FD (I2CMaster:846)
78: Info   0.00:00:00.270 Write FIFO, data 000000F9 (I2CMaster:846)
79: Info   0.00:00:00.270 Write FIFO, data 0000007D (I2CMaster:846)
80: Info   0.00:00:00.270 Write FIFO, data 00000079 (I2CMaster:846)
81: Info   0.00:00:00.280 Write FIFO, data 0000002D (I2CMaster:846)
82: Info   0.00:00:00.280 Write FIFO, data 00000029 (I2CMaster:846)
83: Info   0.00:00:00.280 Write FIFO, data 0000006D (I2CMaster:846)
84: Info   0.00:00:00.290 Write FIFO, data 00000069 (I2CMaster:846)
85: Info   0.00:00:00.290 Write FIFO, data 000000CD (I2CMaster:846)
86: Info   0.00:00:00.290 Write FIFO, data 000000C9 (I2CMaster:846)
87: Info   0.00:00:00.290 Write FIFO, data 0000006D (I2CMaster:846)
88: Info   0.00:00:00.300 Write FIFO, data 00000069 (I2CMaster:846)
89: Info   0.00:00:00.300 Write FIFO, data 0000004D (I2CMaster:846)
90: Info   0.00:00:00.300 Write FIFO, data 00000049 (I2CMaster:846)
91: Info   0.00:00:02.290 Write FIFO, data 0000000C (I2CMaster:846)
92: Info   0.00:00:02.290 Write FIFO, data 00000008 (I2CMaster:846)
93: Info   0.00:00:02.290 Write FIFO, data 0000008C (I2CMaster:846)
94: Info   0.00:00:02.300 Write FIFO, data 00000088 (I2CMaster:846)
95: Info   0.00:00:02.300 Rebooting (main:43)
96: Info   0.00:00:02.300 Write FIFO, data 00000000 (I2CMaster:846)
97: Info   0.00:00:02.300 Write FIFO, data 00000004 (I2CMaster:846)
98: Info   0.00:00:02.310 Write FIFO, data 00000000 (I2CMaster:846)
99: Info   0.00:00:02.310 Write FIFO, data 00000084 (I2CMaster:846)
100: Info   0.00:00:02.310 Write FIFO, data 00000080 (I2CMaster:846)
101: Info   0.00:00:02.320 Reboot (System:154)
102: Info   0.00:00:02.320 InterruptSystem::Shutdown (InterruptHandler:153)
103: Debug  0.00:00:02.320 InterruptSystem::DisableInterrupts (InterruptHandler:166)
```

- Line 6-7: We see the 4 bits 0x03 being sent, once with the E bit on, once with the bit off.
This is the start of the initialization sequence
- Line 8-9: We see the 4 bits 0x03 being sent again
- Line 10-11: We see the 4 bits 0x03 being sent once more
- Line 12-13: We see the 4 bits 0x02 being sent once more.
This is the end of the initialization sequence
- Line 14: The backlight is switched on (bit 3=1)
- Line 15-18: The 4 bits 0x02 are written followed by 0x08, making for the 8 bits 0x28. Note that the backlight is still on, hence bit 3 is on
- Line 19-22: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF
- Line 23-26: The 8 bits 0x01 are written, meaning Clear Display
- Line 27-30: The 8 bits 0x06 are written, meaning Entry Mode Set with incrementing position, and moving cursor instead of shifting display.
This is the last command sent by the `Initialize()` method
- Line 31-34: The 8 bits 0x0C are written, meaning Display On/Off Control with the display set to ON
This is caused the line in the application code switching the display on
- Line 35-38: The 8 bits 0x0C are written, meaning again Display On/Off Control with the display set to ON, this is caused by the line setting the cursor to hidden
- Line 39-42: The 8 bits 0x01 are written, meaning again Clear Display
- Line 43-46: The 8 bits 0x02 are written, meaning Return Home
- Line 47-50: The 8 bits 0x48 are written, but this time as data, bit 0 (RS) is made high. This means the character 0x48='H' is written
- Line 51-54: The 8 bits 0x65 are written, which means the character 'e' is written
- Line 55-90: The other characters are written
- Line 91-94: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF
- Line 96: The backlight is switched off (bit 3=0)
- Line 97-100: The 8 bits 0x08 are written, meaning Display On/Off Control with the display set to OFF.
This is caused by the destructor of `HD44780DeviceI2C`

## I2C operation with real device {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_WITH_REAL_DEVICE}

After seeing the interaction with the stub, let's attach actual hardware and see how the display behaves.

### Update application code {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_WITH_REAL_DEVICE_UPDATE_APPLICATION_CODE}

Now let's use the actual device.
We'll make the example slightly more entertaining.

We will show a choice of either `Reboot` or `Halt`, when the KY-040 rotary switch is turned the option will toggle between these two.
When the switch is pressed, we will perform the selected action, e.g. either halt or reboot the system.

Update the file code/applications/demo/src/main.cpp.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Format.h>
3: #include <baremetal/I2CMaster.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/PhysicalGPIOPin.h>
6: #include <baremetal/Serialization.h>
7: #include <baremetal/String.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
10: #include <baremetal/stubs/MemoryAccessStubGPIO.h>
11: #include <device/gpio/KY-040.h>
12: #include <device/i2c/HD44780DeviceI2C.h>
13: 
14: LOG_MODULE("main");
15: 
16: using namespace baremetal;
17: using namespace device;
18: 
19: static int option{};
20: static bool select{};
21: static HD44780Device* lcdDevice{};
22: 
23: static void ShowOption();
24: 
25: static void OnEvent(KY040::Event event, void *param)
26: {
27:     LOG_INFO("Event %s", KY040::EventToString(event));
28:     switch (event)
29:     {
30:         case KY040::Event::SwitchDown:
31:             LOG_INFO("Value selected");
32:             select = true;
33:             break;
34:         case KY040::Event::RotateClockwise:
35:             option++;
36:             ShowOption();
37:             break;
38:         case KY040::Event::RotateCounterclockwise:
39:             option--;
40:             ShowOption();
41:             break;
42:         default:
43:             break;
44:     }
45: }
46: 
47: static bool ShouldReboot()
48: {
49:     return (option % 2 == 0);
50: }
51: 
52: static void ShowOption()
53: {
54:     lcdDevice->SetCursorPosition(0, 1);
55:     lcdDevice->Write("                ", 16);
56:     lcdDevice->SetCursorPosition(0, 1);
57:     if (ShouldReboot())
58:     {
59:         LOG_INFO("Select Reboot");
60:         lcdDevice->Write("Reboot", 6);
61:     }
62:     else
63:     {
64:         LOG_INFO("Select Halt");
65:         lcdDevice->Write("Halt", 4);
66:     }
67: }
68: 
69: int main()
70: {
71:     auto& console = GetConsole();
72:     GetLogger().SetLogLevel(LogSeverity::Info);
73: 
74:     KY040 rotarySwitch(11, 9, 10);
75:     rotarySwitch.Initialize();
76:     rotarySwitch.RegisterEventHandler(OnEvent, nullptr);
77: 
78:     uint8 busIndex = 1;
79:     uint8 address{ 0x27 };
80:     const uint8 NumRows    = 2;
81:     const uint8 NumColumns = 16;
82:     I2CMaster i2cMaster;
83:     i2cMaster.Initialize(busIndex);
84:     HD44780DeviceI2C device(i2cMaster, address, NumColumns, NumRows);
85: 
86:     device.Initialize();
87:     lcdDevice = &device;
88: 
89:     console.Write("Select from menu by turning rotary switch and pressing to select option\n");
90: 
91:     device.SetBacklight(true);
92:     device.SetDisplayEnabled(true);
93:     device.SetCursorMode(device::ILCDDevice::CursorMode::Hide);
94: 
95:     device.ClearDisplay();
96:     device.SetCursorPosition(0, 0);
97:     device.Write("Menu", 4);
98:     device.Home();
99:     ShowOption();
100: 
101:     while (!select)
102:     {
103:         WaitForInterrupt();
104:     }
105: 
106:     device.SetBacklight(false);
107:     device.SetDisplayEnabled(false);
108: 
109:     if (ShouldReboot())
110:         LOG_INFO("Rebooting");
111:     else
112:         LOG_INFO("Halting");
113: 
114:     return static_cast<int>(ShouldReboot() ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
115: }
```

- Line 25-45: We handle the callbacks form the KY-040 switch.
When turned we increment of decrement the option, and call `ShowOption()` to update the display.
When pressed, we set the variable `select` to true
- Line 47-50: We create a utility function to see whether the `option` variable is even, meaning we should reboot
- Line 52-67: We update the display with the selected option value.
We only update the second line of the display
- Line 78-84: We instantiate a I2C display controller like before, only this time using a real MemoryAccess instance
- Line 96-98: We write the text "Menu" on the first line
- Line 99: Then we call `ShowOption()` to show the initial option value on the second line
- Line 101-104: While no interrupts occur we wait, looping until the `select` variable is set to true

### Configuring, building and debugging {#TUTORIAL_22_LCD_DISPLAY_I2C_OPERATION_WITH_REAL_DEVICE_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and test.

```text
Info   0.00:00:00.030 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:96)
Info   0.00:00:00.060 Starting up (System:209)
Info   0.00:00:00.080 Initialize bus 1, mode 0, config 0 (I2CMaster:175)
Info   0.00:00:00.110 Set clock 100000 (I2CMaster:218)
Info   0.00:00:00.130 Set up bus 1, config 0, base address 3F804000 (I2CMaster:201)
Select from menu by turning rotary switch and pressing to select option
Info   0.00:00:00.320 Select Reboot (main:59)
Info   0.00:00:03.630 Event RotateCounterclockwise (main:27)
Info   0.00:00:03.630 Select Halt (main:64)
Info   0.00:00:04.350 Event RotateCounterclockwise (main:27)
Info   0.00:00:04.350 Select Reboot (main:59)
Info   0.00:00:04.910 Event RotateCounterclockwise (main:27)
Info   0.00:00:04.910 Select Halt (main:64)
Info   0.00:00:05.500 Event RotateCounterclockwise (main:27)
Info   0.00:00:05.500 Select Reboot (main:59)
Info   0.00:00:06.850 Event SwitchDown (main:27)
Info   0.00:00:06.850 Value selected (main:31)
Info   0.00:00:06.900 Rebooting (main:110)
Info   0.00:00:06.920 Reboot (System:154)
Info   0.00:00:06.940 InterruptSystem::Shutdown (InterruptHandler:153)
```

Next: [23-i2s](23-i2s.md)
