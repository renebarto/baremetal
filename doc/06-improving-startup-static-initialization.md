# Improving startup and static initialization

Contents:
 - [Improving startup](##Improving-startup)
   - [System.h](###System.h)
   - [System.cpp](###System.cpp)
   - [Update project configuration](###Update-project-configuration)
   - [Update startup code](###Update-startup-code)
 - [Initializing static variables](##Initializing-static-variables)
 - [Implementing halt and reboot](##Implementing-halt-and-reboot)

## Improving startup

Until now, in the startup assembly code, we immediately called main().
However, very often, we will need to do some initial setup, such as initializing the console or screen, initializing the memory manager, and initializing static variables.
Also, we will need to create an entry point for the other cores to start.
To start with, we will simply set up a C function named sysinit, which will be called from assemly.
We will then start to populate this function, and later add more initialization.
Next to that we will add a class named System, that will hold methods to reboot or halt the system.

Let's start by creating a new source file and header.

### System.h

We first add the header file. Create a file `code/libraries/baremetal/include/baremetal/System.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : System.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Generic character read / write device interface
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
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: class System
47: {
48: public:
49:     System();
50: 
51:     [[noreturn]] void Halt();
52:     [[noreturn]] void Reboot();
53: };
54: 
55: } // namespace baremetal
56: 
57: enum class ReturnCode
58: {
59:     ExitHalt,
60:     ExitReboot,
61: };
62: 
63: #ifdef __cplusplus
64: extern "C"
65: {
66: #endif
67: 
68:     int               main();
69:     [[noreturn]] void sysinit();
70: 
71: #ifdef __cplusplus
72: }
73: #endif
74: 
```

- Line 46-53: We declare the class System, which has two methods, each of which will not return (hence the `[[noreturn]]` attribute):
  - Halt(), which halts the system
  - Reboot(), which restarts the system
- Line 57-61: We add an enum for the application return code, to enable forcing system halt or system restart.
- Line 68: We forward declare the main() function. Notice that it is wrapped inside `extern "C" { }`. This enforces the functions inside this construction to have C linkage.
- Line 69: We declare the sysinit() method, which will be called from startup assembly code, and will eventually call main()

### System.cpp

We add the implementation for `System.h`. Create a file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: d:\Projects\baremetal.test\code\libraries\baremetal\src\System.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : System.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : System initialization and process functions
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
40: #include <baremetal/System.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/SysConfig.h>
45: #include <baremetal/UART1.h>
46: 
47: using namespace baremetal;
48: 
49: UART1 s_uart;
50: System s_system;
51: 
52: System::System()
53: {
54: }
55: 
56: void System::Halt()
57: {
58:     for (;;) // Satisfy [[noreturn]]
59:     {
60:         DataSyncBarrier();
61:         WaitForInterrupt();
62:     }
63: }
64: 
65: void System::Reboot()
66: {
67:     s_uart.WriteString("Reboot\n");
68: 
69:     DisableIRQs();
70:     DisableFIQs();
71: 
72:     for (;;) // Satisfy [[noreturn]]
73:     {
74:         DataSyncBarrier();
75:         WaitForInterrupt();
76:     }
77: }
78: 
79: #ifdef __cplusplus
80: extern "C"
81: {
82: #endif
83: 
84: void sysinit()
85: {
86:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
87:     EnableIRQs(); // go to TASK_LEVEL
88: 
89:     s_uart.Initialize();
90:     s_uart.WriteString("Starting up\n");
91: 
92:     // clear BSS
93:     extern unsigned char __bss_start;
94:     extern unsigned char __bss_end;
95:     // cppcheck-suppress comparePointers
96:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
97: 
98:     // halt, if KERNEL_MAX_SIZE is not properly set
99:     // cannot inform the user here
100:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
101:     {
102:         s_system.Halt();
103:     }
104: 
105:     extern int main();
106: 
107:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
108:     {
109:         s_system.Reboot();
110:     }
111: 
112:     s_system.Halt();
113: }
114: 
115: #ifdef __cplusplus
116: }
117: #endif
```

### Util.h

We will add a header to declare the protoype of the function memset().
Create a file `code/libraries/baremetal/include/baremetal/Util.h`.

```cpp
```

### Util.cpp

We will implement the function memset().
Create a file `code/libraries/baremetal/src/Util.cpp`.

```cpp
```

### Updating ARMInstructions.h

We will add some more assembly instructions which were used in `System.cpp`.
Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`

```cpp
```

### Update project configuration

We need to add the newly created files to the project.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
```

### Update startup code


## Initializing static variables

## Implementing halt and reboot
