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
115:     void Write(uint8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
116:     void Write(uint32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
117:     void Write(uint64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
118:     void Write(const uint8 *value, size_t size);
119:     void Write(uint8 *value, size_t size);
120:     void Write(const uint32 *value, size_t size);
121:     void Write(uint32 *value, size_t size);
122:     void Write(bool value);
123: 
124:     char ReadChar();
125:     void WriteChar(char ch);
126: 
127: private:
128: };
129: 
130: Console &GetConsole();
131: 
132: } // namespace baremetal
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
  - Line 115: We declare the method `Write()` to write a 8 bit unsigned integer. This will use the `Serialize()` function
  - Line 116: We declare the method `Write()` to write a 32 bit unsigned integer. This will use the `Serialize()` function
  - Line 117: We declare the method `Write()` to write a 64 bit unsigned integer. This will use the `Serialize()` function
  - Line 118: We declare the method `Write()` to write an array of const bytes. This will output a memory dump with 16 bytes per row
  - Line 119: We declare the method `Write()` to write an array of non-const bytes. This will output a memory dump with 16 bytes per row
  - Line 120: We declare the method `Write()` to write an array of const unsigned 32 bit values. This will output a memory dump with 8 words per row
  - Line 121: We declare the method `Write()` to write an array of non-const unsigned 32 bit values. This will output a memory dump with 8 words per row
  - Line 122: We declare the method `Write()` to write a boolean value
  - Line 124: We declare the method `ReadChar()` to read a character from the console
  - Line 125: We declare the method `WriteChar()` to write a character to the console
- Line 130: We declare the accessor for the `Console` class. This will create an instance if needed, initialize it, and return a reference.

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
48: namespace baremetal {
49: 
50: static const char *GetAnsiColorCode(ConsoleColor color)
51: {
52:     switch (color)
53:     {
54:     case ConsoleColor::Black:
55:     case ConsoleColor::DarkGray:
56:         return "0";
57:     case ConsoleColor::Red:
58:     case ConsoleColor::BrightRed:
59:         return "1";
60:     case ConsoleColor::Green:
61:     case ConsoleColor::BrightGreen:
62:         return "2";
63:     case ConsoleColor::Yellow:
64:     case ConsoleColor::BrightYellow:
65:         return "3";
66:     case ConsoleColor::Blue:
67:     case ConsoleColor::BrightBlue:
68:         return "4";
69:     case ConsoleColor::Magenta:
70:     case ConsoleColor::BrightMagenta:
71:         return "5";
72:     case ConsoleColor::Cyan:
73:     case ConsoleColor::BrightCyan:
74:         return "6";
75:     case ConsoleColor::LightGray:
76:     case ConsoleColor::White:
77:         return "7";
78:     default:
79:         return 0;
80:     };
81: }
82: 
83: /// <summary>
84: /// Create a console linked to the specified character device. Note that the constructor is private, so GetConsole() is needed to instantiate the console
85: /// </summary>
86: /// <param name="device">Character device used for output</param>
87: Console::Console(CharDevice *device)
88:     : m_device{device}
89: {
90: }
91: 
92: /// <summary>
93: /// Change the attached device
94: /// </summary>
95: /// <param name="device">Character device to be set to for output</param>
96: void Console::AssignDevice(CharDevice *device)
97: {
98:     m_device = device;
99: }
100: 
101: /// <summary>
102: /// Set console foreground and background color (will output ANSI color codes)
103: /// </summary>
104: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
105: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
106: void Console::SetTerminalColor(ConsoleColor foregroundColor /*= ConsoleColor::Default*/, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
107: {
108:     Write("\033[0");
109:     if (foregroundColor != ConsoleColor::Default)
110:     {
111:         if (foregroundColor <= ConsoleColor::LightGray)
112:         {
113:             Write(";3");
114:         }
115:         else
116:         {
117:             Write(";9");
118:         }
119:         Write(GetAnsiColorCode(foregroundColor));
120:     }
121:     if (backgroundColor != ConsoleColor::Default)
122:     {
123:         if (backgroundColor <= ConsoleColor::LightGray)
124:         {
125:             Write(";4");
126:         }
127:         else
128:         {
129:             Write(";10");
130:         }
131:         Write(GetAnsiColorCode(backgroundColor));
132:     }
133:     Write("m");
134: }
135: 
136: /// <summary>
137: /// Reset console foreground and background back to original (will output ANSI color codes)
138: /// </summary>
139: void Console::ResetTerminalColor()
140: {
141:     SetTerminalColor();
142: }
143: 
144: /// <summary>
145: /// Write a string, using the specified foreground and background color
146: /// </summary>
147: /// <param name="str">String to be written</param>
148: /// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
149: /// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
150: void Console::Write(const char *str, ConsoleColor foregroundColor, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
151: {
152:     static volatile bool inUse{};
153: 
154:     while (inUse)
155:     {
156:         Timer::WaitMilliSeconds(1);
157:     }
158:     inUse = true;
159: 
160:     SetTerminalColor(foregroundColor, backgroundColor);
161:     Write(str);
162:     SetTerminalColor();
163: 
164:     inUse = false;
165: }
166: 
167: /// <summary>
168: /// Write a string without changing the foreground and background color
169: /// </summary>
170: /// <param name="str">String to be written</param>
171: void Console::Write(const char *str)
172: {
173:     while (*str)
174:     {
175:         // convert newline to carriage return + newline
176:         if (*str == '\n')
177:             WriteChar('\r');
178:         WriteChar(*str++);
179:     }
180: }
181: 
182: /// <summary>
183: /// Write an unsigned 8 bit integer value formatted as specified
184: /// </summary>
185: /// <param name="value">Value to be written</param>
186: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
187: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
188: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
189: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
190: void Console::Write(uint8 value, int width, int base, bool showBase, bool leadingZeros)
191: {
192:     const size_t BufferSize = 256;
193:     char buffer[BufferSize];
194: 
195:     Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
196:     Write(buffer);
197: }
198: 
199: /// <summary>
200: /// Write an unsigned 32 bit integer value formatted as specified
201: /// </summary>
202: /// <param name="value">Value to be written</param>
203: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
204: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
205: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
206: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
207: void Console::Write(uint32 value, int width, int base, bool showBase, bool leadingZeros)
208: {
209:     const size_t BufferSize = 256;
210:     char buffer[BufferSize];
211:     
212:     Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
213:     Write(buffer);
214: }
215: 
216: /// <summary>
217: /// Write an unsigned 64 bit integer value formatted as specified
218: /// </summary>
219: /// <param name="value">Value to be written</param>
220: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
221: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
222: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
223: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
224: void Console::Write(uint64 value, int width, int base, bool showBase, bool leadingZeros)
225: {
226:     const size_t BufferSize = 256;
227:     char buffer[BufferSize];
228: 
229:     Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
230:     Write(buffer);
231: }
232: 
233: static const size_t WordsPerRow     = 8;
234: static const size_t MaxDisplayWords = 2048;
235: 
236: /// <summary>
237: /// Write a const array of unsigned 32 bit values. 
238: /// Values are separated by spaces in rows of 8, each row prepended with the offset of the start of the row.
239: /// The maximum count of values shown is 2048.
240: /// </summary>
241: /// <param name="value">Pointer to array of values to be written</param>
242: /// <param name="size">Count of values in array</param>
243: void                Console::Write(const uint32 *value, size_t size)
244: {
245:     if (m_device == nullptr)
246:         return;
247:     if (value != nullptr)
248:     {
249:         Write("\r\n");
250:         size_t displayWords = (size < MaxDisplayWords) ? size : MaxDisplayWords;
251:         // cppcheck-suppress knownConditionTrueFalse
252:         for (size_t i = 0; i < displayWords; i += WordsPerRow)
253:         {
254:             Write(i, 0, 16, false, true);
255:             Write("  ");
256:             for (size_t j = 0; j < WordsPerRow; ++j)
257:             {
258:                 if (j != 0)
259:                     WriteChar(' ');
260:                 if ((i + j) < displayWords)
261:                     Write(value[i + j], 8, 16, false, true);
262:                 else
263:                     Write("        ");
264:             }
265:             Write("\r\n");
266:         }
267:     }
268:     else
269:     {
270:         Write("null\r\n");
271:     }
272: }
273: 
274: /// <summary>
275: /// Write a non-const array of unsigned 32 bit values. 
276: /// Values are separated by spaces in rows of 8, each row prepended with the offset of the start of the row.
277: /// The maximum count of values shown is 2048.
278: /// </summary>
279: /// <param name="value">Pointer to array of values to be written</param>
280: /// <param name="size">Count of values in array</param>
281: void Console::Write(uint32 *value, size_t size)
282: {
283:     Write(reinterpret_cast<const uint32 *>(value), size);
284: }
285: 
286: static const size_t BytesPerRow     = 16;
287: static const size_t MaxDisplayBytes = 4096;
288: 
289: /// <summary>
290: /// Write a const byte array.
291: /// 
292: /// Bytes are separated by spaces in rows of 16, each row prepended with the offset of the start of the row.
293: /// At the end of every row, the bytes are also shown as characters, if printable, otherwise as '.'.
294: /// The maximum count of bytes shown is 4096.
295: /// </summary>
296: /// <param name="value">Pointer to array of bytes to be written</param>
297: /// <param name="size">Count of bytes in array</param>
298: void                Console::Write(const uint8 *value, size_t size)
299: {
300:     if (m_device == nullptr)
301:         return;
302:     if (value != nullptr)
303:     {
304:         Write("\r\n");
305:         size_t displayBytes = (size < MaxDisplayBytes) ? size : MaxDisplayBytes;
306:         // cppcheck-suppress knownConditionTrueFalse
307:         for (size_t i = 0; i < displayBytes; i += BytesPerRow)
308:         {
309:             Write(i, 8, 16, false, true);
310:             Write("  ");
311:             for (size_t j = 0; j < BytesPerRow; ++j)
312:             {
313:                 if (j != 0)
314:                     WriteChar(' ');
315:                 if ((i + j) < displayBytes)
316:                     Write(static_cast<uint8>(value[i + j]), 2, 16, false, true);
317:                 else
318:                     Write("  ");
319:             }
320:             Write("  ");
321:             for (size_t j = 0; j < BytesPerRow; ++j)
322:             {
323:                 if (j != 0)
324:                     WriteChar(' ');
325:                 if ((i + j) < displayBytes)
326:                 {
327:                     auto ch = value[i + j];
328:                     WriteChar(((ch >= 32) && (ch < 127)) ? static_cast<char>(ch) : '.');
329:                 }
330:                 else
331:                 {
332:                     WriteChar(' ');
333:                 }
334:             }
335:             Write("\r\n");
336:         }
337:     }
338:     else
339:     {
340:         Write("null\r\n");
341:     }
342: }
343: 
344: /// <summary>
345: /// Write a non-const byte array.
346: /// 
347: /// Bytes are separated by spaces in rows of 16, each row prepended with the offset of the start of the row.
348: /// At the end of every row, the bytes are also shown as characters, if printable, otherwise as '.'.
349: /// The maximum count of bytes shown is 4096.
350: /// </summary>
351: /// <param name="value">Pointer to array of bytes to be written</param>
352: /// <param name="size">Count of bytes in array</param>
353: void Console::Write(uint8 *value, size_t size)
354: {
355:     if (m_device == nullptr)
356:         return;
357:     Write(reinterpret_cast<const uint8 *>(value), size);
358: }
359: 
360: /// <summary>
361: /// Write a boolean value.
362: /// 
363: /// Will write "true" if the value is true, "false" otherwise
364: /// </summary>
365: /// <param name="value">Value to be written</param>
366: void Console::Write(bool value)
367: {
368:     if (m_device == nullptr)
369:         return;
370:     Write(value ? "true" : "false");
371: }
372: 
373: /// <summary>
374: /// Read a character
375: /// </summary>
376: /// <returns>Character received</returns>
377: char Console::ReadChar()
378: {
379:     char ch{};
380:     if (m_device != nullptr)
381:     {
382:         ch = m_device->Read();
383:     }
384:     return ch;
385: }
386: 
387: /// <summary>
388: /// Write a single character.
389: /// </summary>
390: /// <param name="ch">Character to be written</param>
391: void Console::WriteChar(char ch)
392: {
393:     if (m_device != nullptr)
394:     {
395:         m_device->Write(ch);
396:     }
397: }
398: 
399: /// <summary>
400: /// Retrieve the singleton console
401: ///
402: /// Creates a static instance of Console, and returns a reference to it.
403: /// </summary>
404: /// <returns>A reference to the singleton console.</returns>
405: Console &GetConsole()
406: {
407: #if defined(BAREMETAL_CONSOLE_UART0)
408:     static UART0&  uart = GetUART0();
409:     static Console console(&uart);
410: #elif defined(BAREMETAL_CONSOLE_UART1)
411:     static UART1&  uart = GetUART1();
412:     static Console console(&uart);
413: #else
414:     static Console console(nullptr);
415: #endif
416:     return console;
417: }
418: 
419: } // namespace baremetal
```

- Line 50-81: Implements a color conversion to an ANSI color code. Not that normal and bright colors use the same code, but have a different prefix.
- Line 87-90: Implements the constructor
- Line 96-99: Implements the method `AssignDevice()`
- Line 106-134: Implements the method `SetTerminalColor()`. Here we see that normal and light foreground colors use a different prefix, and similarly for background colors.
- Line 139-142: Implements the method `ResetTerminalColor()`, which simply calls `SetTerminalColor()` with default arguments.
- Line 150-165: Implements the `Write()` method for a string with color specification. This is simply a combination of setting colors, writing the string, and resetting colors.
The only exception is that we wait for the bool inUse to became false. This is a very simple locking mechanism that will prevent multiple cores writing to the console at the same time, to organize output a bit better.
We'll get to that when we actually start using multiple cores.
- Line 171-180: Implements the `Write()` method for a string with no color specification.
- Line 190-197: Implements the `Write()` method for 8 bit unsigned integer. The method to serialize this is not defined in Serialization.h yet, so we're going to add it in a minute.
- Line 207-214: Implements the `Write()` method for 32 bit unsigned integer. 
- Line 224-231: Implements the `Write()` method for 64 bit unsigned integer. 
- Line 243-272: Implements the `Write()` method for an array of const 32 bit unsigned integers.
You can see that we print a maximum of 2048 words, 8 on each line.
If the pointer is nullptr, we print `null`
- Line 281-284: Implements the `Write()` method for an array of non const 32 bit unsigned integers.
- Line 298-342: Implements the `Write()` method for an array of const 8 bit unsigned integers.
You can see that we print a maximum of 4096 bytes, 16 on each line. If the byte is printable, it will also be printed to the right of the 16 value in each row, otherwise it will be printed as '.'.
If the pointer is nullptr, we print `null`
- Line 353-358: Implements the `Write()` method for an array of non const 8 bit unsigned integers.
- Line 366-371: Implements the `Write()` method for a boolean.
- Line 377-385: Implements the `ReadChar()` method.
- Line 391-397: Implements the `WriteChar()` method.
- Line 405-419: Implements the `GetConsole()` function.
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
49: static void Append(char* buffer, size_t bufferSize, char c)
50: {
51:     size_t len = strlen(buffer);
52:     char* p = buffer + len;
53:     if (static_cast<size_t>(p - buffer) < bufferSize)
54:     {
55:         *p++ = c;
56:     }
57:     if (static_cast<size_t>(p - buffer) < bufferSize)
58:     {
59:         *p = '\0';
60:     }
61: }
62: 
63: static void Append(char* buffer, size_t bufferSize, size_t count, char c)
64: {
65:     size_t len = strlen(buffer);
66:     char* p = buffer + len;
67:     while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
68:     {
69:         *p++ = c;
70:         --count;
71:     }
72:     if (static_cast<size_t>(p - buffer) < bufferSize)
73:     {
74:         *p = '\0';
75:     }
76: }
77: 
78: static void Append(char* buffer, size_t bufferSize, const char* str)
79: {
80:     strncat(buffer, str, bufferSize);
81: }
82: 
83: void Format(char* buffer, size_t bufferSize, const char* format, ...)
84: {
85:     va_list var;
86:     va_start(var, format);
87: 
88:     FormatV(buffer, bufferSize, format, var);
89: 
90:     va_end(var);
91: }
92: 
93: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args)
94: {
95:     buffer[0] = '\0';
96: 
97:     while (*format != '\0')
98:     {
99:         if (*format == '%')
100:         {
101:             if (*++format == '%')
102:             {
103:                 Append(buffer, bufferSize, '%');
104:                 format++;
105:                 continue;
106:             }
107: 
108:             bool alternate = false;
109:             if (*format == '#')
110:             {
111:                 alternate = true;
112:                 format++;
113:             }
114: 
115:             bool left = false;
116:             if (*format == '-')
117:             {
118:                 left = true;
119:                 format++;
120:             }
121: 
122:             bool leadingZero = false;
123:             if (*format == '0')
124:             {
125:                 leadingZero = true;
126:                 format++;
127:             }
128: 
129:             size_t width = 0;
130:             while (('0' <= *format) && (*format <= '9'))
131:             {
132:                 width = width * 10 + (*format - '0');
133:                 format++;
134:             }
135: 
136:             unsigned precision = 6;
137:             if (*format == '.')
138:             {
139:                 format++;
140:                 precision = 0;
141:                 while ('0' <= *format && *format <= '9')
142:                 {
143:                     precision = precision * 10 + (*format - '0');
144: 
145:                     format++;
146:                 }
147:             }
148: 
149:             bool haveLong{};
150:             bool haveLongLong{};
151: 
152:             if (*format == 'l')
153:             {
154:                 if (*(format + 1) == 'l')
155:                 {
156:                     haveLongLong = true;
157: 
158:                     format++;
159:                 }
160:                 else
161:                 {
162:                     haveLong = true;
163:                 }
164: 
165:                 format++;
166:             }
167: 
168:             switch (*format)
169:             {
170:             case 'c':
171:                 {
172:                     char ch = static_cast<char>(va_arg(args, int));
173:                     if (left)
174:                     {
175:                         Append(buffer, bufferSize, ch);
176:                         if (width > 1)
177:                         {
178:                             Append(buffer, bufferSize, width - 1, ' ');
179:                         }
180:                     }
181:                     else
182:                     {
183:                         if (width > 1)
184:                         {
185:                             Append(buffer, bufferSize, width - 1, ' ');
186:                         }
187:                         Append(buffer, bufferSize, ch);
188:                     }
189:                 }
190:                 break;
191: 
192:             case 'd':
193:             case 'i':
194:                 if (haveLongLong)
195:                 {
196:                     char str[BufferSize]{};
197:                     Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
198:                     Append(buffer, bufferSize, str);
199:                 }
200:                 else if (haveLong)
201:                 {
202:                     char str[BufferSize]{};
203:                     Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
204:                     Append(buffer, bufferSize, str);
205:                 }
206:                 else
207:                 {
208:                     char str[BufferSize]{};
209:                     Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
210:                     Append(buffer, bufferSize, str);
211:                 }
212:                 break;
213: 
214:             case 'f':
215:                 {
216:                     char str[BufferSize]{};
217:                     Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
218:                     Append(buffer, bufferSize, str);
219:                 }
220:                 break;
221: 
222:             case 'b':
223:                 if (alternate)
224:                 {
225:                     Append(buffer, bufferSize, "0b");
226:                 }
227:                 if (haveLongLong)
228:                 {
229:                     char str[BufferSize]{};
230:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
231:                     Append(buffer, bufferSize, str);
232:                 }
233:                 else if (haveLong)
234:                 {
235:                     char str[BufferSize]{};
236:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
237:                     Append(buffer, bufferSize, str);
238:                 }
239:                 else
240:                 {
241:                     char str[BufferSize]{};
242:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
243:                     Append(buffer, bufferSize, str);
244:                 }
245:                 break;
246: 
247:             case 'o':
248:                 if (alternate)
249:                 {
250:                     Append(buffer, bufferSize, '0');
251:                 }
252:                 if (haveLongLong)
253:                 {
254:                     char str[BufferSize]{};
255:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
256:                     Append(buffer, bufferSize, str);
257:                 }
258:                 else if (haveLong)
259:                 {
260:                     char str[BufferSize]{};
261:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
262:                     Append(buffer, bufferSize, str);
263:                 }
264:                 else
265:                 {
266:                     char str[BufferSize]{};
267:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
268:                     Append(buffer, bufferSize, str);
269:                 }
270:                 break;
271: 
272:             case 's':
273:                 {
274:                     char str[BufferSize]{};
275:                     Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
276:                     Append(buffer, bufferSize, str);
277:                 }
278:                 break;
279: 
280:             case 'u':
281:                 if (haveLongLong)
282:                 {
283:                     char str[BufferSize]{};
284:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
285:                     Append(buffer, bufferSize, str);
286:                 }
287:                 else if (haveLong)
288:                 {
289:                     char str[BufferSize]{};
290:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
291:                     Append(buffer, bufferSize, str);
292:                 }
293:                 else
294:                 {
295:                     char str[BufferSize]{};
296:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
297:                     Append(buffer, bufferSize, str);
298:                 }
299:                 break;
300: 
301:             case 'x':
302:             case 'X':
303:                 if (alternate)
304:                 {
305:                     Append(buffer, bufferSize, "0x");
306:                 }
307:                 if (haveLongLong)
308:                 {
309:                     char str[BufferSize]{};
310:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
311:                     Append(buffer, bufferSize, str);
312:                 }
313:                 else if (haveLong)
314:                 {
315:                     char str[BufferSize]{};
316:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
317:                     Append(buffer, bufferSize, str);
318:                 }
319:                 else
320:                 {
321:                     char str[BufferSize]{};
322:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
323:                     Append(buffer, bufferSize, str);
324:                 }
325:                 break;
326: 
327:             case 'p':
328:                 if (alternate)
329:                 {
330:                     Append(buffer, bufferSize, "0x");
331:                 }
332:                 {
333:                     char str[BufferSize]{};
334:                     Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
335:                     Append(buffer, bufferSize, str);
336:                 }
337:                 break;
338: 
339:             default:
340:                 Append(buffer, bufferSize, '%');
341:                 Append(buffer, bufferSize, *format);
342:                 break;
343:             }
344:         }
345:         else
346:         {
347:             Append(buffer, bufferSize, *format);
348:         }
349: 
350:         format++;
351:     }
352: }
353: 
354: } // namespace baremetal
```

- Line 47: We define a buffer size, to define the buffer, but also to check against writing outside the buffer.
- Line 49-61: Implements the function `Append()` for writing a single character to the buffer, checked for overflow
- Line 63-76: Implements the function `Append()` for writing multiple of a single character to the buffer, checked for overflow
- Line 78-81: Implements the function `Append()` for writing a string to the buffer, checked for overflow
- Line 83-91: Implements the `Format()` function, using the ellipsis operator. This simple creates a `va_list` from the arguments, and calls the other version of `Format()`
- Line 93-358: Implements the `Format()` function, using the `va_list` argument
  - Line 95: We make sure to have an empty buffer
  - Line 97: We scan through the format string
  - Line 99: If the format character is `%` this is a special format operator
    - Line 101-106: In case the format string holds `%%` we see this as verbatim `%`, so we add that character
    - Line 108-113: If the next character is `#` we see this as an alternative version, signalling to add the prefix (only for base 2, 8, 16 integers), and advance
    - Line 115-120: If the next character is `-` we left-align the value, and advance
    - Line 122-127: If the next character is `0` we use leading zeros to fill up the value to its normal length, and advance
    - Line 129-134: If more digits follow, e.g. `%12` or `%012`, we extract the width of the value to be printed, and advance
    - Line 136-147: If a decimal point follows, we expect this to be a floating point value, and expect the next digits to specify the length of the fraction.
    We read all digits, calculate the fraction length, and advance
    - Line 149-166: If the next character is `l` this is seen as a specification for a `long` (32 bit) value (only for integer values).
If another `l` follows, we see this as a `long long` (64 bit) value
    - Line 170-190: If the format character is a `c` we print the value as a character
    - Line 192-212: If the format character is a `d` or `i` we print the value as a signed integer, taking into account the `l` or `ll` prefix
    - Line 214-220: If the format character is a `f` we print the value as a double, taking into account the precision if set
    - Line 222-245: If the format character is a `b` we print the value as a binary unsigned integer, taking into account the `#`, `l` or `ll` prefix.
Note that this is an addition to `printf()` behaviour
    - Line 227-270: If the format character is a `o` we print the value as a octal unsigned integer, taking into account the `#`, `l` or `ll` prefix.
Note that this is an addition to `printf()` behaviour
    - Line 272-278: If the format character is a `s` we print the value as a string
    - Line 280-299: If the format character is a `u` we print the value as a decimal unsigned integer, taking into account the `l` or `ll` prefix
    - Line 301-325: If the format character is a `x` or `X` we print the value as a hexadecimal unsigned integer, taking into account the `#`, `l` or `ll` prefix
    - Line 327-337: If the format character is a `p` we print the value as a pointer, meaning it is printed as a 64 bit unsigned integer
  - Line 345-348: If the format character is not `%` we simply add the character

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
42: #define BAREMETAL_NAME              "Baremetal"
43: 
44: #define BAREMETAL_MAJOR_VERSION     BAREMETAL_MAJOR
45: #define BAREMETAL_MINOR_VERSION     BAREMETAL_MINOR
46: #define BAREMETAL_PATCH_VERSION     BAREMETAL_LEVEL
47: #define BAREMETAL_VERSION_STRING    GetVersion()
48: 
49: namespace baremetal {
50: 
51: void SetupVersion();
52: const char* GetVersion();
53: 
54: }
```

- Line 42: We create a definition for the name of our platform
- Line 44-46: We create definitions of the platform version parts, using the compiler definitions passed from `CMake`
- Line 47: We create a definition to get the version string
- Line 51: We declare a function `SetupVersion()` to build the version string
- Line 52: We declare a function `GetVersion()` to return the version string

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
45: static const size_t BufferSize = 20;
46: static char s_baremetalVersionString[BufferSize]{};
47: static bool s_baremetalVersionSetupDone = false;
48: 
49: void baremetal::SetupVersion()
50: {
51:     if (!s_baremetalVersionSetupDone)
52:     {
53:         Format(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_PATCH_VERSION);
54:         s_baremetalVersionSetupDone = true;
55:     }
56: }
57: 
58: const char* baremetal::GetVersion()
59: {
60:     return s_baremetalVersionString;
61: }
```

- Line 45-46: We define the local variable `s_baremetalVersionString` and its size
- Line 49-56: We implement `SetupVersion()`, which prints a formatted string to `s_baremetalVersionString`
- Line 58-61: We implement `GetVersion(), which simply returns the string `s_baremetalVersionString`

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
49: using namespace baremetal;
50: 
51: LOG_MODULE("Logger");
52: 
53: /// <summary>
54: /// Construct a logger
55: /// </summary>
56: /// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
57: /// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
58: /// <param name="console">Console to print to, defaults to the singleton console instance</param>
59: Logger::Logger(LogSeverity logLevel, Timer *timer /*= nullptr*/, Console &console /*= GetConsole()*/)
60:     : m_initialized{}
61:     , m_timer{timer}
62:     , m_console{console}
63:     , m_level{logLevel}
64: {
65: }
66: 
67: /// <summary>
68: /// Initialize logger
69: /// </summary>
70: /// <returns>true on succes, false on failure</returns>
71: bool Logger::Initialize()
72: {
73:     if (m_initialized)
74:         return true;
75:     SetupVersion();
76:     m_initialized = true; // Stop reentrant calls from happening
77:     LOG_INFO(BAREMETAL_NAME " %s started on %s (AArch64)", BAREMETAL_VERSION_STRING, "Raspberry Pi" /*GetMachineInfo().GetName()*/);
78: 
79:     return true;
80: }
81: 
82: /// <summary>
83: /// Set maximum log level (minimum log priority). Any log statements with a value below this level will be ignored
84: /// </summary>
85: /// <param name="logLevel">Maximum log level</param>
86: void Logger::SetLogLevel(LogSeverity logLevel)
87: {
88:     m_level = logLevel;
89: }
90: 
91: /// <summary>
92: /// Write a string with variable arguments to the logger
93: /// </summary>
94: /// <param name="source">Source name or file name</param>
95: /// <param name="line">Source line number</param>
96: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
97: /// <param name="message">Formatted message string, with variable arguments</param>
98: void Logger::Write(const char *source, int line, LogSeverity severity, const char *message, ...)
99: {
100:     va_list var;
101:     va_start(var, message);
102:     WriteV(source, line, severity, message, var);
103:     va_end(var);
104: }
105: 
106: /// <summary>
107: /// Write a string with variable arguments to the logger
108: /// </summary>
109: /// <param name="source">Source name or file name</param>
110: /// <param name="line">Source line number</param>
111: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
112: /// <param name="message">Formatted message string</param>
113: /// <param name="args">Variable argument list</param>
114: void Logger::WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args)
115: {
116:     if (static_cast<int>(severity) > static_cast<int>(m_level))
117:         return;
118: 
119:     static const size_t BufferSize = 1024;
120:     char buffer[BufferSize]{};
121: 
122:     char sourceString[BufferSize]{};
123:     Format(sourceString, BufferSize, " (%s:%d)", source, line);
124: 
125:     char messageBuffer[BufferSize]{};
126:     FormatV(messageBuffer, BufferSize, message, args);
127: 
128:     switch (severity)
129:     {
130:     case LogSeverity::Panic:
131:         strncat(buffer, "!Panic!", BufferSize);
132:         break;
133:     case LogSeverity::Error:
134:         strncat(buffer, "Error  ", BufferSize);
135:         break;
136:     case LogSeverity::Warning:
137:         strncat(buffer, "Warning", BufferSize);
138:         break;
139:     case LogSeverity::Info:
140:         strncat(buffer, "Info   ", BufferSize);
141:         break;
142:     case LogSeverity::Debug:
143:         strncat(buffer, "Debug  ", BufferSize);
144:         break;
145:     }
146: 
147:     if (m_timer != nullptr)
148:     {
149:         const size_t TimeBufferSize = 32;
150:         char timeBuffer[TimeBufferSize]{};
151:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
152:         if (strlen(timeBuffer) > 0)
153:         {
154:            strncat(buffer, timeBuffer, BufferSize);
155:            strncat(buffer, " ", BufferSize);
156:         }
157:     }
158: 
159:     strncat(buffer, messageBuffer, BufferSize);
160:     strncat(buffer, sourceString, BufferSize);
161:     strncat(buffer, "\n", BufferSize);
162: 
163: #if BAREMETAL_COLOR_OUTPUT
164:     switch (severity)
165:     {
166:     case LogSeverity::Panic:
167:         m_console.Write(buffer, ConsoleColor::BrightRed);
168:         break;
169:     case LogSeverity::Error:
170:         m_console.Write(buffer, ConsoleColor::Red);
171:         break;
172:     case LogSeverity::Warning:
173:         m_console.Write(buffer, ConsoleColor::BrightYellow);
174:         break;
175:     case LogSeverity::Info:
176:         m_console.Write(buffer, ConsoleColor::Cyan);
177:         break;
178:     case LogSeverity::Debug:
179:         m_console.Write(buffer, ConsoleColor::Yellow);
180:         break;
181:     default:
182:         m_console.Write(buffer, ConsoleColor::White);
183:         break;
184:     }
185: #else
186:     m_console.Write(buffer);
187: #endif
188: 
189:     if (severity == LogSeverity::Panic)
190:     {
191:         GetSystem().Halt();
192:     }
193: }
194: 
195: /// <summary>
196: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
197: /// </summary>
198: /// <returns>Reference to the singleton logger instance</returns>
199: Logger &baremetal::GetLogger()
200: {
201:     static LogSeverity defaultSeverity{LogSeverity::Debug};
202:     static Logger      logger(defaultSeverity, &GetTimer());
203:     logger.Initialize();
204:     return logger;
205: }
```

- Line 51: We use the macro `LOG_MODULE` also internally to specify that we are in the `Logger` class itself, and we can use the `LOG*` macros
- Line 59-65: We implement the constructor
- Line 71-80: We implement the `Initialize()` method
  - Line 75: We use the function `SetupVersion()` from `Version.h` to set up the version string
  - Line 77: We use the `Logger` itself to log the first message, stating the platform name and its version
- Line 86-89: We implement the `SetLogLevel()` method. This simply set the maximum log level for filtering
- Line 98-104: We implement the `Write()` method. This simply call `WriteV()` after setting up the variable argument list
- Line 114-193: We implement the `WriteV()` method
  - Line 116-117: If the severity level passed in is to high (priority too low) we simply return without printing
  - Line 119-120: We define a buffer to hold the line to write
  - Line 122-123: We print source name and line number into a separate buffer
  - Line 125-126: We print the message with arguments into a separate buffer
  - Line 128-145: For each level, we add a string to the line buffer
  - Line 147-157: If a `Timer` was passed in, we request the current time, and print it into the line buffer.
We'll add the timer method in a minute
  - Line 159-161: We add the message, source information and end of line to the buffer
  - Line 163-187: Depending on whether we defined `BAREMETAL_COLOR_OUTPUT`, we either simply print the buffer without color, or we use a severity level specific color
  - Line 189-192: If the severity level is `Panic` we halt the system
- Line 199-205: We implement the friend function `GetLogger()` to retrieve the singleton instance of the logger.
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
66: #define assert(expression) (likely(expression) ? ((void)0) : AssertionFailed(#expression, __FILE__, __LINE__))
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
45: LOG_MODULE("Assert");
46: 
47: namespace baremetal {
48: 
49: static void AssertionFailedDefault(const char* expression, const char* fileName, int lineNumber);
50: 
51: static AssertionCallback *s_callback = AssertionFailedDefault;
52: 
53: /// <summary>
54: /// Log assertion failure and halt, is not expected to return (but may if a different assertion failure function is set up)
55: /// </summary>
56: /// <param name="expression">Expression to be printed</param>
57: /// <param name="fileName">Filename of file causing the failed assertion</param>
58: /// <param name="lineNumber">Line number causing the failed assertion</param>
59: void AssertionFailed(const char *expression, const char *fileName, int lineNumber)
60: {
61:     if (s_callback != nullptr)
62:         s_callback(expression, fileName, lineNumber);
63: }
64: 
65: /// <summary>
66: /// Default failed assertion handler
67: /// </summary>
68: /// <param name="expression">Expression to be printed</param>
69: /// <param name="fileName">Filename of file causing the failed assertion</param>
70: /// <param name="lineNumber">Line number causing the failed assertion</param>
71: static void AssertionFailedDefault(const char *expression, const char *fileName, int lineNumber)
72: {
73:     GetLogger().Write(fileName, lineNumber, LogSeverity::Panic, "assertion failed: %s", expression);
74: }
75: 
76: /// <summary>
77: /// Reset the assertion failure handler to the default
78: /// </summary>
79: void ResetAssertionCallback()
80: {
81:     s_callback = AssertionFailedDefault;
82: }
83: 
84: /// <summary>
85: /// Sets up a custom assertion failure handler
86: /// </summary>
87: /// <param name="callback">Assertion failure handler</param>
88: void SetAssertionCallback(AssertionCallback* callback)
89: {
90:     s_callback = callback;
91: }
92: 
93: } // namespace baremetal
```

- Line 51: We define a static variable to point to the set assertion failure handler function, which is set to `AssertionFailedDefault()` by default
- Line 59-63: We implement the function `AssertionFailed()` which is called when the `assert()` macro fails.
This will call the set assertion failure handler function
- Line 71-74: We implement the default assertion failure handler function `AssertionFailedDefault()`, which logs a `Panic` message, and will then halt the system
- Line 79-82: We implement the function `ResetAssertionCallback()` which will reset the assertion failure handler function to default
- Line 88-91: We implement the function `SetAssertionCallback()` which will set a custom assertion failure handler function

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

