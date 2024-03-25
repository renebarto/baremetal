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

## Interrupt handling - Step 1 {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1}

The ARM processor supports two different kinds of interrupts:

- IRQ: Normal interrupts
- FIQ: Fast interrupts

FIQ is a specialized type of interrupt request, which is a standard technique used in computer CPUs to deal with events that need to be processed as they occur, such as receiving data from a network card, or keyboard or mouse actions.
FIQs are specific to the ARM architecture, which supports two types of interrupts; FIQs for fast, low-latency interrupt handling, and standard interrupt requests (IRQs), for more general interrupts.

A FIQ takes priority over an IRQ in an ARM system. Only one FIQ source at a time is supported. 
This helps reduce interrupt latency as the interrupt service routine can be executed directly without determining the source of the interrupt.
A context save is not required for servicing a FIQ since it has its own set of banked registers. This reduces the overhead of context switching.

FIQs are often used for data transfers such as DMA operations.

### ExceptionStub.S {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_EXCEPTIONSTUBS}

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

### ExceptionHandler.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_EXCEPTIONHANDLERH}

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

### ExceptionHandler.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_EXCEPTIONHANDLERCPP}

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

### InterruptHandler.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_INTERRUPTHANDLERH}

We will add the `InterruptHandler()` function.
Similar to waht we did for exceptions, we will also declare a class `InterruptSystem`, which is a singleton, and has its own implementation for `InterruptHandler()`.
We'll only create the bare minimum for this calss for now, but we'll extend it soon.

Create the file `code/libraries/baremetal/include/baremetal/InterruptHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptHandler.h
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
42: #include <baremetal/Interrupt.h>
43: #include <baremetal/Macros.h>
44: #include <baremetal/Types.h>
45: 
46: /// @file
47: /// Interrupt handler function
48: 
49: #ifdef __cplusplus
50: extern "C" {
51: #endif
52: 
53: void InterruptHandler();
54: 
55: #ifdef __cplusplus
56: }
57: #endif
58: 
59: namespace baremetal {
60: 
61: /// <summary>
62: /// IRQ handler function
63: /// </summary>
64: using IRQHandler = void(void* param);
65: 
66: /// <summary>
67: /// InterruptSystem: Handles IRQ and FIQ interrupts for Raspberry Pi
68: /// This is a SINGLETON class
69: /// </summary>
70: class InterruptSystem
71: {
72:     /// @brief Pointer to registered IRQ handler
73:     IRQHandler* m_irqHandler;
74:     /// @brief Pointer to parameter to pass to registered IRQ handler
75:     void* m_irqHandlerParams;
76: 
77:     friend InterruptSystem& GetInterruptSystem();
78: 
79: private:
80:     InterruptSystem();
81: 
82: public:
83:     ~InterruptSystem();
84: 
85:     void Initialize();
86: 
87:     void RegisterIRQHandler(IRQHandler* handler, void* param);
88:     void UnregisterIRQHandler();
89: 
90:     void InterruptHandler();
91: };
92: 
93: InterruptSystem& GetInterruptSystem();
94: 
95: } // namespace baremetal
```

- Line 53: We declare the function `InterruptHandler()`
- Line 64: We declare a callback function type `IRQHandler` to act as the handler for an IRQ
- Line 70-91: We declare the class `InterruptSystem`
  - Line 72: The member variable `m_irqHandler` stores the registered handler
  - Line 74: The member variable `m_irqHandlerParams` stores the parameter to pass to the registered handler
  - Line 77: We make `GetInterruptSystem()` a friend, so it can call the constructor
  - Line 80: We declare a private default constructor, such that only the `GetInterruptSystem()` function can create an instance
  - Line 83: We declare a destructor
  - Line 85: We declare a method `Initialize()` which will set up the interrupt system
  - Line 87: We declare a method `RegisterIRQHandler()` which will register a handler to be called when an interrupt occurs
  - Line 88: We declare a method `UnregisterIRQHandler()` which will unregister a registered handler
  - Line 90: We declate a method `InterruptHandler()` which is called by the global `InterruptHandler()` function
- Line 93: We declare the function `GetInterruptSystem()`, which creates the singleton instance of the `InterruptSystem` class if needed, and returns a reference to it

### InterruptHandler.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_INTERRUPTHANDLERCPP}

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

### ARMRegisters.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_ARMREGISTERSH}

Next to the Raspberry Pi registers for peripherals etc., there are also ARM processor registers, but specific for Raspberry Pi.
The complete set of registers is defined in [documentation](pdf/bcm2836-peripherals.pdf), the most important ones are described in [ARM local device registers](#RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS).

The ARM specific registers are in a different address range, reaching from 0x40000000 to 0x4003FFFF on Raspberry Pi 3, and 0xFF800000 to 0xFF83FFFF on Raspberry Pi 4 and later.

We'll add the definition for the register we will be using.

Create the file `code/libraries/baremetal/include/baremetal/ARMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMRegisters.h
File: d:\Projects\baremetal.github\code\libraries\baremetal\include\baremetal\ARMRegisters.h
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

### Update CMake file {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_UPDATE_CMAKE_FILE}

As we have now added some source files to the `baremetal` library, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
File: d:\Projects\baremetal.github\code\libraries\baremetal\CMakeLists.txt
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

### Update application code {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_20_INTERRUPTS_INTERRUPT_HANDLING__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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


## Interrupt System - Step 2 {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2}

We'll update the class `InterruptSystem` to enable, disable, and handle interrupts.

\todo

### InterruptSystem.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_INTERRUPTSYSTEMH}

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

### TestDetails.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_TESTDETAILSCPP}

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

### TestBase.h {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_TESTBASEH}

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

### TestBase.cpp {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_TESTBASECPP}

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

### Update CMake file {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_20_INTERRUPTS_INTERRUPT_SYSTEM__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

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
