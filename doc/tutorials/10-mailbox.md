# Tutorial 10: Mailbox {#TUTORIAL_10_MAILBOX}

@tableofcontents

## New tutorial setup {#TUTORIAL_10_MAILBOX_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/10-mailbox`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_10_MAILBOX_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-10.a`
- a library `output/Debug/lib/stdlib-10.a`
- an application `output/Debug/bin/10-mailbox.elf`
- an image in `deploy/Debug/10-mailbox-image`

## The Raspberry Pi Mailbox {#TUTORIAL_10_MAILBOX_THE_RASPBERRY_PI_MAILBOX}

In order to set up the other serial console(s), such as UART0 on Raspberry Pi 3 and UART0, 2, 3, 4, 5 on Raspberry Pi 4, we need to use the Raspberry Pi mailbox.

The mailbox is a mechanism to communicate between the ARM cores and the GPU. As you can read in [System startup](#SYSTEM_STARTUP), the GPU is the first part that becomes active after power on.
It runs the firmware, has detailed information on the system and can control some of the peripherals. And, of course, it can render to the screen.

In order to use the mailbox, we need to establish a common address between ARM and GPU to exchange information. This is done through a so-called Coherent page.
We can decide the location of this page, but need to convert the address for the GPU such that it can find the page.

A logical location for the coherent page is a bit after the last address used by our code.

Let's first revisit the memory map and add the coherent page information. Then we will add a memory manager, that for now will only hand out coherent memory pages, and after that we can start using the mailbox.
We'll finalize by using the mailbox to retrieve board information, such as board type, serial number and memory size.

## Updating the memory map - Step 1 {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1}

We need to update the memory map. We discussed the memory map before, in [06-console-uart1](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MEMORYMAPH).
The image below hopefully gives a clearer view on the memory map.

<img src="images/memory-map.png" alt="Memory map" width="800"/>

So, we need to add the coherent region part.

### MemoryMap.h {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_MEMORYMAPH}

Update the file `code/libraries/baremetal/include/baremetal/MemoryMap.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryMap.h
...
81: 
82: #if BAREMETAL_RPI_TARGET == 3
83: /// @brief Region reserved for coherent memory (memory shared between ARM and GPU). We reserve 1 Mb, but make sure then end is rounded
84: #define COHERENT_REGION_SIZE 1 * MEGABYTE
85: #else
86: /// @brief Region reserved for coherent memory (memory shared between ARM and GPU). We reserve 4 Mb, but make sure then end is rounded
87: #define COHERENT_REGION_SIZE 4 * MEGABYTE
88: #endif
89: 
90: /// @brief Region reserved for coherent memory rounded up to 1 Mb with 1 Mb extra space
91: #define MEM_COHERENT_REGION ((MEM_EXCEPTION_STACK_END + 2 * MEGABYTE) & ~(MEGABYTE - 1))
```

- Line 84: For Raspberry Pi 3, we define the coherent region to be 1 Mb.
- Line 87: For Raspberry Pi 4 / 5, we define the coherent region to be 4 Mb.
- Line 91: We leave 1 Mb of space after the Kernel stack, and then align to the next 1 Mb.

### Update the application code {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_UPDATE_THE_APPLICATION_CODE}

#### Serialization.h {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_UPDATE_THE_APPLICATION_CODE_SERIALIZATIONH}

In order to show which exact addresses we have, let's print them in our application.
However, we can write characters and strings to the console, but how about integers?
In standard C we would use `printf()`, in C++ we would use the stream insertion operator, but we don't have those.
So we'll have to write something ourselves. We could simply look around for a `printf()` implementation, but where's the fun in that?

So let's start adding a header for our serialization routines.

For now, we'll simply add one function.

Create the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Serialization.h
5: //
6: // Namespace   : serialization
7: //
8: // Class       : -
9: //
10: // Description : Serialization of types to character buffer
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
45: /// Type serialization functions
46: 
47: namespace baremetal {
48: 
49: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
50: 
51: } // namespace baremetal
```

- Line 48: We declare a function that writes a 32 bit unsigned integer value into a `buffer`, with a maximum `bufferSize` (including the trailing null character).
The value will take `width` characters at most (if zero the space needed is calculated), it will use a `base` which can be between 2 and 36 (so e.g. 16 for hexadecimal).
If `showBase` is true, the base prefix will be added (0b for `base` = 2, 0 for `base` = 8, 0x for `base` = 16).
If the size of the type would require more characters than strictly needed for the value, and `leadingZeros` is true, the value is prefixed with '0' characters.

#### Serialization.cpp {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_UPDATE_THE_APPLICATION_CODE_SERIALIZATIONCPP}

We need to implement the `Serialize` function. As we need to write into a fixed size buffer, we need to check whether what we need to write fits.

Create the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Serialization.cpp
5: //
6: // Namespace   : serialization
7: //
8: // Class       : -
9: //
10: // Description : Serialization of types to character buffer
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
40: #include <baremetal/Serialization.h>
41: 
42: /// @file
43: /// Type serialization functions implementation
44: 
45: namespace baremetal {
46: 
47: /// @brief Write characters with base above 10 as uppercase or not
48: 
49: static bool Uppercase = true;
50: 
51: /// <summary>
52: /// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
53: /// </summary>
54: /// <param name="value">Digit value</param>
55: /// <returns>Converted digit character</returns>
56: static constexpr char GetDigit(uint8 value)
57: {
58:     return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
59: }
60: 
61: /// <summary>
62: /// Calculated the amount of digits needed to represent an unsigned value of bits using base
63: /// </summary>
64: /// <param name="bits">Size of integer in bits</param>
65: /// <param name="base">Base to be used</param>
66: /// <returns>Maximum amount of digits needed</returns>
67: static constexpr int BitsToDigits(int bits, int base)
68: {
69:     int result = 0;
70:     uint64 value = 0xFFFFFFFFFFFFFFFF;
71:     if (bits < 64)
72:         value &= ((1ULL << bits) - 1);
73: 
74:     while (value > 0)
75:     {
76:         value /= base;
77:         result++;
78:     }
79: 
80:     return result;
81: }
82: 
83: /// <summary>
84: /// Serialize a 32 bit unsigned value to buffer.
85: ///
86: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
87: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
88: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
89: ///
90: /// Base is the digit base, which can range from 2 to 36.
91: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16).
92: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
93: /// </summary>
94: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
95: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
96: /// <param name="value">Value to be serialized</param>
97: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
98: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
99: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
100: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
101: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
102: {
103:     if ((base < 2) || (base > 36))
104:         return;
105: 
106:     int       numDigits = 0;
107:     uint64    divisor   = 1;
108:     uint64    divisorLast = 1;
109:     size_t    absWidth  = (width < 0) ? -width : width;
110:     const int numBits   = 32;
111:     const int maxDigits = BitsToDigits(numBits, base);
112:     while ((value >= divisor) && (numDigits <= maxDigits))
113:     {
114:         divisorLast = divisor;
115:         divisor *= base;
116:         ++numDigits;
117:     }
118:     divisor = divisorLast;
119: 
120:     size_t numChars = (numDigits > 0) ? numDigits : 1;
121:     if (showBase)
122:     {
123:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
124:     }
125:     if (absWidth > numChars)
126:         numChars = absWidth;
127:     if (numChars > bufferSize - 1) // Leave one character for \0
128:         return;
129: 
130:     char* bufferPtr = buffer;
131: 
132:     if (showBase)
133:     {
134:         if (base == 2)
135:         {
136:             *bufferPtr++ = '0';
137:             *bufferPtr++ = 'b';
138:         }
139:         else if (base == 8)
140:         {
141:             *bufferPtr++ = '0';
142:         }
143:         else if (base == 16)
144:         {
145:             *bufferPtr++ = '0';
146:             *bufferPtr++ = 'x';
147: 
148:         }
149:     }
150:     if (leadingZeros)
151:     {
152:         if (absWidth == 0)
153:             absWidth = maxDigits;
154:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
155:         {
156:             *bufferPtr++ = '0';
157:         }
158:     }
159:     while (numDigits > 0)
160:     {
161:         int digit = (value / divisor) % base;
162:         *bufferPtr++ = GetDigit(digit);
163:         --numDigits;
164:         divisor /= base;
165:     }
166:     *bufferPtr++ = '\0';
167: }
168: 
169: } // namespace baremetal
```

- Line 49: We define a value `Uppercase` which can be set to false to print hexadecimal values with lowercase letters. The default is uppercase.
- Line 56-59: We define a local function `GetDigit()` to return a 'digit' for the `value` specified. So `0`..`9` for 0 to 9, `A`..`Z` for 10 to 35, or `a`..`z` if `Uppercase` is false.
- Line 67-681: We define a local function `BitsToDigits()` to determine how many digits are needed to represent a type of size `bits` in base `base`. This uses a log`<n>` implementation for integers
- Line 101-167: We implement the `Serialize()` function
  - Line 103-104: We do a sanity check on the base
  - Line 106-118: We calculate how many digits are needed to represent the value using the base specified.
  Meanwhile we calculate a maximum divisor, while keeping the last, which we use when the condition turns false
  - Line 120-126: We set the minimum number of digits needed to 1, we take the prefix into account if desired, and if the width is specified, that is the minimum.
  - Line 127-128: We do a sanity check to see whether the characters to be written fit in the buffer
  - Line 132-149: We print the prefix if needed
  - Line 150-158: We print the leading zeros if needed
  - Line 159-165: We print the digits
  - Line 166: We end the string with a null character

#### main.cpp {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_UPDATE_THE_APPLICATION_CODE_MAINCPP}

Now we can update the application code and print the memory map.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/SysConfig.h>
3: #include <baremetal/Serialization.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: #include <baremetal/UART1.h>
7:
8: using namespace baremetal;
9:
10: int main()
11: {
12:     auto& uart = GetUART1();
13:     uart.WriteString("Hello World!\n");
14:
15:     char buffer[128];
16:     uart.WriteString("------------------------------------------------- ");
17:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_START, 10, 16, true, true);
18:     uart.WriteString(buffer);
19:     uart.WriteString(" MEM_KERNEL_START\n");
20:
21:     uart.WriteString(" Kernel image : Size ");
22:     Serialize(buffer, sizeof(buffer), KERNEL_MAX_SIZE, 10, 16, true, true);
23:     uart.WriteString(buffer);
24:     uart.WriteString("\n");
25:
26:     uart.WriteString("------------------------------------------------- ");
27:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_END, 10, 16, true, true);
28:     uart.WriteString(buffer);
29:     uart.WriteString(" MEM_KERNEL_END\n");
30:
31:     uart.WriteString(" Core 0 stack : Size ");
32:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
33:     uart.WriteString(buffer);
34:     uart.WriteString("\n");
35:
36:     uart.WriteString("------------------------------------------------- ");
37:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK, 10, 16, true, true);
38:     uart.WriteString(buffer);
39:     uart.WriteString(" MEM_KERNEL_STACK\n");
40:
41:     uart.WriteString(" Core 1 stack : Size ");
42:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
43:     uart.WriteString(buffer);
44:     uart.WriteString("\n");
45:
46:     uart.WriteString("------------------------------------------------- ");
47:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + KERNEL_STACK_SIZE, 10, 16, true, true);
48:     uart.WriteString(buffer);
49:     uart.WriteString("\n");
50:
51:     uart.WriteString(" Core 2 stack : Size ");
52:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
53:     uart.WriteString(buffer);
54:     uart.WriteString("\n");
55:
56:     uart.WriteString("------------------------------------------------- ");
57:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + 2 * KERNEL_STACK_SIZE, 10, 16, true, true);
58:     uart.WriteString(buffer);
59:     uart.WriteString("\n");
60:
61:     uart.WriteString(" Core 3 stack : Size ");
62:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
63:     uart.WriteString(buffer);
64:     uart.WriteString("\n");
65:
66:     uart.WriteString("------------------------------------------------- ");
67:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + 3 * KERNEL_STACK_SIZE, 10, 16, true, true);
68:     uart.WriteString(buffer);
69:     uart.WriteString("\n");
70:
71:     uart.WriteString(" Core 0 exception stack : Size ");
72:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
73:     uart.WriteString(buffer);
74:     uart.WriteString("\n");
75:
76:     uart.WriteString("------------------------------------------------- ");
77:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK, 10, 16, true, true);
78:     uart.WriteString(buffer);
79:     uart.WriteString(" MEM_EXCEPTION_STACK\n");
80:
81:     uart.WriteString(" Core 1 exception stack : Size ");
82:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
83:     uart.WriteString(buffer);
84:     uart.WriteString("\n");
85:
86:     uart.WriteString("------------------------------------------------- ");
87:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK + EXCEPTION_STACK_SIZE, 10, 16, true, true);
88:     uart.WriteString(buffer);
89:     uart.WriteString("\n");
90:
91:     uart.WriteString(" Core 2 exception stack : Size ");
92:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
93:     uart.WriteString(buffer);
94:     uart.WriteString("\n");
95:
96:     uart.WriteString("------------------------------------------------- ");
97:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK + 2 * EXCEPTION_STACK_SIZE, 10, 16, true, true);
98:     uart.WriteString(buffer);
99:     uart.WriteString("\n");
100:
101:     uart.WriteString(" Core 3 exception stack : Size ");
102:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
103:     uart.WriteString(buffer);
104:     uart.WriteString("\n");
105:
106:     uart.WriteString("------------------------------------------------- ");
107:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK_END, 10, 16, true, true);
108:     uart.WriteString(buffer);
109:     uart.WriteString(" MEM_EXCEPTION_STACK_END\n");
110:
111:     uart.WriteString(" Unused\n");
112:
113:     uart.WriteString("------------------------------------------------- ");
114:     Serialize(buffer, sizeof(buffer), MEM_COHERENT_REGION, 10, 16, true, true);
115:     uart.WriteString(buffer);
116:     uart.WriteString(" MEM_COHERENT_REGION\n");
117:
118:     uart.WriteString(" Coherent region : Size ");
119:     Serialize(buffer, sizeof(buffer), COHERENT_REGION_SIZE, 10, 16, true, true);
120:     uart.WriteString(buffer);
121:     uart.WriteString("\n");
122:
123:     uart.WriteString("------------------------------------------------- ");
124:     Serialize(buffer, sizeof(buffer), MEM_COHERENT_REGION + COHERENT_REGION_SIZE, 10, 16, true, true);
125:     uart.WriteString(buffer);
126:     uart.WriteString(" \n");
127:
128:     uart.WriteString("Wait 5 seconds\n");
129:     Timer::WaitMilliSeconds(5000);
130:
131:     uart.WriteString("Press r to reboot, h to halt\n");
132:     char ch{};
133:     while ((ch != 'r') && (ch != 'h'))
134:     {
135:         ch = uart.Read();
136:         uart.Write(ch);
137:     }
138:
139:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
140: }
```

The code should speak for itself. Notice that this code is very verbose. We'll improve that later.

### Update project configuration {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
File: d:\Projects\RaspberryPi\baremetal.github.shadow\code\libraries\baremetal\CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
38:     )
39: 
40: set(PROJECT_INCLUDES_PUBLIC
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
53:     )
54: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_MAILBOX_UPDATING_THE_MEMORY_MAP___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

When running the application, you can see the memory map being printed.

For Raspberry Pi 3:

```text
Starting up
Hello World!
------------------------------------------------- 0x0000080000 MEM_KERNEL_START
 Kernel image : Size 0x0000200000
------------------------------------------------- 0x0000280000 MEM_KERNEL_END
 Core 0 stack : Size 0x0000020000
------------------------------------------------- 0x00002A0000 MEM_KERNEL_STACK
 Core 1 stack : Size 0x0000020000
------------------------------------------------- 0x00002C0000
 Core 2 stack : Size 0x0000020000
------------------------------------------------- 0x00002E0000
 Core 3 stack : Size 0x0000020000
------------------------------------------------- 0x0000300000
 Core 0 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000308000 MEM_EXCEPTION_STACK
 Core 1 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000310000
 Core 2 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000318000
 Core 3 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000320000 MEM_EXCEPTION_STACK_END
 Unused
------------------------------------------------- 0x0000500000 MEM_COHERENT_REGION
 Coherent region : Size 0x0000100000
------------------------------------------------- 0x0000600000
Wait 5 seconds
Press r to reboot, h to halt
hHalt
```

For Raspberry Pi 4 / 5:

```text
Starting up
Hello World!
------------------------------------------------- 0x0000080000 MEM_KERNEL_START
 Kernel image : Size 0x0000200000
------------------------------------------------- 0x0000280000 MEM_KERNEL_END
 Core 0 stack : Size 0x0000020000
------------------------------------------------- 0x00002A0000 MEM_KERNEL_STACK
 Core 1 stack : Size 0x0000020000
------------------------------------------------- 0x00002C0000
 Core 2 stack : Size 0x0000020000
------------------------------------------------- 0x00002E0000
 Core 3 stack : Size 0x0000020000
------------------------------------------------- 0x0000300000
 Core 0 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000308000 MEM_EXCEPTION_STACK
 Core 1 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000310000
 Core 2 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000318000
 Core 3 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000320000 MEM_EXCEPTION_STACK_END
 Unused
------------------------------------------------- 0x0000500000 MEM_COHERENT_REGION
 Coherent region : Size 0x0000400000
------------------------------------------------- 0x0000900000
Wait 5 seconds
Press r to reboot, h to halt
hHalt
```

## Setting up for memory management - Step 2 {#TUTORIAL_10_MAILBOX_SETTING_UP_FOR_MEMORY_MANAGEMENT___STEP_2}

As a very small first step, we'll create a class with one single method to retrieve the Coherent page for a specified class of usage.
We will create a class `MemoryManager` for this, that we will extend in later tutorials when actually adding memory management.

### MemoryManager.h {#TUTORIAL_10_MAILBOX_SETTING_UP_FOR_MEMORY_MANAGEMENT___STEP_2_MEMORYMANAGERH}

Create the file `code/libraries/baremetal/include/baremetal/MemoryManager.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryManager.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryManager.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryManager
9: //
10: // Description : Memory handling
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
45: /// Memory management
46: 
47: /// <summary>
48: /// Page slot for requesting coherent memory region
49: /// </summary>
50: enum class CoherentPageSlot
51: {
52:     /// @brief Coherent memory page slot for Raspberry Pi mailbox
53:     PropertyMailbox = 0,
54: };
55: 
56: namespace baremetal {
57: 
58: /// <summary>
59: /// For now, handles assignment of coherent memory slots.
60: /// </summary>
61: class MemoryManager
62: {
63: public:
64:     static uintptr GetCoherentPage(CoherentPageSlot slot);
65: };
66: 
67: } // namespace baremetal
```

- Line 50-54, we declare an enum type to hold the kind of Coherent Page to request. For now this is only one kind, but this will be extended.
- Line 64: We define a static method `GetCoherentPage()` in `MemoryManager` to get the address for a Coherent Page.

### MemoryManager.cpp {#TUTORIAL_10_MAILBOX_SETTING_UP_FOR_MEMORY_MANAGEMENT___STEP_2_MEMORYMANAGERCPP}

Let's implement this method.

Create the file `code/libraries/baremetal/src/MemoryManager.cpp`

```cpp
File: code/libraries/baremetal/src/MemoryManager.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryManager.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryManager
9: //
10: // Description : Memory handling
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
40: #include <baremetal/MemoryManager.h>
41: 
42: #include <baremetal/SysConfig.h>
43: 
44: /// @file
45: /// Memory management implementation
46: 
47: using namespace baremetal;
48: 
49: /// <summary>
50: /// Return the coherent memory page (allocated with the GPU) for the requested page slot
51: /// </summary>
52: /// <param name="slot">Page slot to return the address for</param>
53: /// <returns>Page slot coherent memory address</returns>
54: uintptr MemoryManager::GetCoherentPage(CoherentPageSlot slot)
55: {
56:     uint64 pageAddress = MEM_COHERENT_REGION;
57: 
58:     pageAddress += static_cast<uint32>(slot) * PAGE_SIZE;
59: 
60:     return pageAddress;
61: }
```

- Line 56: We take the addres of the start of the coherent memory region
- Line 58: we multiply the integer value of the specific slot by the `PAGE_SIZE`, which is 64 Kb.

### Update project configuration {#TUTORIAL_10_MAILBOX_SETTING_UP_FOR_MEMORY_MANAGEMENT___STEP_2_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
File: d:\Projects\RaspberryPi\baremetal.github.shadow\code\libraries\baremetal\CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
39:     )
40: 
41: set(PROJECT_INCLUDES_PUBLIC
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
55:     )
56: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_MAILBOX_SETTING_UP_FOR_MEMORY_MANAGEMENT___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.
The behaviour however did not change.

## Adding the mailbox - Step 3 {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3}

We are now ready to add the acutal mailbox. The way it works is quite complex, but there is a pattern. We will take a layered approach here:
- The first thing we'll do is create an abstract interface for the mailbox called `IMailbox`
- We will then declare and implement the basic mailbox mechanics in a class `Mailbox`
- To handle requests to get and set properties we will need to package data in a certain way, we will declare and implement a class `RPIPropertiesInterface` for this
- Finally to do the actual requests, we add a class `RPIProperties` that will have methods for each of the properties we wish to get or set. We will keep this relatively simple for now

### BCMRegisters.h {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_BCMREGISTERSH}

As the mailbox implementation will use some specific registers, we need to add these.
Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
50: /// @brief Address of GPU memory accessible from ARM, mapped as cached memory
51: #define GPU_CACHED_BASE                 0x40000000
52: /// @brief Address of GPU memory accessible from ARM, mapped as uncached memory
53: #define GPU_UNCACHED_BASE               0xC0000000
54: 
55: /// @brief User base address of GPU memory from ARM
56: #define GPU_MEM_BASE                    GPU_UNCACHED_BASE
57: 
58: /// @brief Convert ARM address to GPU bus address (also works for aliases)
59: #define ARM_TO_GPU(addr)                (((addr) & ~0xC0000000) | GPU_MEM_BASE)
60: /// @brief Convert GPU bus address to ARM address (also works for aliases)
61: #define GPU_TO_ARM(addr)                ((addr) & ~0xC0000000)
62: 
63: #if BAREMETAL_RPI_TARGET == 3
64: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 3
65: #define RPI_BCM_IO_BASE                 0x3F000000
66: #elif BAREMETAL_RPI_TARGET == 4
67: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 4
68: #define RPI_BCM_IO_BASE                 0xFE000000
69: #define ARM_IO_BASE                     0xFF840000
70: #else
71: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 5
72: #define RPI_BCM_IO_BASE                 0x107C000000UL
73: #define ARM_IO_BASE                     0x107C000000UL
74: #endif
75: #if BAREMETAL_RPI_TARGET <= 4
76: /// @brief End address for Raspberry PI 3 / 4 BCM I/O
77: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)
78: #else
79: /// @brief End address for Raspberry PI 5 BCM I/O
80: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0x3FFFFFF)
81: #endif
...
104: //---------------------------------------------
105: // Raspberry Pi Mailbox
106: //---------------------------------------------
107: 
108: /// @brief Raspberry Pi Mailbox Registers base address. See @ref RASPBERRY_PI_MAILBOX
109: #define RPI_MAILBOX_BASE                RPI_BCM_IO_BASE + 0x0000B880
110: /// @brief Raspberry Pi Mailbox 0 (incoming) Read register. See @ref RASPBERRY_PI_MAILBOX
111: #define RPI_MAILBOX0_READ               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000000)
112: /// @brief Raspberry Pi Mailbox 0 (incoming) Poll register. See @ref RASPBERRY_PI_MAILBOX
113: #define RPI_MAILBOX0_POLL               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000010)
114: /// @brief Raspberry Pi Mailbox 0 (incoming) Sender register. See @ref RASPBERRY_PI_MAILBOX
115: #define RPI_MAILBOX0_SENDER             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000014)
116: /// @brief Raspberry Pi Mailbox 0 (incoming) Status register. See @ref RASPBERRY_PI_MAILBOX
117: #define RPI_MAILBOX0_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000018)
118: /// @brief Raspberry Pi Mailbox 0 (incoming) Configuration register. See @ref RASPBERRY_PI_MAILBOX
119: #define RPI_MAILBOX_CONFIG              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000001C)
120: /// @brief Raspberry Pi Mailbox 1 (outgoing) Write register. See @ref RASPBERRY_PI_MAILBOX
121: #define RPI_MAILBOX1_WRITE              reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000020)
122: /// @brief Raspberry Pi Mailbox 1 (incoming) Poll register. See @ref RASPBERRY_PI_MAILBOX
123: #define RPI_MAILBOX1_POLL               reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000030)
124: /// @brief Raspberry Pi Mailbox 1 (incoming) Sender register. See @ref RASPBERRY_PI_MAILBOX
125: #define RPI_MAILBOX1_SENDER             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000034)
126: /// @brief Raspberry Pi Mailbox 1 (outgoing) Status register. See @ref RASPBERRY_PI_MAILBOX
127: #define RPI_MAILBOX1_STATUS             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000038)
128: /// @brief Raspberry Pi Mailbox 1 (outgoing) Configuration register. See @ref RASPBERRY_PI_MAILBOX
129: #define RPI_MAILBOX1_CONFIG             reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000003C)
130: /// @brief Raspberry Pi Mailbox buffer requestCode success value. See @ref RASPBERRY_PI_MAILBOX
131: #define RPI_MAILBOX_RESPONSE_SUCCESS    BIT1(31)
132: /// @brief Raspberry Pi Mailbox buffer requestCode failure value
133: #define RPI_MAILBOX_RESPONSE_ERROR      BIT1(31) | BIT1(0)
134: /// @brief Raspberry Pi Mailbox buffer property tag response bit
135: #define RPI_MAILBOX_TAG_RESPONSE        BIT1(31)
136: /// @brief Raspberry Pi Mailbox 0 (incoming) Status register empty bit
137: #define RPI_MAILBOX_STATUS_EMPTY        BIT1(30)
138: /// @brief Raspberry Pi Mailbox 1 (outgoing) Status register full bit. See @ref RASPBERRY_PI_MAILBOX
139: #define RPI_MAILBOX_STATUS_FULL         BIT1(31)
140: /// @brief Raspberry Pi Mailbox buffer requestCode value for request. See @ref RASPBERRY_PI_MAILBOX
141: #define RPI_MAILBOX_REQUEST             0
142: 
```

- Line 51:  We define the address of the VC mapped to ARM address space when L2 caching is enabled (we will not use this)
- Line 53:  We define the address of the VC mapped to ARM address space when L2 caching is disabled
- Line 56:  We define the address of the VC mapped to ARM address we will use (the uncached variant)
- Line 59:  We define a macro to convert an ARM address to the GPU / VC
- Line 61:  We define a macro to convert an GPU / VC address to the ARM
- Line 108-141: We define the register addresses and values for the mailbox

The Mailbox peripheral is not well described, it is not mentioned in the official documentation. There is however information available.
More information on the Mailbox registers can be found in:
- [Raspberry Pi firmware wiki - Mailboxes](https://github.com/raspberrypi/firmware/wiki/Mailboxes)
- [Raspberry Pi firmware wiki - Accessing mailboxes](https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes)
- [Raspberry Pi firmware wiki - Mailbox property interface](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)

We'll dive a bit deeper into the information when discussing the actual code.

The Mailbox register addresses are all prefixed with `RPI_MAILBOX`.

### IMailbox.h {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_IMAILBOXH}

Create the file `code/libraries/baremetal/include/baremetal/IMailbox.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/IMailbox.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : IMailbox.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : IMailbox
9: //
10: // Description : Arm <-> VC mailbox abstract interface
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
45: /// Abstract Mailbox interface
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Mailbox channel
51: /// </summary>
52: enum class MailboxChannel
53: {
54:     /// Power management
55:     ARM_MAILBOX_CH_POWER = 0,
56:     /// Frame buffer
57:     ARM_MAILBOX_CH_FB = 1,
58:     /// Virtual UART
59:     ARM_MAILBOX_CH_VUART = 2,
60:     /// VCHIQ / GPU
61:     ARM_MAILBOX_CH_VCHIQ = 3,
62:     /// LEDs
63:     ARM_MAILBOX_CH_LEDS = 4,
64:     /// Buttons
65:     ARM_MAILBOX_CH_BTNS = 5,
66:     /// Touch screen
67:     ARM_MAILBOX_CH_TOUCH = 6,
68:     /// ?
69:     ARM_MAILBOX_CH_COUNT = 7,
70:     /// Properties / tags ARM -> VC
71:     ARM_MAILBOX_CH_PROP_OUT = 8,
72:     /// Properties / tags VC -> ARM
73:     ARM_MAILBOX_CH_PROP_IN = 9,
74: };
75: 
76: /// <summary>
77: /// Mailbox abstract interface
78: /// </summary>
79: class IMailbox
80: {
81: public:
82:     /// <summary>
83:     /// Default destructor needed for abstract interface
84:     /// </summary>
85:     virtual ~IMailbox() = default;
86: 
87:     /// <summary>
88:     /// Perform a write - read cycle on the mailbox
89:     /// </summary>
90:     /// <param name="address">Address of mailbox data block (converted to GPU address space)</param>
91:     /// <returns>Address of mailbox data block, should be equal to input address</returns>
92:     virtual uintptr WriteRead(uintptr address) = 0;
93: };
94: 
95: } // namespace baremetal
```

As you can see, there are multiple mailboxes available, which are identified by a channel number:

0. Power management
1. Framebuffer
2. Virtual UART
3. VCHIQ (GPU)
4. LEDs
5. Buttons
6. Touch screen
7.
8. Property tags (ARM -> VC)
9. Property tags (VC -> ARM)

For now we will only use channel 8 (`ARM_MAILBOX_CH_PROP_OUT`), channel 9 (`ARM_MAILBOX_CH_PROP_IN`) is currently not used at all.

### Mailbox.h {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_MAILBOXH}

Create the file `code/libraries/baremetal/include/baremetal/Mailbox.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Mailbox.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Mailbox.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Mailbox
9: //
10: // Description : Arm <-> VC mailbox handling
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
42: #include <baremetal/IMailbox.h>
43: #include <baremetal/MemoryAccess.h>
44: 
45: /// @file
46: /// Raspberry Pi Mailbox
47: 
48: namespace baremetal {
49: 
50: /// @brief Mailbox: Handles access to system parameters, stored in the VC
51: ///
52: /// The mailbox handles communication with the Raspberry Pi GPU using communication channels. The most frequently used is the ARM_MAILBOX_CH_PROP_OUT channel
53: class Mailbox : public IMailbox
54: {
55: private:
56:     /// <summary>
57:     /// Channel to be used for mailbox
58:     /// </summary>
59:     MailboxChannel m_channel;
60:     /// <summary>
61:     /// Memory access interface
62:     /// </summary>
63:     IMemoryAccess &m_memoryAccess;
64: 
65: public:
66:     Mailbox(MailboxChannel channel, IMemoryAccess &memoryAccess = GetMemoryAccess());
67: 
68:     uintptr WriteRead(uintptr address) override;
69: 
70: private:
71:     void    Flush();
72:     uintptr Read();
73:     void    Write(uintptr data);
74: };
75: 
76: } // namespace baremetal
```

- Line 59: We keep a MailboxChannel (the mailbox channel ID) passed in to the constructor
- Line 63: We also keep a reference to the MemoryAccess instance passed in to the constructor
- Line 66: We declare a constructor (this can be either used as a normal constructor and as one for testing)
- Line 68: We declare the mailbox interaction method, which is a write-read cycle.
This will send the address passed to the mailbox through its registers, and then read back the result when ready.
- Line 71: We declare a private method to clear the mailbox
- Line 72: We declare a private method to read the mailbox
- Line 73: We declare a private method to write the mailbox

### Mailbox.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_MAILBOXCPP}

Now let's implement the mailbox.

Create the file: `code/libraries/baremetal/src/Mailbox.cpp`

```cpp
File: code/libraries/baremetal/src/Mailbox.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Mailbox.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Mailbox
9: //
10: // Description : Arm <-> VC mailbox handling
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
40: #include <baremetal/Mailbox.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/Timer.h>
46: 
47: /// @file
48: /// Raspberry Pi Mailbox implementation
49: 
50: using namespace baremetal;
51: 
52: /// <summary>
53: /// Construct a mailbox
54: /// </summary>
55: /// <param name="channel">Channel to be used for communication</param>
56: /// <param name="memoryAccess">Memory access interface for registers, default to the singleton memory access instantiation</param>
57: Mailbox::Mailbox(MailboxChannel channel, IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
58:     : m_channel{ channel }
59:     , m_memoryAccess{ memoryAccess }
60: {
61: }
62: 
63: /// <summary>
64: /// Perform a write - read cycle on the mailbox for the selected channel (m_channel)
65: /// </summary>
66: /// <param name="address">Address of mailbox data block (converted to GPU address space)</param>
67: /// <returns>Address of mailbox data block, should be equal to input address</returns>
68: uintptr Mailbox::WriteRead(uintptr address)
69: {
70:     Flush();
71: 
72:     Write(address);
73: 
74:     uint32 result = Read();
75: 
76:     return result;
77: }
78: 
79: /// <summary>
80: /// Flush the mailbox for the selected channel (m_channel), by reading until it is empty. A short wait is added for synchronization reasons.
81: /// </summary>
82: void Mailbox::Flush()
83: {
84:     while (!(m_memoryAccess.Read32(RPI_MAILBOX0_STATUS) & RPI_MAILBOX_STATUS_EMPTY))
85:     {
86:         m_memoryAccess.Read32(RPI_MAILBOX0_READ);
87: 
88:         Timer::WaitMilliSeconds(20);
89:     }
90: }
91: 
92: /// <summary>
93: /// Read back the address of the data block to the mailbox for the selected channel (m_channel)
94: // The address should be equal to what was written, as the mailbox can only handle sequential requests for a channel
95: /// </summary>
96: /// <returns>The pointer to the mailbox data block passed in to Write() on success (in GPU address space).</returns>
97: uintptr Mailbox::Read()
98: {
99:     uintptr result;
100: 
101:     do
102:     {
103:         while (m_memoryAccess.Read32(RPI_MAILBOX0_STATUS) & RPI_MAILBOX_STATUS_EMPTY)
104:         {
105:             NOP();
106:         }
107: 
108:         result = static_cast<uintptr>(m_memoryAccess.Read32(RPI_MAILBOX0_READ));
109:     } while ((result & 0xF) != static_cast<uint32>(m_channel)); // channel number is in the lower 4 bits
110: 
111:     return result & ~0xF;
112: }
113: 
114: /// <summary>
115: /// Write to the mailbox on the selected channel
116: /// </summary>
117: /// <param name="data">Address of mailbox data block (converted to GPU address space)</param>
118: void Mailbox::Write(uintptr data)
119: {
120:     // Data must be 16 byte aligned
121:     if ((data & 0xF) != 0)
122:         return;
123: 
124:     while (m_memoryAccess.Read32(RPI_MAILBOX1_STATUS) & RPI_MAILBOX_STATUS_FULL)
125:     {
126:         NOP();
127:     }
128: 
129:     m_memoryAccess.Write32(RPI_MAILBOX1_WRITE, static_cast<uint32>(m_channel) | static_cast<uint32>(data)); // channel number is in the lower 4 bits
130: }
```

- Line 57-61: We implement the constructor, which is quite straightforward
- Line 68-77: We implement the `WriteRead()` method, which simply calls `Flush()`, `Write()` and then `Read()` and returns the value returned
- Line 82-90: We implement the `Flush()` method.
This keeps reading `RPI_MAILBOX0_READ` register while the `RPI_MAILBOX0_STATUS` register signals that it is not empty, by testing for bit `RPI_MAILBOX_STATUS_EMPTY`.
- Line 97-112: We implement the `Read()` method.
This waits until the `RPI_MAILBOX0_STATUS` register signals that it is not empty, executing NOP instructions.
It then reads the mailbox through the `RPI_MAILBOX0_READ` register, and checks whether the result matches the mailbox channel.
The result is a combination of the address (upper 28 bits, the lower 4 are expected to be 0) and the mailbox channel (lower 4 bits).
If the mailbox channel is not as expected, it keeps reading.
When successful, the mailbox channel is masked away, leaving only the address to return.
The returned address should be equal to the address written
- Line 118-130: We implement the `Write()` method.
This first does a sanity check on the address passed in. This should be 16 byte aligned, so the lower 4 bits should be 0.
This is because the lower 4 bits are used for the mailbox channel.
The method then waits until the `RPI_MAILBOX1_STATUS` register signals that it is not full, by checking the bit `RPI_MAILBOX_STATUS_FULL`,
executing NOP instructions.
And finally, it writes the combination of the address and the mailbox channel to the `RPI_MAILBOX1_WRITE` register

### Update the application code {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE}

#### main.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE_MAINCPP}

Now that we have a mailbox implemented, we can start to use it.
Let's try to retrieve the board serial number.
The first steps will feel awkward, but we'll create a more logical approach step by step.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Mailbox.h>
4: #include <baremetal/MemoryManager.h>
5: #include <baremetal/SysConfig.h>
6: #include <baremetal/Serialization.h>
7: #include <baremetal/System.h>
8: #include <baremetal/Timer.h>
9: #include <baremetal/UART1.h>
10:
11: using namespace baremetal;
12:
13: int main()
14: {
15:     auto& uart = GetUART1();
16:     uart.WriteString("Hello World!\n");
17:
18:     char buffer[128];
19:     uart.WriteString("------------------------------------------------- ");
20:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_START, 10, 16, true, true);
21:     uart.WriteString(buffer);
22:     uart.WriteString(" MEM_KERNEL_START\n");
23:
24:     uart.WriteString(" Kernel image : Size ");
25:     Serialize(buffer, sizeof(buffer), KERNEL_MAX_SIZE, 10, 16, true, true);
26:     uart.WriteString(buffer);
27:     uart.WriteString("\n");
28:
29:     uart.WriteString("------------------------------------------------- ");
30:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_END, 10, 16, true, true);
31:     uart.WriteString(buffer);
32:     uart.WriteString(" MEM_KERNEL_END\n");
33:
34:     uart.WriteString(" Core 0 stack : Size ");
35:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
36:     uart.WriteString(buffer);
37:     uart.WriteString("\n");
38:
39:     uart.WriteString("------------------------------------------------- ");
40:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK, 10, 16, true, true);
41:     uart.WriteString(buffer);
42:     uart.WriteString(" MEM_KERNEL_STACK\n");
43:
44:     uart.WriteString(" Core 1 stack : Size ");
45:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
46:     uart.WriteString(buffer);
47:     uart.WriteString("\n");
48:
49:     uart.WriteString("------------------------------------------------- ");
50:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + KERNEL_STACK_SIZE, 10, 16, true, true);
51:     uart.WriteString(buffer);
52:     uart.WriteString("\n");
53:
54:     uart.WriteString(" Core 2 stack : Size ");
55:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
56:     uart.WriteString(buffer);
57:     uart.WriteString("\n");
58:
59:     uart.WriteString("------------------------------------------------- ");
60:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + 2 * KERNEL_STACK_SIZE, 10, 16, true, true);
61:     uart.WriteString(buffer);
62:     uart.WriteString("\n");
63:
64:     uart.WriteString(" Core 3 stack : Size ");
65:     Serialize(buffer, sizeof(buffer), KERNEL_STACK_SIZE, 10, 16, true, true);
66:     uart.WriteString(buffer);
67:     uart.WriteString("\n");
68:
69:     uart.WriteString("------------------------------------------------- ");
70:     Serialize(buffer, sizeof(buffer), MEM_KERNEL_STACK + 3 * KERNEL_STACK_SIZE, 10, 16, true, true);
71:     uart.WriteString(buffer);
72:     uart.WriteString("\n");
73:
74:     uart.WriteString(" Core 0 exception stack : Size ");
75:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
76:     uart.WriteString(buffer);
77:     uart.WriteString("\n");
78:
79:     uart.WriteString("------------------------------------------------- ");
80:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK, 10, 16, true, true);
81:     uart.WriteString(buffer);
82:     uart.WriteString(" MEM_EXCEPTION_STACK\n");
83:
84:     uart.WriteString(" Core 1 exception stack : Size ");
85:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
86:     uart.WriteString(buffer);
87:     uart.WriteString("\n");
88:
89:     uart.WriteString("------------------------------------------------- ");
90:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK + EXCEPTION_STACK_SIZE, 10, 16, true, true);
91:     uart.WriteString(buffer);
92:     uart.WriteString("\n");
93:
94:     uart.WriteString(" Core 2 exception stack : Size ");
95:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
96:     uart.WriteString(buffer);
97:     uart.WriteString("\n");
98:
99:     uart.WriteString("------------------------------------------------- ");
100:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK + 2 * EXCEPTION_STACK_SIZE, 10, 16, true, true);
101:     uart.WriteString(buffer);
102:     uart.WriteString("\n");
103:
104:     uart.WriteString(" Core 3 exception stack : Size ");
105:     Serialize(buffer, sizeof(buffer), EXCEPTION_STACK_SIZE, 10, 16, true, true);
106:     uart.WriteString(buffer);
107:     uart.WriteString("\n");
108:
109:     uart.WriteString("------------------------------------------------- ");
110:     Serialize(buffer, sizeof(buffer), MEM_EXCEPTION_STACK_END, 10, 16, true, true);
111:     uart.WriteString(buffer);
112:     uart.WriteString(" MEM_EXCEPTION_STACK_END\n");
113:
114:     uart.WriteString(" Unused\n");
115:
116:     uart.WriteString("----------h--------------------------------------- ");
117:     Serialize(buffer, sizeof(buffer), MEM_COHERENT_REGION, 10, 16, true, true);
118:     uart.WriteString(buffer);
119:     uart.WriteString(" MEM_COHERENT_REGION\n");
120:
121:     uart.WriteString(" Coherent region : Size ");
122:     Serialize(buffer, sizeof(buffer), COHERENT_REGION_SIZE, 10, 16, true, true);
123:     uart.WriteString(buffer);
124:     uart.WriteString("\n");
125:
126:     uart.WriteString("------------------------------------------------- ");
127:     Serialize(buffer, sizeof(buffer), MEM_COHERENT_REGION + COHERENT_REGION_SIZE, 10, 16, true, true);
128:     uart.WriteString(buffer);
129:     uart.WriteString(" \n");
130:
131:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
132:     auto mailboxBuffer = MemoryManager::GetCoherentPage(CoherentPageSlot::PropertyMailbox);
133:     uint32* mailboxData = reinterpret_cast<uint32*>(mailboxBuffer);
134:     // Property buffer
135:     mailboxData[0] = 32; // Buffer size (property buffer + tags)
136:     mailboxData[1] = RPI_MAILBOX_REQUEST;
137:     // Tag get board serial
138:     mailboxData[2] = 0x00010004; // Tag for property get board serial
139:     mailboxData[3] = 8; // Size of buffer (request and response)
140:     mailboxData[4] = 0; // Size of request
141:     mailboxData[5] = 0; // return value low word
142:     mailboxData[6] = 0; // return value high word
143:     // Tag end
144:     mailboxData[7] = 0; // Tag for end of list
145:
146:     uintptr bufferAddress = ARM_TO_GPU(reinterpret_cast<uintptr>(mailboxBuffer));
147:
148:     DataSyncBarrier();
149:
150:     uart.WriteString("Send\n");
151:     for (int i = 0; i < 8; ++i)
152:     {
153:         Serialize(buffer, sizeof(buffer), mailboxData[i], 0, 16, true, true);
154:         uart.WriteString(buffer);
155:         uart.WriteString("\n");
156:     }
157:     if ((bufferAddress == mailbox.WriteRead(bufferAddress)) && (mailboxData[1] == RPI_MAILBOX_RESPONSE_SUCCESS))
158:     {
159:         uart.WriteString("Receive\n");
160:         for (int i = 0; i < 8; ++i)
161:         {
162:             Serialize(buffer, sizeof(buffer), mailboxData[i], 0, 16, true, true);
163:             uart.WriteString(buffer);
164:             uart.WriteString("\n");
165:         }
166:         uart.WriteString("Mailbox call succeeded\n");
167:         uart.WriteString("Serial: ");
168:         Serialize(buffer, sizeof(buffer), mailboxData[6], 8, 16, false, true);
169:         uart.WriteString(buffer);
170:         Serialize(buffer, sizeof(buffer), mailboxData[5], 8, 16, false, true);
171:         uart.WriteString(buffer);
172:         uart.WriteString("\n");
173:     }
174:     else
175:     {
176:         uart.WriteString("Mailbox call failed\n");
177:     }
178:
179:     DataMemBarrier();
180:
181:     uart.WriteString("Wait 5 seconds\n");
182:     Timer::WaitMilliSeconds(5000);
183:
184:     uart.WriteString("Press r to reboot, h to halt\n");
185:     char ch{};
186:     while ((ch != 'r') && (ch != 'h'))
187:     {
188:         ch = uart.Read();
189:         uart.Write(ch);
190:     }
191:
192:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
193: }
```

- Line 2-4: We need to include `BCMRegisters.h`, `Mailbox.h` and `MemoryManager.h`.
- Line 131: We instantiate a mailbox, for the `ARM_MAILBOX_CH_PROP_OUT` channel. The `MemoryAccess` instance is the default singleton.
- Line 132: We request the coherent page for the mailbox from `MemoryManager` as the mailbox buffer
- Line 133: We cast this to a uint32 pointer so we can access the words easily. All data in the buffer transferred to the mailbox consists of 32 bit words
- Line 135-144: We fill the buffer with the correct information to request the board serial number. This will be described shortly
- Line 146: We convert the address of the buffer from ARM address space to VC address space
- Line 148: We synchronize the memory access with other cores and the VC using `DataSyncBarrier()`
- Line 150-156: We print the values we're sending to the mailbox
- Line 157: We call the `WriteRead()` method on the mailbox.
If the returned address is the same as the address sent, and the request code (the second word) signals success, the call was successful
- Line 159-165: We print the values we receive from the mailbox
- Line 166-172: We print a success message and the serial number if the call succeeeds
- Line 176: We print a failure message if the call fails
- Line 179: We again synchronize the memory access with other cores and the VC using `DataMemBarrier()` (note the different call / assembly instruction, we will not explain that here, it has to do with releasing and claiming memory)

#### Mailbox buffer organization {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE_MAILBOX_BUFFER_ORGANIZATION}

The images below show the structure of the block sent to and received back from the mailbox.

<img src="images/mailbox-structure-send.png"  alt="Mailbox block structure send" width="1000"/>

<img src="images/mailbox-structure-receive.png"  alt="Mailbox block structure receive" width="1000"/>

In general, the structure of the block remains the same. Every tag stays in the same location, if it is handled by the VC, bit 31 of the request / response code is set to 1. If all tag requests were handled successfully, bit 0 of request code is set to 0, if something failed it is set to 1.
If the request was handled successfully, bit 0 of the response code is 0, otherwise it is 1.

Notice that the size in the tag only covers the tag buffer, but the buffer size in the complete block covers everything, including the header.

So what it comes down to, is that we fill a buffer, with a 8 byte header,
followed by all the requests, each having a 12 byte header and then the request specific buffers.
We end the request with a special end tag.

#### ARMInstructions.h {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE_ARMINSTRUCTIONSH}

We used a new instruction `DataMemBarrier()` that needs to be added.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
50: /// @brief Data sync barrier
51: #define DataSyncBarrier()               asm volatile ("dsb sy" ::: "memory")
52: /// @brief Data memory barrier
53: #define DataMemBarrier()                asm volatile ("dmb sy" ::: "memory")
...
```

### Update project configuration {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
40:     )
41: 
42: set(PROJECT_INCLUDES_PUBLIC
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
58:     )
59: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see the printout of the values sent to the mailbox, and the values returned, as well as the board serial number.
In QEMU you will get a serial number 0000000000000000 as it is not implemented

On my Raspberry Pi 3B:

```text
Starting up
Hello World!
------------------------------------------------- 0x0000080000 MEM_KERNEL_START
 Kernel image : Size 0x0000200000
------------------------------------------------- 0x0000280000 MEM_KERNEL_END
 Core 0 stack : Size 0x0000020000
------------------------------------------------- 0x00002A0000 MEM_KERNEL_STACK
 Core 1 stack : Size 0x0000020000
------------------------------------------------- 0x00002C0000
 Core 2 stack : Size 0x0000020000
------------------------------------------------- 0x00002E0000
 Core 3 stack : Size 0x0000020000
------------------------------------------------- 0x0000300000
 Core 0 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000308000 MEM_EXCEPTION_STACK
 Core 1 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000310000
 Core 2 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000318000
 Core 3 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000320000 MEM_EXCEPTION_STACK_END
 Unused
------------------------------------------------- 0x0000500000 MEM_COHERENT_REGION
 Coherent region : Size 0x0000100000
------------------------------------------------- 0x0000600000
Send
0x00000020
0x00000000
0x00010004
0x00000008
0x00000000
0x00000000
0x00000000
0x00000000
Receive
0x00000020
0x80000000
0x00010004
0x00000008
0x80000008
0xC3D6D0CB
0x00000000
0x00000000
Mailbox call succeeded
Serial: 00000000C3D6D0CB
Wait 5 seconds
Press r to reboot, h to halt
```

On my Raspberry Pi 4B:
```text
Starting up
Hello World!
------------------------------------------------- 0x0000080000 MEM_KERNEL_START
 Kernel image : Size 0x0000200000
------------------------------------------------- 0x0000280000 MEM_KERNEL_END
 Core 0 stack : Size 0x0000020000
------------------------------------------------- 0x00002A0000 MEM_KERNEL_STACK
 Core 1 stack : Size 0x0000020000
------------------------------------------------- 0x00002C0000
 Core 2 stack : Size 0x0000020000
------------------------------------------------- 0x00002E0000
 Core 3 stack : Size 0x0000020000
------------------------------------------------- 0x0000300000
 Core 0 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000308000 MEM_EXCEPTION_STACK
 Core 1 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000310000
 Core 2 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000318000
 Core 3 exception stack : Size 0x0000008000
------------------------------------------------- 0x0000320000 MEM_EXCEPTION_STACK_END
 Unused
------------------------------------------------- 0x0000500000 MEM_COHERENT_REGION
 Coherent region : Size 0x0000100000
------------------------------------------------- 0x0000600000
Send
0x00000020
0x00000000
0x00010004
0x00000008
0x00000000
0x00000000
0x00000000
0x00000000
Receive
0x00000020
0x80000000
0x00010004
0x00000008
0x80000008
0xD18E8B28
0x10000000
0x00000000
Mailbox call succeeded
Serial: 10000000D18E8B28
Wait 5 seconds
Press r to reboot, h to halt
```

## Adding the properties interface - Step 4 {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4}

So we've seen the mailbox work, but also saw we need to do quite some work to fill the data block correctly.
Let's introduce a class that can handle all this work for us.

### RPIPropertiesInterface.h {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_RPIPROPERTIESINTERFACEH}

First we'll add a new class.

Create the file `code/libraries/baremetal/include/baremetal/RPIPropertiesInterface.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/RPIPropertiesInterface.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : RPIPropertiesInterface.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : RPIPropertiesInterface
9: //
10: // Description : Access to BCM2835/6/7 properties using mailbox
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
42: #include <stdlib/Macros.h>
43: #include <baremetal/IMailbox.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: /// @file
47: /// Functionality handling for Raspberry Pi Mailbox
48: 
49: namespace baremetal {
50: 
51: /// @brief Raspberry Pi mailbox property tags
52: enum class PropertyID : uint32
53: {
54:     /// @brief End tag. Should always be last tag in Mailbox buffer
55:     PROPTAG_END                     = 0x00000000,
56:     /// @brief Get firmware revision
57:     PROPTAG_GET_FIRMWARE_REVISION   = 0x00000001,
58:     /// @brief Set cursor info. Defines graphical cursor (width, height, bitmap and hotspot position
59:     PROPTAG_SET_CURSOR_INFO         = 0x00008010,
60:     /// @brief Set cursor state. Defines location and visibility of graphical cursor
61:     PROPTAG_SET_CURSOR_STATE        = 0x00008011,
62:     /// @brief Get Raspberry Pi board model
63:     PROPTAG_GET_BOARD_MODEL         = 0x00010001,
64:     /// @brief Get Raspberry Pi board revision
65:     PROPTAG_GET_BOARD_REVISION      = 0x00010002,
66:     /// @brief Get Raspberry Pi board MAC address
67:     PROPTAG_GET_MAC_ADDRESS         = 0x00010003,
68:     /// @brief Raspberry Pi board serial number
69:     PROPTAG_GET_BOARD_SERIAL        = 0x00010004,
70:     /// @brief Get ARM memory base address and size
71:     PROPTAG_GET_ARM_MEMORY          = 0x00010005,
72:     /// @brief Get VideoCore memory base address and size
73:     PROPTAG_GET_VC_MEMORY           = 0x00010006,
74:     /// @brief Get power state for a device
75:     PROPTAG_GET_POWER_STATE         = 0x00020001,
76:     /// @brief Set power state for a device
77:     PROPTAG_SET_POWER_STATE         = 0x00028001,
78:     /// @brief Get clock rate for a clock ID in Hz
79:     PROPTAG_GET_CLOCK_RATE          = 0x00030002,
80:     /// @brief Get maximum clock rate for a clock ID in Hz
81:     PROPTAG_GET_MAX_CLOCK_RATE      = 0x00030004,
82:     /// @brief Get temperature for a specific ID in thousands of a degree Celsius
83:     PROPTAG_GET_TEMPERATURE         = 0x00030006,
84:     /// @brief Get minimum clock rate for a clock ID in Hz
85:     PROPTAG_GET_MIN_CLOCK_RATE      = 0x00030007,
86:     /// @brief Get turbo setting
87:     PROPTAG_GET_TURBO               = 0x00030009,
88:     /// @brief Get maximum safe temperature for a specific ID in thousands of a degree Celsius
89:     PROPTAG_GET_MAX_TEMPERATURE     = 0x0003000A,
90:     /// @brief Read and return EDID for attached HDMI/DVI device
91:     PROPTAG_GET_EDID_BLOCK          = 0x00030020,
92:     /// @brief Get onboard LED status (status LED, power LED)
93:     PROPTAG_GET_LED_STATE           = 0x00030041,
94:     /// @brief @todo To be defined
95:     PROPTAG_GET_THROTTLED           = 0x00030046,
96:     /// @brief Get measured clock rate for a clock ID in Hz
97:     PROPTAG_GET_CLOCK_RATE_MEASURED = 0x00030047,
98:     /// @brief @todo To be defined
99:     PROPTAG_NOTIFY_XHCI_RESET       = 0x00030058,
100:     /// @brief @todo To be defined
101:     PROPTAG_TEST_LED_STATE          = 0x00034041,
102:     /// @brief Set clock rate for a clock ID in Hz
103:     PROPTAG_SET_CLOCK_RATE          = 0x00038002,
104:     /// @brief Set turbo state
105:     PROPTAG_SET_TURBO               = 0x00038009,
106:     /// @brief @todo To be defined
107:     PROPTAG_SET_DOMAIN_STATE        = 0x00038030,
108:     /// @brief @todo To be defined
109:     PROPTAG_SET_LED_STATE           = 0x00038041,
110:     /// @brief @todo To be defined
111:     PROPTAG_SET_SDHOST_CLOCK        = 0x00038042,
112:     /// @brief @todo To be defined
113:     PROPTAG_ALLOCATE_DISPLAY_BUFFER = 0x00040001,
114:     /// @brief @todo To be defined
115:     PROPTAG_GET_DISPLAY_DIMENSIONS  = 0x00040003,
116:     /// @brief @todo To be defined
117:     PROPTAG_GET_PITCH               = 0x00040008,
118:     /// @brief @todo To be defined
119:     PROPTAG_GET_TOUCHBUF            = 0x0004000F,
120:     /// @brief @todo To be defined
121:     PROPTAG_GET_GPIO_VIRTBUF        = 0x00040010,
122:     /// @brief @todo To be defined
123:     PROPTAG_GET_NUM_DISPLAYS        = 0x00040013,
124:     /// @brief @todo To be defined
125:     PROPTAG_SET_PHYS_WIDTH_HEIGHT   = 0x00048003,
126:     /// @brief @todo To be defined
127:     PROPTAG_SET_VIRT_WIDTH_HEIGHT   = 0x00048004,
128:     /// @brief @todo To be defined
129:     PROPTAG_SET_DEPTH               = 0x00048005,
130:     /// @brief @todo To be defined
131:     PROPTAG_SET_PIXEL_ORDER         = 0x00048006,
132:     /// @brief @todo To be defined
133:     PROPTAG_SET_VIRTUAL_OFFSET      = 0x00048009,
134:     /// @brief @todo To be defined
135:     PROPTAG_SET_PALETTE             = 0x0004800B,
136:     /// @brief @todo To be defined
137:     PROPTAG_WAIT_FOR_VSYNC          = 0x0004800E,
138:     /// @brief @todo To be defined
139:     PROPTAG_SET_BACKLIGHT           = 0x0004800F,
140:     /// @brief @todo To be defined
141:     PROPTAG_SET_DISPLAY_NUM         = 0x00048013,
142:     /// @brief @todo To be defined
143:     PROPTAG_SET_TOUCHBUF            = 0x0004801F,
144:     /// @brief @todo To be defined
145:     PROPTAG_SET_GPIO_VIRTBUF        = 0x00048020,
146:     /// @brief @todo To be defined
147:     PROPTAG_GET_COMMAND_LINE        = 0x00050001,
148:     /// @brief @todo To be defined
149:     PROPTAG_GET_DMA_CHANNELS        = 0x00060001,
150: };
151: 
152: /// @brief Buffer passed to the Raspberry Pi Mailbox
153: struct MailboxBuffer
154: {
155:     /// @brief Total size of buffer in bytes, including bufferSize field. Buffer must be aligned to 16 bytes
156:     uint32 bufferSize;
157:     /// @brief Request code
158:     uint32 requestCode;
159:     /// @brief Property tags to be handled, ended by end tag (PROPTAG_END). Each tag must be aligned to 4 bytes
160:     uint8  tags[0];
161:     // end tag follows
162: } PACKED;
163: 
164: /// @brief Property tag, one for each request
165: struct PropertyTag
166: {
167:     /// @brief Property ID, see PropertyID
168:     uint32 tagID;
169:     /// @brief Size of property tag buffer in bytes, so excluding tagID, tagBufferSize and tagRequestResponse, must be aligned to 4 bytes
170:     uint32 tagBufferSize;
171:     /// @brief Size of buffer for return data in bytes and return status
172:     uint32 tagRequestResponse;
173:     /// @brief Property tag request and response data, padded to align to 4 bytes
174:     uint8  tagBuffer[0];
175: } PACKED;
176: 
177: /// <summary>
178: /// Basic tag structure for a simple property request sending or receiving a 32 bit unsigned number.
179: ///
180: /// This is also used for sanity checks on the size of the request
181: /// </summary>
182: struct PropertyTagSimple
183: {
184:     /// @brief Tag ID of the the requested property
185:     PropertyTag tag;
186:     /// @brief A 32 bit unsigned value being send or requested
187:     uint32   value;
188: } PACKED;
189: 
190: /// <summary>
191: /// End tag structure to end the tag list.
192: ///
193: /// This has a different structure from normal tags, as it is only the tag ID
194: /// </summary>
195: struct PropertyTagEnd
196: {
197:     /// @brief Tag ID of the the requested property
198:     uint32 tagID;
199: } PACKED;
200: 
201: /// <summary>
202: /// Low level functionality for requests on Mailbox interface
203: /// </summary>
204: class RPIPropertiesInterface
205: {
206: private:
207:     /// @brief Reference to mailbox for functions requested
208:     IMailbox &m_mailbox;
209: 
210: public:
211:     explicit RPIPropertiesInterface(IMailbox &mailbox);
212: 
213:     bool GetTag(PropertyID tagID, void *tag, unsigned tagSize);
214: 
215: private:
216:     size_t FillTag(PropertyID tagID, void *tag, unsigned tagSize);
217:     bool   CheckTagResult(void *tag);
218:     bool   GetTags(void *tags, unsigned tagsSize);
219: };
220: 
221: } // namespace baremetal
```

- Line 52-150: We define all the known property tag IDs as an enum type `PropertyID`.
These can be found in the [Raspberry Pi firmware wiki](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)
- Line 153-162: We declare a structure for the mailbox buffer `MailboxBuffer`.
This contains the fields for the mailbox buffer shown in the image in [the application update section above](#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE):
  - bufferSize: The total buffer size of all tags, and the mailbox buffer header, including padding
  - requestCode: The mailbox request code (always set to 0 on request)
  - tags: The space used for the tags, as a placeholder
  - Notice that this struct has property `PACKED`
- Line 165-175: We declare a structure for the property tag `PropertyTag`.
This contains the fields for the tag shown in the image in [the application update section above](#TUTORIAL_10_MAILBOX_ADDING_THE_MAILBOX___STEP_3_UPDATE_THE_APPLICATION_CODE):
  - tagID: The property tag id
  - tagBufferSize: The size of the tag buffer
  - tagRequestResponse: The tag request / response code
  - tagBuffer: The tag buffer contents, as a placeholder
  - Notice that this struct has property `PACKED`
- Line 182-188: We declare a structure for a simple property `PropertyTagSimple` which only holds a single 32 bit value.
This will also be used for sanity checks on the tag sizes. Again this has the property `PACKED`
- Line 195-199: We declare a structure for the end tag `PropertyTagEnd` which only holds the tag ID.
This will be used to end the tag list. Again this has the property `PACKED`
- Line 204-219: We declare the RPIPropertiesInterface class.
  - Line 208: We declare a reference `m_mailbox` to the mailbox instance passed in through the constructor
  - Line 211: We declare the constructor, which receives a Mailbox instance
  - Line 213: We declare the method `GetTag()` to request a property. This has three parameters:
    - tagID: The property tag ID, from the enum specied in `PropertyID`
    - tag: a pointer to a buffer that contains sufficient information for the tag request and its response.
We will declare types for this per property
    - tagSize: The size of the buffer passed as `tag`
  - Line 216: We declare a private method `FillTag()` to fill the tag information with the correct structure for the request (tag ID and tag buffer size)
  - Line 217: We declare a private method `CheckTagResult()` to check the result of a mailbox call (verifiying the returned address)
  - Line 218: We declare a private method `GetTags()` to perform the actual call.
This will fill in the complete mailbox buffer, in the region retrieved from the memory manager for the coherent page,
convert the address, and use the `Mailbox` to perform the call.

You will notice that the structures declared in Line 153-162, 165-175 and 182-188 use the keyword `PACKED`.
We may be using the keyword `ALIGN` as well later.
We will add the definitions for this in `Macros.h`.
The reason for a definition is to make it possible to redefine this for a different compiler.

### Macros.h {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_MACROSH}

Let's add the definitions for PACKED and ALIGN.

Update the file `code/libraries/stdlib/include/stdlib/Macros.h`.

```cpp
File: code/libraries/stdlib/include/stdlib/Macros.h
45: /// @brief Make a struct packed (GNU compiler only)
46: #define PACKED              __attribute__ ((packed))
47: /// @brief Make a struct have alignment of n bytes (GNU compiler only)
48: #define ALIGN(n)            __attribute__ ((aligned (n)))
49: 
50: /// @brief Make a variable a weak instance (GCC compiler only)
51: #define WEAK                __attribute__ ((weak))
```

### RPIPropertiesInterface.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_RPIPROPERTIESINTERFACECPP}

Now we can implement the `RPIPropertiesInterface` class.

Create the file `code/libraries/baremetal/src/RPIPropertiesInterface.cpp`.

```cpp
File: code/libraries/baremetal/src/RPIPropertiesInterface.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : RPIPropertiesInterface.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : RPIPropertiesInterface
9: //
10: // Description : Access to BCM2835/6/7 properties using mailbox
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
40: #include <baremetal/RPIPropertiesInterface.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/ARMInstructions.h>
44: #include <baremetal/BCMRegisters.h>
45: #include <baremetal/MemoryManager.h>
46: 
47: /// @file
48: /// Functionality handling for Raspberry Pi Mailbox implementation
49: 
50: namespace baremetal {
51: 
52: /// <summary>
53: /// Constructs a RPI properties interface object
54: /// </summary>
55: /// <param name="mailbox">Mailbox to be used for requests. Can be a fake for testing purposes</param>
56: RPIPropertiesInterface::RPIPropertiesInterface(IMailbox &mailbox)
57:     : m_mailbox{mailbox}
58: {
59: }
60: 
61: /// <summary>
62: /// Request property tag. The tag data for request must be filled in, the header will be filled in to the buffer.
63: /// The buffer must be large enough to hold the complete tag including its header.
64: /// On successful return, the buffer will be filled with the response data
65: /// </summary>
66: /// <param name="tagID">Property tag to be requested</param>
67: /// <param name="tag">Buffer to tag data, large enough to hold complete Property</param>
68: /// <param name="tagSize">Size of the tag data buffer in bytes</param>
69: /// <returns>Return true on success, false on failure</returns>
70: bool RPIPropertiesInterface::GetTag(PropertyID tagID, void *tag, unsigned tagSize)
71: {
72:     if (FillTag(tagID, tag, tagSize) != tagSize)
73:         return false;
74: 
75:     auto result = GetTags(tag, tagSize);
76: 
77:     if (!result)
78:     {
79:         return false;
80:     }
81: 
82:     return CheckTagResult(tag);
83: }
84: 
85: /// <summary>
86: /// Check whether the property tag was successfully requested, by checking the tagRequestResponse field in the Property header
87: /// </summary>
88: /// <param name="tag">Buffer to property tag data</param>
89: /// <returns>Return true on success, false on failure</returns>
90: bool RPIPropertiesInterface::CheckTagResult(void *tag)
91: {
92:     PropertyTag *header = reinterpret_cast<PropertyTag *>(tag);
93: 
94:     if ((header->tagRequestResponse & RPI_MAILBOX_TAG_RESPONSE) == 0)
95:         return false;
96: 
97:     header->tagRequestResponse &= ~RPI_MAILBOX_TAG_RESPONSE;
98:     return (header->tagRequestResponse != 0);
99: }
100: 
101: /// <summary>
102: /// Fill in tag header for the requested property tag.
103: /// </summary>
104: /// <param name="tagID">Property tag to be requested</param>
105: /// <param name="tag">Buffer to tag data, large enough to hold complete Property</param>
106: /// <param name="tagSize">Size of the tag data buffer in bytes</param>
107: /// <returns>Tag size in bytes</returns>
108: size_t RPIPropertiesInterface::FillTag(PropertyID tagID, void *tag, unsigned tagSize)
109: {
110:     if ((tag == nullptr) || (tagSize < sizeof(PropertyTagSimple)))
111:         return 0;
112: 
113:     PropertyTag *header     = reinterpret_cast<PropertyTag *>(tag);
114:     header->tagID           = static_cast<uint32>(tagID);
115:     header->tagBufferSize   = tagSize - sizeof(PropertyTag);
116:     header->tagRequestResponse = 0;
117: 
118:     return tagSize;
119: }
120: 
121: /// <summary>
122: /// Fill in the Mailbox buffer with the tags requested, and perform the request.
123: /// Will fill in the mailbox buffer header, and the tag data, append the end tag, and perform the mailbox request.
124: /// </summary>
125: /// <param name="tags">Buffer to tag data, for all requested properties, except the end tag</param>
126: /// <param name="tagsSize">Size of the tag data buffer in bytes</param>
127: /// <returns>Return true on success, false on failure</returns>
128: bool RPIPropertiesInterface::GetTags(void *tags, unsigned tagsSize)
129: {
130:     if ((tags == nullptr) || (tagsSize < sizeof(PropertyTagSimple)))
131:         return false;
132: 
133:     unsigned bufferSize = sizeof(MailboxBuffer) + tagsSize + sizeof(PropertyTagEnd);
134:     if ((bufferSize & 3) != 0)
135:         return false;
136: 
137:     MailboxBuffer *buffer = reinterpret_cast<MailboxBuffer *>(MemoryManager::GetCoherentPage(CoherentPageSlot::PropertyMailbox));
138: 
139:     buffer->bufferSize  = bufferSize;
140:     buffer->requestCode = RPI_MAILBOX_REQUEST;
141:     memcpy(buffer->tags, tags, tagsSize);
142: 
143:     PropertyTagEnd *endTag = reinterpret_cast<PropertyTagEnd *>(buffer->tags + tagsSize);
144:     endTag->tagID = static_cast<uint32>(PropertyID::PROPTAG_END);
145: 
146:     DataSyncBarrier();
147: 
148:     uintptr bufferAddress = ARM_TO_GPU(reinterpret_cast<uintptr>(buffer));
149:     if (m_mailbox.WriteRead(bufferAddress) != bufferAddress)
150:     {
151:         return false;
152:     }
153: 
154:     DataMemBarrier();
155: 
156:     if (buffer->requestCode != RPI_MAILBOX_RESPONSE_SUCCESS)
157:     {
158:         return false;
159:     }
160: 
161:     memcpy(tags, buffer->tags, tagsSize);
162: 
163:     return true;
164: }
165: 
166: } // namespace baremetal
```

- Line 56-59: We implement the `RPIPropertiesInterface` constructor. This is quite straightforward
- Line 70-83: We implement the `GetTag()` method.
  - Line 72-73: We fill in the tag data, and return false if the sanity check fails
  - Line 75-80: We request the property, which will return true if successful. If this fails, we return false
  - Line 82: We check whether the tag result is as expected, and return true if ok, false if the check fails
- Line 90-99: We implement the private `CheckTagResult()` method
  - Line 92: We cast the tag pointer to a `Property` struct so we can evaluate it easily
  - Line 94-95: We check that bit 31 on the tag request / response field is set, using `RPI_MAILBOX_TAG_RESPONSE`. This flags successful handling of the tag
  - Line 97-98: We check that the size returned in the tag request / response field is not zero
- Line 108-119: We implement the private `FillTag()` method
  - Line 110-11: We do a sanity check that the tag pointer is not null, and the size is at least the minimum. If not, we return false
  - Line 113: We cast the tag pointer to a `Property` struct so we can fill it easily
  - Line 114: We set the tagID
  - Line 115: We set the tag buffer size (which is the size of the tag - the header size)
  - Line 116: We set the tag request code
- Line 128-164: We implement the private `GetTags()` method
  - Line 130-131: We do a sanity check that the tag pointer is not null, and the size is at least the minimum. If not, we return false
  - Line 133: We calculate the size of the mailbox buffer (in this case the size of the mailbox buffer header, the size of the property tag we wish to rqeuest, and the end tag size)
  - Line 134-135: We do a sanity check that the mailbox buffer size is a multiple of 4. If not, we return false
  - Line 137: We retrieve the coherent page for the mailbox, and cast it to a `MailboxBuffer` for easy manipulation
  - Line 139: We set the mailbox buffer size
  - Line 140: We set the mailbox request code
  - Line 141: We copy the contents of the tag we pass in to the mailbox buffer
  - Line 143: We calculate the address just after the tag, and cast it to a 32 bit unsigned int pointer
  - Line 144: We set the end tag ID
  - Line 146: We perform a sync using `DataSyncBarrier()` as before
  - Line 148: We convert the mailbox buffer address to VC address space as before
  - Line 149-152: We call `WriteRead()` on the mailbox, and check the return value. If not the same as the address we passed in, we return false
  - Line 154: We perform a sync using `DataMemBarrier()` as before
  - Line 156-159: We check that the requestCode is equal to success `0x80000000`, and return false if not
  - Line 161: We copy the tag contents back to the tag

As you can see, we use a function `memcpy()` here to copy data, which is a standard C function. However, we need to implement it.

### Util.h {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_UTILH}

Let's add the `memcpy()` function.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`.

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
...
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: void *memset(void *buffer, int value, size_t length);
52: void* memcpy(void* dest, const void* src, size_t length);
53: 
54: #ifdef __cplusplus
55: }
56: #endif
...
```

### Util.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_UTILCPP}

Now we can implement the `memcpy()` function.

Update the file `code/libraries/baremetal/src/Util.cpp`.

```cpp
File: code/libraries/baremetal/src/Util.cpp
...
62: 
63: /// <summary>
64: /// Standard C memcpy function. Copies memory pointed to by src to buffer pointed to by dest over length bytes
65: /// </summary>
66: /// <param name="dest">Destination buffer pointer</param>
67: /// <param name="src">Source buffer pointer</param>
68: /// <param name="length">Size of buffer to copy in bytes</param>
69: /// <returns>Pointer to destination buffer</returns>
70: void* memcpy(void* dest, const void* src, size_t length)
71: {
72:     uint8* dstPtr = reinterpret_cast<uint8*>(dest);
73:     const uint8* srcPtr = reinterpret_cast<const uint8*>(src);
74: 
75:     while (length-- > 0)
76:     {
77:         *dstPtr++ = *srcPtr++;
78:     }
79:     return dest;
80: }
...
```

### Update the application code {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_UPDATE_THE_APPLICATION_CODE}

Now we can update the application again, to make use of the new `RPIPropertiesInterface` class.
At the same time, we'll also clean up a bit, we'll remove the code printing the memory map as well.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Mailbox.h>
4: #include <baremetal/MemoryManager.h>
5: #include <baremetal/RPIPropertiesInterface.h>
6: #include <baremetal/SysConfig.h>
7: #include <baremetal/Serialization.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
10: #include <baremetal/UART1.h>
11: 
12: using namespace baremetal;
13: 
14: int main()
15: {
16:     auto& uart = GetUART1();
17:     uart.WriteString("Hello World!\n");
18: 
19:     char buffer[128];
20:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
21:     RPIPropertiesInterface properties(mailbox);
22: 
23:     struct PropertyTagSerial
24:     {
25:         PropertyTag tag;
26:         uint32   serial[2];
27:     } PACKED;
28: 
29:     PropertyTagSerial serialProperty;
30:     if (properties.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &serialProperty, sizeof(serialProperty)))
31:     {
32:         uart.WriteString("Mailbox call succeeded\n");
33:         uart.WriteString("Serial: ");
34:         Serialize(buffer, sizeof(buffer), serialProperty.serial[1], 8, 16, false, true);
35:         uart.WriteString(buffer);
36:         Serialize(buffer, sizeof(buffer), serialProperty.serial[0], 8, 16, false, true);
37:         uart.WriteString(buffer);
38:         uart.WriteString("\n");
39:     }
40:     else
41:     {
42:         uart.WriteString("Mailbox call failed\n");
43:     }
44: 
45:     uart.WriteString("Wait 5 seconds\n");
46:     Timer::WaitMilliSeconds(5000);
47: 
48:     uart.WriteString("Press r to reboot, h to halt\n");
49:     char ch{};
50:     while ((ch != 'r') && (ch != 'h'))
51:     {
52:         ch = uart.Read();
53:         uart.Write(ch);
54:     }
55: 
56:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
57: }
```

The code is quite a bit simpler now. We also removed the memory map printout.

- Line 20: We instantiate a `Mailbox`
- Line 21: We instantiate a `RPIPropertiesInterface` using the mailbox
- Line 23-27: We declare a structure `PropertyTagSerial` specifically for the board serial number property (note that the property header is always part of this)
- Line 29: We define a variable of the type `PropertyTagSerial`
- Line 30: We call the `GetTag()` method on the `RPIPropertiesInterface`, passing in the enum for the board serial number property, our `PropertySerial` variable pointer, and its size
- Line 32-38: If successful, we print the serial number
- Line 42: If not we print a failure message

### Update project configuration {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
41:     )
42: 
43: set(PROJECT_INCLUDES_PUBLIC
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
60:     )
61: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES_INTERFACE___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see a success message followed by the board serial number again as expected.

```text
Starting up
Hello World!
Mailbox call succeeded
Serial: 10000000D18E8B28
Wait 5 seconds
Press r to reboot, h to halt
```

## Adding the properties - Step 5 {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5}

A small improvement would be to hide the handling of the specific struct inside separate code, so our application code can become even simpler.
Let's introduce a class that can handle this for us. For now, we'll only add functionality for the board serial number, but we'll start adding more soon.

### RPIProperties.h {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_RPIPROPERTIESH}

Create the file `code/libraries/baremetal/include/baremetal/RPIProperties.h`

```cpp
File: code/libraries/baremetal/include/baremetal/RPIProperties.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : RPIProperties.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : RPIProperties
9: //
10: // Description : Access to BCM2835/2836/2837/2711/2712 properties using mailbox
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
43: #include <baremetal/IMailbox.h>
44: 
45: /// @file
46: /// Top level functionality handling for Raspberry Pi Mailbox
47: 
48: namespace baremetal {
49: 
50: /// <summary>
51: /// Top level functionality for requests on Mailbox interface
52: /// </summary>
53: class RPIProperties
54: {
55: private:
56:     /// @brief Reference to mailbox for functions requested
57:     IMailbox &m_mailbox;
58: 
59: public:
60:     explicit RPIProperties(IMailbox &mailbox);
61: 
62:     bool GetBoardSerial(uint64 &serial);
63: };
64: 
65: } // namespace baremetal
```

We declare the class `RPIProperties` which has a constructor taking a `Mailbox` again, and a method `GetBoardSerial()` which returns the board serial number (a unsigned 64 bit integer).

### RPIProperties.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_RPIPROPERTIESCPP}

We'll implement the class `RPIProperties`

Create the file `code/libraries/baremetal/src/RPIProperties.cpp`

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : RPIProperties.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : RPIProperties
9: //
10: // Description : Access to BCM2835/6/7 properties using mailbox
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
40: #include <baremetal/RPIProperties.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/RPIPropertiesInterface.h>
45: 
46: /// @file
47: /// Top level functionality handling for Raspberry Pi Mailbox implementation
48: 
49: namespace baremetal {
50: 
51: /// <summary>
52: /// Mailbox property tag structure for requesting board serial number.
53: /// </summary>
54: struct PropertyTagSerial
55: {
56:     /// Tag ID, must be equal to PROPTAG_GET_BOARD_REVISION.
57:     PropertyTag tag;
58:     /// The requested serial number/ This is a 64 bit unsigned number, divided up into two times a 32 bit number
59:     uint32   serial[2];
60: } PACKED;
61: 
62: /// <summary>
63: /// Constructor
64: /// </summary>
65: /// <param name="mailbox">Mailbox to be used for requests. Can be a fake for testing purposes</param>
66: RPIProperties::RPIProperties(IMailbox &mailbox)
67:     : m_mailbox{mailbox}
68: {
69: }
70: 
71: /// <summary>
72: /// Request board serial number
73: /// </summary>
74: /// <param name="serial">On return, set to serial number, if successful</param>
75: /// <returns>Return true on success, false on failure</returns>
76: bool RPIProperties::GetBoardSerial(uint64 &serial)
77: {
78:     PropertyTagSerial      tag{};
79:     RPIPropertiesInterface interface(m_mailbox);
80: 
81:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));
82: 
83:     if (result)
84:     {
85:         serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
86:     }
87: 
88:     return result;
89: }
90: 
91: } // namespace baremetal
```

- Line 54-60: We declare the same `PropertySerial` struct as we did before in the application code
- Line 66-69: We implement the `RPIProperties` constructor, which is again quite straightforward.
We store the `Mailbox` reference for use in the methods
- Line 76-89: We implement the method `GetBoardSerial()`
  - Line 78: We instantiate a `ProperySerial` struct
  - Line 79: We instantiate a RPIPropertiesInterface
  - Line 81: We call the `GetTag()` method on the `RPIPropertiesInterface`
  - Line 83-86: If the call was successful, we extract the serial number

### Update the application code {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_UPDATE_THE_APPLICATION_CODE}

Now we can update the application again, to it even simpler.
However, the serial number is 64 bits, and we don't have a serializer for it yet. Let's add it.

#### Serialization.h {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_UPDATE_THE_APPLICATION_CODE_SERIALIZATIONH}

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
...
49: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
50: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros);
...
```

#### Serialization.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_UPDATE_THE_APPLICATION_CODE_SERIALIZATIONCPP}

We need to implement the `Serialize` function for two different types. As the behaviour is very similar, we will create a third static function that can handle both 32 and 64 bit unsigned values.
There is one this a little tricky, as we need to calculate a divisor for 64 bit integers, the divisor can overflow. So we need to add a second variable to keep track of the overflowing.

Create the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
...
49: static bool Uppercase = true;
50: 
51: static void SerializeInternal(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
52: 
...
85: /// <summary>
86: /// Serialize a 32 bit unsigned value to buffer.
87: ///
88: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
89: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
90: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
91: ///
92: /// Base is the digit base, which can range from 2 to 36.
93: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16).
94: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
95: /// </summary>
96: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
97: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
98: /// <param name="value">Value to be serialized</param>
99: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
100: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
101: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
102: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
103: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
104: {
105:     SerializeInternal(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
106: }
107: 
108: /// <summary>
109: /// Serialize a 64 bit unsigned value to buffer.
110: ///
111: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
112: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
113: ///
114: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
115: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
116: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
117: ///
118: /// Base is the digit base, which can range from 2 to 36.
119: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16).
120: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
121: /// </summary>
122: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
123: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
124: /// <param name="value">Value to be serialized</param>
125: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
126: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
127: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
128: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
129: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
130: {
131:     SerializeInternal(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
132: }
133: 
134: /// <summary>
135: /// Internal serialization function, to be used for all unsigned values.
136: ///
137: /// Serialize a unsigned value to buffer.
138: ///
139: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
140: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
141: ///
142: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
143: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
144: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
145: ///
146: /// Base is the digit base, which can range from 2 to 36.
147: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
148: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
149: /// </summary>
150: /// <param name="value">Value to be serialized</param>
151: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
152: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
153: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
154: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
155: /// <param name="numBits">Specifies the number of bits used for the value</param>
156: static void SerializeInternal(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
157: {
158:     if ((base < 2) || (base > 36))
159:         return;
160: 
161:     int       numDigits = 0;
162:     uint64    divisor   = 1;
163:     uint64    divisorLast = 1;
164:     uint64    divisorHigh = 0;
165:     size_t    absWidth  = (width < 0) ? -width : width;
166:     const int maxDigits = BitsToDigits(numBits, base);
167:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
168:     {
169:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
170:         divisorLast = divisor;
171:         divisor *= base;
172:         ++numDigits;
173:     }
174:     divisor = divisorLast;
175: 
176:     size_t numChars = (numDigits > 0) ? numDigits : 1;
177:     if (showBase)
178:     {
179:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
180:     }
181:     if (absWidth > numChars)
182:         numChars = absWidth;
183:     if (numChars > bufferSize - 1) // Leave one character for \0
184:         return;
185: 
186:     char* bufferPtr = buffer;
187: 
188:     if (showBase)
189:     {
190:         if (base == 2)
191:         {
192:             *bufferPtr++ = '0';
193:             *bufferPtr++ = 'b';
194:         }
195:         else if (base == 8)
196:         {
197:             *bufferPtr++ = '0';
198:         }
199:         else if (base == 16)
200:         {
201:             *bufferPtr++ = '0';
202:             *bufferPtr++ = 'x';
203: 
204:         }
205:     }
206:     if (leadingZeros)
207:     {
208:         if (absWidth == 0)
209:             absWidth = maxDigits;
210:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
211:         {
212:             *bufferPtr++ = '0';
213:         }
214:     }
215:     while (numDigits > 0)
216:     {
217:         int digit = (value / divisor) % base;
218:         *bufferPtr++ = GetDigit(digit);
219:         --numDigits;
220:         divisor /= base;
221:     }
222:     *bufferPtr++ = '\0';
223: }
224: 
```

The implementation is similar to before, the main difference is keeping track of the divisor overflow:
- Line 103-106: We call the internal function `SerializeInternal()` while passing 32 as the last parameter to print a 32 bit value
- Line 129-132: We call the internal function `SerializeInternal()` while passing 64 as the last parameter to print a 64 bit value
- Line 156-223: We implement `SerializeInternal()`
    - Line 164: We add a new variable to take the high part (above 64 bits) of the divisor
    - Line 165: The number of bits is now passed in as a parameter
    - Line 167-173: We check whether the high part is not zero, and end the loop in that case.
We also calculate the new value for the high part in the loop

You may argue that it is a waste of resources to extend a 32 bit to 64 bits when printing, however this also save a lot of code, and thus memory footprint, next to simply having less code to maintain.

#### main.cpp {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_UPDATE_THE_APPLICATION_CODE_MAINCPP}

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Mailbox.h>
4: #include <baremetal/MemoryManager.h>
5: #include <baremetal/RPIProperties.h>
6: #include <baremetal/SysConfig.h>
7: #include <baremetal/Serialization.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
10: #include <baremetal/UART1.h>
11:
12: using namespace baremetal;
13:
14: int main()
15: {
16:     auto& uart = GetUART1();
17:     uart.WriteString("Hello World!\n");
18:
19:     char buffer[128];
20:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
21:     RPIProperties properties(mailbox);
22:
23:     uint64 serial{};
24:     if (properties.GetBoardSerial(serial))
25:     {
26:         uart.WriteString("Mailbox call succeeded\n");
27:         uart.WriteString("Serial: ");
28:         Serialize(buffer, sizeof(buffer), serial, 16, 16, false, true);
29:         uart.WriteString(buffer);
30:         uart.WriteString("\n");
31:     }
32:     else
33:     {
34:         uart.WriteString("Mailbox call failed\n");
35:     }
36:
37:     uart.WriteString("Wait 5 seconds\n");
38:     Timer::WaitMilliSeconds(5000);
39:
40:     uart.WriteString("Press r to reboot, h to halt\n");
41:     char ch{};
42:     while ((ch != 'r') && (ch != 'h'))
43:     {
44:         ch = uart.Read();
45:         uart.Write(ch);
46:     }
47:
48:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
49: }
```

Note we removed the inclusion of `RPIPropertiesInterface.h` as we no longer need it.
The code is again quite a bit simpler.

### Update project configuration {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
42:     )
43: 
44: set(PROJECT_INCLUDES_PUBLIC
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
62:     )
63: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_MAILBOX_ADDING_THE_PROPERTIES___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The output will be the same as before.

```text
Starting up
Hello World!
Mailbox call succeeded
Serial: 00000000C3D6D0CB
Wait 5 seconds
Press r to reboot, h to halt
```

Next: [11-uart0](11-uart0.md)

