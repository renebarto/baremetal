# Tutorial 20: Interrupts {#TUTORIAL_20_INTERRUPTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_20_INTERRUPTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/20-interrupts`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_20_INTERRUPTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-20.a`
- an application `output/Debug/bin/20-interrupts.elf`
- an image in `deploy/Debug/20-interrupts-image`

## Exception handling - Step 1 {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1}

Before we can do anything with interrupts, we need to embrace the concept of exceptions.
These are not the same as exceptions in programming languages such as C++, but are exceptions in the context of the processor.

First a bit of detail on the ARM processor.

The ARM processor has 8 different modes of operation:

- User mode (USR): Unprivileged mode in which most applications run
- FIQ mode (FIQ): Entered on a FIQ (fast) interrupt exception
- IRQ mode (IRQ): Entered on an IRQ (nromal) interrupt exception
- Supervisor mode (SVC): Entered on reset or when a Supervisor Call instruction (SVC) is executed
- Monitor (MOD): Entered when the SMC instruction (Secure Monitor Call) is executed or when the processor takes an exception which is configured for secure handling.
Provided to support switching between Secure and Non-secure states.
- Abort mode: Data or instruction fetch is aborted. Entered on a memory access exception
- Undefined mode: Entered when an undefined instruction is executed
- System (SYS): Privileged mode, sharing the register view with User mode
- Hypervisor (HYP): Entered by the Hypervisor Call and Hypervisor Trap exceptions

### Register in the ARM processor {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_REGISTER_IN_THE_ARM_PROCESSOR}

- 64 bit registers X0-X30 (W0-W30 are 32 bit registers using the low significant 32 bits of X0-X30)
- X0-X29: General purpose 64 bit registers)
- X30: Link register
- SP (also WSP for 32 bits): Stack pointer
- PC: Program counter
- XZR (also WZR for 32 bits): Zero register

### Exception levels in the ARM processor {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_LEVELS_IN_THE_ARM_PROCESSOR}
- EL0: Application level
- EL1: Operating system level
- EL2: Hypervisor level
- EL3: Firmware privilege level / Secure monitor level

### Exception level specific registers {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_LEVEL_SPECIFIC_REGISTERS}
Each exception level has its own special registers:
- EL0: SP_EL0
- EL1: SP_EL1, ELR_EL1 (Exception Link Register), SPSR_EL1 (Saved Process Status Register)
- EL2: SP_EL2, ELR_EL2 (Exception Link Register), SPSR_EL2 (Saved Process Status Register)
- EL3: SP_EL3, ELR_EL3 (Exception Link Register), SPSR_EL3 (Saved Process Status Register)

Stack pointers can be used in a different way:
- EL0: EL0t uses SP_EL0
- EL1: EL1t uses SP_EL0, EL1h uses SP_EL1
- EL2: EL2t uses SP_EL0, EL2h uses SP_EL2
- EL3: EL3t uses SP_EL0, EL3h uses SP_EL3

### Exception types {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_TYPES}
- Interrupts: There are two types of interrupts called IRQ and FIQ.
FIQ is higher priority than IRQ. Both of these kinds of exception are typically associated with input pins on the core.
External hardware asserts an interrupt request line and the corresponding exception type is raised when the current instruction finishes executing (although some instructions, those that can load multiple values, can be interrupted), assuming that the interrupt is not disabled
- Aborts: Aborts can be generated either on failed instruction fetches (instruction aborts) or failed data accesses (Data Aborts)
- Reset: Reset is treated as a special vector for the highest implemented Exception level
- Exception generating instructions: Execution of certain instructions can generate exceptions. Such instructions are typically executed to request a service from software that runs at a higher privilege level:
  - The Supervisor Call (SVC) instruction enables User mode programs to request an OS service.
  - The Hypervisor Call (HVC) instruction enables the guest OS to request hypervisor services.
  - The Secure monitor Call (SMC) instruction enables the Normal world to request Secure world services.

### Exception vector {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_VECTOR}

When an exception occurs, the processor must execute handler code which corresponds to the exception.
The location in memory where the handler is stored is called the exception vector. 
Each entry in the vector table is 16 instructions long.

The base address is given by VBAR_ELn and then each entry has a defined offset from this base address.
Each table has 16 entries, with each entry being 128 bytes (32 instructions) in size.
The table effectively consists of 4 sets of 4 entries. Which entry is used depends upon a number of factors:
- The type of exception (SError, FIQ, IRQ or Synchronous)
- If the exception is being taken at the same Exception level, the Stack Pointer to be used (SP0 or SPx)
- If the exception is being taken at a lower Exception level, the execution state of the next lower level (AArch64 or AArch32)

<table>
<tr><th>Offset<th>Exception type<th>Description</tr>
<tr><td>0x000<td>Synchronous<td rowspan="4">Current EL with SP0</tr>
<tr><td>0x080<td>IRQ/vIRQ</tr>
<tr><td>0x100<td>FIQ/vFIQ</tr>
<tr><td>0x180<td>SError/vSError</tr>
<tr><td>0x200<td>Synchronous<td rowspan="4">Current EL with SPx</tr>
<tr><td>0x280<td>IRQ/vIRQ</tr>
<tr><td>0x300<td>FIQ/vFIQ</tr>
<tr><td>0x380<td>SError/vSError</tr>
<tr><td>0x400<td>Synchronous<td rowspan="4">Lower EL using AArch64</tr>
<tr><td>0x480<td>IRQ/vIRQ</tr>
<tr><td>0x500<td>FIQ/vFIQ</tr>
<tr><td>0x580<td>SError/vSError</tr>
<tr><td>0x600<td>Synchronous<td rowspan="4">Lower EL using AArch32</tr>
<tr><td>0x680<td>IRQ/vIRQ</tr>
<tr><td>0x700<td>FIQ/vFIQ</tr>
<tr><td>0x780<td>SError/vSError</tr>
</table>

We'll need to write some more assembly code, that implements the different exception handlers, and creates the exception vector table.

### ARMInstructions.h {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_ARMINSTRUCTIONSH}

We already create a header for the ARM specific instructions, but it is also handy to define some fields for specific ARM registers.
The complete set or ARM registers is documented in [documentation](pdf/arm-architecture-registers.pdf), the most important ones are described in [ARM registers](#ARM_REGISTERS).

We'll add some definitions for some of these registers.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
89: //------------------------------------------------------------------------------
90: // ARM register fields
91: //------------------------------------------------------------------------------
92: 
93: /// @brief SPSR_EL1 M[3:0] field bit shift
94: #define SPSR_EL1_M30_SHIFT 0
95: /// @brief SPSR_EL1 M[3:0] field bit mask
96: #define SPSR_EL1_M30_MASK BITS(0,3)
97: /// @brief SPSR_EL1 M[3:0] field exctraction
98: #define SPSR_EL1_M30(value) ((value >> SPSR_EL1_M30_SHIFT) & SPSR_EL1_M30_MASK)
99: /// @brief SPSR_EL1 M[3:0] field value for EL0t mode
100: #define SPSR_EL1_M30_EL0t 0x0
101: /// @brief SPSR_EL1 M[3:0] field value for EL1t mode
102: #define SPSR_EL1_M30_EL1t 0x4
103: /// @brief SPSR_EL1 M[3:0] field value for EL1h mode
104: #define SPSR_EL1_M30_EL1h 0x5
105: 
106: /// @brief ESR_EL1 EC field bit shift
107: #define ESR_EL1_EC_SHIFT 26
108: /// @brief ESR_EL1 EC field bit mask
109: #define ESR_EL1_EC_MASK  BITS(0,5)
110: /// @brief ESR_EL1 EC field extraction
111: #define ESR_EL1_EC(value) ((value >> ESR_EL1_EC_SHIFT) & ESR_EL1_EC_MASK)
112: /// @brief ESR_EL1 EC field value for unknown exception
113: #define ESR_EL1_EC_UNKNOWN 0x00
114: /// @brief ESR_EL1 EC field value for trapped WF<x> instruction exception
115: #define ESR_EL1_EC_TRAPPED_WFx_INSTRUCTION 0x01
116: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0F
117: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0F 0x03
118: /// @brief ESR_EL1 EC field value for MCRR or MRRC instruction exception when coproc = 0x0F
119: #define ESR_EL1_EC_TRAPPED_MCRR_MRRC_ACCESS_CORPROC_0F 0x04
120: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0E
121: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0E 0x05
122: /// @brief ESR_EL1 EC field value for trapped LDC or STC instruction exception
123: #define ESR_EL1_EC_TRAPPED_LDC_STC_ACCESS 0x06
124: /// @brief ESR_EL1 EC field value for unknown SME, SVE, SIMD or Floating pointer instruction exception
125: #define ESR_EL1_EC_TRAPPED_SME_SVE_SIMD_FP_ACCESS 0x07
126: /// @brief ESR_EL1 EC field value for trapped LD64<x> or ST64<x> instruction exception
127: #define ESR_EL1_EC_TRAPPED_LD64x_ST64x_ACCESS 0x0A
128: /// @brief ESR_EL1 EC field value for trapped MRRC instruction exception when coproc = 0x0C
129: #define ESR_EL1_EC_TRAPPED_MRRC_ACCESS_COPROC_0E 0x0C
130: /// @brief ESR_EL1 EC field value for branch target exception
131: #define ESR_EL1_EC_BRANCH_TARGET_EXCEPTION 0x0D
132: /// @brief ESR_EL1 EC field value for illegal executions state exception
133: #define ESR_EL1_EC_ILLEGAL_EXECUTION_STATE 0x0E
134: /// @brief ESR_EL1 EC field value for trapped SVC 32 bit instruction exception
135: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_32 0x11
136: /// @brief ESR_EL1 EC field value for trapped SVC 64 bit instruction exception
137: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_64 0x15
138: /// @brief ESR_EL1 EC field value for MCR or MRC 64 bit instruction exception
139: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_64 0x18
140: /// @brief ESR_EL1 EC field value for trapped SVE access exception
141: #define ESR_EL1_EC_TRAPPED_SVE_ACCESS 0x19
142: /// @brief ESR_EL1 EC field value for trapped TStart access exception
143: #define ESR_EL1_EC_TRAPPED_TSTART_ACCESS 0x1B
144: /// @brief ESR_EL1 EC field value for pointer authentication failure exception
145: #define ESR_EL1_EC_POINTER_AUTHENTICATION_FAILURE 0x1C
146: /// @brief ESR_EL1 EC field value for trapped SME access exception
147: #define ESR_EL1_EC_TRAPPED_SME_ACCESS 0x1D
148: /// @brief ESR_EL1 EC field value for granule protection check exception
149: #define ESR_EL1_EC_GRANULE_PROTECTION_CHECK 0x1E
150: /// @brief ESR_EL1 EC field value for instruction abort from lower EL exception
151: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL 0x20
152: /// @brief ESR_EL1 EC field value for instruction abort from same EL exception
153: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_SAME_EL 0x21
154: /// @brief ESR_EL1 EC field value for PC alignment fault exception
155: #define ESR_EL1_EC_PC_ALIGNMENT_FAULT 0x22
156: /// @brief ESR_EL1 EC field value for data abort from lower EL exception
157: #define ESR_EL1_EC_DATA_ABORT_FROM_LOWER_EL 0x24
158: /// @brief ESR_EL1 EC field value for data abort from same EL exception
159: #define ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL 0x25
160: /// @brief ESR_EL1 EC field value for SP alignment fault exception
161: #define ESR_EL1_EC_SP_ALIGNMENT_FAULT 0x27
162: /// @brief ESR_EL1 EC field value for trapped 32 bit FP instruction exception
163: #define ESR_EL1_EC_TRAPPED_FP_32 0x28
164: /// @brief ESR_EL1 EC field value for trapped 64 bit FP instruction exception
165: #define ESR_EL1_EC_TRAPPED_FP_64 0x2C
166: /// @brief ESR_EL1 EC field value for SError interrupt exception
167: #define ESR_EL1_EC_SERROR_INTERRUPT 0x2F
168: /// @brief ESR_EL1 EC field value for Breakpoint from lower EL exception
169: #define ESR_EL1_EC_BREAKPOINT_FROM_LOWER_EL 0x30
170: /// @brief ESR_EL1 EC field value for Breakpoint from same EL exception
171: #define ESR_EL1_EC_BREAKPOINT_FROM_SAME_EL 0x31
172: /// @brief ESR_EL1 EC field value for SW step from lower EL exception
173: #define ESR_EL1_EC_SW_STEP_FROM_LOWER_EL 0x32
174: /// @brief ESR_EL1 EC field value for SW step from same EL exception
175: #define ESR_EL1_EC_SW_STEP_FROM_SAME_EL 0x33
176: /// @brief ESR_EL1 EC field value for Watchpoint from lower EL exception
177: #define ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL 0x34
178: /// @brief ESR_EL1 EC field value for Watchpoint from same EL exception
179: #define ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL 0x35
180: /// @brief ESR_EL1 EC field value for 32 bit BKPT instruction exception
181: #define ESR_EL1_EC_BKPT_32 0x38
182: /// @brief ESR_EL1 EC field value for 64 bit BRK instruction exception
183: #define ESR_EL1_EC_BRK_64 0x3C
184: 
185: /// @brief ESR_EL1 ISS field bit shift
186: #define ESR_EL1_ISS_SHIFT 0
187: /// @brief ESR_EL1 ISS field bit mask
188: #define ESR_EL1_ISS_MASK  BITS(0,24)
189: /// @brief ESR_EL1 ISS field extraction
190: #define ESR_EL1_ISS(value) ((value >> ESR_EL1_ISS_SHIFT) & ESR_EL1_ISS_MASK)
```

- Line 94-98: We define the M[3:0] field in the SPSR_EL1 register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 100-104: We define different values for this field
- Line 107-111: We define the EC field in the ESR_EL1 register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 113-183: We define different values for this field
- Line 186-190: We define the ISS field in the ESR_EL1 register for AArch64. See also [ARM registers](#ARM_REGISTERS)

### ExceptionHandler.h {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONHANDLERH}

We will create a prototype for the exception handler.

Create the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExceptionHandler.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Exception handler
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
44: /// @file
45: /// Exception handling
46: 
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: /// @brief Exception abort frame
52: ///
53: /// Storage for register value in case of exception, in order to recover
54: struct AbortFrame
55: {
56:     /// @brief Exception Syndrome Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
57:     uint64	esr_el1;
58:     /// @brief Saved Program Status Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER
59:     uint64	spsr_el1;
60:     /// @brief General-purpose register, Link Register
61:     uint64	x30;
62:     /// @brief Exception Link Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
63:     uint64	elr_el1;
64:     /// @brief Stack Pointer (EL0). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
65:     uint64	sp_el0;
66:     /// @brief Stack Pointer (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
67:     uint64	sp_el1;
68:     /// @brief Fault Address Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
69:     uint64	far_el1;
70:     /// @brief Unused valuem used to align to 64 bytes
71:     uint64	unused;
72: }
73: PACKED;
74: 
75: /// @brief Handles an exception, with the abort frame passed in.
76: ///
77: /// The exception handler is called from assembly code (ExceptionStub.S)
78: /// @param exceptionID Exception type being thrown (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
79: /// @param abortFrame  Filled in AbortFrame instance.
80: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);
81: 
82: #ifdef __cplusplus
83: }
84: #endif
```

- Line 54-73: We create struct `AbortFrame` which will hold all important information on entry. This information is actually registers saved on the stack, as we'll see later.
- Line 80: We declare the exception handler function `ExceptionHandler()` which will receive two parameters.
The first parameter is an exception ID which we will set, the second is a pointer to the `AbortFrame` which is actually the stack pointer value

### ExceptionHandler.cpp {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONHANDLERCPP}

We will implement the exception handler.

Create the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExceptionHandler.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Exception handler
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
40: #include <baremetal/ExceptionHandler.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/ExceptionSystem.h>
44: 
45: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
46: {
47:     EnableFIQs();
48: 
49:     baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
50: }
```

- Line 43: We include the exception system header which we will create next.
This is to create a class that does the handling in C++ domain
- Line 45-50: We implement the `ExceptionHandler()` function. Next to enabling the FIQ interrupts, we simply relay to the `ExceptionSystem` class

### ExceptionSystem.h {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONSYSTEMH}

We will the actual exception handling class `ExceptionSystem`.

Create the file `code/libraries/baremetal/include/baremetal/ExceptionSystem.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionSystem.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExceptionSystem.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : ExceptionSystem
9: //
10: // Description : Exception handler
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
42: #include <baremetal/Macros.h>
43: #include <baremetal/ExceptionHandler.h>
44: 
45: /// @file
46: /// Exception handling system
47: 
48: /// @brief Exception vector table
49: ///
50: /// The exception vector table is used to select an exception type depending on conditions:
51: /// - Whether the current EL is equal to EL0 or EL1 or greater
52: /// - Whether the EL the exception was called from was lower that the current, and which architecture was used.
53: ///
54: /// The exception vector table has a total of 16 entries, each 128 bytes in size.
55: ///
56: /// Offset  Exception type      Description
57: /// 0x000   Synchronous         Current EL with SP0
58: /// 0x080   IRQ/vIRQ
59: /// 0x100   FIQ/vFIQ
60: /// 0x180   SError/vSError
61: ///
62: /// 0x200   Synchronous         Current EL with SPx
63: /// 0x280   IRQ/vIRQ
64: /// 0x300   FIQ/vFIQ
65: /// 0x380   SError/vSError
66: ///
67: /// 0x400   Synchronous         Lower EL using AArch64
68: /// 0x480   IRQ/vIRQ
69: /// 0x500   FIQ/vFIQ
70: /// 0x580   SError/vSError
71: ///
72: /// 0x600   Synchronous         Lower EL using AArch32
73: /// 0x680   IRQ/vIRQ
74: /// 0x700   FIQ/vFIQ
75: /// 0x780   SError/vSError
76: ///
77: /// Each entry simple has a 32 bit pointer to a function
78: 
79: /// <summary>
80: /// Exception vector table
81: /// </summary>
82: struct VectorTable
83: {
84:     /// <summary>
85:     /// Exception vector table contents
86:     /// </summary>
87:     struct
88:     {
89:         uint32	Branch;
90:         uint32	Dummy[31];      // Align to 128 bytes
91:     }
92:     Vector[16];
93: }
94: PACKED;
95: 
96: namespace baremetal {
97: 
98: /// @brief Exception handling system. Handles ARM processor exceptions
99: /// This is a singleton class, created as soon as GetExceptionSystem() is called
100: class ExceptionSystem
101: {
102:     friend ExceptionSystem& GetExceptionSystem();
103: 
104: private:
105:     /// @brief Create a exception handling system. Note that the constructor is private, so GetExceptionSystem() is needed to instantiate the exception handling system
106:     ExceptionSystem();
107: 
108: public:
109:     ~ExceptionSystem();
110: 
111:     /// @brief Throw a ARM exception with an abort frame
112:     /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
113:     /// @param abortFrame   Stored state information at the time of exception
114:     void Throw(unsigned exceptionID, AbortFrame* abortFrame);
115: };
116: 
117: /// @brief Callback function called by the exception handler if registered
118: /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
119: /// @param abortFrame   Stored state information at the time of exception
120: using ExceptionPanicHandler = bool(unsigned exceptionID, AbortFrame* abortFrame);
121: 
122: /// @brief Convert exception ID to a string
123: /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
124: /// @return String representation of exception ID
125: const char* GetExceptionType(unsigned exceptionID);
126: 
127: /// @brief Register an exception callback function, and return the previous one.
128: /// @param handler      Exception handler callback function to register
129: /// @return Previously set exception handler callback function
130: ExceptionPanicHandler* RegisterPanicHandler(ExceptionPanicHandler* handler);
131: 
132: /// @brief Retrieve the singleton exception handling system
133: ///
134: /// Creates a static instance of ExceptionSystem, and returns a reference to it.
135: /// @return A reference to the singleton exception handling system.
136: ExceptionSystem& GetExceptionSystem();
137: 
138: } // namespace baremetal
```

- Line 82-94: We declare the exception vector table type `VectorTable`. This contains the 16 different exception vector entries
- Line 100-115: We declare the class `ExceptionSystem`. This is the acutal handler class for exceptions
  - Line 102: We make the function `GetExceptionSystem()` a friend. This function is the only way to create an instance of `ExceptionSystem`
  - Line 106: We declare the (private) constructor
  - Line 109: We declare the destructor
  - Line 114: We declare the method `Throw()` which is used to handle an exception
- Line 120: We declare a function type `ExceptionPanicHandler` which is used to enable a hook for a function to handle panics
- Line 125: We declare a function `GetExceptionType()` to convert an exception ID to a string
- Line 130: We declare a function `RegisterPanicHandler()` to set or reset a panic function handler
- Line 136 We declare the function `GetExceptionSystem()` which is used to create and return the singleton instance of `ExceptionSystem`

### ExceptionSystem.cpp {#TUTORIAL_20_INTERRUPTS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONSYSTEMCPP}

We will implement the exception handler.

Create the file `code/libraries/baremetal/src/ExceptionSystem.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionSystem.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExceptionSystem.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : ExceptionSystem
9: //
10: // Description : Exception handling class
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
40: #include <baremetal/ExceptionSystem.h>
41: #include <baremetal/Exception.h>
42: 
43: #include <baremetal/ARMInstructions.h>
44: #include <baremetal/Assert.h>
45: #include <baremetal/Logger.h>
46: #include <baremetal/String.h>
47: #include <baremetal/Synchronization.h>
48: #include <baremetal/System.h>
49: #include <baremetal/InterruptHandler.h>
50: 
51: namespace baremetal {
52: 
53: static const char FromExcept[] = "except";
54: 
55: ExceptionPanicHandler* s_exceptionPanicHandler;
56: 
57: ExceptionSystem::~ExceptionSystem()
58: {
59: }
60: 
61: // order must match exception identifiers in baremetal/Exception.h
62: static const char* s_exceptionName[] =
63: {
64:     "Unexpected exception",
65:     "Synchronous exception",
66:     "System error"
67: };
68: 
69: ExceptionSystem::ExceptionSystem()
70: {
71: }
72: 
73: void ExceptionSystem::Throw(unsigned exceptionID, AbortFrame* abortFrame)
74: {
75:     assert(abortFrame != nullptr);
76: 
77:     uint64 sp = abortFrame->sp_el0;
78:     if (SPSR_EL1_M30(abortFrame->spsr_el1) == SPSR_EL1_M30_EL1h)		// EL1h mode?
79:     {
80:         sp = abortFrame->sp_el1;
81:     }
82: 
83:     uint64 EC = ESR_EL1_EC(abortFrame->esr_el1);
84:     uint64 ISS = ESR_EL1_ISS(abortFrame->esr_el1);
85: 
86:     uint64 FAR = 0;
87:     if ((ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL)
88:         || (ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL))
89:     {
90:         FAR = abortFrame->far_el1;
91:     }
92: 
93:     bool halt = true;
94:     if (s_exceptionPanicHandler != nullptr)
95:     {
96:         halt = (*s_exceptionPanicHandler)(exceptionID, abortFrame);
97:     }
98: 
99:     GetLogger().Write(FromExcept, __LINE__, halt ? LogSeverity::Panic : LogSeverity::Error,
100:         "%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
101:         s_exceptionName[exceptionID],
102:         abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
103: }
104: 
105: const char* GetExceptionType(unsigned exceptionID)
106: {
107:     return s_exceptionName[exceptionID];
108: }
109: 
110: ExceptionPanicHandler* RegisterPanicHandler(ExceptionPanicHandler* handler)
111: {
112:     auto result = s_exceptionPanicHandler;
113:     s_exceptionPanicHandler = handler;
114:     return result;
115: }
116: 
117: ExceptionSystem& GetExceptionSystem()
118: {
119:     static ExceptionSystem system;
120:     return system;
121: }
122: 
123: } // namespace baremetal
```

## Interrupt handling - Step 2 {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2}

We'll add a class `InterruptSystem` to enable, disable, and handle interrupts.

### ARMRegisters.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_ARMREGISTERSH}

Next to the Raspberry Pi registers for peripherals etc., there are also ARM processor registers, but specific for Raspberry Pi.
The complete set of registers is defined in [documentation](pdf/bcm2836-peripherals.pdf), the most important ones are described in [ARM local device registers](#RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS).

The ARM specific registers are in a different address range, reaching from 0x40000000 to 0x4003FFFF on Raspberry Pi 3, and 0xFF800000 to 0xFF83FFFF on Raspberry Pi 4 and later.

We'll add the definition for the register we will be using.

Create the file `code/libraries/baremetal/include/baremetal/ARMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMRegisters.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ARMRegisters.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Locations and definitions for Raspberry Pi ARM registers
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
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
40: /// @file
41: /// Register addresses of Raspberry Pi peripheral registers.
42: ///
43: /// For specific registers, we also define the fields and their possible values.
44: 
45: #pragma once
46: 
47: #include <baremetal/Macros.h>
48: #include <baremetal/Types.h>
49: 
50: #if BAREMETAL_RPI_TARGET <= 3
51: #define ARM_LOCAL_BASE 0x40000000
52: #else
53: #define ARM_LOCAL_BASE 0xFF800000
54: #endif
55: 
56: /// @brief Raspberry Pi ARM Local Control Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
57: #define ARM_LOCAL_CONTROL                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000000)
58: /// @brief Raspberry Pi ARM Local Core Timer Prescaler Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
59: #define ARM_LOCAL_PRESCALER                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000008)
60: /// @brief Raspberry Pi ARM Local GPU Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
61: #define ARM_LOCAL_GPU_INT_ROUTING          reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000000C)
62: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
63: #define ARM_LOCAL_PM_ROUTING_SET           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000010)
64: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
65: #define ARM_LOCAL_PM_ROUTING_CLR           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000014)
66: /// @brief Raspberry Pi ARM Local Core Timer Least Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
67: #define ARM_LOCAL_TIMER_LS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000001C)
68: /// @brief Raspberry Pi ARM Local Core Timer Most Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
69: #define ARM_LOCAL_TIMER_MS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000020)
70: #define ARM_LOCAL_INT_ROUTING              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000024)
71: /// @brief Raspberry Pi ARM Local AXI Outstanding Read/Write Counters Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
72: #define ARM_LOCAL_AXI_COUNT                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000002C)
73: /// @brief Raspberry Pi ARM Local AXI Outstanding Interrupt Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
74: #define ARM_LOCAL_AXI_IRQ                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000030)
75: /// @brief Raspberry Pi ARM Local Timer Control / Status Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
76: #define ARM_LOCAL_TIMER_CONTROL            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000034)
77: /// @brief Raspberry Pi ARM Local Timer IRQ Clear / Reload Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
78: #define ARM_LOCAL_TIMER_WRITE              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000038)
79: 
80: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
81: #define ARM_LOCAL_TIMER_INT_CONTROL0       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000040)
82: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
83: #define ARM_LOCAL_TIMER_INT_CONTROL1       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000044)
84: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
85: #define ARM_LOCAL_TIMER_INT_CONTROL2       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000048)
86: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
87: #define ARM_LOCAL_TIMER_INT_CONTROL3       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000004C)
88: 
89: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
90: #define ARM_LOCAL_MAILBOX_INT_CONTROL0 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000050)
91: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
92: #define ARM_LOCAL_MAILBOX_INT_CONTROL1 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000054)
93: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
94: #define ARM_LOCAL_MAILBOX_INT_CONTROL2 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000058)
95: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
96: #define ARM_LOCAL_MAILBOX_INT_CONTROL3 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000005C)
97: 
98: /// @brief Raspberry Pi ARM Local Core 0 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
99: #define ARM_LOCAL_IRQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000060)
100: /// @brief Raspberry Pi ARM Local Core 1 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
101: #define ARM_LOCAL_IRQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000064)
102: /// @brief Raspberry Pi ARM Local Core 2 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
103: #define ARM_LOCAL_IRQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000068)
104: /// @brief Raspberry Pi ARM Local Core 3 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
105: #define ARM_LOCAL_IRQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000006C)
106: 
107: /// @brief Raspberry Pi ARM Local Core 0 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
108: #define ARM_LOCAL_FIQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000070)
109: /// @brief Raspberry Pi ARM Local Core 1 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
110: #define ARM_LOCAL_FIQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000074)
111: /// @brief Raspberry Pi ARM Local Core 2 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
112: #define ARM_LOCAL_FIQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000078)
113: /// @brief Raspberry Pi ARM Local Core 3 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
114: #define ARM_LOCAL_FIQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000007C)
115: 
116: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
117: #define ARM_LOCAL_MAILBOX0_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000080)
118: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
119: #define ARM_LOCAL_MAILBOX1_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000084)
120: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
121: #define ARM_LOCAL_MAILBOX2_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000088)
122: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
123: #define ARM_LOCAL_MAILBOX3_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000008C)
124: 
125: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
126: #define ARM_LOCAL_MAILBOX0_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000090)
127: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
128: #define ARM_LOCAL_MAILBOX1_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000094)
129: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
130: #define ARM_LOCAL_MAILBOX2_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000098)
131: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
132: #define ARM_LOCAL_MAILBOX3_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000009C)
133: 
134: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
135: #define ARM_LOCAL_MAILBOX0_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A0)
136: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
137: #define ARM_LOCAL_MAILBOX1_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A4)
138: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
139: #define ARM_LOCAL_MAILBOX2_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A8)
140: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
141: #define ARM_LOCAL_MAILBOX3_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000AC)
142: 
143: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
144: #define ARM_LOCAL_MAILBOX0_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B0)
145: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
146: #define ARM_LOCAL_MAILBOX1_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B4)
147: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
148: #define ARM_LOCAL_MAILBOX2_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B8)
149: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
150: #define ARM_LOCAL_MAILBOX3_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000BC)
151: 
152: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
153: #define ARM_LOCAL_MAILBOX0_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C0)
154: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
155: #define ARM_LOCAL_MAILBOX1_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C4)
156: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
157: #define ARM_LOCAL_MAILBOX2_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C8)
158: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
159: #define ARM_LOCAL_MAILBOX3_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000CC)
160: 
161: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
162: #define ARM_LOCAL_MAILBOX0_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D0)
163: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
164: #define ARM_LOCAL_MAILBOX1_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D4)
165: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
166: #define ARM_LOCAL_MAILBOX2_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D8)
167: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
168: #define ARM_LOCAL_MAILBOX3_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000DC)
169: 
170: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
171: #define ARM_LOCAL_MAILBOX0_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E0)
172: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
173: #define ARM_LOCAL_MAILBOX1_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E4)
174: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
175: #define ARM_LOCAL_MAILBOX2_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E8)
176: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
177: #define ARM_LOCAL_MAILBOX3_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000EC)
178: 
179: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
180: #define ARM_LOCAL_MAILBOX0_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F0)
181: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
182: #define ARM_LOCAL_MAILBOX1_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F4)
183: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
184: #define ARM_LOCAL_MAILBOX2_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F8)
185: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
186: #define ARM_LOCAL_MAILBOX3_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000FC)
187: 
188: /// @brief Raspberry Pi ARM Local Register region end address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
189: #define ARM_LOCAL_END                      reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000003FFFF)
```

We'll explain the details on each of these registers as we use them.

### InterruptSystem.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_INTERRUPTSYSTEMH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/unittest/include/unittest/TestDetails.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestDetails.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestDetails
9: //
10: // Description : Test detail
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
42: #include <baremetal/String.h>
43: 
44: namespace unittest
45: {
46: 
47: class TestDetails
48: {
49: private:
50:     const baremetal::string m_suiteName;
51:     const baremetal::string m_fixtureName;
52:     const baremetal::string m_testName;
53:     const baremetal::string m_fileName;
54:     const int m_lineNumber;
55: 
56: public:
57:     TestDetails();
58:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
59: 
60:     const baremetal::string& SuiteName() const { return m_suiteName; }
61:     const baremetal::string& FixtureName() const { return m_fixtureName; }
62:     const baremetal::string& TestName() const { return m_testName; }
63:     const baremetal::string& SourceFileName() const { return m_fileName; }
64:     int SourceFileLineNumber() const { return m_lineNumber; }
65: };
66: 
67: } // namespace unittest
```

The `TestDetails` class is added to the `unittest` namespace.

- Line 42: We use strings, so we need to include the header for the `string` class
- Line 47-65: We declare the class `TestDetails` which will hold information on a test
  - Line 50: The class member variable `m_suiteName` is the test suite name
  - Line 51: The class member variable `m_fixtureName` is the test fixture name
  - Line 52: The class member variable `m_testName` is the test name
  - Line 53: The class member variable `m_fileName` is the source file in which the actual test is defined
  - Line 54: The class member variable `m_lineNumber` is the source line in which the actual test is defined
  - Line 57: We declare the default constructor
  - Line 58: We declare the normal constructor which specifies all the needed information
  - Line 60: We declare an accessor `SuiteName()` for the test suite name
  - Line 61: We declare an accessor `FixtureName()` for the test fixture name
  - Line 62: We declare an accessor `TestName()` for the test name
  - Line 63: We declare an accessor `SourceFileName()` for the source file name
  - Line 64: We declare an accessor `SourceFileLineNumber()` for the source line number

### TestDetails.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_TESTDETAILSCPP}

Let's implement the `TestDetails` class.

Create the file `code/libraries/unittest/src/TestDetails.cpp`

```cpp
File: code/libraries/unittest/src/TestDetails.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestDetails.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestDetails
9: //
10: // Description : Test details
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
40: #include <unittest/TestDetails.h>
41:
42: using namespace baremetal;
43:
44: namespace unittest {
45:
46: TestDetails::TestDetails()
47:     : m_suiteName{}
48:     , m_fixtureName{}
49:     , m_testName{}
50:     , m_fileName{}
51:     , m_lineNumber{}
52: {
53: }
54:
55: TestDetails::TestDetails(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
56:     : m_suiteName{ suiteName }
57:     , m_fixtureName{ fixtureName }
58:     , m_testName{ testName }
59:     , m_fileName{ fileName }
60:     , m_lineNumber{ lineNumber }
61: {
62: }
63:
64: } // namespace unittest
```

- Line 46-53: We implement the default constructor
- Line 55-62: We implement the non default constructor

### TestBase.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_TESTBASEH}

We will add a base class for each test. All tests will derive from this class, and implement its `RunImpl()` method to run the actual test.

Create the file `code/libraries/unittest/include/unittest/TestBase.h`

```cpp
File: code/libraries/unittest/include/unittest/TestBase.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestBase.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestBase
9: //
10: // Description : Testcase
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
42: #include <unittest/TestDetails.h>
43:
44: namespace unittest
45: {
46:
47: class TestBase
48: {
49: private:
50:     TestDetails const m_details;
51:     TestBase* m_next;
52:
53: public:
54:     TestBase();
55:     TestBase(const TestBase&) = delete;
56:     TestBase(TestBase&&) = delete;
57:     explicit TestBase(
58:         const baremetal::string& testName,
59:         const baremetal::string& fixtureName = {},
60:         const baremetal::string& suiteName = {},
61:         const baremetal::string& fileName = {},
62:         int lineNumber = {});
63:     virtual ~TestBase();
64:
65:     TestBase& operator = (const TestBase&) = delete;
66:     TestBase& operator = (TestBase&&) = delete;
67:
68:     const TestDetails& Details() const { return m_details; }
69:
70:     void Run();
71:
72:     virtual void RunImpl() const;
73: };
74:
75: } // namespace unittest
```

The `TestBase` class is added to the `unittest` namespace.

- Line 50: We declare the details for the test
- Line 51: We declare a pointer to the next test. Tests will be stored in a linked list
- Line 54: We declare a default constructor
- Line 55-56: We remove the copy constructor and move constructor
- Line 57: We declare an explicit constructor
- Line 63: We declare the destructor. This may be important as we will be inheriting from this class
- Line 65-66: We remove the assignment operators
- Line 68: We declare methods to retrieve details
- Line 70: We declare a method `Run()` to run the test.
This will ultimately invoke the `RunImpl()` virtual method, which is expected to be overriden by an actual test.
- Line 72: We declare the overridable `RunImpl()` method

### TestBase.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_TESTBASECPP}

We'll implement the `TestBase` class.

Create the file `code/libraries/unittest/src/TestBase.cpp`

```cpp
File: code/libraries/unittest/src/TestBase.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestBase.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestBase
9: //
10: // Description : Testcase base class
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
40: #include <unittest/TestBase.h>
41:
42: using namespace baremetal;
43:
44: namespace unittest {
45:
46: TestBase::TestBase()
47:     : m_details{}
48:     , m_next{}
49: {
50: }
51:
52: TestBase::TestBase(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
53:     : m_details{ testName, fixtureName, suiteName, fileName, lineNumber }
54:     , m_next{}
55: {
56: }
57:
58: TestBase::~TestBase()
59: {
60: }
61:
62: void TestBase::Run()
63: {
64:     RunImpl();
65: }
66:
67: void TestBase::RunImpl() const
68: {
69: }
70:
71: } // namespace unittest
```

- Line 46-50: We implement the default constructor
- Line 52-56: We implement the non default constructor
- Line 58-60: We implement the destructor
- Line 62-65: We provide a first implementation for the `Run()` method
- Line 67-69: We provide a default implementation for the `RunImpl()` method

### Update CMake file {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_UPDATE_CMAKE_FILE}

As we have now added some source files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
33:     )
34:
35: set(PROJECT_INCLUDES_PUBLIC
36:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
37:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
38:     )
39: set(PROJECT_INCLUDES_PRIVATE )
40:
...
```

### Update application code {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.

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
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: #include <unittest/TestBase.h>
18:
19: LOG_MODULE("main");
20:
21: using namespace baremetal;
22: using namespace unittest;
23:
24: class MyTest
25:     : public TestBase
26: {
27: public:
28:     MyTest()
29:         : TestBase("MyTest", "", "", __FILE__, __LINE__)
30:     {
31:
32:     }
33:     void RunImpl() const override
34:     {
35:         LOG_DEBUG("In RunImpl");
36:     }
37: };
38:
39: int main()
40: {
41:     auto& console = GetConsole();
42:     LOG_DEBUG("Hello World!");
43:
44:     MyTest test;
45:     test.Run();
46:
47:     LOG_INFO("Wait 5 seconds");
48:     Timer::WaitMilliSeconds(5000);
49:
50:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
51:     char ch{};
52:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
53:     {
54:         ch = console.ReadChar();
55:         console.WriteChar(ch);
56:     }
57:     if (ch == 'p')
58:         assert(false);
59:
60:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
61: }
```

- Line 17: We include the header for `TestBase`
- Line 24-38: We declare and implement the class `MyTest` based on `TestBase`
  - Line 28-32: We declare and implement the constructor. We use the class name as the test name, and set the test fixture name and test suite name to an empty string. The file name and line number are taken from the actual source location
  - Line 33-37: We declare and implement an override for the `RunImpl()` method. It simply logs a string
- Line 45-46: We define an instance of MyTest, and then run the test.

### Configuring, building and debugging {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. We'll first switch off the memory debug output to get a cleaner console.

Update the file `CMakeLists.txt`

```cmake
...
61: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
62: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
63: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
64: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
65: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
66: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
...
```

The application will run the test, and therefore show the log output.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Info   Starting up (System:201)
Debug  Hello World! (main:42)
Debug  In RunImpl (main:35)
Info   Wait 5 seconds (main:47)
Press r to reboot, h to halt, p to fail assertion and panic
```

Next: [21-timer-extension](21-timer-extension.md)
