# Tutorial 19: Interrupts {#TUTORIAL_19_INTERRUPTS}

@tableofcontents

## Tutorial setup {#TUTORIAL_19_INTERRUPTS_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/19-interrupts`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_19_INTERRUPTS_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-19.a`
- a library `output/Debug/lib/stdlib-19.a`
- an application `output/Debug/bin/19-interrupts.elf`
- an image in `deploy/Debug/19-interrupts-image`

## Interrupt handling - Step 1 {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1}

The ARM processor supports two different kinds of interrupts:

- IRQ: Normal interrupts
- FIQ: Fast interrupts

IRQ is a interrupt request, which is a standard technique used in computer CPUs to deal with events that need to be processed as they occur, such as receiving data from a network card, or keyboard or mouse actions.

FIQs are specific to the ARM architecture, which supports two types of interrupts; FIQs for fast, low-latency interrupt handling, and standard interrupt requests (IRQs), for more general interrupts.

A FIQ takes priority over an IRQ in an ARM system. Only one FIQ source at a time is supported.
This helps reduce interrupt latency as the interrupt service routine can be executed directly without determining the source of the interrupt.

FIQs are often used for data transfers such as DMA operations.

The ARM chip has many different interrupts, the Raspberry Pi SoC adds even more for each of the peripheral devices.

### ExceptionStub.S {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_EXCEPTIONSTUBS}

In order to support FIQ and IRQ interrupts, we need to update the exception assembly code.

Update the file `code/libraries/baremetal/src/ExceptionStub.S`

```cpp
File: code/libraries/baremetal/src/ExceptionStub.S
...
84: VectorTable:
85:
86:     // from current EL with sp_el0 (mode EL0, El1t, El2t, EL3t)
87:     vector  SynchronousStub
88:     vector  IRQStub
89:     vector  FIQStub
90:     vector  SErrorStub
91:
92:     // from current EL with sp_elx, x != 0  (mode El1h, El2h, EL3h)
93:     vector  SynchronousStub
94:     vector  IRQStub
95:     vector  FIQStub
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
298:     bic     x0, x0, #0xF                // Clear bit 0
299:     mov     x1, #9
300:     orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
301:     msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
302:     eret                                // Move to EL2h
303:
304:     .data
305:     .align  3
306:     .globl  s_fiqData
307: s_fiqData:                              // Matches FIQData:
308:     .quad   0                           // handler
309:     .quad   0                           // param
310:     .word   0                           // fiqID (unused)
311:
312: //*************************************************
313: // Abort stubs
314: //*************************************************
315:     stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
316:     stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
317:     stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
318:     irq_stub    IRQStub,                                    InterruptHandler
319:
320: // End
```

- Line 88-89: We now change the exception vector table to refer to the `FIQStub` or `IRQStub` for EL1t
- Line 94-95: We also change the exception vector table to refer to the `FIQStub` or `IRQStub` for EL1h
- Line 110-200: We create a macro `irq_stub` for an IRQ interrupt, which saves almost all registers, depending on the define `BAREMETAL_SAVE_VFP_REGS_ON_IRQ`
  - Line 116: We first save X29 and x30, the stack pointer is decremented by 16
  - Line 117-119: We then load ELR_EL1 and SPSR_EL1 and save these, again decrementing the stack pointer
  - Line 120: We enable the FIQ, to allow for priority
  - Line 122-139: If `BAREMETAL_SAVE_VFP_REGS_ON_IRQ` is defined, also all floating point registers (Q0-Q31) are saved, again decrementing the stack pointer
  - Line 140-154: We save x0-x29, again decrementing the stack pointer
  - Line 156: We call the interrupt handler passed into the macro
  - Line 158-172: We restore x0-29, and increment the stack pointer after each load
  - Line 173-190: If `BAREMETAL_SAVE_VFP_REGS_ON_IRQ` is defined, we restore Q0-Q31, again incrementing the stack pointer
  - Line 192: We disable the FIQ again
  - Line 193-195: We load x29 and x30 with the values for ELR_EL1 and SPSR_EL1 and restore these, again incrementing the stack pointer
  - Line 196: Finally we restore x29 and x30, again incrementing the stack pointer
  - Line 198: We return from the interrupt routing using `eret`.
This also reset the state of interrupt enables, etc. as well as the exception level.
Note that even though we disable FIQ here, returning to the original exception level may enable them again
- Line 202-291: We implement the FIQ interrupt handler `FIQStub`. This is similar to the normal interrupt stub, however this is not a macro.
Differences are:
  - Line 208: We don't need to save the ELR_EL1 and SPSR_EL1 registers, also we don't need to enable FIQ, as this FIQ cannot be interrupted
  - Line 242-247: We load the handler address as well as its parameter from the static memory area structure `s_fiqData`
  - Line 287-291: If no handler was set no function is called, we jump to a location that disables the FIQ altogether, and then restores the registers
- Line 304-310: We define the structure s_fiqData, which is 8 byte aligned. Notice this is part of the `.data` segment, just like the stubs from last tutorial
- Line 318: We declare interrupt stub using the `irq_stub` macro, and set its handler to `InterruptHandler()`

### BCMRegisters.h revisit {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_BCMREGISTERSH_REVISIT}

The handling of interrupts is very different between Raspberry Pi 3 and Raspberry Pi 4/5.
This is due to a different implementation of the interrupt controller.
We already added the registers for the interrupt controller in the previous tutorial, but did not go into any detail.
So let's have another look.

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
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
117: #define RPI_INTRCTRL_BASE              RPI_BCM_IO_BASE + 0x0000B000
118:
119: /// @brief Raspberry Pi Interrupt Control basic IRQ pending register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
120: #define RPI_INTRCTRL_IRQ_BASIC_PENDING reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000200)
121: /// @brief Raspberry Pi Interrupt Control register 1 IRQ pending register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
122: #define RPI_INTRCTRL_IRQ_PENDING_1     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000204)
123: /// @brief Raspberry Pi Interrupt Control register 2 IRQ pending register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
124: #define RPI_INTRCTRL_IRQ_PENDING_2     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000208)
125: /// @brief Raspberry Pi Interrupt Control FIQ enable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
126: #ifdef __cplusplus
127: #define RPI_INTRCTRL_FIQ_CONTROL reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x0000020C)
128: #else
129: #define RPI_INTRCTRL_FIQ_CONTROL (RPI_INTRCTRL_BASE + 0x0000020C)
130: #endif
131: /// @brief Raspberry Pi Interrupt Control register 1 IRQ enable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
132: #define RPI_INTRCTRL_ENABLE_IRQS_1      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000210)
133: /// @brief Raspberry Pi Interrupt Control register 2 IRQ enable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
134: #define RPI_INTRCTRL_ENABLE_IRQS_2      reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000214)
135: /// @brief Raspberry Pi Interrupt Control basic IRQ enable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
136: #define RPI_INTRCTRL_ENABLE_BASIC_IRQS  reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000218)
137: /// @brief Raspberry Pi Interrupt Control register 1 IRQ disable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
138: #define RPI_INTRCTRL_DISABLE_IRQS_1     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x0000021C)
139: /// @brief Raspberry Pi Interrupt Control register 2 IRQ disable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
140: #define RPI_INTRCTRL_DISABLE_IRQS_2     reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000220)
141: /// @brief Raspberry Pi Interrupt Control basic IRQ disable register. See @ref RASPBERRY_PI_INTERRUPT_CONTROL
142: #define RPI_INTRCTRL_DISABLE_BASIC_IRQS reinterpret_cast<regaddr>(RPI_INTRCTRL_BASE + 0x00000224)
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
```

Raspberry Pi 3 has an interrupt controller as part of the specific Broadcom SoC, BCM2835/6/7. It supports a total of 84 interrupts:
- 32 in the IRQ1 control group
- 32 in the IRQ2 control group
- 8 in the basic IRQ control group
- 12 ARM local interrupts in the ARM local control group

Interrupts are quite a subject in itself, and we will not go into detail here. Please refer to [Raspberry Pi Interrupt Control](#RASPBERRY_PI_INTERRUPT_CONTROL).

IRQ are enabled and disabled by writing a 1 bit to the corresponnding bit in the corresponding register, e.g. for `IRQ_ARM_TIMER`:
- Enabled by writing 1 to bit 0 of `RPI_INTRCTRL_ENABLE_BASIC_IRQS`
- disabled by writing 1 to bit 0 of `RPI_INTRCTRL_DISABLE_BASIC_IRQS`
- Interrupt pending is checked by reading bit 0 of `RPI_INTRCTRL_IRQ_BASIC_PENDING`

For the interrupts connected to ARM local, this is different, e.g. `IRQ_LOCAL_CNTPNS`:
- Enabled by writing 1 to bit 0 of `ARM_LOCAL_TIMER_INT_CONTROL0`
- Disabled by writing 0 to bit 0 of `ARM_LOCAL_TIMER_INT_CONTROL0`
- Interrupt pending is checked by reading bit 0 of `ARM_LOCAL_IRQ_PENDING0`

For Raspberry Pi 4 and 5 it is a bit more complicated in one way, but slightly better documented in another.
These board use the GIC400 standard interrupt controller integrated into the SoC.
This is a building block well documented by ARM.
There are even more interrupts connected, and these are again not so well documented.

The GIC supports three kinds of interrupts:
- 16 Software Generated Interrupts (SGI)
- 16 Private Peripheral Interrupts (PPI)
- 224 Shared Peripheral Interrupts (SPI)

This makes for a total of 256 IRQ, where the GIC can actually handle up to 1024 interrupt sources.

Again, for a more detailed explanation, please refer to [Raspberry Pi Interrupt Control](#RASPBERRY_PI_INTERRUPT_CONTROL).

Enabling / disabling interrupts on the GIC is a little more complicated, but the same for all interrupts, e.g. for `IRQ_LOCAL_CNTPNS`:

- Enabling the interrupt by writing a 1 to bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ISENABLER0`
- Disabling the interrupt by writing a 1 to bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ICENABLER0`
- Determining whether the interrupt is pending by reading bit 14 (0x0E) + 16 (offset for PPI) = bit 30 in `RPI_GICD_ISPEND0`

Setting up the GIC however, as well as handling the interrupt is somewhat more complicated.

### Interrupts.h {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTSH}

We'll add some of information (for the interrupts we're going to use so far) of the previous section into a header file, and add means to enable / disable interrupts.

Create the file `code/libraries/baremetal/include/baremetal/Interrupts.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Interrupts.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Interrupts.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Interrupts
9: //
10: // Description : Interrupt numbers and enable / disable functionality
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
42: #include "baremetal/BCMRegisters.h"
43:
44: namespace baremetal {
45:
46: #if BAREMETAL_RPI_TARGET == 3
47:
48: // IRQs
49: /// @brief Number of IRQ lines per register for regular ARM_IRQ1 and ARM_IRQ2
50: #define ARM_IRQS_PER_REG         32
51: /// @brief Number of IRQ lines for ARM_IRQ_BASIC
52: #define ARM_IRQS_BASIC_REG       8
53: /// @brief Number of IRQ lines for ARM_IRQ_LOCAL
54: #define ARM_IRQS_LOCAL_REG       12
55:
56: /// @brief Number of normal IRQ registers (IRQ1, IRQ2, Basic IRQ)
57: #define ARM_IRQS_NUM_REGS        3
58:
59: /// @brief Total number of IRQ lines Raspberry Pi 3
60: #define IRQ_LINES                (ARM_IRQS_PER_REG * 2 + ARM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)
61:
62: /// @brief Offset for interrupts in IRQ1 group
63: #define ARM_IRQ1_BASE            0
64: /// @brief Offset for interrupts in IRQ2 group
65: #define ARM_IRQ2_BASE            (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
66: /// @brief Offset for interrupts in Basic IRQ group
67: #define ARM_IRQ_BASIC_BASE       (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
68: /// @brief Offset for interrupts in ARM local group
69: #define ARM_IRQ_LOCAL_BASE       (ARM_IRQ_BASIC_BASE + ARM_IRQS_BASIC_REG)
70: /// @brief Calculate IRQ number for IRQ in IRQ1 group
71: #define ARM_IRQ1(n)              (ARM_IRQ1_BASE + (n))
72: /// @brief Calculate IRQ number for IRQ in IRQ2 group
73: #define ARM_IRQ2(n)              (ARM_IRQ2_BASE + (n))
74: /// @brief Calculate IRQ number for IRQ in Basic IRQ group
75: #define ARM_IRQ_BASIC(n)         (ARM_IRQ_BASIC_BASE + (n))
76: /// @brief Calculate IRQ number for IRQ in ARM local group
77: #define ARM_IRQ_LOCAL(n)         (ARM_IRQ_LOCAL_BASE + (n))
78:
79: /// @brief Check whether an IRQ is pending (only for IRQ1, IRQ2, Basic IRQ group)
80: #define ARM_IC_IRQ_PENDING(irq)  ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_IRQ_PENDING_1 : ((irq) < ARM_IRQBASIC_BASE ? RPI_INTRCTRL_IRQ_PENDING_2 : RPI_INTRCTRL_IRQ_BASIC_PENDING))
81: /// @brief Enable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
82: #define ARM_IC_IRQS_ENABLE(irq)  ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_ENABLE_IRQS_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_ENABLE_IRQS_2 : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
83: /// @brief Disable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
84: #define ARM_IC_IRQS_DISABLE(irq) ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_DISABLE_IRQS_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_DISABLE_IRQS_2 : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
85: /// @brief Calculate bitmask for an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
86: #define ARM_IRQ_MASK(irq)        BIT1((irq) & (ARM_IRQS_PER_REG - 1))
87:
88: /// <summary>
89: /// @brief IRQ interrupt numbers
90: /// </summary>
91: enum class IRQ_ID
92: {
93:     /// @brief Non secure Physical ARM timer
94:     IRQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00), // armv7-timer
95:     /// @brief Secure Physical ARM timer
96:     IRQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
97:     /// @brief Hypervisor Physical ARM timer
98:     IRQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
99:     /// @brief Virtual ARM timer
100:     IRQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
101:     /// @brief Mailbox 0
102:     IRQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
103:     /// @brief Mailbox 1
104:     IRQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
105:     /// @brief Mailbox 2
106:     IRQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
107:     /// @brief Mailbox 3
108:     IRQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
109: };
110:
111: /// <summary>
112: /// @brief FIQ interrupt numbers
113: /// </summary>
114: enum class FIQ_ID
115: {
116:     /// @brief Non secure Physical ARM timer
117:     FIQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00),
118:     /// @brief Secure Physical ARM timer
119:     FIQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
120:     /// @brief Hypervisor Physical ARM timer
121:     FIQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
122:     /// @brief Virtual ARM timer
123:     FIQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
124:     /// @brief Mailbox 0
125:     FIQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
126:     /// @brief Mailbox 1
127:     FIQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
128:     /// @brief Mailbox 2
129:     FIQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
130:     /// @brief Mailbox 3
131:     FIQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
132: };
133:
134: #define ARM_MAX_FIQ 71
135:
136: #else
137:
138: // IRQs
139: /// @brief Software generated interrupt, per core
140: #define GIC_SGI(n)  (0 + (n))
141: /// @brief Private peripheral interrupt, per core
142: #define GIC_PPI(n)  (16 + (n))
143: /// @brief Shared peripheral interrupt, shared between cores
144: #define GIC_SPI(n)  (32 + (n))
145:
146: /// @brief Total number of IRQ lines Raspberry Pi 4 and 5
147: #define IRQ_LINES   256
148:
149: /// <summary>
150: /// @brief IRQ interrupt numbers
151: /// </summary>
152: enum class IRQ_ID
153: {
154:     IRQ_LOCAL_CNTHP = GIC_PPI(0x0A),
155:     IRQ_LOCAL_CNTV = GIC_PPI(0x0B),
156:     IRQ_LOCAL_CNTPS = GIC_PPI(0x0D),
157:     IRQ_LOCAL_CNTPNS = GIC_PPI(0x0E),
158: };
159:
160: /// <summary>
161: /// @brief FIQ interrupt numbers
162: /// </summary>
163: enum class FIQ_ID
164: {
165:     FIQ_LOCAL_CNTHP = GIC_PPI(0x0A),
166:     FIQ_LOCAL_CNTV = GIC_PPI(0x0B),
167:     FIQ_LOCAL_CNTPS = GIC_PPI(0x0D),
168:     FIQ_LOCAL_CNTPNS = GIC_PPI(0x0E),
169: };
170:
171: #define ARM_MAX_FIQ IRQ_LINES
172:
173: #endif
174:
175: void EnableIRQ(IRQ_ID irqID);
176: void DisableIRQ(IRQ_ID irqID);
177:
178: void EnableFIQ(FIQ_ID fiqID);
179: void DisableFIQ(FIQ_ID fiqID);
180:
181: } // namespace baremetal
```

- Line 49-54: We define different interrupt groups for Raspberry Pi 3
- Line 56-57: We define the number of IRQ registers defined for Raspberry Pi 3 as `ARM_IRQS_NUM_REGS`
- Line 59-60: We define the number of IRQ requests defined for Raspberry Pi 3 as `IRQ_LINES`
- Line 62-75: We define the offsets and macros to calculate the IRQ number for each group
- Line 79-86: We define macros to calculate the address to check whether an interrupt is pending (`ARM_IC_IRQ_PENDING`),
to enable it (`ARM_IC_IRQS_ENABLE`), or to disable it (`ARM_IC_IRQS_DISABLE`), as well as a macro `ARM_IRQ_MASK` to calculate the bit mask for an IRQ
- Line 88-109: We define the enum `IRQ_ID` for Raspberry Pi 3, with the ARM local interrupts added
- Line 111-132: We define the enum `FIQ_ID` for Raspberry Pi 3, with the ARM local interrupts added
- Line 134: We define the number of FIQ interrupts defined for Raspberry Pi 3 as `ARM_MAX_FIQ`
- Line 141-144: We define different GIC interrupt groups for Raspberry Pi 4 / 5
- Line 146-147: We define the number of IRQ requests defined for Raspberry Pi 4 / 5 as `IRQ_LINES`
- Line 149-158: We define the enum `IRQ_ID` for Raspberry Pi 4 / 5, with the ARM local timer interrupts added
- Line 160-169: We define the enum `FIQ_ID` for Raspberry Pi 4 / 5, with the ARM local timer interrupts added
- Line 171: We define the number of FIQ interrupts defined for Raspberry Pi 4 / 5 as `ARM_MAX_FIQ`
- Line 175: We declare the function `EnableIRQ()` to enable an IRQ
- Line 176: We declare the function `DisableIRQ()` to disable an IRQ
- Line 178: We declare the function `EnableIRQ()` to enable a FIQ
- Line 179: We declare the function `DisableIRQ()` to disable a FIQ

Quite some details for Raspberry Pi 5 are still unknown (at least to me), I'll be adding more later.

### Interrupts.cpp {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTSCPP}

Let's now implement the functions we declared.

Create the file `code/libraries/baremetal/src/Interrupts.cpp`

```cpp
File: code/libraries/baremetal/src/Interrupts.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Interrupts.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Interrupts
9: //
10: // Description : Interrupt numbers and enable / disable functionality
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
40: #include "baremetal/Interrupts.h"
41:
42: #include "baremetal/ARMRegisters.h"
43: #include "baremetal/Assert.h"
44: #include "baremetal/Logger.h"
45: #include "baremetal/MemoryAccess.h"
46:
47: /// @file
48: /// Interrupt numbers and enable / disable functionality
49:
50: /// @brief Define log name
51: LOG_MODULE("Interrupts");
52:
53: /// @brief Convert irq number to bit pattern for register on GIC (Raspberry Pi 4/5)
54: #define RPI_GICD_IRQ_MASK(irq)        BIT1(irq % 32)
55:
56: /// <summary>
57: /// Enable the requested IRQ
58: /// </summary>
59: /// <param name="irqID">IRQ to enable</param>
60: void baremetal::EnableIRQ(IRQ_ID irqID)
61: {
62:     int irq = static_cast<int>(irqID);
63:     assert(irq < IRQ_LINES);
64:     MemoryAccess mem;
65: #if BAREMETAL_RPI_TARGET == 3
66:
67:     if (irq < ARM_IRQ_LOCAL_BASE)
68:     {
69:         mem.Write32(ARM_IC_IRQS_ENABLE(irq), ARM_IRQ_MASK(irq));
70:     }
71:     else
72:     {
73:         // The only implemented local IRQs so far
74:         assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
75:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(irq - ARM_IRQ_LOCAL_BASE));
76:     }
77:
78: #else
79:
80:     mem.Write32(RPI_GICD_ISENABLER0 + 4 * (irq / 32), RPI_GICD_IRQ_MASK(irq));
81:
82: #endif
83: }
84:
85: /// <summary>
86: /// Disable the requested IRQ
87: /// </summary>
88: /// <param name="irqID">IRQ to disable</param>
89: void baremetal::DisableIRQ(IRQ_ID irqID)
90: {
91:     int irq = static_cast<int>(irqID);
92:     assert(irq < IRQ_LINES);
93:     MemoryAccess mem;
94: #if BAREMETAL_RPI_TARGET == 3
95:
96:     if (irq < ARM_IRQ_LOCAL_BASE)
97:     {
98:         mem.Write32(ARM_IC_IRQS_DISABLE(irq), ARM_IRQ_MASK(irq));
99:     }
100:     else
101:     {
102:         // The only implemented local IRQs so far
103:         assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
104:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(irq - ARM_IRQ_LOCAL_BASE));
105:     }
106:
107: #else
108:
109:     mem.Write32(RPI_GICD_ICENABLER0 + 4 * (irq / 32), BIT1(irq % 32));
110:
111: #endif
112: }
113:
114: /// <summary>
115: /// Enable the requested FIQ
116: /// </summary>
117: /// <param name="fiqID">FIQ to enable</param>
118: void baremetal::EnableFIQ(FIQ_ID fiqID)
119: {
120:     int fiq = static_cast<int>(fiqID);
121:     assert(fiq <= IRQ_LINES);
122:     MemoryAccess mem;
123: #if BAREMETAL_RPI_TARGET == 3
124:
125:     if (fiq < ARM_IRQ_LOCAL_BASE)
126:     {
127:         mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, fiq | BIT1(7));
128:     }
129:     else
130:     {
131:         // The only implemented local IRQs so far
132:         assert(fiqID == FIQ_ID::FIQ_LOCAL_CNTPNS);
133:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
134:                     mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(fiq - ARM_IRQ_LOCAL_BASE + 4)); // FIQ enable bits are bit 4..7
135:     }
136:
137: #else
138:
139:     LOG_PANIC("FIQ not supported yet");
140:
141: #endif
142: }
143:
144: /// <summary>
145: /// Disable the requested FIQ
146: /// </summary>
147: /// <param name="fiqID">FIQ to disable</param>
148: void baremetal::DisableFIQ(FIQ_ID fiqID)
149: {
150:     int fiq = static_cast<int>(fiqID);
151:     assert(fiq <= IRQ_LINES);
152:     MemoryAccess mem;
153: #if BAREMETAL_RPI_TARGET == 3
154:
155:     if (fiq < ARM_IRQ_LOCAL_BASE)
156:     {
157:         mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
158:     }
159:     else
160:     {
161:         // The only implemented local IRQs so far
162:         assert(fiqID == FIQ_ID::FIQ_LOCAL_CNTPNS);
163:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
164:                     mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(fiq - ARM_IRQ_LOCAL_BASE + 4)); // FIQ enable bits are bit 4..7
165:     }
166:
167: #else
168:
169:     LOG_PANIC("FIQ not supported yet");
170:
171: #endif
172: }
```

- Line 42: We include the header for defintions of ARM specific registers.
We still need to add these
- Line 53-54: We create a definition to convert an IRQ number to a bit pattern for Raspberry Pi 4/5
- Line 56-83: We implement the function `EnableIRQ()`
  - Line 62-63: We convert the IRQ to an integer, and perform a sanity check
  - Line 64: We create a `MemoryAccess` instance for register access
  - Line 67-70: For Raspberry Pi 3, if the IRQ is in the IRQ1, IRQ2 or Basic IRQ group, we use the macro `ARM_IC_IRQS_ENABLE` to determine the register to write to, and the macro `ARM_IRQ_MASK` to determine the bitmask to write.
We then write the bit pattern to the selected address
  - Line 72-76: Otherwise we check it is `IRQ_LOCAL_CNTPNS` (the only ARM local interrupt we support so far), and write to the ARM local interrupt control register to set the correct bit
  - Line 80: For Raspberry Pi 4 / 5, we calculate the register address `RPI_GICD_ISENABLERn` (there are 8 register of 32 bits each) and the value to write.
We then write the bit pattern to the selected address
- Line 85-112: We implement the function `DisableIRQ()`
  - Line 91-92: We convert the IRQ to an integer, and perform a sanity check
  - Line 93: We create a `MemoryAccess` instance for register access
  - Line 96-99: For Raspberry Pi 3, if the IRQ is in the IRQ1, IRQ2 or Basic IRQ group, we use the macro `ARM_IC_IRQS_DISABLE` to determine the register to write to, and the macro `ARM_IRQ_MASK` to determine the bitmask to write.
We then write the bit pattern to the selected address
  - Line 101-105: Otherwise we check it is `IRQ_LOCAL_CNTPNS` (the only ARM local interrupt we support so far), and write to the ARM local interrupt control register to reset the correct bit
  - Line 109: For Raspberry Pi 4 / 5, we calculate the register address `RPI_GICD_ICENABLERn` (there are 8 register of 32 bits each) and the value to write.
We then write the bit pattern to the selected address
- Line 114-142: We implement the function `EnableFIQ()`
  - Line 120-121: We convert the FIQ to an integer, and perform a sanity check
  - Line 122: We create a `MemoryAccess` instance for register access
  - Line 126-128: For Raspberry Pi 3, if the FIQ is in the IRQ1, IRQ2 or Basic IRQ group, we write the FIQ id to the `RPI_INTRCTRL_FIQ_CONTROL` register and set bit 7 to flag enabling the FIQ
  - Line 129-134: Otherwise we check it is `FIQ_LOCAL_CNTPNS` (the only ARM local interrupt we support so far), and write to the ARM local interrupt control register to set the correct bit
  - Line 138: For Raspberry Pi 4 / 5, we perform a panic, as we will need to perform a Secure Monitor call in order to set the FIQ, which we did not implement yet.
  As we don't intend to use FIQ yet, we will simply fail for now
- Line 144-172: We implement the function `DisableFIQ()`
  - Line 150-151: We convert the FIQ to an integer, and perform a sanity check
  - Line 152: We create a `MemoryAccess` instance for register access
  - Line 155-158: For Raspberry Pi 3, if the IRQ is in the IRQ1, IRQ2 or Basic IRQ group, we write 0 to the `RPI_INTRCTRL_FIQ_CONTROL` register, resetting set bit 7 to flag no FIQ is enabled
  - Line 160-165: Otherwise we check it is `FIQ_LOCAL_CNTPNS` (the only ARM local interrupt we support so far), and write to the ARM local interrupt control register to reset the correct bit
  - Line 169: For Raspberry Pi 4 / 5, we perform a panic, as we will need to perform a Secure Monitor call in order to set the FIQ, which we did not implement yet.
  As we don't intend to use FIQ yet, we will simply fail for now

### InterruptHandler.h {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERH}

We will add the `InterruptHandler()` function.
Similar to what we did for exceptions, we will also declare a class `InterruptSystem`, which is a singleton, and has its own implementation for `InterruptHandler()`.
We'll only create the bare minimum for this calss for now, but we'll extend it soon.

Create the file `code/libraries/baremetal/include/baremetal/InterruptHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptHandler.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : InterruptHandler.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : InterruptSystem
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
42: #include "stdlib/Macros.h"
43: #include "stdlib/Types.h"
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
60: class IMemoryAccess;
61:
62: /// <summary>
63: /// IRQ handler function
64: /// </summary>
65: using IRQHandler = void(void* param);
66:
67: /// <summary>
68: /// InterruptSystem: Handles IRQ and FIQ interrupts for Raspberry Pi
69: /// This is a SINGLETON class
70: /// </summary>
71: class InterruptSystem
72: {
73:     /// @brief True if class is already initialized
74:     bool m_isInitialized;
75:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
76:     IMemoryAccess& m_memoryAccess;
77:     /// @brief Pointer to registered IRQ handler
78:     IRQHandler* m_irqHandler;
79:     /// @brief Pointer to parameter to pass to registered IRQ handler
80:     void* m_irqHandlerParams;
81:
82:     /// <summary>
83:     /// Construct the singleton InterruptSystem instance if needed, and return a reference to the instance. This is a friend function of class InterruptSystem
84:     /// </summary>
85:     /// <returns>Reference to the singleton InterruptSystem instance</returns>
86:     friend InterruptSystem& GetInterruptSystem();
87:
88: private:
89:     InterruptSystem();
90:
91: public:
92:     ~InterruptSystem();
93:
94:     void Initialize();
95:
96:     void RegisterIRQHandler(IRQHandler* handler, void* param);
97:     void UnregisterIRQHandler();
98:
99:     void InterruptHandler();
100: };
101:
102: InterruptSystem& GetInterruptSystem();
103:
104: } // namespace baremetal
```

- Line 52: We declare the global function `InterruptHandler()`.
Notice that like the `ExceptionHandler()`, this is declared as a `C` function, in order for the linker to link it to the assembly code
- Line 62-65: We declare a callback function type `IRQHandler` to act as the handler for an IRQ
- Line 67-100: We declare the class `InterruptSystem`, much like `ExceptionSystem`
  - Line 73-74: The member variable `m_isInitialized` guards against multiple initialization
  - Line 75-76: The member variable `m_memoryAccess` stores the `IMemoryAccess` instance used.
For now we'll simply use the singleton instance
  - Line 77-78: The member variable `m_irqHandler` stores the registered handler
  - Line 79-80: The member variable `m_irqHandlerParams` stores the parameter to pass to the registered handler
  - Line 86: We make `GetInterruptSystem()` a friend, so it can call the constructor to return a singleton instance of the interrup system
  - Line 89: We declare a private default constructor, such that only the `GetInterruptSystem()` function can create an instance
  - Line 92: We declare a destructor
  - Line 94: We declare a method `Initialize()` which will set up the interrupt system
  - Line 96: We declare a method `RegisterIRQHandler()` which will register a handler to be called when an interrupt occurs
  - Line 97: We declare a method `UnregisterIRQHandler()` which will unregister a registered handler
  - Line 99: We declate a method `InterruptHandler()` which is called by the global `InterruptHandler()` function
- Line 102: We declare the function `GetInterruptSystem()`, which creates the singleton instance of the `InterruptSystem` class if needed, and returns a reference to it

### InterruptHandler.cpp {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERCPP}

We'll implement the `InterruptHandler()` function as well as the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : InterruptHandler.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : InterruptSystem
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
40: #include "baremetal/InterruptHandler.h"
41:
42: #include "baremetal/Assert.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Interrupts.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/MemoryAccess.h"
47:
48: /// @file
49: /// Interrupt handler function implementation
50:
51: using namespace baremetal;
52:
53: /// @brief Define log name
54: LOG_MODULE("InterruptHandler");
55:
56: /// <summary>
57: /// Global interrupt handler function
58: ///
59: /// Is called by the vector table, and relays the call to the singleton InterruptHandler instance
60: /// </summary>
61: void InterruptHandler()
62: {
63:     GetInterruptSystem().InterruptHandler();
64: }
65:
66: /// <summary>
67: /// Create a interrupt system
68: ///
69: /// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
70: /// </summary>
71: InterruptSystem::InterruptSystem()
72:     : m_isInitialized{}
73:     , m_memoryAccess{GetMemoryAccess()}
74:     , m_irqHandler{}
75:     , m_irqHandlerParams{}
76: {
77: }
78:
79: /// <summary>
80: /// Destructor/// </summary>
81: InterruptSystem::~InterruptSystem()
82: {
83: }
84:
85: /// <summary>
86: /// Initialize interrupt system
87: /// </summary>
88: void InterruptSystem::Initialize()
89: {
90:     if (m_isInitialized)
91:         return;
92: #if BAREMETAL_RPI_TARGET == 3
93: #else
94:     // initialize distributor:
95:
96:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
97:
98:     // disable, acknowledge and deactivate all interrupts
99:     for (unsigned n = 0; n < IRQ_LINES / 32; n++)
100:     {
101:         m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
102:         m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
103:         m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
104:     }
105:
106:     // direct all interrupts to core 0 with default priority
107:     for (unsigned n = 0; n < IRQ_LINES / 4; n++)
108:     {
109:         m_memoryAccess.Write32(RPI_GICD_IPRIORITYR0 + 4 * n, RPI_GICD_IPRIORITYR_DEFAULT | RPI_GICD_IPRIORITYR_DEFAULT << 8 | RPI_GICD_IPRIORITYR_DEFAULT << 16 | RPI_GICD_IPRIORITYR_DEFAULT << 24);
110:
111:         m_memoryAccess.Write32(RPI_GICD_ITARGETSR0 + 4 * n, RPI_GICD_ITARGETSR_CORE0 | RPI_GICD_ITARGETSR_CORE0 << 8 | RPI_GICD_ITARGETSR_CORE0 << 16 | RPI_GICD_ITARGETSR_CORE0 << 24);
112:     }
113:
114:     // set all interrupts to level triggered
115:     for (unsigned n = 0; n < IRQ_LINES / 16; n++)
116:     {
117:         m_memoryAccess.Write32(RPI_GICD_ICFGR0 + 4 * n, 0);
118:     }
119:
120:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
121:
122:     // initialize core 0 CPU interface:
123:
124:     m_memoryAccess.Write32(RPI_GICC_PMR, RPI_GICC_PMR_PRIORITY);
125:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
126:
127: #endif
128:     m_isInitialized = true;
129: }
130:
131: /// <summary>
132: /// Register an IRQ handler
133: /// </summary>
134: /// <param name="handler">Handler to register</param>
135: /// <param name="param">Parameter to pass to IRQ handler</param>
136: void InterruptSystem::RegisterIRQHandler(IRQHandler* handler, void* param)
137: {
138:     assert(m_irqHandler == nullptr);
139:
140:     m_irqHandler = handler;
141:     m_irqHandlerParams = param;
142: }
143:
144: /// <summary>
145: /// Unregister an IRQ handler
146: /// </summary>
147: void InterruptSystem::UnregisterIRQHandler()
148: {
149:     assert(m_irqHandler != nullptr);
150:
151:     m_irqHandler = nullptr;
152:     m_irqHandlerParams = nullptr;
153: }
154:
155: /// <summary>
156: /// Handles an interrupt.
157: ///
158: /// The interrupt handler is called from assembly code (ExceptionStub.S)
159: /// </summary>
160: void InterruptSystem::InterruptHandler()
161: {
162: #if BAREMETAL_RPI_TARGET == 3
163:     if (m_irqHandler)
164:         m_irqHandler(m_irqHandlerParams);
165:
166: #else
167:
168:     uint32 iarValue = m_memoryAccess.Read32(RPI_GICC_IAR); // Read Interrupt Acknowledge Register
169:
170:     unsigned irq = iarValue & RPI_GICC_IAR_INTERRUPT_ID_MASK; // Select the currently active interrupt
171:     if (irq < IRQ_LINES)
172:     {
173:         if (irq >= GIC_PPI(0))
174:         {
175:             // Peripheral interrupts (PPI and SPI)
176:             m_irqHandler(m_irqHandlerParams);
177:         }
178:         else
179:         {
180:             // Handle SGI interrupt
181:         }
182:         m_memoryAccess.Write32(RPI_GICC_EOIR, iarValue); // Flag end of interrupt
183:     }
184: #ifndef NDEBUG
185:     else
186:     {
187:         // spurious interrupts
188:         assert(irq >= 1020);
189:         LOG_INFO("Received spurious interrupt %d", iarValue);
190:     }
191: #endif
192:
193: #endif
194: }
195:
196: /// <summary>
197: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
198: ///
199: /// This is a friend function of class InterruptSystem
200: /// </summary>
201: /// <returns>Reference to the singleton interrupt system instance</returns>
202: InterruptSystem& baremetal::GetInterruptSystem()
203: {
204:     static InterruptSystem singleton;
205:     singleton.Initialize();
206:     return singleton;
207: }
```

- Line 56-64: We implement the `InterruptHandler()` function by calling the `InterruptHandler()` method on the singleton `InterruptSystem` instance
- Line 66-77: We implement the constructor
- Line 79-83: We implement the destructor
- Line 85-135: We implement the `Initialize()` method
  - 90-91: We check if initialiation was already done, if so we return
  - 92: For Raspberry Pi 3 we dont need to do anything
  - 96: For Raspberry Pi 4 / 5 we disable the GIC distributor, basically stopping all interrupts
  - 99-104: We disable, acknowledge and deactivate all interrupts.
Notice that we do this for all 8 registers of 32 bits each
  - 107-112: We direct all interrupts to core 0 with default priority.
Notice that priorities take 8 bits, so 4 interrupts are in a 32 bit register, same for the target core
  - 115-118: We set all interrupts to level triggered.
Notice that priorities take 2 bits, so 16 interrupts are in a 32 bit register
  - 120: We re-enable the GIC distributor
  - 124: We set the priority mask to the default priority
  - 125: We enable the GIC CPU interface
  - 128: We set the initialization flag to true
- Line 131-142: We implement the `RegisterIRQHandler()` method. For now this is a simple implementation, we'll extend this later
  - Line 138: We do a sanity check that no interrupt handler is registered yet
  - Line 140-141: We store the handler and its parameter
- Line 144-153: We implement the `UnregisterIRQHandler()` method
  - Line 149: We do a sanity check that an interrupt handler is registered
  - Line 151-152: We clear the handler and its parameter
- Line 155-194: We implement the `InterruptHandler()` method. This will call the handler, if installed
  - Line 163-164: For Raspberry Pi 3, if a handler is installed, we call it
  - Line 168: For Raspberry Pi 4 / 5, we read the Interrupt Acknowledge Register, which denotes the interrupt we need to handle
  - Line 170: We extract the interrupt number
  - Line 171-183: If the interrupt is within the range of defined interrupts, we handle it
    - Line 173-174: If the interrupt is a peripheral interrupt (PPI), we call the handler
    - Line 180: SGI interrupts are currently ignored
    - Line 182: We acknowledge the interrupt
    - Line 186-190: Interrupts with an ID of 1020 or higher are spurious interrupts.
We do a sanity check that this is indeed a spurious interrupt, and print a message
- Line 196-207: We implement the `GetInterruptSystem()` function.
This will create the singleton instance and initialize it if needed, and return a reference to it

### ARMRegisters.h {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_ARMREGISTERSH}

Next to the Raspberry Pi registers for peripherals etc., and ARM processor specific registers, there are also ARM registers, but specific for Raspberry Pi.
These registers are mostly used for the legacy interrupts.
These are not well documented, however there is an [addendum](pdf/bcm2836-additional-info.pdf), see also [ARM local device registers](#RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS).

The ARM specific registers are in a different address range, reaching from 0x40000000 to 0x4003FFFF on Raspberry Pi 3, and 0xFF800000 to 0xFF83FFFF on Raspberry Pi 4 and later.

Create the file `code/libraries/baremetal/include/baremetal/ARMRegisters.h`

```cpp
File: code/libraries/baremetal/include/baremetal/ARMRegisters.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: /// @file
41: /// Register addresses of Raspberry Pi ARM local registers.
42: ///
43: /// ARM local registers are implemented in the Raspberry Pi BCM chip, but are intended to control functionality in the ARM core.
44: /// For specific registers, we also define the fields and their possible values.
45:
46: #pragma once
47:
48: #include "stdlib/Macros.h"
49: #include "stdlib/Types.h"
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
60: #define ARM_LOCAL_CONTROL              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000000)
61: /// @brief Raspberry Pi ARM Local Core Timer Prescaler Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
62: #define ARM_LOCAL_PRESCALER            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000008)
63: /// @brief Raspberry Pi ARM Local GPU Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
64: #define ARM_LOCAL_GPU_INT_ROUTING      reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000000C)
65: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
66: #define ARM_LOCAL_PM_ROUTING_SET       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000010)
67: /// @brief Raspberry Pi ARM Local Performance Monitor Interrupt Routing Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
68: #define ARM_LOCAL_PM_ROUTING_CLR       reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000014)
69: /// @brief Raspberry Pi ARM Local Core Timer Least Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
70: #define ARM_LOCAL_TIMER_LS             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000001C)
71: /// @brief Raspberry Pi ARM Local Core Timer Most Significant Word Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
72: #define ARM_LOCAL_TIMER_MS             reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000020)
73: /// @brief Raspberry Pi ARM Local Interrupt Routing Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
74: #define ARM_LOCAL_INT_ROUTING          reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000024)
75: /// @brief Raspberry Pi ARM Local AXI Outstanding Read/Write Counters Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
76: #define ARM_LOCAL_AXI_COUNT            reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000002C)
77: /// @brief Raspberry Pi ARM Local AXI Outstanding Interrupt Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
78: #define ARM_LOCAL_AXI_IRQ              reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000030)
79: /// @brief Raspberry Pi ARM Local Timer Control / Status Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
80: #define ARM_LOCAL_TIMER_CONTROL        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000034)
81: /// @brief Raspberry Pi ARM Local Timer IRQ Clear / Reload Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
82: #define ARM_LOCAL_TIMER_WRITE          reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000038)
83:
84: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 0 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
85: #define ARM_LOCAL_TIMER_INT_CONTROL0   reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000040)
86: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 1 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
87: #define ARM_LOCAL_TIMER_INT_CONTROL1   reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000044)
88: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 2 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
89: #define ARM_LOCAL_TIMER_INT_CONTROL2   reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000048)
90: /// @brief Raspberry Pi ARM Local Core Timer Interrupt Control Core 3 Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
91: #define ARM_LOCAL_TIMER_INT_CONTROL3   reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000004C)
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
103: #define ARM_LOCAL_IRQ_PENDING0         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000060)
104: /// @brief Raspberry Pi ARM Local Core 1 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
105: #define ARM_LOCAL_IRQ_PENDING1         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000064)
106: /// @brief Raspberry Pi ARM Local Core 2 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
107: #define ARM_LOCAL_IRQ_PENDING2         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000068)
108: /// @brief Raspberry Pi ARM Local Core 3 Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
109: #define ARM_LOCAL_IRQ_PENDING3         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000006C)
110:
111: /// @brief Raspberry Pi ARM Local Core 0 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
112: #define ARM_LOCAL_FIQ_PENDING0         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000070)
113: /// @brief Raspberry Pi ARM Local Core 1 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
114: #define ARM_LOCAL_FIQ_PENDING1         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000074)
115: /// @brief Raspberry Pi ARM Local Core 2 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
116: #define ARM_LOCAL_FIQ_PENDING2         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000078)
117: /// @brief Raspberry Pi ARM Local Core 3 Fast Interrupt Source Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
118: #define ARM_LOCAL_FIQ_PENDING3         reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000007C)
119:
120: /// @brief Raspberry Pi ARM Local Interrupt Source Timer 0
121: #define ARM_LOCAL_INTSRC_TIMER0        BIT1(0)
122: /// @brief Raspberry Pi ARM Local Interrupt Source Timer 1
123: #define ARM_LOCAL_INTSRC_TIMER1        BIT1(1)
124: /// @brief Raspberry Pi ARM Local Interrupt Source Timer 2
125: #define ARM_LOCAL_INTSRC_TIMER2        BIT1(2)
126: /// @brief Raspberry Pi ARM Local Interrupt Source Timer 3
127: #define ARM_LOCAL_INTSRC_TIMER3        BIT1(3)
128: /// @brief Raspberry Pi ARM Local Interrupt Source Mailbox 0
129: #define ARM_LOCAL_INTSRC_MAILBOX0      BIT1(4)
130: /// @brief Raspberry Pi ARM Local Interrupt Source Mailbox 1
131: #define ARM_LOCAL_INTSRC_MAILBOX1      BIT1(5)
132: /// @brief Raspberry Pi ARM Local Interrupt Source Mailbox 2
133: #define ARM_LOCAL_INTSRC_MAILBOX2      BIT1(6)
134: /// @brief Raspberry Pi ARM Local Interrupt Source Mailbox 3
135: #define ARM_LOCAL_INTSRC_MAILBOX3      BIT1(7)
136: /// @brief Raspberry Pi ARM Local Interrupt Source GPU
137: #define ARM_LOCAL_INTSRC_GPU           BIT1(8)
138: /// @brief Raspberry Pi ARM Local Interrupt Source PMU
139: #define ARM_LOCAL_INTSRC_PMU           BIT1(9)
140:
141: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
142: #define ARM_LOCAL_MAILBOX0_SET0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000080)
143: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
144: #define ARM_LOCAL_MAILBOX1_SET0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000084)
145: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
146: #define ARM_LOCAL_MAILBOX2_SET0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000088)
147: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
148: #define ARM_LOCAL_MAILBOX3_SET0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000008C)
149:
150: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
151: #define ARM_LOCAL_MAILBOX0_SET1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000090)
152: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
153: #define ARM_LOCAL_MAILBOX1_SET1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000094)
154: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
155: #define ARM_LOCAL_MAILBOX2_SET1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x00000098)
156: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
157: #define ARM_LOCAL_MAILBOX3_SET1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x0000009C)
158:
159: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
160: #define ARM_LOCAL_MAILBOX0_SET2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A0)
161: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
162: #define ARM_LOCAL_MAILBOX1_SET2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A4)
163: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
164: #define ARM_LOCAL_MAILBOX2_SET2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000A8)
165: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
166: #define ARM_LOCAL_MAILBOX3_SET2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000AC)
167:
168: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
169: #define ARM_LOCAL_MAILBOX0_SET3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B0)
170: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
171: #define ARM_LOCAL_MAILBOX1_SET3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B4)
172: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
173: #define ARM_LOCAL_MAILBOX2_SET3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000B8)
174: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Set Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
175: #define ARM_LOCAL_MAILBOX3_SET3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000BC)
176:
177: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
178: #define ARM_LOCAL_MAILBOX0_CLR0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C0)
179: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
180: #define ARM_LOCAL_MAILBOX1_CLR0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C4)
181: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
182: #define ARM_LOCAL_MAILBOX2_CLR0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000C8)
183: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 0 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
184: #define ARM_LOCAL_MAILBOX3_CLR0        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000CC)
185:
186: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
187: #define ARM_LOCAL_MAILBOX0_CLR1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D0)
188: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
189: #define ARM_LOCAL_MAILBOX1_CLR1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D4)
190: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
191: #define ARM_LOCAL_MAILBOX2_CLR1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000D8)
192: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 1 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
193: #define ARM_LOCAL_MAILBOX3_CLR1        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000DC)
194:
195: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
196: #define ARM_LOCAL_MAILBOX0_CLR2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E0)
197: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
198: #define ARM_LOCAL_MAILBOX1_CLR2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E4)
199: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
200: #define ARM_LOCAL_MAILBOX2_CLR2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000E8)
201: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 2 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
202: #define ARM_LOCAL_MAILBOX3_CLR2        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000EC)
203:
204: /// @brief Raspberry Pi ARM Local Core Mailbox 0 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
205: #define ARM_LOCAL_MAILBOX0_CLR3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F0)
206: /// @brief Raspberry Pi ARM Local Core Mailbox 1 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
207: #define ARM_LOCAL_MAILBOX1_CLR3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F4)
208: /// @brief Raspberry Pi ARM Local Core Mailbox 2 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
209: #define ARM_LOCAL_MAILBOX2_CLR3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000F8)
210: /// @brief Raspberry Pi ARM Local Core Mailbox 3 Core 3 Clear Register base address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
211: #define ARM_LOCAL_MAILBOX3_CLR3        reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000000FC)
212:
213: /// @brief Raspberry Pi ARM Local Register region end address. See @ref RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS
214: #define ARM_LOCAL_END                  reinterpret_cast<regaddr>(ARM_LOCAL_BASE + 0x000003FFFF)
```

- Line 84-85: We create definition for the Core Timer Interrupt Control register for core 0
- Line 86-87: We create definition for the Core Timer Interrupt Control register for core 1
- Line 88-89: We create definition for the Core Timer Interrupt Control register for core 2
- Line 90-91: We create definition for the Core Timer Interrupt Control register for core 3

The rest of the registers will be explained as we use them.

### Update application code {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_UPDATE_APPLICATION_CODE}

Let's start generating some interrupts. For now, we'll do this all in the `main()` code, we will be moving and generalizing this later.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/ARMRegisters.h"
3: #include "baremetal/Assert.h"
4: #include "baremetal/BCMRegisters.h"
5: #include "baremetal/Console.h"
6: #include "baremetal/InterruptHandler.h"
7: #include "baremetal/Interrupts.h"
8: #include "baremetal/Logger.h"
9: #include "baremetal/MemoryAccess.h"
10: #include "baremetal/System.h"
11: #include "baremetal/Timer.h"
12:
13: LOG_MODULE("main");
14:
15: using namespace baremetal;
16:
17: #define TICKS_PER_SECOND 2 // Timer ticks per second
18:
19: static uint32 clockTicksPerSystemTick;
20:
21: void IntHandler(void* param)
22: {
23:     uint64 counterCompareValue;
24:     GetTimerCompareValue(counterCompareValue);
25:     SetTimerCompareValue(counterCompareValue + clockTicksPerSystemTick);
26:
27:     LOG_INFO("Ping");
28: }
29:
30: void EnableIRQ()
31: {
32:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0, GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(1));
33:     EnableIRQ(IRQ_ID::IRQ_LOCAL_CNTPNS);
34: }
35:
36: void DisableIRQ()
37: {
38:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0, GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(1));
39:     DisableIRQ(IRQ_ID::IRQ_LOCAL_CNTPNS);
40: }
41:
42: int main()
43: {
44:     auto& console = GetConsole();
45:
46:     auto exceptionLevel = CurrentEL();
47:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
48:
49:     uint64 counterFreq{};
50:     GetTimerFrequency(counterFreq);
51:     assert(counterFreq % TICKS_PER_SECOND == 0);
52:     clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
53:     LOG_INFO("Clock ticks per second: %d, clock ticks per interrupt: %d", counterFreq, clockTicksPerSystemTick);
54:
55:     GetInterruptSystem().RegisterIRQHandler(IntHandler, nullptr);
56:
57:     EnableIRQ();
58:
59:     uint64 counter;
60:     GetTimerCounter(counter);
61:     SetTimerCompareValue(counter + clockTicksPerSystemTick);
62:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
63:
64:     LOG_INFO("Wait 5 seconds");
65:     Timer::WaitMilliSeconds(5000);
66:
67:     DisableIRQ();
68:
69:     GetInterruptSystem().UnregisterIRQHandler();
70:
71:     console.Write("Press r to reboot, h to halt\n");
72:     char ch{};
73:     while ((ch != 'r') && (ch != 'h'))
74:     {
75:         ch = console.ReadChar();
76:         console.WriteChar(ch);
77:     }
78:
79:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
80: }
```

- Line 17: We create a definition to set the timer to trigger 2 times a second
- Line 19: We define a variable to hold the number of clock ticks for every timer tick
- Line 21-28: We define a IRQ handler function `IntHandler()`, which reprograms the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and print some text
- Line 30-34: We define a function `EnableIRQ()`, which enables the IRQ for the ARM non secure local timer for core 0
- Line 36-40: We define a function `DisableIRQ()`, which disables the IRQ for the ARM non secure local timer for core 0
- Line 49-53: We calculate from the clock tick frequency how many clock ticks are in a timer tick, and print the calculated number
- Line 55: We register the handler function
- Line 57: We enable the timer IRQ
- Line 59-62: We set the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and enable the interrupt for the timer
- Line 69: We disable the timer IRQ
- Line 71: We unregister the handler function

### Configuring, building and debugging {#TUTORIAL_19_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

The application will start the timer, and after 5 seconds stop it again. As we set the timer to tick twice a second, we expect to see 10 ticks happening.

On Raspberry Pi 3:

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:92)
Info   Starting up (System:211)
Info   Current EL: 1 (main:47)
Info   Clock ticks per second: 19200000, clock ticks per interrupt: 9600000 (main:53)
Info   Wait 5 seconds (main:64)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Press r to reboot, h to halt
```

On Raspberry Pi 4:

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:92)
Info   Starting up (System:211)
Info   Current EL: 1 (main:47)
Info   Clock ticks per second: 54000000, clock ticks per interrupt: 27000000 (main:53)
Info   Wait 5 seconds (main:64)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Info   Ping (main:27)
Press r to reboot, h to halt
```

## Interrupt System - Step 2 {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2}

We'll update the class `InterruptSystem` to enable, disable, and handle interrupts in a more generic way.
We'll also add support for fast interrupts (FIQ).

### InterruptHandler.h {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTHANDLERH}

Update the file `code/libraries/baremetal/include/baremetal/InterruptHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptHandler.h
...
42: #include "stdlib/Macros.h"
43: #include "stdlib/Types.h"
44: #include "baremetal/Interrupts.h"
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
81:     void* param;
82:     /// @brief ID of FIQ
83:     uint32 fiqID;
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
95:     bool m_isInitialized;
96:     /// @brief Memory access interface
97:     IMemoryAccess& m_memoryAccess;
98:     /// @brief Pointer to registered IRQ handler for each IRQ
99:     IRQHandler* m_irqHandlers[IRQ_LINES];
100:     /// @brief Parameter to pass to registered IRQ handler
101:     void* m_irqHandlersParam[IRQ_LINES];
102:
103:     /// <summary>
104:     /// Construct the singleton InterruptSystem instance if needed, and return a reference to the instance. This is a friend function of class InterruptSystem
105:     /// </summary>
106:     /// <returns>Reference to the singleton InterruptSystem instance</returns>
107:     friend InterruptSystem& GetInterruptSystem();
108:
109: private:
110:     /// @brief Create a interrupt system. Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
111:     InterruptSystem();
112:
113: public:
114:     InterruptSystem(IMemoryAccess& memoryAccess);
115:     ~InterruptSystem();
116:
117:     void Initialize();
118:     void Shutdown();
119:
120:     void DisableInterrupts();
121:     void EnableInterrupts();
122:
123:     void RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param);
124:     void UnregisterIRQHandler(IRQ_ID irqID);
125:
126:     void RegisterFIQHandler(FIQ_ID fiqID, FIQHandler* handler, void* param);
127:     void UnregisterFIQHandler(FIQ_ID fiqID);
128:
129:     void InterruptHandler();
130:
131: private:
132:     bool CallIRQHandler(IRQ_ID irqID);
133: };
134:
135: InterruptSystem& GetInterruptSystem();
136:
137: } // namespace baremetal
138:
139: /// @brief FIQ administration, see Exception.S
140: extern baremetal::FIQData s_fiqData;
```

- Line 44: We need to include the header for interrupt definitions
- Line 61-64: We add a declaration for the `FIQHandler()` function type, much like the `IRQHandler()` function type
- Line 71-84: We add a declaration for the `FIQData` struct, which mirrors the data defined in `ExceptionStub.S'
- Line 98-99: We change the member variable `m_irqHandler` to an array of interrupt handlers `m_irqHandlers`, one for every possible interrupt
- Line 100-101: We change the member variable `m_irqHandlerParams` to an array of interrupt handlers parameters `m_irqHandlersParam`
- Line 114: We add a declaration for a specialized constructor taken a `MemoryAccess` reference, meant for testing
- Line 118: We add the method `Shutdown()`, which disables all interrupts, and disables interrupts altogether.
It also clears the initialization flag
- Line 120: We add the method `DisableInterrupts()`, which disables all interrupts to the interrupt controller, e.g. it clears interrupt enables on all IRQ lines, as well as the FIQ
- Line 121: We add the method `EmableInterrupts()`, which enables interrupts to the interrupt controller
- Line 123: We change the method `RegisterIRQHandler()` to also take an IRQ id
- Line 124: We change the method `UnregisterIRQHandler()` to take an IRQ id
- Line 126: We add the method `RegisterFIQHandler()` which registers a FIQ handler for the specified FIQ id. Only one FIQ handler can be active at any time
- Line 127: We add the method `UnregisterFIQHandler()` which unregisters the previously registered FIQ handler. As there can only be one FIQ handler registered at any time, we don't need to specify the FIQ id
- Line 132: We add the private method `CallIRQHandler()` which calls the registered IRQ handler for the IRQ with the specified id
- Line 139-140: We declare the FIQData struct instance s_fiqData, which is defined in `ExceptionStub.S'

### InterruptHandler.cpp {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTHANDLERCPP}

Let's update the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
...
40: #include "baremetal/InterruptHandler.h"
41:
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/ARMRegisters.h"
44: #include "baremetal/Assert.h"
45: #include "baremetal/BCMRegisters.h"
46: #include "baremetal/Interrupts.h"
47: #include "baremetal/Logger.h"
48: #include "baremetal/MemoryAccess.h"
49: #include "stdlib/Util.h"
...
69: /// <summary>
70: /// Create a interrupt system
71: ///
72: /// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
73: /// </summary>
74: InterruptSystem::InterruptSystem()
75:     : m_isInitialized{}
76:     , m_memoryAccess{GetMemoryAccess()}
77:     , m_irqHandlers{}
78:     , m_irqHandlersParam{}
79: {
80: }
81:
82: /// <summary>
83: /// Constructs a specialized InterruptSystem instance which injects a custom IMemoryAccess instance. This is intended for testing.
84: /// </summary>
85: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
86: InterruptSystem::InterruptSystem(IMemoryAccess& memoryAccess)
87:     : m_isInitialized{}
88:     , m_memoryAccess{memoryAccess}
89:     , m_irqHandlers{}
90:     , m_irqHandlersParam{}
91: {
92: }
93:
94: /// <summary>
95: /// Destructor/// </summary>
96: InterruptSystem::~InterruptSystem()
97: {
98:     Shutdown();
99: }
100:
101: /// <summary>
102: /// Initialize interrupt system
103: /// </summary>
104: void InterruptSystem::Initialize()
105: {
106:     if (m_isInitialized)
107:         return;
108:
109:     memset(m_irqHandlers, 0, IRQ_LINES * sizeof(IRQHandler*));
110:     memset(m_irqHandlersParam, 0, IRQ_LINES * sizeof(void*));
111:
112:     DisableInterrupts();
113:
114:     EnableIRQs();
115:
116: #if BAREMETAL_RPI_TARGET == 3
117: #else
118:     // direct all interrupts to core 0 with default priority
119:     for (unsigned n = 0; n < IRQ_LINES / 4; n++)
120:     {
121:         m_memoryAccess.Write32(RPI_GICD_IPRIORITYR0 + 4 * n, RPI_GICD_IPRIORITYR_DEFAULT | RPI_GICD_IPRIORITYR_DEFAULT << 8 | RPI_GICD_IPRIORITYR_DEFAULT << 16 | RPI_GICD_IPRIORITYR_DEFAULT << 24);
122:
123:         m_memoryAccess.Write32(RPI_GICD_ITARGETSR0 + 4 * n, RPI_GICD_ITARGETSR_CORE0 | RPI_GICD_ITARGETSR_CORE0 << 8 | RPI_GICD_ITARGETSR_CORE0 << 16 | RPI_GICD_ITARGETSR_CORE0 << 24);
124:     }
125:
126:     // set all interrupts to level triggered
127:     for (unsigned n = 0; n < IRQ_LINES / 16; n++)
128:     {
129:         m_memoryAccess.Write32(RPI_GICD_ICFGR0 + 4 * n, 0);
130:     }
131:
132:     // initialize core 0 CPU interface:
133:
134:     m_memoryAccess.Write32(RPI_GICC_PMR, RPI_GICC_PMR_PRIORITY);
135: #endif
136:
137:     EnableInterrupts();
138:
139:     m_isInitialized = true;
140: }
141:
142: /// <summary>
143: /// Shutdown interrupt system, disable all
144: /// </summary>
145: void InterruptSystem::Shutdown()
146: {
147:     DisableIRQs();
148:
149:     DisableInterrupts();
150:     m_isInitialized = false;
151: }
152:
153: /// <summary>
154: /// Disable all IRQ interrupts
155: /// </summary>
156: void InterruptSystem::DisableInterrupts()
157: {
158: #if BAREMETAL_RPI_TARGET == 3
159:     m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
160:
161:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
162:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
163:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
164:     m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
165: #else
166:     // initialize distributor
167:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
168:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_DISABLE);
169:     // disable, acknowledge and deactivate all interrupts
170:     for (unsigned n = 0; n < IRQ_LINES / 32; n++)
171:     {
172:         m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
173:         m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
174:         m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
175:     }
176: #endif
177: }
178:
179: /// <summary>
180: /// Enable IRQ interrupts
181: /// </summary>
182: void InterruptSystem::EnableInterrupts()
183: {
184: #if BAREMETAL_RPI_TARGET == 3
185: #else
186:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
187:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
188: #endif
189: }
190:
191: /// <summary>
192: /// Enable and register an IRQ handler
193: ///
194: /// Enable the IRQ with specified index, and register its handler.
195: /// </summary>
196: /// <param name="irqID">IRQ ID</param>
197: /// <param name="handler">Handler to register for this IRQ</param>
198: /// <param name="param">Parameter to pass to IRQ handler</param>
199: void InterruptSystem::RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param)
200: {
201:     uint32 irq = static_cast<int>(irqID);
202:     assert(irq < IRQ_LINES);
203:     assert(m_irqHandlers[irq] == nullptr);
204:
205:     EnableIRQ(irqID);
206:
207:     m_irqHandlers[irq] = handler;
208:     m_irqHandlersParam[irq] = param;
209: }
210:
211: /// <summary>
212: /// Disable and unregister an IRQ handler
213: ///
214: /// Disable the IRQ with specified index, and unregister its handler.
215: /// </summary>
216: /// <param name="irqID">IRQ ID</param>
217: void InterruptSystem::UnregisterIRQHandler(IRQ_ID irqID)
218: {
219:     uint32 irq = static_cast<int>(irqID);
220:     assert(irq < IRQ_LINES);
221:     assert(m_irqHandlers[irq] != nullptr);
222:
223:     m_irqHandlers[irq] = nullptr;
224:     m_irqHandlersParam[irq] = nullptr;
225:
226:     DisableIRQ(irqID);
227: }
228:
229: /// <summary>
230: /// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
231: /// </summary>
232: /// <param name="fiqID">FIQ interrupt number</param>
233: /// <param name="handler">FIQ interrupt handler</param>
234: /// <param name="param">FIQ interrupt data</param>
235: // cppcheck-suppress unusedFunction
236: void InterruptSystem::RegisterFIQHandler(FIQ_ID fiqID, FIQHandler* handler, void* param)
237: {
238:     uint32 fiq = static_cast<int>(fiqID);
239:     assert(fiq <= IRQ_LINES);
240:     assert(handler != nullptr);
241:     assert(s_fiqData.handler == nullptr);
242:
243:     s_fiqData.handler = handler;
244:     s_fiqData.param = param;
245:     s_fiqData.fiqID = fiq;
246:
247:     EnableFIQ(fiqID);
248: }
249:
250: /// <summary>
251: /// Disable and unregister a FIQ interrupt handler
252: /// </summary>
253: /// <param name="fiqID">FIQ interrupt number, to check against set FIQ</param>
254: void InterruptSystem::UnregisterFIQHandler(FIQ_ID fiqID)
255: {
256:     uint32 fiq = static_cast<int>(fiqID);
257:     assert(s_fiqData.handler != nullptr);
258:     assert(s_fiqData.fiqID == fiq);
259:     DisableFIQ(fiqID);
260:
261:     s_fiqData.handler = nullptr;
262:     s_fiqData.param = nullptr;
263: }
264:
265: /// <summary>
266: /// Handles an interrupt.
267: ///
268: /// The interrupt handler is called from assembly code (ExceptionStub.S)
269: /// </summary>
270: void InterruptSystem::InterruptHandler()
271: {
272: #if BAREMETAL_RPI_TARGET == 3
273:     uint32 localpendingIRQs = m_memoryAccess.Read32(ARM_LOCAL_IRQ_PENDING0);
274:     if (localpendingIRQs & ARM_LOCAL_INTSRC_TIMER1) // the only implemented local IRQ so far
275:     {
276:         CallIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
277:
278:         return;
279:     }
280:
281:     uint32 pendingIRQs[ARM_IRQS_NUM_REGS];
282:     pendingIRQs[0] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_1);
283:     pendingIRQs[1] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_2);
284:     pendingIRQs[2] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_BASIC_PENDING) & 0xFF; // Only 8 basic interrupts
285:
286:     for (unsigned reg = 0; reg < ARM_IRQS_NUM_REGS; reg++)
287:     {
288:         uint32 pendingIRQ = pendingIRQs[reg];
289:         if (pendingIRQ != 0)
290:         {
291:             unsigned irqID = reg * ARM_IRQS_PER_REG;
292:
293:             do
294:             {
295:                 if ((pendingIRQ & 1) && CallIRQHandler(static_cast<IRQ_ID>(irqID)))
296:                 {
297:                     return;
298:                 }
299:
300:                 pendingIRQ >>= 1;
301:                 irqID++;
302:             } while (pendingIRQ != 0);
303:         }
304:     }
305:
306: #else
307:
308:     uint32 iarValue = m_memoryAccess.Read32(RPI_GICC_IAR); // Read Interrupt Acknowledge Register
309:
310:     uint32 irq = iarValue & RPI_GICC_IAR_INTERRUPT_ID_MASK; // Select the currently active interrupt
311:     if (irq < IRQ_LINES)
312:     {
313:         if (irq >= GIC_PPI(0))
314:         {
315:             // Peripheral interrupts (PPI and SPI)
316:             CallIRQHandler(static_cast<IRQ_ID>(irq));
317:         }
318:         else
319:         {
320:             // Handle SGI interrupt
321:         }
322:         m_memoryAccess.Write32(RPI_GICC_EOIR, iarValue); // Flag end of interrupt
323:     }
324: #ifndef NDEBUG
325:     else
326:     {
327:         // spurious interrupts
328:         assert(irq >= 1020);
329:         LOG_INFO("Received spurious interrupt %d", iarValue);
330:     }
331: #endif
332:
333: #endif
334: }
335:
336: /// <summary>
337: /// Call the IRQ handler for the specified IRQ ID
338: /// </summary>
339: /// <param name="irqID">ID of the IRQ</param>
340: /// <returns>True if a IRQ handler was found, false if not</returns>
341: bool InterruptSystem::CallIRQHandler(IRQ_ID irqID)
342: {
343:     uint32 irq = static_cast<int>(irqID);
344:     assert(irq < IRQ_LINES);
345:     IRQHandler* handler = m_irqHandlers[irq];
346:
347:     if (handler != nullptr)
348:     {
349:         (*handler)(m_irqHandlersParam[irq]);
350:
351:         return true;
352:     }
353: #ifndef NDEBUG
354:     LOG_INFO("Unhandled interrupt %d", irq);
355: #endif
356:
357:     DisableIRQ(irqID);
358:
359:     return false;
360: }
361:
362: /// <summary>
363: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
364: ///
365: /// This is a friend function of class InterruptSystem
366: /// </summary>
367: /// <returns>Reference to the singleton interrupt system instance</returns>
368: InterruptSystem& baremetal::GetInterruptSystem()
369: {
370:     static InterruptSystem singleton;
371:     singleton.Initialize();
372:     return singleton;
373: }
```

- Line 42: We need to include the header for ARM instructions as we need to enable / disable interrupts
- Line 43: We need to include the header for ARM specific registers
- Line 49: We need to include the header for utility functions
- Line 69-80: We update the default constructor to initialize the new member variables
- Line 82-92: We implement the specialized constructor. The only difference is the `IMemoryAccess` instance
- Line 94-99: We update the destructor to call `Shutdown()`
- Line 101-139: We update the `Initialize()` method set up the administration for IRQs, disable all interrupt lines, but enable the IRQs system wide
  - Line 106-107: We check whether the system is already initialized, and return if so
  - Line 109-110: We zero out the vectors with interrupt handlers and their parameters
  - Line 112: We call `DisableInterrupts()` to disable all interrupts
  - Line 114: We call `EnableIRQs()` to switch on overall IRQs
  - Line 116: For Raspberry Pi 3, there is nothing lef to do
  - Line 118-134: For Raspberry Pi 4 / 5 we still need some initialization, like in the previous version.
  - LIne 137: We call `EnableInterrupts()` to ensable all interrupts
Notice that some initialization is moved to `DisableInterrupts()`, and we use `EnableInterrupts()` to re-enable the GIC distributor and controller
- Line 142-151: We implement the `Shutdown()` method
  - Line 147: We disable all interrupts at system level
  - Line 149: We disable all interrupts at interrupt controller level
  - Line 150: We set the initialization flag to false
- Line 153-178: We implement the `DisableInterrupts()` method
  - Line 159-164: For Raspberry Pi 3, we disable the FIQ, and reset all interrupt enable bits for the IRQ1, IRQ2, Basic IRQ, and ARM local interrupts
  - Line 166-175: We disable the GIC distributor and controller, and reset all interrupt enable bits for interrupts
- Line 179-189: We implement the `EnableInterrupts()` method
  - Line 186-187: We enable the GIC distributor and controller
- Line 191-209: We update the `RegisterIRQHandler()` method to register the IRQ handler, and enable its IRQ line
- Line 211-227: We update the `UnregisterIRQHandler()` method to remove the registered IRQ handler, and disable its IRQ line
- Line 229-248: We implement the `RegisterFIQHandler()` method to register the FIQ handler, and enable it
- Line 250-263: We implement the `UnregisterIRQHandler()` method to remove the registered FIQ handler, and disable it
- Line 265-334: We update the `InterruptHandler()` method
  - Line 273-304: For Raspberry Pi 3
    - Line 273-279: We check whether the ARM local timer fired, and call its interrupt handler if so
    - Line 281-284: We read all interrupt pending bits for the IRQ1, IRQ2 and Basic IRQ groups
    - Line 286-304: We scan through all pending bits, if one is set, we call its interrupt handler and return.
So at most one interrupt handler is called
  - Line 308-331: For Raspberry Pi 4 / 5
    - Line 308: We read the IAR register to find which interrupt we need to handle
    - Line 311-324: If the interrupt id is within limits, we call its interrupt handler, and acknowledge the interrupt
    - Line 326-330: If not, and debug mode is on, we check that this is a spurious interrupt, and print a message with the interrupt id
- Line 336-360: We implement the `CallIRQHandler()` method
  - Line 343-344: We perform a sanity check that the interrupt is within limits
  - Line 345: We retrieve the interrupt handler pointer
  - Line 347-352: If the interrupt handler is installed, we call it and return true
  - Line 354: If no handler is installed and debug mode is on, we print a message
  - Line 357-359: As at this point the interrupt is not handled, we disable the interrupt for the future and return false

### Interrupt.h {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTH}

We'll add definitions for all IRQ and FIQ interrupts.

Update the file `code/libraries/baremetal/include/baremetal/Interrupts.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Interrupts.h
44: namespace baremetal {
45:
46: #if BAREMETAL_RPI_TARGET == 3
47:
48: // IRQs
49: /// @brief Number of IRQ lines per register for regular ARM_IRQ1 and ARM_IRQ2
50: #define ARM_IRQS_PER_REG         32
51: /// @brief Number of IRQ lines for ARM_IRQ_BASIC
52: #define ARM_IRQS_BASIC_REG       8
53: /// @brief Number of IRQ lines for ARM_IRQ_LOCAL
54: #define ARM_IRQS_LOCAL_REG       12
55:
56: /// @brief Number of normal IRQ registers (IRQ1, IRQ2, Basic IRQ)
57: #define ARM_IRQS_NUM_REGS        3
58:
59: /// @brief Total number of IRQ lines Raspberry Pi 3
60: #define IRQ_LINES                (ARM_IRQS_PER_REG * 2 + ARM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)
61:
62: /// @brief Offset for interrupts in IRQ1 group
63: #define ARM_IRQ1_BASE            0
64: /// @brief Offset for interrupts in IRQ2 group
65: #define ARM_IRQ2_BASE            (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
66: /// @brief Offset for interrupts in Basic IRQ group
67: #define ARM_IRQ_BASIC_BASE       (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
68: /// @brief Offset for interrupts in ARM local group
69: #define ARM_IRQ_LOCAL_BASE       (ARM_IRQ_BASIC_BASE + ARM_IRQS_BASIC_REG)
70: /// @brief Calculate IRQ number for IRQ in IRQ1 group
71: #define ARM_IRQ1(n)              (ARM_IRQ1_BASE + (n))
72: /// @brief Calculate IRQ number for IRQ in IRQ2 group
73: #define ARM_IRQ2(n)              (ARM_IRQ2_BASE + (n))
74: /// @brief Calculate IRQ number for IRQ in Basic IRQ group
75: #define ARM_IRQ_BASIC(n)         (ARM_IRQ_BASIC_BASE + (n))
76: /// @brief Calculate IRQ number for IRQ in ARM local group
77: #define ARM_IRQ_LOCAL(n)         (ARM_IRQ_LOCAL_BASE + (n))
78:
79: /// @brief Check whether an IRQ is pending (only for IRQ1, IRQ2, Basic IRQ group)
80: #define ARM_IC_IRQ_PENDING(irq)  ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_IRQ_PENDING_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_IRQ_PENDING_2 : RPI_INTRCTRL_IRQ_BASIC_PENDING))
81: /// @brief Enable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
82: #define ARM_IC_IRQS_ENABLE(irq)  ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_ENABLE_IRQS_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_ENABLE_IRQS_2 : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
83: /// @brief Disable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
84: #define ARM_IC_IRQS_DISABLE(irq) ((irq) < ARM_IRQ2_BASE ? RPI_INTRCTRL_DISABLE_IRQS_1 : ((irq) < ARM_IRQ_BASIC_BASE ? RPI_INTRCTRL_DISABLE_IRQS_2 : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
85: /// @brief Calculate bitmask for an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
86: #define ARM_IRQ_MASK(irq)        BIT1((irq) & (ARM_IRQS_PER_REG - 1))
87:
88: /// <summary>
89: /// @brief IRQ interrupt numbers
90: /// </summary>
91: enum class IRQ_ID
92: {
93:     /// @brief BCM timer 0
94:     IRQ_TIMER0 = ARM_IRQ1(0x00), // bcm2835-system-timer
95:     /// @brief BCM timer 1
96:     IRQ_TIMER1 = ARM_IRQ1(0x01),
97:     /// @brief BCM timer 2
98:     IRQ_TIMER2 = ARM_IRQ1(0x02),
99:     /// @brief BCM timer 3
100:     IRQ_TIMER3 = ARM_IRQ1(0x03),
101:     /// @brief H.264 codec 0
102:     IRQ_CODEC0 = ARM_IRQ1(0x04),
103:     /// @brief H.264 codec 1
104:     IRQ_CODEC1 = ARM_IRQ1(0x05),
105:     /// @brief H.264 codec 2
106:     IRQ_CODEC2 = ARM_IRQ1(0x06),
107:     /// @brief JPEG
108:     IRQ_JPEG = ARM_IRQ1(0x07),
109:     /// @brief ISP
110:     IRQ_ISP = ARM_IRQ1(0x08),
111:     /// @brief USB
112:     IRQ_USB = ARM_IRQ1(0x09), // bcm2708-usb
113:     /// @brief VideoCore 3D
114:     IRQ_3D = ARM_IRQ1(0x0A), // vc4-v3d
115:     /// @brief TXP / Transpose
116:     IRQ_TRANSPOSER = ARM_IRQ1(0x0B), // bcm2835-txp
117:     /// @brief Multicore sync 0
118:     IRQ_MULTICORESYNC0 = ARM_IRQ1(0x0C),
119:     /// @brief Multicore sync 1
120:     IRQ_MULTICORESYNC1 = ARM_IRQ1(0x0D),
121:     /// @brief Multicore sync 2
122:     IRQ_MULTICORESYNC2 = ARM_IRQ1(0x0E),
123:     /// @brief Multicore sync 3
124:     IRQ_MULTICORESYNC3 = ARM_IRQ1(0x0F),
125:     /// @brief DMA channel 0 interrupt
126:     IRQ_DMA0 = ARM_IRQ1(0x10),
127:     /// @brief DMA channel 1 interrupt
128:     IRQ_DMA1 = ARM_IRQ1(0x11),
129:     /// @brief DMA channel 2 interrupt, I2S PCM TX
130:     IRQ_DMA2 = ARM_IRQ1(0x12),
131:     /// @brief DMA channel 3 interrupt, I2S PCM RX
132:     IRQ_DMA3 = ARM_IRQ1(0x13),
133:     /// @brief DMA channel 4 interrupt, SMI
134:     IRQ_DMA4 = ARM_IRQ1(0x14),
135:     /// @brief DMA channel 5 interrupt, PWM
136:     IRQ_DMA5 = ARM_IRQ1(0x15),
137:     /// @brief DMA channel 6 interrupt, SPI TX
138:     IRQ_DMA6 = ARM_IRQ1(0x16),
139:     /// @brief DMA channel 7 interrupt, SPI RX
140:     IRQ_DMA7 = ARM_IRQ1(0x17),
141:     /// @brief DMA channel 8 interrupt
142:     IRQ_DMA8 = ARM_IRQ1(0x18),
143:     /// @brief DMA channel 9 interrupt
144:     IRQ_DMA9 = ARM_IRQ1(0x19),
145:     /// @brief DMA channel 10 interrupt
146:     IRQ_DMA10 = ARM_IRQ1(0x1A),
147:     /// @brief DMA channel 11-14 interrupt, EMMC
148:     IRQ_DMA11 = ARM_IRQ1(0x1B),
149:     /// @brief DMA channel 11-14 interrupt, UART TX
150:     IRQ_DMA12 = ARM_IRQ1(0x1B),
151:     /// @brief DMA channel 11-14 interrupt, undocumented
152:     IRQ_DMA13 = ARM_IRQ1(0x1B),
153:     /// @brief DMA channel 11-14 interrupt, UART RX
154:     IRQ_DMA14 = ARM_IRQ1(0x1B),
155:     /// @brief DMA channel shared interrupt
156:     IRQ_DMA_SHARED = ARM_IRQ1(0x1C),
157:     /// @brief AUX interrupt (UART1, SPI0/1)
158:     IRQ_AUX = ARM_IRQ1(0x1D),
159:     /// @brief ARM interrupt
160:     IRQ_ARM = ARM_IRQ1(0x1E),
161:     /// @brief VPU interrupt
162:     IRQ_VPUDMA = ARM_IRQ1(0x1F),
163:
164:     /// @brief USB Host Port interrupt
165:     IRQ_HOSTPORT = ARM_IRQ2(0x00),
166:     /// @brief JPEHVS interruptG
167:     IRQ_VIDEOSCALER = ARM_IRQ2(0x01),
168:     /// @brief CCP2TX interrupt
169:     IRQ_CCP2TX = ARM_IRQ2(0x02),
170:     /// @brief SDC interrupt
171:     IRQ_SDC = ARM_IRQ2(0x03),
172:     /// @brief DSI 0 (display) interrupt
173:     IRQ_DSI0 = ARM_IRQ2(0x04),
174:     /// @brief AVE interrupt
175:     IRQ_AVE = ARM_IRQ2(0x05),
176:     /// @brief CSI 0 (camera) interrupt
177:     IRQ_CAM0 = ARM_IRQ2(0x06),
178:     /// @brief CSI 1 (camera) interrupt
179:     IRQ_CAM1 = ARM_IRQ2(0x07),
180:     /// @brief HDMI 0 interrupt
181:     IRQ_HDMI0 = ARM_IRQ2(0x08),
182:     /// @brief HDMI 1 interrupt
183:     IRQ_HDMI1 = ARM_IRQ2(0x09),
184:     /// @brief GPU pixel valve 2 interrupt
185:     IRQ_PIXELVALVE2 = ARM_IRQ2(0x0A),
186:     /// @brief I2C / SPI slave interrupt
187:     IRQ_I2CSPISLV = ARM_IRQ2(0x0B),
188:     /// @brief DSI 1 (display) interrupt
189:     IRQ_DSI1 = ARM_IRQ2(0x0C),
190:     /// @brief GPU pixel valve 0 interrupt
191:     IRQ_PIXELVALVE0 = ARM_IRQ2(0x0D),
192:     /// @brief GPU pixel valve 1 interrupt
193:     IRQ_PIXELVALVE1 = ARM_IRQ2(0x0E),
194:     /// @brief CPR interrupt
195:     IRQ_CPR = ARM_IRQ2(0x0F),
196:     /// @brief SMI (firmware) interrupt
197:     IRQ_SMI = ARM_IRQ2(0x10),
198:     /// @brief GPIO 0 interrupt
199:     IRQ_GPIO0 = ARM_IRQ2(0x11),
200:     /// @brief GPIO 1 interrupt
201:     IRQ_GPIO1 = ARM_IRQ2(0x12),
202:     /// @brief GPIO 2 interrupt
203:     IRQ_GPIO2 = ARM_IRQ2(0x13),
204:     /// @brief GPIO 3 interrupt
205:     IRQ_GPIO3 = ARM_IRQ2(0x14),
206:     /// @brief I2C interrupt
207:     IRQ_I2C = ARM_IRQ2(0x15),
208:     /// @brief SPI interrupt
209:     IRQ_SPI = ARM_IRQ2(0x16),
210:     /// @brief I2S interrupt
211:     IRQ_I2SPCM = ARM_IRQ2(0x17),
212:     /// @brief SD host interrupt
213:     IRQ_SDHOST = ARM_IRQ2(0x18),
214:     /// @brief PL011 UART interrupt (UART0)
215:     IRQ_UART = ARM_IRQ2(0x19),
216:     /// @brief SLIMBUS interrupt
217:     IRQ_SLIMBUS = ARM_IRQ2(0x1A),
218:     /// @brief GPU? vector interrupt
219:     IRQ_VEC = ARM_IRQ2(0x1B),
220:     /// @brief CPG interrupt
221:     IRQ_CPG = ARM_IRQ2(0x1C),
222:     /// @brief RNG (random number generator) interrupt
223:     IRQ_RNG = ARM_IRQ2(0x1D),
224:     /// @brief EMMC interrupt
225:     IRQ_ARASANSDIO = ARM_IRQ2(0x1E),
226:     /// @brief AVSPMON interrupt
227:     IRQ_AVSPMON = ARM_IRQ2(0x1F),
228:
229:     /// @brief ARM timer interrupt
230:     IRQ_ARM_TIMER = ARM_IRQ_BASIC(0x00),
231:     /// @brief ARM mailbox interrupt
232:     IRQ_ARM_MAILBOX = ARM_IRQ_BASIC(0x01),
233:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
234:     IRQ_ARM_DOORBELL_0 = ARM_IRQ_BASIC(0x02),
235:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
236:     IRQ_ARM_DOORBELL_1 = ARM_IRQ_BASIC(0x03),
237:     /// @brief VPU halted 0 interrupt
238:     IRQ_VPU0_HALTED = ARM_IRQ_BASIC(0x04),
239:     /// @brief VPU halted 1 interrupt
240:     IRQ_VPU1_HALTED = ARM_IRQ_BASIC(0x05),
241:     /// @brief Illegal type 0 interrupt
242:     IRQ_ILLEGAL_TYPE0 = ARM_IRQ_BASIC(0x06),
243:     /// @brief Illegal type 1 interrupt
244:     IRQ_ILLEGAL_TYPE1 = ARM_IRQ_BASIC(0x07),
245:
246:     /// @brief Non secure Physical ARM timer
247:     IRQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00), // armv7-timer
248:     /// @brief Secure Physical ARM timer
249:     IRQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
250:     /// @brief Hypervisor Physical ARM timer
251:     IRQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
252:     /// @brief Virtual ARM timer
253:     IRQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
254:     /// @brief Mailbox 0
255:     IRQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
256:     /// @brief Mailbox 1
257:     IRQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
258:     /// @brief Mailbox 2
259:     IRQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
260:     /// @brief Mailbox 3
261:     IRQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
262:     /// @brief Cascaded GPU interrupts
263:     IRQ_LOCAL_GPU = ARM_IRQ_LOCAL(0x08),
264:     /// @brief Performance Monitoring Unit
265:     IRQ_LOCAL_PMU = ARM_IRQ_LOCAL(0x09),
266:     /// @brief AXI bus idle, on core 0 only
267:     IRQ_LOCAL_AXI_IDLE = ARM_IRQ_LOCAL(0x0A),
268:     /// @brief Local timer
269:     IRQ_LOCAL_LOCALTIMER = ARM_IRQ_LOCAL(0x0B),
270: };
271:
272: /// <summary>
273: /// @brief FIQ interrupt numbers
274: /// </summary>
275: enum class FIQ_ID
276: {
277:     /// @brief BCM timer 0
278:     FIQ_TIMER0 = ARM_IRQ1(0x00),
279:     /// @brief BCM timer 1
280:     FIQ_TIMER1 = ARM_IRQ1(0x01),
281:     /// @brief BCM timer 2
282:     FIQ_TIMER2 = ARM_IRQ1(0x02),
283:     /// @brief BCM timer 3
284:     FIQ_TIMER3 = ARM_IRQ1(0x03),
285:     /// @brief H.264 codec 0
286:     FIQ_CODEC0 = ARM_IRQ1(0x04),
287:     /// @brief H.264 codec 1
288:     FIQ_CODEC1 = ARM_IRQ1(0x05),
289:     /// @brief H.264 codec 2
290:     FIQ_CODEC2 = ARM_IRQ1(0x06),
291:     /// @brief JPEG
292:     FIQ_JPEG = ARM_IRQ1(0x07),
293:     /// @brief ISP
294:     FIQ_ISP = ARM_IRQ1(0x08),
295:     /// @brief USB
296:     FIQ_USB = ARM_IRQ1(0x09),
297:     /// @brief VideoCore 3D
298:     FIQ_3D = ARM_IRQ1(0x0A),
299:     /// @brief TXP / Transpose
300:     FIQ_TRANSPOSER = ARM_IRQ1(0x0B),
301:     /// @brief Multicore sync 0
302:     FIQ_MULTICORESYNC0 = ARM_IRQ1(0x0C),
303:     /// @brief Multicore sync 1
304:     FIQ_MULTICORESYNC1 = ARM_IRQ1(0x0D),
305:     /// @brief Multicore sync 2
306:     FIQ_MULTICORESYNC2 = ARM_IRQ1(0x0E),
307:     /// @brief Multicore sync 3
308:     FIQ_MULTICORESYNC3 = ARM_IRQ1(0x0F),
309:     /// @brief DMA channel 0 interrupt
310:     FIQ_DMA0 = ARM_IRQ1(0x10),
311:     /// @brief DMA channel 1 interrupt
312:     FIQ_DMA1 = ARM_IRQ1(0x11),
313:     /// @brief DMA channel 2 interrupt, I2S PCM TX
314:     FIQ_DMA2 = ARM_IRQ1(0x12),
315:     /// @brief DMA channel 3 interrupt, I2S PCM RX
316:     FIQ_DMA3 = ARM_IRQ1(0x13),
317:     /// @brief DMA channel 4 interrupt, SMI
318:     FIQ_DMA4 = ARM_IRQ1(0x14),
319:     /// @brief DMA channel 5 interrupt, PWM
320:     FIQ_DMA5 = ARM_IRQ1(0x15),
321:     /// @brief DMA channel 6 interrupt, SPI TX
322:     FIQ_DMA6 = ARM_IRQ1(0x16),
323:     /// @brief DMA channel 7 interrupt, SPI RX
324:     FIQ_DMA7 = ARM_IRQ1(0x17),
325:     /// @brief DMA channel 8 interrupt
326:     FIQ_DMA8 = ARM_IRQ1(0x18),
327:     /// @brief DMA channel 9 interrupt
328:     FIQ_DMA9 = ARM_IRQ1(0x19),
329:     /// @brief DMA channel 10 interrupt
330:     FIQ_DMA10 = ARM_IRQ1(0x1A),
331:     /// @brief DMA channel 11-14 interrupt, EMMC
332:     FIQ_DMA11 = ARM_IRQ1(0x1B),
333:     /// @brief DMA channel 11-14 interrupt, UART TX
334:     FIQ_DMA12 = ARM_IRQ1(0x1B),
335:     /// @brief DMA channel 11-14 interrupt, undocumented
336:     FIQ_DMA13 = ARM_IRQ1(0x1B),
337:     /// @brief DMA channel 11-14 interrupt, UART RX
338:     FIQ_DMA14 = ARM_IRQ1(0x1B),
339:     /// @brief DMA channel shared interrupt
340:     FIQ_DMA_SHARED = ARM_IRQ1(0x1C),
341:     /// @brief AUX interrupt (UART1, SPI0/1)
342:     FIQ_AUX = ARM_IRQ1(0x1D),
343:     /// @brief ARM interrupt
344:     FIQ_ARM = ARM_IRQ1(0x1E),
345:     /// @brief VPU interrupt
346:     FIQ_VPUDMA = ARM_IRQ1(0x1F),
347:
348:     /// @brief USB Host Port interrupt
349:     FIQ_HOSTPORT = ARM_IRQ2(0x00),
350:     /// @brief JPEHVS interruptG
351:     FIQ_VIDEOSCALER = ARM_IRQ2(0x01),
352:     /// @brief CCP2TX interrupt
353:     FIQ_CCP2TX = ARM_IRQ2(0x02),
354:     /// @brief SDC interrupt
355:     FIQ_SDC = ARM_IRQ2(0x03),
356:     /// @brief DSI 0 (display) interrupt
357:     FIQ_DSI0 = ARM_IRQ2(0x04),
358:     /// @brief AVE interrupt
359:     FIQ_AVE = ARM_IRQ2(0x05),
360:     /// @brief CSI 0 (camera) interrupt
361:     FIQ_CAM0 = ARM_IRQ2(0x06),
362:     /// @brief CSI 1 (camera) interrupt
363:     FIQ_CAM1 = ARM_IRQ2(0x07),
364:     /// @brief HDMI 0 interrupt
365:     FIQ_HDMI0 = ARM_IRQ2(0x08),
366:     /// @brief HDMI 1 interrupt
367:     FIQ_HDMI1 = ARM_IRQ2(0x09),
368:     /// @brief GPU pixel valve 2 interrupt
369:     FIQ_PIXELVALVE2 = ARM_IRQ2(0x0A),
370:     /// @brief I2C / SPI slave interrupt
371:     FIQ_I2CSPISLV = ARM_IRQ2(0x0B),
372:     /// @brief DSI 1 (display) interrupt
373:     FIQ_DSI1 = ARM_IRQ2(0x0C),
374:     /// @brief GPU pixel valve 0 interrupt
375:     FIQ_PIXELVALVE0 = ARM_IRQ2(0x0D),
376:     /// @brief GPU pixel valve 1 interrupt
377:     FIQ_PIXELVALVE1 = ARM_IRQ2(0x0E),
378:     /// @brief CPR interrupt
379:     FIQ_CPR = ARM_IRQ2(0x0F),
380:     /// @brief SMI (firmware) interrupt
381:     FIQ_SMI = ARM_IRQ2(0x10),
382:     /// @brief GPIO 0 interrupt
383:     FIQ_GPIO0 = ARM_IRQ2(0x11),
384:     /// @brief GPIO 1 interrupt
385:     FIQ_GPIO1 = ARM_IRQ2(0x12),
386:     /// @brief GPIO 2 interrupt
387:     FIQ_GPIO2 = ARM_IRQ2(0x13),
388:     /// @brief GPIO 3 interrupt
389:     FIQ_GPIO3 = ARM_IRQ2(0x14),
390:     /// @brief I2C interrupt
391:     FIQ_I2C = ARM_IRQ2(0x15),
392:     /// @brief SPI interrupt
393:     FIQ_SPI = ARM_IRQ2(0x16),
394:     /// @brief I2S interrupt
395:     FIQ_I2SPCM = ARM_IRQ2(0x17),
396:     /// @brief SD host interrupt
397:     FIQ_SDHOST = ARM_IRQ2(0x18),
398:     /// @brief PL011 UART interrupt (UART0)
399:     FIQ_UART = ARM_IRQ2(0x19),
400:     /// @brief SLIMBUS interrupt
401:     FIQ_SLIMBUS = ARM_IRQ2(0x1A),
402:     /// @brief GPU? vector interrupt
403:     FIQ_VEC = ARM_IRQ2(0x1B),
404:     /// @brief CPG interrupt
405:     FIQ_CPG = ARM_IRQ2(0x1C),
406:     /// @brief RNG (random number generator) interrupt
407:     FIQ_RNG = ARM_IRQ2(0x1D),
408:     /// @brief EMMC interrupt
409:     FIQ_ARASANSDIO = ARM_IRQ2(0x1E),
410:     /// @brief AVSPMON interrupt
411:     FIQ_AVSPMON = ARM_IRQ2(0x1F),
412:
413:     /// @brief ARM timer interrupt
414:     FIQ_ARM_TIMER = ARM_IRQ_BASIC(0x00),
415:     /// @brief ARM mailbox interrupt
416:     FIQ_ARM_MAILBOX = ARM_IRQ_BASIC(0x01),
417:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
418:     FIQ_ARM_DOORBELL_0 = ARM_IRQ_BASIC(0x02),
419:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
420:     FIQ_ARM_DOORBELL_1 = ARM_IRQ_BASIC(0x03),
421:     /// @brief VPU halted 0 interrupt
422:     FIQ_VPU0_HALTED = ARM_IRQ_BASIC(0x04),
423:     /// @brief VPU halted 1 interrupt
424:     FIQ_VPU1_HALTED = ARM_IRQ_BASIC(0x05),
425:     /// @brief Illegal type 0 interrupt
426:     FIQ_ILLEGAL_TYPE0 = ARM_IRQ_BASIC(0x06),
427:     /// @brief Illegal type 1 interrupt
428:     FIQ_ILLEGAL_TYPE1 = ARM_IRQ_BASIC(0x07),
429:
430:     /// @brief Secure Physical ARM timer
431:     FIQ_LOCAL_CNTPS = ARM_IRQ_LOCAL(0x00),
432:     /// @brief Non secure Physical ARM timer
433:     FIQ_LOCAL_CNTPNS = ARM_IRQ_LOCAL(0x01),
434:     /// @brief Hypervisor Physical ARM timer
435:     FIQ_LOCAL_CNTHP = ARM_IRQ_LOCAL(0x02),
436:     /// @brief Virtual ARM timer
437:     FIQ_LOCAL_CNTV = ARM_IRQ_LOCAL(0x03),
438:     /// @brief Mailbox 0
439:     FIQ_LOCAL_MAILBOX0 = ARM_IRQ_LOCAL(0x04),
440:     /// @brief Mailbox 1
441:     FIQ_LOCAL_MAILBOX1 = ARM_IRQ_LOCAL(0x05),
442:     /// @brief Mailbox 2
443:     FIQ_LOCAL_MAILBOX2 = ARM_IRQ_LOCAL(0x06),
444:     /// @brief Mailbox 3
445:     FIQ_LOCAL_MAILBOX3 = ARM_IRQ_LOCAL(0x07),
446:     /// @brief Cascaded GPU interrupts
447:     FIQ_LOCAL_GPU = ARM_IRQ_LOCAL(0x08),
448:     /// @brief Performance Monitoring Unit
449:     FIQ_LOCAL_PMU = ARM_IRQ_LOCAL(0x09),
450:     /// @brief AXI bus idle, on core 0 only
451:     FIQ_LOCAL_AXI_IDLE = ARM_IRQ_LOCAL(0x0A),
452:     /// @brief Local timer
453:     FIQ_LOCAL_LOCALTIMER = ARM_IRQ_LOCAL(0x0B),
454: };
455:
456: #else
457:
458: // IRQs
459: /// @brief Software generated interrupt, per core
460: #define GIC_SGI(n) (0 + (n))
461: /// @brief Private peripheral interrupt, per core
462: #define GIC_PPI(n) (16 + (n))
463: /// @brief Shared peripheral interrupt, shared between cores
464: #define GIC_SPI(n) (32 + (n))
465:
466: /// @brief Total number of IRQ lines Raspberry Pi 4 and 5
467: #define IRQ_LINES  256
468:
469: /// <summary>
470: /// @brief IRQ interrupt numbers
471: /// </summary>
472: enum class IRQ_ID
473: {
474:     /// @brief Hypervisor Physical ARM timer
475:     IRQ_LOCAL_CNTHP = GIC_PPI(0x0A),
476:     /// @brief Virtual ARM timer
477:     IRQ_LOCAL_CNTV = GIC_PPI(0x0B),
478:     /// @brief Secure Physical ARM timer
479:     IRQ_LOCAL_CNTPS = GIC_PPI(0x0D),
480:     /// @brief Non secure Physical ARM timer
481:     IRQ_LOCAL_CNTPNS = GIC_PPI(0x0E),
482:
483:     /// @brief ARM core mailbox 0, core 0
484:     IRQ_LOCAL_MAILBOX_0_0 = GIC_SPI(0x00),
485:     /// @brief ARM core mailbox 1, core 0
486:     IRQ_LOCAL_MAILBOX_1_0 = GIC_SPI(0x01),
487:     /// @brief ARM core mailbox 2, core 0
488:     IRQ_LOCAL_MAILBOX_2_0 = GIC_SPI(0x02),
489:     /// @brief ARM core mailbox 3, core 0
490:     IRQ_LOCAL_MAILBOX_3_0 = GIC_SPI(0x03),
491:     /// @brief ARM core mailbox 0, core 1
492:     IRQ_LOCAL_MAILBOX_0_1 = GIC_SPI(0x04),
493:     /// @brief ARM core mailbox 1, core 1
494:     IRQ_LOCAL_MAILBOX_1_1 = GIC_SPI(0x05),
495:     /// @brief ARM core mailbox 2, core 1
496:     IRQ_LOCAL_MAILBOX_2_1 = GIC_SPI(0x06),
497:     /// @brief ARM core mailbox 3, core 1
498:     IRQ_LOCAL_MAILBOX_3_1 = GIC_SPI(0x07),
499:     /// @brief ARM core mailbox 0, core 2
500:     IRQ_LOCAL_MAILBOX_0_2 = GIC_SPI(0x08),
501:     /// @brief ARM core mailbox 1, core 2
502:     IRQ_LOCAL_MAILBOX_1_2 = GIC_SPI(0x09),
503:     /// @brief ARM core mailbox 2, core 2
504:     IRQ_LOCAL_MAILBOX_2_2 = GIC_SPI(0x0A),
505:     /// @brief ARM core mailbox 3, core 2
506:     IRQ_LOCAL_MAILBOX_3_2 = GIC_SPI(0x0B),
507:     /// @brief ARM core mailbox 0, core 3
508:     IRQ_LOCAL_MAILBOX_0_3 = GIC_SPI(0x0C),
509:     /// @brief ARM core mailbox 1, core 3
510:     IRQ_LOCAL_MAILBOX_1_3 = GIC_SPI(0x0D),
511:     /// @brief ARM core mailbox 2, core 3
512:     IRQ_LOCAL_MAILBOX_2_3 = GIC_SPI(0x0E),
513:     /// @brief ARM core mailbox 3, core 3
514:     IRQ_LOCAL_MAILBOX_3_3 = GIC_SPI(0x0F),
515:
516:     /// @brief PMU (performance monitoring unit) core 0 interrupt
517:     IRQ_PMU0 = GIC_SPI(0x10),
518:     /// @brief PMU (performance monitoring unit) core 1 interrupt
519:     IRQ_PMU1 = GIC_SPI(0x11),
520:     /// @brief PMU (performance monitoring unit) core 2 interrupt
521:     IRQ_PMU2 = GIC_SPI(0x12),
522:     /// @brief PMU (performance monitoring unit) core 3 interrupt
523:     IRQ_PMU3 = GIC_SPI(0x13),
524:     /// @brief AXI bus error
525:     IRQ_LOCAL_AXI_ERR = GIC_SPI(0x14),
526:     /// @brief ARM mailbox interrupt
527:     IRQ_ARM_MAILBOX = GIC_SPI(0x21),
528:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
529:     IRQ_ARM_DOORBELL_0 = GIC_SPI(0x22),
530:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
531:     IRQ_ARM_DOORBELL_1 = GIC_SPI(0x23),
532:     /// @brief VPU halted 0 interrupt
533:     IRQ_VPU0_HALTED = GIC_SPI(0x24),
534:     /// @brief VPU halted 1 interrupt
535:     IRQ_VPU1_HALTED = GIC_SPI(0x25),
536:     /// @brief ARM address error interrupt
537:     IRQ_ILLEGAL_TYPE0 = GIC_SPI(0x26),
538:     /// @brief ARM AXI error interrupt
539:     IRQ_ILLEGAL_TYPE1 = GIC_SPI(0x27),
540:     /// @brief USB Host port interrupt
541:     IRQ_HOSTPORT = GIC_SPI(0x28),
542:
543:     /// @brief BCM system timer 0 interrupt, when compare value is hit
544:     IRQ_TIMER0 = GIC_SPI(0x40),
545:     /// @brief BCM system timer 1 interrupt, when compare value is hit
546:     IRQ_TIMER1 = GIC_SPI(0x41),
547:     /// @brief BCM system timer 2 interrupt, when compare value is hit
548:     IRQ_TIMER2 = GIC_SPI(0x42),
549:     /// @brief BCM system timer 3 interrupt, when compare value is hit
550:     IRQ_TIMER3 = GIC_SPI(0x43),
551:     /// @brief H.264 codec 0 interrupt
552:     IRQ_CODEC0 = GIC_SPI(0x44),
553:     /// @brief H.264 codec 1 interrupt
554:     IRQ_CODEC1 = GIC_SPI(0x45),
555:     /// @brief H.264 codec 2 interrupt
556:     IRQ_CODEC2 = GIC_SPI(0x46),
557:     /// @brief JPEG interrupt
558:     IRQ_JPEG = GIC_SPI(0x49),
559:     /// @brief ISP interrupt
560:     IRQ_ISP = GIC_SPI(0x49),
561:     /// @brief USB interrupt
562:     IRQ_USB = GIC_SPI(0x49),
563:     /// @brief VideoCore 3D interrupt
564:     IRQ_3D = GIC_SPI(0x4A),
565:     /// @brief GPU transposer interrupt
566:     IRQ_TRANSPOSER = GIC_SPI(0x4B),
567:     /// @brief Multicore sync 0 interrupt
568:     IRQ_MULTICORESYNC0 = GIC_SPI(0x4C),
569:     /// @brief Multicore sync 1 interrupt
570:     IRQ_MULTICORESYNC1 = GIC_SPI(0x4D),
571:     /// @brief Multicore sync 2 interrupt
572:     IRQ_MULTICORESYNC2 = GIC_SPI(0x4E),
573:     /// @brief Multicore sync 3 interrupt
574:     IRQ_MULTICORESYNC3 = GIC_SPI(0x4F),
575:     /// @brief DMA channel 0 interrupt
576:     IRQ_DMA0 = GIC_SPI(0x50),
577:     /// @brief DMA channel 1 interrupt
578:     IRQ_DMA1 = GIC_SPI(0x51),
579:     /// @brief DMA channel 2 interrupt, I2S PCM TX
580:     IRQ_DMA2 = GIC_SPI(0x52),
581:     /// @brief DMA channel 3 interrupt, I2S PCM RX
582:     IRQ_DMA3 = GIC_SPI(0x53),
583:     /// @brief DMA channel 4 interrupt, SMI
584:     IRQ_DMA4 = GIC_SPI(0x54),
585:     /// @brief DMA channel 5 interrupt, PWM
586:     IRQ_DMA5 = GIC_SPI(0x55),
587:     /// @brief DMA channel 6 interrupt, SPI TX
588:     IRQ_DMA6 = GIC_SPI(0x56),
589:     /// @brief DMA channel 7/8 interrupt, SPI RX
590:     IRQ_DMA7 = GIC_SPI(0x57),
591:     /// @brief DMA channel 7/8 interrupt, undocumented
592:     IRQ_DMA8 = GIC_SPI(0x57),
593:     /// @brief DMA channel 9/10 interrupt, undocumented
594:     IRQ_DMA9 = GIC_SPI(0x58),
595:     /// @brief DMA channel 9/10 interrupt, HDMI
596:     IRQ_DMA10 = GIC_SPI(0x58),
597:     /// @brief DMA channel 11 interrupt, EMMC
598:     IRQ_DMA11 = GIC_SPI(0x59),
599:     /// @brief DMA channel 12 interrupt, UART TX
600:     IRQ_DMA12 = GIC_SPI(0x5A),
601:     /// @brief DMA channel 13 interrupt, undocumented
602:     IRQ_DMA13 = GIC_SPI(0x5B),
603:     /// @brief DMA channel 14 interrupt, UART RX
604:     IRQ_DMA14 = GIC_SPI(0x5C),
605:     /// @brief AUX UART 1 / SPI 0/1 interrupt
606:     IRQ_AUX_UART = GIC_SPI(0x5D),
607:     /// @brief AUX UART 1 / SPI 0/1 interrupt
608:     IRQ_AUX_SPI = GIC_SPI(0x5D),
609:     /// @brief ARM interrupt
610:     IRQ_ARM = GIC_SPI(0x5E),
611:     /// @brief DMA channel 15 interrupt
612:     IRQ_DMA15 = GIC_SPI(0x5F),
613:     /// @brief HMDI CEC interrupt
614:     IRQ_HDMI_CEC = GIC_SPI(0x60),
615:     /// @brief HVS (video scaler) interrupt
616:     IRQ_VIDEOSCALER = GIC_SPI(0x61),
617:     /// @brief Video decoder interrupt
618:     IRQ_DECODER = GIC_SPI(0x62),
619:     /// @brief DSI 0 (display) interrupt
620:     IRQ_DSI0 = GIC_SPI(0x64),
621:     /// @brief GPU pixel valve 2 interrupt
622:     IRQ_PIXELVALVE2 = GIC_SPI(0x65),
623:     /// @brief CSI 0 (camera) interrupt
624:     IRQ_CAM0 = GIC_SPI(0x66),
625:     /// @brief CSI 1 (camera) interrupt
626:     IRQ_CAM1 = GIC_SPI(0x67),
627:     /// @brief HDMI 0 interrupt
628:     IRQ_HDMI0 = GIC_SPI(0x68),
629:     /// @brief HDMI 1 interrupt
630:     IRQ_HDMI1 = GIC_SPI(0x69),
631:     /// @brief GPU pixel valve 3 interrupt
632:     IRQ_PIXELVALVE3 = GIC_SPI(0x6A),
633:     /// @brief SPI BSC slave interrupt
634:     IRQ_SPI_BSC = GIC_SPI(0x6A),
635:     /// @brief DSI 1 (display) interrupt
636:     IRQ_DSI1 = GIC_SPI(0x6C),
637:     /// @brief GPU pixel valve 0 interrupt
638:     IRQ_PIXELVALVE0 = GIC_SPI(0x6D),
639:     /// @brief GPU pixel valve 1 / 4 interrupt
640:     IRQ_PIXELVALVE1 = GIC_SPI(0x6E),
641:     /// @brief CPR interrupt
642:     IRQ_CPR = GIC_SPI(0x6F),
643:     /// @brief SMI (firmware) interrupt
644:     IRQ_FIRMWARE = GIC_SPI(0x70),
645:     /// @brief GPIO 0 interrupt
646:     IRQ_GPIO0 = GIC_SPI(0x71),
647:     /// @brief GPIO 1 interrupt
648:     IRQ_GPIO1 = GIC_SPI(0x72),
649:     /// @brief GPIO 2 interrupt
650:     IRQ_GPIO2 = GIC_SPI(0x73),
651:     /// @brief GPIO 3 interrupt
652:     IRQ_GPIO3 = GIC_SPI(0x74),
653:     /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
654:     IRQ_I2C = GIC_SPI(0x75),
655:     /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
656:     IRQ_SPI = GIC_SPI(0x76),
657:     /// @brief I2S interrupt
658:     IRQ_I2SPCM = GIC_SPI(0x77),
659:     /// @brief SD host interrupt
660:     IRQ_SDHOST = GIC_SPI(0x78),
661:     /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
662:     IRQ_UART = GIC_SPI(0x79),
663:     /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
664:     IRQ_SLIMBUS = GIC_SPI(0x7A),
665:     /// @brief GPU? VEC interrupt
666:     IRQ_VEC = GIC_SPI(0x7B),
667:     /// @brief CPG interrupt
668:     IRQ_CPG = GIC_SPI(0x7C),
669:     /// @brief RNG (random number generator) interrupt
670:     IRQ_RNG = GIC_SPI(0x7D),
671:     /// @brief EMMC / EMMC2 interrupt
672:     IRQ_ARASANSDIO = GIC_SPI(0x7E),
673:     /// @brief Ethernet PCIe secure interrupt
674:     IRQ_ETH_PCIE_S = GIC_SPI(0x7F),
675:     /// @brief PCI Express AVS interrupt
676:     IRQ_AVS = GIC_SPI(0x89),
677:     /// @brief PCI Express Ethernet A interrupt
678:     IRQ_PCIE_INTA = GIC_SPI(0x8F),
679:     /// @brief PCI Express Ethernet B interrupt
680:     IRQ_PCIE_INTB = GIC_SPI(0x90),
681:     /// @brief PCI Express Ethernet C interrupt
682:     IRQ_PCIE_INTC = GIC_SPI(0x91),
683:     /// @brief PCI Express Ethernet D interrupt
684:     IRQ_PCIE_INTD = GIC_SPI(0x92),
685:     /// @brief PCI Express Host A interrupt
686:     IRQ_PCIE_HOST_INTA = GIC_SPI(0x93),
687:     /// @brief PCI Express Host MSI interrupt
688:     IRQ_PCIE_HOST_MSI = GIC_SPI(0x94),
689:     /// @brief Ethernet interrupt
690:     IRQ_GENET_0_A = GIC_SPI(0x9D),
691:     /// @brief Ethernet interrupt
692:     IRQ_GENET_0_B = GIC_SPI(0x9E),
693:     /// @brief USB XHCI interrupt
694:     IRQ_XHCI_INTERNAL = GIC_SPI(0xB0),
695: };
696:
697: /// <summary>
698: /// @brief IRQ interrupt numbers
699: /// </summary>
700: enum class FIQ_ID
701: {
702:     /// @brief Hypervisor Physical ARM timer
703:     FIQ_LOCAL_CNTHP = GIC_PPI(0x0A),
704:     /// @brief Virtual ARM timer
705:     FIQ_LOCAL_CNTV = GIC_PPI(0x0B),
706:     /// @brief Secure Physical ARM timer
707:     FIQ_LOCAL_CNTPS = GIC_PPI(0x0D),
708:     /// @brief Non secure Physical ARM timer
709:     FIQ_LOCAL_CNTPNS = GIC_PPI(0x0E),
710:
711:     /// @brief ARM core mailbox 0, core 0
712:     FIQ_LOCAL_MAILBOX_0_0 = GIC_SPI(0x00),
713:     /// @brief ARM core mailbox 1, core 0
714:     FIQ_LOCAL_MAILBOX_1_0 = GIC_SPI(0x01),
715:     /// @brief ARM core mailbox 2, core 0
716:     FIQ_LOCAL_MAILBOX_2_0 = GIC_SPI(0x02),
717:     /// @brief ARM core mailbox 3, core 0
718:     FIQ_LOCAL_MAILBOX_3_0 = GIC_SPI(0x03),
719:     /// @brief ARM core mailbox 0, core 1
720:     FIQ_LOCAL_MAILBOX_0_1 = GIC_SPI(0x04),
721:     /// @brief ARM core mailbox 1, core 1
722:     FIQ_LOCAL_MAILBOX_1_1 = GIC_SPI(0x05),
723:     /// @brief ARM core mailbox 2, core 1
724:     FIQ_LOCAL_MAILBOX_2_1 = GIC_SPI(0x06),
725:     /// @brief ARM core mailbox 3, core 1
726:     FIQ_LOCAL_MAILBOX_3_1 = GIC_SPI(0x07),
727:     /// @brief ARM core mailbox 0, core 2
728:     FIQ_LOCAL_MAILBOX_0_2 = GIC_SPI(0x08),
729:     /// @brief ARM core mailbox 1, core 2
730:     FIQ_LOCAL_MAILBOX_1_2 = GIC_SPI(0x09),
731:     /// @brief ARM core mailbox 2, core 2
732:     FIQ_LOCAL_MAILBOX_2_2 = GIC_SPI(0x0A),
733:     /// @brief ARM core mailbox 3, core 2
734:     FIQ_LOCAL_MAILBOX_3_2 = GIC_SPI(0x0B),
735:     /// @brief ARM core mailbox 0, core 3
736:     FIQ_LOCAL_MAILBOX_0_3 = GIC_SPI(0x0C),
737:     /// @brief ARM core mailbox 1, core 3
738:     FIQ_LOCAL_MAILBOX_1_3 = GIC_SPI(0x0D),
739:     /// @brief ARM core mailbox 2, core 3
740:     FIQ_LOCAL_MAILBOX_2_3 = GIC_SPI(0x0E),
741:     /// @brief ARM core mailbox 3, core 3
742:     FIQ_LOCAL_MAILBOX_3_3 = GIC_SPI(0x0F),
743:
744:     /// @brief PMU (performance monitoring unit) core 0 interrupt
745:     FIQ_PMU0 = GIC_SPI(0x10),
746:     /// @brief PMU (performance monitoring unit) core 1 interrupt
747:     FIQ_PMU1 = GIC_SPI(0x11),
748:     /// @brief PMU (performance monitoring unit) core 2 interrupt
749:     FIQ_PMU2 = GIC_SPI(0x12),
750:     /// @brief PMU (performance monitoring unit) core 3 interrupt
751:     FIQ_PMU3 = GIC_SPI(0x13),
752:     /// @brief AXI bus error
753:     FIQ_LOCAL_AXI_ERR = GIC_SPI(0x14),
754:     /// @brief ARM mailbox interrupt
755:     FIQ_ARM_MAILBOX = GIC_SPI(0x21),
756:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
757:     FIQ_ARM_DOORBELL_0 = GIC_SPI(0x22),
758:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
759:     FIQ_ARM_DOORBELL_1 = GIC_SPI(0x23),
760:     /// @brief VPU halted 0 interrupt
761:     FIQ_VPU0_HALTED = GIC_SPI(0x24),
762:     /// @brief VPU halted 1 interrupt
763:     FIQ_VPU1_HALTED = GIC_SPI(0x25),
764:     /// @brief ARM address error interrupt
765:     FIQ_ILLEGAL_TYPE0 = GIC_SPI(0x26),
766:     /// @brief ARM AXI error interrupt
767:     FIQ_ILLEGAL_TYPE1 = GIC_SPI(0x27),
768:     /// @brief USB Host port interrupt
769:     FIQ_HOSTPORT = GIC_SPI(0x28),
770:
771:     /// @brief BCM system timer 0 interrupt, when compare value is hit
772:     FIQ_TIMER0 = GIC_SPI(0x40),
773:     /// @brief BCM system timer 1 interrupt, when compare value is hit
774:     FIQ_TIMER1 = GIC_SPI(0x41),
775:     /// @brief BCM system timer 2 interrupt, when compare value is hit
776:     FIQ_TIMER2 = GIC_SPI(0x42),
777:     /// @brief BCM system timer 3 interrupt, when compare value is hit
778:     FIQ_TIMER3 = GIC_SPI(0x43),
779:     /// @brief H.264 codec 0 interrupt
780:     FIQ_CODEC0 = GIC_SPI(0x44),
781:     /// @brief H.264 codec 1 interrupt
782:     FIQ_CODEC1 = GIC_SPI(0x45),
783:     /// @brief H.264 codec 2 interrupt
784:     FIQ_CODEC2 = GIC_SPI(0x46),
785:     /// @brief JPEG interrupt
786:     FIQ_JPEG = GIC_SPI(0x49),
787:     /// @brief ISP interrupt
788:     FIQ_ISP = GIC_SPI(0x49),
789:     /// @brief USB interrupt
790:     FIQ_USB = GIC_SPI(0x49),
791:     /// @brief VideoCore 3D interrupt
792:     FIQ_3D = GIC_SPI(0x4A),
793:     /// @brief GPU transposer interrupt
794:     FIQ_TRANSPOSER = GIC_SPI(0x4B),
795:     /// @brief Multicore sync 0 interrupt
796:     FIQ_MULTICORESYNC0 = GIC_SPI(0x4C),
797:     /// @brief Multicore sync 1 interrupt
798:     FIQ_MULTICORESYNC1 = GIC_SPI(0x4D),
799:     /// @brief Multicore sync 2 interrupt
800:     FIQ_MULTICORESYNC2 = GIC_SPI(0x4E),
801:     /// @brief Multicore sync 3 interrupt
802:     FIQ_MULTICORESYNC3 = GIC_SPI(0x4F),
803:     /// @brief DMA channel 0 interrupt
804:     FIQ_DMA0 = GIC_SPI(0x50),
805:     /// @brief DMA channel 1 interrupt
806:     FIQ_DMA1 = GIC_SPI(0x51),
807:     /// @brief DMA channel 2 interrupt, I2S PCM TX
808:     FIQ_DMA2 = GIC_SPI(0x52),
809:     /// @brief DMA channel 3 interrupt, I2S PCM RX
810:     FIQ_DMA3 = GIC_SPI(0x53),
811:     /// @brief DMA channel 4 interrupt, SMI
812:     FIQ_DMA4 = GIC_SPI(0x54),
813:     /// @brief DMA channel 5 interrupt, PWM
814:     FIQ_DMA5 = GIC_SPI(0x55),
815:     /// @brief DMA channel 6 interrupt, SPI TX
816:     FIQ_DMA6 = GIC_SPI(0x56),
817:     /// @brief DMA channel 7/8 interrupt, SPI RX
818:     FIQ_DMA7 = GIC_SPI(0x57),
819:     /// @brief DMA channel 7/8 interrupt, undocumented
820:     FIQ_DMA8 = GIC_SPI(0x57),
821:     /// @brief DMA channel 9/10 interrupt, undocumented
822:     FIQ_DMA9 = GIC_SPI(0x58),
823:     /// @brief DMA channel 9/10 interrupt, HDMI
824:     FIQ_DMA10 = GIC_SPI(0x58),
825:     /// @brief DMA channel 11 interrupt, EMMC
826:     FIQ_DMA11 = GIC_SPI(0x59),
827:     /// @brief DMA channel 12 interrupt, UART TX
828:     FIQ_DMA12 = GIC_SPI(0x5A),
829:     /// @brief DMA channel 13 interrupt, undocumented
830:     FIQ_DMA13 = GIC_SPI(0x5B),
831:     /// @brief DMA channel 14 interrupt, UART RX
832:     FIQ_DMA14 = GIC_SPI(0x5C),
833:     /// @brief AUX UART 1 / SPI 0/1 interrupt
834:     FIQ_AUX_UART = GIC_SPI(0x5D),
835:     /// @brief AUX UART 1 / SPI 0/1 interrupt
836:     FIQ_AUX_SPI = GIC_SPI(0x5D),
837:     /// @brief ARM interrupt
838:     FIQ_ARM = GIC_SPI(0x5E),
839:     /// @brief DMA channel 15 interrupt
840:     FIQ_DMA15 = GIC_SPI(0x5F),
841:     /// @brief HMDI CEC interrupt
842:     FIQ_HDMI_CEC = GIC_SPI(0x60),
843:     /// @brief HVS (video scaler) interrupt
844:     FIQ_VIDEOSCALER = GIC_SPI(0x61),
845:     /// @brief Video decoder interrupt
846:     FIQ_DECODER = GIC_SPI(0x62),
847:     /// @brief DSI 0 (display) interrupt
848:     FIQ_DSI0 = GIC_SPI(0x64),
849:     /// @brief GPU pixel valve 2 interrupt
850:     FIQ_PIXELVALVE2 = GIC_SPI(0x65),
851:     /// @brief CSI 0 (camera) interrupt
852:     FIQ_CAM0 = GIC_SPI(0x66),
853:     /// @brief CSI 1 (camera) interrupt
854:     FIQ_CAM1 = GIC_SPI(0x67),
855:     /// @brief HDMI 0 interrupt
856:     FIQ_HDMI0 = GIC_SPI(0x68),
857:     /// @brief HDMI 1 interrupt
858:     FIQ_HDMI1 = GIC_SPI(0x69),
859:     /// @brief GPU pixel valve 3 interrupt
860:     FIQ_PIXELVALVE3 = GIC_SPI(0x6A),
861:     /// @brief SPI BSC slave interrupt
862:     FIQ_SPI_BSC = GIC_SPI(0x6A),
863:     /// @brief DSI 1 (display) interrupt
864:     FIQ_DSI1 = GIC_SPI(0x6C),
865:     /// @brief GPU pixel valve 0 interrupt
866:     FIQ_PIXELVALVE0 = GIC_SPI(0x6D),
867:     /// @brief GPU pixel valve 1 / 4 interrupt
868:     FIQ_PIXELVALVE1 = GIC_SPI(0x6E),
869:     /// @brief CPR interrupt
870:     FIQ_CPR = GIC_SPI(0x6F),
871:     /// @brief SMI (firmware) interrupt
872:     FIQ_FIRMWARE = GIC_SPI(0x70),
873:     /// @brief GPIO 0 interrupt
874:     FIQ_GPIO0 = GIC_SPI(0x71),
875:     /// @brief GPIO 1 interrupt
876:     FIQ_GPIO1 = GIC_SPI(0x72),
877:     /// @brief GPIO 2 interrupt
878:     FIQ_GPIO2 = GIC_SPI(0x73),
879:     /// @brief GPIO 3 interrupt
880:     FIQ_GPIO3 = GIC_SPI(0x74),
881:     /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
882:     FIQ_I2C = GIC_SPI(0x75),
883:     /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
884:     FIQ_SPI = GIC_SPI(0x76),
885:     /// @brief I2S interrupt
886:     FIQ_I2SPCM = GIC_SPI(0x77),
887:     /// @brief SD host interrupt
888:     FIQ_SDHOST = GIC_SPI(0x78),
889:     /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
890:     FIQ_UART = GIC_SPI(0x79),
891:     /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
892:     FIQ_SLIMBUS = GIC_SPI(0x7A),
893:     /// @brief GPU? VEC interrupt
894:     FIQ_VEC = GIC_SPI(0x7B),
895:     /// @brief CPG interrupt
896:     FIQ_CPG = GIC_SPI(0x7C),
897:     /// @brief RNG (random number generator) interrupt
898:     FIQ_RNG = GIC_SPI(0x7D),
899:     /// @brief EMMC / EMMC2 interrupt
900:     FIQ_ARASANSDIO = GIC_SPI(0x7E),
901:     /// @brief Ethernet PCIe secure interrupt
902:     FIQ_ETH_PCIE_S = GIC_SPI(0x7F),
903:     /// @brief PCI Express AVS interrupt
904:     FIQ_AVS = GIC_SPI(0x89),
905:     /// @brief PCI Express Ethernet A interrupt
906:     FIQ_PCIE_INTA = GIC_SPI(0x8F),
907:     /// @brief PCI Express Ethernet B interrupt
908:     FIQ_PCIE_INTB = GIC_SPI(0x90),
909:     /// @brief PCI Express Ethernet C interrupt
910:     FIQ_PCIE_INTC = GIC_SPI(0x91),
911:     /// @brief PCI Express Ethernet D interrupt
912:     FIQ_PCIE_INTD = GIC_SPI(0x92),
913:     /// @brief PCI Express Host A interrupt
914:     FIQ_PCIE_HOST_INTA = GIC_SPI(0x93),
915:     /// @brief PCI Express Host MSI interrupt
916:     FIQ_PCIE_HOST_MSI = GIC_SPI(0x94),
917:     /// @brief Ethernet interrupt
918:     FIQ_GENET_0_A = GIC_SPI(0x9D),
919:     /// @brief Ethernet interrupt
920:     FIQ_GENET_0_B = GIC_SPI(0x9E),
921:     /// @brief USB XHCI interrupt
922:     FIQ_XHCI_INTERNAL = GIC_SPI(0xB0),
923: };
924:
925: #endif
926:
927: void EnableIRQ(IRQ_ID irqID);
928: void DisableIRQ(IRQ_ID irqID);
929:
930: void EnableFIQ(FIQ_ID fiqID);
931: void DisableFIQ(FIQ_ID fiqID);
932:
933: } // namespace baremetal
```

We'll not get into each and every IRQ or FIQ id, but give some descriptions on macros used

- Line 48-454: We define IRQ related macros for Raspberry Pi 3
  - Line 49-54: We define the number of IRQ supported by IRQ 1/2, IRQ basic and ARM local IRQ
  - Line 56-57: We define the number of BCM interrupt registers
  - Line 59-60: We define the total count of IRQ interrupts supported by the system for Raspberry 3
  - Line 62-77: We define the ID range for IRQ 1/2, IRQ basic and ARM local IRQ
  - Line 79-80: We define a macro to access the IRQ pending register for BCM IRQ lines
  - Line 81-82: We define a macro to access the IRQ enable register for BCM IRQ lines
  - Line 83-84: We define a macro to access the IRQ disable register for BCM IRQ lines
  - Line 85-86: We define a macro to determine the mask for the selected IRQ id to access the IRQ pending / enable / disable register
  - Line 88-270: We define all currently supported IRQ for Raspberry 3
  - Line 272-454: We define all currently supported FIQ for Raspberry 3
- Line 459-923: We define IRQ related macros for Raspberry Pi 4
  - Line 459-464: We define macros to access software generated interrupts, private per core interrupts and interrupts shared by all cores for Raspberry 4
  - Line 466-467: We define the total count of IRQ interrupts supported by the system for Raspberry 4
  - Line 469-695: We define all currently supported IRQ for Raspberry 4 / 5
  - Line 697-925: We define all currently supported FIQ for Raspberry 4 / 5

### System.cpp {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_SYSTEMCPP}

As we've added the `Shutdown()` method to `InterruptSystem`, it is wise to make sure we nicely shutdown the interrupt system before halting or rebooting.
Let's add a call for this.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
118: void System::Halt()
119: {
120:     LOG_INFO("Halt");
121:     GetInterruptSystem().Shutdown();
122:     Timer::WaitMilliSeconds(WaitTime);
...
142: void System::Reboot()
143: {
144:     LOG_INFO("Reboot");
145:     GetInterruptSystem().Shutdown();
146:     Timer::WaitMilliSeconds(WaitTime);
...
```

### Update application code {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_UPDATE_APPLICATION_CODE}

We'll make a few small changes to use the new `InterruptSystem` implementation.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/InterruptHandler.h"
5: #include "baremetal/Interrupts.h"
6: #include "baremetal/Logger.h"
7: #include "baremetal/System.h"
8: #include "baremetal/Timer.h"
9:
10: LOG_MODULE("main");
11:
12: using namespace baremetal;
13:
14: #define TICKS_PER_SECOND 2 // Timer ticks per second
15:
16: static uint32 clockTicksPerSystemTick;
17:
18: void IntHandler(void* param)
19: {
20:     uint64 counterCompareValue;
21:     GetTimerCompareValue(counterCompareValue);
22:     SetTimerCompareValue(counterCompareValue + clockTicksPerSystemTick);
23:
24:     LOG_INFO("Ping");
25: }
26:
27: int main()
28: {
29:     auto& console = GetConsole();
30:
31:     auto exceptionLevel = CurrentEL();
32:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
33:
34:     uint64 counterFreq{};
35:     GetTimerFrequency(counterFreq);
36:     assert(counterFreq % TICKS_PER_SECOND == 0);
37:     clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
38:     LOG_INFO("Clock ticks per second: %d, clock ticks per interrupt: %d", counterFreq, clockTicksPerSystemTick);
39:
40:     GetInterruptSystem().RegisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS, IntHandler, nullptr);
41:
42:     uint64 counter;
43:     GetTimerCounter(counter);
44:     SetTimerCompareValue(counter + clockTicksPerSystemTick);
45:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
46:
47:     LOG_INFO("Wait 5 seconds");
48:     Timer::WaitMilliSeconds(5000);
49:
50:     GetInterruptSystem().UnregisterIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
51:
52:     console.Write("Press r to reboot, h to halt\n");
53:     char ch{};
54:     while ((ch != 'r') && (ch != 'h'))
55:     {
56:         ch = console.ReadChar();
57:         console.WriteChar(ch);
58:     }
59:
60:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
61: }
```

- Line 40: We replace the code to register the IRQ handler with a single call to `RegisterIRQHandler()`
- Line 50: We replace the code to unregister the IRQ handler with a single call to `UnregisterIRQHandler()`

### Configuring, building and debugging {#TUTORIAL_19_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will act exactly as before. We expect to see 10 ticks happening.

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:92)
Info   Starting up (System:211)
Info   Current EL: 1 (main:32)
Info   Clock ticks per second: 54000000, clock ticks per interrupt: 27000000 (main:38)
Info   Wait 5 seconds (main:47)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Info   Ping (main:24)
Press r to reboot, h to halt
```

Next: [20-timer-extension](20-timer-extension.md)
