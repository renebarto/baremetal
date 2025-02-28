# Tutorial 17: Exceptions {#TUTORIAL_17_EXCEPTIONS}

@tableofcontents

## New tutorial setup {#TUTORIAL_17_EXCEPTIONS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-exceptions`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_17_EXCEPTIONS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- a library `output/Debug/lib/stdlib-17.a`
- an application `output/Debug/bin/17-exceptions.elf`
- an image in `deploy/Debug/17-exceptions-image`

## Exception handling - Step 1 {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1}

Before we can do anything with interrupts, we need to embrace the concept of exceptions.
These are not the same as exceptions in programming languages such as C++, but are exceptions in the context of the processor.
For example, suppose you are dividing by zero. This is an exception that the processor will raise.
Also when accessing memory that is not available, or when an interrupt is raised, an exception is raised.

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

### Registers in the ARM processor {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_REGISTERS_IN_THE_ARM_PROCESSOR}

The ARM processor has a large set of registers, some of which are general purpose, some are special purpose, and some are system registers:
- 64 bit registers X0-X30 (W0-W30 are 32 bit registers using the low significant 32 bits of X0-X30)
  - X0-X29: General purpose 64 bit registers)
  - X30: Link register (LR)
- 128 bit floating point registers Q0-Q31 (D0-D31 are 64 bit registers using the low significant 64 bits, similar for S0-S31 with 32 bits, H0-H31 with 16 bits, B0-B31 with 8 bits)
- SP (also WSP for 32 bits): Stack pointer
- PC: Program counter (this is not accessible directly, but needs specific instructions)
- XZR (also WZR for 32 bits): Zero register
- There is also a large set of system registers. See [ARM registers](#ARM_REGISTERS), [documentation](pdf/arm-architecture-registers.pdf), and [ARM Architecture Reference Manual](pdf/arm-aarch64-reference-manual.pdf) for more information.

### Exception levels in the ARM processor {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_LEVELS_IN_THE_ARM_PROCESSOR}

ARMv8 has four exception levels:
- EL0: Application level
- EL1: Operating system level (there is both a EL1t and a EL1h mode, which is a higher privilege level)
- EL2: Hypervisor level (there is both a EL3t and a EL3h mode, which is a higher privilege level)
- EL3: Firmware privilege level / Secure monitor level (there is both a EL3t and a EL3h mode, which is a higher privilege level)

Be aware that for Raspberry Pi, the VC starts firmware at EL3, so EL3 is not available for us, unless we change the boot code.
However starting at EL2 is sufficient, as we will normally only need EL0 for applications and EL1 for "OS" code.

See the image below:

<img src="images/exception-levels.png" alt="Exception levels" width="600px"/>

### Exception level specific registers {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_LEVEL_SPECIFIC_REGISTERS}

Each exception level has its own special registers:
- EL0: SP_EL0
- EL1: SP_EL1, ELR_EL1 (Exception Link Register), SPSR_EL1 (Saved Process Status Register), VBAR_EL1 (Vector Base Address Register)_
- EL2: SP_EL2, ELR_EL2 (Exception Link Register), SPSR_EL2 (Saved Process Status Register), VBAR_EL2 (Vector Base Address Register)
- EL3: SP_EL3, ELR_EL3 (Exception Link Register), SPSR_EL3 (Saved Process Status Register), VBAR_EL3 (Vector Base Address Register)

Stack pointers can be used in a different way:
- EL0: EL0t uses SP_EL0
- EL1: EL1t uses SP_EL0, EL1h uses SP_EL1
- EL2: EL2t uses SP_EL0, EL2h uses SP_EL2
- EL3: EL3t uses SP_EL0, EL3h uses SP_EL3

### Exception types {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_TYPES}

- Interrupts: There are two types of interrupts called IRQ and FIQ
  - FIQ is higher priority than IRQ. Both of these kinds of exception are typically associated with input pins on the core.
  - External hardware asserts an interrupt request line and the corresponding exception type is raised when the current instruction finishes executing (although some instructions, those that can load multiple values, can be interrupted), assuming that the interrupt is not disabled
  - IRQ can be interrupted by FIQ
  - FIQ cannot be interrupted by other FIQ, only one can be active at any time
- Aborts: Aborts can be generated either on failed instruction fetches (instruction aborts) or failed data accesses (data aborts)
- Reset: Reset is treated as a special vector for the highest implemented Exception level
- Exception generating instructions: Execution of certain instructions can generate exceptions. Such instructions are typically executed to request a service from software that runs at a higher privilege level:
  - The Supervisor Call (SVC) instruction enables User mode programs to request an OS service.
  - The Hypervisor Call (HVC) instruction enables the guest OS to request hypervisor services.
  - The Secure monitor Call (SMC) instruction enables the Normal world to request Secure world services.

### Exception vector {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTION_VECTOR}

When an exception occurs, the processor must execute handler code which corresponds to the exception.
The location in memory where the handler is stored is called the exception vector.

The base address is given by VBAR_ELn, then each entry has a defined offset from this base address.
Each table has 16 entries, with each entry being 128 bytes (32 instructions) in size.

Each exception level (except EL0) has its own exception vector.
The table effectively consists of 4 sets of 4 entries each. Which entry is used depends upon a number of factors:
- The type of exception (SError, FIQ, IRQ or Synchronous)
- If the exception is being taken at the same Exception level, the Stack Pointer to be used (SP_EL0 or SP_ELx)
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

From [Aarch64 Exception Model](pdf/aarch64-exception-model.pdf):

Synchronous exceptions are exceptions that can be caused by, or are related to, the instruction that is currently being executed.
Synchronous exceptions are synchronous to the execution stream, as they are directly related to the currently executing instruction.
For example a synchronous exception would be triggered by an instruction attempting to write to a read-only location as defined by the MMU.

System Error (SError) is an exception type that is intended to be generated by the memory system in response to unexpected events.
We do not expect these events, but need to know if they have occurred.
These are reported asynchronously because the instruction that triggered the event may have already been retired.

In AArch64, synchronous aborts cause a synchronous exception. Asynchronous
aborts cause an SError interrupt exception.

We'll need to write some more assembly code, that implements the different exception handlers, and creates the exception vector table.

### Macros.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_MACROSH}

We are going to use a new macro `BITS` in the previous file, which we'll need to define.

Update the file `code/libraries/stdlib/include/stdlib/Macros.h`

```cpp
File: code/libraries/stdlib/include/stdlib/Macros.h
...
66: /// @brief Convert bit range into integer
67: /// @param n Start (low) bit index
68: /// @param m End (high) bit index
69: #define BITS(n,m)           (((1UL << (m-n+1)) - 1) << (n))
```

This defines the `BITS` macro to create a mask for bit sequences (from bit n to up to and including bit m)

### ARMInstructions.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_ARMINSTRUCTIONSH}

We already created a header for the ARM specific instructions, but it is also handy to define some fields for specific ARM registers.
The complete set of ARM registers is documented in [documentation](pdf/arm-architecture-registers.pdf), the most important ones are described in [ARM registers](#ARM_REGISTERS).

We'll add some definitions for some of these registers.

Update the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
...
42: #include <baremetal/Macros.h>
43:
...
92: /// @brief Get current exception level
93: #define GetCurrentEL(value)             asm volatile ("mrs %0, CurrentEL" : "=r" (value))
94: 
95: /// @brief Get Saved Program Status Register (EL1)
96: #define GetSPSR_EL1(value)              asm volatile ("mrs %0, SPSR_EL1" : "=r" (value))
97: 
98: /// @brief Get Exception Syndrome Register (EL1)
99: #define GetESR_EL1(value)               asm volatile ("mrs %0, ESR_EL1" : "=r" (value))
100: 
101: /// @brief SPSR_EL M[3:0] field bit shift
102: #define SPSR_EL1_M30_SHIFT 0
103: /// @brief SPSR_EL M[3:0] field bit mask
104: #define SPSR_EL1_M30_MASK BITS(0,3)
105: /// @brief SPSR_EL M[3:0] field exctraction
106: #define SPSR_EL1_M30(value) ((value >> SPSR_EL1_M30_SHIFT) & SPSR_EL1_M30_MASK)
107: /// @brief SPSR_EL M[3:0] field value for EL0t mode
108: #define SPSR_EL_M30_EL0t 0x0
109: /// @brief SPSR_EL M[3:0] field value for EL1t mode
110: #define SPSR_EL_M30_EL1t 0x4
111: /// @brief SPSR_EL M[3:0] field value for EL1h mode
112: #define SPSR_EL_M30_EL1h 0x5
113: /// @brief SPSR_EL M[3:0] field value for EL2t mode
114: #define SPSR_EL_M30_EL2t 0x8
115: /// @brief SPSR_EL M[3:0] field value for EL2h mode
116: #define SPSR_EL_M30_EL2h 0x9
117: /// @brief SPSR_EL M[3:0] field value for EL3t mode
118: #define SPSR_EL_M30_EL3t 0xC
119: /// @brief SPSR_EL M[3:0] field value for EL3h mode
120: #define SPSR_EL_M30_EL3h 0xD
121: 
122: /// @brief ESR_EL1 EC field bit shift
123: #define ESR_EL1_EC_SHIFT 26
124: /// @brief ESR_EL1 EC field bit mask
125: #define ESR_EL1_EC_MASK  BITS(0,5)
126: /// @brief ESR_EL1 EC field extraction
127: #define ESR_EL1_EC(value) ((value >> ESR_EL1_EC_SHIFT) & ESR_EL1_EC_MASK)
128: /// @brief ESR_EL1 EC field value for unknown exception
129: #define ESR_EL1_EC_UNKNOWN 0x00
130: /// @brief ESR_EL1 EC field value for trapped WF<x> instruction exception
131: #define ESR_EL1_EC_TRAPPED_WFx_INSTRUCTION 0x01
132: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0F
133: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0F 0x03
134: /// @brief ESR_EL1 EC field value for MCRR or MRRC instruction exception when coproc = 0x0F
135: #define ESR_EL1_EC_TRAPPED_MCRR_MRRC_ACCESS_CORPROC_0F 0x04
136: /// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0E
137: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0E 0x05
138: /// @brief ESR_EL1 EC field value for trapped LDC or STC instruction exception
139: #define ESR_EL1_EC_TRAPPED_LDC_STC_ACCESS 0x06
140: /// @brief ESR_EL1 EC field value for unknown SME, SVE, SIMD or Floating pointer instruction exception
141: #define ESR_EL1_EC_TRAPPED_SME_SVE_SIMD_FP_ACCESS 0x07
142: /// @brief ESR_EL1 EC field value for trapped LD64<x> or ST64<x> instruction exception
143: #define ESR_EL1_EC_TRAPPED_LD64x_ST64x_ACCESS 0x0A
144: /// @brief ESR_EL1 EC field value for trapped MRRC instruction exception when coproc = 0x0C
145: #define ESR_EL1_EC_TRAPPED_MRRC_ACCESS_COPROC_0E 0x0C
146: /// @brief ESR_EL1 EC field value for branch target exception
147: #define ESR_EL1_EC_BRANCH_TARGET_EXCEPTION 0x0D
148: /// @brief ESR_EL1 EC field value for illegal executions state exception
149: #define ESR_EL1_EC_ILLEGAL_EXECUTION_STATE 0x0E
150: /// @brief ESR_EL1 EC field value for trapped SVC 32 bit instruction exception
151: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_32 0x11
152: /// @brief ESR_EL1 EC field value for trapped SVC 64 bit instruction exception
153: #define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_64 0x15
154: /// @brief ESR_EL1 EC field value for MCR or MRC 64 bit instruction exception
155: #define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_64 0x18
156: /// @brief ESR_EL1 EC field value for trapped SVE access exception
157: #define ESR_EL1_EC_TRAPPED_SVE_ACCESS 0x19
158: /// @brief ESR_EL1 EC field value for trapped TStart access exception
159: #define ESR_EL1_EC_TRAPPED_TSTART_ACCESS 0x1B
160: /// @brief ESR_EL1 EC field value for pointer authentication failure exception
161: #define ESR_EL1_EC_POINTER_AUTHENTICATION_FAILURE 0x1C
162: /// @brief ESR_EL1 EC field value for trapped SME access exception
163: #define ESR_EL1_EC_TRAPPED_SME_ACCESS 0x1D
164: /// @brief ESR_EL1 EC field value for granule protection check exception
165: #define ESR_EL1_EC_GRANULE_PROTECTION_CHECK 0x1E
166: /// @brief ESR_EL1 EC field value for instruction abort from lower EL exception
167: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL 0x20
168: /// @brief ESR_EL1 EC field value for instruction abort from same EL exception
169: #define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_SAME_EL 0x21
170: /// @brief ESR_EL1 EC field value for PC alignment fault exception
171: #define ESR_EL1_EC_PC_ALIGNMENT_FAULT 0x22
172: /// @brief ESR_EL1 EC field value for data abort from lower EL exception
173: #define ESR_EL1_EC_DATA_ABORT_FROM_LOWER_EL 0x24
174: /// @brief ESR_EL1 EC field value for data abort from same EL exception
175: #define ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL 0x25
176: /// @brief ESR_EL1 EC field value for SP alignment fault exception
177: #define ESR_EL1_EC_SP_ALIGNMENT_FAULT 0x27
178: /// @brief ESR_EL1 EC field value for trapped 32 bit FP instruction exception
179: #define ESR_EL1_EC_TRAPPED_FP_32 0x28
180: /// @brief ESR_EL1 EC field value for trapped 64 bit FP instruction exception
181: #define ESR_EL1_EC_TRAPPED_FP_64 0x2C
182: /// @brief ESR_EL1 EC field value for SError interrupt exception
183: #define ESR_EL1_EC_SERROR_INTERRUPT 0x2F
184: /// @brief ESR_EL1 EC field value for Breakpoint from lower EL exception
185: #define ESR_EL1_EC_BREAKPOINT_FROM_LOWER_EL 0x30
186: /// @brief ESR_EL1 EC field value for Breakpoint from same EL exception
187: #define ESR_EL1_EC_BREAKPOINT_FROM_SAME_EL 0x31
188: /// @brief ESR_EL1 EC field value for SW step from lower EL exception
189: #define ESR_EL1_EC_SW_STEP_FROM_LOWER_EL 0x32
190: /// @brief ESR_EL1 EC field value for SW step from same EL exception
191: #define ESR_EL1_EC_SW_STEP_FROM_SAME_EL 0x33
192: /// @brief ESR_EL1 EC field value for Watchpoint from lower EL exception
193: #define ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL 0x34
194: /// @brief ESR_EL1 EC field value for Watchpoint from same EL exception
195: #define ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL 0x35
196: /// @brief ESR_EL1 EC field value for 32 bit BKPT instruction exception
197: #define ESR_EL1_EC_BKPT_32 0x38
198: /// @brief ESR_EL1 EC field value for 64 bit BRK instruction exception
199: #define ESR_EL1_EC_BRK_64 0x3C
200: 
201: /// @brief ESR_EL1 ISS field bit shift
202: #define ESR_EL1_ISS_SHIFT 0
203: /// @brief ESR_EL1 ISS field bit mask
204: #define ESR_EL1_ISS_MASK  BITS(0,24)
205: /// @brief ESR_EL1 ISS field extraction
206: #define ESR_EL1_ISS(value) ((value >> ESR_EL1_ISS_SHIFT) & ESR_EL1_ISS_MASK)
```

- Line 42: We need to include the macro header for a new definition we're going to use
- Line 93: We add an instruction to retrieve the value of the `CurrentEL` register, which holds the current exception level
- Line 96: We add an instruction to retrieve the value of the `SPR_EL1` (Saved Program Status Register EL1), which hold information on the cause of an exception
- Line 99: We add an instruction to retrieve the value of the `ESR_EL1` (Exception Status Register EL1), which hold information on the cause of an exception
- Line 101-106: We define the M3:0 field in the `SPSR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 107-120: We define different values for this field.
Note that all exception levels are included, and have a prefix `SPSR_EL_`. However for SPSR_EL1, only the values for EL0t, EL1t, EL1h are possible
- Line 122-128: We define the EC field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS)
- Line 129-199: We define different values for this field. This is quite a list of different exception types
- Line 201-206: We define the ISS field in the `ESR_EL1` register for AArch64. See also [ARM registers](#ARM_REGISTERS).
We will not sum up all possibilities here, are each exception type has its own ISS field definition

### Exception.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONH}

We will define some values for different types of exceptions. This will not include FIQ or IRQ, as these will be handled differently later.

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

### ExceptionHandler.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONHANDLERH}

We will create a prototype for the exception handler.

Create the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
43: #include <baremetal/System.h>
44: 
45: /// @file
46: /// Exception handler function
47: 
48: /// @brief Exception abort frame
49: ///
50: /// Storage for register value in case of exception, in order to recover
51: struct AbortFrame
52: {
53:     /// @brief Exception Syndrome Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
54:     uint64	esr_el1;
55:     /// @brief Saved Program Status Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER
56:     uint64	spsr_el1;
57:     /// @brief General-purpose register, Link Register
58:     uint64	x30;
59:     /// @brief Exception Link Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
60:     uint64	elr_el1;
61:     /// @brief Stack Pointer (EL0). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
62:     uint64	sp_el0;
63:     /// @brief Stack Pointer (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
64:     uint64	sp_el1;
65:     /// @brief Fault Address Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
66:     uint64	far_el1;
67:     /// @brief Unused valuem used to align to 64 bytes
68:     uint64	unused;
69: }
70: /// @brief Just specifies the struct is packed
71: PACKED;
72: 
73: /// @brief Handles an unexpected exception, with the abort frame passed in.
74: ///
75: /// The exception handler is called from assembly code (ExceptionStub.S)
76: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_UNEXPECTED)
77: /// @param abortFrame  Filled in AbortFrame instance.
78: void UnexpectedHandler(uint64 exceptionID, AbortFrame* abortFrame);
79: 
80: /// @brief Handles a synchronous exception, with the abort frame passed in.
81: ///
82: /// The exception handler is called from assembly code (ExceptionStub.S)
83: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_SYNCHRONOUS)
84: /// @param abortFrame  Filled in AbortFrame instance.
85: void SynchronousExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);
86: 
87: /// @brief Handles a system error exception, with the abort frame passed in.
88: ///
89: /// The exception handler is called from assembly code (ExceptionStub.S)
90: /// @param exceptionID Exception type being thrown (in this case EXCEPTION_SYSTEM_ERROR)
91: /// @param abortFrame  Filled in AbortFrame instance.
92: void SystemErrorHandler(uint64 exceptionID, AbortFrame* abortFrame);
```

- Line 51-71: We create a packed struct `AbortFrame` which will hold all important information on entry. This information actually contains registers saved on the stack, as we'll see later
- Line 78: We declare the exception handler for an unexpected exception `UnexpectedHandler()` which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_UNEXPECTED), the second is a pointer to the `AbortFrame` which is actually the stack pointer value
- Line 85: We declare the exception handler for a synchronous exception (e.g. data exception, illegal instruction exception,break exception) `SynchronousExceptionHandler()` which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_SYNCHRONOUS), the second is a pointer to the `AbortFrame` which is actually the stack pointer value
- Line 92: We declare the exception handler for a system error exception `SystemErrorHandler()` (implementation defined) which will receive two parameters.
The first parameter is an exception ID which we will set (in this case EXCEPTION_SYSTEM_ERROR), the second is a pointer to the `AbortFrame` which is actually the stack pointer value

### ExceptionHandler.cpp {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONHANDLERCPP}

We will implement the exception handlers.

Create the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
44: #include <baremetal/System.h>
45: 
46: /// @file
47: /// Exception handler function implementation
48: 
49: /// @brief Define log name
50: LOG_MODULE("ExceptionHandler");
51: 
52: using namespace baremetal;
53: 
54: void UnexpectedHandler(uint64 exceptionID, AbortFrame* abortFrame)
55: {
56:     auto exceptionLevel = CurrentEL();
57:     LOG_NO_ALLOC_INFO("UnexpectedHandler at EL: %d", static_cast<int>(exceptionLevel));
58: }
59: 
60: void SynchronousExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
61: {
62:     uint32 esr_el1{};
63:     GetESR_EL1(esr_el1);
64:     uint32 esr_ec = ESR_EL1_EC(esr_el1);
65:     uint32 esr_iss = ESR_EL1_ISS(esr_el1);
66:     auto exceptionLevel = CurrentEL();
67:     LOG_NO_ALLOC_INFO("SynchronousHandler at EL: %d EC=%02x ISS=%07x", static_cast<int>(exceptionLevel), esr_ec, esr_iss);
68: }
69: 
70: void SystemErrorHandler(uint64 exceptionID, AbortFrame* abortFrame)
71: {
72:     auto exceptionLevel = CurrentEL();
73:     LOG_NO_ALLOC_INFO("SystemErrorHandler at EL: %d", static_cast<int>(exceptionLevel));
74: }
```

- Line 54-58: We implement the function `UnexpectedHandler()`, which simply prints the current exception level and some text
- Line 60-68: We implement the function `SynchronousExceptionHandler()`, which extracts two fields from the `ESR_EL1` register (using the macros and instructions we just added), and prints the values, next to the current exception.
- Line 70-74: We implement the function `SystemErrorHandler()`, which simply prints the current exception level and some text

Note that we use the `NO_ALLOC` versions for logging, as memory allocation may again throw exceptions.

### BCMRegisters.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_BCMREGISTERSH}

As the exception stub we're going to write needs to disable / enable interrupts, we need to define some of the interrupt controller registers for this.

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
47: #include <stdlib/Macros.h>
48: #ifdef __cplusplus
49: #include <stdlib/Types.h>
50: #endif
...
106: //---------------------------------------------
107: // Interrupt Controller
108: //---------------------------------------------
109: 
110: #if BAREMETAL_RPI_TARGET == 3
111: 
112: //---------------------------------------------
113: // RPI 3 Interrupt Controller
114: //---------------------------------------------
115: 
116: /// @brief Raspberry Pi Interrupt Control Registers base address. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
117: #define RPI_INTRCTRL_BASE               RPI_BCM_IO_BASE + 0x0000B000
118: 
119: /// @brief Raspberry Pi Interrupt Control basic IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
120: #define RPI_INTRCTRL_IRQ_BASIC_PENDING  reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000200)
121: /// @brief Raspberry Pi Interrupt Control register 1 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
122: #define RPI_INTRCTRL_IRQ_PENDING_1      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000204)
123: /// @brief Raspberry Pi Interrupt Control register 2 IRQ pending register. See @ref RASPBERRY_PI_MAILBOX
124: #define RPI_INTRCTRL_IRQ_PENDING_2      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000208)
125: /// @brief Raspberry Pi Interrupt Control FIQ enable register. See @ref RASPBERRY_PI_MAILBOX
126: #ifdef __cplusplus
127: #define RPI_INTRCTRL_FIQ_CONTROL        reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000020C)
128: #else
129: #define RPI_INTRCTRL_FIQ_CONTROL        (RPI_MAILBOX_BASE + 0x0000020C)
130: #endif
131: /// @brief Raspberry Pi Interrupt Control register 1 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
132: #define RPI_INTRCTRL_ENABLE_IRQS_1      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000210)
133: /// @brief Raspberry Pi Interrupt Control register 2 IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
134: #define RPI_INTRCTRL_ENABLE_IRQS_2      reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000214)
135: /// @brief Raspberry Pi Interrupt Control basic IRQ enable register. See @ref RASPBERRY_PI_MAILBOX
136: #define RPI_INTRCTRL_ENABLE_BASIC_IRQS  reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000218)
137: /// @brief Raspberry Pi Interrupt Control register 1 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
138: #define RPI_INTRCTRL_DISABLE_IRQS_1     reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000021C)
139: /// @brief Raspberry Pi Interrupt Control register 2 IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
140: #define RPI_INTRCTRL_DISABLE_IRQS_2     reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000220)
141: /// @brief Raspberry Pi Interrupt Control basic IRQ disable register. See @ref RASPBERRY_PI_MAILBOX
142: #define RPI_INTRCTRL_DISABLE_BASIC_IRQS reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x00000224)
143: 
144: #else
145: //---------------------------------------------
146: // RPI 4 / 5 Interrupt Controller (GIC-400)
147: //---------------------------------------------
148: 
149: #if BAREMETAL_RPI_TARGET == 4
150: #define RPI_GIC_BASE                    ARM_IO_BASE
151: #else
152: #define RPI_GIC_BASE                    (ARM_IO_BASE + 0x3FF8000UL)
153: #endif
154: #define RPI_GICD_BASE                   (RPI_GIC_BASE + 0x00001000)
155: #define RPI_GICC_BASE                   (RPI_GIC_BASE + 0x00002000)
156: #define RPI_GIC_END                     (RPI_GIC_BASE + 0x00007FFF)
157: 
158: // The following definitions are valid for non-secure access,
159: // if not labeled otherwise.
160: 
161: // GIC distributor registers
162: #define RPI_GICD_CTLR                   reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0000)
163: #define RPI_GICD_CTLR_DISABLE           (0 << 0)
164: #define RPI_GICD_CTLR_ENABLE            (1 << 0)
165: // secure access
166: #define RPI_GICD_CTLR_ENABLE_GROUP0     (1 << 0)
167: #define RPI_GICD_CTLR_ENABLE_GROUP1     (1 << 1)
168: #define RPI_GICD_IGROUPR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0080)   // Secure access for group 0
169: #define RPI_GICD_ISENABLER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0100)
170: #define RPI_GICD_ICENABLER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0180)
171: #define RPI_GICD_ISPENDR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0200)
172: #define RPI_GICD_ICPENDR0               reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0280)
173: #define RPI_GICD_ISACTIVER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0300)
174: #define RPI_GICD_ICACTIVER0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0380)
175: #define RPI_GICD_IPRIORITYR0            reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0400)
176: #define RPI_GICD_IPRIORITYR_DEFAULT     0xA0
177: #define RPI_GICD_IPRIORITYR_FIQ         0x40
178: #define RPI_GICD_ITARGETSR0             reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0800)
179: #define RPI_GICD_ITARGETSR_CORE0        (1 << 0)
180: #define RPI_GICD_ICFGR0                 reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0C00)
181: #define RPI_GICD_ICFGR_LEVEL_SENSITIVE  (0 << 1)
182: #define RPI_GICD_ICFGR_EDGE_TRIGGERED   (1 << 1)
183: #define RPI_GICD_SGIR                   reinterpret_cast<regaddr>(RPI_GICD_BASE + 0x0F00)
184: #define RPI_GICD_SGIR_SGIINTID__MASK    0x0F
185: #define RPI_GICD_SGIR_CPU_TARGET_LIST__SHIFT    16
186: #define RPI_GICD_SGIR_TARGET_LIST_FILTER__SHIFT 24
187: 
188: // GIC CPU interface registers
189: #define RPI_GICC_CTLR                   reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0000)
190: #define RPI_GICC_CTLR_DISABLE           (0 << 0)
191: #define RPI_GICC_CTLR_ENABLE            (1 << 0)
192: // secure access
193: #define RPI_GICC_CTLR_ENABLE_GROUP0     (1 << 0)
194: #define RPI_GICC_CTLR_ENABLE_GROUP1     (1 << 1)
195: #define RPI_GICC_CTLR_FIQ_ENABLE        (1 << 3)
196: #define RPI_GICC_PMR                    reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0004)
197: #define RPI_GICC_PMR_PRIORITY           (0xF0 << 0)
198: #define RPI_GICC_IAR                    reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x000C)
199: #define RPI_GICC_IAR_INTERRUPT_ID_MASK  0x3FF
200: #define RPI_GICC_IAR_CPUID_SHIFT        10
201: #define RPI_GICC_IAR_CPUID_MASK         (3 << RPI_GICC_IAR_CPUID_SHIFT)
202: #define RPI_GICC_EOIR                   reinterpret_cast<regaddr>(RPI_GICC_BASE + 0x0010)
203: #define RPI_GICC_EOIR_EOIINTID_MASK     0x3FF
204: #define RPI_GICC_EOIR_CPUID_SHIFT       10
205: #define RPI_GICC_EOIR_CPUID_MASK        (3 << RPI_GICC_EOIR_CPUID_SHIFT)
206: 
207: #ifdef __cplusplus
208: #define RPI_INTRCTRL_FIQ_CONTROL        reinterpret_cast<regaddr>(RPI_MAILBOX_BASE + 0x0000020C)
209: #else
210: #define RPI_INTRCTRL_FIQ_CONTROL        (RPI_MAILBOX_BASE + 0x0000020C)
211: #endif
212: 
213: #endif
214: 
...
```

- Line 48-50: As this header is also included by the exception stub coming up next, we need to protect against specific C/C++ definitions, so we only include the type definitions header if we are building for C++
- Line 106-213: We add definitions for registers relating to the Raspberry Pi interrupt control register.
Some of these are used by the exception stub code. We'll get to these registers in a later tutorial

### ExceptionStub.S {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_EXCEPTIONSTUBS}

Next we need to write some assembly to implement the exception vectors.

Create the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
115:     bic     x0, x0, #0xF                // Clear bit 0
116:     mov     x1, #9
117:     orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
118:     msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
119:     eret                                // Move to EL2h
120:
121:     .data
122:
123:     .align  3
124:
125: //*************************************************
126: // Abort stubs
127: //*************************************************
128:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   UnexpectedHandler
129:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  SynchronousExceptionHandler
130:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, SystemErrorHandler
131: 
132: // End
```

- Line 48-54: We create a macro `vector` to define a vector in the vector table.
Every vector is aligned to 128 bytes, hence the statement `.align 7` which means align by 7 bits, or 128 (1 << 7) bytes.
The only code we add is a jump to the current stub, passed as `handler`
- Line 56-77: We create a macro `stub` for a generic stub, which saves the system registers `ESR_EL11`, `SPSR_EL1`, `ELR_EL1`, `SP_EL0` and `FAR_EL1` and the registers `x30` (LR) and `SP`, in general purpose registers x0-x6.
These are then stored on the stack, every time decreasing the stack pointer by 16 bytes.
Then the exception type passed through `exception` is stored in x0, and the stack pointer in x1.
Finally the exception handler passed through `handler` is branched to.
This will effectively call the exception handler function `void handler(uint64 exceptionID, AbortFrame* abortFrame)` with `exceptionID` = x0, `abortFrame` = x1.
See below for a more detailed explanation of how the abort frame is set up
- Line 79-108: We implement the actual exception vector table.
This has 16 entries using the macro `vector`, each aligned to 128 bytes, while the complete table is aligned to 2048 (1 << 11) bytes, hence `.align 11`
As you can see, we are not anticipating any IRQ or FIQ interrupt, nor any AArch32 exceptions. When an exception from EL1 to EL2 occurs, we use the Hypervisor call stub `HVCStub` for this
- Line 113-119: We implement the hypervisor call stub `HVCStub`.
The function reads the `SPSR_EL2` system register, and sets it to switch to EL2h exception level, then moves to that that exception level using `eret`.
This also resets the state of interrupt enables, etc.
Effectively, we switch back to EL2h mode with this
- Line 123-129: We define the `s_fiqData` structure, aligned to 8 bytes. We'll discuss in detail this later, but for now, this is data related to a FIQ being handled
- Line 134: We declare unexpected exceptions stub using the `stub` macro, and set its handler to `UnexpectedHandler()`
- Line 135: We declare synchronous exceptions stub using the `stub` macro, and set its handler to `SynchronousExceptionHandler()`
- Line 136: We declare system errors stub using the `stub` macro, and set its handler to `SystemErrorHandler()`

Note that this means we need to defined the functions `UnexpectedHandler()`, `SynchronousExceptionHandler()`, `SystemErrorHandler()`

### System.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_SYSTEMH}

We'll add a function to extract the current exception level, to be printed for information.

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

### System.cpp {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_SYSTEMCPP}

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

### Update application code {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_UPDATE_APPLICATION_CODE}

Ok, so now we creation the infrastructure for dealing with processor exceptions, let's see whether they work.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/ExceptionHandler.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/System.h>
7: #include <baremetal/Timer.h>
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

### Update project configuration {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_UPDATE_PROJECT_CONFIGURATION}

As we added a new source file, we'll update the project CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionStub.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Malloc.cpp
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
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
55:     )
56: 
57: set(PROJECT_INCLUDES_PUBLIC
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Exception.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Iterator.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Malloc.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Synchronization.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
91:     )
92: set(PROJECT_INCLUDES_PRIVATE )
```

### CMake file {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_CMAKE_FILE}

Let's switch off the memory debugging for now, to avoid cluttering the output.

Update the file `CMakeLists.txt`
```cmake
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
69: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
70: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
71: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
72: 
```

### baremetal.ld {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_BAREMETALLD}

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

### Startup.S {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_STARTUPS}

We have defined the exception vector table, but we need to install the vector table in order for it to be used.
This is done by setting the VBAR_EL1 register to the address of the vector table. We'll also set VBAR_EL2 to this address, to enable running our code when we move back to EL2 on some exceptions.
Notice that we also added some more explanation to the comments.

Update the file `code/libraries/baremetal/src/Startup.S`

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
44: /// @file
45: /// System startup assembly code
46: 
47: .macro armv8_switch_to_el1_m, xreg1, xreg2
48: 
49:     // Initialize Generic Timers
50:     mrs \xreg1, cnthctl_el2             // Read Counter-timer Hypervisor Control register (EL2)
51:     orr \xreg1, \xreg1, #0x3            // Enable EL1 access to timers. Sets bit 1 (EL0VCTEN) and 0 (EL0PCTEN). This disables trapping access to CNTVCT_EL0, CNTFRQ_EL0, CNTPCT_EL0, CNTFRQ_EL0 in EL0
52:     msr cnthctl_el2, \xreg1             // Write Counter-timer Hypervisor Control register (EL2)
53:     msr cntvoff_el2, xzr                // Write 0 to Counter-timer Virtual Offset register. This resets the offset of the virtual timer
54: 
55:     // Initilize MPID/MPIDR registers
56:     mrs \xreg1, midr_el1                // Read Main ID Register (EL1)
57:     mrs \xreg2, mpidr_el1               // Read Multiprocessor Affinity Register (EL1)
58:     msr vpidr_el2, \xreg1               // Write Virtualization Processor ID Register (EL2)
59:     msr vmpidr_el2, \xreg2              // Virtualization Multiprocessor ID Register
60: 
61:     // Disable coprocessor traps
62:     // Set RES1 bits (13,9,7..0) + TSM,TZ to 1
63:     // Set RES0 bits (63..32,29..21,19..14,11) +
64:     // TCPAC,TAM,TTA,TFP, to 0
65:     // Enables access to CPACR_EL1, AMUSERENR_EL0, AMCFGR_EL0, AMCGCR_EL0, AMCNTENCLR0_EL0, AMCNTENCLR1_EL0,
66:     // AMCNTENSET0_EL0, AMCNTENSET1_EL0, AMCR_EL0, AMEVCNTR0<n>_EL0, AMEVCNTR1<n>_EL0, AMEVTYPER0<n>_EL0, AMEVTYPER1<n>_EL0, FPCR, FPSR, FPEXC32_EL2)
67:     mov \xreg1, #0x33ff
68:     msr cptr_el2, \xreg1                // Write Architectural Feature Trap Register (EL2). Disable coprocessor traps to EL2
69:     msr hstr_el2, xzr                   // Write Hypervisor System Trap Register (EL2). Disable coprocessor traps to EL2
70:     mov \xreg1, #3 << 20
71:     msr cpacr_el1, \xreg1               // Enable FP/SIMD at EL1
72: 
73:     // Initialize HCR_EL2
74:     mov \xreg1, #(1 << 31)              // 64bit EL1
75:     msr hcr_el2, \xreg1                 // Write Hypervisor Configuration Register (EL2). Set bit 31 (RW), which enforces EL1 to be in AArch64 mode
76: 
77:     // SCTLR_EL1 initialization
78:     //
79:     // setting RES1 bits (29,28,23,22,20,11) to 1
80:     // and RES0 bits (31,30,27,21,17,13,10,6) +
81:     // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
82:     // CP15BEN,SA0,SA,C,A,M to 0
83:     mov \xreg1, #0x0800
84:     movk \xreg1, #0x30d0, lsl #16
85:     msr sctlr_el1, \xreg1               // Write System Control Register (EL1)
86: 
87:     // Return to the EL1_SP1 mode from EL2
88:     // Set RES1 bits (none) + D,A,I,F,M[2] to 1
89:     // Set RES0 bits (63..32,27..26,19..14) +
90:     // N,Z,C,V,TCO,DIT,UAO,PAN,SS,IL,ALLINT,SSBS,BTYPE,M[4:3],M[1:0] to 0
91:     // Save and restore D,A,I,F bits, move to EL1t
92:     mov \xreg1, #0x3c4
93:     msr spsr_el2, \xreg1                // Write Saved Program Status Register (EL2), EL1_SP0 | D | A | I | F
94:     adr \xreg1, label1                  // Set return address when EL2 exception occurs
95:     msr elr_el2, \xreg1                 // Write Exception Link Register (EL2)
96:     eret
97: label1:
98: .endm
99: 
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
126: 
```

Line 111-112: We set the vector table for EL2 to the address of the vector table
Line 120-121: We set the vector table for EL1 to the address of the vector table

### Configuring, building and debugging {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_HANDLING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:18)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tInfo   SynchronousHandler at EL: 1 EC=3C ISS=00003E8 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=21 ISS=0000000 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=22 ISS=0000000 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=22 ISS=0000000 (ExceptionHandler:67)
```

If we cause a memory violation, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:18)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mInfo   SynchronousHandler at EL: 1 EC=25 ISS=0000000 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=21 ISS=0000000 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=22 ISS=0000000 (ExceptionHandler:67)
Info   SynchronousHandler at EL: 1 EC=22 ISS=0000000 (ExceptionHandler:67)
```

The exceptions will continue to keep coming, as we we jump back to the code generating the exception.

As can be found in [Arm Architecture Registers](pdf/arm-architecture-registers.pdf), page 570, exception code 3C stands for "BRK instruction execution in AArch64 state", and on page 571, exception code 25 stands for "Data Abort taken without a change in Exception level."

## Exception system - Step 2 {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2}

To improve our code a little, let's create a class for handling exceptions.

### ExceptionHandler.h {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONHANDLERH}

We'll add the class `ExceptionSystem` and change the exception handling to a single function `ExceptionHandler()`.

Update the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
...
42: #include <stdlib/Types.h>
43: #include <baremetal/System.h>
...
48: #ifdef __cplusplus
49: extern "C" {
50: #endif
51: 
52: /// <summary>
53: /// Exception abort frame
54: ///
55: /// Storage for register value in case of exception, in order to recover
56: /// </summary>
57: struct AbortFrame
58: {
59:     /// @brief Exception Syndrome Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
60:     uint64	esr_el1;
61:     /// @brief Saved Program Status Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER
62:     uint64	spsr_el1;
63:     /// @brief General-purpose register, Link Register
64:     uint64	x30;
65:     /// @brief Exception Link Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
66:     uint64	elr_el1;
67:     /// @brief Stack Pointer (EL0). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
68:     uint64	sp_el0;
69:     /// @brief Stack Pointer (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
70:     uint64	sp_el1;
71:     /// @brief Fault Address Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
72:     uint64	far_el1;
73:     /// @brief Unused valuem used to align to 64 bytes
74:     uint64	unused;
75: }
76: /// @brief Just specifies the struct is packed
77: PACKED;
78: 
79: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);
80: 
81: #ifdef __cplusplus
82: }
83: #endif
84: 
85: namespace baremetal {
86: 
87: /// <summary>
88: /// Exception handling system. Handles ARM processor exceptions
89: ///
90: /// This is a singleton class, created as soon as GetExceptionSystem() is called
91: /// </summary>
92: class ExceptionSystem
93: {
94:     friend ExceptionSystem& GetExceptionSystem();
95: 
96: private:
97:     ExceptionSystem();
98: 
99: public:
100:     ~ExceptionSystem();
101: 
102:     void Throw(unsigned exceptionID, AbortFrame* abortFrame);
103: };
104: 
105: /// <summary>
106: /// Injectable exception handler
107: /// </summary>
108: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
109: /// <param name="abortFrame">Stored state information at the time of exception</param>
110: /// <returns>ReturnCode::ExitHalt if the system should be halted, ReturnCode::ExitReboot if the system should reboot</returns>
111: using ExceptionPanicHandler = ReturnCode(unsigned exceptionID, AbortFrame* abortFrame);
112: 
113: const char* GetExceptionType(unsigned exceptionID);
114: 
115: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler);
116: 
117: ExceptionSystem& GetExceptionSystem();
118: 
119: } // namespace baremetal
```

- Line 75: We change the three exception handlers to a single one, `ExceptionHandler()`.
Notice that we mark it a `C` function, in order for the linker to be able to link to the assembly code
- Line 84-95: We declare the class `ExceptionSystem`
  - Line 86: We make the function `GetExceptionSystem()` a friend to the class, so we can use it to create the singleton instance
  - Line 89: We declare the private (default) constructor, so only the `GetExceptionSystem()` function can be used to create and instance
  - Line 92: We declare the destructor
  - Line 94: We declare the method `Throw()` which will be called by `ExceptionHandler()`
- Line 103: We declare the type for an injectable exception panic handler.
This will be call by the `Throw()` method of `ExceptionSystem` if set, and will return true if the system should be halted, false if not
- Line 105: We declare a helper function `GetExceptionType()` to convert an exception type into a string
- Line 107: We declare a method to set the exception panic handler, returning the old installed handler, if any
- Line 109: We declare the function `GetExceptionSystem()` to create the singleton instance of the `ExceptionSystem` class, if needed, and rreturn it

### ExceptionHandler.cpp {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONHANDLERCPP}

We'll implement the newly added `ExceptionSystem` class and reimplement the ExceptionHandler() function.

Update the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
...
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/Assert.h>
44: #include <baremetal/Logger.h>
45: #include <baremetal/System.h>
...
55: /// <summary>
56: /// Handles an exception, with the abort frame passed in.
57: ///
58: /// The exception handler is called from assembly code (ExceptionStub.S)
59: /// </summary>
60: /// <param name="exceptionID">Exception type being thrown (one of EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS, EXCEPTION_SYSTEM_ERROR)</param>
61: /// <param name="abortFrame">Filled in AbortFrame instance</param>
62: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
63: {
64:     baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
65: }
66: 
67: namespace baremetal {
68: 
69: /// <summary>
70: /// Exception panic handler
71: ///
72: /// If set, the panic handler is called first, and if it returns false, the system is not halted (as it would by default)
73: /// </summary>
74: static ExceptionPanicHandler* s_exceptionPanicHandler{};
75: 
76: /// <summary>
77: /// Names exception types
78: ///
79: /// Order must match exception identifiers in baremetal/Exception.h
80: /// </summary>
81: static const char* s_exceptionName[] =
82: {
83:     "Unexpected exception",
84:     "Synchronous exception",
85:     "System error"
86: };
87: 
88: /// <summary>
89: /// Constructor
90: ///
91: /// Note that the constructor is private, so GetExceptionSystem() is needed to instantiate the exception handling system
92: /// </summary>
93: ExceptionSystem::ExceptionSystem()
94: {
95: }
96: 
97: /// <summary>
98: /// Destructor
99: /// </summary>
100: ExceptionSystem::~ExceptionSystem()
101: {
102: }
103: 
104: /// <summary>
105: /// Throw an exception to the exception system
106: /// </summary>
107: /// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
108: /// <param name="abortFrame">Stored state information at the time of exception</param>
109: void ExceptionSystem::Throw(unsigned exceptionID, AbortFrame* abortFrame)
110: {
111:     assert(abortFrame != nullptr);
112: 
113:     uint64 sp = abortFrame->sp_el0;
114:     if (SPSR_EL1_M30(abortFrame->spsr_el1) == SPSR_EL_M30_EL1h)		// EL1h mode?
115:     {
116:         sp = abortFrame->sp_el1;
117:     }
118: 
119:     uint64 EC = ESR_EL1_EC(abortFrame->esr_el1);
120:     uint64 ISS = ESR_EL1_ISS(abortFrame->esr_el1);
121: 
122:     uint64 FAR = 0;
123:     if ((ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL)
124:         || (ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL))
125:     {
126:         FAR = abortFrame->far_el1;
127:     }
128: 
129:     ReturnCode action = ReturnCode::ExitHalt;
130:     if (s_exceptionPanicHandler != nullptr)
131:     {
132:         action = (*s_exceptionPanicHandler)(exceptionID, abortFrame);
133:     }
134: 
135:     if (action == ReturnCode::ExitHalt)
136:         LOG_PANIC("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
137:             s_exceptionName[exceptionID],
138:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
139:     else
140:     {
141:         LOG_ERROR("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
142:             s_exceptionName[exceptionID],
143:             abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
144:         GetSystem().Reboot();
145:     }
146: }
147: 
148: /// @brief Convert exception ID to a string
149: /// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
150: /// @return String representation of exception ID
151: const char* GetExceptionType(unsigned exceptionID)
152: {
153:     return s_exceptionName[exceptionID];
154: }
155: 
156: /// @brief Register an exception callback function, and return the previous one.
157: /// @param handler      Exception handler callback function to register
158: /// @return Previously set exception handler callback function
159: ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler)
160: {
161:     auto result = s_exceptionPanicHandler;
162:     s_exceptionPanicHandler = handler;
163:     return result;
164: }
165: 
166: /// <summary>
167: /// Construct the singleton exception system instance if needed, and return a reference to the instance
168: ///
169: /// This is a friend function of class ExceptionSystem
170: /// </summary>
171: /// <returns>Reference to the singleton exception system instance</returns>
172: ExceptionSystem& GetExceptionSystem()
173: {
174:     static ExceptionSystem system;
175:     return system;
176: }
177: 
178: } // namespace baremetal
```

- Line 61-64: We implement the function `ExceptionHandler()` by calling the `Throw()` method on the singleton `ExceptionSystem` instance
- Line 73: We define a static variable `s_exceptionPanicHandler` to hold the injected exception panic handler pointer.
It is initialized as nullptr, meaning no handler is installed
- Line 80-85: We define the mapping `s_exceptionName` from exception type to string
- Line 92-94: We implement the `ExceptionSystem` constructor
- Line 99-101: We implement the `ExceptionSystem` destructor
- Line 108-145: We implement the method `Throw()`
  - Line 112-116: We determine the stack pointer at the time of the exception, which is the EL0 stack pointer, unless the exception level was EL1h (We can only be in exception level EL0 or EL1, as we specifically move to EL1 in the startup code)
  - Line 118-119: We extract the EC and ISS fields from the ESR_EL1 register value
  - Line 121-126: If the exception has an exception code relating to an instruction abort, a data abort or a watchpoint, we get the value of the Fault Address Register, which holds the address which generated the exception
  - Line 128-132: We call the exception panic handler if installed, setting its return value as the return code. If no handler is installed, the default is to halt
  - Line 134-144: If we need to halt, we perform a panic log (which will halt the system), otherwise we log an error and reboot
- Line 150-153: We implement the function `GetExceptionType()`
- Line 158-163: We implement the function `RegisterPanicHandler()`, which sets the `s_exceptionPanicHandler` variable, and returns the original value of this variable
- Line 171-175: We implement the function `GetExceptionSystem()` in the by now common way

### ExceptionStub.S {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_EXCEPTIONSTUBS}

We'll now map the exception vector to the newly added `ExceptionHandler()` function.

Update the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
...
125: //*************************************************
126: // Abort stubs
127: //*************************************************
128:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
129:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
130:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
131: 
132: // End
```

We now changed the functions called for the different types of exceptions to a single one, `ExceptionHandler`

### Update application code {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_UPDATE_APPLICATION_CODE}

Let's implement and use an exception panic handler, that forces the system to reboot on exception

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/ExceptionHandler.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/System.h>
7: #include <baremetal/Timer.h>
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

### Update project configuration {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_UPDATE_PROJECT_CONFIGURATION}

As no files were added, we don't need to update the project CMake file.

### Configuring, building and debugging {#TUTORIAL_17_EXCEPTIONS_EXCEPTION_SYSTEM___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

If we cause a trap, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
tError  Synchronous exception (PC 00000000000808D8, EC 000000000000003C, ISS 00000000000003E8, FAR 0000000000000000, SP 000000000029FFB0, LR 00000000000808CC, SPSR 0000000060000304) (ExceptionHandler:141)
Info   Reboot (System:152)
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

If we cause a memory violation, the output will be:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
mError  Synchronous exception (PC 00000000000808EC, EC 0000000000000025, ISS 0000000000000000, FAR FFFFFFFFFF000000, SP 000000000029FFB0, LR 00000000000808CC, SPSR 0000000060000304) (ExceptionHandler:141)
Info   Reboot (System:152)
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:23)
Press r to reboot, h to halt, t to cause a trap, m to cause a memory violation
```

In both cases, the system reboots.

Next: [18-interrupts](18-interrupts.md)
