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

## Interrupt handling - Step 1 {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1}

The ARM processor supports two different kinds of interrupts:

- IRQ: Normal interrupts
- FIQ: Fast interrupts

FIQ is a specialized type of interrupt request, which is a standard technique used in computer CPUs to deal with events that need to be processed as they occur, such as receiving data from a network card, or keyboard or mouse actions.
FIQs are specific to the ARM architecture, which supports two types of interrupts; FIQs for fast, low-latency interrupt handling, and standard interrupt requests (IRQs), for more general interrupts.

A FIQ takes priority over an IRQ in an ARM system. Only one FIQ source at a time is supported. 
This helps reduce interrupt latency as the interrupt service routine can be executed directly without determining the source of the interrupt.
A context save is not required for servicing a FIQ since it has its own set of banked registers. This reduces the overhead of context switching.

FIQs are often used for data transfers such as DMA operations.

### ExceptionStub.S {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_EXCEPTIONSTUBS}

In order to support FIQ and IRQ interrupts, we need to update the exception assembly code.

Update the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
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
156:     bl      \handler                    // Call interrupt handler
157: 
158:     ldr     x0, [sp], #16               // Restore x0-x28 from stack
159:     ldp     x1, x2, [sp], #16
160:     ldp     x3, x4, [sp], #16
161:     ldp     x5, x6, [sp], #16
162:     ldp     x7, x8, [sp], #16
163:     ldp     x9, x10, [sp], #16
164:     ldp     x11, x12, [sp], #16
165:     ldp     x13, x14, [sp], #16
166:     ldp     x15, x16, [sp], #16
167:     ldp     x17, x18, [sp], #16
168:     ldp     x19, x20, [sp], #16
169:     ldp     x21, x22, [sp], #16
170:     ldp     x23, x24, [sp], #16
171:     ldp     x25, x26, [sp], #16
172:     ldp     x27, x28, [sp], #16
173: #ifdef BAREMETAL_SAVE_VFP_REGS_ON_IRQ
174:     ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
175:     ldp     q2, q3, [sp], #32
176:     ldp     q4, q5, [sp], #32
177:     ldp     q6, q7, [sp], #32
178:     ldp     q8, q9, [sp], #32
179:     ldp     q10, q11, [sp], #32
180:     ldp     q12, q13, [sp], #32
181:     ldp     q14, q15, [sp], #32
182:     ldp     q16, q17, [sp], #32
183:     ldp     q18, q19, [sp], #32
184:     ldp     q20, q21, [sp], #32
185:     ldp     q22, q23, [sp], #32
186:     ldp     q24, q25, [sp], #32
187:     ldp     q26, q27, [sp], #32
188:     ldp     q28, q29, [sp], #32
189:     ldp     q30, q31, [sp], #32
190: #endif // BAREMETAL_SAVE_VFP_REGS_ON_IRQ
191: 
192:     msr     DAIFSet, #1                 // Disable FIQ
193:     ldp     x29, x30, [sp], #16         // Restore from stack
194:     msr     elr_el1, x29                // Restore Exception Link Register (EL1)
195:     msr     spsr_el1, x30               // Restore Saved Program Status Register (EL1)
196:     ldp     x29, x30, [sp], #16         // restore x29, x30 from stack
197: 
198:     eret                                // Restore previous EL
199: 
200: .endm
201: 
202: //*************************************************
203: // FIQ stub
204: //*************************************************
205:     .globl  FIQStub
206: FIQStub:
207: #ifdef BAREMETAL_SAVE_VFP_REGS_ON_FIQ
208:     stp     q30, q31, [sp, #-32]!       // Save q0-q31 onto stack
209:     stp     q28, q29, [sp, #-32]!
210:     stp     q26, q27, [sp, #-32]!
211:     stp     q24, q25, [sp, #-32]!
212:     stp     q22, q23, [sp, #-32]!
213:     stp     q20, q21, [sp, #-32]!
214:     stp     q18, q19, [sp, #-32]!
215:     stp     q16, q17, [sp, #-32]!
216:     stp     q14, q15, [sp, #-32]!
217:     stp     q12, q13, [sp, #-32]!
218:     stp     q10, q11, [sp, #-32]!
219:     stp     q8, q9, [sp, #-32]!
220:     stp     q6, q7, [sp, #-32]!
221:     stp     q4, q5, [sp, #-32]!
222:     stp     q2, q3, [sp, #-32]!
223:     stp     q0, q1, [sp, #-32]!
224: #endif // BAREMETAL_SAVE_VFP_REGS_ON_FIQ
225:     stp     x29, x30, [sp, #-16]!       // Save x0-x28 onto stack
226:     stp     x27, x28, [sp, #-16]!
227:     stp     x25, x26, [sp, #-16]!
228:     stp     x23, x24, [sp, #-16]!
229:     stp     x21, x22, [sp, #-16]!
230:     stp     x19, x20, [sp, #-16]!
231:     stp     x17, x18, [sp, #-16]!
232:     stp     x15, x16, [sp, #-16]!
233:     stp     x13, x14, [sp, #-16]!
234:     stp     x11, x12, [sp, #-16]!
235:     stp     x9, x10, [sp, #-16]!
236:     stp     x7, x8, [sp, #-16]!
237:     stp     x5, x6, [sp, #-16]!
238:     stp     x3, x4, [sp, #-16]!
239:     stp     x1, x2, [sp, #-16]!
240:     str     x0, [sp, #-16]!
241: 
242:     ldr     x2, =s_fiqData
243:     ldr     x1, [x2]                    // Get s_fiqData.handler
244:     cmp     x1, #0                      // Is handler set?
245:     b.eq    no_fiq_handler
246:     ldr     x0, [x2, #8]                // Get s_fiqData.param
247:     blr     x1                          // Call handler
248: 
249: restore_after_fiq_handler:
250:     ldr     x0, [sp], #16               // Restore x0-x28 from stack
251:     ldp     x1, x2, [sp], #16
252:     ldp     x3, x4, [sp], #16
253:     ldp     x5, x6, [sp], #16
254:     ldp     x7, x8, [sp], #16
255:     ldp     x9, x10, [sp], #16
256:     ldp     x11, x12, [sp], #16
257:     ldp     x13, x14, [sp], #16
258:     ldp     x15, x16, [sp], #16
259:     ldp     x17, x18, [sp], #16
260:     ldp     x19, x20, [sp], #16
261:     ldp     x21, x22, [sp], #16
262:     ldp     x23, x24, [sp], #16
263:     ldp     x25, x26, [sp], #16
264:     ldp     x27, x28, [sp], #16
265:     ldp     x29, x30, [sp], #16
266: #ifdef BAREMETAL_SAVE_VFP_REGS_ON_FIQ
267:     ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
268:     ldp     q2, q3, [sp], #32
269:     ldp     q4, q5, [sp], #32
270:     ldp     q6, q7, [sp], #32
271:     ldp     q8, q9, [sp], #32
272:     ldp     q10, q11, [sp], #32
273:     ldp     q12, q13, [sp], #32
274:     ldp     q14, q15, [sp], #32
275:     ldp     q16, q17, [sp], #32
276:     ldp     q18, q19, [sp], #32
277:     ldp     q20, q21, [sp], #32
278:     ldp     q22, q23, [sp], #32
279:     ldp     q24, q25, [sp], #32
280:     ldp     q26, q27, [sp], #32
281:     ldp     q28, q29, [sp], #32
282:     ldp     q30, q31, [sp], #32
283: #endif // BAREMETAL_SAVE_VFP_REGS_ON_FIQ
284: 
285:     eret                                // Restore previous EL
286: 
287: no_fiq_handler:
288:     ldr     x1, =RPI_INTRCTRL_FIQ_CONTROL // Disable FIQ (if handler is not set)
289:     mov     w0, #0
290:     str     w0, [x1]
291:     b       restore_after_fiq_handler
292: 
293: /*
294:  * HVC stub
295:  */
296: HVCStub:                                // Return to EL2h mode
297:     mrs     x0, spsr_el2                // Read Saved Program Status Register (EL2)
298:     bic     x0, x0, #0xF                // Clear bit 3:0
299:     mov     x1, #9
300:     orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
301:     msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
302:     eret                                // Move to EL2h
303: 
304:     .data
305: 
306:     .align  3
307: 
308:     .globl  s_fiqData
309: s_fiqData:                              // Matches FIQData:
310:     .quad   0                           // handler
311:     .quad   0                           // param
312:     .word   0                           // fiqID (unused)
313: 
314: //*************************************************
315: // Abort stubs
316: //*************************************************
317:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
318:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
319:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
320:     irq_stub    IRQStub,                                    InterruptHandler
321: 
322: // End
```

- Line 113-200: We create a macro `irq_stub` for an IRQ interrupt, which saves almost all registers, depending on the define `BAREMETAL_SAVE_VFP_REGS_ON_IRQ`.
These are then stored on the stack, every time decreasing the stack pointer by 32 or 16 bytes, depending on the registers.
The FIQ interrupts are enabled while the interrupt is being handled, to allow for priority.
After saving the registers the interrupt handler passed through `handler` is called, and after than the registers are restored.
Then FIQ interrupts are disabled again, and the stub returns to the state before the interrupt using `eret`.
This also reset the state of interrupt enables, etc. as well as the exception level. Note that even though we disable FIQ here, returning to the original exception level may enable them again
- Line 205-291: We implement the FIQ interrupt stub `FIQStub`. This is similar to the normal interrupt stub, however this is not a macro.
Also, the pointer to the handler is retrieved from a memory area structure `s_fiqData`, as well as the parameter to pass to the handler.
If no handler was set no function is called, and the registers are simply restored
- Line 306-312: We define the structure s_fiqData, which is 8 byte aligned
- Line 320: We declare interrupt stub using the `irq_stub` macro, and set its handler to `InterruptHandler()`

### ExceptionHandler.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_EXCEPTIONHANDLERH}

We will move the `InterruptHandler()` function to a new source file, so let's remove it from the exception handler code first.

Update the file `code/libraries/baremetal/include/baremetal/ExceptionHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ExceptionHandler.h
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

- Line 81: We removed the `InterruptHandler()` declaration

### ExceptionHandler.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_EXCEPTIONHANDLERCPP}

We'll also remove the `InterruptHandler()` function from the source code.

Update the file `code/libraries/baremetal/src/ExceptionHandler.cpp`

```cpp
File: code/libraries/baremetal/src/ExceptionHandler.cpp
...
54: // <summary>
55: /// Handles an exception, with the abort frame passed in.
56: ///
57: /// The exception handler is called from assembly code (ExceptionStub.S)
58: /// </summary>
59: /// <param name="exceptionID">Exception type being thrown (one of EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS, EXCEPTION_SYSTEM_ERROR)</param>
60: /// <param name="abortFrame">Filled in AbortFrame instance</param>
61: void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
62: {
63:     baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
64: }
65: 
```

- Line 66: We removed the `InterruptHandler()` implementation

### InterruptHandler.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERH}

We will add the `InterruptHandler()` function.
Similar to waht we did for exceptions, we will also declare a class `InterruptSystem`, which is a singleton, and has its own implementation for `InterruptHandler()`.
We'll only create the bare minimum for this calss for now, but we'll extend it soon.

Create the file `code/libraries/baremetal/include/baremetal/InterruptHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptHandler.h
File: d:\Projects\baremetal\tutorial\20-interrupts\code\libraries\baremetal\include\baremetal\InterruptHandler.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : InterruptHandler.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Interrupt handler
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
42: #include <baremetal/Macros.h>
43: #include <baremetal/Types.h>
44: 
45: /// @file
46: /// Interrupt handler function
47: 
48: #ifdef __cplusplus
49: extern "C" {
50: #endif
51: 
52: void InterruptHandler();
53: 
54: #ifdef __cplusplus
55: }
56: #endif
57: 
58: namespace baremetal {
59: 
60: /// <summary>
61: /// IRQ handler function
62: /// </summary>
63: using IRQHandler = void(void* param);
64: 
65: /// <summary>
66: /// InterruptSystem: Handles IRQ and FIQ interrupts for Raspberry Pi
67: /// This is a SINGLETON class
68: /// </summary>
69: class InterruptSystem
70: {
71:     /// @brief Pointer to registered IRQ handler
72:     IRQHandler* m_irqHandler;
73:     /// @brief Pointer to parameter to pass to registered IRQ handler
74:     void* m_irqHandlerParams;
75: 
76:     /// <summary>
77:     /// Construct the singleton InterruptSystem instance if needed, and return a reference to the instance. This is a friend function of class InterruptSystem
78:     /// </summary>
79:     /// <returns>Reference to the singleton InterruptSystem instance</returns>
80:     friend InterruptSystem& GetInterruptSystem();
81: 
82: private:
83:     InterruptSystem();
84: 
85: public:
86:     ~InterruptSystem();
87: 
88:     void Initialize();
89: 
90:     void RegisterIRQHandler(IRQHandler* handler, void* param);
91:     void UnregisterIRQHandler();
92: 
93:     void InterruptHandler();
94: };
95: 
96: InterruptSystem& GetInterruptSystem();
97: 
98: } // namespace baremetal
```

- Line 52: We declare the function `InterruptHandler()`
- Line 63: We declare a callback function type `IRQHandler` to act as the handler for an IRQ
- Line 69-94: We declare the class `InterruptSystem`
  - Line 72: The member variable `m_irqHandler` stores the registered handler
  - Line 74: The member variable `m_irqHandlerParams` stores the parameter to pass to the registered handler
  - Line 80: We make `GetInterruptSystem()` a friend, so it can call the constructor
  - Line 83: We declare a private default constructor, such that only the `GetInterruptSystem()` function can create an instance
  - Line 86: We declare a destructor
  - Line 88: We declare a method `Initialize()` which will set up the interrupt system
  - Line 90: We declare a method `RegisterIRQHandler()` which will register a handler to be called when an interrupt occurs
  - Line 91: We declare a method `UnregisterIRQHandler()` which will unregister a registered handler
  - Line 93: We declate a method `InterruptHandler()` which is called by the global `InterruptHandler()` function
- Line 96: We declare the function `GetInterruptSystem()`, which creates the singleton instance of the `InterruptSystem` class if needed, and returns a reference to it

### InterruptHandler.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERCPP}

We'll implement the `InterruptHandler()` function as well as the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : InterruptHandler.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Interrupt handler
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
40: #include <baremetal/InterruptHandler.h>
41: 
42: #include <baremetal/Assert.h>
43: 
44: /// @file
45: /// Interrupt handler function implementation
46: 
47: using namespace baremetal;
48: 
49: void InterruptHandler()
50: {
51:     GetInterruptSystem().InterruptHandler();
52: }
53: 
54: /// <summary>
55: /// Create a interrupt system
56: ///
57: /// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
58: /// </summary>
59: InterruptSystem::InterruptSystem()
60:     : m_irqHandler{}
61:     , m_irqHandlerParams{}
62: {
63: }
64: 
65: /// <summary>
66: /// Destructor
67: /// </summary>
68: InterruptSystem::~InterruptSystem()
69: {
70: }
71: 
72: /// <summary>
73: /// Initialize interrupt system
74: /// </summary>
75: void InterruptSystem::Initialize()
76: {
77: }
78: 
79: /// <summary>
80: /// Register an IRQ handler
81: /// </summary>
82: /// <param name="handler">Handler to register</param>
83: /// <param name="param">Parameter to pass to IRQ handler</param>
84: void InterruptSystem::RegisterIRQHandler(IRQHandler* handler, void* param)
85: {
86:     assert(m_irqHandler == nullptr);
87: 
88:     m_irqHandler = handler;
89:     m_irqHandlerParams = param;
90: }
91: 
92: /// <summary>
93: /// Unregister an IRQ handler
94: /// </summary>
95: void InterruptSystem::UnregisterIRQHandler()
96: {
97:     assert(m_irqHandler != nullptr);
98: 
99:     m_irqHandler = nullptr;
100:     m_irqHandlerParams = nullptr;
101: }
102: 
103: /// <summary>
104: /// Handles an interrupt.
105: ///
106: /// The interrupt handler is called from assembly code (ExceptionStub.S)
107: /// </summary>
108: void InterruptSystem::InterruptHandler()
109: {
110:     if (m_irqHandler)
111:         m_irqHandler(m_irqHandlerParams);
112: }
113: 
114: /// <summary>
115: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
116: ///
117: /// This is a friend function of class InterruptSystem
118: /// </summary>
119: /// <returns>Reference to the singleton interrupt system instance</returns>
120: InterruptSystem& baremetal::GetInterruptSystem()
121: {
122:     static InterruptSystem system;
123:     system.Initialize();
124:     return system;
125: }
```

- Line 49-52: We implement the `InterruptHandler()` function by called the `InterruptHandler()` method on the singleton `InterruptSystem` instance
- Line 59-63: We implement the constructor
- Line 68-70: We implement the destructor
- Line 75-77: We implement the `Initialize()` method. For now this is empty
- Line 84-90: We implement the `RegisterIRQHandler()` method
- Line 95-101: We implement the `UnregisterIRQHandler()` method
- Line 108-112: We implement the `InterruptHandler()` method. This will call the handler, if installed
- Line 120-125: We implement the `GetInterruptSystem()` function

### ARMRegisters.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_ARMREGISTERSH}

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
40: /// @file
41: /// Register addresses of Raspberry Pi ARM local registers.
42: ///
43: /// ARM local registers are implemented in the Raspberry Pi BCM chip, but are intended to control functionality in the ARM core.
44: /// For specific registers, we also define the fields and their possible values.
45: 
46: #pragma once
47: 
48: #include <baremetal/Macros.h>
49: #include <baremetal/Types.h>
50: 
51: #if BAREMETAL_RPI_TARGET <= 3
52: /// @brief Base address for ARM Local registers
53: #define ARM_LOCAL_BASE 0x40000000
54: #else
55: /// @brief Base address for ARM Local registers
56: #define ARM_LOCAL_BASE 0xFF800000
57: #endif
58: 
59: /// @brief Raspberry Pi ARM Local Control Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
60: #define ARM_LOCAL_CONTROL                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000000)
61: /// @brief Raspberry Pi ARM Local Core Timer Prescaler Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
62: #define ARM_LOCAL_PRESCALER                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000008)
63: /// @brief Raspberry Pi ARM Local GPU Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
64: #define ARM_LOCAL_GPU_INT_ROUTING          reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000000C)
65: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
66: #define ARM_LOCAL_PM_ROUTING_SET           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000010)
67: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
68: #define ARM_LOCAL_PM_ROUTING_CLR           reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000014)
69: /// @brief Raspberry Pi ARM Local Core Timer Least Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
70: #define ARM_LOCAL_TIMER_LS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000001C)
71: /// @brief Raspberry Pi ARM Local Core Timer Most Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
72: #define ARM_LOCAL_TIMER_MS                 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000020)
73: /// @brief Raspberry Pi ARM Local Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
74: #define ARM_LOCAL_INT_ROUTING              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000024)
75: /// @brief Raspberry Pi ARM Local AXI Outstanding Read/Write Counters Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
76: #define ARM_LOCAL_AXI_COUNT                reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000002C)
77: /// @brief Raspberry Pi ARM Local AXI Outstanding Interrupt Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
78: #define ARM_LOCAL_AXI_IRQ                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000030)
79: /// @brief Raspberry Pi ARM Local Timer Control / Status Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
80: #define ARM_LOCAL_TIMER_CONTROL            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000034)
81: /// @brief Raspberry Pi ARM Local Timer IRQ Clear / Reload Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
82: #define ARM_LOCAL_TIMER_WRITE              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000038)
83: 
84: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
85: #define ARM_LOCAL_TIMER_INT_CONTROL0       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000040)
86: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
87: #define ARM_LOCAL_TIMER_INT_CONTROL1       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000044)
88: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
89: #define ARM_LOCAL_TIMER_INT_CONTROL2       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000048)
90: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
91: #define ARM_LOCAL_TIMER_INT_CONTROL3       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000004C)
92: 
93: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
94: #define ARM_LOCAL_MAILBOX_INT_CONTROL0 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000050)
95: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
96: #define ARM_LOCAL_MAILBOX_INT_CONTROL1 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000054)
97: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
98: #define ARM_LOCAL_MAILBOX_INT_CONTROL2 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000058)
99: /// @brief Raspberry Pi ARM Local Core Mailbox Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
100: #define ARM_LOCAL_MAILBOX_INT_CONTROL3 reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000005C)
101: 
102: /// @brief Raspberry Pi ARM Local Core 0 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
103: #define ARM_LOCAL_IRQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000060)
104: /// @brief Raspberry Pi ARM Local Core 1 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
105: #define ARM_LOCAL_IRQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000064)
106: /// @brief Raspberry Pi ARM Local Core 2 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
107: #define ARM_LOCAL_IRQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000068)
108: /// @brief Raspberry Pi ARM Local Core 3 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
109: #define ARM_LOCAL_IRQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000006C)
110: 
111: /// @brief Raspberry Pi ARM Local Core 0 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
112: #define ARM_LOCAL_FIQ_PENDING0             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000070)
113: /// @brief Raspberry Pi ARM Local Core 1 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
114: #define ARM_LOCAL_FIQ_PENDING1             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000074)
115: /// @brief Raspberry Pi ARM Local Core 2 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
116: #define ARM_LOCAL_FIQ_PENDING2             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000078)
117: /// @brief Raspberry Pi ARM Local Core 3 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
118: #define ARM_LOCAL_FIQ_PENDING3             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000007C)
119: 
120: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
121: #define ARM_LOCAL_MAILBOX0_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000080)
122: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
123: #define ARM_LOCAL_MAILBOX1_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000084)
124: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
125: #define ARM_LOCAL_MAILBOX2_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000088)
126: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
127: #define ARM_LOCAL_MAILBOX3_SET0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000008C)
128: 
129: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
130: #define ARM_LOCAL_MAILBOX0_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000090)
131: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
132: #define ARM_LOCAL_MAILBOX1_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000094)
133: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
134: #define ARM_LOCAL_MAILBOX2_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000098)
135: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
136: #define ARM_LOCAL_MAILBOX3_SET1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000009C)
137: 
138: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
139: #define ARM_LOCAL_MAILBOX0_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A0)
140: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
141: #define ARM_LOCAL_MAILBOX1_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A4)
142: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
143: #define ARM_LOCAL_MAILBOX2_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A8)
144: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
145: #define ARM_LOCAL_MAILBOX3_SET2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000AC)
146: 
147: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
148: #define ARM_LOCAL_MAILBOX0_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B0)
149: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
150: #define ARM_LOCAL_MAILBOX1_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B4)
151: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
152: #define ARM_LOCAL_MAILBOX2_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B8)
153: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
154: #define ARM_LOCAL_MAILBOX3_SET3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000BC)
155: 
156: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
157: #define ARM_LOCAL_MAILBOX0_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C0)
158: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
159: #define ARM_LOCAL_MAILBOX1_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C4)
160: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
161: #define ARM_LOCAL_MAILBOX2_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C8)
162: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
163: #define ARM_LOCAL_MAILBOX3_CLR0            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000CC)
164: 
165: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
166: #define ARM_LOCAL_MAILBOX0_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D0)
167: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
168: #define ARM_LOCAL_MAILBOX1_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D4)
169: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
170: #define ARM_LOCAL_MAILBOX2_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D8)
171: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
172: #define ARM_LOCAL_MAILBOX3_CLR1            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000DC)
173: 
174: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
175: #define ARM_LOCAL_MAILBOX0_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E0)
176: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
177: #define ARM_LOCAL_MAILBOX1_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E4)
178: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
179: #define ARM_LOCAL_MAILBOX2_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E8)
180: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
181: #define ARM_LOCAL_MAILBOX3_CLR2            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000EC)
182: 
183: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
184: #define ARM_LOCAL_MAILBOX0_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F0)
185: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
186: #define ARM_LOCAL_MAILBOX1_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F4)
187: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
188: #define ARM_LOCAL_MAILBOX2_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F8)
189: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
190: #define ARM_LOCAL_MAILBOX3_CLR3            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000FC)
191: 
192: /// @brief Raspberry Pi ARM Local Register region end address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
193: #define ARM_LOCAL_END                      reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000003FFFF)
```

- Line 85: We create definition for the Core Timer Interrupt Control register for core 0
- Line 87: We create definition for the Core Timer Interrupt Control register for core 1
- Line 89: We create definition for the Core Timer Interrupt Control register for core 2
- Line 91: We create definition for the Core Timer Interrupt Control register for core 3

The rest of the registers will be explained as we use them.

### Update CMake file {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_UPDATE_CMAKE_FILE}

As we have now added some source files to the `baremetal` library, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
File: d:\Projects\baremetal\code\libraries\baremetal\CMakeLists.txt
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionStub.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/InterruptHandler.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
53:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
55:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
56:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
57:     )
58: 
59: set(PROJECT_INCLUDES_PUBLIC
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMRegisters.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/InterruptHandler.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
91:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
92:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
93:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
94:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
95:     )
96: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_UPDATE_APPLICATION_CODE}

Let's start generating some interrupts. For now, we'll do this all in the `main()` code, we will be moving and generalizing this later.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: #include <baremetal/ARMInstructions.h>
7: #include <baremetal/ARMRegisters.h>
8: #include <baremetal/BCMRegisters.h>
9: #include <baremetal/MemoryAccess.h>
10: #include <baremetal/InterruptHandler.h>
11: 
12: LOG_MODULE("main");
13: 
14: using namespace baremetal;
15: 
16: #define TICKS_PER_SECOND 2 // Timer ticks per second
17: 
18: static uint32 clockTicksPerSystemTick;
19: 
20: void IntHandler(void* param)
21: {
22:     uint64 counterCompareValue;
23:     GetTimerCompareValue(counterCompareValue);
24:     SetTimerCompareValue(counterCompareValue + clockTicksPerSystemTick);
25: 
26:     LOG_INFO("Ping");
27: }
28: 
29: void EnableIRQ()
30: {
31:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
32:         GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT(1));
33: }
34: 
35: void DisableIRQ()
36: {
37:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
38:         GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT(1));
39: }
40: 
41: int main()
42: {
43:     auto& console = GetConsole();
44: 
45:     auto exceptionLevel = CurrentEL();
46:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
47: 
48:     uint64 counterFreq{};
49:     GetTimerFrequency(counterFreq);
50:     assert(counterFreq % TICKS_PER_SECOND == 0);
51:     clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
52: 
53:     uint64 counter;
54:     GetTimerCounter(counter);
55:     SetTimerCompareValue(counter + clockTicksPerSystemTick);
56:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
57: 
58:     GetInterruptSystem().RegisterIRQHandler(IntHandler, nullptr);
59: 
60:     EnableIRQ();
61: 
62:     LOG_INFO("Wait 5 seconds");
63:     Timer::WaitMilliSeconds(5000);
64: 
65:     DisableIRQ();
66: 
67:     GetInterruptSystem().UnregisterIRQHandler();
68: 
69:     console.Write("Press r to reboot, h to halt\n");
70:     char ch{};
71:     while ((ch != 'r') && (ch != 'h'))
72:     {
73:         ch = console.ReadChar();
74:         console.WriteChar(ch);
75:     }
76: 
77:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
78: }
```

- Line 16: We create a definition to set the timer to trigger 2 times a second
- Line 18: We define a variable to hold the number of clock ticks for every timer tick
- Line 20-27: We define a IRQ handler function `IntHandler()`, which reprograms the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and print some text
- Line 29-33: We define a function `EnableIRQ()`, which enables the IRQ for the ARM local time for core 0
- Line 35-39: We define a function `DisableIRQ()`, which disables the IRQ for the ARM local time for core 0
- Line 48-51: We calculate from the clock tick frequency how many clock ticks are in a timer tick
- Line 53-56: We set the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and enable the interrupt for the timer
- Line 58: We register the handler function
- Line 60: We enable the timer IRQ
- Line 65: We disable the timer IRQ
- Line 67: We unregister the handler function

### Configuring, building and debugging {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

The application will start the timer, and after 5 seconds stop it again. As we set the timer to tick twice a second, we expect to see 10 ticks happening.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:49)
Info   Wait 5 seconds (main:65)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Info   Ping (main:26)
Press r to reboot, h to halt
hInfo   Halt (System:129)
```

## Interrupt System - Step 2 {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2}

We'll update the class `InterruptSystem` to enable, disable, and handle interrupts.
We'll also add support for fast interrupts (FIQ).

### InterruptSystem.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTSYSTEMH}

Update the file `code/libraries/baremetal/include/baremetal/InterruptSystem.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptSystem.h
...
59: namespace baremetal {
60: 
61: /// <summary>
62: /// FIQ handler function
63: /// </summary>
64: using FIQHandler = void(void* param);
65: 
66: /// <summary>
67: /// IRQ handler function
68: /// </summary>
69: using IRQHandler = void(void* param);
70: 
71: /// <summary>
72: /// FIQ data
73: ///
74: /// This is data stored in Exceptions.S
75: /// </summary>
76: struct FIQData
77: {
78:     /// @brief FIQ handler
79:     FIQHandler* handler;
80:     /// @brief Parameter to pass to registered FIQ handler
81:     void*       param;
82:     /// @brief ID of FIQ
83:     uint32      fiqID;
84: } PACKED;
85: 
86: class IMemoryAccess;
87: 
88: /// <summary>
89: /// InterruptSystem: Handles IRQ and FIQ interrupts for Raspberry Pi
90: /// This is a SINGLETON class
91: /// </summary>
92: class InterruptSystem
93: {
94:     /// @brief True if class is already initialized
95:     bool m_initialized;
96:     /// @brief Pointer to registered IRQ handler for each IRQ
97:     IRQHandler* m_irqHandlers[IRQ_LINES];
98:     /// @brief Parameter to pass to registered IRQ handler
99:     void* m_irqHandlersParams[IRQ_LINES];
100:     /// @brief Memory access interface
101:     IMemoryAccess& m_memoryAccess;
102: 
103:     /// <summary>
104:     /// Construct the singleton InterruptSystem instance if needed, and return a reference to the instance. This is a friend function of class InterruptSystem
105:     /// </summary>
106:     /// <returns>Reference to the singleton InterruptSystem instance</returns>
107:     friend InterruptSystem& GetInterruptSystem();
108: 
109: private:
110: 
111:     /// @brief Create a interrupt system. Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
112:     InterruptSystem();
113: 
114: public:
115:     InterruptSystem(IMemoryAccess& memoryAccess);
116:     ~InterruptSystem();
117: 
118:     void Initialize();
119: 
120:     void DisableInterrupts();
121: 
122:     void RegisterIRQHandler(unsigned irqID, IRQHandler* handler, void* param);
123:     void UnregisterIRQHandler(unsigned irqID);
124: 
125:     void RegisterFIQHandler(unsigned fiqID, FIQHandler* handler, void* param);
126:     void UnregisterFIQHandler();
127: 
128:     static void EnableIRQ(unsigned irqID);
129:     static void DisableIRQ(unsigned irqID);
130: 
131:     static void EnableFIQ(unsigned fiqID);
132:     static void DisableFIQ();
133: 
134:     void InterruptHandler();
135: 
136: private:
137:     bool CallIRQHandler(unsigned irqID);
138: };
139: 
140: InterruptSystem& GetInterruptSystem();
141: 
142: } // namespace baremetal
143: 
144: /// @brief FIQ administration, see Exception.S
145: extern baremetal::FIQData s_fiqData;
```

- Line 64: We add a declaration for the `FIQHandler()` function type, much like the `IRQHandler()` function type
- Line 76-84: We add a declaration for the `FIQData` struct, which mirrors the data defined in `ExceptionStub.S'
- Line 115: We add a declaration for a specialized constructor taken a `MemoryAccess` reference, meant for testing
- Line 120: We add the method `DisableInterrupts()`, which disables all interrupts, e.g. it disables interrupt enables on all IRQ lines, as well as the FIQ
- Line 122: We change the method `RegisterIRQHandler()` to also take an IRQ id
- Line 123: We change the method `UnregisterIRQHandler()` to take an IRQ id
- Line 125: We add the method `RegisterFIQHandler()` which registers a FIQ handler for the specified FIQ id. Only one FIQ handler can be active at any time
- Line 126: We add the method `UnregisterFIQHandler()` which unregisters the previously registered FIQ handler. As there can only be one FIQ handler registered at any time, we don't need to specify the FIQ id
- Line 128: We add the static method `EnableIRQ()` which enables the IRQ with the specified id
- Line 129: We add the static method `DisableIRQ()` which disables the IRQ with the specified id
- Line 131: We add the static method `EnableFIQ()` which enables the FIQ with the specified id
- Line 132: We add the static method `DisableFIQ()` which disables any enable FIQ
- Line 137: We add the method `CallIRQHandler()` which calls the registered IRQ handler for the IRQ with the specified id
- Line 145: We declare the FIQData struct instance s_fiqData, which is defined in `ExceptionStub.S'

### InterruptHandler.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTHANDLERCPP}

Let's update the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
...
63: /// <summary>
64: /// Create a interrupt system
65: ///
66: /// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
67: /// </summary>
68: InterruptSystem::InterruptSystem()
69:     : m_initialized{}
70:     , m_irqHandlers{}
71:     , m_irqHandlersParams{}
72:     , m_memoryAccess{ GetMemoryAccess() }
73: {
74: }
75: 
76: /// <summary>
77: /// Constructs a specialized InterruptSystem instance which injects a custom IMemoryAccess instance. This is intended for testing.
78: /// </summary>
79: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
80: InterruptSystem::InterruptSystem(IMemoryAccess& memoryAccess)
81:     : m_initialized{}
82:     , m_irqHandlers{}
83:     , m_irqHandlersParams{}
84:     , m_memoryAccess{ memoryAccess }
85: {
86: }
87: 
88: /// <summary>
89: /// Destructor
90: /// </summary>
91: InterruptSystem::~InterruptSystem()
92: {
93:     DisableIRQs();
94: 
95:     DisableInterrupts();
96: }
97: 
98: /// <summary>
99: /// Initialize interrupt system
100: /// </summary>
101: void InterruptSystem::Initialize()
102: {
103:     if (m_initialized)
104:         return;
105:     for (unsigned irqID = 0; irqID < IRQ_LINES; irqID++)
106:     {
107:         m_irqHandlers[irqID] = nullptr;
108:         m_irqHandlersParams[irqID] = nullptr;
109:     }
110: 
111:     DisableInterrupts();
112: 
113:     EnableIRQs();
114: 
115:     m_initialized = true;
116: }
117: 
118: /// <summary>
119: /// Disable all IRQ interrupts
120: /// </summary>
121: void InterruptSystem::DisableInterrupts()
122: {
123:     m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
124: 
125:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
126:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
127:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
128:     m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
129: }
130: 
131: /// <summary>
132: /// Enable and register an IRQ handler
133: /// 
134: /// Enable the IRQ with specified index, and register its handler.
135: /// IRQ are divided into groups:
136: /// - IRQ1: 0..31 (System Timer, GPU, DMA, ...)
137: /// - IRQ2: 32..63 (HDMI, CAM, GPIO, SPI, I2C, I2S, RNG, ...)
138: /// - IRQ_Basic: 64..71 (ARM standard interrupts)
139: /// - IRQ_ARM_LOcal: 72..83 (ARM local interrupts)
140: /// </summary>
141: /// <param name="irqID">IRQ number 0..83</param>
142: /// <param name="handler">Handler to register for this IRQ</param>
143: /// <param name="param">Parameter to pass to IRQ handler</param>
144: void InterruptSystem::RegisterIRQHandler(unsigned irqID, IRQHandler* handler, void* param)
145: {
146:     assert(irqID < IRQ_LINES);
147:     assert(m_irqHandlers[irqID] == nullptr);
148: 
149:     m_irqHandlers[irqID] = handler;
150:     m_irqHandlersParams[irqID] = param;
151: 
152:     EnableIRQ(irqID);
153: }
154: 
155: /// <summary>
156: /// Disable and unregister an IRQ handler
157: /// 
158: /// Disable the IRQ with specified index, and unregister its handler.
159: /// </summary>
160: /// <param name="irqID">IRQ number 0..83</param>
161: void InterruptSystem::UnregisterIRQHandler(unsigned irqID)
162: {
163:     assert(irqID < IRQ_LINES);
164:     assert(m_irqHandlers[irqID] != nullptr);
165: 
166:     DisableIRQ(irqID);
167: 
168:     m_irqHandlers[irqID] = nullptr;
169:     m_irqHandlersParams[irqID] = nullptr;
170: }
171: 
172: /// <summary>
173: /// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
174: /// </summary>
175: /// <param name="fiqID">FIQ interrupt number</param>
176: /// <param name="handler">FIQ interrupt handler</param>
177: /// <param name="param">FIQ interrupt data</param>
178: // cppcheck-suppress unusedFunction
179: void InterruptSystem::RegisterFIQHandler(unsigned fiqID, FIQHandler *handler, void *param)
180: {
181:     assert(fiqID <= BCM_MAX_FIQ);
182:     assert(handler != nullptr);
183:     assert(s_fiqData.handler == nullptr);
184: 
185:     s_fiqData.handler = handler;
186:     s_fiqData.param   = param;
187: 
188:     EnableFIQ(fiqID);
189: }
190: 
191: /// <summary>
192: /// Disable and unregister a FIQ interrupt handler
193: /// </summary>
194: void InterruptSystem::UnregisterFIQHandler()
195: {
196:     assert(s_fiqData.handler != nullptr);
197: 
198:     DisableFIQ();
199: 
200:     s_fiqData.handler = nullptr;
201:     s_fiqData.param   = nullptr;
202: }
203: 
204: /// <summary>
205: /// Enable an IRQ interrupt. And interrupt cannot be chained, so only one handler can be connected
206: /// RPI3 has 64 IRQ + 8 basic IRQ + 12 local IRQ. The local IRQ are handled differently
207: /// RPI4 has 256 IRQ, and no local IRQ (not handled here yes, uses the GIC)
208: /// </summary>
209: /// <param name="irqID"></param>
210: void InterruptSystem::EnableIRQ(unsigned irqID)
211: {
212:     assert(irqID < IRQ_LINES);
213: 
214:     if (irqID < ARM_IRQLOCAL_BASE)
215:     {
216:         GetInterruptSystem().m_memoryAccess.Write32(ARM_IC_IRQS_ENABLE(irqID), ARM_IRQ_MASK(irqID));
217:     }
218:     else
219:     {
220:         assert(irqID == ARM_IRQLOCAL0_CNTPNS); // The only implemented local IRQ so far
221:         GetInterruptSystem().m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
222:             GetInterruptSystem().m_memoryAccess.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT(1));
223:     }
224: }
225: 
226: /// <summary>
227: /// Disable an IRQ interrupt.
228: /// RPI3 has 64 IRQ + 8 basic IRQ + 12 local IRQ. The local IRQ are handled differently
229: /// RPI4 has 256 IRQ, and no local IRQ (not handled here yes, uses the GIC)
230: /// </summary>
231: /// <param name="irqID"></param>
232: void InterruptSystem::DisableIRQ(unsigned irqID)
233: {
234:     assert(irqID < IRQ_LINES);
235: 
236:     if (irqID < ARM_IRQLOCAL_BASE)
237:     {
238:         GetInterruptSystem().m_memoryAccess.Write32(ARM_IC_IRQS_DISABLE(irqID), ARM_IRQ_MASK(irqID));
239:     }
240:     else
241:     {
242:         assert(irqID == ARM_IRQLOCAL0_CNTPNS); // The only implemented local IRQ so far
243:         GetInterruptSystem().m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
244:             GetInterruptSystem().m_memoryAccess.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT(1));
245:     }
246: }
247: 
248: /// <summary>
249: /// Enable a FIQ interrupt. Only one can be enabled at any time
250: /// </summary>
251: /// <param name="fiqID">FIQ interrupt number</param>
252: void InterruptSystem::EnableFIQ(unsigned fiqID)
253: {
254:     assert(fiqID <= BCM_MAX_FIQ);
255: 
256:     GetInterruptSystem().m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, fiqID | FIQ_INTR_ENABLE);
257: }
258: 
259: /// <summary>
260: /// Disabled the enabled FIQ interrupt (if any).
261: /// </summary>
262: void InterruptSystem::DisableFIQ()
263: {
264:     GetInterruptSystem().m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
265: }
266: 
267: /// <summary>
268: /// Handles an interrupt.
269: ///
270: /// The interrupt handler is called from assembly code (ExceptionStub.S)
271: /// </summary>
272: void InterruptSystem::InterruptHandler()
273: {
274:     uint32 localpendingIRQs = m_memoryAccess.Read32(ARM_LOCAL_IRQ_PENDING0);
275:     assert(!(localpendingIRQs & ~(1 << 1 | 0xF << 4 | 1 << 8)));
276:     if (localpendingIRQs & (1 << 1)) // the only implemented local IRQ so far
277:     {
278:         CallIRQHandler(ARM_IRQLOCAL0_CNTPNS);
279: 
280:         return;
281:     }
282: 
283:     uint32 pendingIRQs[ARM_IC_IRQ_REGS];
284:     pendingIRQs[0] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_1);
285:     pendingIRQs[1] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_2);
286:     pendingIRQs[2] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_BASIC_PENDING) & 0xFF; // Only 8 basic interrupts
287: 
288:     for (unsigned reg = 0; reg < ARM_IC_IRQ_REGS; reg++)
289:     {
290:         uint32 pendingIRQ = pendingIRQs[reg];
291:         if (pendingIRQ != 0)
292:         {
293:             unsigned irqID = reg * BCM_IRQS_PER_REG;
294: 
295:             do
296:             {
297:                 if ((pendingIRQ & 1) && CallIRQHandler(irqID))
298:                 {
299:                     return;
300:                 }
301: 
302:                 pendingIRQ >>= 1;
303:                 irqID++;
304:             } while (pendingIRQ != 0);
305:         }
306:     }
307: }
308: 
309: /// <summary>
310: /// Call the IRQ handler for the specified IRQ ID
311: /// </summary>
312: /// <param name="irqID">ID of the IRQ</param>
313: /// <returns>True if a IRQ handler was found, false if not</returns>
314: bool InterruptSystem::CallIRQHandler(unsigned irqID)
315: {
316:     assert(irqID < IRQ_LINES);
317:     IRQHandler* handler = m_irqHandlers[irqID];
318: 
319:     if (handler != nullptr)
320:     {
321:         (*handler)(m_irqHandlersParams[irqID]);
322: 
323:         return true;
324:     }
325: 
326:     DisableIRQ(irqID);
327: 
328:     return false;
329: }
330: 
331: /// <summary>
332: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
333: ///
334: /// This is a friend function of class InterruptSystem
335: /// </summary>
336: /// <returns>Reference to the singleton interrupt system instance</returns>
337: InterruptSystem& baremetal::GetInterruptSystem()
338: {
339:     static InterruptSystem system;
340:     system.Initialize();
341:     return system;
342: }
```

- Line 68-74: We update the default constructor to initialize the new member variables
- Line 80-86: We implement the specialized constructor
- Line 91-96: We update the destructor to disable all interrupts
- Line 101-116: We update the `Initialize()` method set up the administration for IRQs, disable all interrupt lines, but enable the IRQs system wide.
Note that we use `m_initialized` to protect against multiple initialization
- Line 121-129: We implement the `DisableInterrupts()` method
- Line 144-153: We update the `RegisterIRQHandler()` method to administer the registered IRQ handler, and enable its IRQ line
- Line 161-170: We update the `UnregisterIRQHandler()` method to remove the registered IRQ handler, and disable its IRQ line
- Line 179-189: We implement the `RegisterFIQHandler()` method to administer the registered FIQ handler, and enable it
- Line 194-202: We implement the `UnregisterIRQHandler()` method to remove the registered FIQ handler, and disable it
- Line 210-224: We implement the `EnableIRQ()` method.
Not that ARM local interrupts are handled differently, and that currently only one ARM local interrupt is supported, i.e. `ARM_IRQLOCAL0_CNTPNS`
- Line 232-246: We implement the `DisableIRQ()` method
- Line 252-257: We implement the `EnableFIQ()` method.
- Line 262-265: We implement the `DisableFIQ()` method
- Line 272-307: We update the `InterruptHandler()` method. This checks for the ARM local interrupt pending for `ARM_IRQLOCAL0_CNTPNS`,
then check whether any of the IRQ 1, IRQ 2 and IRQ basic interrupts are pending, and calls `CallIRQHandler()` for each pending interrupt, until one has been called or no interrupts are pending..
So at most one interrupt is handled in each call to `InterruptHandler()`
- Line 316-329: We implement the `CallIRQHandler()` method. This call the IRQ handler if registered, otherwise disables the interrupt for the future

### Interrupt.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTH}

We'll add definitions for all IRQ and FIQ interrupts.

Create the file `code/libraries/baremetal/include/baremetal/Interrupt.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Interrupt.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Interrupt.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Interrupt definitions
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
43: /// Interrupt handler function
44: 
45: #if BAREMETAL_RPI_TARGET <= 3
46: 
47: // IRQ
48: 
49: /// @brief Total count of BCM IRQ registers (GPU and basic)
50: #define ARM_IC_IRQ_REGS 3
51: 
52: /// @brief NUmber of IRQ lines in each GPU IRQ register
53: #define BCM_IRQS_PER_REG         32
54: /// @brief NUmber of IRQ lines in the basic IRQ register
55: #define BCM_IRQS_BASIC_REG       8
56: /// @brief NUmber of IRQ lines in the ARM local IRQ register
57: #define ARM_IRQS_LOCAL_REG       12
58: 
59: /// @brief GPU IRQ 1 register holds IRQ 0..31. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
60: #define BCM_IRQ1_BASE            0
61: /// @brief GPU IRQ 2 register holds IRQ 32..63. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
62: #define BCM_IRQ2_BASE            (BCM_IRQ1_BASE + BCM_IRQS_PER_REG)
63: /// @brief Basic IRQ register holds 8 IRQs. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
64: #define BCM_IRQBASIC_BASE        (BCM_IRQ2_BASE + BCM_IRQS_PER_REG)
65: /// @brief IRQ local register holds 12 IRQs. See @ref RASPBERRY_PI_BCM_LOCAL_DEVICE_REGISTERS
66: #define ARM_IRQLOCAL_BASE        (BCM_IRQBASIC_BASE + BCM_IRQS_BASIC_REG)
67: 
68: /// @brief Determine register to read for IRQ pending status (BCM IRQs only)
69: #define ARM_IC_IRQ_PENDING(irq)                                                                                                                                \
70:     ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_IRQ_PENDING_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_IRQ_PENDING_2 : RPI_INTRCTRL_IRQ_BASIC_PENDING))
71: /// @brief Determine register to enable IRQ (BCM IRQs only)
72: #define ARM_IC_IRQS_ENABLE(irq)                                                                                                                                \
73:     ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_ENABLE_IRQS_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_ENABLE_IRQS_2 : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
74: /// @brief Determine register to disable IRQ (BCM IRQs only)
75: #define ARM_IC_IRQS_DISABLE(irq)                                                                                                                               \
76:     ((irq) < BCM_IRQ2_BASE ? RPI_INTRCTRL_DISABLE_IRQS_1 : ((irq) < BCM_IRQBASIC_BASE ? RPI_INTRCTRL_DISABLE_IRQS_2 : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
77: /// @brief Determine mask to read / write IRQ register (BCM IRQs only)
78: #define ARM_IRQ_MASK(irq) (1 << ((irq) & (BCM_IRQS_PER_REG - 1)))
79: 
80: // IRQ 1 interrupts
81: 
82: /// @brief RPI System Timer Compare 0 interrupt (bcm2835-system-timer)
83: #define BCM_IRQ_TIMER0           (BCM_IRQ1_BASE + 0)
84: /// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
85: #define BCM_IRQ_TIMER1           (BCM_IRQ1_BASE + 1)
86: /// @brief RPI System Timer Compare 2 interrupt (bcm2835-system-timer)
87: #define BCM_IRQ_TIMER2           (BCM_IRQ1_BASE + 2)
88: /// @brief RPI System Timer Compare 3 interrupt (bcm2835-system-timer)
89: #define BCM_IRQ_TIMER3           (BCM_IRQ1_BASE + 3)
90: /// @brief RPI Codec 0 interrupt (bcm2835-cprman)
91: #define BCM_IRQ_CODEC0           (BCM_IRQ1_BASE + 4)
92: /// @brief RPI Codec 1 interrupt (bcm2835-cprman)
93: #define BCM_IRQ_CODEC1           (BCM_IRQ1_BASE + 5)
94: /// @brief RPI Codec 2 interrupt
95: #define BCM_IRQ_CODEC2           (BCM_IRQ1_BASE + 6)
96: /// @brief RPI JPEG interrupt
97: #define BCM_IRQ_JPEG             (BCM_IRQ1_BASE + 7)
98: /// @brief TBD
99: #define BCM_IRQ_ISP              (BCM_IRQ1_BASE + 8)
100: /// @brief RPI USB interrupt (bcm2708-usb)
101: #define BCM_IRQ_USB              (BCM_IRQ1_BASE + 9)
102: /// @brief RPI VC4 3D interrupt (vc4-v3d)
103: #define BCM_IRQ_3D               (BCM_IRQ1_BASE + 10)
104: /// @brief RPI Transposer (TXP) interrupt (bcm2835-txp)
105: #define BCM_IRQ_TRANSPOSER       (BCM_IRQ1_BASE + 11)
106: /// @brief TBD
107: #define BCM_IRQ_MULTICORESYNC0   (BCM_IRQ1_BASE + 12)
108: /// @brief TBD
109: #define BCM_IRQ_MULTICORESYNC1   (BCM_IRQ1_BASE + 13)
110: /// @brief TBD
111: #define BCM_IRQ_MULTICORESYNC2   (BCM_IRQ1_BASE + 14)
112: /// @brief TBD
113: #define BCM_IRQ_MULTICORESYNC3   (BCM_IRQ1_BASE + 15)
114: /// @brief RPI DMA channel 0 interrupt (bcm2835-dma)
115: #define BCM_IRQ_DMA0             (BCM_IRQ1_BASE + 16)
116: /// @brief RPI DMA channel 1 interrupt (bcm2835-dma)
117: #define BCM_IRQ_DMA1             (BCM_IRQ1_BASE + 17)
118: /// @brief RPI DMA channel 2 interrupt (bcm2835-dma)
119: #define BCM_IRQ_DMA2             (BCM_IRQ1_BASE + 18)
120: /// @brief RPI DMA channel 3 interrupt (bcm2835-dma)
121: #define BCM_IRQ_DMA3             (BCM_IRQ1_BASE + 19)
122: /// @brief RPI DMA channel 4 interrupt (bcm2835-dma)
123: #define BCM_IRQ_DMA4             (BCM_IRQ1_BASE + 20)
124: /// @brief RPI DMA channel 5 interrupt (bcm2835-dma)
125: #define BCM_IRQ_DMA5             (BCM_IRQ1_BASE + 21)
126: /// @brief RPI DMA channel 6 interrupt (bcm2835-dma)
127: #define BCM_IRQ_DMA6             (BCM_IRQ1_BASE + 22)
128: /// @brief RPI DMA channel 7 interrupt (bcm2835-dma)
129: #define BCM_IRQ_DMA7             (BCM_IRQ1_BASE + 23)
130: /// @brief RPI DMA channel 8 interrupt (bcm2835-dma)
131: #define BCM_IRQ_DMA8             (BCM_IRQ1_BASE + 24)
132: /// @brief RPI DMA channel 9 interrupt (bcm2835-dma)
133: #define BCM_IRQ_DMA9             (BCM_IRQ1_BASE + 25)
134: /// @brief RPI DMA channel 10 interrupt (bcm2835-dma)
135: #define BCM_IRQ_DMA10            (BCM_IRQ1_BASE + 26)
136: /// @brief RPI DMA channel 11/12/13/14 interrupt (bcm2835-dma)
137: #define BCM_IRQ_DMA11            (BCM_IRQ1_BASE + 27)
138: /// @brief RPI DMA shared interrupt (bcm2835-dma)
139: #define BCM_IRQ_DMA_SHARED       (BCM_IRQ1_BASE + 28)
140: /// @brief RPI Auxiliary Peripheral interrupt (bcm2835-aux-uart, bcm43438-bt)
141: #define BCM_IRQ_AUX              (BCM_IRQ1_BASE + 29)
142: /// @brief TBD
143: #define BCM_IRQ_ARM              (BCM_IRQ1_BASE + 30)
144: /// @brief TBD
145: #define BCM_IRQ_VPUDMA           (BCM_IRQ1_BASE + 31)
146: 
147: // IRQ 2 interrupts
148: 
149: /// @brief RPI USB Host interrupt (bcm2708-usb)
150: #define BCM_IRQ_HOSTPORT         (BCM_IRQ2_BASE + 0)
151: /// @brief RPI Videoscaler interrupt (bcm2835-hvs)
152: #define BCM_IRQ_VIDEOSCALER      (BCM_IRQ2_BASE + 1)
153: /// @brief TBD
154: #define BCM_IRQ_CCP2TX           (BCM_IRQ2_BASE + 2)
155: /// @brief TBD
156: #define BCM_IRQ_SDC              (BCM_IRQ2_BASE + 3)
157: /// @brief RPI DSI0 interrupt (bcm2835-dsi0)
158: #define BCM_IRQ_DSI0             (BCM_IRQ2_BASE + 4)
159: /// @brief RPI AVE interrupt (bcm2711-pixelvalve2)
160: #define BCM_IRQ_AVE              (BCM_IRQ2_BASE + 5)
161: /// @brief RPI CAM 0 interrupt (bcm2835-unicam)
162: #define BCM_IRQ_CAM0             (BCM_IRQ2_BASE + 6)
163: /// @brief RPI CAM 1 interrupt (bcm2835-unicam)
164: #define BCM_IRQ_CAM1             (BCM_IRQ2_BASE + 7)
165: /// @brief RPI HDMI 0 interrupt (bcm2835-hdmi)
166: #define BCM_IRQ_HDMI0            (BCM_IRQ2_BASE + 8)
167: /// @brief RPI HDMI 1 interrupt (bcm2835-hdmi)
168: #define BCM_IRQ_HDMI1            (BCM_IRQ2_BASE + 9)
169: /// @brief RPI Pixel valve 2 interrupt (bcm2835-pixelvalve2, bcm2711-pixelvalve3)
170: #define BCM_IRQ_PIXELVALVE1      (BCM_IRQ2_BASE + 10)
171: /// @brief RPI I2C slave interrupt
172: #define BCM_IRQ_I2CSPISLV        (BCM_IRQ2_BASE + 11)
173: /// @brief RPI DSI1 interrupt (bcm2835-dsi1)
174: #define BCM_IRQ_DSI1             (BCM_IRQ2_BASE + 12)
175: /// @brief RPI Pixel valve 0 interrupt (bcm2835-pixelvalve0, bcm2711-pixelvalve0)
176: #define BCM_IRQ_PWA0             (BCM_IRQ2_BASE + 13)
177: /// @brief RPI Pixel valve 1 interrupt (bcm2835-pixelvalve1, bcm2711-pixelvalve1, bcm2711-pixelvalve4)
178: #define BCM_IRQ_PWA1             (BCM_IRQ2_BASE + 14)
179: /// @brief TBD
180: #define BCM_IRQ_CPR              (BCM_IRQ2_BASE + 15)
181: /// @brief RPI SMI interrupt (bcm2835-smi, rpi-firmware-kms)
182: #define BCM_IRQ_SMI              (BCM_IRQ2_BASE + 16)
183: /// @brief RPI GPIO 0 interrupt (bcm2835-gpio)
184: #define BCM_IRQ_GPIO0            (BCM_IRQ2_BASE + 17)
185: /// @brief RPI GPIO 1 interrupt (bcm2835-gpio)
186: #define BCM_IRQ_GPIO1            (BCM_IRQ2_BASE + 18)
187: /// @brief RPI GPIO 2 interrupt
188: #define BCM_IRQ_GPIO2            (BCM_IRQ2_BASE + 19)
189: /// @brief RPI GPIO 3 interrupt
190: #define BCM_IRQ_GPIO3            (BCM_IRQ2_BASE + 20)
191: /// @brief RPI I2C interrupt (bcm2835-i2c)
192: #define BCM_IRQ_I2C              (BCM_IRQ2_BASE + 21)
193: /// @brief RPI SPI interrupt (bcm2835-spi)
194: #define BCM_IRQ_SPI              (BCM_IRQ2_BASE + 22)
195: /// @brief RPI I2C audio interrupt
196: #define BCM_IRQ_I2SPCM           (BCM_IRQ2_BASE + 23)
197: /// @brief RPI EMMC / SDIO interrupt (bcm2835-sdhost)
198: #define BCM_IRQ_SDIO             (BCM_IRQ2_BASE + 24)
199: /// @brief RPI UART interrupt (arm,primecell, serial@7e201000, bcm43438-bt)
200: #define BCM_IRQ_UART             (BCM_IRQ2_BASE + 25)
201: /// @brief TBD
202: #define BCM_IRQ_SLIMBUS          (BCM_IRQ2_BASE + 26)
203: /// @brief RPI VEC interrupt (bcm2835-vec, bcm2711-vec)
204: #define BCM_IRQ_VEC              (BCM_IRQ2_BASE + 27)
205: /// @brief TBD
206: #define BCM_IRQ_CPG              (BCM_IRQ2_BASE + 28)
207: /// @brief RPI RNG interrupt (bcm2835-rng)
208: #define BCM_IRQ_RNG              (BCM_IRQ2_BASE + 29)
209: /// @brief RPI SDHCI (bcm2835-sdhci, bcm2711-emmc2)
210: #define BCM_IRQ_ARASANSDIO       (BCM_IRQ2_BASE + 30)
211: /// @brief TBD
212: #define BCM_IRQ_AVSPMON          (BCM_IRQ2_BASE + 31)
213: 
214: // IRQ basic interrupts
215: 
216: /// @brief RPI ARM Timer interrupt interrupt
217: #define BCM_IRQ_BCM_TIMER        (BCM_IRQBASIC_BASE + 0)
218: /// @brief RPI ARM Mailbox interrupt interrupt (bcm2835-mbox)
219: #define BCM_IRQ_BCM_MAILBOX      (BCM_IRQBASIC_BASE + 1)
220: /// @brief RPI ARM Doorbell 0 interrupt interrupt (bcm2835-vchiq, bcm2711-vchiq)
221: #define BCM_IRQ_BCM_DOORBELL_0   (BCM_IRQBASIC_BASE + 2)
222: /// @brief RPI ARM Doorbell 1 interrupt interrupt
223: #define BCM_IRQ_BCM_DOORBELL_1   (BCM_IRQBASIC_BASE + 3)
224: /// @brief RPI ARM GPU 0 halted interrupt (bcm2835-cprman)
225: #define BCM_IRQ_VPU0_HALTED      (BCM_IRQBASIC_BASE + 4)
226: /// @brief RPI ARM GPU 1 halted interrupt (bcm2835-cprman)
227: #define BCM_IRQ_VPU1_HALTED      (BCM_IRQBASIC_BASE + 5)
228: /// @brief RPI ARM Illegal access type 1 interrupt
229: #define BCM_IRQ_ILLEGAL_TYPE0    (BCM_IRQBASIC_BASE + 6)
230: /// @brief RPI ARM Illegal access type 0 interrupt
231: #define BCM_IRQ_ILLEGAL_TYPE1    (BCM_IRQBASIC_BASE + 7)
232: 
233: // ARM local interrupts
234: 
235: /// @brief RPI ARM Local Counter-timer Physical Secure Timer interrupt (armv7-timer)
236: #define ARM_IRQLOCAL0_CNTPS      (ARM_IRQLOCAL_BASE + 0)
237: /// @brief RPI ARM Local Counter-timer Physical Timer interrupt (armv7-timer)
238: #define ARM_IRQLOCAL0_CNTPNS     (ARM_IRQLOCAL_BASE + 1)
239: /// @brief RPI ARM Local Counter-timer Hypervisor Timer interrupt (armv7-timer)
240: #define ARM_IRQLOCAL0_CNTHP      (ARM_IRQLOCAL_BASE + 2)
241: /// @brief RPI ARM Local Counter-timer Virtual Timer interrupt (armv7-timer)
242: #define ARM_IRQLOCAL0_CNTV       (ARM_IRQLOCAL_BASE + 3)
243: /// @brief RPI ARM Local Mailbox 0 interrupt
244: #define ARM_IRQLOCAL0_MAILBOX0   (ARM_IRQLOCAL_BASE + 4)
245: /// @brief RPI ARM Local Mailbox 1 interrupt
246: #define ARM_IRQLOCAL0_MAILBOX1   (ARM_IRQLOCAL_BASE + 5)
247: /// @brief RPI ARM Local Mailbox 2 interrupt
248: #define ARM_IRQLOCAL0_MAILBOX2   (ARM_IRQLOCAL_BASE + 6)
249: /// @brief RPI ARM Local Mailbox 3 interrupt
250: #define ARM_IRQLOCAL0_MAILBOX3   (ARM_IRQLOCAL_BASE + 7)
251: /// @brief RPI ARM Local GPU interrupt
252: #define ARM_IRQLOCAL0_GPU        (ARM_IRQLOCAL_BASE + 8) // cascaded GPU interrupts
253: /// @brief RPI ARM Local Performance Monitor Unit interrupt (cortex-a7-pmu)
254: #define ARM_IRQLOCAL0_PMU        (ARM_IRQLOCAL_BASE + 9)
255: /// @brief RPI ARM Local AXI interrupt
256: #define ARM_IRQLOCAL0_AXI_IDLE   (ARM_IRQLOCAL_BASE + 10) // on core 0 only
257: /// @brief RPI ARM Local Timer interrupt
258: #define ARM_IRQLOCAL0_LOCALTIMER (ARM_IRQLOCAL_BASE + 11)
259: 
260: /// @brief Total count of IRQ lines on RPI3
261: #define IRQ_LINES                (BCM_IRQS_PER_REG * 2 + BCM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)
262: 
263: #elif BAREMETAL_RPI_TARGET == 4
264: 
265: /// @brief GIC interrupts private per core
266: #define GIC_PPI(n)             (16 + (n))
267: /// @brief GIC interrupts shared between cores
268: #define GIC_SPI(n)             (32 + (n))
269: 
270: // ARM local interrupts
271: /// @brief RPI ARM Local Counter-timer Physical Timer interrupt (RPI4)
272: #define BCM_IRQLOCAL0_CNTPNS   GIC_PPI(14)
273: 
274: // BCM basic IRQs
275: /// @brief RPI BCM Mailbox interrupt (RPI4)
276: #define BCM_IRQ_BCM_MAILBOX    GIC_SPI(0x21)
277: /// @brief RPI BCM Doorbell 0 interrupt (RPI4)
278: #define BCM_IRQ_BCM_DOORBELL_0 GIC_SPI(0x22) // (bcm2711-vchiq)
279: /// @brief RPI BCM Hostport interrupt (RPI4)
280: #define BCM_IRQ_HOSTPORT       GIC_SPI(0x28) // (bcm2708-usb)
281: 
282: // IRQ 1 interrupts
283: /// @brief RPI System Timer Compare 0 interrupt (bcm2835-system-timer)
284: #define BCM_IRQ_TIMER0         GIC_SPI(0x40) // (bcm2835-system-timer)
285: /// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
286: #define BCM_IRQ_TIMER1         GIC_SPI(0x41) // (bcm2835-system-timer)
287: /// @brief RPI System Timer Compare 2 interrupt (bcm2835-system-timer)
288: #define BCM_IRQ_TIMER2         GIC_SPI(0x42) // (bcm2835-system-timer)
289: /// @brief RPI System Timer Compare 3 interrupt (bcm2835-system-timer)
290: #define BCM_IRQ_TIMER3         GIC_SPI(0x43) // (bcm2835-system-timer)
291: /// @brief RPI USB interrupt (bcm2708-usb)
292: #define BCM_IRQ_USB            GIC_SPI(0x49) // (bcm2708-usb)
293: /// @brief RPI VC4 3D interrupt (vc4-v3d)
294: #define BCM_IRQ_3D             GIC_SPI(0x4A) // (2711-v3d)
295: /// @brief RPI Transposer (TXP) interrupt (bcm2835-txp)
296: #define BCM_IRQ_TRANSPOSER     GIC_SPI(0x4B) // (bcm2835-system-timer)
297: /// @brief RPI DMA channel 0 interrupt (bcm2835-dma)
298: #define BCM_IRQ_DMA0           GIC_SPI(0x50) // (bcm2835-dma)
299: /// @brief RPI DMA channel 1 interrupt (bcm2835-dma)
300: #define BCM_IRQ_DMA1           GIC_SPI(0x51) // (bcm2835-dma)
301: /// @brief RPI DMA channel 2 interrupt (bcm2835-dma)
302: #define BCM_IRQ_DMA2           GIC_SPI(0x52) // (bcm2835-dma)
303: /// @brief RPI DMA channel 3 interrupt (bcm2835-dma)
304: #define BCM_IRQ_DMA3           GIC_SPI(0x53) // (bcm2835-dma)
305: /// @brief RPI DMA channel 4 interrupt (bcm2835-dma)
306: #define BCM_IRQ_DMA4           GIC_SPI(0x54) // (bcm2835-dma)
307: /// @brief RPI DMA channel 5 interrupt (bcm2835-dma)
308: #define BCM_IRQ_DMA5           GIC_SPI(0x55) // (bcm2835-dma)
309: /// @brief RPI DMA channel 6 interrupt (bcm2835-dma)
310: #define BCM_IRQ_DMA6           GIC_SPI(0x56) // (bcm2835-dma)
311: /// @brief RPI DMA channel 7 interrupt (bcm2835-dma)
312: #define BCM_IRQ_DMA7           GIC_SPI(0x57) // (bcm2835-dma)
313: /// @brief RPI DMA channel 8 interrupt (bcm2835-dma)
314: #define BCM_IRQ_DMA8           GIC_SPI(0x57) // same value
315: /// @brief RPI DMA channel 9 interrupt (bcm2835-dma)
316: #define BCM_IRQ_DMA9           GIC_SPI(0x58) // (bcm2835-dma)
317: /// @brief RPI DMA channel 10 interrupt (bcm2835-dma)
318: #define BCM_IRQ_DMA10          GIC_SPI(0x58) // same value
319: /// @brief RPI DMA channel 11 interrupt (bcm2835-dma)
320: #define BCM_IRQ_DMA11          GIC_SPI(0x59) // (bcm2711-dma)
321: /// @brief RPI DMA channel 12 interrupt (bcm2835-dma)
322: #define BCM_IRQ_DMA12          GIC_SPI(0x5A) // (bcm2711-dma)
323: /// @brief RPI DMA channel 13 interrupt (bcm2835-dma)
324: #define BCM_IRQ_DMA13          GIC_SPI(0x5B) // (bcm2711-dma)
325: /// @brief RPI DMA channel 14 interrupt (bcm2835-dma)
326: #define BCM_IRQ_DMA14          GIC_SPI(0x5C) // (bcm2711-dma)
327: /// @brief RPI Auxiliary Peripheral interrupt (bcm2835-aux-uart, bcm43438-bt)
328: #define BCM_IRQ_AUX            GIC_SPI(0x5D) // (bcm2835-aux-uart)
329: /// @brief TBD
330: #define BCM_IRQ_INTC           GIC_SPI(0x60) // (bcm2711-l2-intc) (HDMI0, HDMI1)
331: /// @brief RPI Videoscaler interrupt (bcm2835-hvs)
332: #define BCM_IRQ_VIDEOSCALER    GIC_SPI(0x61) // (bcm2711-hvs)
333: /// @brief TBD
334: #define BCM_IRQ_CCP2TX         GIC_SPI(0x62) // (rpivid-vid-decoder)
335: /// @brief RPI DSI0 interrupt (bcm2835-dsi0)
336: #define BCM_IRQ_DSI0           GIC_SPI(0x64) // (bcm2835-dsi0)
337: /// @brief RPI AVE interrupt (bcm2711-pixelvalve2)
338: #define BCM_IRQ_AVE            GIC_SPI(0x65) // (bcm2711-pixelvalve2)
339: /// @brief RPI CAM 0 interrupt (bcm2835-unicam)
340: #define BCM_IRQ_CAM0           GIC_SPI(0x66) // (bcm2835-unicam)
341: /// @brief RPI CAM 1 interrupt (bcm2835-unicam)
342: #define BCM_IRQ_CAM1           GIC_SPI(0x67) // (bcm2835-unicam)
343: /// @brief TBD
344: #define BCM_IRQ_PWA3           GIC_SPI(0x6A) // (bcm2711-pixelvalve3)
345: /// @brief RPI DSI interrupt (bcm2835-dsi1)
346: #define BCM_IRQ_DSI1           GIC_SPI(0x6C) // (bcm2711-dsi1)
347: /// @brief TBD
348: #define BCM_IRQ_PWA0           GIC_SPI(0x6D) // (bcm2711-pixelvalve0)
349: /// @brief TBD
350: #define BCM_IRQ_PWA1           GIC_SPI(0x6E) // (bcm2711-pixelvalve1, bcm2711-pixelvalve4)
351: /// @brief TBD
352: #define BCM_IRQ_SMI            GIC_SPI(0x70) // (rpi-firmware-kms-2711, bcm2835-smi)
353: /// @brief RPI GPIO 0 interrupt (bcm2835-gpio)
354: #define BCM_IRQ_GPIO0          GIC_SPI(0x71) // (bcm2711-gpio)
355: /// @brief RPI GPIO 1 interrupt (bcm2835-gpio)
356: #define BCM_IRQ_GPIO1          GIC_SPI(0x72) // (bcm2711-gpio)
357: /// @brief RPI GPIO 2 interrupt
358: #define BCM_IRQ_GPIO2          GIC_SPI(0x73)
359: /// @brief RPI GPIO 3 interrupt
360: #define BCM_IRQ_GPIO3          GIC_SPI(0x74)
361: /// @brief RPI I2C interrupt (bcm2835-i2c)
362: #define BCM_IRQ_I2C            GIC_SPI(0x75) // (bcm2835-i2c)
363: /// @brief RPI SPI interrupt (bcm2835-spi)
364: #define BCM_IRQ_SPI            GIC_SPI(0x76) // (bcm2835-spi)
365: /// @brief RPI EMMC / SDIO interrupt (bcm2835-sdhost)
366: #define BCM_IRQ_SDIO           GIC_SPI(0x78) // (bcm2835-sdhost)
367: /// @brief RPI UART interrupt (arm,primecell, serial@7e201000, bcm43438-bt)
368: #define BCM_IRQ_UART           GIC_SPI(0x79) // (arm,primecell)
369: /// @brief RPI VEC interrupt (bcm2835-vec, bcm2711-vec)
370: #define BCM_IRQ_VEC            GIC_SPI(0x7B) // (bcm2711-vec)
371: /// @brief RPI SDHCI (bcm2835-sdhci, bcm2711-emmc2)
372: #define BCM_IRQ_ARASANSDIO     GIC_SPI(0x7E) // (bcm2835-sdhci, bcm2711-emmc2)
373: /// @brief TBD
374: #define BCM_IRQ_PCIE_HOST_INTA GIC_SPI(0x8F) // (bcm2711-pcie)
375: /// @brief TBD
376: #define BCM_IRQ_PCIE_HOST_MSI2 GIC_SPI(0x93) // (bcm2711-pcie)
377: /// @brief TBD
378: #define BCM_IRQ_PCIE_HOST_MSI  GIC_SPI(0x94) // (bcm2711-pcie)
379: /// @brief TBD
380: #define BCM_IRQ_BCM54213_0     GIC_SPI(0x9D) // (bcm2711-genet-v5)
381: /// @brief TBD
382: #define BCM_IRQ_BCM54213_1     GIC_SPI(0x9E) // (bcm2711-genet-v5)
383: /// @brief TBD
384: #define BCM_IRQ_XHCI_INTERNAL  GIC_SPI(0xB0) // (generic-xhci)
385: 
386: /// @brief Total count of IRQ lines on RPI4
387: #define IRQ_LINES              256
388: 
389: #else // BAREMETAL_RPI_TARGET >= 5
390: 
391: /// @brief GIC interrupts private per core
392: #define GIC_PPI(n)             (16 + (n))
393: /// @brief GIC interrupts shared between cores
394: #define GIC_SPI(n)             (32 + (n))
395: 
396: // IRQs
397: /// @brief RPI Videoscaler interrupt (bcm2835-hvs)
398: #define BCM_IRQ_VIDEOSCALER    GIC_SPI(0x09) // (bcm2712-hvs)
399: 
400: #endif
401: 
402: // FIQ
403: 
404: #if BAREMETAL_RPI_TARGET <= 3
405: 
406: /// @brief RPI System Timer Compare 0 interrupt (bcm2835-system-timer)
407: #define BCM_FIQ_TIMER0         0
408: /// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
409: #define BCM_FIQ_TIMER1         1
410: /// @brief RPI System Timer Compare 2 interrupt (bcm2835-system-timer)
411: #define BCM_FIQ_TIMER2         2
412: /// @brief RPI System Timer Compare 3 interrupt (bcm2835-system-timer)
413: #define BCM_FIQ_TIMER3         3
414: /// @brief RPI Codec 0 interrupt (bcm2835-cprman)
415: #define BCM_FIQ_CODEC0         4
416: /// @brief RPI Codec 1 interrupt (bcm2835-cprman)
417: #define BCM_FIQ_CODEC1         5
418: /// @brief RPI Codec 2 interrupt
419: #define BCM_FIQ_CODEC2         6
420: /// @brief RPI JPEG interrupt
421: #define BCM_FIQ_JPEG           7
422: /// @brief TBD
423: #define BCM_FIQ_ISP            8
424: /// @brief RPI USB interrupt (bcm2708-usb)
425: #define BCM_FIQ_USB            9
426: /// @brief RPI VC4 3D interrupt (vc4-v3d)
427: #define BCM_FIQ_3D             10
428: /// @brief RPI Transposer (TXP) interrupt (bcm2835-txp)
429: #define BCM_FIQ_TRANSPOSER     11
430: /// @brief TBD
431: #define BCM_FIQ_MULTICORESYNC0 12
432: /// @brief TBD
433: #define BCM_FIQ_MULTICORESYNC1 13
434: /// @brief TBD
435: #define BCM_FIQ_MULTICORESYNC2 14
436: /// @brief TBD
437: #define BCM_FIQ_MULTICORESYNC3 15
438: /// @brief RPI DMA channel 0 interrupt (bcm2835-dma)
439: #define BCM_FIQ_DMA0           16
440: /// @brief RPI DMA channel 1 interrupt (bcm2835-dma)
441: #define BCM_FIQ_DMA1           17
442: /// @brief RPI DMA channel 2 interrupt (bcm2835-dma)
443: #define BCM_FIQ_DMA2           18
444: /// @brief RPI DMA channel 3 interrupt (bcm2835-dma)
445: #define BCM_FIQ_DMA3           19
446: /// @brief RPI DMA channel 4 interrupt (bcm2835-dma)
447: #define BCM_FIQ_DMA4           20
448: /// @brief RPI DMA channel 5 interrupt (bcm2835-dma)
449: #define BCM_FIQ_DMA5           21
450: /// @brief RPI DMA channel 6 interrupt (bcm2835-dma)
451: #define BCM_FIQ_DMA6           22
452: /// @brief RPI DMA channel 7 interrupt (bcm2835-dma)
453: #define BCM_FIQ_DMA7           23
454: /// @brief RPI DMA channel 8 interrupt (bcm2835-dma)
455: #define BCM_FIQ_DMA8           24
456: /// @brief RPI DMA channel 9 interrupt (bcm2835-dma)
457: #define BCM_FIQ_DMA9           25
458: /// @brief RPI DMA channel 10 interrupt (bcm2835-dma)
459: #define BCM_FIQ_DMA10          26
460: /// @brief RPI DMA channel 11/12/13/14 interrupt (bcm2835-dma)
461: #define BCM_FIQ_DMA11          27
462: /// @brief RPI DMA shared interrupt (bcm2835-dma)
463: #define BCM_FIQ_DMA_SHARED     28
464: /// @brief RPI Auxiliary Peripheral interrupt (bcm2835-aux-uart, bcm43438-bt)
465: #define BCM_FIQ_AUX            29
466: /// @brief TBD
467: #define BCM_FIQ_ARM            30
468: /// @brief TBD
469: #define BCM_FIQ_VPUDMA         31
470: /// @brief RPI USB Host interrupt (bcm2708-usb)
471: #define BCM_FIQ_HOSTPORT       32
472: /// @brief RPI Videoscaler interrupt (bcm2835-hvs)
473: #define BCM_FIQ_VIDEOSCALER    33
474: /// @brief TBD
475: #define BCM_FIQ_CCP2TX         34
476: /// @brief TBD
477: #define BCM_FIQ_SDC            35
478: /// @brief RPI DSI0 interrupt (bcm2835-dsi0)
479: #define BCM_FIQ_DSI0           36
480: /// @brief RPI AVE interrupt (bcm2711-pixelvalve2)
481: #define BCM_FIQ_AVE            37
482: /// @brief RPI CAM 0 interrupt (bcm2835-unicam)
483: #define BCM_FIQ_CAM0           38
484: /// @brief RPI CAM 1 interrupt (bcm2835-unicam)
485: #define BCM_FIQ_CAM1           39
486: /// @brief RPI HDMI 0 interrupt (bcm2835-hdmi)
487: #define BCM_FIQ_HDMI0          40
488: /// @brief RPI HDMI 1 interrupt (bcm2835-hdmi)
489: #define BCM_FIQ_HDMI1          41
490: /// @brief RPI Pixel valve 2 interrupt (bcm2835-pixelvalve2, bcm2711-pixelvalve3)
491: #define BCM_FIQ_PIXELVALVE1    42
492: /// @brief RPI I2C slave interrupt
493: #define BCM_FIQ_I2CSPISLV      43
494: /// @brief RPI DSI1 interrupt (bcm2835-dsi1)
495: #define BCM_FIQ_DSI1           44
496: /// @brief RPI Pixel valve 0 interrupt (bcm2835-pixelvalve0, bcm2711-pixelvalve0)
497: #define BCM_FIQ_PWA0           45
498: /// @brief RPI Pixel valve 1 interrupt (bcm2835-pixelvalve1, bcm2711-pixelvalve1, bcm2711-pixelvalve4)
499: #define BCM_FIQ_PWA1           46
500: /// @brief TBD
501: #define BCM_FIQ_CPR            47
502: /// @brief RPI SMI interrupt (bcm2835-smi, rpi-firmware-kms)
503: #define BCM_FIQ_SMI            48
504: /// @brief RPI GPIO 0 interrupt (bcm2835-gpio)
505: #define BCM_FIQ_GPIO0          49
506: /// @brief RPI GPIO 1 interrupt (bcm2835-gpio)
507: #define BCM_FIQ_GPIO1          50
508: /// @brief RPI GPIO 2 interrupt
509: #define BCM_FIQ_GPIO2          51
510: /// @brief RPI GPIO 3 interrupt
511: #define BCM_FIQ_GPIO3          52
512: /// @brief RPI I2C interrupt (bcm2835-i2c)
513: #define BCM_FIQ_I2C            53
514: /// @brief RPI SPI interrupt (bcm2835-spi)
515: #define BCM_FIQ_SPI            54
516: /// @brief RPI I2C audio interrupt
517: #define BCM_FIQ_I2SPCM         55
518: /// @brief RPI EMMC / SDIO interrupt (bcm2835-sdhost)
519: #define BCM_FIQ_SDIO           56
520: /// @brief RPI UART interrupt (arm,primecell, serial@7e201000, bcm43438-bt)
521: #define BCM_FIQ_UART           57
522: /// @brief TBD
523: #define BCM_FIQ_SLIMBUS        58
524: /// @brief RPI VEC interrupt (bcm2835-vec, bcm2711-vec)
525: #define BCM_FIQ_VEC            59
526: /// @brief TBD
527: #define BCM_FIQ_CPG            60
528: /// @brief RPI RNG interrupt (bcm2835-rng)
529: #define BCM_FIQ_RNG            61
530: /// @brief RPI SDHCI (bcm2835-sdhci, bcm2711-emmc2)
531: #define BCM_FIQ_ARASANSDIO     62
532: /// @brief TBD
533: #define BCM_FIQ_AVSPMON        63
534: /// @brief RPI ARM Timer interrupt interrupt
535: #define BCM_FIQ_BCM_TIMER      64
536: /// @brief RPI ARM Mailbox interrupt interrupt (bcm2835-mbox)
537: #define BCM_FIQ_BCM_MAILBOX    65
538: /// @brief RPI ARM Doorbell 0 interrupt interrupt (bcm2835-vchiq, bcm2711-vchiq)
539: #define BCM_FIQ_BCM_DOORBELL_0 66
540: /// @brief RPI ARM Doorbell 1 interrupt interrupt
541: #define BCM_FIQ_BCM_DOORBELL_1 67
542: /// @brief RPI ARM GPU 0 halted interrupt (bcm2835-cprman)
543: #define BCM_FIQ_VPU0_HALTED    68
544: /// @brief RPI ARM GPU 1 halted interrupt (bcm2835-cprman)
545: #define BCM_FIQ_VPU1_HALTED    69
546: /// @brief RPI ARM Illegal access type 1 interrupt
547: #define BCM_FIQ_ILLEGAL_TYPE0  70
548: /// @brief RPI ARM Illegal access type 0 interrupt
549: #define BCM_FIQ_ILLEGAL_TYPE1  71
550: 
551: /// @brief Maximum index of FIQ interrupts RPI3
552: #define BCM_MAX_FIQ            71
553: 
554: /// @brief FIQ interrupt enable bit
555: #define FIQ_INTR_ENABLE   BIT(7)
556: 
557: #else // BAREMETAL_RPI_TARGET >= 4
558: 
559: // FIQs
560: 
561: /// @brief RPI System Timer Compare 1 interrupt (bcm2835-system-timer)
562: #define BCM_FIQ_TIMER1 BCM_IRQ_TIMER1
563: /// @brief RPI GPIO 3 interrupt
564: #define BCM_FIQ_GPIO3  BCM_IRQ_GPIO3
565: /// @brief RPI UART interrupt (arm,primecell, serial@7e201000, bcm43438-bt)
566: #define BCM_FIQ_UART   BCM_IRQ_UART
567: 
568: /// @brief Maximum count of FIQ interrupts RPI4 and later
569: #define BCM_MAX_FIQ    IRQ_LINES
570: 
571: #endif // if BAREMETAL_RPI_TARGET <= 3
```

We'll not get into each and every IRQ or FIQ id, but give some descriptions on macros used

- Line 45-262: We define IRQ related macros for Raspberry PI 3
  - Line 50: We define the number of BCM interrupt registers
  - Line 53-57: We define the number of IRQ supported by IRQ 1/2, IEQ basic and ARM local IRQ
  - Line 60-66: We define the ID range for IRQ 1/2, IEQ basic and ARM local IRQ
  - Line 69-70: We define macros to access the IRQ pending register for BCM IRQ lines
  - Line 72-73: We define macros to access the IRQ enable register for BCM IRQ lines
  - Line 75-76: We define macros to access the IRQ disable register for BCM IRQ lines
  - Line 78: We define macros to determine the mask for the selected IRQ id to access the IRQ pending / enable / disable register
  - Line 83-258: We define all currently supported FIQ for Raspberry 3
  - Line 261: We define the total count of IRQ interrupts supported by the system for Raspberry 3
- Line 263-388: We define IRQ related macros for Raspberry PI 4
  - Line 266-268: We define macros to access private per core interrupts and interrupts shared by all cores for Raspberry 4
  - Line 272-384: We define all currently supported FIQ for Raspberry 4
  - Line 387: We define the total count of IRQ interrupts supported by the system for Raspberry 4
- Line 389-400: We define IRQ related macros for Raspberry PI 5
  - Line 392-394: We define macros to access private per core interrupts and interrupts shared by all cores for Raspberry 5
  - Line 398: We define all currently supported FIQ for Raspberry 5
- Line 404-556: We define FIQ related macros for Raspberry PI 3
  - Line 407-549: We define all currently supported FIQ for Raspberry 3
  - Line 552: We define the total count of FIQ interrupts supported by the system for Raspberry 3
  - Line 555: We define enable bit for FIQ for Raspberry 3
- Line 561-569: We define IRQ related macros for Raspberry PI 4 and later
  - Line 562-566: We define all currently supported FIQ for Raspberry 4 and later
  - Line 569: We define the total count of FIQ interrupts supported by the system for Raspberry 4 and later

### Update CMake file {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_UPDATE_CMAKE_FILE}

As we have not added any source files to the `baremetal` library, we do not need to update its CMake file.

### Update application code {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_UPDATE_APPLICATION_CODE}

We'll make a few small changes to use the new `InterruptSystem` implementation.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
...
28: int main()
29: {
30:     auto& console = GetConsole();
31: 
32:     MemoryAccess memoryAccess;
33: 
34:     auto exceptionLevel = CurrentEL();
35:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
36: 
37:     GetInterruptSystem().RegisterIRQHandler(ARM_IRQLOCAL0_CNTPNS, IntHandler, nullptr);
38: 
39:     uint64 counterFreq{};
40:     GetTimerFrequency(counterFreq);
41:     assert(counterFreq % TICKS_PER_SECOND == 0);
42:     clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
43: 
44:     uint64 counter;
45:     GetTimerCounter(counter);
46:     SetTimerCompareValue(counter + clockTicksPerSystemTick);
47:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
48: 
49:     LOG_INFO("Wait 5 seconds");
50:     Timer::WaitMilliSeconds(5000);
51: 
52:     SetTimerControl(0);
53: 
54:     GetInterruptSystem().UnregisterIRQHandler(ARM_IRQLOCAL0_CNTPNS);
55: 
56:     console.Write("Press r to reboot, h to halt\n");
57:     char ch{};
58:     while ((ch != 'r') && (ch != 'h'))
59:     {
60:         ch = console.ReadChar();
61:         console.WriteChar(ch);
62:     }
63: 
64:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
65: }
```

- Line 37: We replace the code to register the IRQ handler with a single call to `RegisterIRQHandler()`
- Line 54: We replace the code to unregister the IRQ handler with a single call to `UnregisterIRQHandler()`

### Configuring, building and debugging {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. 

The application will act exactly as before. We expect to see 10 ticks happening.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:208)
Info   Current EL: 1 (main:35)
Info   Wait 5 seconds (main:49)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Info   Ping (main:25)
Press r to reboot, h to halt
hInfo   Halt (System:129)
```

Next: [21-timer-extension](21-timer-extension.md)
