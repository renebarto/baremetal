# Tutorial 12: Console {#TUTORIAL_12_CONSOLE}

@tableofcontents

## New tutorial setup {#TUTORIAL_12_CONSOLE_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_12_CONSOLE_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-12.a`
- a library `output/Debug/lib/stdlib-12.a`
- an application `output/Debug/bin/12-logger.elf`
- an image in `deploy/Debug/12-logger-image`

## Adding a console {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE}

We can now write to either UART0 or UART1, and we have a common ancestor, `CharDevice`.
It would be nice if we could write to the console in a more generic way, and preferably in color if supported.

For this, we will need to be able to set a default console, by injecting an instance of a `CharDevice`.
Later on, we can add the screen or maybe a file as a device for logging as well.

The first step we need to take is get hold of a console, depending on what is set as the default.
We will add a definition in the root CMake file to select whether this is is UART0 or UART1, and retrieve a console based on this.
The console will have some additional functionality, such as enabling the use of ANSI colors.

### CMakeSettings.json {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_CMAKESETTINGSJSON}

We'll start by adding the option for a default console.
Were going to add two variables to CMake:

- `BAREMETAL_CONSOLE_UART0`
- `BAREMETAL_CONSOLE_UART1`

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

### Root CMake file {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_ROOT_CMAKE_FILE}

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

### Console.h {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_CONSOLEH}

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
42: #include "baremetal/Device.h"
43: #include "stdlib/Types.h"
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
58:     Black = 0,
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
97:     /// Return the singleton Console instance as a reference. This is a friend function of class Console
98:     /// </summary>
99:     /// <returns>Reference to the singleton Console instance</returns>
100:     friend Console& GetConsole();
101: 
102: private:
103:     /// @brief Default device to write to, specified in constructor. This device will be used when AssignDevice() is called with nullptr
104:     Device* m_defaultDevice;
105:     /// @brief Device to write to
106:     Device* m_device;
107:     /// @brief Singleton instance
108:     static Console* s_instance;
109: 
110: public:
111:     explicit Console(Device* device);
112:     void AssignDevice(Device* device);
113:     void SetTerminalColor(ConsoleColor foregroundColor = ConsoleColor::Default, ConsoleColor backgroundColor = ConsoleColor::Default);
114:     void ResetTerminalColor();
115: 
116:     void Write(const char* str, ConsoleColor foregroundColor, ConsoleColor backgroundColor = ConsoleColor::Default);
117:     void Write(const char* str);
118:     void Flush();
119: 
120:     char ReadChar();
121:     void WriteChar(char ch);
122: 
123: private:
124: };
125: 
126: Console& GetConsole();
127: 
128: } // namespace baremetal
129: 
```

- Line 54-89: We declare an enum class `ConsoleColor` for the foreground or background color
- Line 91-125: We declare the `Console` class
  - Line 100: We declare the `GetConsole()` function as a friend to the class.
  This will again create a singleton instance
  - Line 103-104: We declare a member variable `m_defaultDevice` to hold the default device used for the console.
  This device will be the default if no other device is assigned
  - Line 105-106: We declare a member variable `m_device` to hold the actual character device used for the console.
  In this case we use a pointer so that we can also point to nothing
  - Line 107-108: We declare a static member variable `s_instance` to hold the pointer to the singleton instance of the console.
  In this specific case setting up the console as a static local variable does not work
  - Line 111: We declare the constructor, this is not private as we need to specify a parameter
  - Line 112: We declare the method `AssignDevice()` to assign or reassign a device to the console
  - Line 113: We declare the method `SetTerminalColor()` to set the foreground and background color for the console.
  This outputs ANSI color codes
  - Line 114: We declare the method `ResetTerminalColor()` to reset the foreground and background color for the console.
  This outputs ANSI color codes
  - Line 116: We declare the method `Flush()` to flush the contents of the console buffers
  - Line 117: We declare the method `Write()` to write a string with the foreground and background color specified.
  This outputs ANSI color codes
  - Line 118: We declare the method `Write()` to write a string without setting the color
  - Line 120: We declare the method `ReadChar()` to read a character from the console
  - Line 121: We declare the method `WriteChar()` to write a character to the console
- Line 125: We declare the accessor `GetConsole()` for the `Console` class. This will return a reference to the console instance.
In this case a console instance needs to be created first.
This will be done in the `System` class later on

### Console.cpp {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_CONSOLECPP}

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
40: #include "baremetal/Console.h"
41: 
42: #include "baremetal/Serialization.h"
43: #include "baremetal/Timer.h"
44: #include "baremetal/UART0.h"
45: #include "baremetal/UART1.h"
46: #include "stdlib/Util.h"
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
58: static const char* GetAnsiColorCode(ConsoleColor color)
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
91: Console* Console::s_instance{};
92: 
93: /// <summary>
94: /// Create a console linked to the specified character device. Note that the constructor is private, so GetConsole() is needed to instantiate the console
95: /// </summary>
96: /// <param name="device">Character device used for output</param>
97: Console::Console(Device* device)
98:     : m_defaultDevice{device}
99:     , m_device{device}
100: {
101:     s_instance = this;
102: }
103: 
104: /// <summary>
105: /// Change the attached device
106: /// </summary>
107: /// <param name="device">Character device to be set to for output</param>
108: void Console::AssignDevice(Device* device)
109: {
110:     m_device = device;
111:     if (device == nullptr)
112:         m_device = m_defaultDevice;
113: }
114: 
115: /// <summary>
116: /// Set console foreground and background color (will output ANSI color codes)
117: /// </summary>
118: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
119: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
120: void Console::SetTerminalColor(ConsoleColor foregroundColor /*= ConsoleColor::Default*/, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
121: {
122:     Write("\033[0");
123:     if (foregroundColor != ConsoleColor::Default)
124:     {
125:         if (foregroundColor <= ConsoleColor::LightGray)
126:         {
127:             Write(";3");
128:         }
129:         else
130:         {
131:             Write(";9");
132:         }
133:         Write(GetAnsiColorCode(foregroundColor));
134:     }
135:     if (backgroundColor != ConsoleColor::Default)
136:     {
137:         if (backgroundColor <= ConsoleColor::LightGray)
138:         {
139:             Write(";4");
140:         }
141:         else
142:         {
143:             Write(";10");
144:         }
145:         Write(GetAnsiColorCode(backgroundColor));
146:     }
147:     Write("m");
148: }
149: 
150: /// <summary>
151: /// Reset console foreground and background back to original (will output ANSI color codes)
152: /// </summary>
153: void Console::ResetTerminalColor()
154: {
155:     SetTerminalColor();
156: }
157: 
158: /// <summary>
159: /// Write a string, using the specified foreground and background color
160: /// </summary>
161: /// <param name="str">String to be written</param>
162: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
163: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
164: void Console::Write(const char* str, ConsoleColor foregroundColor, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
165: {
166:     static volatile bool inUse{};
167: 
168:     while (inUse)
169:     {
170:         Timer::WaitMilliSeconds(1);
171:     }
172:     inUse = true;
173: 
174:     SetTerminalColor(foregroundColor, backgroundColor);
175:     Write(str);
176:     SetTerminalColor();
177: 
178:     inUse = false;
179: }
180: 
181: /// <summary>
182: /// Write a string without changing the foreground and background color
183: /// </summary>
184: /// <param name="str">String to be written</param>
185: void Console::Write(const char* str)
186: {
187:     if (m_device != nullptr)
188:     {
189:         m_device->Write(str, strlen(str));
190:     }
191: }
192: 
193: /// <summary>
194: /// Flush the device buffers
195: /// </summary>
196: void Console::Flush()
197: {
198:     if (m_device != nullptr)
199:     {
200:         m_device->Flush();
201:     }
202: }
203: 
204: /// <summary>
205: /// Read a character
206: /// </summary>
207: /// <returns>Character received</returns>
208: char Console::ReadChar()
209: {
210:     char ch{};
211:     if (m_device != nullptr)
212:     {
213:         m_device->Read(&ch, 1);
214:     }
215:     return ch;
216: }
217: 
218: /// Write a single character.
219: /// </summary>
220: /// <param name="ch">Character to be written</param>
221: void Console::WriteChar(char ch)
222: {
223:     if (m_device != nullptr)
224:     {
225:         m_device->Write(ch);
226:     }
227: }
228: 
229: /// <summary>
230: /// Retrieve the singleton console
231: ///
232: /// Creates a static instance of Console, and returns a reference to it.
233: /// </summary>
234: /// <returns>A reference to the singleton console.</returns>
235: Console& GetConsole()
236: {
237:     return *Console::s_instance;
238: }
239: 
240: } // namespace baremetal
```

- Line 53-89: We define a local function `GetAnsiColorCode()` to convert from a color to an ANSI color code.
Note that normal and bright colors use the same code, but have a different prefix.
- Line 91: We define and initialize the static singleton instance pointer.
By default this is nullptr, so we need to instantiate the singleton
- Line 93-102: We implement the constructor.
This will save the device specified as the default as well as the current device.
We also assign the singleton instance of the `Console` class.
So the constructor is expected to only be run once.
We have no way to enforce this yet, this will be done later
- Line 104-113: We implement the method `AssignDevice()`.
If the device pointer is a null pointer, we use the set default device.
This needs to be instantiated first 
- Line 115-148: We implement the method `SetTerminalColor()`.
Here we see that normal and light foreground colors use a different prefix, and similarly for background colors
- Line 150-156: We implement the method `ResetTerminalColor()`, which simply calls `SetTerminalColor()` with default arguments
- Line 158-179: We implement the `Write()` method for a string with color specification.
This is simply a combination of setting colors, writing the string, and resetting colors.
The only exception is that we wait for the static variable inUse to become false.
This is a very simple locking mechanism that will prevent multiple cores writing to the console at the same time, to organize output a bit better.
There are better ways of doing this, e.g using atomics, however we don't have these yet.
We'll get to that when we actually start using multiple cores.
- Line 181-191: Implements the `Write()` method for a string with no color specification.
Notice that this is very similar to writing a string in `UART0` and `UART1`.
- Line 193-202: We implement the `Flush()` method.
This simply calls `Flush()` on the device if defined
- Line 204-216: We implement the `ReadChar()` method
- Line 218-227: We implement the `WriteChar()` method
- Line 229-240: We implement the `GetConsole()` function

### Util.h {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_UTILH}

We are using a function `strlen()` which is in the standard C library, but we need to define it.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`:

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
51: void* memset(void* buffer, int value, size_t length);
52: void* memcpy(void* dest, const void* src, size_t length);
53: 
54: size_t strlen(const char* str);
```

### Util.cpp {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_UTILCPP}

We are using a function `strlen()` which is in the standard C library, but we need to define it.

Update the file `code/libraries/stdlib/src/Util.cpp`:

```cpp
File: code/libraries/stdlib/src/Util.cpp
...
81: 
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
```

### System.cpp {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_SYSTEMCPP}

We should switch the code in `System.cpp` to the console, and instantiate the default console based on the definitions passed from `CMakeSettings.json`.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
40: #include "baremetal/System.h"
41: 
42: #include "stdlib/Util.h"
43: #include "baremetal/ARMInstructions.h"
44: #include "baremetal/BCMRegisters.h"
45: #include "baremetal/Console.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "baremetal/SysConfig.h"
48: #include "baremetal/Timer.h"
49: #include "baremetal/UART0.h"
50: #include "baremetal/UART1.h"
...
106: void System::Halt()
107: {
108:     GetConsole().Write("Halt\n", ConsoleColor::Cyan);
109:     Timer::WaitMilliSeconds(WaitTime);
...
129: void System::Reboot()
130: {
131:     GetConsole().Write("Reboot\n", ConsoleColor::Cyan);
132:     Timer::WaitMilliSeconds(WaitTime);
...
180: #if defined(BAREMETAL_CONSOLE_UART0)
181:     auto& uart = GetUART0();
182:     uart.Initialize(115200);
183:     uart.WriteString("Setting up UART0\n");
184:     Console console(&uart);
185: #elif defined(BAREMETAL_CONSOLE_UART1)
186:     auto& uart = GetUART1();
187:     uart.Initialize(115200);
188:     uart.WriteString("Setting up UART1\n");
189:     Console console(&uart);
190: #else
191:     Console console(nullptr);
192: #endif
193:     console.Write("Starting up\n", ConsoleColor::Cyan);
194: 
195:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
...
```

Note that line numbers have changed due to the `Doxygen` comments that were added.

- Line 45: We include the Console header
- Line 117: We write to the console, in cyan color
- Line 140: We write to the console, in cyan color
- Line 190: We write to the console, in cyan color

### Update application code {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_UPDATE_APPLICATION_CODE}

Let's use the console now.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/BCMRegisters.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/Mailbox.h"
5: #include "baremetal/MemoryManager.h"
6: #include "baremetal/RPIProperties.h"
7: #include "baremetal/SysConfig.h"
8: #include "baremetal/Serialization.h"
9: #include "baremetal/System.h"
10: #include "baremetal/Timer.h"
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

### Configuring, building and debugging {#TUTORIAL_12_CONSOLE_ADDING_A_CONSOLE_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color:

<img src="images/tutorial-12-console.png" alt="Console output" width="400"/>

Next: [13-logger](13-logger.md)
