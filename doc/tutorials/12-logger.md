# Tutorial 12: Logger {#TUTORIAL_12_LOGGER}

@tableofcontents

## New tutorial setup {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_12_LOGGER_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-12.a`
- a library `output/Debug/lib/stdlib-12.a`
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
Later on, we can add the screen or maybe a file as a device for logging as well.
In order to enable writing variable argument lists, we will need to use strings, which grow automatically as needed.
This then means we need to be able to allocate heap memory.
Also, we will need a way to retrieve the current time.

So, all in all, quite some work to do.

## Console - Step 1 {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1}

The first step we need to take is get hold of a console, depending on what is set as the default.
We will add a definition in the root CMake file to select whether this is is UART0 or UART1, and retrieve a console based on this.
The console will have some additional functionality, such as enabling the use of ANSI colors.

### CMakeSettings.json {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_CMAKESETTINGSJSON}

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
5:             "name": "BareMetal-RPI3-Debug",
...
10:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=ON -DBAREMETAL_CONSOLE_UART0=ON",
...
17:             "name": "BareMetal-RPI4-Debug",
...
22:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
29:             "name": "BareMetal-RPI5-Debug",
...
34:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
41:             "name": "BareMetal-RPI3-Release",
...
46:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
53:             "name": "BareMetal-RPI4-Release",
...
58:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
65:             "name": "BareMetal-RPI5-Release",
...
70:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
77:             "name": "BareMetal-RPI3-RelWithDebInfo",
...
82:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
89:             "name": "BareMetal-RPI4-RelWithDebInfo",
...
94:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
101:             "name": "BareMetal-RPI5-RelWithDebInfo",
...
106:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
113:             "name": "BareMetal-RPI3-MinSizeRel",
...
118:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
125:             "name": "BareMetal-RPI4-MinSizeRel",
...
130:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
137:             "name": "BareMetal-RPI5-MinSizeRel",
...
142:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF -DBAREMETAL_CONSOLE_UART0=ON",
...
```

In other words, in all configurations we will add `-DBAREMETAL_CONSOLE_UART0=ON` to make UART0 the default console.

### Root CMake file {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_ROOT_CMAKE_FILE}

Now, we'll define the new variables, and create definitions for our code accordingly.

Update `CMakeLists.txt`:

```cmake
File: CMakeLists.txt
...
47: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
48: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
49:
...
73: set(DEFINES_C
74:     PLATFORM_BAREMETAL
75:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
76:     )
77: if (BAREMETAL_CONSOLE_UART0)
78:     list(APPEND DEFINES_C BAREMETAL_CONSOLE_UART0)
79: endif()
80: if (BAREMETAL_CONSOLE_UART1)
81:     list(APPEND DEFINES_C BAREMETAL_CONSOLE_UART1)
82: endif()
...
237:
238: message(STATUS "Baremetal settings:")
239: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
240: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
241: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
242: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
243: message(STATUS "-- Debug output to UART0:            ${BAREMETAL_CONSOLE_UART0}")
244: message(STATUS "-- Debug output to UART1:            ${BAREMETAL_CONSOLE_UART1}")
...
```

- Line 47-48: We define the two new variables, and set them to off by default
- Line 77-79: If `BAREMETAL_CONSOLE_UART0` is set to on, We add the definition `BAREMETAL_CONSOLE_UART0`
- Line 80-82: If `BAREMETAL_CONSOLE_UART1` is set to on, We add the definition `BAREMETAL_CONSOLE_UART1`
- Line 238-244: We also print the current settings for all relevant variables

### Console.h {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_CONSOLEH}

We will now add the console class declaration.

Create the file `code/libraries/baremetal/include/baremetal/Console.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Console.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
43: #include <baremetal/CharDevice.h>
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
- Line 94-120: We declare the `Console` class
  - Line 100: We declare the `GetConsole()` function as a friend to the class.
  This will again create a singleton instance
  - Line 104: We declare a member variable to hold the actual character device used for the console.
  In this case we use a pointer so that we can also point to nothing
  - Line 106: We declare the constructor, which is private, so `GetConsole()` needs to be called to create an instance
  - Line 109: We declare the method `AssignDevice()` to assign or reassign a device to the console
  - Line 110: We declare the method `SetTerminalColor()` to set the foreground and background color for the console.
  This outputs ANSI color codes
  - Line 111: We declare the method `ResetTerminalColor()` to reset the foreground and background color for the console.
  This outputs ANSI color codes
  - Line 113: We declare the method `Write()` to write a string with the foreground and background color specified.
  This outputs ANSI color codes
  - Line 114: We declare the method `Write()` to write a string without setting the color
  - Line 116: We declare the method `ReadChar()` to read a character from the console
  - Line 117: We declare the method `WriteChar()` to write a character to the console
- Line 122: We declare the accessor `GetConsole()` for the `Console` class. This will create an instance if needed, initialize it, and return a reference.

### Console.cpp {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_CONSOLECPP}

Create the file `code/libraries/baremetal/src/Console.cpp`:

```cpp
File: code/libraries/baremetal/src/Console.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/Console.h>
41:
42: #include <stdlib/Util.h>
43: #include <baremetal/Serialization.h>
44: #include <baremetal/Timer.h>
45: #include <baremetal/UART0.h>
46: #include <baremetal/UART1.h>
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

- Line 58-89: Implements a conversion from a color to an ANSI color code.
Note that normal and bright colors use the same code, but have a different prefix.
- Line 95-98: Implements the constructor.
- Line 104-107: Implements the method `AssignDevice()`.
- Line 114-142: Implements the method `SetTerminalColor()`. Here we see that normal and light foreground colors use a different prefix, and similarly for background colors.
- Line 147-150: Implements the method `ResetTerminalColor()`, which simply calls `SetTerminalColor()` with default arguments.
- Line 158-173: Implements the `Write()` method for a string with color specification.
This is simply a combination of setting colors, writing the string, and resetting colors.
The only exception is that we wait for the bool inUse to become false. This is a very simple locking mechanism that will prevent multiple cores writing to the console at the same time, to organize output a bit better.
There are better ways of doing this, e.g using atomics, however we don't have these yet. We'll get to that when we actually start using multiple cores.
- Line 179-188: Implements the `Write()` method for a string with no color specification.
Notice that this is very similar to writing a string in `UART0` and `UART1`.
- Line 194-202: Implements the `ReadChar()` method
- Line 208-214: Implements the `WriteChar()` method
- Line 222-234: Implements the `GetConsole()` function.
Notice how we define a static UART0 instance when `BAREMETAL_CONSOLE_UART0` is defined,
otherwise we define a static UART1 instance when `BAREMETAL_CONSOLE_UART1` is defined,
and if both are not defined, we set the console to a nullptr device.

### Update application code {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_UPDATE_APPLICATION_CODE}

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
28:         Serialize(buffer, sizeof(buffer), serial, 16, 16, false, true);
29:         console.Write(buffer);
30:         console.Write("\n");
31:     }
32:     else
33:     {
34:         console.Write("Mailbox call failed\n", ConsoleColor::Red);
35:     }
36:
37:     console.Write("Wait 5 seconds\n");
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

- Line 16: We retrieve the singleton Console instance
- Line 17: We use the console to print the `Hello World!` string, but now in color
- Line 26-27: We again print the string through the console, in default color
- Line 28: We serialize the serial number to a buffer
- Line 29: We print the contents of the buffer to the console
- Line 30: We print the literal string through the console, in default color
- Line 34: We print the literal string through the console, in red this time to flag failure
- Line 37-46: We again use the console instead of the UART0 instance

### System.cpp {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_SYSTEMCPP}

As we switch the main application to the console, we should also switch the code in `System.cpp` to the console, otherwise might be using a different device for output, with strange effects.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include <stdlib/Util.h>
43: #include <baremetal/ARMInstructions.h>
44: #include <baremetal/BCMRegisters.h>
45: #include <baremetal/Console.h>
46: #include <baremetal/MemoryAccess.h>
47: #include <baremetal/SysConfig.h>
48: #include <baremetal/Timer.h>
...
115: void System::Halt()
116: {
117:     GetConsole().Write("Halt\n", ConsoleColor::Cyan);
118:     Timer::WaitMilliSeconds(WaitTime);
...
138: void System::Reboot()
139: {
140:     GetConsole().Write("Reboot\n", ConsoleColor::Cyan);
141:     Timer::WaitMilliSeconds(WaitTime);
...
190:     GetConsole().Write("Starting up\n", ConsoleColor::Cyan);
191:
192:     extern int main();
...
```

Note that line numbers have changed due to the `Doxygen` comments that were added.

- Line 45: We include the Console header
- Line 117: We write to the console, in cyan color
- Line 140: We write to the console, in cyan color
- Line 190: We write to the console, in cyan color

### Update project configuration {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_UPDATE_PROJECT_CONFIGURATION}

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

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_CONSOLE___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color:

<img src="images/tutorial-12-console.png" alt="Console output" width="400"/>

## Printing a formatted string - Step 2 {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_2}

In order to introduce the actual logging functionality, we would like to be able to print using variable arguments,
much like the standard C `printf()` function.

For variable arguments, we need to be able to handle these. Normally we would have functions or definitions such as `va_start`, `va_end` and `va_arg` for this, by including `stdarg.h`.
Luckily, GCC offers these as builtin functions.
We will then need to implement formatting of strings to a buffer, using format strings like `printf()` uses.
We'll also want to support printing version information, so we'll provide for a way to pass on the version in the build, and make a string out of it.
Finally we can then add the logger class.

### StdArg.h {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_2_STDARGH}

So let's add a header to act as the standard C library's `stdarg.h` would.

Create the file `code/libraries/stdlib/include/stdlib/StdArg.h`

```cpp
File: code/libraries/stdlib/include/stdlib/StdArg.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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

### Format.h {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_2_FORMATH}

We'll define two variants of a formatting function for printing to a string with variable arguments.
One using the ellipsis argument `...`, the other with a `va_list` argument.

Create the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: /// @file
43: /// Formatting functionality
44:
45: #include <stdlib/StdArg.h>
46: #include <stdlib/Types.h>
47:
48: namespace baremetal {
49:
50: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args);
51: void Format(char* buffer, size_t bufferSize, const char* format, ...);
52:
53: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_12_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_2_FORMATCPP}

Now we will implement the formatting functions.

Be aware that this is a poor man's implementation, but it will do for now.

Create the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/Format.h>
41:
42: #include <stdlib/Util.h>
43: #include <baremetal/Serialization.h>
44:
45: namespace baremetal {
46:
47: const size_t BufferSize = 4096;
48:
49: /// <summary>
50: /// Append a character to the buffer
51: /// </summary>
52: /// <param name="buffer">Buffer to write to</param>
53: /// <param name="bufferSize">Size of the buffer</param>
54: /// <param name="c">Character to append</param>
55: static void Append(char* buffer, size_t bufferSize, char c)
56: {
57:     size_t len = strlen(buffer);
58:     char* p = buffer + len;
59:     if (static_cast<size_t>(p - buffer) < bufferSize)
60:     {
61:         *p++ = c;
62:     }
63:     if (static_cast<size_t>(p - buffer) < bufferSize)
64:     {
65:         *p = '\0';
66:     }
67: }
68:
69: /// <summary>
70: /// Append a set of identical characters to the buffer
71: /// </summary>
72: /// <param name="buffer">Buffer to write to</param>
73: /// <param name="bufferSize">Size of the buffer</param>
74: /// <param name="count">Number of characters to append</param>
75: /// <param name="c">Character to append</param>
76: static void Append(char* buffer, size_t bufferSize, size_t count, char c)
77: {
78:     size_t len = strlen(buffer);
79:     char* p = buffer + len;
80:     while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
81:     {
82:         *p++ = c;
83:         --count;
84:     }
85:     if (static_cast<size_t>(p - buffer) < bufferSize)
86:     {
87:         *p = '\0';
88:     }
89: }
90:
91: /// <summary>
92: /// Append a string to the buffer
93: /// </summary>
94: /// <param name="buffer">Buffer to write to</param>
95: /// <param name="bufferSize">Size of the buffer</param>
96: /// <param name="str">String to append</param>
97: static void Append(char* buffer, size_t bufferSize, const char* str)
98: {
99:     strncat(buffer, str, bufferSize);
100: }
101:
102: void Format(char* buffer, size_t bufferSize, const char* format, ...)
103: {
104:     va_list var;
105:     va_start(var, format);
106:
107:     FormatV(buffer, bufferSize, format, var);
108:
109:     va_end(var);
110: }
111:
112: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args)
113: {
114:     buffer[0] = '\0';
115:
116:     while (*format != '\0')
117:     {
118:         if (*format == '%')
119:         {
120:             if (*++format == '%')
121:             {
122:                 Append(buffer, bufferSize, '%');
123:                 format++;
124:                 continue;
125:             }
126:
127:             bool alternate = false;
128:             if (*format == '#')
129:             {
130:                 alternate = true;
131:                 format++;
132:             }
133:
134:             bool left = false;
135:             if (*format == '-')
136:             {
137:                 left = true;
138:                 format++;
139:             }
140:
141:             bool leadingZero = false;
142:             if (*format == '0')
143:             {
144:                 leadingZero = true;
145:                 format++;
146:             }
147:
148:             size_t width = 0;
149:             while (('0' <= *format) && (*format <= '9'))
150:             {
151:                 width = width * 10 + (*format - '0');
152:                 format++;
153:             }
154:
155:             unsigned precision = 6;
156:             if (*format == '.')
157:             {
158:                 format++;
159:                 precision = 0;
160:                 while ('0' <= *format && *format <= '9')
161:                 {
162:                     precision = precision * 10 + (*format - '0');
163:
164:                     format++;
165:                 }
166:             }
167:
168:             bool haveLong{};
169:             bool haveLongLong{};
170:
171:             if (*format == 'l')
172:             {
173:                 if (*(format + 1) == 'l')
174:                 {
175:                     haveLongLong = true;
176:
177:                     format++;
178:                 }
179:                 else
180:                 {
181:                     haveLong = true;
182:                 }
183:
184:                 format++;
185:             }
186:
187:             switch (*format)
188:             {
189:             case 'c':
190:                 {
191:                     char ch = static_cast<char>(va_arg(args, int));
192:                     if (left)
193:                     {
194:                         Append(buffer, bufferSize, ch);
195:                         if (width > 1)
196:                         {
197:                             Append(buffer, bufferSize, width - 1, ' ');
198:                         }
199:                     }
200:                     else
201:                     {
202:                         if (width > 1)
203:                         {
204:                             Append(buffer, bufferSize, width - 1, ' ');
205:                         }
206:                         Append(buffer, bufferSize, ch);
207:                     }
208:                 }
209:                 break;
210:
211:             case 'd':
212:             case 'i':
213:                 if (haveLongLong)
214:                 {
215:                     char str[BufferSize]{};
216:                     Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
217:                     Append(buffer, bufferSize, str);
218:                 }
219:                 else if (haveLong)
220:                 {
221:                     char str[BufferSize]{};
222:                     Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
223:                     Append(buffer, bufferSize, str);
224:                 }
225:                 else
226:                 {
227:                     char str[BufferSize]{};
228:                     Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
229:                     Append(buffer, bufferSize, str);
230:                 }
231:                 break;
232:
233:             case 'f':
234:                 {
235:                     char str[BufferSize]{};
236:                     Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
237:                     Append(buffer, bufferSize, str);
238:                 }
239:                 break;
240:
241:             case 'b':
242:                 if (alternate)
243:                 {
244:                     Append(buffer, bufferSize, "0b");
245:                 }
246:                 if (haveLongLong)
247:                 {
248:                     char str[BufferSize]{};
249:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
250:                     Append(buffer, bufferSize, str);
251:                 }
252:                 else if (haveLong)
253:                 {
254:                     char str[BufferSize]{};
255:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
256:                     Append(buffer, bufferSize, str);
257:                 }
258:                 else
259:                 {
260:                     char str[BufferSize]{};
261:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
262:                     Append(buffer, bufferSize, str);
263:                 }
264:                 break;
265:
266:             case 'o':
267:                 if (alternate)
268:                 {
269:                     Append(buffer, bufferSize, '0');
270:                 }
271:                 if (haveLongLong)
272:                 {
273:                     char str[BufferSize]{};
274:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
275:                     Append(buffer, bufferSize, str);
276:                 }
277:                 else if (haveLong)
278:                 {
279:                     char str[BufferSize]{};
280:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
281:                     Append(buffer, bufferSize, str);
282:                 }
283:                 else
284:                 {
285:                     char str[BufferSize]{};
286:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
287:                     Append(buffer, bufferSize, str);
288:                 }
289:                 break;
290:
291:             case 's':
292:                 {
293:                     char str[BufferSize]{};
294:                     Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
295:                     Append(buffer, bufferSize, str);
296:                 }
297:                 break;
298:
299:             case 'u':
300:                 if (haveLongLong)
301:                 {
302:                     char str[BufferSize]{};
303:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
304:                     Append(buffer, bufferSize, str);
305:                 }
306:                 else if (haveLong)
307:                 {
308:                     char str[BufferSize]{};
309:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
310:                     Append(buffer, bufferSize, str);
311:                 }
312:                 else
313:                 {
314:                     char str[BufferSize]{};
315:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
316:                     Append(buffer, bufferSize, str);
317:                 }
318:                 break;
319:
320:             case 'x':
321:             case 'X':
322:                 if (alternate)
323:                 {
324:                     Append(buffer, bufferSize, "0x");
325:                 }
326:                 if (haveLongLong)
327:                 {
328:                     char str[BufferSize]{};
329:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
330:                     Append(buffer, bufferSize, str);
331:                 }
332:                 else if (haveLong)
333:                 {
334:                     char str[BufferSize]{};
335:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
336:                     Append(buffer, bufferSize, str);
337:                 }
338:                 else
339:                 {
340:                     char str[BufferSize]{};
341:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
342:                     Append(buffer, bufferSize, str);
343:                 }
344:                 break;
345:
346:             case 'p':
347:                 if (alternate)
348:                 {
349:                     Append(buffer, bufferSize, "0x");
350:                 }
351:                 {
352:                     char str[BufferSize]{};
353:                     Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
354:                     Append(buffer, bufferSize, str);
355:                 }
356:                 break;
357:
358:             default:
359:                 Append(buffer, bufferSize, '%');
360:                 Append(buffer, bufferSize, *format);
361:                 break;
362:             }
363:         }
364:         else
365:         {
366:             Append(buffer, bufferSize, *format);
367:         }
368:
369:         format++;
370:     }
371: }
372:
373: } // namespace baremetal
```

- Line 47: We define a buffer size, to define the buffer, but also to check against writing outside the buffer.
- Line 55-67: Implements the function `Append()` for writing a single character to the buffer, checked for overflow.
Here we use the standard function `srtrlen()` which we will need to define
- Line 76-89: Implements the function `Append()` for writing multiple instances of a single character to the buffer, checked for overflow.
Again this uses the standard function `strlen()`
- Line 97-100: Implements the function `Append()` for writing a string to the buffer, checked for overflow.
This uses a standard C function strncat, which we will need to define
- Line 102-110: Implements the `Format()` function, using the ellipsis operator.
This simple creates a `va_list` from the arguments, and calls the other version of `Format()`.
- Line 112-371: Implements the `Format()` function, using the `va_list` argument
  - Line 114: We make sure to have an empty buffer
  - Line 116-371: We scan through the format string
    - Line 118-363: If the format character is `%` this is a special format operator
      - Line 120-125: In case the format string holds `%%` we see this as verbatim `%`, so we add that character
      - Line 127-132: If the next character is `#` we see this as an alternative version, signalling to add the prefix (only for base 2, 8, 16 integers), and advance
      - Line 134-139: If the next character is `-` we left-align the value, and advance
      - Line 141-146: If the next character is `0` we use leading zeros to fill up the value to its normal length, and advance
      - Line 148-153: If more digits follow, e.g. `%12` or `%012`, we extract the width of the value to be printed, and advance
      - Line 155-166: If a decimal point follows, we expect this to be a floating point value, and expect the next digits to specify the length of the fraction.
We read all digits, calculate the fraction length, and advance. In case there is no decimal point, we set the default fraction length to 6
      - Line 168-185: If the next character is `l` this is seen as a specification for a `long` (32 bit) value (only for integer values).
If another `l` follows, we see this as a `long long` (64 bit) value
      - Line 189-209: If the format character is a `c` we print the value as a character, taking into account the alignment
      - Line 211-231: If the format character is a `d` or `i` we print the value as a signed integer, taking into account the `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 233-239: If the format character is a `f` we print the value as a double, taking into account the precision if set, as well as the width, alignment and whether to use leading zeros.
Note that this is different from normal `printf()` behaviour, where %f means float, and %lf means double
      - Line 241-264: If the format character is a `b` we print the value as a binary unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros.
Note that this is an addition to `printf()` behaviour
      - Line 266-289: If the format character is a `o` we print the value as a octal unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros.
Note that this is an addition to `printf()` behaviour
      - Line 291-297: If the format character is a `s` we print the value as a string, taking into account the alignment and width
      - Line 299-318: If the format character is a `u` we print the value as a decimal unsigned integer, taking into account the `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 320-344: If the format character is a `x` or `X` we print the value as a hexadecimal unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 346-356: If the format character is a `p` we print the value as a pointer, meaning it is printed as a 64 bit unsigned integer in hexadecimal, taking into account the `#`, as well as the width, alignment and whether to use leading zeros
      - Line 358-361: Otherwise we simple print `%` and the character
      - Line 364-367: If the format character is not `%` we simply add the character

## Adding the Logger class - Step 3 {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3}

### Adding version information {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION}

In order to be able to print the version of our code, we will add some infrastructure to CMake, to pass definitions on to our code.
We will extract the latest tag from our repository, and use that as the version number. If there is no tag, we will assume the version is `0.0.0`.

#### Updating the root CMake file {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_UPDATING_THE_ROOT_CMAKE_FILE}

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
19: if (NOT "${GIT_TAG}" STREQUAL "")
20:     string(REGEX MATCH "^[0-9]*\.[0-9]*\.[0-9]*" VERSION_NUMBER ${GIT_TAG})
21: endif()
22:
23: if("${VERSION_NUMBER}" STREQUAL "")
24:     set(VERSION_NUMBER 0.0.0)
25: endif()
26:
27: parse_version(VERSION_NUMBER VERSION_MAJOR VERSION_MINOR VERSION_LEVEL VERSION_BUILD)
28:
29: set(VERSION_COMPOSED ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}.${VERSION_BUILD})
30: message(STATUS "Version: ${VERSION_COMPOSED}")
31:
...
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
...
91: if (BAREMETAL_COLOR_LOGGING)
92:     set(BAREMETAL_COLOR_OUTPUT 1)
93: else ()
94:     set(BAREMETAL_COLOR_OUTPUT 0)
95: endif()
96: set(BAREMETAL_LOAD_ADDRESS 0x80000)
97:
98: set(DEFINES_C
99:     PLATFORM_BAREMETAL
100:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
101:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
102:     BAREMETAL_MAJOR=${VERSION_MAJOR}
103:     BAREMETAL_MINOR=${VERSION_MINOR}
104:     BAREMETAL_LEVEL=${VERSION_LEVEL}
105:     BAREMETAL_BUILD=${VERSION_BUILD}
106:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
107:     )
108:
...
274: message(STATUS "-- Debug output to UART0:            ${BAREMETAL_CONSOLE_UART0}")
275: message(STATUS "-- Debug output to UART1:            ${BAREMETAL_CONSOLE_UART1}")
276: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
277: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
278: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
279: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
280: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
281: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
282:
```

- Line 13: We include the CMake utility functions
- Line 15: We use a utility function (to be defined) `get_git_tag` to retrieve the latest tag in `Git` and store it in variable `GIT_TAG`
- Line 17: We print the tag retrieved
- Line 19-21: If we found a tag, we match it to the pattern `{digit}+.{digit}+.{digit}+` and ignore the rest. This is stored in the variable `VERSION_NUMBER`
- Line 23-25: If the version extracted is empty, we set it to the default `0.0.0`
- Line 27: We use a utility function (to be defined) `parse_version` to extract the components from the version.
The components are stored respectively in variables `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_LEVEL` and `VERSION_BUILD`.
Notice that we have 4 components to the version, so for now the last one will always be equal to 0
- Line 29-30: We recompose the version from the components into variable `VERSION_COMPOSED`, and print it
- Line 68: We define the variable `BAREMETAL_COLOR_LOGGING` which is `ON` by default
- Line 91-95: If `BAREMETAL_COLOR_LOGGING` is `ON`, we set the variable `BAREMETAL_COLOR_OUTPUT` to 1, otherwise we set it to 0
- Line 98-107: We add compiler definitions using the version components, the composed version, and the ANSI color log output selection
- Line 269-281: We print the variable settings for ANSI color log output, and version information

#### Adding functionality to the CMake utility file {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_ADDING_FUNCTIONALITY_TO_THE_CMAKE_UTILITY_FILE}

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
71:         message(SEND_ERROR "Incorrectly specified version number: ${version_number}")
72:     endif()
73: endfunction()
74:
...
```

- Line 18-34: We define the function `get_git_tag` which returns the latest tag
  - Line 19-23: Depending on the platform, we determine the executable for git.
On Linux, we can simple try to find the package, on Windows, we assume that it is installed in `C:\Program Files\git`.
  - Line 24-28: We run git to get tags matching `<digit>+.<digit>+.<digit>+` and take the one from `HEAD`, i.e. the latest version.
This tag is stored in variable `OUTPUT`, the variable `RESULT` is used to store the execution return code
  - Line 29-33: If the command was executed without problems (`RESULT` is equal to 0), we set the output variable to `OUTPUT` (we used `PARENT_SCOPE` to return the value to outside the function).
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

#### Version.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_VERSIONH}

Now that we have set compiler definitions for the version, we can use them in code.

Create the file `code/libraries/baremetal/include/baremetal/Version.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Version.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
52: /// @brief Level version number (specified by define at compile time)
53: #define BAREMETAL_LEVEL_VERSION     BAREMETAL_LEVEL
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

#### Version.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_VERSIONCPP}

We need to implement the functions we just declared.

Create the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/Version.h>
41:
42: #include <stdlib/Util.h>
43: #include <baremetal/Format.h>
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
65:         Format(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_LEVEL_VERSION);
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

- Line 49-51: We define the local variable `s_baremetalVersionString` and its size
- Line 61-68: We implement `SetupVersion()`, which prints a formatted string to `s_baremetalVersionString`
- Line 74-77: We implement `GetVersion()`, which simply returns the string `s_baremetalVersionString`

#### Logger.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_LOGGERH}

We'll now add the `Logger` class, which can be used to log to the console, at different log levels.

Create the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include <stdlib/StdArg.h>
43: #include <stdlib/Types.h>
44: #include <baremetal/Console.h>
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
83:     bool        m_isInitialized;
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
- Line 73-99: We declare the `Logger` class
  - Line 79: We declare the `GetLogger()` function as a friend.
As before, this is the way to instantiate and retrieve the singleton
  - Line 83: The member variable `m_isInitialized` is used to guard against multiple initializations
  - Line 85: We keep a pointer in the member variable `m_timer` to the `Timer` instance which will be used to request the current time for logging
  - Line 87: We keep a reference in the member variable `m_console` to the `Console` instance
  - Line 89: The member variable `m_level` is used as comparison level for logging.
  Any log statements with a priority equal to or higher than `m_level` (with a value equal to or lower than `m_level`) will be shown, others will be ignored
  - Line 91: We declare the constructor for `Logger`
  - Line 94: The method `Initialize()` initializes the logger, guarded by `m_isInitialized`
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

#### Logger.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_ADDING_VERSION_INFORMATION_LOGGERCPP}

We will add the implementation for the `Logger` class.

Create the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/Logger.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/Console.h>
44: #include <baremetal/Format.h>
45: #include <baremetal/System.h>
46: #include <baremetal/Timer.h>
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
64:     : m_isInitialized{}
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
77:     if (m_isInitialized)
78:         return true;
79:     SetupVersion();
80:     m_isInitialized = true; // Stop reentrant calls from happening
81:     LOG_INFO(BAREMETAL_NAME " %s started on %s %s (AArch64)", BAREMETAL_VERSION_STRING, "Raspberry Pi", BAREMETAL_RPI_TARGET);
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
  - Line 81: We use the `Logger` itself to log the first message, stating the platform name, the target platform and its version
- Line 90-93: We implement the `SetLogLevel()` method. This simply set the maximum log level for filtering
- Line 102-108: We implement the `Write()` method. This simply calls `WriteV()` after setting up the variable argument list
- Line 118-197: We implement the `WriteV()` method
  - Line 120-121: If the severity level passed in is too high (priority too low) we simply return without printing
  - Line 123-124: We define a buffer to hold the line to write
  - Line 126-127: We print source name and line number into a separate buffer
  - Line 129-130: We print the message with arguments into a separate buffer
  - Line 132-149: For each level, we add a string to the line buffer denoting the severity level
  - Line 151-161: If a `Timer` was passed in, we request the current time, and print it into the line buffer.
We'll add the timer method in a minute
  - Line 163-165: We add the message, source information and end of line to the buffer
  - Line 167-191: Depending on whether we defined `BAREMETAL_COLOR_OUTPUT`, we either simply print the buffer without color, or we use a severity level specific color
  - Line 193-196: If the severity level is `Panic` we halt the system
- Line 203-209: We implement the friend function `GetLogger()` to retrieve the singleton instance of the logger.
As a default, we set the maximum log level to `Debug` meaning that everything is logged. We also use the singleton `Timer` instance

### Updating the Timer class {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_TIMER_CLASS}

#### Timer.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_TIMER_CLASS_TIMERH}

We need to add the method `GetTimeString()` to the `Timer` class.

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
73: public:
74:     Timer(IMemoryAccess &memoryAccess);
75: 
76:     void GetTimeString(char* buffer, size_t bufferSize);
77: 
78:     static void WaitCycles(uint32 numCycles);
79: 
...
```

#### Timer.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_TIMER_CLASS_TIMERCPP}

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

### Extending the serialization functions {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_EXTENDING_THE_SERIALIZATION_FUNCTIONS}

#### Serialization.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_EXTENDING_THE_SERIALIZATION_FUNCTIONS_SERIALIZATIONH}

As we wish to convert more types to string due to the variable arguments, we need to extend the set of serialization functions.
We'll be adding functions for `uint8`, `int32`, `int64`, `long long`, `unsigned long long`, `const char*` and `double`.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
47: namespace baremetal {
48: 
49: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros);
50: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
51: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros);
52: 
53: void Serialize(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros);
54: void Serialize(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros);
55: /// <summary>
56: /// Serialize long long int value, type specific specialization
57: /// </summary>
58: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
59: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
60: /// <param name="value">Value to be serialized</param>
61: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
62: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
63: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
64: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
65: inline void Serialize(char* buffer, size_t bufferSize, long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
66: {
67:     Serialize(buffer, bufferSize, static_cast<int64>(value), width, base, showBase, leadingZeros);
68: }
69: /// <summary>
70: /// Serialize unsigned long long int value, type specific specialization
71: /// </summary>
72: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
73: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
74: /// <param name="value">Value to be serialized</param>
75: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
76: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
77: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
78: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
79: inline void Serialize(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
80: {
81:     Serialize(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
82: }
83: void Serialize(char* buffer, size_t bufferSize, const char* value, int width, bool quote);
84: void Serialize(char* buffer, size_t bufferSize, double value, int width, int precision);
85: 
86: } // namespace baremetal
```

- Line 49: We declare a serialization function for uint8 (unsigned 8 bit integer)
- Line 53: We declare a serialization function for int32 (signed 32 bit integer)
- Line 54: We declare a serialization function for int64 (signed 64 bit integer)
- Line 65-68: We declare a serialization function for long long (this is mostly equal to int64, but the compiler sees it as a different type)
- Line 79-82: We declare a serialization function for unsigned long long (this is mostly equal to uint64, but the compiler sees it as a different type)
- Line 83: We declare a serialization function for const char* (for strings)
- Line 84: We declare a serialization function for double

#### Serialization.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_EXTENDING_THE_SERIALIZATION_FUNCTIONS_SERIALIZATIONCPP}

Let's implement the new functions.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
47: namespace baremetal {
48: 
49: /// @brief Write characters with base above 10 as uppercase or not
50: static bool           Uppercase = true;
51: 
52: static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
53: static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
54: 
55: /// <summary>
56: /// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
57: /// </summary>
58: /// <param name="value">Digit value</param>
59: /// <returns>Converted digit character</returns>
60: static constexpr char GetDigit(uint8 value)
61: {
62:     return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
63: }
64: 
65: /// <summary>
66: /// Calculated the amount of digits needed to represent an unsigned value of bits using base
67: /// </summary>
68: /// <param name="bits">Size of integer in bits</param>
69: /// <param name="base">Base to be used</param>
70: /// <returns>Maximum amount of digits needed</returns>
71: static constexpr int BitsToDigits(int bits, int base)
72: {
73:     int result = 0;
74:     uint64 value = 0xFFFFFFFFFFFFFFFF;
75:     if (bits < 64)
76:         value &= ((1ULL << bits) - 1);
77: 
78:     while (value > 0)
79:     {
80:         value /= base;
81:         result++;
82:     }
83: 
84:     return result;
85: }
86: 
87: /// <summary>
88: /// Serialize a 8 bit unsigned value to buffer.
89: ///
90: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
91: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
92: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
93: ///
94: /// Base is the digit base, which can range from 2 to 36.
95: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
96: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
97: /// </summary>
98: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
99: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
100: /// <param name="value">Value to be serialized</param>
101: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
102: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
103: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
104: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
105: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros)
106: {
107:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 8);
108: }
109: 
110: /// <summary>
111: /// Serialize a 32 bit signed value to buffer.
112: ///
113: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
114: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
115: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
116: ///
117: /// Base is the digit base, which can range from 2 to 36.
118: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
119: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
120: /// </summary>
121: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
122: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
123: /// <param name="value">Value to be serialized</param>
124: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
125: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
126: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
127: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
128: void Serialize(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
129: {
130:     SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
131: }
132: 
133: /// <summary>
134: /// Serialize a 32 bit unsigned value to buffer.
135: ///
136: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
137: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
138: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
139: ///
140: /// Base is the digit base, which can range from 2 to 36.
141: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
142: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
143: /// </summary>
144: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
145: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
146: /// <param name="value">Value to be serialized</param>
147: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
148: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
149: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
150: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
151: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
152: {
153:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
154: }
155: 
156: /// <summary>
157: /// Serialize a 64 bit signed value to buffer.
158: ///
159: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
160: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
161: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
162: ///
163: /// Base is the digit base, which can range from 2 to 36.
164: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
165: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
166: /// </summary>
167: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
168: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
169: /// <param name="value">Value to be serialized</param>
170: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
171: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
172: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
173: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
174: void Serialize(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
175: {
176:     SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
177: }
178: 
179: /// <summary>
180: /// Serialize a 64 bit unsigned value to buffer.
181: ///
182: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
183: /// Width specifies the minimum width in characters. The value is always written right aligned.
184: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
185: ///
186: /// Base is the digit base, which can range from 2 to 36.
187: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
188: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
189: /// </summary>
190: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
191: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
192: /// <param name="value">Value to be serialized</param>
193: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
194: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
195: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
196: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
197: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
198: {
199:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
200: }
201: 
202: /// <summary>
203: /// Serialize a double value to buffer. The value will be printed as a fixed point number.
204: ///
205: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
206: /// Width is currently unused.
207: /// Precision specifies the number of digits behind the decimal pointer
208: /// </summary>
209: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
210: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
211: /// <param name="value">Value to be serialized</param>
212: /// <param name="width">Unused</param>
213: /// <param name="precision">Number of digits after the decimal point to use</param>
214: void Serialize(char* buffer, size_t bufferSize, double value, int width, int precision)
215: {
216:     bool negative{};
217:     if (value < 0)
218:     {
219:         negative = true;
220:         value = -value;
221:     }
222: 
223:     if (bufferSize == 0)
224:         return;
225: 
226:     // We can only print values with integral parts up to what uint64 can hold
227:     if (value > static_cast<double>(static_cast<uint64>(-1)))
228:     {
229:         strncpy(buffer, "overflow", bufferSize);
230:         return;
231:     }
232: 
233:     *buffer = '\0';
234:     if (negative)
235:         strncpy(buffer, "-", bufferSize);
236: 
237:     uint64 integralPart = static_cast<uint64>(value);
238:     const size_t TmpBufferSize = 32;
239:     char tmpBuffer[TmpBufferSize];
240:     Serialize(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
241:     strncat(buffer, tmpBuffer, bufferSize);
242:     const int MaxPrecision = 7;
243: 
244:     if (precision != 0)
245:     {
246:         strncat(buffer, ".", bufferSize);
247: 
248:         if (precision > MaxPrecision)
249:         {
250:             precision = MaxPrecision;
251:         }
252: 
253:         uint64 precisionPower10 = 1;
254:         for (int i = 1; i <= precision; i++)
255:         {
256:             precisionPower10 *= 10;
257:         }
258: 
259:         value -= static_cast<double>(integralPart);
260:         value *= static_cast<double>(precisionPower10);
261: 
262:         Serialize(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
263:         strncat(buffer, tmpBuffer, bufferSize);
264:         precision -= strlen(tmpBuffer);
265:         while (precision--)
266:         {
267:             strncat(buffer, "0", bufferSize);
268:         }
269:     }
270: }
271: 
272: /// <summary>
273: /// Serialize a const char * value to buffer. The value can be quoted.
274: ///
275: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
276: /// If quote is true, the string is printed within double quotes (\")
277: /// </summary>
278: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
279: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
280: /// <param name="value">String to be serialized</param>
281: /// <param name="width">Unused</param>
282: /// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
283: void Serialize(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
284: {
285:     size_t numChars = strlen(value);
286:     if (quote)
287:         numChars += 2;
288: 
289:     // Leave one character for \0
290:     if (numChars > bufferSize - 1)
291:         return;
292: 
293:     char* bufferPtr = buffer;
294: 
295:     if (quote)
296:         *bufferPtr++ = '\"';
297:     while (*value)
298:     {
299:         *bufferPtr++ = *value++;
300:     }
301:     if (quote)
302:         *bufferPtr++ = '\"';
303: }
304: 
305: /// <summary>
306: /// Internal serialization function, to be used for all signed values.
307: ///
308: /// Serialize a signed value to buffer.
309: ///
310: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
311: /// Width specifies the minimum width in characters. The value is always written right aligned.
312: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
313: ///
314: /// Base is the digit base, which can range from 2 to 36.
315: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
316: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
317: /// </summary>
318: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
319: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
320: /// <param name="value">Value to be serialized</param>
321: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
322: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
323: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
324: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
325: /// <param name="numBits">Specifies the number of bits used for the value</param>
326: static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
327: {
328:     if ((base < 2) || (base > 36))
329:         return;
330: 
331:     int       numDigits = 0;
332:     bool      negative = (value < 0);
333:     uint64    absVal = static_cast<uint64>(negative ? -value : value);
334:     uint64    divisor = 1;
335:     uint64    divisorLast = 1;
336:     size_t    absWidth = (width < 0) ? -width : width;
337:     const int maxDigits = BitsToDigits(numBits, base);
338:     while ((absVal >= divisor) && (numDigits <= maxDigits))
339:     {
340:         divisorLast = divisor;
341:         divisor *= base;
342:         ++numDigits;
343:     }
344:     divisor = divisorLast;
345: 
346:     size_t numChars = (numDigits > 0) ? numDigits : 1;
347:     if (showBase)
348:     {
349:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
350:     }
351:     if (negative)
352:     {
353:         numChars++;
354:     }
355:     if (absWidth > numChars)
356:         numChars = absWidth;
357:     // Leave one character for \0
358:     if (numChars > bufferSize - 1)
359:         return;
360: 
361:     char* bufferPtr = buffer;
362:     if (negative)
363:     {
364:         *bufferPtr++ = '-';
365:     }
366: 
367:     if (showBase)
368:     {
369:         if (base == 2)
370:         {
371:             *bufferPtr++ = '0';
372:             *bufferPtr++ = 'b';
373:         }
374:         else if (base == 8)
375:         {
376:             *bufferPtr++ = '0';
377:         }
378:         else if (base == 16)
379:         {
380:             *bufferPtr++ = '0';
381:             *bufferPtr++ = 'x';
382:         }
383:     }
384:     if (leadingZeros)
385:     {
386:         if (absWidth == 0)
387:             absWidth = maxDigits;
388:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
389:         {
390:             *bufferPtr++ = '0';
391:         }
392:     }
393:     else
394:     {
395:         if (numDigits == 0)
396:         {
397:             *bufferPtr++ = '0';
398:         }
399:     }
400:     while (numDigits > 0)
401:     {
402:         int digit = (absVal / divisor) % base;
403:         *bufferPtr++ = GetDigit(digit);
404:         --numDigits;
405:         divisor /= base;
406:     }
407:     *bufferPtr++ = '\0';
408: }
409: 
410: /// <summary>
411: /// Internal serialization function, to be used for all unsigned values.
412: ///
413: /// Serialize a unsigned value to buffer.
414: ///
415: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
416: /// Width specifies the minimum width in characters. The value is always written right aligned.
417: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
418: ///
419: /// Base is the digit base, which can range from 2 to 36.
420: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
421: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
422: /// </summary>
423: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
424: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
425: /// <param name="value">Value to be serialized</param>
426: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
427: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
428: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
429: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
430: /// <param name="numBits">Specifies the number of bits used for the value</param>
431: static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
432: {
433:     if ((base < 2) || (base > 36))
434:         return;
435: 
436:     int       numDigits = 0;
437:     uint64    divisor = 1;
438:     uint64    divisorLast = 1;
439:     uint64    divisorHigh = 0;
440:     size_t    absWidth = (width < 0) ? -width : width;
441:     const int maxDigits = BitsToDigits(numBits, base);
442:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
443:     {
444:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
445:         divisorLast = divisor;
446:         divisor *= base;
447:         ++numDigits;
448:     }
449:     divisor = divisorLast;
450: 
451:     size_t numChars = (numDigits > 0) ? numDigits : 1;
452:     if (showBase)
453:     {
454:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
455:     }
456:     if (absWidth > numChars)
457:         numChars = absWidth;
458:     // Leave one character for \0
459:     if (numChars > bufferSize - 1)
460:         return;
461: 
462:     char* bufferPtr = buffer;
463: 
464:     if (showBase)
465:     {
466:         if (base == 2)
467:         {
468:             *bufferPtr++ = '0';
469:             *bufferPtr++ = 'b';
470:         }
471:         else if (base == 8)
472:         {
473:             *bufferPtr++ = '0';
474:         }
475:         else if (base == 16)
476:         {
477:             *bufferPtr++ = '0';
478:             *bufferPtr++ = 'x';
479:         }
480:     }
481:     if (leadingZeros)
482:     {
483:         if (absWidth == 0)
484:             absWidth = maxDigits;
485:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
486:         {
487:             *bufferPtr++ = '0';
488:         }
489:     }
490:     else
491:     {
492:         if (numDigits == 0)
493:         {
494:             *bufferPtr++ = '0';
495:         }
496:     }
497:     while (numDigits > 0)
498:     {
499:         int digit = (value / divisor) % base;
500:         *bufferPtr++ = GetDigit(digit);
501:         --numDigits;
502:         divisor /= base;
503:     }
504:     *bufferPtr++ = '\0';
505: }
506: 
507: } // namespace baremetal
```

- Line 52: We declare a static function `SerializeInternalUInt()`, which is equal to the previous `SerializeInternal()` function.
- Line 53: We declare a static function `SerializeInternalInt()`, which is the version of `SerializeInternal()` for signed integers.
- Line 105-108: Serialization for uint8 uses `SerializeInternalUInt()`
- Line 128-131: Serialization for int32 uses `SerializeInternalInt()`
- Line 151-154: Serialization for uint32 now uses `SerializeInternalUInt()`
- Line 174-177: Serialization for int64 uses `SerializeInternalInt()`
- Line 197-200: Serialization for uint64 now uses `SerializeInternalUInt()`
- Line 214-270: We implement `Serialize()` for double.
Note that we use `strncpy()` and `strlen()` next to `strncat()`
- Line 283-303: We implement `Serialize()` for const char*
- Line 326-408: We implement `SerializeInternalInt()`.
The main difference with `SerializeInternalUInt()` is that we can have negative values.
Therefore we take the absolute value and set a flag for negative values
- Line 431-505: We implement `SerializeInternalUInt()`, which is the same as the previous `SerializeInternal()` function

### Updating the utility functions {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_UTILITY_FUNCTIONS}

#### Util.h {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_UTILITY_FUNCTIONS_UTILH}

We need to add the function `strncat()` for the `Logger` class, and `strlen()` and `strncpy()` for serialization.
You may recognize these as standard C functions.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
54: size_t strlen(const char* str);
55: char* strncpy(char* dest, const char* src, size_t maxLen);
56: char* strncat(char* dest, const char* src, size_t maxLen);
57:
```

#### Util.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATING_THE_UTILITY_FUNCTIONS_UTILCPP}

Let's implement the new functions.

Update the file `code/libraries/stdlib/src/Util.cpp`

```cpp
File: code/libraries/stdlib/src/Util.cpp
82: /// <summary>
83: /// Standard C strlen function. Calculates the length of a string, in other words the index to the first '\0' character
84: /// </summary>
85: /// <param name="str">String for which to calculate the length</param>
86: /// <returns>Length of the string</returns>
87: size_t strlen(const char* str)
88: {
89:     size_t result = 0;
90:
91:     while (*str++)
92:     {
93:         result++;
94:     }
95:
96:     return result;
97: }
98:
99: /// <summary>
100: /// Standard C strncpy function. Copies a string, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
101: /// </summary>
102: /// <param name="dest">Pointer to destination buffer</param>
103: /// <param name="src">Pointer to source buffer</param>
104: /// <param name="maxLen">Maximum number of characters to copy</param>
105: /// <returns>Pointer to destination buffer</returns>
106: char* strncpy(char* dest, const char* src, size_t maxLen)
107: {
108:     char* p = dest;
109:
110:     while (maxLen > 0)
111:     {
112:         if (*src == '\0')
113:         {
114:             break;
115:         }
116:
117:         *p++ = *src++;
118:         maxLen--;
119:     }
120:
121:     if (maxLen == 0)
122:     {
123:         p = dest + maxLen - 1;
124:     }
125:     if (p >= dest)
126:         *p = '\0';
127:
128:     return dest;
129: }
130:
131: /// <summary>
132: /// Standard C strncat function. Appends a string to the destination, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
133: /// </summary>
134: /// <param name="dest">Pointer to destination buffer</param>
135: /// <param name="src">Pointer to source buffer</param>
136: /// <param name="maxLen">Maximum number of characters to copy</param>
137: /// <returns>Pointer to destination buffer</returns>
138: char* strncat(char* dest, const char* src, size_t maxLen)
139: {
140:     char* p = dest;
141:
142:     while ((maxLen > 0) && *p)
143:     {
144:         p++;
145:         maxLen--;
146:     }
147:
148:     while ((maxLen > 0) && *src)
149:     {
150:         *p++ = *src++;
151:         maxLen--;
152:     }
153:
154:     if (maxLen == 0)
155:     {
156:         p = dest + maxLen - 1;
157:     }
158:     if (p >= dest)
159:         *p = '\0';
160:
161:     return dest;
162: }
163:
```

### Using the Logger class {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_USING_THE_LOGGER_CLASS}

#### System.cpp {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_USING_THE_LOGGER_CLASS_SYSTEMCPP}

We will be using the logger in the `sysinit()` function, to instantiate it and print the first log message.
Also, we will print logging info in `Halt()` and `Reboot()`.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include <stdlib/Util.h>
43: #include <baremetal/ARMInstructions.h>
44: #include <baremetal/BCMRegisters.h>
45: #include <baremetal/Logger.h>
46: #include <baremetal/MemoryAccess.h>
47: #include <baremetal/SysConfig.h>
48: #include <baremetal/Timer.h>
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
194:     GetLogger();
195:     LOG_INFO("Starting up");
196: 
197:     extern int main();
198: 
```

- Line 45: We replace the include for `Console.h` with `Logger.h`
- Line 49: We add an include for `Version.h`
- Line 57: We set the module name for logging to `System`
- Line 121: We use `LOG_INFO` to log the message `Halt`
- Line 144: We use `LOG_INFO` to log the message `Reboot`
- Line 194: We instantiate  the `Logger` singleton
- Line 195: We use `LOG_INFO` to log the message `Starting up`

### Update the application code {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATE_THE_APPLICATION_CODE}

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
12:
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
Notice that we use the format string `%016llx` to write a zero leading 16 digit hexedecimal representation of the serial number, using a signed 64 bit integer
- Line 34: We use `LOG_ERROR` to write an error message if the mailbox call fails
- Line 37: We use `LOG_INFO` to write the message

Notice also that we keep using the console, as we need to read and write characters from and to the console.

### Update project configuration {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
47:     )
48: 
49: set(PROJECT_INCLUDES_PUBLIC
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
73:     )
74: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color, depending on the log severity level, and also print source name and line number:

<img src="images/tutorial-12-logger.png" alt="Console output" width="800"/>

## Assertion - Step 4 {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4}

It is handy to have a function or macro available to check for a condition, and panic if this is not fulfilled. Very similar to the standard C assert() macro.

So let's add this, and log a Panic message if the condition check fails.

### Assert.h {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_ASSERTH}

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
This will call the assertion failure handler, which log a `Panic` message and halts the system by default, unless a custom asstion failure handler is installed
- Line 57: We declare the prototype of the assertion failure handler function `AssertionCallback`
- Line 59: We declare the function `ResetAssertionCallback()` which resets the assertion failure handler to the default
- Line 60: We declare the function `SetAssertionCallback()` which sets a custom assertion failure handler
- Line 66: We define the `assert()` macro. This will invoke the assertion failure function `AssertionFailed()` in case the assertion fails

### Assert.cpp {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_ASSERTCPP}

Create the file `code/libraries/baremetal/src/Assert.cpp`

```cpp
File: code/libraries/baremetal/src/Assert.cpp
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

- Line 53: We declare a static function `AssertionFailedDefault()` as the default assertion failure function
- Line 58: We define a static variable to point to the set assertion failure handler function, which is set to `AssertionFailedDefault()` by default
- Line 66-70: We implement the function `AssertionFailed()` which is called when the `assert()` macro fails.
This will call the set assertion failure handler function
- Line 78-81: We implement the default assertion failure handler function `AssertionFailedDefault()`, which logs a `Panic` message, and will then halt the system
- Line 86-89: We implement the function `ResetAssertionCallback()` which will reset the assertion failure handler function to default
- Line 95-98: We implement the function `SetAssertionCallback()` which will set a custom assertion failure handler function

### Macros.h {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_MACROSH}

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

### Update application code {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_UPDATE_APPLICATION_CODE}

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

### Update project configuration {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_UPDATE_PROJECT_CONFIGURATION}

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

### Configuring, building and debugging {#TUTORIAL_12_LOGGER_ASSERTION___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

When pressing the `p` key, the application will assert and halt:

<img src="images/tutorial-12-assert.png" alt="Console output" width="800"/>

We can now start adding sanity checks that fail assertion if not successful.

Next: [13-board-information](13-board-information.md)
