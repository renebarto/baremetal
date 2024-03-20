# Tutorial 06: Improving startup and static initialization {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION}

@tableofcontents

## New tutorial setup {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/06-improving-startup-static-initialization`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-06.a`
- an application `output/Debug/bin/06-improving-startup-static-initialization.elf`
- an image in `deploy/Debug/06-improving-startup-static-initialization-image`

## Improving startup - step 1 {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1}

Until now, in the startup assembly code, we immediately called main().
However, very often, we will need to do some initial setup, such as initializing the console or screen, initializing the memory manager, and initializing static variables.
Also, we will need to create an entry point for the other cores to start.
To start with, we will simply set up a C function named sysinit, which will be called from assembly. This is still only to start core 0.
We will then start to populate this function, and later add more initialization.
Next to that we will add a class named System, that will hold methods to reboot or halt the system.
For now, the implementation will be quite simple, we will add the actual implementation later.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorial/06-improving-startup-static-initialization` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/06-improving-startup-static-initialization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

### Update UART1.h {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UPDATE_UART1H}

First, we'll add a method to return a global UART1 instance that we will create in UART1.cpp.

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
106: // Encapsulation for the UART1 device.
107: class UART1
108: {
109:     friend UART1& GetUART1();
110:
111: private:
112:     bool            m_initialized;
113:
114: public:
115:     // Constructs a default UART1 instance.
116:     UART1();
117:     // Initialize the UART1 device. Only performed once, guarded by m_initialized.
118:     //
119:     //  Set baud rate and characteristics (115200 8N1) and map to GPIO
120:     void Initialize();
121:     // Read a character
122:     char Read();
123:     // Write a character
124:     void Write(char c);
125:     // Write a string
126:     void WriteString(const char *str);
127:
128: private:
129:     // Set GPIO pin mode
130:     bool SetMode(uint8 pinNumber, GPIOMode mode);
131:     // Set GPIO pin function
132:     bool SetFunction(uint8 pinNumber, GPIOFunction function);
133:     // Set GPIO pin pull mode
134:     bool SetPullMode(uint8 pinNumber, GPIOPullMode pullMode);
135:     // Switch GPIO off
136:     bool Off(uint8 pinNumber, GPIOMode mode);
137: };
138:
139: UART1& GetUART1();
140:
141: } // namespace baremetal
```

- line 109: We declare a friend function `GetUART1()` to the UART1 class.
- line 139: We declare the actual function `GetUART1()` to the UART1 class, which returns a reference to an instance of UART1.

### Update UART1.cpp {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UPDATE_UART1CPP}

We will implement the new function `GetUART1()`.

Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
244:
245: UART1 s_uart;
246:
247: UART1& GetUART1()
248: {
249:     s_uart.Initialize();
250:     return s_uart;
251: }
252:
253: } // namespace baremetal
```

We define a variable named `s_uart` here, and in the `GetUART1()` function, we first call `Initialize()` on it (which guards against multiple initialization), and then return the intstance.
We should have defined `s_uart` as static, but that does not work so far, as we cannot inititialize static variables yet. We'll get to that.

### System.h {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_SYSTEMH}

We'll add the System class.
We first add the header file.
Create a file `code/libraries/baremetal/include/baremetal/System.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : System.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : System
9: //
10: // Description : Generic character read / write device interface
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
42: #include <baremetal/Types.h>
43:
44: namespace baremetal {
45:
46: class System
47: {
48:     friend System& GetSystem();
49:
50: public:
51:     System();
52:
53:     [[noreturn]] void Halt();
54:     [[noreturn]] void Reboot();
55: };
56:
57: System& GetSystem();
58:
59: } // namespace baremetal
60:
61: enum class ReturnCode
62: {
63:     ExitHalt,
64:     ExitReboot,
65: };
66:
67: #ifdef __cplusplus
68: extern "C"
69: {
70: #endif
71:
72: int               main();
73: [[noreturn]] void sysinit();
74:
75: #ifdef __cplusplus
76: }
77: #endif
```

- Line 46-55: We declare the class System, which has two methods, each of which will not return (hence the `[[noreturn]]` attribute):
  - Halt(), which halts the system
  - Reboot(), which restarts the system
- Line 48: We declare a friend function `GetSystem()` in the System class.
- Line 57: We declare the actual function `GetSystem()` to return a reference to the (singleton) System instance.
- Line 61-65: We add an enum for the application return code, to enable forcing system halt or system restart.
- Line 72: We forward declare the main() function. Notice that it is wrapped inside `extern "C" { }`. This enforces the functions inside this construction to have C linkage.
- Line 73: We declare the sysinit() method, which will be called from startup assembly code, and will eventually call main()

### System.cpp {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_SYSTEMCPP}

We add the implementation for `System.h`. Create a file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : System.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : System
9: //
10: // Description : System initialization and process functions
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
40: #include <baremetal/System.h>
41:
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/SysConfig.h>
45: #include <baremetal/UART1.h>
46: #include <baremetal/Util.h>
47:
48: using namespace baremetal;
49:
50: System s_system;
51:
52: System& baremetal::GetSystem()
53: {
54:     return s_system;
55: }
56:
57: System::System()
58: {
59: }
60:
61: void System::Halt()
62: {
63:     GetUART1().WriteString("Halt\n");
64:
65:     for (;;) // Satisfy [[noreturn]]
66:     {
67:         DataSyncBarrier();
68:         WaitForInterrupt();
69:     }
70: }
71:
72: void System::Reboot()
73: {
74:     GetUART1().WriteString("Reboot\n");
75:
76:     DisableIRQs();
77:     DisableFIQs();
78:
79:     for (;;) // Satisfy [[noreturn]]
80:     {
81:         DataSyncBarrier();
82:         WaitForInterrupt();
83:     }
84: }
85:
86: #ifdef __cplusplus
87: extern "C"
88: {
89: #endif
90:
91: void sysinit()
92: {
93:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
94:     EnableIRQs(); // go to TASK_LEVEL
95:
96:     // clear BSS
97:     extern unsigned char __bss_start;
98:     extern unsigned char __bss_end;
100:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
101:
102:     // halt, if KERNEL_MAX_SIZE is not properly set
103:     // cannot inform the user here
104:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
105:     {
106:         GetSystem().Halt();
107:     }
108:
109:     GetUART1().WriteString("Starting up\n");
110:
111:     extern int main();
112:
113:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
114:     {
115:         GetSystem().Reboot();
116:     }
117:
118:     GetSystem().Halt();
119: }
120:
121: #ifdef __cplusplus
122: }
123: #endif
```

- Line 50-55: We define a variable named `s_system` here, and in the `GetSystem()` function and return the intstance.
We should have defined `s_system` as static, but that does not work so far, as we cannot inititialize static variables yet. We'll get to that.
- Line 57-59: We implement the `System` constructor.
- Line 61-70: We implement `System::Halt()`. This simply prints and then ends up in an endless loop waiting for events.
This also calls some functions (actually assembly instructions) that we will implement shortly.
- Line 72-84: We implement `System::Reboot()`. This simply prints and then ends up in an endless loop waiting for events.
This also calls some functions (actually assembly instructions) that we will implement shortly.
- Line 86-89: We enter the `C` linkage region.
- Line 91-119: We implements `sysinit()`.
  - Line 93: We enable fast interrupts (FIQ). Note that the function `EnableFIQs` still needs to be implemented.
  - Line 94: We enable normal interrupts (IRQ). Note that the function `EnableIRQs` still needs to be implemented.
  - Line 97-98: We declare the variables for the bss section, they are defined in `baremetal.ld`
  - Line 100: We clear the bss section with a call to the standard C function memset.
Note that the function `memset` still needs to be implemented.
  - Line 104-107: We perform a sanity check that our memory region contains the full bss section. If not, we halt the system.
  - Line 109: We write the first line to the UART.
  - Line 111: We forward declare `main()`.
  - Line 113: We call `main()`. If it returned `ReturnCode::ExitReboot`, we call `System::Reboot` on the singleton System object
  - Line 118: Otherwise we call `System::Halt` on the singleton System object
- Line 121-123: We exit the `C` linkage region again.

Notice that we include a new header in the top of the source:

```cpp
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/SysConfig.h>
45: #include <baremetal/UART1.h>
46: #include <baremetal/Util.h>
```

The header `Util.h` declares the `memset()` function used above and will be described below.

### Util.h {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UTILH}

We will add a header to declare the protoype of the function memset().
This function is normally part of the standard C library, but as we are defining our own platform, we need to implement it.

Create a file `code/libraries/baremetal/include/baremetal/Util.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Util.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Util.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Utility functions
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
42: #include <baremetal/Types.h>
43:
44: #ifdef __cplusplus
45: extern "C" {
46: #endif
47:
48: void* memset(void* buffer, int value, size_t length);
49:
50: #ifdef __cplusplus
51: }
52: #endif
```

This header simply declares the function memset (which is a standard C library function, but needs to be implemented specifically for the platform).
Notice that this function is again wrapped in `extern "C" { }` to enforce C linking.

### Util.cpp {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UTILCPP}

We will implement the function memset(). For now we will use a simple C++ implementation, we will move to assembly later on.

Create a file `code/libraries/baremetal/src/Util.cpp`.

```cpp
File: code/libraries/baremetal/src/Util.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Util.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Utility functions
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
40: #include <baremetal/Util.h>
41:
42: void* memset(void* buffer, int value, size_t length)
43: {
44:     uint8* ptr = reinterpret_cast<uint8*>(buffer);
45:
46:     while (length-- > 0)
47:     {
48:         *ptr++ = static_cast<char>(value);
49:     }
50:     return buffer;
51: }
```

### ARMInstructions.h {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_ARMINSTRUCTIONSH}

We will add some more assembly instructions which were used in `System.cpp`.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`, and add the following lines at the end:

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
49: // Data sync barrier
50: #define DataSyncBarrier()               asm volatile ("dsb sy" ::: "memory")
51:
52: // Wait for interrupt
53: #define WaitForInterrupt()              asm volatile ("wfi")
54:
55: // Enable IRQss. Clear bit 1 of DAIF register.
56: #define	EnableIRQs()                    asm volatile ("msr DAIFClr, #2")
57: // Disable IRQs. Set bit 1 of DAIF register.
58: #define	DisableIRQs()                   asm volatile ("msr DAIFSet, #2")
59: // Enable FIQs. Clear bit 0 of DAIF register.
60: #define	EnableFIQs()                    asm volatile ("msr DAIFClr, #1")
61: // Disable FIQs. Set bit 0 of DAIF register.
62: #define	DisableFIQs()                   asm volatile ("msr DAIFSet, #1")
```

- Line 50: This instruction enforces a synchronization with other cores concerning memory.
- Line 53: This instruction waits for interrupts on the current core.
- Line 56: This instruction enables interrupts
- Line 58: This instruction disables interrupts
- Line 60: This instruction enables fast interrupts
- Line 62: This instruction disables fast interrupts

### Update startup code {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UPDATE_STARTUP_CODE}

We need to update the startup code so that it calls sysinit() instead of main().

Update the file `code/libraries/baremetal/src/Startup.S`:

```asm
File: code/libraries/baremetal/src/Startup.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Startup.S
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Startup code. This is the entry point to any executable. It puts all cores except core 0 in sleep mode.
11: //               For core 0, it sets the stack pointer to just below the code (as the stack grows down), and then calls main().
12: //               Note: this file is based on the Circle startup assembly file by Rene Stange.
13: //
14: //------------------------------------------------------------------------------
15: //
16: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
17: //
18: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
19: //
20: // Permission is hereby granted, free of charge, to any person
21: // obtaining a copy of this software and associated documentation
22: // files(the "Software"), to deal in the Software without
23: // restriction, including without limitation the rights to use, copy,
24: // modify, merge, publish, distribute, sublicense, and /or sell copies
25: // of the Software, and to permit persons to whom the Software is
26: // furnished to do so, subject to the following conditions :
27: //
28: // The above copyright notice and this permission notice shall be
29: // included in all copies or substantial portions of the Software.
30: //
31: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
32: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
33: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
34: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
35: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
36: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
37: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
38: // DEALINGS IN THE SOFTWARE.
39: //
40: //------------------------------------------------------------------------------
41:
42: #include <baremetal/SysConfig.h>
43:
44: .macro armv8_switch_to_el1_m, xreg1, xreg2
45:
46:     // Initialize Generic Timers
47:     mrs \xreg1, cnthctl_el2
48:     orr \xreg1, \xreg1, #0x3            // Enable EL1 access to timers
49:     msr cnthctl_el2, \xreg1
50:     msr cntvoff_el2, xzr
51:
52:     // Initilize MPID/MPIDR registers
53:     mrs \xreg1, midr_el1
54:     mrs \xreg2, mpidr_el1
55:     msr vpidr_el2, \xreg1
56:     msr vmpidr_el2, \xreg2
57:
58:     // Disable coprocessor traps
59:     mov \xreg1, #0x33ff
60:     msr cptr_el2, \xreg1                // Disable coprocessor traps to EL2
61:     msr hstr_el2, xzr                   // Disable coprocessor traps to EL2
62:     mov \xreg1, #3 << 20
63:     msr cpacr_el1, \xreg1               // Enable FP/SIMD at EL1
64:
65:     // Initialize HCR_EL2
66:     mov \xreg1, #(1 << 31)              // 64bit EL1
67:     msr hcr_el2, \xreg1
68:
69:     // SCTLR_EL1 initialization
70:     //
71:     // setting RES1 bits (29,28,23,22,20,11) to 1
72:     // and RES0 bits (31,30,27,21,17,13,10,6) +
73:     // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
74:     // CP15BEN,SA0,SA,C,A,M to 0
75:     mov \xreg1, #0x0800
76:     movk \xreg1, #0x30d0, lsl #16
77:     msr sctlr_el1, \xreg1
78:
79:     // Return to the EL1_SP1 mode from EL2
80:     mov \xreg1, #0x3c4
81:     msr spsr_el2, \xreg1                // EL1_SP0 | D | A | I | F
82:     adr \xreg1, label1
83:     msr elr_el2, \xreg1
84:     eret
85: label1:
86: .endm
87:
88: .section .init
89:
90:     .globl _start
91: _start:                                 // normally entered from armstub8 in EL2 after boot
92:     mrs x0, CurrentEL                   // check if already in EL1t mode?
93:     cmp x0, #4
94:     beq EL1
95:
96:     ldr x0, =MEM_EXCEPTION_STACK        // IRQ, FIQ and exception handler run in EL1h
97:     msr sp_el1, x0                      // init their stack
98:
99:     armv8_switch_to_el1_m x0, x1
100:
101: EL1:
102:     ldr x0, =MEM_KERNEL_STACK           // main thread runs in EL1t and uses sp_el0
103:     mov sp, x0                          // init its stack
104:
105:     b sysinit                           // Jump to main()
106:
107: // End
```

The only difference is that we call `sysinit()` instead of `main()` when starting the system.

### Update project configuration {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UPDATE_PROJECT_CONFIGURATION}

We need to add the newly created files to the project.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...

29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
34:     )
35:
36: set(PROJECT_INCLUDES_PUBLIC
37:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
38:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
46:     )
47: set(PROJECT_INCLUDES_PRIVATE )

...
```

### Update application code {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_UPDATE_APPLICATION_CODE}

We will no longer define the UART instance in the application, but use the one exported from `UART1.h` through the function `GetUART1()`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/System.h>
3: #include <baremetal/UART1.h>
4:
5: using namespace baremetal;
6:
7: int main()
8: {
9:     GetUART1().WriteString("Hello World!\n");
10:     return static_cast<int>(ReturnCode::ExitHalt);
11: }
```

### Configure, build and debug {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP__STEP_1_CONFIGURE_BUILD_AND_DEBUG}

The project will not behave much different from before, but will print more information to the console:

```text
(qemu:7684): Gtk-WARNING **: 23:05:33.587: Could not load a pixbuf from icon theme.
This may indicate that pixbuf loaders or the mime database could not be found.
Starting up
Hello World!
Halt
qemu: QEMU: Terminated via GDBstub
```

The first printed line is done in `sysinit()`, the call to `GetUART1()` will initialize the UART.
The second line is printed as before in `main()`.
The third line is printed as part of `System::Halt()`, as we return `static_cast<int>(ReturnCode::ExitHalt)` from `main()`.

## Initializing static variables - Step 2 {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2}

You may have noticed how we defined two variables in `System.cpp` and `UART1.cpp` that we could not make static, due to missing initialization code.
My preferred way of dealing with a singleton is wrapping it into a function to retrieve their reference, so we can simply call this function,
and it will be instantiated the first time the function is called.

### Call static initializers {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_CALL_STATIC_INITIALIZERS}

Let's first call static initialization code to `System.cpp`.

Update the file `code/libraries/baremetal/src/System.cpp`:

```cpp
File: \code/libraries/baremetal/src/System.cpp
...

91: void sysinit()
92: {
93:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
94:     EnableIRQs(); // go to TASK_LEVEL
95:
96:     // clear BSS
97:     extern unsigned char __bss_start;
98:     extern unsigned char __bss_end;
100:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
101:
102:     // halt, if KERNEL_MAX_SIZE is not properly set
103:     // cannot inform the user here
104:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
105:     {
106:         GetSystem().Halt();
107:     }
108:
109:     // Call constructors of static objects
110:     extern void (*__init_start)(void);
111:     extern void (*__init_end)(void);
112:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
113:     {
114:         (**func)();
115:     }
116:
117:     GetUART1().WriteString("Starting up\n");
118:
119:     extern int main();
120:
121:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
122:     {
123:         GetSystem().Reboot();
124:     }
125:
126:     GetSystem().Halt();
127: }

...
```

We've added line 109-115 here, which loop through the initialization array generated by the compiler.
This is part of the `.init_array` section in `baremetal.ld', which also defines the start and end of the array:

```text
File: baremetal.ld
78:     /* Executable static initialization section */
79:     .init_array : {
80:         __init_start = .;
81:
82:         KEEP(*(.init_array*))
83:
84:         __init_end = .;
85:     }
86:
```

So we simple loop through this array, and call the function pointed to (which has signature `void()`).

### Update GetSystem() {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_UPDATE_GETSYSTEM}

We will now update the implementation `GetSystem()` to use a static local variable.
Update 'code/libraries\baremetal/include/baremetal/System.cpp':

```cpp
File: code/libraries\baremetal/include/baremetal/System.cpp
...

48: using namespace baremetal;
49:
50: System& baremetal::GetSystem()
51: {
52:     static System value;
53:     return value;
54: }
55:
56: System::System()
57: {
58: }
59:

...
```

- Line 50: We removed the variables `s_system`.
- Line 50-54: We update `GetSystem()`. Note how we add a local static variable to the function.
This will make sure there is only one instantiation of this variable.
Also this will make sure the variable is instantiated only when the function is first called.

### Update GetUART1() {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_UPDATE_GETUART1}

We will now update the implementation of `GetUART1()` to use static local variables:

Update 'code/libraries\baremetal/include/baremetal/UART1.cpp':

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...

245: UART1& GetUART1()
246: {
247:     static UART1 value;
248:     value.Initialize();
249:
250:     return value;
251: }
252:
253: } // namespace baremetal
```

- Line 245: We removed the variables `s_uart`.
- Line 245-251: We update `GetUART1()`. Note how we add a local static variable to the function.
This will make sure there is only one instantiation of this variable.
Also this will make sure the variable is instantiated only when the function is first called.
We will call the `Initialize()` method, which will be called every time the function is called, however, due to the guard we built in, the actual initialization will only happen once.

### Add static initializer functions {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_ADD_STATIC_INITIALIZER_FUNCTIONS}

If you would now build the code, you would get linker errors:

```text
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe:
 ../output/Debug/lib/libbaremetal.a(System.cpp.obj): in function `baremetal::GetSystem()':
  D:\Projects\baremetal.github\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:52:(.text+0x38): undefined reference to `__cxa_guard_acquire'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:52:(.text+0x68): undefined reference to `__cxa_guard_release'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/Debug/lib/libbaremetal.a(System.cpp.obj): in function `baremetal::GetUART()':
  D:\Projects\baremetal.github\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:87:(.text+0x138): undefined reference to `__cxa_guard_acquire'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:87:(.text+0x168): undefined reference to `__cxa_guard_release'
D:\Projects\baremetal.github\cmake-BareMetal-Debug\collect2.exe : error : ld returned 1 exit status
```

This is due to the fact that the runtime expects to have two functions defined, that deal with static initialization:
- __cxa_guard_acquire
- __cxa_guard_release

These need to be implemented by us.

Create a file `code/libraries/baremetal/include/baremetal/CXAGuard.cpp':

```cpp
File: code/libraries/baremetal/src/CXAGuard.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CXAGuard.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Guard functions, used to initialize static objects inside a function
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
40: // The semantic of the functions __cxa_guard_*() herein is based
41: // on the file cxa_guard.cxx of the LLVM Compiler Infrastructure,
42: // with this license:
43: //
44: // ==============================================================================
45: // LLVM Release License
46: // ==============================================================================
47: // University of Illinois/NCSA
48: // Open Source License
49: //
50: // Copyright (c) 2003-2010 University of Illinois at Urbana-Champaign.
51: // All rights reserved.
52: //
53: // Developed by:
54: //
55: //     LLVM Team
56: //
57: //     University of Illinois at Urbana-Champaign
58: //
59: //     http://llvm.org
60: //
61: // Permission is hereby granted, free of charge, to any person obtaining a copy of
62: // this software and associated documentation files (the "Software"), to deal with
63: // the Software without restriction, including without limitation the rights to
64: // use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
65: // of the Software, and to permit persons to whom the Software is furnished to do
66: // so, subject to the following conditions:
67: //
68: //     * Redistributions of source code must retain the above copyright notice,
69: //       this list of conditions and the following disclaimers.
70: //
71: //     * Redistributions in binary form must reproduce the above copyright notice,
72: //       this list of conditions and the following disclaimers in the
73: //       documentation and/or other materials provided with the distribution.
74: //
75: //     * Neither the names of the LLVM Team, University of Illinois at
76: //       Urbana-Champaign, nor the names of its contributors may be used to
77: //       endorse or promote products derived from this Software without specific
78: //       prior written permission.
79: //
80: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
81: // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
82: // FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
83: // CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
84: // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
85: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
86: // SOFTWARE.
87:
88: #include <baremetal/Types.h>
89:
90: // byte index into the guard object
91: #define INDEX_HAS_RUN		0
92: #define INDEX_IN_USE		1
93:
94: extern "C" int __cxa_guard_acquire(volatile uint8* guardObject)
95: {
96:     if (guardObject[INDEX_HAS_RUN] != 0)
97:     {
98:         return 0;                           // Do not run constructor
99:     }
100:
101:     guardObject[INDEX_IN_USE] = 1;
102:
103:     return 1;                               // Run constructor
104: }
105:
106: extern "C" void __cxa_guard_release(volatile uint8* guardObject)
107: {
108:     guardObject[INDEX_HAS_RUN] = 1;
109:     guardObject[INDEX_IN_USE] = 0;
110: }
111:
112: extern "C" void __cxa_guard_abort(volatile uint8* guardObject)
113: {
114:     guardObject[INDEX_IN_USE] = 0;
115: }
```

We define three functions here, of which one is not currently used.

- Line 94-104: We implement `__cxa_guard_acquire` which acquires an object for static initialization
  - We are passed in an object pointer, which points to a two byte structure.
    - The first byte states whether it has been initialized
    - The second byte states whether it is currently in use for initialization
  - We check whether the object is already initialized, if so we return.
  - We set the in-use flag
- Line 106-110: We implement `__cxa_guard_release` which releases an object for static initialization
  - We set the initialized flag
  - We clear the in-use flag
- Line 112-115: We implement `__cxa_guard_abort` which aborts the static initialization for an object
  - We clear the in-use flag

### Update project configuration {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_UPDATE_PROJECT_CONFIGURATION}

We need to add the newly added source file to the project:

Update file `code/libraries/baremetal/CMakeLists.txt':

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...

29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
35:     )

...
```

### Configure, build and debug {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES__STEP_2_CONFIGURE_BUILD_AND_DEBUG}

The project will not behave different from before. However, it is interesting to see how static variables are initialized.
Try setting a breakpoint on the functions in `CXAGuard.cpp` and see when they get called.
Another interesting breakpoint is for example inside the `GetUART1()` function.

## Implementing halt and reboot - Step 3 {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3}

We will again need to write to some registers to implement halt and reboot functionality.

### BCMRegisters.h {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_BCMREGISTERSH}

We need to update the header containing register addresses again.

Update the file code/libraries/baremetal/include/baremetal/BCMRegisters.h

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...

52: // End address for Raspberry PI BCM I/O
53: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)
54:
55: //---------------------------------------------
56: // Raspberry Pi Power Management
57: //---------------------------------------------
58:
59: #define RPI_PWRMGT_BASE                 RPI_BCM_IO_BASE + 0x00100000
60: #define RPI_PWRMGT_RSTC                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x0000001C)
61: #define RPI_PWRMGT_RSTS                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000020)
62: #define RPI_PWRMGT_WDOG                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000024)
63: #define RPI_PWRMGT_WDOG_MAGIC           0x5A000000
64: #define RPI_PWRMGT_RSTC_CLEAR           0xFFFFFFCF
65: #define RPI_PWRMGT_RSTC_REBOOT          0x00000020
66: #define RPI_PWRMGT_RSTC_RESET           0x00000102
67: #define RPI_PWRMGT_RSTS_PART_CLEAR      0xFFFFFAAA
68:
69: //---------------------------------------------
70: // Raspberry Pi GPIO
71: //---------------------------------------------

...
```

We will not go into the details of these registers, as they are not officially described as far as I know.
We will simply use proven code to implement halt and reboot.

### System.cpp {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_SYSTEMCPP}

#### Implement System::Halt() {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_SYSTEMCPP_IMPLEMENT_SYSTEMHALT}

Update the file code/libraries/baremetal/src/System.cpp

```cpp
File: code/libraries/baremetal/src/System.cpp
...

60: void System::Halt()
61: {
62:     GetUART1().WriteString("Halt\n");
63:
64:     // power off the SoC (GPU + CPU)
65:     auto r = *(RPI_PWRMGT_RSTS);
66:     r &= ~RPI_PWRMGT_RSTS_PART_CLEAR;
67:     r |= 0x555; // partition 63 used to indicate halt
68:     *(RPI_PWRMGT_RSTS) = (RPI_PWRMGT_WDOG_MAGIC | r);
69:     *(RPI_PWRMGT_WDOG) = (RPI_PWRMGT_WDOG_MAGIC | 1);
70:     *(RPI_PWRMGT_RSTC) = (RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
71:
72:     for (;;) // Satisfy [[noreturn]]
73:     {
74:         DataSyncBarrier();
75:         WaitForInterrupt();
76:     }
77: }

...
```

- Line 65: So we read the register `RPI_PWRMGT_RSTS` (probably means reset status)
- Line 66: We mask out bits 1, 3, 5, 7, 9, 11.
These 6 bits seem to indicate the partition we're booting from. This is normally 0, but all 1's is a special case.
- Line 67: We set the partition to boot from to 63, to enforce a halt.
- Line 68: We write the result to the `RPI_PWRMGT_RSTS` register, we apparently need to add a magic number to the value.
- Line 69: We write to the watchdog timer, presumably the value is the magic number with 10 added for 10 seconds.
- Line 70: We write a 1 to bit 5 in the reset control register, again with the magic number, in order to trigger a reset.

#### Implement System::System() {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_SYSTEMCPP_IMPLEMENT_SYSTEMSYSTEM}

Update the file code/libraries/baremetal/src/System.cpp

```cpp
File: code/libraries/baremetal/src/System.cpp
...

79: void System::Reboot()
80: {
81:     GetUART1().WriteString("Reboot\n");
82:
83:     DisableIRQs();
84:     DisableFIQs();
85:
86:     // power off the SoC (GPU + CPU)
87:     auto r = *(RPI_PWRMGT_RSTS);
88:     r &= ~RPI_PWRMGT_RSTS_PART_CLEAR;
89:     *(RPI_PWRMGT_RSTS) = (RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
90:     *(RPI_PWRMGT_WDOG) = (RPI_PWRMGT_WDOG_MAGIC | 1);
91:     *(RPI_PWRMGT_RSTC) = (RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
92:
93:     for (;;) // Satisfy [[noreturn]]
94:     {
95:         DataSyncBarrier();
96:         WaitForInterrupt();
97:     }
98: }

...
```

The functionality is almost equal, except that the boot partition is set to 0 instead of 63.

### Add option to reboot or halt {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_ADD_OPTION_TO_REBOOT_OR_HALT}

Now that we can actually reboot and halt the system, it would be nice to let the user choose.
We will read a character from the serial console, if it is `h` we will halt, if it is `r` we will reboot.

#### Update application code {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_ADD_OPTION_TO_REBOOT_OR_HALT_UPDATE_APPLICATION_CODE}

Update the file code/applications/demo/src/main.cpp

```cpp
File: code/applications/demo/src/main.cpp
...

7: int main()
8: {
9:     auto& uart = GetUART1();
10:     uart.WriteString("Hello World!\n");
11:
12:     uart.WriteString("Press r to reboot, h to halt\n");
13:     char ch{};
14:     while ((ch != 'r') && (ch != 'h'))
15:     {
16:         ch = uart.Read();
17:         uart.Write(ch);
18:     }
19:
20:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
21: }

...
```

- Line 9: Here we use a variable to retrieve the UART instance, note that we use `auto&` here, forgetting the reference forces a copy, which is not desired.
- Line 12: We simply tell the user to type a character
- Line 13-18: We read a character and loop until it is either `h` or `r`
- Line 20: We return the correct return code depending on the choice

### Update project configuration {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_UPDATE_PROJECT_CONFIGURATION}

As nothing was added or removed, we do not need to update the CMake files.

### Configure, build and debug {#TUTORIAL_06_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT__STEP_3_CONFIGURE_BUILD_AND_DEBUG}

Where building are running, you will notice that the application waits for a key to be pressed. If `r` is pressed the system will reboot, if `h` is pressed it will halt.

In QEMU this will manifest as a restart of the application (reboot) or exiting QEMU (halt).

```text
(qemu:22740): Gtk-WARNING **: 14:33:30.963: Could not load a pixbuf from icon theme.
This may indicate that pixbuf loaders or the mime database could not be found.
Starting up
Hello World!
Press r to reboot, h to halt
rReboot
Starting up
Hello World!
Press r to reboot, h to halt
hHalt
```

On a physical board, you will see two undesired effects:

```text
Hello World!
Press r to reboot, h to halt
�rR�Starting up
Hello World!
Press r to reboot, h to halt
�hH
```

First, you will see strange characters being written, second, you will see that the string `Reboot` or `Halt` is not fully displayed.
This has to do with the fact that we need to give the UART some time to do its work. We'll get around to that when we dive into timers.

Next: [07-generalization](07-generalization.md)
