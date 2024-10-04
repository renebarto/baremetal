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

## Exception handling - Step 1 {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1}

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

### Register in the ARM processor {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_REGISTER_IN_THE_ARM_PROCESSOR}

- 64 bit registers X0-X30 (W0-W30 are 32 bit registers using the low significant 32 bits of X0-X30)
- 128 bit floating point registers Q0-Q31 (D0-D31 are 64 bit registers using the low significant 64 bits, similar for S0-S31 with 32 bits, H0-H31 with 16 bits, B0-B31 with 8 bits)
- X0-X29: General purpose 64 bit registers)
- X30: Link register (LR)
- SP (also WSP for 32 bits): Stack pointer
- PC: Program counter (this is not accessible directly, but needs specific instructions)
- XZR (also WZR for 32 bits): Zero register
- There is also a large set of system registers. See [ARM registers](#ARM_REGISTERS), [documentation](pdf/arm-architecture-registers.pdf), and [ARM Architecture Reference Manual](pdf/AArch64ReferenceManual.1410976032.pdf) for more information.

### Exception levels in the ARM processor {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_LEVELS_IN_THE_ARM_PROCESSOR}

ARMv8 has four exception levels:
- EL0: Application level
- EL1: Operating system level
- EL2: Hypervisor level
- EL3: Firmware privilege level / Secure monitor level

### Exception level specific registers {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_LEVEL_SPECIFIC_REGISTERS}

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

### Exception types {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_TYPES}

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

### Exception vector {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_VECTOR}

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

### Macros.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_MACROSH}

We are going to use a new macro `BITS` in the previous file, which we'll need to define.

Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
64: /// @brief Convert bit range into integer
65: /// BITS(n, m) results in an integer where bits n to m are set to 1, the rest to 0
66: /// @param n Start (low) bit index
67: /// @param m End (high) bit index
68: #define BITS(n,m)           (((1UL << (m-n+1)) - 1) << (n))
```

This defines the `BITS` macro to create a mask for bit sequences (from bit n to up to and including bit m)

### ARMInstructions.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_ARMINSTRUCTIONSH}

We already created a header for the ARM specific instructions, but it is also handy to define some fields for specific ARM registers.
The complete set or ARM registers is documented in [documentation](pdf/arm-architecture-registers.pdf), the most important ones are described in [ARM registers](#ARM_REGISTERS).

We'll add definitions for some of these registers.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
42: #include <baremetal/Macros.h>
43:
...
91: /// @brief Get current exception level
92: #define GetCurrentEL(value)             asm volatile ("mrs %0, CurrentEL" : "=r" (value))
93: /// @brief SPSR_EL1 M[3:0] field bit shift
94: #define CURRENT_EL_SHIFT 2
95: /// @brief SPSR_EL1 M[3:0] field bit mask
96: #define CURRENT_EL_MASK BITS(2,3)
97: 
98: /// @brief Get Saved Program Status Register (EL1)
99: #define GetSPSR_EL1(value)              asm volatile ("mrs %0, SPSR_EL1" : "=r" (value))
100: 
101: /// @brief Get Exception Syndrome Register (EL1)
102: #define GetESR_EL1(value)               asm volatile ("mrs %0, ESR_EL1" : "=r" (value))
103: 
104: /// @brief SPSR_EL1 M[3:0] field bit shift
105: #define SPSR_EL1_M30_SHIFT 0
106: /// @brief SPSR_EL1 M[3:0] field bit mask
107: #define SPSR_EL1_M30_MASK BITS(0,3)
108: /// @brief SPSR_EL1 M[3:0] field exctraction
109: #define SPSR_EL1_M30(value) ((value >> SPSR_EL1_M30_SHIFT) & SPSR_EL1_M30_MASK)
110: /// @brief SPSR_EL1 M[3:0] field value for EL0t mode
111: #define SPSR_EL1_M30_EL0t 0x0
112: /// @brief SPSR_EL1 M[3:0] field value for EL1t mode
113: #define SPSR_EL1_M30_EL1t 0x4
114: /// @brief SPSR_EL1 M[3:0] field value for EL1h mode
115: #define SPSR_EL1_M30_EL1h 0x5
116: 
117: /// @brief ESR_EL1 EC field bit shift
118: #define ESR_EL1_EC_SHIFT 26
119: /// @brief ESR_EL1 EC field bit mask
120: #define ESR_EL1_EC_MASK  BITS(0,5)
121: /// @brief ESR_EL1 EC field extraction
122: #define ESR_EL1_EC(value) ((value >> ESR_EL1_EC_SHIFT) & ESR_EL1_EC_MASK)
123: /// @brief ESR_EL1 EC field value for unknown exception
124: #define ESR_EL1_EC_UNKNOWN 0x00
125: /// @brief ESR_EL1 EC field value for trapped WF<x> instruction exception
126: #define ESR_EL1_EC_TRAPPED_WFx_INSTRUCTION 0x01
127: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0F
128: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0F 0x03
129: /// @brief ESR_EL1 EC field value for MCRR or MRRC instruction exception when coproc = 0x0F
130: #define ESR_EL1_EC_TRAPPED_MCRR_MRRC_ACCESS_CORPROC_0F 0x04
131: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0E
132: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0E 0x05
133: /// @brief ESR_EL1 EC field value for trapped LDC or STC instruction exception
134: #define ESR_EL1_EC_TRAPPED_LDC_STC_ACCESS 0x06
135: /// @brief ESR_EL1 EC field value for unknown SME, SVE, SIMD or Floating pointer instruction exception
136: #define ESR_EL1_EC_TRAPPED_SME_SVE_SIMD_FP_ACCESS 0x07
137: /// @brief ESR_EL1 EC field value for trapped LD64<x> or ST64<x> instruction exception
138: #define ESR_EL1_EC_TRAPPED_LD64x_ST64x_ACCESS 0x0A
139: /// @brief ESR_EL1 EC field value for trapped MRRC instruction exception when coproc = 0x0C
140: #define ESR_EL1_EC_TRAPPED_MRRC_ACCESS_COPROC_0E 0x0C
141: /// @brief ESR_EL1 EC field value for branch target exception
142: #define ESR_EL1_EC_BRANCH_TARGET_EXCEPTION 0x0D
143: /// @brief ESR_EL1 EC field value for illegal executions state exception
144: #define ESR_EL1_EC_ILLEGAL_EXECUTION_STATE 0x0E
145: /// @brief ESR_EL1 EC field value for trapped SVC 32 bit instruction exception
146: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_32 0x11
147: /// @brief ESR_EL1 EC field value for trapped SVC 64 bit instruction exception
148: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_64 0x15
149: /// @brief ESR_EL1 EC field value for MCR or MRC 64 bit instruction exception
150: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_64 0x18
151: /// @brief ESR_EL1 EC field value for trapped SVE access exception
152: #define ESR_EL1_EC_TRAPPED_SVE_ACCESS 0x19
153: /// @brief ESR_EL1 EC field value for trapped TStart access exception
154: #define ESR_EL1_EC_TRAPPED_TSTART_ACCESS 0x1B
155: /// @brief ESR_EL1 EC field value for pointer authentication failure exception
156: #define ESR_EL1_EC_POINTER_AUTHENTICATION_FAILURE 0x1C
157: /// @brief ESR_EL1 EC field value for trapped SME access exception
158: #define ESR_EL1_EC_TRAPPED_SME_ACCESS 0x1D
159: /// @brief ESR_EL1 EC field value for granule protection check exception
160: #define ESR_EL1_EC_GRANULE_PROTECTION_CHECK 0x1E
161: /// @brief ESR_EL1 EC field value for instruction abort from lower EL exception
162: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL 0x20
163: /// @brief ESR_EL1 EC field value for instruction abort from same EL exception
164: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_SAME_EL 0x21
165: /// @brief ESR_EL1 EC field value for PC alignment fault exception
166: #define ESR_EL1_EC_PC_ALIGNMENT_FAULT 0x22
167: /// @brief ESR_EL1 EC field value for data abort from lower EL exception
168: #define ESR_EL1_EC_DATA_ABORT_FROM_LOWER_EL 0x24
169: /// @brief ESR_EL1 EC field value for data abort from same EL exception
170: #define ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL 0x25
171: /// @brief ESR_EL1 EC field value for SP alignment fault exception
172: #define ESR_EL1_EC_SP_ALIGNMENT_FAULT 0x27
173: /// @brief ESR_EL1 EC field value for trapped 32 bit FP instruction exception
174: #define ESR_EL1_EC_TRAPPED_FP_32 0x28
175: /// @brief ESR_EL1 EC field value for trapped 64 bit FP instruction exception
176: #define ESR_EL1_EC_TRAPPED_FP_64 0x2C
177: /// @brief ESR_EL1 EC field value for SError interrupt exception
178: #define ESR_EL1_EC_SERROR_INTERRUPT 0x2F
179: /// @brief ESR_EL1 EC field value for Breakpoint from lower EL exception
180: #define ESR_EL1_EC_BREAKPOINT_FROM_LOWER_EL 0x30
181: /// @brief ESR_EL1 EC field value for Breakpoint from same EL exception
182: #define ESR_EL1_EC_BREAKPOINT_FROM_SAME_EL 0x31
183: /// @brief ESR_EL1 EC field value for SW step from lower EL exception
184: #define ESR_EL1_EC_SW_STEP_FROM_LOWER_EL 0x32
185: /// @brief ESR_EL1 EC field value for SW step from same EL exception
186: #define ESR_EL1_EC_SW_STEP_FROM_SAME_EL 0x33
187: /// @brief ESR_EL1 EC field value for Watchpoint from lower EL exception
188: #define ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL 0x34
189: /// @brief ESR_EL1 EC field value for Watchpoint from same EL exception
190: #define ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL 0x35
191: /// @brief ESR_EL1 EC field value for 32 bit BKPT instruction exception
192: #define ESR_EL1_EC_BKPT_32 0x38
193: /// @brief ESR_EL1 EC field value for 64 bit BRK instruction exception
194: #define ESR_EL1_EC_BRK_64 0x3C
195: 
196: /// @brief ESR_EL1 ISS field bit shift
197: #define ESR_EL1_ISS_SHIFT 0
198: /// @brief ESR_EL1 ISS field bit mask
199: #define ESR_EL1_ISS_MASK  BITS(0,24)
200: /// @brief ESR_EL1 ISS field extraction
201: #define ESR_EL1_ISS(value) ((value >> ESR_EL1_ISS_SHIFT) & ESR_EL1_ISS_MASK)
```

- Line 42: We need to include the macro header for a new definition we're going to use
- Line 92: We add an instruction to retrieve the value of the `CurrentEL` register, which holds the current exception level
- Line 94-96: We define the mask and shift for the exception level in the CurrentEL register
- Line 99: We add an instruction to retrieve the value of the `SPSR_EL1` (Saved Program Status Register EL1), which holds information on state of the processor when an exception occurs
- Line 102: We add an instruction to retrieve the value of the `ESR_EL1` (Exception Status Register EL1), which hold information on the cause of an exception
- Line 105-109: We define the M3:0 field in the `SPSR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 110-115: We define different values for this field
- Line 118-122: We define the EC field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 124-194: We define different values for this field
- Line 197-201: We define the ISS field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)

### Exception.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONH}

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

### ExceptionHandler.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONHANDLERH}

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

### ExceptionHandler.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONHANDLERCPP}

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

### BCMRegisters.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_BCMREGISTERSH}

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

### ExceptionStub.S {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONSTUBS}

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
17: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
110: /*
111:  * HVC stub
112:  */
113: HVCStub:                                // Return to EL2h mode
114:     mrs     x0, spsr_el2                // Read Saved Program Status Register (EL2)
115:     bic     x0, x0, #0xF                // Clear bit 3:0
116:     mov     x1, #9
117:     orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
118:     msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
119:     eret                                // Move to EL2h
120: 
121:     .data
122: 
123:     .align  3
124: 
125:     .globl  s_fiqData
126: s_fiqData:                              // Matches FIQData:
127:     .quad   0                           // handler
128:     .quad   0                           // param
129:     .word   0                           // fiqID (unused)
130: 
131: //*************************************************
132: // Abort stubs
133: //*************************************************
134:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   UnexpectedHandler
135:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  SynchronousExceptionHandler
136:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, SystemErrorHandler
137: 
138: // End
```

- Line 48-54: We create a macro `vector` to define a vector in the vector table.
Every vector is aligned to 128 bytes, hence the statement `.align 7` which align at 7 bits, or 128 (1 << 7) bytes.
The only code we add is a jump (branch, b) to the current stub, passed as `handler`
- Line 56-77: We create a macro `stub` for a generic stub, which saves the system registers `ESR_EL11`, `SPSR_EL1`, `ELR_EL1`, `SP_EL0` and `FAR_EL1` and the registers `x30` (LR) and `SP`, in general purpose registers x0-x6.
These are then stored on the stack, every time decreasing the stack pointer by 16 bytes.
Then the exception type passed through `exception` is stored in x0, and the stack pointer in x1.
Finally the exception handler passed through `handler` is branched to.
This will effectively call the exception handler function `void handler(uint64 exceptionID, AbortFrame* abortFrame)` with `exceptionID` = x0, `abortFrame` = x1
- Line 79-108: We implement the actual exception vector table.
This has 16 entries using the macro `vector`, each aligned to 128 bytes, while the complete table is aligned to 2048 (1 << 11) bytes, hence `.align 11`
- Line 113-119: We implement the hypervisor call stub `HVCStub`.
The function reads the `SPSR_EL2` system register, and sets it to switch to EL2h exception level, then moves to that that exception level using `eret`.
This also resets the state of interrupt enables, etc.
- Line 123-129: We define the `s_fiqData` structure, aligned to 8 bytes
- Line 134: We declare unexpected exceptions stub using the `stub` macro, and set its handler to `UnexpectedHandler()`
- Line 135: We declare synchronous exceptions stub using the `stub` macro, and set its handler to `SynchronousExceptionHandler()`
- Line 136: We declare system errors stub using the `stub` macro, and set its handler to `SystemErrorHandler()`

Note that this means we need to define the functions `UnexpectedHandler()`, `SynchronousExceptionHandler()`, `SystemErrorHandler()`

### System.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_SYSTEMH}

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
56: 
...
```

### System.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_SYSTEMCPP}

We'll implement the newly added `CurrentEL()` function.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
90: uint8 baremetal::CurrentEL()
91: {
92:     uint32 registerValue{};
93:     GetCurrentEL(registerValue);
94:     return (registerValue & CURRENT_EL_MASK) >> CURRENT_EL_SHIFT;
95: }
96: 
...
```

This function used the assembly macro `GetCurrentEL` to retrieve the `CurrentEL` system register value, and extracts the bits 2..3 which hold the exception level.

### Update application code {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_UPDATE_APPLICATION_CODE}

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
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: 
13: int main()
14: {
15:     auto& console = GetConsole();
16: 
17:     auto exceptionLevel = CurrentEL();
18:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
19: 
20:     console.Write("Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation\n");
21:     char ch{};
22:     while ((ch != 'r') && (ch != 'h') && (ch != 't') && (ch != 'm'))
23:     {
24:         ch = console.ReadChar();
25:         console.WriteChar(ch);
26:     }
27: 
28:     if (ch == 't')
29:         // Trap
30:         __builtin_trap();
31:     else if (ch == 'm')
32:     {
33:         // Memory failure
34:         auto r = *((volatile unsigned int*)0xFFFFFFFFFF000000);
35:         // make gcc happy about unused variables :-)
36:         r++;
37:     }
38: 
39:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
40: }
```

- Line 28-30: If we press `t`, we cause a standard system trap, which results in a debug break
- Line 32-37: If we press 'm', we cause a data abort due to reading a non-existent memory location

### Update project configuration {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_UPDATE_PROJECT_CONFIGURATION}

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

### baremetal.ld {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_BAREMETALLD}

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

### Startup.S

The exception vector was defined, but we need to use it. As we are running in EL1 normally, we will set the exception vector for EL1.
This is done through the `vbar_el2` (Vector Base Address Register EL2) or `vbar_el1` (Vector Base Address Register EL1) register depending on the exception level the system was in when the exception occurs.
See [Arm Architecture Registers](pdf/arm-architecture-registers.pdf), page 2436 and 2439.

Update the file `code/libraries/baremetal/src/Startup.S`

```cmake
File: code/libraries/baremetal/src/Startup.S
...
100: .section .init
101: 
102:     .globl _start
103: _start:                                 // Normally entered from armstub8 in EL2 after boot
104:     mrs x0, CurrentEL                   // Check if already in EL1t mode?
105:     cmp x0, #4
106:     beq EL1
107: 
108:     ldr x0, =MEM_EXCEPTION_STACK        // IRQ, FIQ and exception handler run in EL1h
109:     msr sp_el1, x0                      // Write Stack Pointer (EL1), init El1 stack
110: 
111:     ldr	x0, =VectorTable                // Init exception vector table for EL2
112:     msr	vbar_el2, x0                    // Write Vector Base Address Register (EL2)
113: 
114:     armv8_switch_to_el1_m x0, x1        // Move to EL1
115: 
116: EL1:
117:     ldr x0, =MEM_KERNEL_STACK           // Main thread runs in EL1t and uses sp_el0
118:     mov sp, x0                          // init its stack
119: 
120:     ldr	x0, =VectorTable	            // Init exception vector table for EL1
121:     msr	vbar_el1, x0                    // Write Vector Base Address Register (EL1)
122: 
123:     b sysinit                           // Jump to C/C++ code
124: 
125: // End
```

### Configuring, building and debugging {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:18)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tInfo   SynchronousHandler EC=3C ISS=00003E8 (ExceptionHandler:64)
Info   SynchronousHandler EC=21 ISS=0000000 (ExceptionHandler:64)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:64)
```

If we cause a memory violation, the output will be:

```text
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:18)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mInfo   SynchronousHandler EC=25 ISS=0000000 (ExceptionHandler:64)
Info   SynchronousHandler EC=21 ISS=0000000 (ExceptionHandler:64)
Info   SynchronousHandler EC=22 ISS=0000000 (ExceptionHandler:64)
```

As can be found in [Arm Architecture Registers](pdf/arm-architecture-registers.pdf), page 570, exception code 3C stands for "BRK instruction execution in AArch64 state", and on page 571, exception code 25 stands for "Data Abort taken without a change in Exception level."

The exception condition does not get resolved, so after the function returns, a new exception is thrown:

- 21 stands for "Instruction Abort taken without a change in Exception level."
- 22 stands for "PC alignment fault exception."

## Exception system - Step 2 {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2}

To improve our code a little, let's create a class for handling exceptions.

### ExceptionHandler.h {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONHANDLERH}

We'll add the class `ExceptionSystem` and change the exception handling to a single function `ExceptionHandler()`.

Update the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
...
42: #include <baremetal/Types.h>
43: #include <baremetal/System.h>
...
48: #ifdef __cplusplus
49: extern "C" {
50: #endif
51: 
52: /// @brief Exception abort frame
53: ///
54: /// Storage for register value in case of exception, in order to recover
55: struct AbortFrame
56: {
57:     /// @brief Exception Syndrome Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
58:     uint64	esr_el1;
59:     /// @brief Saved Program Status Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER
60:     uint64	spsr_el1;
61:     /// @brief General-purpose register, Link Register
62:     uint64	x30;
63:     /// @brief Exception Link Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
64:     uint64	elr_el1;
65:     /// @brief Stack Pointer (EL0). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
66:     uint64	sp_el0;
67:     /// @brief Stack Pointer (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
68:     uint64	sp_el1;
69:     /// @brief Fault Address Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
70:     uint64	far_el1;
71:     /// @brief Unused valuem used to align to 64 bytes
72:     uint64	unused;
73: }
74: /// @brief Just specifies the struct is packed
75: PACKED;
76: 
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
95: /// <summary>
96: /// Exception handling system. Handles ARM processor exceptions
97: ///
98: /// This is a singleton class, created as soon as GetExceptionSystem() is called
99: /// </summary>
100: class ExceptionSystem
101: {
102:     /// <summary>
103:     /// Construct the singleton exception system instance if needed, and return a reference to the instance. This is a friend function of class ExceptionSystem
104:     /// </summary>
105:     /// <returns>Reference to the singleton system instance</returns>
106:     friend ExceptionSystem& GetExceptionSystem();
107: 
108: private:
109:     ExceptionSystem();
110: 
111: public:
112:     ~ExceptionSystem();
113: 
114:     void Throw(unsigned exceptionID, AbortFrame* abortFrame);
115: };
116: 
117: /// <summary>
118: /// Injectable exception handler
119: /// </summary>
120: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
121: /// <param name="abortFrame">Stored state information at the time of exception</param>
122: /// <returns>ReturnCode::ExitHalt if the system should be halted, ReturnCode::ExitReboot if the system should reboot</returns>
123: using ExceptionPanicHandler = ReturnCode(unsigned exceptionID, AbortFrame* abortFrame);
124: 
125: const char* GetExceptionType(unsigned exceptionID);
126: 
127: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler);
128: 
129: ExceptionSystem& GetExceptionSystem();
130: 
131: } // namespace baremetal
```

- Line 82: We change the three exception handlers to a single one, `ExceptionHandler()`
- Line 87: We declare the interrupt handler, `InterruptHandler()`
- Line 10-115: We declare the class `ExceptionSystem`
  - Line 106: We make the function `GetExceptionSystem()` a friend to the class, so we can use it to create the singleton instance
  - Line 109: We declare the private (default) constructor, so only the `GetExceptionSystem()` function can be used to create and instance
  - Line 112: We declare the destructor
  - Line 114: We declare the method `Throw()` which will be called by `ExceptionHandler()`
- Line 123: We declare the type for an injectable exception panic handler.
This will be call by the `Throw()` method of `ExceptionSystem` if set, and will return true if the system should be halted, false if not
- Line 125: We declare a helper function `GetExceptionType()` to convert an exception type into a string
- Line 127: We declare a method to set the exception panic handler, returning the old installed handler, if any
- Line 129: We declare the function `GetExceptionSystem()` that creates the singleton instance of the `ExceptionSystem` class, if needed

### ExceptionHandler.cpp {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONHANDLERCPP}

We'll implement the newly added `ExceptionSystem` class and reimplement the ExceptionHandler() function.

Update the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
...
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/Assert.h>
44: #include <baremetal/Logger.h>
45: 
46: /// @file
47: /// Exception handler function implementation
48: 
49: /// @brief Define log name
50: LOG_MODULE("ExceptionHandler");
51: 
52: /// <summary>
53: /// Handles an exception, with the abort frame passed in.
54: ///
55: /// The exception handler is called from assembly code (ExceptionStub.S)
56: /// </summary>
57: /// <param name="exceptionID">Exception type being thrown (one of EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS, EXCEPTION_SYSTEM_ERROR)</param>
58: /// <param name="abortFrame">Filled in AbortFrame instance</param>
59: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
60: {
61:     baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
62: }
63: 
64: void InterruptHandler()
65: {
66: }
67: 
68: namespace baremetal {
69: 
70: /// <summary>
71: /// Exception panic handler
72: ///
73: /// If set, the panic handler is called first, and if it returns false, the system is not halted (as it would by default)
74: /// </summary>
75: static ExceptionPanicHandler* s_exceptionPanicHandler{};
76: 
77: /// <summary>
78: /// Names exception types
79: ///
80: /// Order must match exception identifiers in baremetal/Exception.h
81: /// </summary>
82: static const char* s_exceptionName[] =
83: {
84:     "Unexpected exception",
85:     "Synchronous exception",
86:     "System error"
87: };
88: 
89: /// <summary>
90: /// Constructor
91: ///
92: /// Note that the constructor is private, so GetExceptionSystem() is needed to instantiate the exception handling system
93: /// </summary>
94: ExceptionSystem::ExceptionSystem()
95: {
96: }
97: 
98: /// <summary>
99: /// Destructor
100: /// </summary>
101: ExceptionSystem::~ExceptionSystem()
102: {
103: }
104: 
105: /// <summary>
106: /// Throw an exception to the exception system
107: /// </summary>
108: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
109: /// <param name="abortFrame">Stored state information at the time of exception</param>
110: void ExceptionSystem::Throw(unsigned exceptionID, AbortFrame* abortFrame)
111: {
112:     assert(abortFrame != nullptr);
113: 
114:     uint64 sp = abortFrame->sp_el0;
115:     if (SPSR_EL1_M30(abortFrame->spsr_el1) == SPSR_EL1_M30_EL1h)		// EL1h mode?
116:     {
117:         sp = abortFrame->sp_el1;
118:     }
119: 
120:     uint64 EC = ESR_EL1_EC(abortFrame->esr_el1);
121:     uint64 ISS = ESR_EL1_ISS(abortFrame->esr_el1);
122: 
123:     uint64 FAR = 0;
124:     if ((ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL)
125:         || (ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL))
126:     {
127:         FAR = abortFrame->far_el1;
128:     }
129: 
130:     ReturnCode action = ReturnCode::ExitHalt;
131:     if (s_exceptionPanicHandler != nullptr)
132:     {
133:         action = (*s_exceptionPanicHandler)(exceptionID, abortFrame);
134:     }
135: 
136:     if (action == ReturnCode::ExitHalt)
137:         LOG_PANIC("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
138:             s_exceptionName[exceptionID],
139:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
140:     else
141:     {
142:         LOG_ERROR("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
143:             s_exceptionName[exceptionID],
144:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
145:         GetSystem().Reboot();
146:     }
147: }
148: 
149: /// @brief Convert exception ID to a string
150: /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
151: /// @return String representation of exception ID
152: const char* GetExceptionType(unsigned exceptionID)
153: {
154:     return s_exceptionName[exceptionID];
155: }
156: 
157: /// @brief Register an exception callback function, and return the previous one.
158: /// @param handler      Exception handler callback function to register
159: /// @return Previously set exception handler callback function
160: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler)
161: {
162:     auto result = s_exceptionPanicHandler;
163:     s_exceptionPanicHandler = handler;
164:     return result;
165: }
166: 
167: /// <summary>
168: /// Construct the singleton exception system instance if needed, and return a reference to the instance
169: ///
170: /// This is a friend function of class ExceptionSystem
171: /// </summary>
172: /// <returns>Reference to the singleton exception system instance</returns>
173: ExceptionSystem& GetExceptionSystem()
174: {
175:     static ExceptionSystem system;
176:     return system;
177: }
178: 
179: } // namespace baremetal
```

- Line 43: As we are going to use `assert()`, we need to include its header
- Line 59-62: We implement the function `ExceptionHandler()` by calling the `Throw()` method on the singleton `ExceptionSystem` instance
- Line 64-66: We implement the function `ExceptionHandler()` by calling the `Throw()` method on the singleton `ExceptionSystem` instance
- Line 715: We define a static variable `s_exceptionPanicHandler` to hold the injected exception panic handler pointer.
It is initialized as nullptr, meaning not handler is installed
- Line 82-87: We define the mapping `s_exceptionName` from exception type to string
- Line 94-96: We implement the `ExceptionSystem` constructor
- Line 101-103: We implement the `ExceptionSystem` destructor
- Line 11-147: We implement the method `Throw()`
  - Line 114-118: We determine the stack pointer at the time of the exception, which is the EL0 stack pointer, unless the exception level was EL1h (We can only be in exception level EL0 or EL1, as we specifically move to EL1 in the startup code)
  - Line 120-121: We extract the EC and ISS fields from the ESR_EL1 register value
  - Line 124-128: If the exception has an exception code relating to an instruction abort, a data abort or a watchpoint, We get the value of the Fault Address Register, which holds the address which generated the exception
  - Line 130-134: We call the exception panic handler if installed, setting its return value as the return code. If no handler is installed, the default is to halt
  - Line 136-146: If we need to halt, we perform a panic log (which will halt the system), otherwise we log an error and reboot
- Line 152-155: We implement the function `GetExceptionType()`
- Line 160-165: We implement the function `RegisterPanicHandler()`, which sets the `s_exceptionPanicHandler` variable, and returns the original value of this variable
- Line 173-177: We implement the function `GetExceptionSystem()` in the by now common way

### ExceptionStub.S {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONSTUBS}

We'll implement the newly added `CurrentEL()` function.

Update the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
...
131: //*************************************************
132: // Abort stubs
133: //*************************************************
134:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
135:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
136:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
...
```

We now changed the functions called for the different types of exceptions to a single one, `ExceptionHandler`

### Update application code {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_UPDATE_APPLICATION_CODE}

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
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: 
13: static ReturnCode RebootOnException(unsigned /*exceptionID*/, AbortFrame* /*abortFrame*/)
14: {
15:     return ReturnCode::ExitReboot;
16: }
17: 
18: int main()
19: {
20:     auto& console = GetConsole();
21: 
22:     auto exceptionLevel = CurrentEL();
23:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
24: 
25:     console.Write("Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation\n");
26:     char ch{};
27:     while ((ch != 'r') && (ch != 'h') && (ch != 't') && (ch != 'm'))
28:     {
29:         ch = console.ReadChar();
30:         console.WriteChar(ch);
31:     }
32:     RegisterExceptionPanicHandler(RebootOnException);
33: 
34:     if (ch == 't')
35:         // Trap
36:         __builtin_trap();
37:     else if (ch == 'm')
38:     {
39:         // Memory failure
40:         auto r = *((volatile unsigned int*)0xFFFFFFFFFF000000);
41:         // make gcc happy about unused variables :-)
42:         r++;
43:     }
44: 
45:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
46: }
```

- Line 13-16: We add a function `RebootOnException()` that simply returns `ReturnCode::ExitReboot`
- Line 32: We inject the function `RebootOnException()` to be called on exceptions

### Update project configuration {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_UPDATE_PROJECT_CONFIGURATION}

As no files were added, we don't need to update the project CMake file.

### Configuring, building and debugging {#TUTORIAL_19_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tError  Synchronous exception (PC 00000000000808D8, EC 000000000000003C, ISS 00000000000003E8, FAR 0000000000000000, SP 000000000029FFB0, LR 00000000000808CC, SPSR 0000000060000304) (ExceptionHandler:138)
Info   Reboot (System:152)
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

If we cause a memory violation, the output will be:

```text
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mError  Synchronous exception (PC 00000000000808EC, EC 0000000000000025, ISS 0000000000000000, FAR FFFFFFFFFF000000, SP 000000000029FFB0, LR 00000000000808CC, SPSR 0000000060000304) (ExceptionHandler:138)
Info   Reboot (System:152)
Info   Baremetal 0.0.0 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

As we can see, in the second case the exception was caused by memory address 0xFFFFFFFFFF000000, which is exactly the faulty address we were trying to read from.

In both cases, the system reboots.

Next: [20-interrupts](20-interrupts.md)
