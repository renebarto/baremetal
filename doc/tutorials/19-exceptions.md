# Tutorial 19: Exceptions {#TUTORIAL_19_EXCEPTIONS}

@tableofcontents

## New tutorial setup {#TUTORIAL_19_EXCEPTIONS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/19-exceptions`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_19_EXCEPTIONS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-19.a`
- an application `output/Debug/bin/19-exceptions.elf`
- an image in `deploy/Debug/19-exceptions-image`

## Exception handling - Step 1 {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1}

Before we can do anything with interrupts, we need to embrace the concept of exceptions.
These are not the same as exceptions in programming languages such as C++, but are exceptions in the context of the processor.

First a bit of detail on the ARM processor.

The ARM processor has 8 different modes of operation:

- User mode (USR): Unprivileged mode in which most applications run
- FIQ mode (FIQ): Entered on a FIQ (fast) interrupt exception
- IRQ mode (IRQ): Entered on an IRQ (normal) interrupt exception
- Supervisor mode (SVC): Entered on reset or when a Supervisor Call instruction (SVC) is executed
- Secure Monitor (MON): Entered when the SMC instruction (Secure Monitor Call) is executed or when the processor takes an exception which is configured for secure handling.
Provided to support switching between Secure and Non-secure states.
- Abort mode (ABT): Data or instruction fetch is aborted. Entered on a memory access exception
- Undefined mode (UND): Entered when an undefined instruction is executed
- System (SYS): Privileged mode, sharing the register view with User mode
- Hypervisor (HYP): Entered by the Hypervisor Call and Hypervisor Trap exceptions

### Register in the ARM processor {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_REGISTER_IN_THE_ARM_PROCESSOR}

- 64 bit registers X0-X30 (W0-W30 are 32 bit registers using the low significant 32 bits of X0-X30)
- 128 bit floating point registers Q0-Q31 (D0-D31 are 64 bit registers using the low significant 64 bits, similar for S0-S31 with 32 bits, H0-H31 with 16 bits, B0-B31 with 8 bits)
- X0-X29: General purpose 64 bit registers)
- X30: Link register (LR)
- SP (also WSP for 32 bits): Stack pointer
- PC: Program counter (this is not accessible directly, but needs specific instructions)
- XZR (also WZR for 32 bits): Zero register
- There is also a large set of system registers. See [ARM registers](#ARM_REGISTERS), [documentation](pdf/arm-architecture-registers.pdf), and [ARM� Architecture Reference Manual](pdf/AArch64ReferenceManual.1410976032.pdf) for more information.

### Exception levels in the ARM processor {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_LEVELS_IN_THE_ARM_PROCESSOR}

ARMv8 has four exception levels:
- EL0: Application level
- EL1: Operating system level
- EL2: Hypervisor level
- EL3: Firmware privilege level / Secure monitor level

### Exception level specific registers {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_LEVEL_SPECIFIC_REGISTERS}

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

### Exception types {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_TYPES}

- Interrupts: There are two types of interrupts called IRQ and FIQ.
FIQ is higher priority than IRQ. Both of these kinds of exception are typically associated with input pins on the core.
External hardware asserts an interrupt request line and the corresponding exception type is raised when the current instruction finishes executing (although some instructions, those that can load multiple values, can be interrupted), assuming that the interrupt is not disabled
  - IRQ can be interrupted by FIQ
  - FIQ cannot be interrupted by other FIQ, only one can be active at any time
- Aborts: Aborts can be generated either on failed instruction fetches (instruction aborts) or failed data accesses (data aborts)
- Reset: Reset is treated as a special vector for the highest implemented Exception level
- Exception generating instructions: Execution of certain instructions can generate exceptions. Such instructions are typically executed to request a service from software that runs at a higher privilege level:
  - The Supervisor Call (SVC) instruction enables User mode programs to request an OS service.
  - The Hypervisor Call (HVC) instruction enables the guest OS to request hypervisor services.
  - The Secure monitor Call (SMC) instruction enables the Normal world to request Secure world services.

### Exception vector {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTION_VECTOR}

When an exception occurs, the processor must execute handler code which corresponds to the exception.
The location in memory where the handler is stored is called the exception vector.
Each entry in the vector table is 16 instructions long.

The base address is given by VBAR_ELn and then each entry has a defined offset from this base address.
Each table has 16 entries, with each entry being 128 bytes (32 instructions) in size. Each exception level (except EL0) has its own exception vector.
The table effectively consists of 4 sets of 4 entries each. Which entry is used depends upon a number of factors:
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

### Macros.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_MACROSH}

We are going to use a new macro `BITS` in the previous file, which we'll need to define.

Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
64: /// @brief Convert bit range into integer
65: /// @param n Start (low) bit index
66: /// @param m End (high) bit index
67: #define BITS(n,m)           (((1U << (m-n+1)) - 1) << (n))
```

This defines the `BITS` macro to create a mask for bit sequences (from bit n to up to and including bit m)

### ARMInstructions.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_ARMINSTRUCTIONSH}

We already create a header for the ARM specific instructions, but it is also handy to define some fields for specific ARM registers.
The complete set or ARM registers is documented in [documentation](pdf/arm-architecture-registers.pdf), the most important ones are described in [ARM registers](#ARM_REGISTERS).

We'll add some definitions for some of these registers.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
42: #include <baremetal/Macros.h>
43:
...
91: /// @brief Get current exception level
92: #define GetCurrentEL(value)             asm volatile ("mrs %0, CurrentEL" : "=r" (value))
93:
94: /// @brief Get Saved Program Status Register (EL1)
95: #define GetSPSR_EL1(value)              asm volatile ("mrs %0, SPSR_EL1" : "=r" (value))
96:
97: /// @brief Get Exception Syndrome Register (EL1)
98: #define GetESR_EL1(value)               asm volatile ("mrs %0, ESR_EL1" : "=r" (value))
99:
100: /// @brief SPSR_EL1 M[3:0] field bit shift
101: #define SPSR_EL1_M30_SHIFT 0
102: /// @brief SPSR_EL1 M[3:0] field bit mask
103: #define SPSR_EL1_M30_MASK BITS(0,3)
104: /// @brief SPSR_EL1 M[3:0] field exctraction
105: #define SPSR_EL1_M30(value) ((value >> SPSR_EL1_M30_SHIFT) & SPSR_EL1_M30_MASK)
106: /// @brief SPSR_EL1 M[3:0] field value for EL0t mode
107: #define SPSR_EL1_M30_EL0t 0x0
108: /// @brief SPSR_EL1 M[3:0] field value for EL1t mode
109: #define SPSR_EL1_M30_EL1t 0x4
110: /// @brief SPSR_EL1 M[3:0] field value for EL1h mode
111: #define SPSR_EL1_M30_EL1h 0x5
112:
113: /// @brief ESR_EL1 EC field bit shift
114: #define ESR_EL1_EC_SHIFT 26
115: /// @brief ESR_EL1 EC field bit mask
116: #define ESR_EL1_EC_MASK  BITS(0,5)
117: /// @brief ESR_EL1 EC field extraction
118: #define ESR_EL1_EC(value) ((value >> ESR_EL1_EC_SHIFT) & ESR_EL1_EC_MASK)
119: /// @brief ESR_EL1 EC field value for unknown exception
120: #define ESR_EL1_EC_UNKNOWN 0x00
121: /// @brief ESR_EL1 EC field value for trapped WF<x> instruction exception
122: #define ESR_EL1_EC_TRAPPED_WFx_INSTRUCTION 0x01
123: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0F
124: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0F 0x03
125: /// @brief ESR_EL1 EC field value for MCRR or MRRC instruction exception when coproc = 0x0F
126: #define ESR_EL1_EC_TRAPPED_MCRR_MRRC_ACCESS_CORPROC_0F 0x04
127: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0E
128: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0E 0x05
129: /// @brief ESR_EL1 EC field value for trapped LDC or STC instruction exception
130: #define ESR_EL1_EC_TRAPPED_LDC_STC_ACCESS 0x06
131: /// @brief ESR_EL1 EC field value for unknown SME, SVE, SIMD or Floating pointer instruction exception
132: #define ESR_EL1_EC_TRAPPED_SME_SVE_SIMD_FP_ACCESS 0x07
133: /// @brief ESR_EL1 EC field value for trapped LD64<x> or ST64<x> instruction exception
134: #define ESR_EL1_EC_TRAPPED_LD64x_ST64x_ACCESS 0x0A
135: /// @brief ESR_EL1 EC field value for trapped MRRC instruction exception when coproc = 0x0C
136: #define ESR_EL1_EC_TRAPPED_MRRC_ACCESS_COPROC_0E 0x0C
137: /// @brief ESR_EL1 EC field value for branch target exception
138: #define ESR_EL1_EC_BRANCH_TARGET_EXCEPTION 0x0D
139: /// @brief ESR_EL1 EC field value for illegal executions state exception
140: #define ESR_EL1_EC_ILLEGAL_EXECUTION_STATE 0x0E
141: /// @brief ESR_EL1 EC field value for trapped SVC 32 bit instruction exception
142: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_32 0x11
143: /// @brief ESR_EL1 EC field value for trapped SVC 64 bit instruction exception
144: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_64 0x15
145: /// @brief ESR_EL1 EC field value for MCR or MRC 64 bit instruction exception
146: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_64 0x18
147: /// @brief ESR_EL1 EC field value for trapped SVE access exception
148: #define ESR_EL1_EC_TRAPPED_SVE_ACCESS 0x19
149: /// @brief ESR_EL1 EC field value for trapped TStart access exception
150: #define ESR_EL1_EC_TRAPPED_TSTART_ACCESS 0x1B
151: /// @brief ESR_EL1 EC field value for pointer authentication failure exception
152: #define ESR_EL1_EC_POINTER_AUTHENTICATION_FAILURE 0x1C
153: /// @brief ESR_EL1 EC field value for trapped SME access exception
154: #define ESR_EL1_EC_TRAPPED_SME_ACCESS 0x1D
155: /// @brief ESR_EL1 EC field value for granule protection check exception
156: #define ESR_EL1_EC_GRANULE_PROTECTION_CHECK 0x1E
157: /// @brief ESR_EL1 EC field value for instruction abort from lower EL exception
158: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL 0x20
159: /// @brief ESR_EL1 EC field value for instruction abort from same EL exception
160: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_SAME_EL 0x21
161: /// @brief ESR_EL1 EC field value for PC alignment fault exception
162: #define ESR_EL1_EC_PC_ALIGNMENT_FAULT 0x22
163: /// @brief ESR_EL1 EC field value for data abort from lower EL exception
164: #define ESR_EL1_EC_DATA_ABORT_FROM_LOWER_EL 0x24
165: /// @brief ESR_EL1 EC field value for data abort from same EL exception
166: #define ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL 0x25
167: /// @brief ESR_EL1 EC field value for SP alignment fault exception
168: #define ESR_EL1_EC_SP_ALIGNMENT_FAULT 0x27
169: /// @brief ESR_EL1 EC field value for trapped 32 bit FP instruction exception
170: #define ESR_EL1_EC_TRAPPED_FP_32 0x28
171: /// @brief ESR_EL1 EC field value for trapped 64 bit FP instruction exception
172: #define ESR_EL1_EC_TRAPPED_FP_64 0x2C
173: /// @brief ESR_EL1 EC field value for SError interrupt exception
174: #define ESR_EL1_EC_SERROR_INTERRUPT 0x2F
175: /// @brief ESR_EL1 EC field value for Breakpoint from lower EL exception
176: #define ESR_EL1_EC_BREAKPOINT_FROM_LOWER_EL 0x30
177: /// @brief ESR_EL1 EC field value for Breakpoint from same EL exception
178: #define ESR_EL1_EC_BREAKPOINT_FROM_SAME_EL 0x31
179: /// @brief ESR_EL1 EC field value for SW step from lower EL exception
180: #define ESR_EL1_EC_SW_STEP_FROM_LOWER_EL 0x32
181: /// @brief ESR_EL1 EC field value for SW step from same EL exception
182: #define ESR_EL1_EC_SW_STEP_FROM_SAME_EL 0x33
183: /// @brief ESR_EL1 EC field value for Watchpoint from lower EL exception
184: #define ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL 0x34
185: /// @brief ESR_EL1 EC field value for Watchpoint from same EL exception
186: #define ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL 0x35
187: /// @brief ESR_EL1 EC field value for 32 bit BKPT instruction exception
188: #define ESR_EL1_EC_BKPT_32 0x38
189: /// @brief ESR_EL1 EC field value for 64 bit BRK instruction exception
190: #define ESR_EL1_EC_BRK_64 0x3C
191:
192: /// @brief ESR_EL1 ISS field bit shift
193: #define ESR_EL1_ISS_SHIFT 0
194: /// @brief ESR_EL1 ISS field bit mask
195: #define ESR_EL1_ISS_MASK  BITS(0,24)
196: /// @brief ESR_EL1 ISS field extraction
197: #define ESR_EL1_ISS(value) ((value >> ESR_EL1_ISS_SHIFT) & ESR_EL1_ISS_MASK)
```

- Line 42: We need to include the macro header for a new definition we're going to use
- Line 92: We add an instruction to retrieve the value of the `CurrentEL` register, which holds the current exception level
- Line 95: We add an instruction to retrieve the value of the `SPR_EL1` (Saved Program Status Register EL1), which hold information on the cause of an exception
- Line 98: We add an instruction to retrieve the value of the `ESR_EL1` (Exception Status Register EL1), which hold information on the cause of an exception
- Line 100-105: We define the M3:0 field in the `SPSR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 106-111: We define different values for this field
- Line 113-119: We define the EC field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 120-190: We define different values for this field
- Line 192-197: We define the ISS field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)

### Exception.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONH}

We will define some values for different types of exceptions.

Create the file `code/libraries/baremetal/include/baremetal/Exception.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Exception.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Exception.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Exception definitions
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
42: /// @file
43: /// Exception types
44:
45: /// @brief Unexpected exception
46: #define EXCEPTION_UNEXPECTED                0
47: /// @brief Synchronous exception
48: #define EXCEPTION_SYNCHRONOUS               1
49: /// @brief System error
50: #define EXCEPTION_SYSTEM_ERROR              2
51:
```

### ExceptionHandler.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONHANDLERH}

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
45: /// Exception handler function
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
73: /// @brief Just specifies the struct is packed
74: PACKED;
75:
76: /// @brief Handles an unexpected exception, with the abort frame passed in.
77: ///
78: /// The exception handler is called from assembly code (ExceptionStub.S)
79: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_UNEXPECTED)
80: /// @param abortFrame  Filled in AbortFrame instance.
81: void UnexpectedHandler(uint64 exceptionID, AbortFrame* abortFrame);
82:
83: /// @brief Handles a synchronous exception, with the abort frame passed in.
84: ///
85: /// The exception handler is called from assembly code (ExceptionStub.S)
86: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_SYNCHRONOUS)
87: /// @param abortFrame  Filled in AbortFrame instance.
88: void SynchronousExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);
89:
90: /// @brief Handles a system error exception, with the abort frame passed in.
91: ///
92: /// The exception handler is called from assembly code (ExceptionStub.S)
93: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_SYSTEM_ERROR)
94: /// @param abortFrame  Filled in AbortFrame instance.
95: void SystemErrorHandler(uint64 exceptionID, AbortFrame* abortFrame);
96:
97: /// @brief Handles an interrupt.
98: ///
99: /// The interrupt handler is called from assembly code (ExceptionStub.S)
100: void InterruptHandler();
101:
102: #ifdef __cplusplus
103: }
104: #endif
```

- Line 54-74: We create struct `AbortFrame` which will hold all important information on entry. This information is actually registers saved on the stack, as we'll see later
- Line 81: We declare the exception handler for an unexpected exception `UnexpectedHandler()` which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_UNEXPECTED), the second is a pointer to the `AbortFrame` which is actually the stack pointer value
- Line 88: We declare the exception handler for a synchronous exception (e.g. data exception, illegal instruction exception,break exception) `SynchronousExceptionHandler()` which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_SYNCHRONOUS), the second is a pointer to the `AbortFrame` which is actually the stack pointer value
- Line 95: We declare the exception handler for a system error exception `SystemErrorHandler()` (implementation defined) which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_SYSTEM_ERROR), the second is a pointer to the `AbortFrame` which is actually the stack pointer value
- Line 100: We declare the interrupt handler `InterruptHandler()`. This will later be moved, it is for now just added to avoid linker errors

### ExceptionHandler.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONHANDLERCPP}

We will implement the exception handlers.

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
43: #include <baremetal/Logger.h>
44:
45: /// @file
46: /// Exception handler function implementation
47:
48: /// @brief Define log name
49: LOG_MODULE("ExceptionHandler");
50:
51: using namespace baremetal;
52:
53: void UnexpectedHandler(uint64 exceptionID, AbortFrame* abortFrame)
54: {
55:     LOG_INFO("UnexpectedHandler");
56: }
57:
58: void SynchronousExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
59: {
60:     uint32 esr_el1{};
61:     GetESR_EL1(esr_el1);
62:     uint32 esr_ec = ESR_EL1_EC(esr_el1);
63:     uint32 esr_iss = ESR_EL1_ISS(esr_el1);
64:     LOG_INFO("SynchronousHandler EC=%02x ISS=%07x", esr_ec, esr_iss);
65: }
66:
67: void SystemErrorHandler(uint64 exceptionID, AbortFrame* abortFrame)
68: {
69:     LOG_INFO("SystemErrorHandler");
70: }
71:
72: void InterruptHandler()
73: {
74: }
```

- Line 53-56: We implement the function `UnexpectedHandler()`, which simply prints some text
- Line 58-65: We implement the function `SynchronousExceptionHandler()`, which extracts two fields from the `ESR_EL1` register (using the macros and instructions we just added), and prints the values.
- Line 67-70: We implement the function `SystemErrorHandler()`, which simply prints some text
- Line 72-74: We implement the function `InterruptHandler()`, which does nothing

### BCMRegisters.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_BCMREGISTERSH}

We are using a new macro `BITS` in the previous file, which we'll need to define.

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
47: #include <baremetal/Macros.h>
48: #ifdef __cplusplus
49: #include <baremetal/Types.h>
50: #endif
...
96: //---------------------------------------------
97: // Interrupt Controller
98: //---------------------------------------------
99:
100: /// @brief Raspberry Pi Interrupt Control Registers base address. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
101: #define RPI_INTRCTRL_BASE               RPI_BCM_IO_BASE + 0x0000B000
102:
103: /// @brief Raspberry Pi Interrupt Control basic IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
104: #define RPI_INTRCTRL_IRQ_BASIC_PENDING  reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000200)
105: /// @brief Raspberry Pi Interrupt Control register 1 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
106: #define RPI_INTRCTRL_IRQ_PENDING_1      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000204)
107: /// @brief Raspberry Pi Interrupt Control register 2 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
108: #define RPI_INTRCTRL_IRQ_PENDING_2      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000208)
109: /// @brief Raspberry Pi Interrupt Control FIQ enable register. See @ref RASPBERRY_PI_MAILBOX
110: #ifdef __cplusplus
111: #define RPI_INTRCTRL_FIQ_CONTROL        reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000020C)
112: #else
113: #define RPI_INTRCTRL_FIQ_CONTROL        (RPI_MAILBOX_BASE + 0x0000020C)
114: #endif
115: /// @brief Raspberry Pi Interrupt Control register 1 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
116: #define RPI_INTRCTRL_ENABLE_IRQS_1      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000210)
117: /// @brief Raspberry Pi Interrupt Control register 2 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
118: #define RPI_INTRCTRL_ENABLE_IRQS_2      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000214)
119: /// @brief Raspberry Pi Interrupt Control basic IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
120: #define RPI_INTRCTRL_ENABLE_BASIC_IRQS  reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000218)
121: /// @brief Raspberry Pi Interrupt Control register 1 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
122: #define RPI_INTRCTRL_DISABLE_IRQS_1     reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000021C)
123: /// @brief Raspberry Pi Interrupt Control register 2 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
124: #define RPI_INTRCTRL_DISABLE_IRQS_2     reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000220)
125: /// @brief Raspberry Pi Interrupt Control basic IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
126: #define RPI_INTRCTRL_DISABLE_BASIC_IRQS reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000224)
127:
...
```

- Line 48-50: As this header is also included by the exception stub comming up next, we need to protect against specific C/C++ definitions, so we only include the type definitions header if we are building for C++
- Line 100-126: We add definitions for registers relating to the Raspberry Pi interrupt control register. Some of these are used by the exception stub code

### ExceptionStub.S {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_EXCEPTIONSTUBS}

Next we need to write some assembly to implement the exception vectors.

Create the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExceptionStub.S
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Exception stub. This defines the functions to be called in case a processor exception is thown.
11: //               Note: this file is based on the Circle startup assembly file by Rene Stange.
12: //
13: //------------------------------------------------------------------------------
14: //
15: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
16: //
17: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
18: //
19: // Permission is hereby granted, free of charge, to any person
20: // obtaining a copy of this software and associated documentation
21: // files(the "Software"), to deal in the Software without
22: // restriction, including without limitation the rights to use, copy,
23: // modify, merge, publish, distribute, sublicense, and /or sell copies
24: // of the Software, and to permit persons to whom the Software is
25: // furnished to do so, subject to the following conditions :
26: //
27: // The above copyright notice and this permission notice shall be
28: // included in all copies or substantial portions of the Software.
29: //
30: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
31: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
32: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
33: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
34: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
35: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
36: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
37: // DEALINGS IN THE SOFTWARE.
38: //
39: //------------------------------------------------------------------------------
40:
41: #include <baremetal/BCMRegisters.h>
42: #include <baremetal/Exception.h>
43: #include <baremetal/SysConfig.h>
44:
45: /// @file
46: /// Exception stub assembly code
47:
48: .macro	vector handler
49:
50:     .align	7
51:
52:     b \handler                          // Jump to handler
53:
54: .endm
55:
56: .macro  stub name, exception, handler
57:
58:     .globl  \name
59: \name:
60:     mrs     x0, esr_el1                 // Read Exception Syndrome Register (EL1)
61:     mrs     x1, spsr_el1                // Read Saved Program Status Register (EL1)
62:     mov     x2, x30                     // X30=Link Register(LR)
63:     mrs     x3, elr_el1                 // Read Exception Link Register (EL1)
64:     mrs     x4, sp_el0                  // Read Stack Pointer (EL0)
65:     mov     x5, sp                      // Get Stack Pointer (EL1)
66:     mrs     x6, far_el1                 // Fault Address Register (EL1)
67:
68:     str     x6, [sp, #-16]!             // Store X6 on stack, decrement stack
69:     stp     x4, x5, [sp, #-16]!         // Store X4,X5 on stack, decrement stack
70:     stp     x2, x3, [sp, #-16]!         // Store X2,X3 on stack, decrement stack
71:     stp     x0, x1, [sp, #-16]!         // Store X0,X1 on stack, decrement stack
72:
73:     mov     x0, #\exception             // Get exception function pointer
74:     mov     x1, sp                      // Save stack pointer
75:     b       \handler                    // Calls handler(uint64 exceptionID, AbortFrame* abortFrame). x0 = exception is exceptionID, x1 = sp = abortFrame. Never returns
76:
77: .endm
78:
79:     .text
80:
81:     .align  11
82:
83:     .globl  VectorTable
84: VectorTable:
85:
86:     // from current EL with sp_el0 (mode EL0, El1t, El2t, EL3t)
87:     vector  SynchronousStub
88:     vector  UnexpectedStub
89:     vector  UnexpectedStub
90:     vector  SErrorStub
91:
92:     // from current EL with sp_elx, x != 0  (mode El1h, El2h, EL3h)
93:     vector  SynchronousStub
94:     vector  UnexpectedStub
95:     vector  UnexpectedStub
96:     vector  SErrorStub
97:
98:     // from lower EL, target EL minus 1 is AArch64
99:     vector  HVCStub
100:     vector  UnexpectedStub
101:     vector  UnexpectedStub
102:     vector  UnexpectedStub
103:
104:     // from lower EL, target EL minus 1 is AArch32
105:     vector  UnexpectedStub
106:     vector  UnexpectedStub
107:     vector  UnexpectedStub
108:     vector  UnexpectedStub
109:
110: //*************************************************
111: // IRQ stub
112: //*************************************************
113: .macro irq_stub name, handler
114:     .globl  \name
115: \name:
116:     stp     x29, x30, [sp, #-16]!       // Save x29, x30 onto stack
117:     mrs     x29, elr_el1                // Read Exception Link Register (EL1)
118:     mrs     x30, spsr_el1               // Read Saved Program Status Register (EL1)
119:     stp     x29, x30, [sp, #-16]!       // Save onto stack
120:     msr     DAIFClr, #1                 // Enable FIQ
121:
122: #ifdef SAVE_VFP_REGS_ON_IRQ
123:     stp     q30, q31, [sp, #-32]!       // Save q0-q31 onto stack
124:     stp     q28, q29, [sp, #-32]!
125:     stp     q26, q27, [sp, #-32]!
126:     stp     q24, q25, [sp, #-32]!
127:     stp     q22, q23, [sp, #-32]!
128:     stp     q20, q21, [sp, #-32]!
129:     stp     q18, q19, [sp, #-32]!
130:     stp     q16, q17, [sp, #-32]!
131:     stp     q14, q15, [sp, #-32]!
132:     stp     q12, q13, [sp, #-32]!
133:     stp     q10, q11, [sp, #-32]!
134:     stp     q8, q9, [sp, #-32]!
135:     stp     q6, q7, [sp, #-32]!
136:     stp     q4, q5, [sp, #-32]!
137:     stp     q2, q3, [sp, #-32]!
138:     stp     q0, q1, [sp, #-32]!
139: #endif
140:     stp     x27, x28, [sp, #-16]!       // Save x0-x28 onto stack
141:     stp     x25, x26, [sp, #-16]!
142:     stp     x23, x24, [sp, #-16]!
143:     stp     x21, x22, [sp, #-16]!
144:     stp     x19, x20, [sp, #-16]!
145:     stp     x17, x18, [sp, #-16]!
146:     stp     x15, x16, [sp, #-16]!
147:     stp     x13, x14, [sp, #-16]!
148:     stp     x11, x12, [sp, #-16]!
149:     stp     x9, x10, [sp, #-16]!
150:     stp     x7, x8, [sp, #-16]!
151:     stp     x5, x6, [sp, #-16]!
152:     stp     x3, x4, [sp, #-16]!
153:     stp     x1, x2, [sp, #-16]!
154:     str     x0, [sp, #-16]!
155:
156:     ldr     x0, =IRQReturnAddress       // Store return address for profiling
157:     str     x29, [x0]
158:
159:     bl      \handler                    // Call interrupt handler
160:
161:     ldr     x0, [sp], #16               // Restore x0-x28 from stack
162:     ldp     x1, x2, [sp], #16
163:     ldp     x3, x4, [sp], #16
164:     ldp     x5, x6, [sp], #16
165:     ldp     x7, x8, [sp], #16
166:     ldp     x9, x10, [sp], #16
167:     ldp     x11, x12, [sp], #16
168:     ldp     x13, x14, [sp], #16
169:     ldp     x15, x16, [sp], #16
170:     ldp     x17, x18, [sp], #16
171:     ldp     x19, x20, [sp], #16
172:     ldp     x21, x22, [sp], #16
173:     ldp     x23, x24, [sp], #16
174:     ldp     x25, x26, [sp], #16
175:     ldp     x27, x28, [sp], #16
176: #ifdef SAVE_VFP_REGS_ON_IRQ
177:     ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
178:     ldp     q2, q3, [sp], #32
179:     ldp     q4, q5, [sp], #32
180:     ldp     q6, q7, [sp], #32
181:     ldp     q8, q9, [sp], #32
182:     ldp     q10, q11, [sp], #32
183:     ldp     q12, q13, [sp], #32
184:     ldp     q14, q15, [sp], #32
185:     ldp     q16, q17, [sp], #32
186:     ldp     q18, q19, [sp], #32
187:     ldp     q20, q21, [sp], #32
188:     ldp     q22, q23, [sp], #32
189:     ldp     q24, q25, [sp], #32
190:     ldp     q26, q27, [sp], #32
191:     ldp     q28, q29, [sp], #32
192:     ldp     q30, q31, [sp], #32
193: #endif
194:
195:     msr     DAIFSet, #1                 // Disable FIQ
196:     ldp     x29, x30, [sp], #16         // Restore from stack
197:     msr     elr_el1, x29                // Restore Exception Link Register (EL1)
198:     msr     spsr_el1, x30               // Restore Saved Program Status Register (EL1)
199:     ldp     x29, x30, [sp], #16         // restore x29, x30 from stack
200:
201:     eret                                // Restore previous EL
202:
203: .endm
204:
205: //*************************************************
206: // FIQ stub
207: //*************************************************
208:     .globl  FIQStub
209: FIQStub:
210: #ifdef SAVE_VFP_REGS_ON_FIQ
211:     stp     q30, q31, [sp, #-32]!       // Save q0-q31 onto stack
212:     stp     q28, q29, [sp, #-32]!
213:     stp     q26, q27, [sp, #-32]!
214:     stp     q24, q25, [sp, #-32]!
215:     stp     q22, q23, [sp, #-32]!
216:     stp     q20, q21, [sp, #-32]!
217:     stp     q18, q19, [sp, #-32]!
218:     stp     q16, q17, [sp, #-32]!
219:     stp     q14, q15, [sp, #-32]!
220:     stp     q12, q13, [sp, #-32]!
221:     stp     q10, q11, [sp, #-32]!
222:     stp     q8, q9, [sp, #-32]!
223:     stp     q6, q7, [sp, #-32]!
224:     stp     q4, q5, [sp, #-32]!
225:     stp     q2, q3, [sp, #-32]!
226:     stp     q0, q1, [sp, #-32]!
227: #endif
228:     stp     x29, x30, [sp, #-16]!       // Save x0-x28 onto stack
229:     stp     x27, x28, [sp, #-16]!
230:     stp     x25, x26, [sp, #-16]!
231:     stp     x23, x24, [sp, #-16]!
232:     stp     x21, x22, [sp, #-16]!
233:     stp     x19, x20, [sp, #-16]!
234:     stp     x17, x18, [sp, #-16]!
235:     stp     x15, x16, [sp, #-16]!
236:     stp     x13, x14, [sp, #-16]!
237:     stp     x11, x12, [sp, #-16]!
238:     stp     x9, x10, [sp, #-16]!
239:     stp     x7, x8, [sp, #-16]!
240:     stp     x5, x6, [sp, #-16]!
241:     stp     x3, x4, [sp, #-16]!
242:     stp     x1, x2, [sp, #-16]!
243:     str     x0, [sp, #-16]!
244:
245:     ldr     x2, =s_fiqData
246:     ldr     x1, [x2]                    // Get s_fiqData.handler
247:     cmp     x1, #0                      // Is handler set?
248:     b.eq    no_fiq_handler
249:     ldr     x0, [x2, #8]                // Get s_fiqData.param
250:     blr     x1                          // Call handler
251:
252: restore_after_fiq_handler:
253:     ldr     x0, [sp], #16               // Restore x0-x28 from stack
254:     ldp     x1, x2, [sp], #16
255:     ldp     x3, x4, [sp], #16
256:     ldp     x5, x6, [sp], #16
257:     ldp     x7, x8, [sp], #16
258:     ldp     x9, x10, [sp], #16
259:     ldp     x11, x12, [sp], #16
260:     ldp     x13, x14, [sp], #16
261:     ldp     x15, x16, [sp], #16
262:     ldp     x17, x18, [sp], #16
263:     ldp     x19, x20, [sp], #16
264:     ldp     x21, x22, [sp], #16
265:     ldp     x23, x24, [sp], #16
266:     ldp     x25, x26, [sp], #16
267:     ldp     x27, x28, [sp], #16
268:     ldp     x29, x30, [sp], #16
269: #ifdef SAVE_VFP_REGS_ON_FIQ
270:     ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
271:     ldp     q2, q3, [sp], #32
272:     ldp     q4, q5, [sp], #32
273:     ldp     q6, q7, [sp], #32
274:     ldp     q8, q9, [sp], #32
275:     ldp     q10, q11, [sp], #32
276:     ldp     q12, q13, [sp], #32
277:     ldp     q14, q15, [sp], #32
278:     ldp     q16, q17, [sp], #32
279:     ldp     q18, q19, [sp], #32
280:     ldp     q20, q21, [sp], #32
281:     ldp     q22, q23, [sp], #32
282:     ldp     q24, q25, [sp], #32
283:     ldp     q26, q27, [sp], #32
284:     ldp     q28, q29, [sp], #32
285:     ldp     q30, q31, [sp], #32
286: #endif
287:
288:     eret                                // Restore previous EL
289:
290: no_fiq_handler:
291:     ldr     x1, =RPI_INTRCTRL_FIQ_CONTROL // Disable FIQ (if handler is not set)
292:     mov     w0, #0
293:     str     w0, [x1]
294:     b       restore_after_fiq_handler
295:
296: #if BAREMETAL_RPI_TARGET >= 4
297:
298: /*
299:  * SMC stub
300:  */
301: .macro smc_stub name, handler
302:     .globl  \name
303: \name:
304:     ldr     x2, =SMCStack
305:     mov     sp, x2                      // Set SMC stack
306:     str     x30, [sp, #-16]!            // Save x30 (LR) on stack
307:     bl      \handler                    // Call handler
308:     ldr     x30, [sp], #16              // Restore x30 (LR) from stack
309:     eret
310:
311: .endm
312:
313: #endif
314:
315: /*
316:  * HVC stub
317:  */
318: HVCStub:                                // Return to EL2h mode
319:     mrs     x0, spsr_el2                // Read Saved Program Status Register (EL2)
320:     bic     x0, x0, #0xF                // Clear bit 3:0
321:     mov     x1, #9
322:     orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
323:     msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
324:     eret                                // Move to EL2h
325:
326:     .data
327:
328:     .align  3
329:
330:     .globl  s_fiqData
331: s_fiqData:                              // Matches FIQData:
332:     .quad   0                           // handler
333:     .quad   0                           // param
334:     .word   0                           // fiqID (unused)
335:
336:     .align  3
337:
338:     .globl  IRQReturnAddress
339: IRQReturnAddress:
340:     .quad   0
341:
342: #if BAREMETAL_RPI_TARGET >= 4
343:
344:     .bss
345:
346:     .align  4
347:
348:     .space  128
349: SMCStack:
350:
351: #endif
352:
353: //*************************************************
354: // Abort stubs
355: //*************************************************
356:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   UnexpectedHandler
357:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  SynchronousExceptionHandler
358:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, SystemErrorHandler
359:     irq_stub    IRQStub,                                    InterruptHandler
360: #if BAREMETAL_RPI_TARGET >= 4
361:     smc_stub    SMCStub,                                    SecureMonitorHandler
362: #endif
363:
364: // End
```

- Line 48-54: We create a macro `vector` to define a vector in the vector table.
Every vector is aligned to 128 bytes, hence the statement `.align 7` which align by 7 bits, or 128 (1 << 7) bytes.
The only code we add is a jump to the current stub, passed as `handler`
- Line 56-77: We create a macro `stub` for a generic stub, which saves the system registers `ESR_EL11`, `SPSR_EL1`, `ELR_EL1`, `SP_EL0` and `FAR_EL1` and the registers `x30` (LR) and `SP`, in general purpose registers x0-x6.
These are then stored on the stack, every time decreasing the stack pointer by 16 bytes.
Then the exception type passed through `exception` is stored in x0, and the stack pointer in x1.
Finally the exception handler passed through `handler` is branched to.
This will effectively call the exception handler function `void handler(uint64 exceptionID, AbortFrame* abortFrame)` with `exceptionID` = x0, `abortFrame` = x1
- Line 79-108: We implement the actual exception vector table.
This has 16 entries using the macro `vector`, each aligned to 128 bytes, while the complete table is aligned to 2048 (1 << 11) bytes, hence `.align 11`
- Line 113-193: We create a macro `irq_stub` for an interrupt stub, which saves almost all registers, depending on the define `SAVE_VFP_REGS_ON_IRQ`.
These are then stored on the stack, every time decreasing the stack pointer by 32 or 16 bytes, depending on the registers.
The FIQ interrupts are enabled while the interrupt is being handled.
After saving the registers the interrupt handler passed through `handler` is called, and after than the registers are restored.
Then FIQ interrupts are disabled again, and the stub returns to the state before the interrupt using `eret`.
This also reset the state of interrupt enables, etc. as well as the exception level
- Line 208-288: We implement the FIQ interrupt stub `FIQStub`. This is similar to the normal interrupt stub, however this is not a macro.
Also, the pointer to the handler is retrieved from a memory area structure `s_fiqData`, as well as the parameter to pass to the handler.
If no handler was set no function is called, and the registers are simply restored
- Line 301-311: For Raspberry Pi 4 and later, we create a macro `smc_stub` for an secure monitor call stub, which saves the link register `x30` on its own stack `SMCStack` and calls the handler passed through `handler`.
After the call returns, the linker register is restored, and the stub returns to the state before the interrupt using `eret`.
This also reset the state of interrupt enables, etc. as well as the exception level
- Line 318-324: We implement the hypervisor call stub `HVCStub`. This is similar to the SMC stub, however this is not a macro.
The function reads the `SPSR_EL2` system register, and sets it to switch to EL2h exception level, then moves to that that exception level using `eret`.
This also reset the state of interrupt enables, etc.
- Line 328-334: We define the `s_fiqData` structure, aligned to 8 bytes
- Line 336-338: We define an address for profiling of interrupts. We'll get to that later. THis is also 8 bytes aligned
- Line 344-349: For Raspberry Pi 4 and later, we define the stack for the supervisor monitor call `SMCStack`. Note that the stack pointer is set at the bottom of the stack
- Line 356: We declare unexpected exceptions stub using the `stub` macro, and set its handler to `UnexpectedHandler()`
- Line 357: We declare synchronous exceptions stub using the `stub` macro, and set its handler to `SynchronousExceptionHandler()`
- Line 358: We declare system errors stub using the `stub` macro, and set its handler to `SystemErrorHandler()`
- Line 359: We declare interrupt stub using the `irq_stub` macro, and set its handler to `InterruptHandler()`
- Line 361: For Raspberry Pi 4 and later, we declare SMC stub using the `smc_stub` macro, and set its handler to `SecureMonitorHandler()`

Not that this means we need to defined the functions `UnexpectedHandler()`, `SynchronousExceptionHandler()`, `SystemErrorHandler()`, `InterruptHandler()` and for RaspberryPi 4 and later also `SecureMonitorHandler()`, like we did in the exception handler implementation

### System.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_SYSTEMH}

We'll add a function to extract the current exception level, to be printer for information.

Update the file `code/libraries/baremetal/include/baremetal/System.h`

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
...
51: /// <summary>
52: /// Determine current exception level. See also \ref ARM_REGISTERS_REGISTER_OVERVIEW_CURRENTEL_REGISTER
53: /// </summary>
54: /// <returns>Current exception level (0..3)</returns>
55: extern uint8 CurrentEL();
...
```

### System.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_SYSTEMCPP}

We'll implement the newly added `CurrentEL()` function.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
90: uint8 baremetal::CurrentEL()
91: {
92:     uint32 registerValue{};
93:     GetCurrentEL(registerValue);
94:     return (registerValue & 0x0000000C) >> 2;
95: }
...
```

This function used the assembly macro `GetCurrentEL` to retrieve the `CurrentEL` system register value, and extracts the bits 2..3 which hold the exception level.

### Update application code {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_UPDATE_APPLICATION_CODE}

Ok, so now we creation the infrastructure for dealing with processor exceptions, let's see whether they work.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: #include <baremetal/ExceptionHandler.h>
7: #include <baremetal/BCMRegisters.h>
8:
9: #include <unittest/unittest.h>
10:
11: LOG_MODULE("main");
12:
13: using namespace baremetal;
14:
15: int main()
16: {
17:     auto& console = GetConsole();
18:
19:     auto exceptionLevel = CurrentEL();
20:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
21:
22:     console.Write("Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation\n");
23:     char ch{};
24:     while ((ch != 'r') && (ch != 'h') && (ch != 't') && (ch != 'm'))
25:     {
26:         ch = console.ReadChar();
27:         console.WriteChar(ch);
28:     }
29:     if (ch == 't')
30:         // Trap
31:         __builtin_trap();
32:     else if (ch == 'm')
33:     {
34:         // Memory failure
35:         auto r = *((volatile unsigned int*)0xFFFFFFFFFF000000);
36:         // make gcc happy about unused variables :-)
37:         r++;
38:     }
39:
40:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
41: }
```

- Line 29-31: If we press `t`, we cause a standard system trap, which results in a debug break
- Line 33-38: If we press 'm', we cause a data abort due to reading a non-existent memory location

### Update project configuration {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_UPDATE_TEST_PROJECT_CONFIGURATION}

As we added a new source file, we'll update the project CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionStub.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
53:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
55:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
56:     )
57:
58: set(PROJECT_INCLUDES_PUBLIC
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
91:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
92:     )
93: set(PROJECT_INCLUDES_PRIVATE )
```

### baremetal.ld {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_BAREMETALLD}

We need to update the linker definition file to make sure the exception handling frame is defined.

Update the file `baremetal.ld`

```cmake
File: baremetal.ld
...
87:     /* Exception handling data */
88:     .eh_frame : {
89:         *(.eh_frame*)
90:     } : data
91:
...
```

### Configuring, building and debugging {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:20)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tInfo   SynchronousHandler EC=3C ISS=00003E8 (ExceptionHandler:63)
Info   SynchronousHandler EC=21 ISS=0000000 (ExceptionHandler:63)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:63)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:63)
```

If we cause a memory violation, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:20)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mInfo   SynchronousHandler EC=25 ISS=0000000 (ExceptionHandler:63)
Info   SynchronousHandler EC=21 ISS=0000000 (ExceptionHandler:63)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:63)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:63)
```

As can be found in [Arm� Architecture Registers](pdf/arm-architecture-registers.pdf), page 570, exception code 3C stands for "BRK instruction execution in AArch64 state", and on page 571, exception code 25 stands for "Data Abort taken without a change in Exception level."

## Exception system - Step 2 {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2}

To improve our code a little, let's create a class for handling exceptions.

### ExceptionHandler.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2_EXCEPTIONHANDLERH}

We'll add the class `ExceptionSystem` and change the exception handling to a single function `ExceptionHandler()`.

Update the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
...
42: #include <baremetal/Types.h>
43: #include <baremetal/System.h>
...
77: /// @brief Handles an exception, with the abort frame passed in.
78: ///
79: /// The exception handler is called from assembly code (ExceptionStub.S)
80: /// @param exceptionID Exception type being thrown (one of EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS, EXCEPTION_SYSTEM_ERROR)
81: /// @param abortFrame  Filled in AbortFrame instance.
82: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);
83: 
84: /// @brief Handles an interrupt.
85: ///
86: /// The interrupt handler is called from assembly code (ExceptionStub.S)
87: void InterruptHandler();
88: 
89: #ifdef __cplusplus
90: }
91: #endif
92: 
93: namespace baremetal {
94: 
95: /// @brief Exception handling system. Handles ARM processor exceptions
96: /// This is a singleton class, created as soon as GetExceptionSystem() is called
97: class ExceptionSystem
98: {
99:     /// <summary>
100:     /// Construct the singleton exception system instance if needed, and return a reference to the instance. This is a friend function of class ExceptionSystem
101:     /// </summary>
102:     /// <returns>Reference to the singleton system instance</returns>
103:     friend ExceptionSystem& GetExceptionSystem();
104: 
105: private:
106:     ExceptionSystem();
107: 
108: public:
109:     ~ExceptionSystem();
110: 
111:     void Throw(unsigned exceptionID, AbortFrame* abortFrame);
112: };
113: 
114: /// <summary>
115: /// Injectable exception handler
116: /// </summary>
117: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
118: /// <param name="abortFrame">Stored state information at the time of exception</param>
119: /// <returns>ReturnCode::ExitHalt if the system should be halted, ReturnCode::ExitReboot if the system should reboot</returns>
120: using ExceptionPanicHandler = ReturnCode(unsigned exceptionID, AbortFrame* abortFrame);
121: 
122: const char* GetExceptionType(unsigned exceptionID);
123: 
124: /// <summary>
125: /// Register a panic handler
126: /// </summary>
127: /// <param name="handler">Exception panic handler</param>
128: /// <returns>The previously set handler</returns>
129: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler);
130: 
131: ExceptionSystem& GetExceptionSystem();
132: 
133: } // namespace baremetal
```

- Line 82: We change the three exception handlers to a single one, `ExceptionHandler()`
- Line 97-112: We declare the class `ExceptionSystem`
  - Line 103: We make the function `GetExceptionSystem()` a friend to the class, so we can use it to create the singleton instance
  - Line 106: We declare the private (default) constructor, so only the `GetExceptionSystem()` function can be used to create and instance
  - Line 109: We declare the destructor
  - Line 111: We declare the method `Throw()` which will be called by `ExceptionHandler()`
- Line 120: We declare the type for an injectable exception panic handler.
This will be call by the `Throw()` method of `ExceptionSystem` if set, and will return true if the system should be halted, false if not
- Line 122: We declare a helper function `GetExceptionType()` to convert an exception type into a string
- Line 129: We declare a method to set the exception panice handler, returning the old installed handler, if any
- Line 131: We declare the function `GetExceptionSystem()` that creates the singleton instance of the `ExceptionSystem` class, if needed

### ExceptionHandler.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2_EXCEPTIONHANDLERCPP}

We'll implement the newly added `ExceptionSystem` class and reimplement the ExceptionHandler() function.

Update the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
...
54: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
55: {
56:     baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
57: }
58: 
59: void InterruptHandler()
60: {
61: }
62: 
63: namespace baremetal {
64: 
65: /// <summary>
66: /// Exception panic handler
67: ///
68: /// If set, the panic handler is called first, and if it returns false, the system is not halted (as it would by default)
69: /// </summary>
70: static ExceptionPanicHandler* s_exceptionPanicHandler{};
71: 
72: /// <summary>
73: /// Names exception types
74: ///
75: /// Order must match exception identifiers in baremetal/Exception.h
76: /// </summary>
77: static const char* s_exceptionName[] =
78: {
79:     "Unexpected exception",
80:     "Synchronous exception",
81:     "System error"
82: };
83: 
84: /// <summary>
85: /// Constructor
86: ///
87: /// Note that the constructor is private, so GetExceptionSystem() is needed to instantiate the exception handling system
88: /// </summary>
89: ExceptionSystem::ExceptionSystem()
90: {
91: }
92: 
93: /// <summary>
94: /// Destructor
95: /// </summary>
96: ExceptionSystem::~ExceptionSystem()
97: {
98: }
99: 
100: /// <summary>
101: /// Throw an exception to the exception system
102: /// </summary>
103: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
104: /// <param name="abortFrame">Stored state information at the time of exception</param>
105: void ExceptionSystem::Throw(unsigned exceptionID, AbortFrame* abortFrame)
106: {
107:     assert(abortFrame != nullptr);
108: 
109:     uint64 sp = abortFrame->sp_el0;
110:     if (SPSR_EL1_M30(abortFrame->spsr_el1) == SPSR_EL1_M30_EL1h)		// EL1h mode?
111:     {
112:         sp = abortFrame->sp_el1;
113:     }
114: 
115:     uint64 EC = ESR_EL1_EC(abortFrame->esr_el1);
116:     uint64 ISS = ESR_EL1_ISS(abortFrame->esr_el1);
117: 
118:     uint64 FAR = 0;
119:     if ((ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL)
120:         || (ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL))
121:     {
122:         FAR = abortFrame->far_el1;
123:     }
124: 
125:     ReturnCode action = ReturnCode::ExitHalt;
126:     if (s_exceptionPanicHandler != nullptr)
127:     {
128:         action = (*s_exceptionPanicHandler)(exceptionID, abortFrame);
129:     }
130: 
131:     if (action == ReturnCode::ExitHalt)
132:         LOG_PANIC("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
133:             s_exceptionName[exceptionID],
134:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
135:     else
136:     {
137:         LOG_ERROR("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
138:             s_exceptionName[exceptionID],
139:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
140:         GetSystem().Reboot();
141:     }
142: }
143: 
144: /// @brief Convert exception ID to a string
145: /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
146: /// @return String representation of exception ID
147: const char* GetExceptionType(unsigned exceptionID)
148: {
149:     return s_exceptionName[exceptionID];
150: }
151: 
152: /// @brief Register an exception callback function, and return the previous one.
153: /// @param handler      Exception handler callback function to register
154: /// @return Previously set exception handler callback function
155: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler)
156: {
157:     auto result = s_exceptionPanicHandler;
158:     s_exceptionPanicHandler = handler;
159:     return result;
160: }
161: 
162: /// @brief Retrieve the singleton exception handling system
163: ///
164: /// Creates a static instance of ExceptionSystem, and returns a reference to it.
165: /// @return A reference to the singleton exception handling system.
166: ExceptionSystem& GetExceptionSystem()
167: {
168:     static ExceptionSystem system;
169:     return system;
170: }
171: 
172: } // namespace baremetal
```

- Line 54-57: We implement the function `ExceptionHandler()` by calling the `Throw()` method on the singleton `ExceptionSystem` instance
- Line 70: We define a static variable `s_exceptionPanicHandler` to hold the injected exception panic handler pointer.
It is initialized as nullptr, meaning not handler is installed
- Line 77-82: We define the mapping `s_exceptionName` from exception type to string
- Line 89-91: We implement the `ExceptionSystem` constructor
- Line 96-98: We implement the `ExceptionSystem` destructor
- Line 105-142: We implement the method `Throw()`
  - Line 109-113: We determine the stack pointer at the time of the exception, which is the EL0 stack pointer, unless the exception level was EL1h (We can only be in exception level EL0 or EL1, as we specifically move to EL1 in the startup code)
  - Line 115-116: We extract the EC and ISS fields from the ESR_EL1 register value
  - Line 119-123: If the exception has an exception code relating to an instruction abort, a data abort or a watchpoint, We get the value of the Fault Address Register, which holds the address which generated the exception
  - Line 125-129: We call the exception panic handler if installed, setting its return value as the return code. If no handler is installed, the default is to halt
  - Line 131-141: If we need to halt, we perform a panic log (which will halt the system), otherwise we log an error and reboot
- Line 147-150: We implement the function `GetExceptionType()`
- Line 155-160: We implement the function `RegisterPanicHandler()`, which sets the `s_exceptionPanicHandler` variable, and returns the original value of this variable
- Line 166-170: We implement the function `GetExceptionSystem()` in the by now common way

### ExceptionStub.S {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2_EXCEPTIONSTUBS}

We'll implement the newly added `CurrentEL()` function.

Update the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
...
353: //*************************************************
354: // Abort stubs
355: //*************************************************
356:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
357:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
358:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
359:     irq_stub    IRQStub,                                    InterruptHandler
360: #if BAREMETAL_RPI_TARGET >= 4
361:     smc_stub    SMCStub,                                    SecureMonitorHandler
362: #endif
363: 
...
```

We now changed the functions called for the different types of exceptions to a single one, `ExceptionHandler`

### Update application code {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING__STEP_1_UPDATE_APPLICATION_CODE}

Let's implement and use an exception panic handler, that forces the system to reboot on exception

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: #include <baremetal/ExceptionHandler.h>
7: #include <baremetal/BCMRegisters.h>
8: 
9: #include <unittest/unittest.h>
10: 
11: LOG_MODULE("main");
12: 
13: using namespace baremetal;
14: 
15: static ReturnCode RebootOnException(unsigned /*exceptionID*/, AbortFrame* /*abortFrame*/)
16: {
17:     return ReturnCode::ExitReboot;
18: }
19: 
20: int main()
21: {
22:     auto& console = GetConsole();
23: 
24:     auto exceptionLevel = CurrentEL();
25:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
26: 
27:     console.Write("Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation\n");
28:     char ch{};
29:     while ((ch != 'r') && (ch != 'h') && (ch != 't') && (ch != 'm'))
30:     {
31:         ch = console.ReadChar();
32:         console.WriteChar(ch);
33:     }
34:     RegisterExceptionPanicHandler(RebootOnException);
35: 
36:     if (ch == 't')
37:         // Trap
38:         __builtin_trap();
39:     else if (ch == 'm')
40:     {
41:         // Memory failure
42:         auto r = *((volatile unsigned int*)0xFFFFFFFFFF000000);
43:         // make gcc happy about unused variables :-)
44:         r++;
45:     }
46: 
47:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
48: }
```

- Line 15-18: We add a function `RebootOnException()` that simply returns `ReturnCode::ExitReboot`
- Line 34: We inject the function `RebootOnException()` to be called on exceptions

### Update project configuration {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2_UPDATE_TEST_PROJECT_CONFIGURATION}

As no files were added, we don't need to update the project CMake file.

### Configuring, building and debugging {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:25)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tError  Synchronous exception (PC 00000000000808D8, EC 000000000000003C, ISS 00000000000003E8, FAR 0000000000000000, SP 000000000029FFB0, LR 0000000060000304, SPSR 0000000060000304) (ExceptionHandler:137)
Info   Reboot (System:152)
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:25)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

If we cause a memory violation, the output will be:

```text
Info   Reboot (System:152)
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:25)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mError  Synchronous exception (PC 00000000000808EC, EC 0000000000000025, ISS 0000000000000000, FAR FFFFFFFFFF000000, SP 000000000029FFB0, LR 0000000060000304, SPSR 0000000060000304) (ExceptionHandler:137)
Info   Reboot (System:152)
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:25)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

In both cases, the system reboots.

Next: [20-interrupts](20-interrupts.md)
