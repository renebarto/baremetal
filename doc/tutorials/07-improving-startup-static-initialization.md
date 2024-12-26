# Tutorial 07: Improving startup and static initialization {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION}

@tableofcontents

## New tutorial setup {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/07-improving-startup-static-initialization`.
In the same way, the project names are adapted to make sure there are no conflicts.

If you're curious to see how this works, or just want to dive directly into the code,
here there is a complete copy of what we work towards in this section.
Its CMake root will clearly be `tutorial/07-improving-startup-static-initialization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

### Tutorial results {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-07.a`
- an application `output/Debug/bin/07-improving-startup-static-initialization.elf`
- an image in `deploy/Debug/07-improving-startup-static-initialization-image`

## Improving startup - step 1 {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1}

Until now, in the startup assembly code, we immediately called main().
However, very often, we will need to do some initial setup, such as initializing the console or screen, initializing the memory manager, and initializing static variables.
Also, we will need to create an entry point for the other cores to start.
To start with, we will simply set up a C function named sysinit, which will be called from assembly. This is still only to start core 0.
We will then start to populate this function, and later add more initialization.
Next to that we will add a class named System, that will hold methods to reboot or halt the system.
For now, the implementation will be quite simple, we will add the actual implementation later.

### Update UART1.h {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UPDATE_UART1H}

First, we'll add a method to return a global UART1 instance that we will create in UART1.cpp.

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
112: /// <summary>
113: /// Encapsulation for the UART1 device.
114: /// </summary>
115: class UART1
116: {
117:     /// <summary>
118:     /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
119:     /// </summary>
120:     /// <returns>Reference to the singleton UART1 instance</returns>
121:     friend UART1 &GetUART1();
122: 
123: private:
124:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
125:     bool m_isInitialized;
126: 
127: public:
128:     UART1();
129: 
130:     void Initialize();
131:     char Read();
132:     void Write(char c);
133:     void WriteString(const char *str);
134: 
135: private:
136:     // Set GPIO pin mode
137:     bool SetMode(uint8 pinNumber, GPIOMode mode);
138:     // Set GPIO pin function
139:     bool SetFunction(uint8 pinNumber, GPIOFunction function);
140:     // Set GPIO pin pull mode
141:     bool SetPullMode(uint8 pinNumber, GPIOPullMode pullMode);
142:     // Switch GPIO off
143:     bool Off(uint8 pinNumber, GPIOMode mode);
144: };
145: 
146: UART1 &GetUART1();
147: 
148: } // namespace baremetal
```

- Line 121: We declare a friend function `GetUART1()` to the UART1 class.
- Line 146: We declare the actual function `GetUART1()`, which returns a reference to an instance of UART1.

### Update UART1.cpp {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UPDATE_UART1CPP}

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

We define a variable named `s_uart` here, and in the `GetUART1()` function, we first call `Initialize()` on it (which guards against multiple initialization), and then return the instance.
We should have defined `s_uart` as static, but that does not work so far, as we cannot inititialize static variables yet. We'll get to that.

### System.h {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_SYSTEMH}

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
42: #include <baremetal/Types.h>
43: 
44: /// @file
45: /// System startup / shutdown functionality
46: 
47: baremetal {
48: 
49: /// <summary>
50: /// System startup / shutdown handling class
51: /// </summary>
52: class System
53: {
54:     /// <summary>
55:     /// Construct the singleton System instance if needed, and return a reference to the instance. This is a friend function of class System
56:     /// </summary>
57:     /// <returns>Reference to the singleton system instance</returns>
58:     friend System &GetSystem();
59: 
60: 
61: public:
62:     System();
63: 
64:     [[noreturn]] void Halt();
65:     [[noreturn]] void Reboot();
66: };
67: 
68: System &GetSystem();
69: 
70: } // namespace baremetal
71: 
72: /// <summary>
73: /// Return code for main() function
74: /// </summary>
75: enum class ReturnCode
76: {
77:     /// @brief If main() returns this, the system will be halted
78:     ExitHalt,
79:     /// @brief If main() returns this, the system will be rebooted
80:     ExitReboot,
81: };
82: 
83: #ifdef __cplusplus
84: extern "C"
85: {
86: #endif
87: 
88: /// <summary>
89: /// Forward declared main() function
90: /// </summary>
91: /// <returns>Integer cast of ReturnCode</returns>
92: int main();
93: /// <summary>
94: /// System initialization function. This is the entry point of the C / C++ code for the system for Core 0
95: /// </summary>
96: [[noreturn]] void sysinit();
97: 
98: #ifdef __cplusplus
99: }
100: #endif
```

- Line 52-66: We declare the class System, which has two methods, each of which will not return (hence the `[[noreturn]]` attribute):
  - Halt(), which halts the system
  - Reboot(), which restarts the system
- Line 58: We declare a friend function `GetSystem()` in the System class
- Line 68: We declare the actual function `GetSystem()` to return a reference to the (singleton) System instance
- Line 75-81: We add an enum for the application return code, to enable forcing system halt or system restart
- Line 83-86: We enter the `C` linkage region, which mean the function declared next will have C names when linking, instead of the normal C++ mangled names
- Line 92: We forward declare the main() function. Notice that it is wrapped inside `extern "C" { }`. This enforces the functions inside this construction to have C linkage
- Line 96: We declare the sysinit() method, which will be called from startup assembly code, and will eventually call main()
- Line 98-100: We leave the `C` linkage region again

### System.cpp {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_SYSTEMCPP}

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
40: #include <baremetal/System.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/SysConfig.h>
46: #include <baremetal/UART1.h>
47: #include <baremetal/Util.h>
48: 
49: /// @file
50: /// System startup / shutdown functionality implementation
51: 
52: using namespace baremetal;
53: 
54: System s_system;
55: 
56: /// <summary>
57: /// Construct the singleton system handler if needed, and return a reference to the instance
58: /// </summary>
59: /// <returns>Reference to the singleton system handler</returns>
60: System &baremetal::GetSystem()
61: {
62:     return s_system;
63: }
64: 
65: /// <summary>
66: /// Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
67: /// </summary>
68: System::System()
69: {
70: }
71: 
72: /// <summary>
73: /// Halts the system. This function will not return
74: /// </summary>
75: void System::Halt()
76: {
77:     GetUART1().WriteString("Halt\n");
78: 
79:     for (;;) // Satisfy [[noreturn]]
80:     {
81:         DataSyncBarrier();
82:         WaitForInterrupt();
83:     }
84: }
85: 
86: /// <summary>
87: /// Reboots the system. This function will not return
88: /// </summary>
89: void System::Reboot()
90: {
91:     GetUART1().WriteString("Reboot\n");
92: 
93:     DisableIRQs();
94:     DisableFIQs();
95: 
96:     for (;;) // Satisfy [[noreturn]]
97:     {
98:         DataSyncBarrier();
99:         WaitForInterrupt();
100:     }
101: }
102: 
103: #ifdef __cplusplus
104: extern "C"
105: {
106: #endif
107: 
108: void sysinit()
109: {
110:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
111:     EnableIRQs(); // go to TASK_LEVEL
112: 
113:     // clear BSS
114:     extern unsigned char __bss_start;
115:     extern unsigned char __bss_end;
116:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
117: 
118:     // halt, if KERNEL_MAX_SIZE is not properly set
119:     // cannot inform the user here
120:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
121:     {
122:         GetSystem().Halt();
123:     }
124: 
125:     GetUART1().WriteString("Starting up\n");
126: 
127:     extern int main();
128: 
129:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
130:     {
131:         GetSystem().Reboot();
132:     }
133: 
134:     GetSystem().Halt();
135: }
136: 
137: #ifdef __cplusplus
138: }
139: #endif
140: 
```

- Line 54: We define a variable named `s_system` here
- Line 60-63: We define the `GetSystem()` function to return the intstance `s_system`.
We should have defined `s_system` as static, but that does not work so far, as we cannot inititialize static variables yet. We'll get to that.
- Line 68-70: We implement the `System` constructor.
- Line 75-84: We implement `System::Halt()`. This simply prints and then ends up in an endless loop waiting for events.
This also calls some functions (actually assembly instructions) that we will implement shortly.
- Line 89-101: We implement `System::Reboot()`. This simply prints and then ends up in an endless loop waiting for events.
As part of the execution we disable fast interrupts (FIQ) and normal interrupts (IRQ).
Note that the functions `DisableFIQs` and `DisableIRQs` still need to be implemented.
This also calls some other assembly functions that we will implement shortly.
- Line 103-106: We enter the `C` linkage region.
- Line 108-135: We implement `sysinit()`.
  - Line 110: We enable fast interrupts (FIQ). Note that the function `EnableFIQs` still needs to be implemented.
  - Line 111: We enable normal interrupts (IRQ). Note that the function `EnableIRQs` still needs to be implemented.
  - Line 114-115: We declare the variables for the bss section, they are defined in `baremetal.ld`
  - Line 116: We clear the bss section with a call to the standard C function memset.
Note that the function `memset` (normally a standard function) still needs to be implemented.
Note also that we no longer use the __bss_size variable. We can remove this, as well as the clearing code in the startup assembly.
  - Line 120-123: We perform a sanity check that our memory region contains the full bss section. If not, we halt the system.
  - Line 125: We write the first line to the UART.
  - Line 127: We forward declare `main()`.
  - Line 129-132: We call `main()`. If it returned `ReturnCode::ExitReboot`, we call `System::Reboot` on the singleton System object
  - Line 134: Otherwise we call `System::Halt` on the singleton System object
- Line 137-139: We exit the `C` linkage region again.

Notice that we include a new header in the top of the source:

```cpp
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/SysConfig.h>
45: #include <baremetal/UART1.h>
46: #include <baremetal/Util.h>
```

The header `Util.h` declares the `memset()` function used above and will be declared and implemented below.

### Util.h {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UTILH}

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
42: #include <baremetal/Types.h>
43: 
44: /// @file
45: /// Standard C library utility functions
46: 
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: void *memset(void *buffer, int value, size_t length);
52: 
53: #ifdef __cplusplus
54: }
55: #endif
```

This header simply declares the function memset (which is a standard C library function, but needs to be implemented specifically for the platform).
Notice that this function is again wrapped in `extern "C" { }` to enforce C linkage.

### Util.cpp {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UTILCPP}

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
40: #include <baremetal/Util.h>
41: 
42: /// @file
43: /// Standard C library utility functions implementation
44: 
45: /// <summary>
46: /// Standard C memset function. Fills memory pointed to by buffer with value bytes over length bytes
47: /// </summary>
48: /// <param name="buffer">Buffer pointer</param>
49: /// <param name="value">Value used for filling the buffer (only lower byte is used)</param>
50: /// <param name="length">Size of the buffer to fill in bytes</param>
51: /// <returns>Pointer to buffer</returns>
52: void *memset(void *buffer, int value, size_t length)
53: {
54:     uint8 *ptr = reinterpret_cast<uint8 *>(buffer);
55: 
56:     while (length-- > 0)
57:     {
58:         *ptr++ = static_cast<char>(value);
59:     }
60:     return buffer;
61: }
```

This function should speak for itself. It sets the buffer starting at `buffer` for `length` bytes to `value` for each byte.
Then it returns the pointer to the buffer.

### ARMInstructions.h {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_ARMINSTRUCTIONSH}

We will add some more assembly instructions which were used in `System.cpp`.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`, and add the following lines at the end:

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
50: /// @brief Data sync barrier
51: #define DataSyncBarrier()               asm volatile ("dsb sy" ::: "memory")
52: 
53: /// @brief Wait for interrupt
54: #define WaitForInterrupt()              asm volatile ("wfi")
55: 
56: /// @brief Enable IRQs. Clear bit 1 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
57: #define	EnableIRQs()                    asm volatile ("msr DAIFClr, #2")
58: /// @brief Disable IRQs. Set bit 1 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
59: #define	DisableIRQs()                   asm volatile ("msr DAIFSet, #2")
60: /// @brief Enable FIQs. Clear bit 0 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
61: #define	EnableFIQs()                    asm volatile ("msr DAIFClr, #1")
62: /// @brief Disable FIQs. Set bit 0 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
63: #define	DisableFIQs()                   asm volatile ("msr DAIFSet, #1")
```

- Line 51: This instruction enforces a synchronization with other cores concerning memory.
- Line 54: This instruction waits for interrupts on the current core.
- Line 57: This instruction enables interrupts
- Line 59: This instruction disables interrupts
- Line 61: This instruction enables fast interrupts
- Line 63: This instruction disables fast interrupts

The latter four functions make use of the DAIF register. This register is used to set interrupt flags:
- D: Process state D mask
- A: SError interrupt mask bit
- I: IRQ mask bit.
- F: FIQ mask bit.

These bits are manipulated by writing to the DAIFSet or DAIFClr registers.

For more information see [DAIF register](#ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER).

### Update startup code {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UPDATE_STARTUP_CODE}

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
18: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

We clean up a bit, removing things that will be handled in C / C++ code from now on:
- We don't check for Core 0 any longer. Cores 1 to 3 will not be running anyhow unless we start them, and the entry address will have point to a different entry point.
- We no longer cleas the .bss segment. That is now done in `sysinit()`
- We call jump to `sysinit()` instead of calling `main()` when starting the system
- We no longer start the waitevent loop, as this will be handled in the System methods.

### Update project configuration {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UPDATE_PROJECT_CONFIGURATION}

We need to add the newly created files to the project.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
35:     )
36: 
37: set(PROJECT_INCLUDES_PUBLIC
38:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
48:     )
49: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_UPDATE_APPLICATION_CODE}

We will no longer define the UART instance in the application, but use the one exported from `UART1.h` through the function `GetUART1()`.

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/System.h>
3: #include <baremetal/UART1.h>
4: 
5: /// @file
6: /// Demo application main code
7: 
8: using namespace baremetal;
9: 
10: /// <summary>
11: /// Demo application main code
12: /// </summary>
13: /// <returns>For now always 0</returns>
14: int main()
15: {
16:     GetUART1().WriteString("Hello World!\n");
17:     for (int i = 0; i < 1000000; ++i)
18:         NOP();
19:     return static_cast<int>(ReturnCode::ExitHalt);
20: }
```

### Configure, build and debug {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPROVING_STARTUP___STEP_1_CONFIGURE_BUILD_AND_DEBUG}

The project will not behave much different from before, but will print more information to the console:

```text
Starting up
Hello World!
Halt
```

The first printed line is done in `sysinit()`, the call to `GetUART1()` will initialize the UART.
The second line is printed as before in `main()`.
The third line is printed as part of `System::Halt()`, as we return `static_cast<int>(ReturnCode::ExitHalt)` from `main()`.

## Initializing static variables - Step 2 {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2}

You may have noticed how we defined two variables in `System.cpp` and `UART1.cpp` that we could not make static, due to missing initialization code.
My preferred way of dealing with a singleton is wrapping it into a function to retrieve their reference, so we can simply call this function,
and it will be instantiated the first time the function is called. There are some caveats here, which will discuss when this becomes relevant.

### Call static initializers {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_CALL_STATIC_INITIALIZERS}

Let's first call static initialization code to `System.cpp`.

Update the file `code/libraries/baremetal/src/System.cpp`:

```cpp
File: \code/libraries/baremetal/src/System.cpp
...
109: void sysinit()
110: {
111:     EnableFIQs(); // go to IRQ_LEVEL, EnterCritical() will not work otherwise
112:     EnableIRQs(); // go to TASK_LEVEL
113: 
114:     // clear BSS
115:     extern unsigned char __bss_start;
116:     extern unsigned char __bss_end;
117:     memset(&__bss_start, 0, &__bss_end - &__bss_start);
118: 
119:     // halt, if KERNEL_MAX_SIZE is not properly set
120:     // cannot inform the user here
121:     if (MEM_KERNEL_END < reinterpret_cast<uintptr>(&__bss_end))
122:     {
123:         GetSystem().Halt();
124:     }
125: 
126:     // Call constructors of static objects
127:     extern void (*__init_start)(void);
128:     extern void (*__init_end)(void);
129:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
130:     {
131:         (**func)();
132:     }
133: 
134:     GetUART1().WriteString("Starting up\n");
135: 
136:     extern int main();
137: 
138:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
139:     {
140:         GetSystem().Reboot();
141:     }
142: 
143:     GetSystem().Halt();
144: }
...
```

We've added line 126-132 here, which loop through the initialization array generated by the compiler.
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

Behind the scenes however, the compiler collects all static initializer functions, and places the pointers to these in the `.init_array`.
Similarly, although not used here, there is also a `.fini_array` for destruction of static variables.

### Update GetSystem() {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_UPDATE_GETSYSTEM}

We will now update the implementation `GetSystem()` to use a static local variable.
Update 'code/libraries\baremetal/include/baremetal/System.cpp':

```cpp
File: code/libraries\baremetal/include/baremetal/System.cpp
...
52: using namespace baremetal;
53: 
54: /// <summary>
55: /// Construct the singleton system handler if needed, and return a reference to the instance
56: /// </summary>
57: /// <returns>Reference to the singleton system handler</returns>
58: System& baremetal::GetSystem()
59: {
60:     static System value;
61:     return value;
62: }
63: 
64: /// <summary>
65: /// Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
66: /// </summary>
67: System::System()
68: {
69: }
...
```

- Line 55: We removed the variables `s_system`.
- Line 58-62: We update `GetSystem()`. Note how we add a local static variable to the function.
This will make sure there is only one instantiation of this variable.
Also this will make sure the variable is instantiated only when the function is first called.

### Update GetUART1() {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_UPDATE_GETUART1}

We will now update the implementation of `GetUART1()` to use static local variables:

Update 'code/libraries\baremetal/include/baremetal/UART1.cpp':

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
267: /// <summary>
268: /// Construct the singleton UART1 device if needed, and return a reference to the instance
269: /// </summary>
270: /// <returns>Reference to the singleton UART1 device</returns>
271: UART1& GetUART1()
272: {
273:     static UART1 value;
274:     value.Initialize();
275: 
276:     return value;
277: }
278: 
279: } // namespace baremetal
```

- Line 268: We removed the variables `s_uart`.
- Line 271-277: We update `GetUART1()`. Note how we add a local static variable to the function.
This will make sure there is only one instantiation of this variable.
Also this will make sure the variable is instantiated only when the function is first called.
We will call the `Initialize()` method, which will be called every time the function is called, however, due to the guard we built in, the actual initialization will only happen once.

### Add static initializer functions {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_ADD_STATIC_INITIALIZER_FUNCTIONS}

If you would now build the code, you would get linker errors:

```text
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/RPI3/Debug/lib/libbaremetal.a(System.cpp.obj): in function `baremetal::GetSystem()':
  D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/System.cpp:60:(.text+0x38): undefined reference to `__cxa_guard_acquire'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/System.cpp:60:(.text+0x68): undefined reference to `__cxa_guard_release'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/RPI3/Debug/lib/libbaremetal.a(UART1.cpp.obj): in function `baremetal::GetUART1()':
  D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/UART1.cpp:273:(.text+0x730): undefined reference to `__cxa_guard_acquire'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/UART1.cpp:273:(.text+0x760): undefined reference to `__cxa_guard_release'
D:\Projects\baremetal\cmake-BareMetal-RPI3-Debug\collect2.exe : error : ld returned 1 exit status
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
90: /// @file
91: /// Static variable acquire / release functionality
92: 
93: // Byte index into the guard object
94: /// @brief Index to byte signalling that the static object was acquired and released
95: #define INDEX_HAS_RUN		0
96: /// @brief Index to byte signalling that the static object was acquired but not released yes
97: #define INDEX_IN_USE		1
98: 
99: /// <summary>
100: /// Acquire guard for static initialization
101: /// </summary>
102: /// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
103: /// <returns>Returns 1 if acquire is successful, 0 if already done</returns>
104: extern "C" int __cxa_guard_acquire(volatile uint8 * guardObject)
105: {
106:     // Did we already initialize this object?
107:     if (guardObject[INDEX_HAS_RUN] != 0)
108:     {
109:         return 0;                           // Do not run constructor
110:     }
111: 
112:     // Lock this guard while acquired
113:     guardObject[INDEX_IN_USE] = 1;
114: 
115:     return 1;                               // Run constructor
116: }
117: 
118: /// <summary>
119: /// Release the acquired guard
120: /// </summary>
121: /// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
122: extern "C" void __cxa_guard_release(volatile uint8 * guardObject)
123: {
124:     // Set acquire / release cycle complete
125:     guardObject[INDEX_HAS_RUN] = 1;
126:     guardObject[INDEX_IN_USE] = 0;
127: }
128: 
129: /// <summary>
130: /// Abort the static object initialization, release the acquired object
131: /// </summary>
132: /// <param name="guardObject">Guard object pointer to two bytes for a static object</param>
133: extern "C" void __cxa_guard_abort(volatile uint8 * guardObject)
134: {
135:     guardObject[INDEX_IN_USE] = 0;
136: }
```

We define three functions here, of which one is not currently used.

- Line 104-110: We implement `__cxa_guard_acquire` which acquires an object for static initialization
  - We are passed in an object pointer, which points to a two byte structure.
    - The first byte states whether it has been initialized
    - The second byte states whether it is currently in use for initialization
  - We check whether the object is already initialized, if so we return 0 to indicate initialization is not needed.
  - We set the in-use flag and return 1 to indicate initialization is needed
- Line 122-127: We implement `__cxa_guard_release` which releases an object for static initialization
  - We set the initialized flag
  - We clear the in-use flag
- Line 133-136: We implement `__cxa_guard_abort` which aborts the static initialization for an object
  - We clear the in-use flag

### Update project configuration {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_UPDATE_PROJECT_CONFIGURATION}

We need to add the newly added source file to the project:

Update file `code/libraries/baremetal/CMakeLists.txt':

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
36:     )
37: 
38: set(PROJECT_INCLUDES_PUBLIC
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
49:     )
50: set(PROJECT_INCLUDES_PRIVATE )
51: 
...
```

### Configure, build and debug {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_INITIALIZING_STATIC_VARIABLES___STEP_2_CONFIGURE_BUILD_AND_DEBUG}

The project will not behave different from before. However, it is interesting to see how static variables are initialized.
Try setting a breakpoint on the functions in `CXAGuard.cpp` and see when they get called.
Another interesting breakpoint is for example inside the `GetUART1()` function.

## Implementing halt and reboot - Step 3 {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3}

We will again need to write to some registers to implement halt and reboot functionality.

### BCMRegisters.h {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_BCMREGISTERSH}

We need to update the header containing register addresses again.

Update the file code/libraries/baremetal/include/baremetal/BCMRegisters.h

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
66: /// @brief End address for Raspberry PI 5 BCM I/O
67: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0x3FFFFFF)
68: #endif
69: 
70: //---------------------------------------------
71: // Raspberry Pi Power Management
72: //---------------------------------------------
73: 
74: // The power management features of Raspberry Pi are not well documented.
75: // Most information is extracted from the source code of an old BCM2835 watchdog timer
76: // A good reference is https://elixir.bootlin.com/linux/latest/source/drivers/watchdog/bcm2835_wdt.c or
77: // https://github.com/torvalds/linux/blob/master/drivers/watchdog/bcm2835_wdt.c
78: 
79: /// @brief Raspberry Pi Power management Registers base address. See @ref RASPBERRY_PI_POWER_MANAGEMENT
80: #define RPI_PWRMGT_BASE                 RPI_BCM_IO_BASE + 0x00100000
81: /// @brief Raspberry Pi Power management reset control register
82: #define RPI_PWRMGT_RSTC                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x0000001C)
83: /// @brief Raspberry Pi Power management reset sector register
84: #define RPI_PWRMGT_RSTS                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000020)
85: /// @brief Raspberry Pi Power management watchdog register
86: #define RPI_PWRMGT_WDOG                 reinterpret_cast<regaddr>(RPI_PWRMGT_BASE + 0x00000024)
87: /// @brief Raspberry Pi Power management magic number, to be ORed with value when setting register values
88: #define RPI_PWRMGT_WDOG_MAGIC           0x5A000000
89: /// @brief Raspberry Pi Power management clear mask.
90: #define RPI_PWRMGT_RSTC_CLEAR           0xFFFFFFCF
91: /// @brief Raspberry Pi Power management full reset bit. Can be used to check if watchdog timer is still running
92: #define RPI_PWRMGT_RSTC_REBOOT          BIT1(5)
93: /// @brief Raspberry Pi Power management watchdog timer reset code
94: #define RPI_PWRMGT_RSTC_RESET           0x00000102
95: /// @brief Raspberry Pi Power management partition bit clear mask for reset sector register. Sector number is a combination of bits 0, 2, 4, 6, 8 and 10, Sector 63 is a special case forcing a halt
96: #define RPI_PWRMGT_RSTS_PARTITION_CLEAR 0xFFFFFAAA
97: /// @brief Convert partition to register value. Partition value bits are interspersed with 0 bits
98: #define RPI_PARTITIONVALUE(x)           (((x) >> 0) & 0x01)  << 0 | (((x) >> 1) & 0x01) << 2 |  (((x) >> 2) & 0x01) << 4 |  (((x) >> 3) & 0x01) << 6 |  (((x) >> 4) & 0x01) << 8 |  (((x) >> 5) & 0x01) << 10
99: 
100: //---------------------------------------------
101: // Raspberry Pi GPIO
102: //---------------------------------------------
...
```

We will not go into the details of these registers, as they are not officially described as far as I know.
There is code in Raspberry Pi OS that also controls these registers, but without much documentation. See [here](https://github.com/raspberrypi/linux/blob/rpi-6.6.y/drivers/watchdog/bcm2835_wdt.c).
We will simply use proven working code to implement halt and reboot.

### System.cpp {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_SYSTEMCPP}

#### Implement System::Halt() {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_SYSTEMCPP_IMPLEMENT_SYSTEMHALT}

Update the file code/libraries/baremetal/src/System.cpp

```cpp
File: code/libraries/baremetal/src/System.cpp
...
71: /// <summary>
72: /// Halts the system. This function will not return
73: /// </summary>
74: void System::Halt()
75: {
76:     GetUART1().WriteString("Halt\n");
77: 
78:     for (int i = 0; i < 1000000; ++i)
79:         NOP();
80: 
81:     // power off the SoC (GPU + CPU)
82:     auto r = MemoryAccess::Read32(RPI_PWRMGT_RSTS);
83:     r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
84:     r |= RPI_PARTITIONVALUE(63);                        // Partition 63 used to indicate halt
85:     MemoryAccess::Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r);
86:     MemoryAccess::Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
87:     MemoryAccess::Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
88: 
89:     for (;;) // Satisfy [[noreturn]]
90:     {
91:         DataSyncBarrier();
92:         WaitForInterrupt();
93:     }
94: }
...
```

- Line 78-79: We wait a bit to make sure the console output is sent
- Line 82: We read the register `RPI_PWRMGT_RSTS` (probably means reset status)
- Line 83: We mask out bits 1, 3, 5, 7, 9, 11. These contain the sector value
These 6 bits indicate the partition we're booting from. 
This is normally 0, but all 1's is a special case (sector 63 means halt)
- Line 84: We set the partition to boot from to 63, to enforce a halt
- Line 85: We write the result to the `RPI_PWRMGT_RSTS` register, we apparently need to add a magic number to the value.
- Line 86: We write to the watchdog timer, presumably the value is the magic number with 10 added for 10 seconds.
- Line 87: We write a 1 to bit 5 in the reset control register, again with the magic number, in order to trigger a reset.

#### Implement System::Reboot() {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_SYSTEMCPP_IMPLEMENT_SYSTEMREBOOT}

Update the file code/libraries/baremetal/src/System.cpp

```cpp
File: code/libraries/baremetal/src/System.cpp
...
96: /// <summary>
97: /// Reboots the system. This function will not return
98: /// </summary>
99: void System::Reboot()
100: {
101:     GetUART1().WriteString("Reboot\n");
102: 
103:     DisableIRQs();
104:     DisableFIQs();
105: 
106:     // power off the SoC (GPU + CPU)
107:     auto r = MemoryAccess::Read32(RPI_PWRMGT_RSTS);
108:     r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
109:     MemoryAccess::Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
110:     MemoryAccess::Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
111:     MemoryAccess::Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
112: 
113:     for (;;) // Satisfy [[noreturn]]
114:     {
115:         DataSyncBarrier();
116:         WaitForInterrupt();
117:     }
118: }
...
```

The functionality is almost equal, except that the boot partition is set to 0 instead of 63.

### Add option to reboot or halt {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_ADD_OPTION_TO_REBOOT_OR_HALT}

Now that we can actually reboot and halt the system, it would be nice to let the user choose.
We will read a character from the serial console, if it is `h` we will halt, if it is `r` we will reboot.

#### Update application code {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_ADD_OPTION_TO_REBOOT_OR_HALT_UPDATE_APPLICATION_CODE}

Update the file code/applications/demo/src/main.cpp

```cpp
File: code/applications/demo/src/main.cpp
...
10: /// <summary>
11: /// Demo application main code
12: /// </summary>
13: /// <returns>For now always 0</returns>
14: int main()
15: {
16:     auto& uart = GetUART1();
17:     uart.WriteString("Hello World!\n");
18: 
19:     uart.WriteString("Press r to reboot, h to halt\n");
20:     char ch{};
21:     while ((ch != 'r') && (ch != 'h'))
22:     {
23:         ch = uart.Read();
24:         uart.Write(ch);
25:     }
26: 
27:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
28: }
```

- Line 16: Here we use a variable to retrieve the UART instance, note that we use `auto&` here, forgetting the reference forces a copy, which is not desired.
- Line 19: We simply tell the user to type a character
- Line 21-25: We read a character and loop until it is either `h` or `r`
- Line 27: We return the correct return code depending on the choice

### Update project configuration {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_UPDATE_PROJECT_CONFIGURATION}

As nothing was added or removed, we do not need to update the CMake files.

### Configure, build and debug {#TUTORIAL_07_IMPROVING_STARTUP_AND_STATIC_INITIALIZATION_IMPLEMENTING_HALT_AND_REBOOT___STEP_3_CONFIGURE_BUILD_AND_DEBUG}

Where building are running, you will notice that the application waits for a key to be pressed. If `r` is pressed the system will reboot, if `h` is pressed it will halt.

In QEMU this will manifest as a restart of the application (reboot) or exiting QEMU (halt).

```text
Starting up
Hello World!
Press r to reboot, h to halt
rReboot
Starting up
Hello World!
Press r to reboot, h to halt
hHalt
```

If we do not add the waiting loop, on a physical board, you will see two undesired effects:

```text
Hello World!
Press r to reboot, h to halt
�rR�Starting up
Hello World!
Press r to reboot, h to halt
�hH
```

First, you will see strange characters being written, second, you will see that the string `Reboot` or `Halt` is not fully displayed.
This has to do with the fact that we need to give the UART some time to do its work.
That is why we add the wait loop.
You will notice on a physical board that it actually reboots when `r` is pressed.

We'll get around to improving this when we dive into timers.

Next: [08-generalization](08-generalization.md)
