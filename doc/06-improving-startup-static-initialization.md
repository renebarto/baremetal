# Improving startup and static initialization

Contents:
 - [Improving startup](##Improving-startup)
   - [System.h](###System.h)
   - [System.cpp](###System.cpp)
   - [Util.h](###Util.h)
   - [Util.cpp](###Util.cpp)
   - [Updating ARMInstructions.h](###Updating-ARMInstructions.h)
   - [Update startup code](###Update-startup-code)
   - [Update project configuration](###Update-project-configuration)
   - [Update application code](###Update-application-code)
   - [Configuring, building and debugging](###Configuring-building-and-debugging)
 - [Initializing static variables](##Initializing-static-variables)
   - [Call static initializers](###Call-static-initializers)
   - [Add retrieval function for System](###Add-retrieval-function-for-System)
   - [Implement GetSystem() and GetUART1()](###Implement-GetSystem-and-GetUART1)
   - [Add static initializer functions](###Add-static-initializer-functions)
   - [Update CMake file for baremetal](###Update-CMake-file-for-baremetal)
   - [Configure, build and debug](###Configure-build-and-debug)
 - [Implementing halt and reboot](##Implementing-halt-and-reboot)

## Improving startup

Until now, in the startup assembly code, we immediately called main().
However, very often, we will need to do some initial setup, such as initializing the console or screen, initializing the memory manager, and initializing static variables.
Also, we will need to create an entry point for the other cores to start.
To start with, we will simply set up a C function named sysinit, which will be called from assemly.
We will then start to populate this function, and later add more initialization.
Next to that we will add a class named System, that will hold methods to reboot or halt the system.
For now, the implementation will be quite simple, we will add the actual implementation later.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorials/06-improving-startup-static-initialization` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/06-improving-startup-static-initialization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

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
8: // Class       : System
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
46: class UART1;
47: 
48: class System
49: {
50: public:
51:     System();
52: 
53:     [[noreturn]] void Halt();
54:     [[noreturn]] void Reboot();
55: };
56: 
57: UART1& GetUART();
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
72:     int               main();
73:     [[noreturn]] void sysinit();
74: 
75: #ifdef __cplusplus
76: }
77: #endif
```

- Line 46: We forward declare the class UART1, as it is returned later as a result in Line 57.
- Line 48-55: We declare the class System, which has two methods, each of which will not return (hence the `[[noreturn]]` attribute):
  - Halt(), which halts the system
  - Reboot(), which restarts the system
- Line 57: We add a function to return a reference to the (singleton) UART1 instance. We can reuse this in our application.
- Line 61-65: We add an enum for the application return code, to enable forcing system halt or system restart.
- Line 72: We forward declare the main() function. Notice that it is wrapped inside `extern "C" { }`. This enforces the functions inside this construction to have C linkage.
- Line 73: We declare the sysinit() method, which will be called from startup assembly code, and will eventually call main()

### System.cpp

We add the implementation for `System.h`. Create a file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
46: #include <baremetal/Util.h>
47: 
48: using namespace baremetal;
49: 
50: UART1 s_uart;
51: System s_system;
52: 
53: System::System()
54: {
55: }
56: 
57: void System::Halt()
58: {
59:     s_uart.WriteString("Halt\n");
60: 
61:     for (;;) // Satisfy [[noreturn]]
62:     {
63:         DataSyncBarrier();
64:         WaitForInterrupt();
65:     }
66: }
67: 
68: void System::Reboot()
69: {
70:     s_uart.WriteString("Reboot\n");
71: 
72:     DisableIRQs();
73:     DisableFIQs();
74: 
75:     for (;;) // Satisfy [[noreturn]]
76:     {
77:         DataSyncBarrier();
78:         WaitForInterrupt();
79:     }
80: }
81: 
82: UART1& baremetal::GetUART()
83: {
84:     return s_uart;
85: }
86: 
87: #ifdef __cplusplus
88: extern "C"
89: {
90: #endif
91: 
92: void sysinit()
93: {
94:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
95:     EnableIRQs(); // go to TASK_LEVEL
96: 
97:     s_uart.Initialize();
98:     s_uart.WriteString("Starting up\n");
99: 
100:     // clear BSS
101:     extern unsigned char __bss_start;
102:     extern unsigned char __bss_end;
103:     // cppcheck-suppress comparePointers
104:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
105: 
106:     // halt, if KERNEL_MAX_SIZE is not properly set
107:     // cannot inform the user here
108:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
109:     {
110:         s_system.Halt();
111:     }
112: 
113:     extern int main();
114: 
115:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
116:     {
117:         s_system.Reboot();
118:     }
119: 
120:     s_system.Halt();
121: }
122: 
123: #ifdef __cplusplus
124: }
125: #endif
```

- Line 50: We declare the singleton class UART1 instantiation named `s_uart`.
Notice that we do not use the keyword static here, as we do not have the means to instantiate static variables yet.
- Line 51: We declare the singleton class System instantiation named `s_system`.
- Line 53-55: We implement the `System` constructor.
- Line 57-66: We implement `System::Halt()`. This simply prints and then ends up in an endless loop waiting for events.
- Line 68-80: We implement `System::Reboot()`. This simply prints and then ends up in an endless loop waiting for events.
- Line 82-85: We implement `GetUART()`. This returns the singleton UART1 instance.
- Line 87-90: We enter the `C` linkage region.
- Line 92-121: We implements `sysinit()`.
  - Line 94: We enable fast interrupts (FIQ).
  - Line 95: We enable normal interrupts (IRQ).
  - Line 97-98: We initialize the UART1 instance, and write the first line to the UART.
  - Line 101-102: We declare the variables for the bss section, they are defined in `Startup.S`
  - Line 104: We clear the bss section with a call to the standard C function memset (which still needs to be implemented).
  - Line 108-111: We perform a sanity check that our memory region contains the full bss section. If not, we halt the system.
  - Line 133-115: We forward declare `main()` and then call it.
  - Line 117: If `main()` returned `ReturnCode::ExitReboot`, we call `System::Reboot` on the singleton System object
  - Line 120: Otherwise we call `System::Halt` on the singleton System object
- Line 123-125: We exit the `C` linkage region again.

### Util.h

We will add a header to declare the protoype of the function memset().
This function is normally part of the standard C library, but as we are defining our own platform, we need to implement it.

Create a file `code/libraries/baremetal/include/baremetal/Util.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Util.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
44: #if defined(PLATFORM_BAREMETAL)
45: 
46: #ifdef __cplusplus
47: extern "C" {
48: #endif
49: 
50: void* memset(void* buffer, int value, size_t length);
51: 
52: #ifdef __cplusplus
53: }
54: #endif
55: 
56: #endif
```

This header simply declares the function memset (which is a standard C library function, but needs to be implemented specifically for the platform).
Notice that this function is again wrapped in `extern "C" { }` to enforce C linking.

### Util.cpp

We will implement the function memset(). For now we will use a simple C++ implementation, we will move to assembly later on.

Create a file `code/libraries/baremetal/src/Util.cpp`.

```cpp
File: code/libraries/baremetal/src/Util.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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

### Updating ARMInstructions.h

We will add some more assembly instructions which were used in `System.cpp`.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`, and add the following lines at the end:

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
49: 
50: /// @brief Data sync barrier
51: #define DataSyncBarrier()               asm volatile ("dsb sy" ::: "memory")
52: 
53: /// @brief Wait for interrupt
54: #define WaitForInterrupt()              asm volatile ("wfi")
55: 
56: /// @brief Enable IRQss. Clear bit 1 of DAIF register. See \ref DAIF_REGISTER
57: #define	EnableIRQs()                    asm volatile ("msr DAIFClr, #2")
58: /// @brief Disable IRQs. Set bit 1 of DAIF register. See \ref DAIF_REGISTER
59: #define	DisableIRQs()                   asm volatile ("msr DAIFSet, #2")
60: /// @brief Enable FIQs. Clear bit 0 of DAIF register. See \ref DAIF_REGISTER
61: #define	EnableFIQs()                    asm volatile ("msr DAIFClr, #1")
62: /// @brief Disable FIQs. Set bit 0 of DAIF register. See \ref DAIF_REGISTER
63: #define	DisableFIQs()                   asm volatile ("msr DAIFSet, #1")
64: 
```

- Line 51: This instruction enforces a synchronization with other cores concerning memory.
- Line 54: This instruction waits for interrupts on the current core.
- Line 57: This instruction enables interrupts
- Line 59: This instruction disables interrupts
- Line 61: This instruction enables fast interrupts
- Line 63: This instruction disables fast interrupts

### Update startup code

We need to update the startup code so that it calls sysinit() instead of main().

Update the file `code/libraries/baremetal/src/Startup.S`:

```asm
File: code/libraries/baremetal/src/Startup.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
18: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
47:     mrs	\xreg1, cnthctl_el2
48:     orr	\xreg1, \xreg1, #0x3            // Enable EL1 access to timers
49:     msr	cnthctl_el2, \xreg1
50:     msr	cntvoff_el2, xzr
51: 
52:     // Initilize MPID/MPIDR registers
53:     mrs	\xreg1, midr_el1
54:     mrs	\xreg2, mpidr_el1
55:     msr	vpidr_el2, \xreg1
56:     msr	vmpidr_el2, \xreg2
57: 
58:     // Disable coprocessor traps
59:     mov	\xreg1, #0x33ff
60:     msr	cptr_el2, \xreg1                // Disable coprocessor traps to EL2
61:     msr	hstr_el2, xzr                   // Disable coprocessor traps to EL2
62:     mov	\xreg1, #3 << 20
63:     msr	cpacr_el1, \xreg1               // Enable FP/SIMD at EL1
64: 
65:     // Initialize HCR_EL2
66:     mov	\xreg1, #(1 << 31)              // 64bit EL1
67:     msr	hcr_el2, \xreg1
68: 
69:     // SCTLR_EL1 initialization
70:     //
71:     // setting RES1 bits (29,28,23,22,20,11) to 1
72:     // and RES0 bits (31,30,27,21,17,13,10,6) +
73:     // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
74:     // CP15BEN,SA0,SA,C,A,M to 0
75:     mov	\xreg1, #0x0800
76:     movk \xreg1, #0x30d0, lsl #16
77:     msr	sctlr_el1, \xreg1
78: 
79:     // Return to the EL1_SP1 mode from EL2
80:     mov	\xreg1, #0x3c4
81:     msr	spsr_el2, \xreg1                // EL1_SP0 | D | A | I | F
82:     adr	\xreg1, label1
83:     msr	elr_el2, \xreg1
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

### Update project configuration

We need to add the newly created files to the project.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: project(baremetal
5: 	DESCRIPTION "Bare metal library"
6: 	LANGUAGES CXX ASM)
7: 
8: set(PROJECT_TARGET_NAME ${PROJECT_NAME})
9: 
10: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
11: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
12: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
13: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
14: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
15: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
16: set(PROJECT_INCLUDE_DIRS_PRIVATE )
17: set(PROJECT_INCLUDE_DIRS_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
18: 
19: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
20: 
21: set(PROJECT_DEPENDENCIES
22:     )
23: 
24: set(PROJECT_LIBS
25:     ${LINKER_LIBRARIES}
26:     ${PROJECT_DEPENDENCIES}
27:     )
28: 
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
48: 
49: if (CMAKE_VERBOSE_MAKEFILE)
50:     display_list("Package                           : " ${PROJECT_NAME} )
51:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
52:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
53:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
54:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
55:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
56:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
57:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
58:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
59:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
60:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
61:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
62:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
63:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
64:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
65:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
66:     display_list("Link libs                         : " ${PROJECT_LIBS} )
67:     display_list("Source files                      : " ${PROJECT_SOURCES} )
68:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
69:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
70: endif()
71: 
72: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
73: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
74: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
75: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
76: target_compile_definitions(${PROJECT_NAME} PRIVATE
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
80:     )
81: target_compile_definitions(${PROJECT_NAME} PUBLIC
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
85:     )
86: target_compile_options(${PROJECT_NAME} PRIVATE
87:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
88:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
89:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
90:     )
91: target_compile_options(${PROJECT_NAME} PUBLIC
92:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
93:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
94:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
95:     )
96: 
97: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
98: 
99: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
100: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
101:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
102: endif()
103: 
104: link_directories(${LINK_DIRECTORIES})
105: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
106: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
107: 
108: show_target_properties(${PROJECT_NAME})
109: 
```

### Update application code

We will no longer define the UART instance in the applciation, but use the one exported from System.h through the function `GetUART()`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/System.h"
3: #include "baremetal/UART1.h"
4: 
5: using namespace baremetal;
6: 
7: int main()
8: {
9:     GetUART().WriteString("Hello World!\n");
10:     return static_cast<int>(ReturnCode::ExitHalt);
11: }
12: 
```

### Configuring, building and debugging

The project will not behave much different from before, but will print more information to the console:

```text
(qemu:11432): Gtk-WARNING **: 17:13:50.297: Could not load a pixbuf from icon theme.
This may indicate that pixbuf loaders or the mime database could not be found.
Starting up
Hello World!
Halt
qemu: QEMU: Terminated via GDBstub
```

The first printed line is done in `sysinit()`, just after the UART is instantiated.
The second line is printed as before in `main()`.
The third line is printed as part of `System::Halt()`, as we return `static_cast<int>(ReturnCode::ExitHalt)` from `main()`.

## Initializing static variables

You may have noticed how we defined two variables in `System.cpp` that we could not make static, due to missing initialization code in `Startup.S`.
My preferred way of dealing with singletons is wrapping them into functions to retrieve their reference, so we can simply call this function,
 and it will be instantiated the first time the function is called.

### Call static initializers

Let's first call initialiation code to `System.cpp`.

Update the file `code/libraries/baremetal/src/System.cpp`:

```cpp
File: \code/libraries/baremetal/src/System.cpp
...

92: void sysinit()
93: {
94:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
95:     EnableIRQs(); // go to TASK_LEVEL
96: 
97:     s_uart.Initialize();
98:     s_uart.WriteString("Starting up\n");
99: 
100:     // clear BSS
101:     extern unsigned char __bss_start;
102:     extern unsigned char __bss_end;
103:     // cppcheck-suppress comparePointers
104:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
105: 
106:     // halt, if KERNEL_MAX_SIZE is not properly set
107:     // cannot inform the user here
108:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
109:     {
110:         s_system.Halt();
111:     }
112: 
113:     // Call constructors of static objects
114:     extern void (*__init_start)(void);
115:     extern void (*__init_end)(void);
116:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
117:     {
118:         (**func)();
119:     }
120: 
121:     extern int main();
122: 
123:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
124:     {
125:         s_system.Reboot();
126:     }
127: 
128:     s_system.Halt();
129: }

...
```

We've added line 113-119 here, which loop through the initialization array generated by the compiler. This is part of the `.init_array` section in `baremetal.ld', which also defines the start and end of the array:

```text
File: d:\Projects\baremetal.test\baremetal.ld
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

So we simple loop through this array, and call the function pointer to (which has signature `void()`).

### Add retrieval function for System

We will move `GetUART()` to UART1.h (renamed to `GetUART1()`, as it fits more logically there.
We will also add `GetSystem()` to System.h that will return our `System` instance:

#### GetUART1()

Update 'code/libraries\baremetal/include/baremetal/UART1.h':

```cpp
File: d:\Projects\baremetal.test\code\libraries\baremetal\include\baremetal\UART1.h
...

106: /// @brief Encapsulation for the UART1 device.
107: ///
108: class UART1
109: {
110:     friend UART1& GetUART();
111: 
112: private:
113:     bool            m_initialized;
114: 
115: public:
116:     /// @brief Constructs a default UART1 instance.
117:     UART1();
118:     /// @brief Initialize the UART1 device. Only performed once, guarded by m_initialized.
119:     ///
120:     ///  Set baud rate and characteristics (115200 8N1) and map to GPIO
121:     void Initialize();
122:     /// @brief Read a character
123:     /// @return Character read
124:     char Read();
125:     /// @brief Write a character
126:     /// @param c Character to be written
127:     void Write(char c);
128:     /// @brief Write a string
129:     /// @param str String to be written
130:     void WriteString(const char *str);
131: 
132: private:
133:     /// @param mode GPIO mode to be selected.
134:     /// @return true if successful, false otherwise
135:     bool SetMode(uint8 pinNumber, GPIOMode mode);
136:     /// @brief Set GPIO pin function
137:     /// @param function GPIO function to be selected.
138:     bool SetFunction(uint8 pinNumber, GPIOFunction function);
139:     /// @brief Set GPIO pin pull mode
140:     /// @param pullMode GPIO pull mode to be used.
141:     bool SetPullMode(uint8 pinNumber, GPIOPullMode pullMode);
142:     /// @brief Switch GPIO off
143:     bool Off(uint8 pinNumber, GPIOMode mode);
144: };
145: 
146: UART1& GetUART1();

...
```

#### GetSystem()

Update 'code/libraries\baremetal/include/baremetal/System.h':

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
File: d:\Projects\baremetal.test\code\libraries\baremetal\include\baremetal\System.h
...

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

...
```

- Line 48: We add `GetSystem()` as a friend function to the `System` class.
- Line 57: We add the declaration of `GetSystem()` which returns a reference to a `System` instance.

### Implement GetSystem() and GetUART1()

#### GetUART1()

We will now change the implementation of `GetUART1()` to use static variables:

Update 'code/libraries\baremetal/include/baremetal/UART1.cpp':

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...

243: UART1& GetUART1()
244: {
245:     static UART1 value;
246:     value.Initialize();
247: 
248:     return value;
249: }

...
```

- Line 243-249: We reimplement `GetUART()` to also have a local static variable.
We will call the `Initialize()` method on in, which will be called every time the function is called, however, due to the guard we built in, the actual initialization will only happen once.

#### GetSystem()

We will now implement `GetSystem()` and change the implementation of `GetUART()` to use static variables:

Update 'code/libraries\baremetal/include/baremetal/System.cpp':

```cpp
File: code/libraries\baremetal/include/baremetal/System.cpp
...

File: d:\Projects\baremetal.test\code\libraries\baremetal\src\System.cpp
File: d:\Projects\baremetal.test\code\libraries\baremetal\src\System.cpp
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
60: void System::Halt()
61: {
62:     GetUART1().WriteString("Halt\n");
63: 
64:     for (;;) // Satisfy [[noreturn]]
65:     {
66:         DataSyncBarrier();
67:         WaitForInterrupt();
68:     }
69: }
70: 
71: void System::Reboot()
72: {
73:     GetUART1().WriteString("Reboot\n");
74: 
75:     DisableIRQs();
76:     DisableFIQs();
77: 
78:     for (;;) // Satisfy [[noreturn]]
79:     {
80:         DataSyncBarrier();
81:         WaitForInterrupt();
82:     }
83: }
84: 
85: #ifdef __cplusplus
86: extern "C"
87: {
88: #endif
89: 
90: void sysinit()
91: {
92:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
93:     EnableIRQs(); // go to TASK_LEVEL
94: 
95:     // clear BSS
96:     extern unsigned char __bss_start;
97:     extern unsigned char __bss_end;
98:     // cppcheck-suppress comparePointers
99:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
100: 
101:     // halt, if KERNEL_MAX_SIZE is not properly set
102:     // cannot inform the user here
103:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
104:     {
105:         GetSystem().Halt();
106:     }
107: 
108:     // Call constructors of static objects
109:     extern void (*__init_start)(void);
110:     extern void (*__init_end)(void);
111:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
112:     {
113:         (**func)();
114:     }
115: 
116:     GetUART1().WriteString("Starting up\n");
117: 
118:     extern int main();
119: 
120:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
121:     {
122:         GetSystem().Reboot();
123:     }
124: 
125:     GetSystem().Halt();
126: }

...
```

- Line 50: We removed the variables `s_uart` and `s_system`.
- Line 50-54: We implement `GetSystem()`. Note how we add a local static variable to the function.
This will make sure there is only one instantation of this variable.
Also this will make sure the variable is instantiated only when the function is first called.
- Line 62: We will now call `GetUART()` to retrieve the UART instance.
- Line 73: Likewise.
- Line 95: We moved the call to print to the stage after the static initializers have run.
The static initializer calls may reset information on static objects.
- Line 105: We will now call `GetSystem()` to retrieve the System instance.
Here, we still use a static variable, however, this is only to halt the system.
- Line 116: We will now call `GetUART1()` to retrieve the UART instance.
We will no longer call `Initialize()`, and the getter function already does this.
- Line 122: We will now call `GetSystem()`.
- Line 125: Likewise.

### Add static initializer functions

If you would now build the code, you would get linker errors:

```text
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe:
 ../output/Debug/lib/libbaremetal.a(System.cpp.obj): in function `baremetal::GetSystem()':
  D:\Projects\baremetal.github\tutorial\06-improving-startup-static-initialization\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:52:(.text+0x38): undefined reference to `__cxa_guard_acquire'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\tutorial\06-improving-startup-static-initialization\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:52:(.text+0x68): undefined reference to `__cxa_guard_release'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/Debug/lib/libbaremetal.a(System.cpp.obj): in function `baremetal::GetUART()':
  D:\Projects\baremetal.github\tutorial\06-improving-startup-static-initialization\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:87:(.text+0x138): undefined reference to `__cxa_guard_acquire'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\tutorial\06-improving-startup-static-initialization\cmake-Baremetal-Debug/../code/libraries/baremetal/src/System.cpp:87:(.text+0x168): undefined reference to `__cxa_guard_release'
D:\Projects\baremetal.github\tutorial\06-improving-startup-static-initialization\cmake-BareMetal-Debug\collect2.exe : error : ld returned 1 exit status
```

This is due to the fact that the runtime expects to have two functions defined, that deal with static initialization:
- __cxa_guard_acquire
- __cxa_guard_release

These need to be implemented by us.

Create a file `code/libraries/baremetal/include/baremetal/CXAGuard.cpp':

```cpp
File: code/libraries/baremetal/src/CXAGuard.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
116: 
```

We define three functions here, of which one is not currently used.

- Line 94-104: We implement `__cxa_guard_acquire` which acquires an object for static initialization
  - We are passed in an object pointer, which points to a two byte structure.
The first byte states whether it has been initialized, the second byte states whether is currently being used for initialization.
  - We check whether the object is already initialized, if so we return.
  - We set the in-use flag
- Line 106-110: We implement `__cxa_guard_release` which releases an object for static initialization
  - We set the initialized flag
  - We clear the in-use flag
- Line 112-115" We implement `__cxa_guard_abort` which aborts the static initialization for an object
  - We clear the in-use flag

### Update CMake file for baremetal

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

### Configure, build and debug

The project will not behave different from before. However, it is interesting to see how static variables are initialized.
Try setting a breakpoint on the functions in `CXAGuard.cpp` and see when they get called.
Another interesting breakpoint is for example inside the `GetUART1()` function.

## Implementing halt and reboot

