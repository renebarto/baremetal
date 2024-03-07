# Tutorial 12: Logger {#TUTORIAL_12_LOGGER}

@tableofcontents

## New tutorial setup {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-12.a`
- an application `output/Debug/bin/12-logger.elf`
- an image in `deploy/Debug/12-logger-image`

## Adding a logger {#TUTORIAL_12_LOGGER_ADDING_A_LOGGER}

We can now write to either UART0 or UART1, and we have a common ancestor, `CharDevice`.
It would be nice if we could write to the console in a more generic way, using a timestamp, a log level, a variable number of arguments, and preferably in color if supported.
A log statement might look like this:

```cpp
LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
```

For this, we will need to be able to set a default logger device, by injecting an instance of a `CharDevice`, and then enable printing to that device, much like `printf()`.
Later on, we can add the screen or something else as a device for logging as well.
In order to enable writing variable argument lists, we will need to use strings, which grow automatically as needed.
This then means we need to be able to allocate heap memory.
Also, we will need a way to retrieve the current time.

So, all in all, quite some work to do.

## Console - Step 1 {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1}

The first step we need to take is get hold of a console, depending on what is set as the default.
We will add a definition in the root CMake file to select whether this is is UART0 or UART1, and retrieve a console based on this.
The console will have some additional functionality, such as enabling the use of ANSI colors.

### CMakeSettings.json {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_CMAKESETTINGSJSON}

We'll start by adding the option for a default console.
Were going to add two variables to CMake:

- BAREMETAL_CONSOLE_UART0
- BAREMETAL_CONSOLE_UART1

If `BAREMETAL_CONSOLE_UART0` is defined, we'll use UART0 for the console,
otherwise if `BAREMETAL_CONSOLE_UART1` is defined, we'll use UART1 for the console.
If neither is defined, we will set the console to nullptr, or nothing.

Update `CMakeSettings.json`:

```text
File: CMakeSettings.json
...
5:       "name": "BareMetal-Debug",
...
10:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBAREMETAL_CONSOLE_UART0=ON",
...
17:       "name": "BareMetal-RPI4-Debug",
...
22:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBAREMETAL_CONSOLE_UART0=ON",
...
29:       "name": "BareMetal-Release",
...
34:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBAREMETAL_CONSOLE_UART0=ON",
...
41:       "name": "BareMetal-RPI4-Release",
...
46:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBAREMETAL_CONSOLE_UART0=ON",
...
53:       "name": "BareMetal-RelWithDebInfo",
...
58:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBAREMETAL_CONSOLE_UART0=ON",
...
65:       "name": "BareMetal-RPI4-RelWithDebInfo",
...
70:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBAREMETAL_CONSOLE_UART0=ON",
...
77:       "name": "BareMetal-MinSizeRel",
...
82:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBAREMETAL_CONSOLE_UART0=ON",
...
89:       "name": "BareMetal-RPI4-MinSizeRel",
...
94:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBAREMETAL_CONSOLE_UART0=ON",
...
```

In other words, in all configurations we will add `-DBAREMETAL_CONSOLE_UART0=ON` to make UART0 the default console.

### Root CMake file {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_ROOT_CMAKE_FILE}

Now, we'll define the new variables, and create definitions for our code accordingly.

Update `CMakeLists.txt`:

```cmake
File: CMakeLists.txt
...
44: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
45: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
...
66: set(DEFINES_C
67:     PLATFORM_BAREMETAL
68:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
69:     USE_PHYSICAL_COUNTER
70:     )
71: if (BAREMETAL_CONSOLE_UART0)
72:     list(APPEND DEFINES_C BAREMETAL_CONSOLE_UART0)
73: endif()
74: if (BAREMETAL_CONSOLE_UART1)
75:     list(APPEND DEFINES_C BAREMETAL_CONSOLE_UART1)
76: endif()
...
227: message(STATUS "Baremetal settings:")
228: message(STATUS "-- RPI target:          ${BAREMETAL_RPI_TARGET}")
229: message(STATUS "-- Architecture options:${BAREMETAL_ARCH_CPU_OPTIONS}")
230: message(STATUS "-- Kernel name:         ${BAREMETAL_TARGET_KERNEL}")
231: message(STATUS "-- Kernel load address: ${BAREMETAL_LOAD_ADDRESS}")
232: message(STATUS "-- Debug ouput to UART0:${BAREMETAL_CONSOLE_UART0}")
233: message(STATUS "-- Debug ouput to UART1:${BAREMETAL_CONSOLE_UART1}")
234:
...
```

- Line 44-45: We define the two new variables, and set them to off by default
- Line 71-73: If `BAREMETAL_CONSOLE_UART0` is set to on, We add the definition `BAREMETAL_CONSOLE_UART0`
- Line 74-76: If `BAREMETAL_CONSOLE_UART1` is set to on, We add the definition `BAREMETAL_CONSOLE_UART1`
- Line 227-233: We also print the current settings for all relevant variables

### Console.h {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_CONSOLEH}

We will now add the console class declaration.

Create the file `code/libraries/baremetal/include/baremetal/Console.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Console.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Console.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Console
9: //
10: // Description : Console writer with ANSI color support
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: #include <baremetal/CharDevice.h>
43: #include <baremetal/Types.h>
44:
45: /// @file
46: /// Console
47: ///
48: /// Used to write logging information. Supports use of ANSI color coding
49:
50: namespace baremetal {
51:
52: /// @brief  Console color (foreground or background)
53: enum class ConsoleColor
54: {
55:     /// @brief Default brings the color back to the color originally set
56:     Default = -1,
57:     /// @brief Black color
58:     Black   = 0,
59:     /// @brief Red color
60:     Red,
61:     /// @brief Green color
62:     Green,
63:     /// @brief Yellow color
64:     Yellow,
65:     /// @brief Blue color
66:     Blue,
67:     /// @brief Magenta color
68:     Magenta,
69:     /// @brief Cyan color
70:     Cyan,
71:     /// @brief Light gray color
72:     LightGray,
73:     /// @brief Dark gray color (light version of Black)
74:     DarkGray,
75:     /// @brief Bright red color (light version of Red)
76:     BrightRed,
77:     /// @brief Bright green color (light version of Green)
78:     BrightGreen,
79:     /// @brief Bright yellow color (light version of Yellow)
80:     BrightYellow,
81:     /// @brief Bright blue color (light version of Blue)
82:     BrightBlue,
83:     /// @brief Bright magenta color (light version of Magenta)
84:     BrightMagenta,
85:     /// @brief Bright cyan color (light version of Cyan)
86:     BrightCyan,
87:     /// @brief White color (light version of LightGray)
88:     White,
89: };
90:
91: /// @brief Class to output to the console.
92: ///
93: /// This is a singleton, in that it is not possible to create a default instance (GetConsole() needs to be used for this)
94: class Console
95: {
96:     /// <summary>
97:     /// Construct the singleton Console instance if needed, and return a reference to the instance. This is a friend function of class Console
98:     /// </summary>
99:     /// <returns>Reference to the singleton Console instance</returns>
100:     friend Console &GetConsole();
101:
102: private:
103:     /// @brief Character device to write to
104:     CharDevice *m_device;
105:
106:     explicit Console(CharDevice *device);
107:
108: public:
109:     void AssignDevice(CharDevice *device);
110:     void SetTerminalColor(ConsoleColor foregroundColor = ConsoleColor::Default, ConsoleColor backgroundColor = ConsoleColor::Default);
111:     void ResetTerminalColor();
112:
113:     void Write(const char *str, ConsoleColor foregroundColor, ConsoleColor backgroundColor = ConsoleColor::Default);
114:     void Write(const char *str);
115:
116:     char ReadChar();
117:     void WriteChar(char ch);
118:
119: private:
120: };
121:
122: Console &GetConsole();
123:
124: } // namespace baremetal
```

- Line 53-89: We declare an enum type for the foreground or background color
- Line 94-128: We declare the `Console` class
  - Line 100: We declare the `GetConsole()` function as a friend to the class
  - Line 104: We declare a member variable to hold the actual character device used for the console
  - Line 106: We declare the constructor, which is private, so `GetConsole()` needs to be called to create an instance
  - Line 109: We declare the method `AssignDevice()` to assign or reassign an device to the console
  - Line 110: We declare the method `SetTerminalColor()` to set the foreground and background color for the console. This outputs ANSI color codes
  - Line 111: We declare the method `ResetTerminalColor()` to reset the foreground and background color for the console. This outputs ANSI color codes
  - Line 113: We declare the method `Write()` to write a string with the foreground and background color specified
  - Line 114: We declare the method `Write()` to write a string without setting the color
  - Line 116: We declare the method `ReadChar()` to read a character from the console
  - Line 117: We declare the method `WriteChar()` to write a character to the console
- Line 122: We declare the accessor for the `Console` class. This will create an instance if needed, initialize it, and return a reference.

### Console.cpp {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_CONSOLECPP}

Create the file `code/libraries/baremetal/src/Console.cpp`:

```cpp
File: code/libraries/baremetal/src/Console.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Console.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Console
9: //
10: // Description : Console writer with ANSI color support
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include <baremetal/Console.h>
41: 
42: #include <baremetal/Serialization.h>
43: #include <baremetal/Timer.h>
44: #include <baremetal/UART0.h>
45: #include <baremetal/UART1.h>
46: #include <baremetal/Util.h>
47: 
48: /// @file
49: /// Console implementation
50: 
51: namespace baremetal {
52: 
53: /// <summary>
54: /// Determine ANSI color string for specified color
55: /// </summary>
56: /// <param name="color">ANSI color to use</param>
57: /// <returns>ANSI color string</returns>
58: static const char *GetAnsiColorCode(ConsoleColor color)
59: {
60:     switch (color)
61:     {
62:     case ConsoleColor::Black:
63:     case ConsoleColor::DarkGray:
64:         return "0";
65:     case ConsoleColor::Red:
66:     case ConsoleColor::BrightRed:
67:         return "1";
68:     case ConsoleColor::Green:
69:     case ConsoleColor::BrightGreen:
70:         return "2";
71:     case ConsoleColor::Yellow:
72:     case ConsoleColor::BrightYellow:
73:         return "3";
74:     case ConsoleColor::Blue:
75:     case ConsoleColor::BrightBlue:
76:         return "4";
77:     case ConsoleColor::Magenta:
78:     case ConsoleColor::BrightMagenta:
79:         return "5";
80:     case ConsoleColor::Cyan:
81:     case ConsoleColor::BrightCyan:
82:         return "6";
83:     case ConsoleColor::LightGray:
84:     case ConsoleColor::White:
85:         return "7";
86:     default:
87:         return 0;
88:     };
89: }
90: 
91: /// <summary>
92: /// Create a console linked to the specified character device. Note that the constructor is private, so GetConsole() is needed to instantiate the console
93: /// </summary>
94: /// <param name="device">Character device used for output</param>
95: Console::Console(CharDevice *device)
96:     : m_device{device}
97: {
98: }
99: 
100: /// <summary>
101: /// Change the attached device
102: /// </summary>
103: /// <param name="device">Character device to be set to for output</param>
104: void Console::AssignDevice(CharDevice *device)
105: {
106:     m_device = device;
107: }
108: 
109: /// <summary>
110: /// Set console foreground and background color (will output ANSI color codes)
111: /// </summary>
112: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
113: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
114: void Console::SetTerminalColor(ConsoleColor foregroundColor /*= ConsoleColor::Default*/, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
115: {
116:     Write("\033[0");
117:     if (foregroundColor != ConsoleColor::Default)
118:     {
119:         if (foregroundColor <= ConsoleColor::LightGray)
120:         {
121:             Write(";3");
122:         }
123:         else
124:         {
125:             Write(";9");
126:         }
127:         Write(GetAnsiColorCode(foregroundColor));
128:     }
129:     if (backgroundColor != ConsoleColor::Default)
130:     {
131:         if (backgroundColor <= ConsoleColor::LightGray)
132:         {
133:             Write(";4");
134:         }
135:         else
136:         {
137:             Write(";10");
138:         }
139:         Write(GetAnsiColorCode(backgroundColor));
140:     }
141:     Write("m");
142: }
143: 
144: /// <summary>
145: /// Reset console foreground and background back to original (will output ANSI color codes)
146: /// </summary>
147: void Console::ResetTerminalColor()
148: {
149:     SetTerminalColor();
150: }
151: 
152: /// <summary>
153: /// Write a string, using the specified foreground and background color
154: /// </summary>
155: /// <param name="str">String to be written</param>
156: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
157: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
158: void Console::Write(const char *str, ConsoleColor foregroundColor, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
159: {
160:     static volatile bool inUse{};
161: 
162:     while (inUse)
163:     {
164:         Timer::WaitMilliSeconds(1);
165:     }
166:     inUse = true;
167: 
168:     SetTerminalColor(foregroundColor, backgroundColor);
169:     Write(str);
170:     SetTerminalColor();
171: 
172:     inUse = false;
173: }
174: 
175: /// <summary>
176: /// Write a string without changing the foreground and background color
177: /// </summary>
178: /// <param name="str">String to be written</param>
179: void Console::Write(const char *str)
180: {
181:     while (*str)
182:     {
183:         // convert newline to carriage return + newline
184:         if (*str == '\n')
185:             WriteChar('\r');
186:         WriteChar(*str++);
187:     }
188: }
189: 
190: /// <summary>
191: /// Read a character
192: /// </summary>
193: /// <returns>Character received</returns>
194: char Console::ReadChar()
195: {
196:     char ch{};
197:     if (m_device != nullptr)
198:     {
199:         ch = m_device->Read();
200:     }
201:     return ch;
202: }
203: 
204: /// <summary>
205: /// Write a single character.
206: /// </summary>
207: /// <param name="ch">Character to be written</param>
208: void Console::WriteChar(char ch)
209: {
210:     if (m_device != nullptr)
211:     {
212:         m_device->Write(ch);
213:     }
214: }
215: 
216: /// <summary>
217: /// Retrieve the singleton console
218: ///
219: /// Creates a static instance of Console, and returns a reference to it.
220: /// </summary>
221: /// <returns>A reference to the singleton console.</returns>
222: Console &GetConsole()
223: {
224: #if defined(BAREMETAL_CONSOLE_UART0)
225:     static UART0&  uart = GetUART0();
226:     static Console console(&uart);
227: #elif defined(BAREMETAL_CONSOLE_UART1)
228:     static UART1&  uart = GetUART1();
229:     static Console console(&uart);
230: #else
231:     static Console console(nullptr);
232: #endif
233:     return console;
234: }
235: 
236: } // namespace baremetal
```

- Line 58-89: Implements a color conversion to an ANSI color code. Not that normal and bright colors use the same code, but have a different prefix.
- Line 95-98: Implements the constructor
- Line 104-107: Implements the method `AssignDevice()`
- Line 114-142: Implements the method `SetTerminalColor()`. Here we see that normal and light foreground colors use a different prefix, and similarly for background colors.
- Line 147-150: Implements the method `ResetTerminalColor()`, which simply calls `SetTerminalColor()` with default arguments.
- Line 158-173: Implements the `Write()` method for a string with color specification. This is simply a combination of setting colors, writing the string, and resetting colors.
The only exception is that we wait for the bool inUse to became false. This is a very simple locking mechanism that will prevent multiple cores writing to the console at the same time, to organize output a bit better.
We'll get to that when we actually start using multiple cores.
- Line 179-188: Implements the `Write()` method for a string with no color specification.
- Line 194-1202: Implements the `ReadChar()` method.
- Line 208-214: Implements the `WriteChar()` method.
- Line 222-234: Implements the `GetConsole()` function.
Notice how we define a static UART0 instance when `BAREMETAL_CONSOLE_UART0` is defined,
otherwise we define a static UART1 instance when `BAREMETAL_CONSOLE_UART1` is defined,
and if both are not defined, we set the console to a nullptr device.

### Serialization.h {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_SERIALIZATIONH}

We need to add serialization of an 8 bit unsigned integer.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```text
File: code/libraries/baremetal/include/baremetal/Serialization.h
47: namespace baremetal {
48:
49: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros);
50: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
51: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros);
52:
53: } // namespace baremetal
54:
```

- Line 49: Declares a function similar to the existing two, for a 8 bit unsigned integer

### Serialization.cpp {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_SERIALIZATIONCPP}

Of course we also need to add the implementation for the newly added function.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```text
File: code/libraries/baremetal/src/Serialization.cpp
105: /// <summary>
106: /// Serialize a 8 bit unsigned value to buffer.
107: ///
108: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
109: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
110: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
111: ///
112: /// Base is the digit base, which can range from 2 to 36.
113: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
114: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
115: /// </summary>
116: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
117: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
118: /// <param name="value">Value to be serialized</param>
119: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
120: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
121: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
122: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
123: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros)
124: {
125:     SerializeInternal(buffer, bufferSize, value, width, base, showBase, leadingZeros, 8);
126: }
127:
```

As you can see, it again re-uses the `SerializeInternal()` function.

### Update application code {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_UPDATE_APPLICATION_CODE}

Let's use the console now.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/Mailbox.h>
5: #include <baremetal/MemoryManager.h>
6: #include <baremetal/RPIProperties.h>
7: #include <baremetal/SysConfig.h>
8: #include <baremetal/Serialization.h>
9: #include <baremetal/System.h>
10: #include <baremetal/Timer.h>
11:
12: using namespace baremetal;
13:
14: int main()
15: {
16:     auto& console = GetConsole();
17:     console.Write("Hello World!\n", ConsoleColor::Yellow);
18:
19:     char buffer[128];
20:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
21:     RPIProperties properties(mailbox);
22:
23:     uint64 serial{};
24:     if (properties.GetBoardSerial(serial))
25:     {
26:         console.Write("Mailbox call succeeded\n");
27:         console.Write("Serial: ");
28:         console.Write(serial, 0, 16, false, true);
29:         console.Write("\n");
30:     }
31:     else
32:     {
33:         console.Write("Mailbox call failed\n", ConsoleColor::Red);
34:     }
35:
36:     console.Write("Wait 5 seconds\n");
37:     Timer::WaitMilliSeconds(5000);
38:
39:     console.Write("Press r to reboot, h to halt\n");
40:     char ch{};
41:     while ((ch != 'r') && (ch != 'h'))
42:     {
43:         ch = console.ReadChar();
44:         console.WriteChar(ch);
45:     }
46:
47:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
48: }
```

- Line 16: We retrieve the singleton Console instance
- Line 17: We use the console to print the `Hello World!` string, but now in color
- Line 26-27: We again print the string through the console, in default color
- Line 28: We can now use the console directly to print the serial number
- Line 29: We again print the string through the console, in default color
- Line 33: We again print the string through the console, in red this time
- Line 36-45: We again use the console instead of the UART0 instance

### System.cpp {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_SYSTEMCPP}

As we switch the main application to the console, we should also switch the code in `System.cpp` to the console, otherwise we will be suddenly checking the port over, with strange effects.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Console.h>
45: #include <baremetal/MemoryAccess.h>
46: #include <baremetal/SysConfig.h>
47: #include <baremetal/Timer.h>
48: #include <baremetal/Util.h>
...
112: /// <summary>
113: /// Halts the system. This function will not return
114: /// </summary>
115: void System::Halt()
116: {
117:     GetConsole().Write("Halt\n", ConsoleColor::Cyan);
118:     Timer::WaitMilliSeconds(WaitTime);
119:
120:     // power off the SoC (GPU + CPU)
121:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
122:     r &= ~RPI_PWRMGT_RSTS_PART_CLEAR;
123:     r |= 0x555; // partition 63 used to indicate halt
124:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r);
125:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 1);
126:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
127:
128:     for (;;) // Satisfy [[noreturn]]
129:     {
130:         DataSyncBarrier();
131:         WaitForInterrupt();
132:     }
133: }
134:
135: /// <summary>
136: /// Reboots the system. This function will not return
137: /// </summary>
138: void System::Reboot()
139: {
140:     GetConsole().Write("Reboot\n", ConsoleColor::Cyan);
141:     Timer::WaitMilliSeconds(WaitTime);
142:
143:     DisableIRQs();
144:     DisableFIQs();
145:
146:     // power off the SoC (GPU + CPU)
147:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
148:     r &= ~RPI_PWRMGT_RSTS_PART_CLEAR;
149:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
150:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 1);
151:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
152:
153:     for (;;) // Satisfy [[noreturn]]
154:     {
155:         DataSyncBarrier();
156:         WaitForInterrupt();
157:     }
158: }
...
165: void sysinit()
166: {
167:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
168:     EnableIRQs(); // go to TASK_LEVEL
169:
170:     // clear BSS
171:     extern unsigned char __bss_start;
172:     extern unsigned char __bss_end;
173:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
174:
175:     // halt, if KERNEL_MAX_SIZE is not properly set
176:     // cannot inform the user here
177:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
178:     {
179:         GetSystem().Halt();
180:     }
181:
182:     // Call constructors of static objects
183:     extern void (*__init_start)(void);
184:     extern void (*__init_end)(void);
185:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
186:     {
187:         (**func)();
188:     }
189:
190:     GetConsole().Write("Starting up\n", ConsoleColor::Cyan);
191:
192:     extern int main();
193:
194:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
195:     {
196:         GetSystem().Reboot();
197:     }
198:
199:     GetSystem().Halt();
200: }
...
```

Note that line number have changed due to the `Doxygen` comments that were added.

- Line 44: We include the Console header
- Line 117: We write to the console, in cyan color
- Line 140: We write to the console, in cyan color
- Line 190: We write to the console, in cyan color

### Update project configuration {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
46:     )
47:
48: set(PROJECT_INCLUDES_PUBLIC
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
73:     )
74: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_CONSOLE__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color:

<img src="images/tutorial-12-console.png" alt="Console output" width="400"/>

## Printing a formatted string - Step 2 {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING__STEP_2}

In order to introduce the actual logging functionality, we would like to be able to print using variable arguments,
much like the standard C `printf()` function.

For variable arguments, we need to be able to handle these. Normally we would have functions or definitions such as `va_start`, `va_end` and `va_arg` for this, by inclusing `stdarg.h`.
Luckily, GCC offers these as builtin functions.
We will then need to implement formatting of strings to a buffer, using format strings like `printf()` uses.
We'll also want to support printing version information, so we'll provide for a way to pass on the version in the build, and make a string out of it.
Finally we can then add the logger class.

### StdArg.h {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING__STEP_2_STDARGH}

So let's add a header to act as the standard C library's `stdarg.h` would.

Create the file `code/libraries/baremetal/include/baremetal/StdArg.h`

```cpp
File: code/libraries/baremetal/include/baremetal/StdArg.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : StdArg.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Variable arguments handling
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: /// @file
43: /// Standard variable argument list handling using builtin functionality in GCC
44:
45: // prevent warning, if <stdarg.h> from toolchain is included too
46: #ifndef _STDARG_H
47:
48: /// @brief declare standard va_list type
49: typedef __builtin_va_list va_list;
50:
51: /// @brief define standard va_start macro
52: #define va_start(arg, last)     __builtin_va_start (arg, last)
53: /// @brief define standard va_end macro
54: #define va_end(arg)             __builtin_va_end (arg)
55: /// @brief define standard va_arg macro
56: #define va_arg(arg, type)       __builtin_va_arg (arg, type)
57:
58: #endif
```

- Line 46: We protect against duplicate definitions in case we use the standard C version `stdarg.h`
- Line 49: We define the type `va_list` as a built-in version
- Line 52: We define `va_start` as a built-in version
- Line 54: We define `va_end` as a built-in version
- Line 56: We define `va_arg` as a built-in version

### Format.h {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING__STEP_2_FORMATH}

We'll define two variants of a formatting function for printing to a string with variable arguments.
One using the ellipsis argument `...`, the other with a `va_list` argument.

Create the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Format.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : String formatting using standard argument handling
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: #include <baremetal/StdArg.h>
43: #include <baremetal/Types.h>
44:
45: namespace baremetal {
46:
47: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args);
48: void Format(char* buffer, size_t bufferSize, const char* format, ...);
49:
50: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING__STEP_2_FORMATCPP}

Now we will implement the formatting functions.

Be aware that this is a poor man's implementation, but it will do for now.

Create the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Format.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : String formatting using standard argument handling
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include <baremetal/Format.h>
41: 
42: #include <baremetal/Serialization.h>
43: #include <baremetal/Util.h>
44: 
45: namespace baremetal {
46: 
47: const size_t BufferSize = 1024;
48: 
49: 
50: /// <summary>
51: /// Append a character to the buffer
52: /// </summary>
53: /// <param name="buffer">Buffer to write to</param>
54: /// <param name="bufferSize">Size of the buffer</param>
55: /// <param name="c">Character to append</param>
56: static void Append(char* buffer, size_t bufferSize, char c)
57: {
58:     size_t len = strlen(buffer);
59:     char* p = buffer + len;
60:     if (static_cast<size_t>(p - buffer) < bufferSize)
61:     {
62:         *p++ = c;
63:     }
64:     if (static_cast<size_t>(p - buffer) < bufferSize)
65:     {
66:         *p = '\0';
67:     }
68: }
69: 
70: /// <summary>
71: /// Append a set of identical characters to the buffer
72: /// </summary>
73: /// <param name="buffer">Buffer to write to</param>
74: /// <param name="bufferSize">Size of the buffer</param>
75: /// <param name="count">Number of characters to append</param>
76: /// <param name="c">Character to append</param>
77: static void Append(char* buffer, size_t bufferSize, size_t count, char c)
78: {
79:     size_t len = strlen(buffer);
80:     char* p = buffer + len;
81:     while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
82:     {
83:         *p++ = c;
84:         --count;
85:     }
86:     if (static_cast<size_t>(p - buffer) < bufferSize)
87:     {
88:         *p = '\0';
89:     }
90: }
91: 
92: /// <summary>
93: /// Append a string to the buffer
94: /// </summary>
95: /// <param name="buffer">Buffer to write to</param>
96: /// <param name="bufferSize">Size of the buffer</param>
97: /// <param name="str">String to append</param>
98: static void Append(char* buffer, size_t bufferSize, const char* str)
99: {
100:     strncat(buffer, str, bufferSize);
101: }
102: 
103: void Format(char* buffer, size_t bufferSize, const char* format, ...)
104: {
105:     va_list var;
106:     va_start(var, format);
107: 
108:     FormatV(buffer, bufferSize, format, var);
109: 
110:     va_end(var);
111: }
112: 
113: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args)
114: {
115:     buffer[0] = '\0';
116: 
117:     while (*format != '\0')
118:     {
119:         if (*format == '%')
120:         {
121:             if (*++format == '%')
122:             {
123:                 Append(buffer, bufferSize, '%');
124:                 format++;
125:                 continue;
126:             }
127: 
128:             bool alternate = false;
129:             if (*format == '#')
130:             {
131:                 alternate = true;
132:                 format++;
133:             }
134: 
135:             bool left = false;
136:             if (*format == '-')
137:             {
138:                 left = true;
139:                 format++;
140:             }
141: 
142:             bool leadingZero = false;
143:             if (*format == '0')
144:             {
145:                 leadingZero = true;
146:                 format++;
147:             }
148: 
149:             size_t width = 0;
150:             while (('0' <= *format) && (*format <= '9'))
151:             {
152:                 width = width * 10 + (*format - '0');
153:                 format++;
154:             }
155: 
156:             unsigned precision = 6;
157:             if (*format == '.')
158:             {
159:                 format++;
160:                 precision = 0;
161:                 while ('0' <= *format && *format <= '9')
162:                 {
163:                     precision = precision * 10 + (*format - '0');
164: 
165:                     format++;
166:                 }
167:             }
168: 
169:             bool haveLong{};
170:             bool haveLongLong{};
171: 
172:             if (*format == 'l')
173:             {
174:                 if (*(format + 1) == 'l')
175:                 {
176:                     haveLongLong = true;
177: 
178:                     format++;
179:                 }
180:                 else
181:                 {
182:                     haveLong = true;
183:                 }
184: 
185:                 format++;
186:             }
187: 
188:             switch (*format)
189:             {
190:             case 'c':
191:                 {
192:                     char ch = static_cast<char>(va_arg(args, int));
193:                     if (left)
194:                     {
195:                         Append(buffer, bufferSize, ch);
196:                         if (width > 1)
197:                         {
198:                             Append(buffer, bufferSize, width - 1, ' ');
199:                         }
200:                     }
201:                     else
202:                     {
203:                         if (width > 1)
204:                         {
205:                             Append(buffer, bufferSize, width - 1, ' ');
206:                         }
207:                         Append(buffer, bufferSize, ch);
208:                     }
209:                 }
210:                 break;
211: 
212:             case 'd':
213:             case 'i':
214:                 if (haveLongLong)
215:                 {
216:                     char str[BufferSize]{};
217:                     Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
218:                     Append(buffer, bufferSize, str);
219:                 }
220:                 else if (haveLong)
221:                 {
222:                     char str[BufferSize]{};
223:                     Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
224:                     Append(buffer, bufferSize, str);
225:                 }
226:                 else
227:                 {
228:                     char str[BufferSize]{};
229:                     Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
230:                     Append(buffer, bufferSize, str);
231:                 }
232:                 break;
233: 
234:             case 'f':
235:                 {
236:                     char str[BufferSize]{};
237:                     Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
238:                     Append(buffer, bufferSize, str);
239:                 }
240:                 break;
241: 
242:             case 'b':
243:                 if (alternate)
244:                 {
245:                     Append(buffer, bufferSize, "0b");
246:                 }
247:                 if (haveLongLong)
248:                 {
249:                     char str[BufferSize]{};
250:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
251:                     Append(buffer, bufferSize, str);
252:                 }
253:                 else if (haveLong)
254:                 {
255:                     char str[BufferSize]{};
256:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
257:                     Append(buffer, bufferSize, str);
258:                 }
259:                 else
260:                 {
261:                     char str[BufferSize]{};
262:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
263:                     Append(buffer, bufferSize, str);
264:                 }
265:                 break;
266: 
267:             case 'o':
268:                 if (alternate)
269:                 {
270:                     Append(buffer, bufferSize, '0');
271:                 }
272:                 if (haveLongLong)
273:                 {
274:                     char str[BufferSize]{};
275:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
276:                     Append(buffer, bufferSize, str);
277:                 }
278:                 else if (haveLong)
279:                 {
280:                     char str[BufferSize]{};
281:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
282:                     Append(buffer, bufferSize, str);
283:                 }
284:                 else
285:                 {
286:                     char str[BufferSize]{};
287:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
288:                     Append(buffer, bufferSize, str);
289:                 }
290:                 break;
291: 
292:             case 's':
293:                 {
294:                     char str[BufferSize]{};
295:                     Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
296:                     Append(buffer, bufferSize, str);
297:                 }
298:                 break;
299: 
300:             case 'u':
301:                 if (haveLongLong)
302:                 {
303:                     char str[BufferSize]{};
304:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
305:                     Append(buffer, bufferSize, str);
306:                 }
307:                 else if (haveLong)
308:                 {
309:                     char str[BufferSize]{};
310:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
311:                     Append(buffer, bufferSize, str);
312:                 }
313:                 else
314:                 {
315:                     char str[BufferSize]{};
316:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
317:                     Append(buffer, bufferSize, str);
318:                 }
319:                 break;
320: 
321:             case 'x':
322:             case 'X':
323:                 if (alternate)
324:                 {
325:                     Append(buffer, bufferSize, "0x");
326:                 }
327:                 if (haveLongLong)
328:                 {
329:                     char str[BufferSize]{};
330:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
331:                     Append(buffer, bufferSize, str);
332:                 }
333:                 else if (haveLong)
334:                 {
335:                     char str[BufferSize]{};
336:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
337:                     Append(buffer, bufferSize, str);
338:                 }
339:                 else
340:                 {
341:                     char str[BufferSize]{};
342:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
343:                     Append(buffer, bufferSize, str);
344:                 }
345:                 break;
346: 
347:             case 'p':
348:                 if (alternate)
349:                 {
350:                     Append(buffer, bufferSize, "0x");
351:                 }
352:                 {
353:                     char str[BufferSize]{};
354:                     Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
355:                     Append(buffer, bufferSize, str);
356:                 }
357:                 break;
358: 
359:             default:
360:                 Append(buffer, bufferSize, '%');
361:                 Append(buffer, bufferSize, *format);
362:                 break;
363:             }
364:         }
365:         else
366:         {
367:             Append(buffer, bufferSize, *format);
368:         }
369: 
370:         format++;
371:     }
372: }
373: 
374: } // namespace baremetal
```

- Line 47: We define a buffer size, to define the buffer, but also to check against writing outside the buffer.
- Line 56-68: Implements the function `Append()` for writing a single character to the buffer, checked for overflow
- Line 77-90: Implements the function `Append()` for writing multiple of a single character to the buffer, checked for overflow
- Line 98-101: Implements the function `Append()` for writing a string to the buffer, checked for overflow
- Line 103-111: Implements the `Format()` function, using the ellipsis operator. This simple creates a `va_list` from the arguments, and calls the other version of `Format()`
- Line 113-372: Implements the `Format()` function, using the `va_list` argument
  - Line 115: We make sure to have an empty buffer
  - Line 117: We scan through the format string
  - Line 119: If the format character is `%` this is a special format operator
    - Line 121-126: In case the format string holds `%%` we see this as verbatim `%`, so we add that character
    - Line 128-133: If the next character is `#` we see this as an alternative version, signalling to add the prefix (only for base 2, 8, 16 integers), and advance
    - Line 135-140: If the next character is `-` we left-align the value, and advance
    - Line 142-147: If the next character is `0` we use leading zeros to fill up the value to its normal length, and advance
    - Line 149-154: If more digits follow, e.g. `%12` or `%012`, we extract the width of the value to be printed, and advance
    - Line 156-167: If a decimal point follows, we expect this to be a floating point value, and expect the next digits to specify the length of the fraction.
    We read all digits, calculate the fraction length, and advance
    - Line 169-186: If the next character is `l` this is seen as a specification for a `long` (32 bit) value (only for integer values).
If another `l` follows, we see this as a `long long` (64 bit) value
    - Line 190-210: If the format character is a `c` we print the value as a character
    - Line 212-232: If the format character is a `d` or `i` we print the value as a signed integer, taking into account the `l` or `ll` prefix
    - Line 234-250: If the format character is a `f` we print the value as a double, taking into account the precision if set
    - Line 242-265: If the format character is a `b` we print the value as a binary unsigned integer, taking into account the `#`, `l` or `ll` prefix.
Note that this is an addition to `printf()` behaviour
    - Line 267-290: If the format character is a `o` we print the value as a octal unsigned integer, taking into account the `#`, `l` or `ll` prefix.
Note that this is an addition to `printf()` behaviour
    - Line 292-298: If the format character is a `s` we print the value as a string
    - Line 300-319: If the format character is a `u` we print the value as a decimal unsigned integer, taking into account the `l` or `ll` prefix
    - Line 321-345: If the format character is a `x` or `X` we print the value as a hexadecimal unsigned integer, taking into account the `#`, `l` or `ll` prefix
    - Line 347-357: If the format character is a `p` we print the value as a pointer, meaning it is printed as a 64 bit unsigned integer
  - Line 365-368: If the format character is not `%` we simply add the character

## Adding the Logger class - Step 3 {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3}

### Adding version information {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION}

In order to be able to print the version of our code, we will add some infrastructure to CMake, to pass definitions on to our code.
We will extract the latest tag from our repository, and use that as the version number. If there is no tag, we will assume the version is `0.0.0`.

#### Updating the root CMake file {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_UPDATING_THE_ROOT_CMAKE_FILE}

We'll add reading the most recent tag from `Git`, converting it to a version number, and then splitting the version number into components.
Additionaly, we'll add a variable to control whether logging is done using ANSI color coding.

Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
13: include (functions)
14:
15: get_git_tag(GIT_TAG)
16:
17: message(STATUS "Tag found: ${GIT_TAG}")
18:
19: string(REGEX MATCH "^[0-9]*\.[0-9]*\.[0-9]*" VERSION_NUMBER ${GIT_TAG})
20:
21: if("${VERSION_NUMBER}" STREQUAL "")
22:     set(VERSION_NUMBER 0.0.0)
23: endif()
24:
25: parse_version(VERSION_NUMBER VERSION_MAJOR VERSION_MINOR VERSION_LEVEL VERSION_BUILD)
26:
27: set(VERSION_COMPOSED ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}.${VERSION_BUILD})
28: message(STATUS "Version: ${VERSION_COMPOSED}")
29:
...
61: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
62: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
63: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
...
82: if (BAREMETAL_COLOR_LOGGING)
83:     set(BAREMETAL_COLOR_OUTPUT 1)
84: else ()
85:     set(BAREMETAL_COLOR_OUTPUT 0)
86: endif()
87: set(BAREMETAL_LOAD_ADDRESS 0x80000)
88:
...
89: set(DEFINES_C
90:     PLATFORM_BAREMETAL
91:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
92:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
93:     USE_PHYSICAL_COUNTER
94:     BAREMETAL_MAJOR=${VERSION_MAJOR}
95:     BAREMETAL_MINOR=${VERSION_MINOR}
96:     BAREMETAL_LEVEL=${VERSION_LEVEL}
97:     BAREMETAL_BUILD=${VERSION_BUILD}
98:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
99:     )
...
263: message(STATUS "-- Color log output:    ${BAREMETAL_COLOR_LOGGING}")
264: message(STATUS "-- Version major:       ${VERSION_MAJOR}")
265: message(STATUS "-- Version minor:       ${VERSION_MINOR}")
266: message(STATUS "-- Version level:       ${VERSION_LEVEL}")
267: message(STATUS "-- Version build:       ${VERSION_BUILD}")
268: message(STATUS "-- Version composed:    ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
```

- Line 13: We include the CMake utility functions
- Line 15: We use a utility function (to be defined) `get_git_tag` to retrieve the latest tag in `Git` and store it in variable `GIT_TAG`
- Line 17: We print the tag retrieved
- Line 19: We match the tag to the pattern `digit+.digit+.digit+` and ignore the rest. This is stored in the variable `VERSION_NUMBER`
- Line 21-23: If the version extracted is empty, we set it to the default `0.0.0`
- Line 25: We use a utility function (to be defined) `parse_version` to extract the components from the version.
The components are stored respectively in variables `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_LEVEL` and `VERSION_BUILD`
- Line 27-28: We recompose the version from the components into variable `VERSION_COMPOSED`, and print it
- Line 63: We define the variable `BAREMETAL_COLOR_LOGGING` which is `ON` by default
- Line 82-86: If `BAREMETAL_COLOR_LOGGING` is `ON`, we set the variable `BAREMETAL_COLOR_OUTPUT` to 1, otherwise we set it to 0
- Line 89-99: We add compiler definitions using the version components, the composed version, and the ANSI color log output selection
- Line 263-268: We print the variable settings for ANSI color log output, and version information

#### Adding functionality to the CMake utility file {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_ADDING_FUNCTIONALITY_TO_THE_CMAKE_UTILITY_FILE}

We just used two utility functions that still need to be defined.

Update the file `cmake/functions.cmake`

```cmake
File: cmake/functions.cmake
...
18: function(get_git_tag out)
19:     if (UNIX)
20:         find_package(Git)
21:     else()
22:         find_program(GIT_EXECUTABLE git PATHS "C:/Program Files/Git/bin")
23:     endif()
24:     execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "[0-9]*.[0-9]*.[0-9]*" --tags --abbrev=5 HEAD
25:         RESULT_VARIABLE RESULT
26:         OUTPUT_VARIABLE OUTPUT
27:         ERROR_QUIET
28:         OUTPUT_STRIP_TRAILING_WHITESPACE)
29:     if (${RESULT} EQUAL 0)
30:         set(${out} "${OUTPUT}" PARENT_SCOPE)
31:     else()
32:         set(${out} "" PARENT_SCOPE)
33:     endif()
34: endfunction()
35:
36: function(parse_version version_number version_major version_minor version_level version_build)
37:     if ("${${version_number}}" STREQUAL "")
38:         set(${version_number} "0.0.0.0" PARENT_SCOPE)
39:     endif()
40:     string(REPLACE "." ";" VERSION_PARTS "${${version_number}}")
41:     LIST(LENGTH VERSION_PARTS VERSION_NUM_PARTS)
42:
43:     set(VERSION_MAJOR 0)
44:     if (VERSION_NUM_PARTS GREATER_EQUAL 1)
45:         list(GET VERSION_PARTS 0 VERSION_MAJOR)
46:     endif()
47:     set(${version_major} "${VERSION_MAJOR}" PARENT_SCOPE)
48:
49:     set(VERSION_MINOR 0)
50:     if (VERSION_NUM_PARTS GREATER_EQUAL 2)
51:         list(GET VERSION_PARTS 1 VERSION_MINOR)
52:     endif()
53:     set(${version_minor} "${VERSION_MINOR}" PARENT_SCOPE)
54:
55:     set(VERSION_LEVEL 0)
56:     if (VERSION_NUM_PARTS GREATER_EQUAL 3)
57:         list(GET VERSION_PARTS 2 VERSION_LEVEL)
58:     endif()
59:     set(${version_level} "${VERSION_LEVEL}" PARENT_SCOPE)
60:
61:     set(VERSION_BUILD 0)
62:     if (VERSION_NUM_PARTS GREATER_EQUAL 4)
63:         list(GET VERSION_PARTS 3 VERSION_BUILD)
64:     endif()
65:     set(${version_build} "${VERSION_BUILD}" PARENT_SCOPE)
66:
67:     if ("${VERSION_MAJOR}" STREQUAL "" OR
68:         "${VERSION_MINOR}" STREQUAL "" OR
69:         "${VERSION_LEVEL}" STREQUAL "" OR
70:         "${VERSION_BUILD}" STREQUAL "")
71:         message(SEND_ERROR "Incorrectly specified MSI number: ${version_number}")
72:     endif()
73: endfunction()
...
```

- Line 18-35: We define the function `get_git_tag` which returns the latest tag
  - Line 19-23: Depending on the platform, we determine the executable for git.
On Linux, we can simple try to find the package, on Windows, we assume that it is installed in `C:\Program Files\git`.
  - Line 24-28: We run git to get tags matching `<digit>+.<digit>+.<digit>+` and take the one from `HEAD`, i.e. the latest version.
This tag is stored in variable `OUTPUT`, the variable `RESULT` is used to store the execution return code
  - Line 29-33: If the command was executed without problems, we set the output variable to `OUTPUT` (we used `PARENT_SCOPE` to return the value to outside the function).
If the command failed, we set the output to an empty string
- Line 36-73: We define the function `parse_version` which splits the version string into components `<major>.<minor>.<level>.<build>`.
Any parts that are not present are set to 0
  - Line 37-39: If the version string is empty we set it to `0.0.0.0` (again we use `PARENT_SCOPE`)
  - Line 40-41: We replace the dots with semicolons, so we can create a list from the string, the result is stored in variable `VERSION_PARTS`.
We determined the length of the list and store it in variable `VERSION_NUM_PARTS`
  - Line 43-47: We set the major part to 0 as a default.
If the number of parts is at least 1, we get the first item in the list and stored it in variable `VERSION_MAJOR` (again we use `PARENT_SCOPE`)
  - Line 49-53: We set the minor part to 0 as a default.
If the number of parts is at least 2, we get the second item in the list and stored it in variable `VERSION_MINOR` (again we use `PARENT_SCOPE`)
  - Line 55-59: We set the level part to 0 as a default.
If the number of parts is at least 3, we get the third item in the list and stored it in variable `VERSION_LEVEL` (again we use `PARENT_SCOPE`)
  - Line 61-65: We set the build part to 0 as a default.
If the number of parts is at least 4, we get the fourth item in the list and stored it in variable `VERSION_BUILD` (again we use `PARENT_SCOPE`)
  - Line 67-72: We perform a sanity check on the parts, and print an error message if one of them is empty

#### Version.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_VERSIONH}

Now that we have set compiler definitions for the version, we can use them in code.

Create the file `code/libraries/baremetal/include/baremetal/Version.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Version.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Version.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Baremetal version information
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: /// @file
43: /// Build version
44: 
45: /// @brief Platform name
46: #define BAREMETAL_NAME              "Baremetal"
47: 
48: /// @brief Major version number (specified by define at compile time)
49: #define BAREMETAL_MAJOR_VERSION     BAREMETAL_MAJOR
50: /// @brief Minor version number (specified by define at compile time)
51: #define BAREMETAL_MINOR_VERSION     BAREMETAL_MINOR
52: /// @brief patch version number (specified by define at compile time)
53: #define BAREMETAL_PATCH_VERSION     BAREMETAL_LEVEL
54: /// @brief Version string
55: #define BAREMETAL_VERSION_STRING    GetVersion()
56: 
57: namespace baremetal {
58: 
59: void SetupVersion();
60: const char* GetVersion();
61: 
62: }
```

- Line 46: We create a definition for the name of our platform
- Line 49-53: We create definitions of the platform version parts, using the compiler definitions passed from `CMake`
- Line 55: We create a definition to get the version string
- Line 59: We declare a function `SetupVersion()` to build the version string
- Line 60: We declare a function `GetVersion()` to return the version string

#### Version.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_VERSIONCPP}

We need to implement the functions we just declared.

Create the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Version.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Baremetal version information
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include <baremetal/Version.h>
41: 
42: #include <baremetal/Format.h>
43: #include <baremetal/Util.h>
44: 
45: /// @file
46: /// Build version implementation
47: 
48: /// @brief Buffer size of version string buffer
49: static const size_t BufferSize = 20;
50: /// @brief Version string buffer
51: static char s_baremetalVersionString[BufferSize]{};
52: /// @brief Flag to check if version set up was already done
53: static bool s_baremetalVersionSetupDone = false;
54: 
55: /// <summary>
56: /// Set up version string
57: /// 
58: /// The version string is written into a buffer without allocating memory.
59: /// This is important, as we may be logging before memory management is set up.
60: /// </summary>
61: void baremetal::SetupVersion()
62: {
63:     if (!s_baremetalVersionSetupDone)
64:     {
65:         Format(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_PATCH_VERSION);
66:         s_baremetalVersionSetupDone = true;
67:     }
68: }
69: 
70: /// <summary>
71: /// Return version string
72: /// </summary>
73: /// <returns>Version string</returns>
74: const char* baremetal::GetVersion()
75: {
76:     return s_baremetalVersionString;
77: }
```

- Line 59-51: We define the local variable `s_baremetalVersionString` and its size
- Line 61-68: We implement `SetupVersion()`, which prints a formatted string to `s_baremetalVersionString`
- Line 74-77: We implement `GetVersion(), which simply returns the string `s_baremetalVersionString`

#### Logger.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_LOGGERH}

We'll now add the `Logger` class, which can be used to log to the console, at different log levels.

Create the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Logger.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Logger
9: //
10: // Description : Basic logging to a device
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: #include <baremetal/Console.h>
43: #include <baremetal/StdArg.h>
44: #include <baremetal/Types.h>
45: 
46: /// @file
47: /// Logger functionality
48: 
49: namespace baremetal {
50: 
51: /// <summary>
52: /// Logging severity classes
53: /// </summary>
54: enum class LogSeverity
55: {
56:     /// @brief Halt the system after processing this message
57:     Panic,
58:     /// @brief Severe error in this component, system may continue to work
59:     Error,
60:     /// @brief Non-severe problem, component continues to work
61:     Warning,
62:     /// @brief Informative message, which is interesting for the system user
63:     Info,
64:     /// @brief Message, which is only interesting for debugging this component
65:     Debug
66: };
67: 
68: class Timer;
69: 
70: /// <summary>
71: /// Logger class
72: /// </summary>
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
83:     bool        m_initialized;
84:     /// @brief Pointer to timer instance
85:     Timer      *m_timer;
86:     /// @brief Reference to console instance
87:     Console    &m_console;
88:     /// @brief Currently set logging severity level
89:     LogSeverity m_level;
90: 
91:     explicit Logger(LogSeverity logLevel, Timer *timer = nullptr, Console &console = GetConsole());
92: 
93: public:
94:     bool Initialize();
95:     void SetLogLevel(LogSeverity logLevel);
96: 
97:     void Write(const char *source, int line, LogSeverity severity, const char *message, ...);
98:     void WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args);
99: };
100: 
101: Logger &GetLogger();
102: 
103: /// @brief Define the static variable From to the specified name, to support printing a different file specification in LOG_* macros
104: #define LOG_MODULE(name)       static const char From[] = name
105: 
106: /// @brief Log a panic message
107: #define LOG_PANIC(...)         GetLogger().Write(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
108: /// @brief Log an error message
109: #define LOG_ERROR(...)         GetLogger().Write(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
110: /// @brief Log a warning message
111: #define LOG_WARNING(...)       GetLogger().Write(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
112: /// @brief Log a info message
113: #define LOG_INFO(...)          GetLogger().Write(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
114: /// @brief Log a debug message
115: #define LOG_DEBUG(...)         GetLogger().Write(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
116: 
117: /// @brief Log a message with specified severity and message string
118: #define LOG(severity, message) GetLogger().Write(From, __LINE__, severity, message);
119: 
120: } // namespace baremetal
```

- Line 54-66: We declare an enum type `LogSeverity` to signify the logging level
- Line 73-101: We declare the `Logger` class
  - Line 79: We declare the `GetLogger()` function as a friend
  - Line 83: The member variable `m_initialized` is used to guard against multiple initializations
  - Line 85: We keep a pointer to the `Timer` instance which will be used to request the current time for logging
  - Line 87: We keep a reference to the `Console` instance
  - Line 89: The member variable `m_level` is used as a reference level for logging.
  Any log statements with a priority equal to or higher than `m_level` (with a value equal to or lower than `m_level`) will be shown, others will be ignored
  - Line 91: We declare the constructor for `Logger`
  - Line 94: The method `Initialize()` initializes the logger, guarded by `m_initialized`
  - Line 97: The method `Write()` writes a log statement for a specific source file and line number, using a specific severity level, and with a format string and variable arguments
  - Line 98: The method `WriteV()` writes a log statement for a specific source file and line number, using a specific severity level, and with a format string and a variable argument list
- Line 101: We declare the accessor for the singleton `Logger` instance, `GetLogger()`
- Line 104: The macro `LOG_MODULE` is meant to define a variable that is used by the other macros, to specify the source file or source name to be used
- Line 107: The macro `LOG_PANIC` is meant to log at the highest level `Panic` (which will cause a Halt)
- Line 109: The macro `LOG_ERROR` writes to the log at level `Error`
- Line 111: The macro `LOG_WARNING` writes to the log at level `Warning`
- Line 113: The macro `LOG_INFO` writes to the log at level `Info`
- Line 115: The macro `LOG_DEBUG` writes to the log at level `Debug`
- Line 118: The macro `LOG` writes to the log at the specified level, with a single string as the argument

#### Logger.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_ADDING_VERSION_INFORMATION_LOGGERCPP}

We will add the implementation for the `Logger` class.

Create the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Logger.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Logger
9: //
10: // Description : Basic logging to a device
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include <baremetal/Logger.h>
41: 
42: #include <baremetal/Console.h>
43: #include <baremetal/Format.h>
44: #include <baremetal/System.h>
45: #include <baremetal/Timer.h>
46: #include <baremetal/Util.h>
47: #include <baremetal/Version.h>
48: 
49: /// @file
50: /// Logger functionality implementation
51: 
52: using namespace baremetal;
53: 
54: /// @brief Define log name
55: LOG_MODULE("Logger");
56: 
57: /// <summary>
58: /// Construct a logger
59: /// </summary>
60: /// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
61: /// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
62: /// <param name="console">Console to print to, defaults to the singleton console instance</param>
63: Logger::Logger(LogSeverity logLevel, Timer *timer /*= nullptr*/, Console &console /*= GetConsole()*/)
64:     : m_initialized{}
65:     , m_timer{timer}
66:     , m_console{console}
67:     , m_level{logLevel}
68: {
69: }
70: 
71: /// <summary>
72: /// Initialize logger
73: /// </summary>
74: /// <returns>true on succes, false on failure</returns>
75: bool Logger::Initialize()
76: {
77:     if (m_initialized)
78:         return true;
79:     SetupVersion();
80:     m_initialized = true; // Stop reentrant calls from happening
81:     LOG_INFO(BAREMETAL_NAME " %s started on %s (AArch64)", BAREMETAL_VERSION_STRING, "Raspberry Pi" /*GetMachineInfo().GetName()*/);
82: 
83:     return true;
84: }
85: 
86: /// <summary>
87: /// Set maximum log level (minimum log priority). Any log statements with a value below this level will be ignored
88: /// </summary>
89: /// <param name="logLevel">Maximum log level</param>
90: void Logger::SetLogLevel(LogSeverity logLevel)
91: {
92:     m_level = logLevel;
93: }
94: 
95: /// <summary>
96: /// Write a string with variable arguments to the logger
97: /// </summary>
98: /// <param name="source">Source name or file name</param>
99: /// <param name="line">Source line number</param>
100: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
101: /// <param name="message">Formatted message string, with variable arguments</param>
102: void Logger::Write(const char *source, int line, LogSeverity severity, const char *message, ...)
103: {
104:     va_list var;
105:     va_start(var, message);
106:     WriteV(source, line, severity, message, var);
107:     va_end(var);
108: }
109: 
110: /// <summary>
111: /// Write a string with variable arguments to the logger
112: /// </summary>
113: /// <param name="source">Source name or file name</param>
114: /// <param name="line">Source line number</param>
115: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
116: /// <param name="message">Formatted message string</param>
117: /// <param name="args">Variable argument list</param>
118: void Logger::WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args)
119: {
120:     if (static_cast<int>(severity) > static_cast<int>(m_level))
121:         return;
122: 
123:     static const size_t BufferSize = 1024;
124:     char buffer[BufferSize]{};
125: 
126:     char sourceString[BufferSize]{};
127:     Format(sourceString, BufferSize, " (%s:%d)", source, line);
128: 
129:     char messageBuffer[BufferSize]{};
130:     FormatV(messageBuffer, BufferSize, message, args);
131: 
132:     switch (severity)
133:     {
134:     case LogSeverity::Panic:
135:         strncat(buffer, "!Panic!", BufferSize);
136:         break;
137:     case LogSeverity::Error:
138:         strncat(buffer, "Error  ", BufferSize);
139:         break;
140:     case LogSeverity::Warning:
141:         strncat(buffer, "Warning", BufferSize);
142:         break;
143:     case LogSeverity::Info:
144:         strncat(buffer, "Info   ", BufferSize);
145:         break;
146:     case LogSeverity::Debug:
147:         strncat(buffer, "Debug  ", BufferSize);
148:         break;
149:     }
150: 
151:     if (m_timer != nullptr)
152:     {
153:         const size_t TimeBufferSize = 32;
154:         char timeBuffer[TimeBufferSize]{};
155:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
156:         if (strlen(timeBuffer) > 0)
157:         {
158:             strncat(buffer, timeBuffer, BufferSize);
159:             strncat(buffer, " ", BufferSize);
160:         }
161:     }
162: 
163:     strncat(buffer, messageBuffer, BufferSize);
164:     strncat(buffer, sourceString, BufferSize);
165:     strncat(buffer, "\n", BufferSize);
166: 
167: #if BAREMETAL_COLOR_OUTPUT
168:     switch (severity)
169:     {
170:     case LogSeverity::Panic:
171:         m_console.Write(buffer, ConsoleColor::BrightRed);
172:         break;
173:     case LogSeverity::Error:
174:         m_console.Write(buffer, ConsoleColor::Red);
175:         break;
176:     case LogSeverity::Warning:
177:         m_console.Write(buffer, ConsoleColor::BrightYellow);
178:         break;
179:     case LogSeverity::Info:
180:         m_console.Write(buffer, ConsoleColor::Cyan);
181:         break;
182:     case LogSeverity::Debug:
183:         m_console.Write(buffer, ConsoleColor::Yellow);
184:         break;
185:     default:
186:         m_console.Write(buffer, ConsoleColor::White);
187:         break;
188:     }
189: #else
190:     m_console.Write(buffer);
191: #endif
192: 
193:     if (severity == LogSeverity::Panic)
194:     {
195:         GetSystem().Halt();
196:     }
197: }
198: 
199: /// <summary>
200: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
201: /// </summary>
202: /// <returns>Reference to the singleton logger instance</returns>
203: Logger &baremetal::GetLogger()
204: {
205:     static LogSeverity defaultSeverity{LogSeverity::Debug};
206:     static Logger      logger(defaultSeverity, &GetTimer());
207:     logger.Initialize();
208:     return logger;
209: }
```

- Line 55: We use the macro `LOG_MODULE` also internally to specify that we are in the `Logger` class itself, and we can use the `LOG*` macros
- Line 63-69: We implement the constructor
- Line 75-84: We implement the `Initialize()` method
  - Line 79: We use the function `SetupVersion()` from `Version.h` to set up the version string
  - Line 81: We use the `Logger` itself to log the first message, stating the platform name and its version
- Line 90-93: We implement the `SetLogLevel()` method. This simply set the maximum log level for filtering
- Line 102-108: We implement the `Write()` method. This simply call `WriteV()` after setting up the variable argument list
- Line 118-197: We implement the `WriteV()` method
  - Line 120-121: If the severity level passed in is to high (priority too low) we simply return without printing
  - Line 123-124: We define a buffer to hold the line to write
  - Line 126-127: We print source name and line number into a separate buffer
  - Line 129-130: We print the message with arguments into a separate buffer
  - Line 132-149: For each level, we add a string to the line buffer
  - Line 151-161: If a `Timer` was passed in, we request the current time, and print it into the line buffer.
We'll add the timer method in a minute
  - Line 163-165: We add the message, source information and end of line to the buffer
  - Line 167-191: Depending on whether we defined `BAREMETAL_COLOR_OUTPUT`, we either simply print the buffer without color, or we use a severity level specific color
  - Line 193-196: If the severity level is `Panic` we halt the system
- Line 203-209: We implement the friend function `GetLogger()` to retrieve the singleton instance of the logger.
As a default, we set the maximum log level to `Debug` meaning that everything is logged. We also use the singleton `Timer` instance

### Updating the Timer class {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_UPDATING_THE_TIMER_CLASS}

#### Timer.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_UPDATING_THE_TIMER_CLASS_TIMERH}

We need to add the method `GetTimeString()` to the `Timer` class.

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
72: public:
73:     Timer(IMemoryAccess& memoryAccess);
74:
75:     void GetTimeString(char* buffer, size_t bufferSize);
76:
77:     static void WaitCycles(uint32 numCycles);
78:
...
```

#### Timer.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_UPDATING_THE_TIMER_CLASS_TIMERCPP}

We then implement method `GetTimeString()` for the `Timer` class.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
75: /// <summary>
76: /// Write string representing current time according to our time zone to the buffer.
77: ///
78: /// For now returns an empty string
79: /// </summary>
80: /// <param name="buffer">Buffer to receive the time string</param>
81: /// <param name="bufferSize">Size of the buffer, to protect against overflowing the buffer</param>
82: void Timer::GetTimeString(char* buffer, size_t bufferSize)
83: {
84:     if (bufferSize > 0)
85:         *buffer = '\0';
86: }
87:
...
```

So for now we simply return an empty string. Updating the clock will take some more effort, which will be done later.

### Using the Logger class {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_USING_THE_LOGGER_CLASS}

#### System.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_USING_THE_LOGGER_CLASS_SYSTEMCPP}

We will be using the logger in the `sysinit()` function, to instanntiate it and print the first log message.
Also, we will print logging info in `Halt()` and `Reboot()`.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Logger.h>
45: #include <baremetal/MemoryAccess.h>
46: #include <baremetal/SysConfig.h>
47: #include <baremetal/Timer.h>
48: #include <baremetal/Util.h>
49: #include <baremetal/Version.h>
...
56: /// @brief Define log name for this module
57: LOG_MODULE("System");
58:
...
119: void System::Halt()
120: {
121:     LOG_INFO("Halt");
122:     Timer::WaitMilliSeconds(WaitTime);
...
142: void System::Reboot()
143: {
144:     LOG_INFO("Reboot");
145:     Timer::WaitMilliSeconds(WaitTime);
...
194:     SetupVersion();
195:
196:     GetLogger();
197:     LOG_INFO("Starting up");
198:
199:     extern int main();
```

- Line 44: We replace the include for `Console.h` with `Logger.h`
- Line 49: We add an include for `Version.h`
- Line 57: We set the module name for logging to `System`
- Line 121: We use `LOG_INFO` to log the message `Halt`
- Line 144: We use `LOG_INFO` to log the message `Reboot`
- Line 194: We call the function `SetupVersion()` to create the version string
- Line 196: We instantiate  the `Logger` singleton
- Line 197: We use `LOG_INFO` to log the message `Starting up`

### Update the application code {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_UPDATE_THE_APPLICATION_CODE}

We can now also update the application to use the logging macros.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/Logger.h>
5: #include <baremetal/Mailbox.h>
6: #include <baremetal/MemoryManager.h>
7: #include <baremetal/RPIProperties.h>
8: #include <baremetal/SysConfig.h>
9: #include <baremetal/Serialization.h>
10: #include <baremetal/System.h>
11: #include <baremetal/Timer.h>
F2:
13: LOG_MODULE("main");
14:
15: using namespace baremetal;
16:
17: int main()
18: {
19:     auto& console = GetConsole();
20:     LOG_DEBUG("Hello World!");
21:
22:     char buffer[128];
23:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
24:     RPIProperties properties(mailbox);
25:
26:     uint64 serial{};
27:     if (properties.GetBoardSerial(serial))
28:     {
29:         LOG_INFO("Mailbox call succeeded");
30:         LOG_INFO("Serial: %016llx", serial);
31:     }
32:     else
33:     {
34:         LOG_ERROR("Mailbox call failed");
35:     }
36:
37:     LOG_INFO("Wait 5 seconds");
38:     Timer::WaitMilliSeconds(5000);
39:
40:     console.Write("Press r to reboot, h to halt\n");
41:     char ch{};
42:     while ((ch != 'r') && (ch != 'h'))
43:     {
44:         ch = console.ReadChar();
45:         console.WriteChar(ch);
46:     }
47:
48:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
49: }
```

- Line 4: We add the include for `Logger.h`
- Line 13: We use `LOG_MODULE` to set the logging name to `main`
- Line 20: We use `LOG_DEBUG` in this case (to show the color differences) to log the `Hello World!` message
- Line 29-30: We can now use the variable arguments to shorten the logging for the serial number.
Notice that we use the format string `%016llx` to write a zero leading 16 digit hexedecimal representation of the serial number
- Line 34: We use `LOG_ERROR` to write an error message if the mailbox call fails
- Line 37: We use `LOG_INFO` to write the message

Notice also that we keep using the console, as we need to read and write characters from and to the console.

### Update project configuration {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
49:     )
50:
51: set(PROJECT_INCLUDES_PUBLIC
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
80:     )
81: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color, depending on the log severity level, and also print source name and line number:

<img src="images/tutorial-12-logger.png" alt="Console output" width="600"/>

## Assertion - Step 4 {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4}

It is handy to have a function or macro available to check for a condition, and panic if this is not fulfilled. Very similar to the standard C assert() macro.

So let's add this, and log a Panic message if the condition check fails.

### Assert.h {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_ASSERTH}

Create the file `code/libraries/baremetal/include/baremetal/Assert.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Assert.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Assert.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Assertion functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
42: #include <baremetal/Macros.h>
43: #include <baremetal/Types.h>
44:
45: /// @file
46: /// Assertion functions
47:
48: namespace baremetal {
49:
50: #ifdef NDEBUG
51: /// If building for release, assert is replaced by nothing
52: #define assert(expr) ((void)0)
53: #else
54: void AssertionFailed(const char *expression, const char *fileName, int lineNumber);
55:
56: /// @brief Assertion callback function, which can be installed to handle a failed assertion
57: using AssertionCallback = void(const char *expression, const char *fileName, int lineNumber);
58:
59: void ResetAssertionCallback();
60: void SetAssertionCallback(AssertionCallback* callback);
61:
62: /// @brief Assertion. If the assertion fails, AssertionFailed is called.
63: ///
64: /// <param name="expression">Expression to evaluate.
65: /// If true the assertion succeeds and nothing happens, if false the assertion fails, and the assertion failure handler is invoked.</param>
66: #define assert(expression) (likely(expression) ? ((void)0) : baremetal::AssertionFailed(#expression, __FILE__, __LINE__))
67:
68: #endif
69:
70: } // namespace baremetal
```

- Line 52: If we build for release, we simply ignore the assertion
- Line 54: We declare the assertion failure function `AssertionFailed()`.
This will call the assertion failure handler, which log a `Panic` message and halts the system by default
- Line 57: We declare the prototype of the assertion failure handler function `AssertionCallback`
- Line 59: We declare the function `ResetAssertionCallback()` which resets the assertion failure handler to the default
- Line 60: We declare the function `SetAssertionCallback()` which sets a custom assertion failure handler
- Line 66: We define the `assert()` macro. This will invoke the assertion failure function `AssertionFailed()` in case the assertion fails

### Assert.cpp {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_ASSERTCPP}

Create the file `code/libraries/baremetal/src/Assert.cpp`

```cpp
File: code/libraries/baremetal/src/Assert.cpp
File: d:\Projects\baremetal.github\tutorial\12-logger\code\libraries\baremetal\src\Assert.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Assert.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Assertion functions
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include <baremetal/Assert.h>
41: 
42: #include <baremetal/Logger.h>
43: #include <baremetal/System.h>
44: 
45: /// @file
46: /// Assertion functions implementation
47: 
48: /// @brief Define log name
49: LOG_MODULE("Assert");
50: 
51: namespace baremetal {
52: 
53: static void AssertionFailedDefault(const char* expression, const char* fileName, int lineNumber);
54: 
55: /// @brief Assertion callback function
56: ///
57: /// Set to the default assertion handler function at startup, but can be overriden
58: static AssertionCallback *s_callback = AssertionFailedDefault;
59: 
60: /// <summary>
61: /// Log assertion failure and halt, is not expected to return (but may if a different assertion failure function is set up)
62: /// </summary>
63: /// <param name="expression">Expression to be printed</param>
64: /// <param name="fileName">Filename of file causing the failed assertion</param>
65: /// <param name="lineNumber">Line number causing the failed assertion</param>
66: void AssertionFailed(const char *expression, const char *fileName, int lineNumber)
67: {
68:     if (s_callback != nullptr)
69:         s_callback(expression, fileName, lineNumber);
70: }
71: 
72: /// <summary>
73: /// Default failed assertion handler
74: /// </summary>
75: /// <param name="expression">Expression to be printed</param>
76: /// <param name="fileName">Filename of file causing the failed assertion</param>
77: /// <param name="lineNumber">Line number causing the failed assertion</param>
78: static void AssertionFailedDefault(const char *expression, const char *fileName, int lineNumber)
79: {
80:     GetLogger().Write(fileName, lineNumber, LogSeverity::Panic, "assertion failed: %s", expression);
81: }
82: 
83: /// <summary>
84: /// Reset the assertion failure handler to the default
85: /// </summary>
86: void ResetAssertionCallback()
87: {
88:     s_callback = AssertionFailedDefault;
89: }
90: 
91: /// <summary>
92: /// Sets up a custom assertion failure handler
93: /// </summary>
94: /// <param name="callback">Assertion failure handler</param>
95: void SetAssertionCallback(AssertionCallback* callback)
96: {
97:     s_callback = callback;
98: }
99: 
100: } // namespace baremetal
```

- Line 58: We define a static variable to point to the set assertion failure handler function, which is set to `AssertionFailedDefault()` by default
- Line 66-70: We implement the function `AssertionFailed()` which is called when the `assert()` macro fails.
This will call the set assertion failure handler function
- Line 78-81: We implement the default assertion failure handler function `AssertionFailedDefault()`, which logs a `Panic` message, and will then halt the system
- Line 86-89: We implement the function `ResetAssertionCallback()` which will reset the assertion failure handler function to default
- Line 95-98: We implement the function `SetAssertionCallback()` which will set a custom assertion failure handler function

### Macros.h {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_MACROSH}

We use the construct `likely()` in the `assert()` macro. This needs to be defined.

Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
53: /// @brief Make branch prediction expect exp to be true (GCC compiler only)
54: /// @param exp Expression to be evaluated
55: #define likely(exp)         __builtin_expect (!!(exp), 1)
56: /// @brief Make branch prediction expect exp to be false (GCC compiler only)
57: /// @param exp Expression to be evaluated
58: #define unlikely(exp)       __builtin_expect (!!(exp), 0)
59:
```

These macros make use of builtin functionality in the compiler to influence the branch prediction.

### Update application code {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_UPDATE_APPLICATION_CODE}

Let us for the sake of demonstration add a failing assertion to the application code.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/RPIProperties.h>
9: #include <baremetal/SysConfig.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/System.h>
12: #include <baremetal/Timer.h>
13:
14: LOG_MODULE("main");
15:
16: using namespace baremetal;
17:
18: int main()
19: {
20:     auto& console = GetConsole();
21:     LOG_DEBUG("Hello World!");
22:
23:     char buffer[128];
24:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
25:     RPIProperties properties(mailbox);
26:
27:     uint64 serial{};
28:     if (properties.GetBoardSerial(serial))
29:     {
30:         LOG_INFO("Mailbox call succeeded");
31:         LOG_INFO("Serial: %016llx", serial);
32:     }
33:     else
34:     {
35:         LOG_ERROR("Mailbox call failed");
36:     }
37:
38:     LOG_INFO("Wait 5 seconds");
39:     Timer::WaitMilliSeconds(5000);
40:
41:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
42:     char ch{};
43:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
44:     {
45:         ch = console.ReadChar();
46:         console.WriteChar(ch);
47:     }
48:     if (ch == 'p')
49:         assert(false);
50:
51:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
52: }
```

- Line 2: We add the include for `Assert.h`
- Line 41: We print a different message, adding the input `p` to force a panic
- Line 48-49: If `p` was pressed, we perform a failed assertion

### Update project configuration {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
50:     )
51:
52: set(PROJECT_INCLUDES_PUBLIC
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
82:     )
83: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_ASSERTION__STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

When pressing the `p` key, the application will assert and halt:

<img src="images/tutorial-12-assert.png" alt="Console output" width="700"/>

We can now start adding sanity checks that fail assertion if not successful.

Next: [13-board-information](13-board-information.md)

