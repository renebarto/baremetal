# Tutorial 18: Interrupts {#TUTORIAL_18_INTERRUPTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_18_INTERRUPTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-interrupts`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_18_INTERRUPTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-18.a`
- a library `output/Debug/lib/stdlib-18.a`
- an application `output/Debug/bin/18-interrupts.elf`
- an image in `deploy/Debug/18-interrupts-image`

## Interrupt handling - Step 1 {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1}

The ARM processor supports two different kinds of interrupts:

- IRQ: Normal interrupts
- FIQ: Fast interrupts

IRQ is a interrupt request, which is a standard technique used in computer CPUs to deal with events that need to be processed as they occur, such as receiving data from a network card, or keyboard or mouse actions.

FIQs are specific to the ARM architecture, which supports two types of interrupts; FIQs for fast, low-latency interrupt handling, and standard interrupt requests (IRQs), for more general interrupts.

A FIQ takes priority over an IRQ in an ARM system. Only one FIQ source at a time is supported.
This helps reduce interrupt latency as the interrupt service routine can be executed directly without determining the source of the interrupt.
A context save is not required for servicing a FIQ since it has its own set of banked registers. This reduces the overhead of context switching.

FIQs are often used for data transfers such as DMA operations.

The ARM chip has many different interrupt, the Raspberry Pi SoC add even more for each of the peripheral devices.

### ExceptionStub.S {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_EXCEPTIONSTUBS}

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
- Line 113-200: We create a macro `irq_stub` for an IRQ interrupt, which saves almost all registers, depending on the define `BAREMETAL_SAVE_VFP_REGS_ON_IRQ`.
If `BAREMETAL_SAVE_VFP_REGS_ON_IRQ` is defined, also all floating point registers (Q0-Q31) are saved.
These are then stored on the stack, every time decreasing the stack pointer by 32 or 16 bytes, depending on the registers.
The FIQ interrupts are enabled while the interrupt is being handled, to allow for priority.
After saving the registers the interrupt handler passed through `handler` parameter is called, and after the call returns, the registers are restored.
Then FIQ interrupts are disabled again, and the stub returns to the state before the interrupt using `eret`.
This also reset the state of interrupt enables, etc. as well as the exception level. Note that even though we disable FIQ here, returning to the original exception level may enable them again
- Line 205-291: We implement the FIQ interrupt handler `FIQStub`. This is similar to the normal interrupt stub, however this is not a macro.
Also, the pointer to the handler is retrieved from a memory area structure `s_fiqData`, as well as the parameter to pass to the handler.
If no handler was set no function is called, and the registers are simply restored
- Line 306-310: We define the structure s_fiqData, which is 8 byte aligned. Notice this is part of the `.data` segment, just like the stubs from last tutorial
- Line 318: We declare interrupt stub using the `irq_stub` macro, and set its handler to `InterruptHandler()`

### BCMRegisters.h revisit {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_BCMREGISTERSH_REVISIT}

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

### Interrupts.h {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTSH}

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
42: #include <baremetal/BCMRegisters.h>
43:
44: namespace baremetal {
45:
46: #if BAREMETAL_RPI_TARGET == 3
47:
48: // IRQs
49: /// @brief Number of IRQ lines per register for regular ARM_IRQ1 and ARM_IRQ2
50: #define ARM_IRQS_PER_REG            32
51: /// @brief Number of IRQ lines for ARM_IRQ_BASIC
52: #define ARM_IRQS_BASIC_REG          8
53: /// @brief Number of IRQ lines for ARM_IRQ_LOCAL
54: #define ARM_IRQS_LOCAL_REG          12
55:
56: /// @brief Total number of IRQ lines Raspberry Pi 3
57: #define IRQ_LINES                   (ARM_IRQS_PER_REG * 2 + ARM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)
58:
59: /// @brief Offset for interrupts in IRQ1 group
60: #define ARM_IRQ1_BASE               0
61: /// @brief Offset for interrupts in IRQ2 group
62: #define ARM_IRQ2_BASE               (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
63: /// @brief Offset for interrupts in Basic IRQ group
64: #define ARM_IRQ_BASIC_BASE          (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
65: /// @brief Offset for interrupts in ARM local group
66: #define ARM_IRQ_LOCAL_BASE          (ARM_IRQ_BASIC_BASE + ARM_IRQS_BASIC_REG)
67: /// @brief Calculate IRQ number for IRQ in IRQ1 group
68: #define ARM_IRQ1(n)                 (ARM_IRQ1_BASE + (n))
69: /// @brief Calculate IRQ number for IRQ in IRQ2 group
70: #define ARM_IRQ2(n)                 (ARM_IRQ2_BASE + (n))
71: /// @brief Calculate IRQ number for IRQ in Basic IRQ group
72: #define ARM_IRQ_BASIC(n)             (ARM_IRQ_BASIC_BASE + (n))
73: /// @brief Calculate IRQ number for IRQ in ARM local group
74: #define ARM_IRQ_LOCAL(n)            (ARM_IRQ_LOCAL_BASE + (n))
75:
76: /// @brief Check whether an IRQ is pending (only for IRQ1, IRQ2, Basic IRQ group)
77: #define ARM_IC_IRQ_PENDING(irq)     ((irq) < ARM_IRQ2_BASE              \
78:                                     ? RPI_INTRCTRL_IRQ_PENDING_1        \
79:                                     : ((irq) < ARM_IRQBASIC_BASE        \
80:                                         ? RPI_INTRCTRL_IRQ_PENDING_2    \
81:                                         : RPI_INTRCTRL_IRQ_BASIC_PENDING))
82: /// @brief Enable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
83: #define ARM_IC_IRQS_ENABLE(irq)     ((irq) < ARM_IRQ2_BASE              \
84:                                     ? RPI_INTRCTRL_ENABLE_IRQS_1        \
85:                                     : ((irq) < ARM_IRQ_BASIC_BASE        \
86:                                     ? RPI_INTRCTRL_ENABLE_IRQS_2        \
87:                                     : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
88: /// @brief Disable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
89: #define ARM_IC_IRQS_DISABLE(irq)    ((irq) < ARM_IRQ2_BASE              \
90:                                     ? RPI_INTRCTRL_DISABLE_IRQS_1       \
91:                                     : ((irq) < ARM_IRQ_BASIC_BASE        \
92:                                     ? RPI_INTRCTRL_DISABLE_IRQS_2       \
93:                                     : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
94: /// @brief Calculate bitmask for an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
95: #define ARM_IRQ_MASK(irq)           BIT1((irq) & (ARM_IRQS_PER_REG-1))
96:
97: /// <summary>
98: /// @brief IRQ interrupt numbers
99: /// </summary>
100: enum class IRQ_ID
101: {
102:     /// @brief Non secure Physical ARM timer
103:     IRQ_LOCAL_CNTPS                  = ARM_IRQ_LOCAL(0x00), // armv7-timer
104:     /// @brief Secure Physical ARM timer
105:     IRQ_LOCAL_CNTPNS                 = ARM_IRQ_LOCAL(0x01),
106:     /// @brief Hypervisor Physical ARM timer
107:     IRQ_LOCAL_CNTHP                  = ARM_IRQ_LOCAL(0x02),
108:     /// @brief Virtual ARM timer
109:     IRQ_LOCAL_CNTV                   = ARM_IRQ_LOCAL(0x03),
110:     /// @brief Mailbox 0
111:     IRQ_LOCAL_MAILBOX0               = ARM_IRQ_LOCAL(0x04),
112:     /// @brief Mailbox 1
113:     IRQ_LOCAL_MAILBOX1               = ARM_IRQ_LOCAL(0x05),
114:     /// @brief Mailbox 2
115:     IRQ_LOCAL_MAILBOX2               = ARM_IRQ_LOCAL(0x06),
116:     /// @brief Mailbox 3
117:     IRQ_LOCAL_MAILBOX3               = ARM_IRQ_LOCAL(0x07),
118: };
119:
120: /// <summary>
121: /// @brief FIQ interrupt numbers
122: /// </summary>
123: enum class FIQ_ID
124: {
125:     /// @brief Non secure Physical ARM timer
126:     FIQ_LOCAL_CNTPS                 = ARM_IRQ_LOCAL(0x00),
127:     /// @brief Secure Physical ARM timer
128:     FIQ_LOCAL_CNTPNS                = ARM_IRQ_LOCAL(0x01),
129:     /// @brief Hypervisor Physical ARM timer
130:     FIQ_LOCAL_CNTHP                 = ARM_IRQ_LOCAL(0x02),
131:     /// @brief Virtual ARM timer
132:     FIQ_LOCAL_CNTV                  = ARM_IRQ_LOCAL(0x03),
133:     /// @brief Mailbox 0
134:     FIQ_LOCAL_MAILBOX0              = ARM_IRQ_LOCAL(0x04),
135:     /// @brief Mailbox 1
136:     FIQ_LOCAL_MAILBOX1              = ARM_IRQ_LOCAL(0x05),
137:     /// @brief Mailbox 2
138:     FIQ_LOCAL_MAILBOX2              = ARM_IRQ_LOCAL(0x06),
139:     /// @brief Mailbox 3
140:     FIQ_LOCAL_MAILBOX3              = ARM_IRQ_LOCAL(0x07),
141: };
142:
143: #define ARM_MAX_FIQ                 71
144:
145: #else
146:
147: // IRQs
148: /// @brief Software generated interrupt, per core
149: #define GIC_SGI(n)          (0  + (n))
150: /// @brief Private peripheral interrupt, per core
151: #define GIC_PPI(n)          (16 + (n))
152: /// @brief Shared peripheral interrupt, shared between cores
153: #define GIC_SPI(n)          (32 + (n))
154:
155: /// @brief Total number of IRQ lines Raspberry Pi 4 and 5
156: #define IRQ_LINES           256
157:
158: /// <summary>
159: /// @brief IRQ interrupt numbers
160: /// </summary>
161: enum class IRQ_ID
162: {
163:     IRQ_LOCAL_CNTHP         = GIC_PPI(0x0A),
164:     IRQ_LOCAL_CNTV          = GIC_PPI(0x0B),
165:     IRQ_LOCAL_CNTPS         = GIC_PPI(0x0D),
166:     IRQ_LOCAL_CNTPNS        = GIC_PPI(0x0E),
167: };
168:
169: /// <summary>
170: /// @brief FIQ interrupt numbers
171: /// </summary>
172: enum class FIQ_ID
173: {
174:     FIQ_LOCAL_CNTHP         = GIC_PPI(0x0A),
175:     FIQ_LOCAL_CNTV          = GIC_PPI(0x0B),
176:     FIQ_LOCAL_CNTPS         = GIC_PPI(0x0D),
177:     FIQ_LOCAL_CNTPNS        = GIC_PPI(0x0E),
178: };
179:
180: #define ARM_MAX_FIQ         IRQ_LINES
181:
182: #endif
183:
184: void EnableIRQ(IRQ_ID irqID);
185: void DisableIRQ(IRQ_ID irqID);
186:
187: void EnableFIQ(FIQ_ID fiqID);
188: void DisableFIQ(FIQ_ID fiqID);
189:
190: } // namespace baremetal
```

- Line 52-54: We define different interrupt groups for Raspberry Pi 3
- Line 56-57: We define the number of IRQ requests defined for Raspberry Pi 3 as `IRQ_LINES`
- Line 59-74: We define the offsets and macros to calculate the IRQ number for each group
- Line 76-95: We define macros to calculate the address to check whether an interrupt is pending (`ARM_IC_IRQ_PENDING`),
to enable it (`ARM_IC_IRQS_ENABLE`), or to disable it (`ARM_IC_IRQS_DISABLE`), as well as a macro `ARM_IRQ_MASK` to calculate the bit mask for an IRQ
- Line 100-118: We define the enum `IRQ_ID` for Raspberry Pi 3, with the ARM local interrupts added
- Line 123-141: We define the enum `FIQ_ID` for Raspberry Pi 3, with the ARM local interrupts added
- Line 143: We define the number of FIQ interrupts defined for Raspberry Pi 3 as `ARM_MAX_FIQ`
- Line 147-153: We define different GIC interrupt groups for Raspberry Pi 4 / 5
- Line 155-156: We define the number of IRQ requests defined for Raspberry Pi 4 / 5 as `IRQ_LINES`
- Line 161-167: We define the enum `IRQ_ID` for Raspberry Pi 4 / 5, with the ARM local timer interrupts added
- Line 172-178: We define the enum `FIQ_ID` for Raspberry Pi 4 / 5, with the ARM local timer interrupts added
- Line 180: We define the number of FIQ interrupts defined for Raspberry Pi 4 / 5 as `ARM_MAX_FIQ`
- Line 184: We declare the function `EnableIRQ()` to enable an IRQ
- Line 185: We declare the function `DisableIRQ()` to disable an IRQ
- Line 187: We declare the function `EnableIRQ()` to enable a FIQ
- Line 188: We declare the function `DisableIRQ()` to disable a FIQ

Quite some details for Raspberry Pi 5 are still unknown (at least to me), I'll be adding more later.

### Interrupts.cpp {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTSCPP}

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
40: #include <baremetal/Interrupts.h>
41:
42: #include <baremetal/ARMRegisters.h>
43: #include <baremetal/Assert.h>
44: #include <baremetal/Logger.h>
45: #include <baremetal/MemoryAccess.h>
46:
47: /// @file
48: /// Interrupt numbers and enable / disable functionality
49:
50: /// @brief Define log name
51: LOG_MODULE("Interrupts");
52:
53: void baremetal::EnableIRQ(IRQ_ID irqID)
54: {
55:     int irq = static_cast<int>(irqID);
56:     assert(irq < IRQ_LINES);
57:     MemoryAccess mem;
58: #if BAREMETAL_RPI_TARGET == 3
59:
60:     if (irq < ARM_IRQ_LOCAL_BASE)
61:     {
62:         mem.Write32(ARM_IC_IRQS_ENABLE(irq), ARM_IRQ_MASK(irq));
63:     }
64:     else
65:     {
66:         // The only implemented local IRQs so far
67:         assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPS || irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
68:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
69:             mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(irq - ARM_IRQ_LOCAL_BASE));
70:     }
71:
72: #else
73:
74:     mem.Write32(RPI_GICD_ISENABLER0 + 4 * (irq / 32), BIT1(irq % 32));
75:
76: #endif
77: }
78:
79: void baremetal::DisableIRQ(IRQ_ID irqID)
80: {
81:     int irq = static_cast<int>(irqID);
82:     assert(irq < IRQ_LINES);
83:     MemoryAccess mem;
84: #if BAREMETAL_RPI_TARGET == 3
85:
86:     if (irq < ARM_IRQ_LOCAL_BASE)
87:     {
88:         mem.Write32(ARM_IC_IRQS_DISABLE(irq), ARM_IRQ_MASK(irq));
89:     }
90:     else
91:     {
92:         // The only implemented local IRQs so far
93:         assert(irqID == IRQ_ID::IRQ_LOCAL_CNTPS || irqID == IRQ_ID::IRQ_LOCAL_CNTPNS);
94:         mem.Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
95:             mem.Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(irq - ARM_IRQ_LOCAL_BASE));
96:     }
97:
98: #else
99:
100:     mem.Write32(RPI_GICD_ICENABLER0 + 4 * (irq / 32), BIT1(irq % 32));
101:
102: #endif
103: }
104:
105: void baremetal::EnableFIQ(FIQ_ID fiqID)
106: {
107:     int fiq = static_cast<int>(fiqID);
108:     assert(fiq <= ARM_MAX_FIQ);
109:     MemoryAccess mem;
110: #if BAREMETAL_RPI_TARGET == 3
111:
112:     mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, fiq | BIT1(7));
113:
114: #else
115:
116:     LOG_PANIC("FIQ not supported yet");
117:
118: #endif
119: }
120:
121: void baremetal::DisableFIQ(FIQ_ID fiqID)
122: {
123:     int fiq = static_cast<int>(fiqID);
124:     assert(fiq <= ARM_MAX_FIQ);
125:     MemoryAccess mem;
126: #if BAREMETAL_RPI_TARGET == 3
127:
128:     mem.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
129:
130: #else
131:
132:     LOG_PANIC("FIQ not supported yet");
133:
134: #endif
135: }
```

- Line 53-88: We implement the function `EnableIRQ()`
  - Line 55-56: We convert the IRQ to an integer, and perform as sanity check
  - Line 57: We create a `MemoryAccess` instance for register access
  - Line 60-63: For Raspberry Pi 3, if the IRQ is in the IRQ1, IRQ2 or Basic IRQ group, we use the macro `ARM_IC_IRQS_ENABLE` to determine the register to write to, and the macro `ARM_IRQ_MASK` to determine the bitmask to write.
We then write the bit pattern to the selected address
  - Line 64-70: Otherwise if it is `IRQ_LOCAL_CNTPS` or `IRQ_LOCAL_CNTPNS`, we write to the ARM local interrupt control register with the correct bit patern
  - Line 74: For Raspberry Pi 4 / 5, we calculate the register address `RPI_GICD_ISENABLERn` (there are 8 register of 32 bits each) and the value to write.
We then write the bit pattern to the selected address
- Line 79-103: We implement the function `DisableIRQ()`
  - Line 81-82: We convert the IRQ to an integer, and perform as sanity check
  - Line 83: We create a `MemoryAccess` instance for register access
  - Line 86-89: For Raspberry Pi 3, if the IRQ is in the IRQ1, IRQ2 or Basic IRQ group, we use the macro `ARM_IC_IRQS_ENABLE` to determine the register to write to, and the macro `ARM_IRQ_MASK` to determine the bitmask to write.
We then write the bit pattern to the selected address
  - Line 91-96: Otherwise if it is `IRQ_LOCAL_CNTPS` or `IRQ_LOCAL_CNTPNS`, we write to the ARM local interrupt control register with the correct bit patern
  - Line 100: For Raspberry Pi 4 / 5, we calculate the register address `RPI_GICD_ISENABLERn` (there are 8 register of 32 bits each) and the value to write.
We then write the bit pattern to the selected address
- Line 105-119: We implement the function `EnableFIQ()`
  - Line 107-108: We convert the FIQ to an integer, and perform as sanity check
  - Line 109: We create a `MemoryAccess` instance for register access
  - Line 112: For Raspberry Pi 3, we write the FIQ id to the `RPI_INTRCTRL_FIQ_CONTROL` register and set bit 7 to flag enabling the FIQ.
  - Line 116: For Raspberry Pi 4 / 5, we perform a panic, as we will need to perform a Secure Monitor call in order to set the FIQ, which we did not implement yet.
  As we don't intend to use FIQ yet, we will simply fail for now
- Line 121-135: We implement the function `DisableFIQ()`
  - Line 123-124: We convert the FIQ to an integer, and perform as sanity check
  - Line 125: We create a `MemoryAccess` instance for register access
  - Line 128: For Raspberry Pi 3, we write 0 to the `RPI_INTRCTRL_FIQ_CONTROL` register, resetting set bit 7 to flag no FIQ is enabled.
  - Line 132: For Raspberry Pi 4 / 5, we perform a panic, as we will need to perform a Secure Monitor call in order to set the FIQ, which we did not implement yet.
  As we don't intend to use FIQ yet, we will simply fail for now

### InterruptHandler.h {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERH}

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
42: #include <stdlib/Macros.h>
43: #include <stdlib/Types.h>
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
74:     bool            m_isInitialized;
75:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
76:     IMemoryAccess&  m_memoryAccess;
77:     /// @brief Pointer to registered IRQ handler
78:     IRQHandler*     m_irqHandler;
79:     /// @brief Pointer to parameter to pass to registered IRQ handler
80:     void*           m_irqHandlerParams;
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
Notice that like the `ExceptionHandler()`, this is declared as a `C` function, in order for the linker to link it to rhe assembly code.
- Line 65: We declare a callback function type `IRQHandler` to act as the handler for an IRQ
- Line 71-100: We declare the class `InterruptSystem`, much like `ExceptionSystem`
  - Line 74: The member variable `m_isInitialized` guards against multiple initialization
  - Line 76: The member variable `m_memoryAccess` stores the `IMemoryAccess` instance used.
For now we'll simply use the singleton instance
  - Line 78: The member variable `m_irqHandler` stores the registered handler
  - Line 80: The member variable `m_irqHandlerParams` stores the parameter to pass to the registered handler
  - Line 86: We make `GetInterruptSystem()` a friend, so it can call the constructor to return a singleton instance of the interrup system
  - Line 89: We declare a private default constructor, such that only the `GetInterruptSystem()` function can create an instance
  - Line 92: We declare a destructor
  - Line 94: We declare a method `Initialize()` which will set up the interrupt system
  - Line 96: We declare a method `RegisterIRQHandler()` which will register a handler to be called when an interrupt occurs
  - Line 97: We declare a method `UnregisterIRQHandler()` which will unregister a registered handler
  - Line 99: We declate a method `InterruptHandler()` which is called by the global `InterruptHandler()` function
- Line 102: We declare the function `GetInterruptSystem()`, which creates the singleton instance of the `InterruptSystem` class if needed, and returns a reference to it

### InterruptHandler.cpp {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_INTERRUPTHANDLERCPP}

We'll implement the `InterruptHandler()` function as well as the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Interrupts.h>
45: #include <baremetal/Logger.h>
46: #include <baremetal/MemoryAccess.h>
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
73:     , m_memoryAccess { GetMemoryAccess() }
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
109:         m_memoryAccess.Write32(RPI_GICD_IPRIORITYR0 + 4 * n, RPI_GICD_IPRIORITYR_DEFAULT
110:             | RPI_GICD_IPRIORITYR_DEFAULT << 8
111:             | RPI_GICD_IPRIORITYR_DEFAULT << 16
112:             | RPI_GICD_IPRIORITYR_DEFAULT << 24);
113:
114:         m_memoryAccess.Write32(RPI_GICD_ITARGETSR0 + 4 * n, RPI_GICD_ITARGETSR_CORE0
115:             | RPI_GICD_ITARGETSR_CORE0 << 8
116:             | RPI_GICD_ITARGETSR_CORE0 << 16
117:             | RPI_GICD_ITARGETSR_CORE0 << 24);
118:     }
119:
120:     // set all interrupts to level triggered
121:     for (unsigned n = 0; n < IRQ_LINES / 16; n++)
122:     {
123:         m_memoryAccess.Write32(RPI_GICD_ICFGR0 + 4 * n, 0);
124:     }
125:
126:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
127:
128:     // initialize core 0 CPU interface:
129:
130:     m_memoryAccess.Write32(RPI_GICC_PMR, RPI_GICC_PMR_PRIORITY);
131:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
132:
133: #endif
134:     m_isInitialized = true;
135: }
136:
137: /// <summary>
138: /// Register an IRQ handler
139: /// </summary>
140: /// <param name="handler">Handler to register</param>
141: /// <param name="param">Parameter to pass to IRQ handler</param>
142: void InterruptSystem::RegisterIRQHandler(IRQHandler* handler, void* param)
143: {
144:     assert(m_irqHandler == nullptr);
145:
146:     m_irqHandler = handler;
147:     m_irqHandlerParams = param;
148: }
149:
150: /// <summary>
151: /// Unregister an IRQ handler
152: /// </summary>
153: void InterruptSystem::UnregisterIRQHandler()
154: {
155:     assert(m_irqHandler != nullptr);
156:
157:     m_irqHandler = nullptr;
158:     m_irqHandlerParams = nullptr;
159: }
160:
161: /// <summary>
162: /// Handles an interrupt.
163: ///
164: /// The interrupt handler is called from assembly code (ExceptionStub.S)
165: /// </summary>
166: void InterruptSystem::InterruptHandler()
167: {
168: #if BAREMETAL_RPI_TARGET == 3
169:     if (m_irqHandler)
170:         m_irqHandler(m_irqHandlerParams);
171:
172: #else
173:
174:     uint32 iarValue = m_memoryAccess.Read32(RPI_GICC_IAR);      // Read Interrupt Acknowledge Register
175:
176:     unsigned irq = iarValue & RPI_GICC_IAR_INTERRUPT_ID_MASK;   // Select the currently active interrupt
177:     if (irq < IRQ_LINES)
178:     {
179:         if (irq >= GIC_PPI(0))
180:         {
181:             // Peripheral interrupts (PPI and SPI)
182:             m_irqHandler(m_irqHandlerParams);
183:         }
184:         else
185:         {
186:             // Handle SGI interrupt
187:         }
188:         m_memoryAccess.Write32(RPI_GICC_EOIR, iarValue);        // Flag end of interrupt
189:     }
190: #ifndef NDEBUG
191:     else
192:     {
193:         // spurious interrupts
194:         assert(irq >= 1020);
195:         LOG_INFO("Received spurious interrupt %d", iarValue);
196:     }
197: #endif
198:
199: #endif
200: }
201:
202: /// <summary>
203: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
204: ///
205: /// This is a friend function of class InterruptSystem
206: /// </summary>
207: /// <returns>Reference to the singleton interrupt system instance</returns>
208: InterruptSystem& baremetal::GetInterruptSystem()
209: {
210:     static InterruptSystem singleton;
211:     singleton.Initialize();
212:     return singleton;
213: }
```

- Line 61-64: We implement the `InterruptHandler()` function by calling the `InterruptHandler()` method on the singleton `InterruptSystem` instance
- Line 71-77: We implement the constructor
- Line 81-83: We implement the destructor
- Line 88-135: We implement the `Initialize()` method
  - 90-91: We check if initialiation was already done, if so we return
  - 92: For Raspberry Pi 3 we dont need to do anything
  - 96: For Raspberry Pi 4 / 5 we disable the GIC distributor, basically stopping all interrupts
  - 99-104: We disable, acknowledge and deactivate all interrupts.
Notice that we do this for all 8 registers of 32 bits each
  - 107-118: We direct all interrupts to core 0 with default priority.
Notice that priorities take 8 bits, so 4 interrupts are in a 32 bit register, same for the target core
  - 120-124: We set all interrupts to level triggered.
Notice that priorities take 2 bits, so 16 interrupts are in a 32 bit register
  - 126: We re-enable the GIC distributor
  - 130: We set the priority mask to the default priority
  - 131: We enable the GIC CPU interface
  - 134: We set the initialization flag to true
- Line 142-148: We implement the `RegisterIRQHandler()` method. For now this is a simple implementation, we'll extend this later
  - Line 144: We do a sanity check that no interrupt handler is registered yet
  - Line 146-147: We store the handler and its parameter
- Line 150-158: We implement the `UnregisterIRQHandler()` method
  - Line 155: We do a sanity check that an interrupt handler is registered
  - Line 157-158: We clear the handler and its parameter
- Line 166-200: We implement the `InterruptHandler()` method. This will call the handler, if installed
  - Line 169-170: For Raspberry Pi 3, if a handler is installed, we call it
  - Line 174: For Raspberry Pi 4 / 5, we read the Interrupt Acknowledge Register, which denotes the interrupt we need to handle
  - Line 176: We extract the interrupt number
  - Line 177-189: If the interrupt is within the range of defined interrupts, we handle it
    - Line 181-182: If the interrupt is a peripheral interrupt (PPI), we call the handler
    - Line 186: SGI interrupts are currently ignored
    - Line 186: We acknowledge the interrupt
    - Line 192-196: Interrupts with an ID of 1020 or higher are spurious interrupts.
We do a sanity check that this is indeed a spurious interrupt, and print a message
- Line 208-213: We implement the `GetInterruptSystem()` function.
This will create the singleton instance and initialize it if needed, and return a reference to it

### ARMRegisters.h {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_ARMREGISTERSH}

Next to the Raspberry Pi registers for peripherals etc., and ARM processor specific registers, there are also ARM registers, but specific for Raspberry Pi.
These are not well documented, however there is an [addendum](pdf/bcm2836-additional-info.pdf), see also [ARM local device registers](#RASPBERRY_PI_ARM_LOCAL_DEVICE_REGISTERS).

The ARM specific registers are in a different address range, reaching from 0x40000000 to 0x4003FFFF on Raspberry Pi 3, and 0xFF800000 to 0xFF83FFFF on Raspberry Pi 4 and later.

We'll add the definition for the register we will be using.

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

### Update CMake file {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_UPDATE_CMAKE_FILE}

As we have now added some source files to the `baremetal` library, we need to update its CMake file.

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
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/InterruptHandler.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Interrupts.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Malloc.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
53:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
54:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
55:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
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
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Exception.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ExceptionHandler.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/InterruptHandler.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Interrupts.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Iterator.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Malloc.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
88:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
89:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Synchronization.h
90:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
91:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
92:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
93:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
94:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
95:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
96:     )
97: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_UPDATE_APPLICATION_CODE}

Let's start generating some interrupts. For now, we'll do this all in the `main()` code, we will be moving and generalizing this later.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/ARMRegisters.h>
3: #include <baremetal/Assert.h>
4: #include <baremetal/BCMRegisters.h>
5: #include <baremetal/Console.h>
6: #include <baremetal/InterruptHandler.h>
7: #include <baremetal/Interrupts.h>
8: #include <baremetal/Logger.h>
9: #include <baremetal/MemoryAccess.h>
10: #include <baremetal/System.h>
11: #include <baremetal/Timer.h>
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
32:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
33:         GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) | BIT1(1));
34:     EnableIRQ(IRQ_ID::IRQ_LOCAL_CNTPNS);
35: }
36:
37: void DisableIRQ()
38: {
39:     GetMemoryAccess().Write32(ARM_LOCAL_TIMER_INT_CONTROL0,
40:         GetMemoryAccess().Read32(ARM_LOCAL_TIMER_INT_CONTROL0) & ~BIT1(1));
41:     DisableIRQ(IRQ_ID::IRQ_LOCAL_CNTPNS);
42: }
43:
44: int main()
45: {
46:     auto& console = GetConsole();
47:
48:     auto exceptionLevel = CurrentEL();
49:     LOG_INFO("Current EL: %d", static_cast<int>(exceptionLevel));
50:
51:     uint64 counterFreq{};
52:     GetTimerFrequency(counterFreq);
53:     assert(counterFreq % TICKS_PER_SECOND == 0);
54:     clockTicksPerSystemTick = counterFreq / TICKS_PER_SECOND;
55:     LOG_INFO("Clock ticks per second: %d, clock ticks per interrupt: %d", counterFreq, clockTicksPerSystemTick);
56:
57:     GetInterruptSystem().RegisterIRQHandler(IntHandler, nullptr);
58:
59:     EnableIRQ();
60:
61:     uint64 counter;
62:     GetTimerCounter(counter);
63:     SetTimerCompareValue(counter + clockTicksPerSystemTick);
64:     SetTimerControl(CNTP_CTL_EL0_ENABLE);
65:
66:     LOG_INFO("Wait 5 seconds");
67:     Timer::WaitMilliSeconds(5000);
68:
69:     DisableIRQ();
70:
71:     GetInterruptSystem().UnregisterIRQHandler();
72:
73:     console.Write("Press r to reboot, h to halt\n");
74:     char ch{};
75:     while ((ch != 'r') && (ch != 'h'))
76:     {
77:         ch = console.ReadChar();
78:         console.WriteChar(ch);
79:     }
80:
81:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
82: }
```

- Line 17: We create a definition to set the timer to trigger 2 times a second
- Line 19: We define a variable to hold the number of clock ticks for every timer tick
- Line 21-28: We define a IRQ handler function `IntHandler()`, which reprograms the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and print some text
- Line 30-35: We define a function `EnableIRQ()`, which enables the IRQ for the ARM non secure local timer for core 0
- Line 37-42: We define a function `DisableIRQ()`, which disables the IRQ for the ARM non secure local timer for core 0
- Line 51-54: We calculate from the clock tick frequency how many clock ticks are in a timer tick
- Line 55: We print the calculated number
- Line 57: We register the handler function
- Line 59: We enable the timer IRQ
- Line 61-64: We set the timer to trigger `clockTicksPerSystemTick` clock ticks from now, and enable the interrupt for the timer
- Line 69: We disable the timer IRQ
- Line 71: We unregister the handler function

### Configuring, building and debugging {#TUTORIAL_18_INTERRUPTS_INTERRUPT_HANDLING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

The application will start the timer, and after 5 seconds stop it again. As we set the timer to tick twice a second, we expect to see 10 ticks happening.

On Raspberry Pi 3:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:49)
Info   Clock ticks per second: 62500000, clock ticks per interrupt: 31250000 (main:55)
Info   Wait 5 seconds (main:66)
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
hInfo   Halt (System:129)
```

On Raspberry Pi 4:

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:83)
Info   Starting up (System:206)
Info   Current EL: 1 (main:49)
Info   Clock ticks per second: 62500000, clock ticks per interrupt: 31250000 (main:55)
Info   Wait 5 seconds (main:66)
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
hInfo   Halt (System:129)
```

## Interrupt System - Step 2 {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2}

We'll update the class `InterruptSystem` to enable, disable, and handle interrupts in a more generic way.
We'll also add support for fast interrupts (FIQ).

### InterruptHandler.h {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTHANDLERH}

Update the file `code/libraries/baremetal/include/baremetal/InterruptHandler.h`

```cpp
File: code/libraries/baremetal/include/baremetal/InterruptHandler.h
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
110:
111:     /// @brief Create a interrupt system. Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
112:     InterruptSystem();
113:
114: public:
115:     InterruptSystem(IMemoryAccess& memoryAccess);
116:     ~InterruptSystem();
117:
118:     void Initialize();
119:     void Shutdown();
120:
121:     void DisableInterrupts();
122:     void EnableInterrupts();
123:
124:     void RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param);
125:     void UnregisterIRQHandler(IRQ_ID irqID);
126:
127:     void RegisterFIQHandler(FIQ_ID fiqID, FIQHandler* handler, void* param);
128:     void UnregisterFIQHandler(FIQ_ID fiqID);
129:
130:     void InterruptHandler();
131:
132: private:
133:     bool CallIRQHandler(IRQ_ID irqID);
134: };
135:
136: InterruptSystem& GetInterruptSystem();
137:
138: } // namespace baremetal
139:
140: /// @brief FIQ administration, see Exception.S
141: extern baremetal::FIQData s_fiqData;
```

- Line 64: We add a declaration for the `FIQHandler()` function type, much like the `IRQHandler()` function type
- Line 76-84: We add a declaration for the `FIQData` struct, which mirrors the data defined in `ExceptionStub.S'
- Line 99: We change the member variable `m_irqHandler` to an array of interrupt handlers `m_irqHandlers`
- Line 101: We change the member variable `m_irqHandlerParams` to an array of interrupt handlers parameters `m_irqHandlersParam`
- Line 115: We add a declaration for a specialized constructor taken a `MemoryAccess` reference, meant for testing
- Line 119: We add the method `Shutdown()`, which disables all interrupts, and disables interrupts altogether.
It also clears the initialization flag
- Line 121: We add the method `DisableInterrupts()`, which disables all interrupts to the interrupt controller, e.g. it clears interrupt enables on all IRQ lines, as well as the FIQ
- Line 122: We add the method `EmableInterrupts()`, which enables interrupts to the interrupt controller
- Line 124: We change the method `RegisterIRQHandler()` to also take an IRQ id
- Line 125: We change the method `UnregisterIRQHandler()` to take an IRQ id
- Line 127: We add the method `RegisterFIQHandler()` which registers a FIQ handler for the specified FIQ id. Only one FIQ handler can be active at any time
- Line 128: We add the method `UnregisterFIQHandler()` which unregisters the previously registered FIQ handler. As there can only be one FIQ handler registered at any time, we don't need to specify the FIQ id
- Line 133: We add the method `CallIRQHandler()` which calls the registered IRQ handler for the IRQ with the specified id
- Line 141: We declare the FIQData struct instance s_fiqData, which is defined in `ExceptionStub.S'

### InterruptHandler.cpp {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTHANDLERCPP}

Let's update the `InterruptSystem` class.

Create the file `code/libraries/baremetal/src/InterruptHandler.cpp`

```cpp
File: code/libraries/baremetal/src/InterruptHandler.cpp
...
69: /// <summary>
70: /// Create a interrupt system
71: ///
72: /// Note that the constructor is private, so GetInterruptSystem() is needed to instantiate the interrupt system control
73: /// </summary>
74: InterruptSystem::InterruptSystem()
75:     : m_isInitialized{}
76:     , m_memoryAccess { GetMemoryAccess() }
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
88:     , m_memoryAccess { memoryAccess }
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
115: #if BAREMETAL_RPI_TARGET == 3
116: #else
117:     // direct all interrupts to core 0 with default priority
118:     for (unsigned n = 0; n < IRQ_LINES / 4; n++)
119:     {
120:         m_memoryAccess.Write32(RPI_GICD_IPRIORITYR0 + 4 * n, RPI_GICD_IPRIORITYR_DEFAULT
121:             | RPI_GICD_IPRIORITYR_DEFAULT << 8
122:             | RPI_GICD_IPRIORITYR_DEFAULT << 16
123:             | RPI_GICD_IPRIORITYR_DEFAULT << 24);
124:
125:         m_memoryAccess.Write32(RPI_GICD_ITARGETSR0 + 4 * n, RPI_GICD_ITARGETSR_CORE0
126:             | RPI_GICD_ITARGETSR_CORE0 << 8
127:             | RPI_GICD_ITARGETSR_CORE0 << 16
128:             | RPI_GICD_ITARGETSR_CORE0 << 24);
129:     }
130:
131:     // set all interrupts to level triggered
132:     for (unsigned n = 0; n < IRQ_LINES / 16; n++)
133:     {
134:         m_memoryAccess.Write32(RPI_GICD_ICFGR0 + 4 * n, 0);
135:     }
136:
137:     // initialize core 0 CPU interface:
138:
139:     m_memoryAccess.Write32(RPI_GICC_PMR, RPI_GICC_PMR_PRIORITY);
140:
141:     EnableInterrupts();
142:
143: #endif
144:     m_isInitialized = true;
145: }
146:
147: /// <summary>
148: /// Shutdown interrupt system, disable all
149: /// </summary>
150: void InterruptSystem::Shutdown()
151: {
152:     DisableIRQs();
153:
154:     DisableInterrupts();
155:     m_isInitialized = false;
156: }
157:
158: /// <summary>
159: /// Disable all IRQ interrupts
160: /// </summary>
161: void InterruptSystem::DisableInterrupts()
162: {
163: #if BAREMETAL_RPI_TARGET == 3
164:     m_memoryAccess.Write32(RPI_INTRCTRL_FIQ_CONTROL, 0);
165:
166:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_1, static_cast<uint32>(-1));
167:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_IRQS_2, static_cast<uint32>(-1));
168:     m_memoryAccess.Write32(RPI_INTRCTRL_DISABLE_BASIC_IRQS, static_cast<uint32>(-1));
169:     m_memoryAccess.Write32(ARM_LOCAL_TIMER_INT_CONTROL0, 0);
170: #else
171:     // initialize distributor:
172:
173:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_DISABLE);
174:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_DISABLE);
175:     // disable, acknowledge and deactivate all interrupts
176:     for (unsigned n = 0; n < IRQ_LINES / 32; n++)
177:     {
178:         m_memoryAccess.Write32(RPI_GICD_ICENABLER0 + 4 * n, ~0);
179:         m_memoryAccess.Write32(RPI_GICD_ICPENDR0 + 4 * n, ~0);
180:         m_memoryAccess.Write32(RPI_GICD_ICACTIVER0 + 4 * n, ~0);
181:     }
182: #endif
183: }
184:
185: /// <summary>
186: /// Enable IRQ interrupts
187: /// </summary>
188: void InterruptSystem::EnableInterrupts()
189: {
190: #if BAREMETAL_RPI_TARGET == 3
191: #else
192:     m_memoryAccess.Write32(RPI_GICC_CTLR, RPI_GICC_CTLR_ENABLE);
193:     m_memoryAccess.Write32(RPI_GICD_CTLR, RPI_GICD_CTLR_ENABLE);
194: #endif
195: }
196:
197: /// <summary>
198: /// Enable and register an IRQ handler
199: ///
200: /// Enable the IRQ with specified index, and register its handler.
201: /// </summary>
202: /// <param name="irqID">IRQ ID</param>
203: /// <param name="handler">Handler to register for this IRQ</param>
204: /// <param name="param">Parameter to pass to IRQ handler</param>
205: void InterruptSystem::RegisterIRQHandler(IRQ_ID irqID, IRQHandler* handler, void* param)
206: {
207:     uint32 irq = static_cast<int>(irqID);
208:     assert(irq < IRQ_LINES);
209:     assert(m_irqHandlers[irq] == nullptr);
210:
211:     EnableIRQ(irqID);
212:
213:     m_irqHandlers[irq] = handler;
214:     m_irqHandlersParam[irq] = param;
215: }
216:
217: /// <summary>
218: /// Disable and unregister an IRQ handler
219: ///
220: /// Disable the IRQ with specified index, and unregister its handler.
221: /// </summary>
222: /// <param name="irqID">IRQ ID</param>
223: void InterruptSystem::UnregisterIRQHandler(IRQ_ID irqID)
224: {
225:     uint32 irq = static_cast<int>(irqID);
226:     assert(irq < IRQ_LINES);
227:     assert(m_irqHandlers[irq] != nullptr);
228:
229:     m_irqHandlers[irq] = nullptr;
230:     m_irqHandlersParam[irq] = nullptr;
231:
232:     DisableIRQ(irqID);
233: }
234:
235: /// <summary>
236: /// Enable and register a FIQ interrupt handler. Only one can be enabled at any time.
237: /// </summary>
238: /// <param name="fiqID">FIQ interrupt number</param>
239: /// <param name="handler">FIQ interrupt handler</param>
240: /// <param name="param">FIQ interrupt data</param>
241: // cppcheck-suppress unusedFunction
242: void InterruptSystem::RegisterFIQHandler(FIQ_ID fiqID, FIQHandler *handler, void *param)
243: {
244:     uint32 fiq = static_cast<int>(fiqID);
245:     assert(fiq <= IRQ_LINES);
246:     assert(handler != nullptr);
247:     assert(s_fiqData.handler == nullptr);
248:
249:     s_fiqData.handler = handler;
250:     s_fiqData.param   = param;
251:     s_fiqData.fiqID   = fiq;
252:
253:     EnableFIQ(fiqID);
254: }
255:
256: /// <summary>
257: /// Disable and unregister a FIQ interrupt handler
258: /// </summary>
259: void InterruptSystem::UnregisterFIQHandler(FIQ_ID fiqID)
260: {
261:     uint32 fiq = static_cast<int>(fiqID);
262:     assert(s_fiqData.handler != nullptr);
263:     assert(s_fiqData.fiqID == fiq);
264:     DisableFIQ(fiqID);
265:
266:     s_fiqData.handler = nullptr;
267:     s_fiqData.param   = nullptr;
268: }
269:
270: /// <summary>
271: /// Handles an interrupt.
272: ///
273: /// The interrupt handler is called from assembly code (ExceptionStub.S)
274: /// </summary>
275: void InterruptSystem::InterruptHandler()
276: {
277: #if BAREMETAL_RPI_TARGET == 3
278:     uint32 localpendingIRQs = m_memoryAccess.Read32(ARM_LOCAL_IRQ_PENDING0);
279:     if (localpendingIRQs & ARM_LOCAL_INTSRC_TIMER1) // the only implemented local IRQ so far
280:     {
281:         CallIRQHandler(IRQ_ID::IRQ_LOCAL_CNTPNS);
282:
283:         return;
284:     }
285:
286:     uint32 pendingIRQs[ARM_IRQS_NUM_REGS];
287:     pendingIRQs[0] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_1);
288:     pendingIRQs[1] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_PENDING_2);
289:     pendingIRQs[2] = m_memoryAccess.Read32(RPI_INTRCTRL_IRQ_BASIC_PENDING) & 0xFF; // Only 8 basic interrupts
290:
291:     for (unsigned reg = 0; reg < ARM_IRQS_NUM_REGS; reg++)
292:     {
293:         uint32 pendingIRQ = pendingIRQs[reg];
294:         if (pendingIRQ != 0)
295:         {
296:             unsigned irqID = reg * ARM_IRQS_PER_REG;
297:
298:             do
299:             {
300:                 if ((pendingIRQ & 1) && CallIRQHandler(static_cast<IRQ_ID>(irqID)))
301:                 {
302:                     return;
303:                 }
304:
305:                 pendingIRQ >>= 1;
306:                 irqID++;
307:             } while (pendingIRQ != 0);
308:         }
309:     }
310:
311: #else
312:
313:     uint32 iarValue = m_memoryAccess.Read32(RPI_GICC_IAR);      // Read Interrupt Acknowledge Register
314:
315:     uint32 irq = iarValue & RPI_GICC_IAR_INTERRUPT_ID_MASK;   // Select the currently active interrupt
316:     if (irq < IRQ_LINES)
317:     {
318:         if (irq >= GIC_PPI(0))
319:         {
320:             // Peripheral interrupts (PPI and SPI)
321:             CallIRQHandler(static_cast<IRQ_ID>(irq));
322:         }
323:         else
324:         {
325:             // Handle SGI interrupt
326:         }
327:         m_memoryAccess.Write32(RPI_GICC_EOIR, iarValue);        // Flag end of interrupt
328:     }
329: #ifndef NDEBUG
330:     else
331:     {
332:         // spurious interrupts
333:         assert(irq >= 1020);
334:         LOG_INFO("Received spurious interrupt %d", iarValue);
335:     }
336: #endif
337:
338: #endif
339: }
340:
341: /// <summary>
342: /// Call the IRQ handler for the specified IRQ ID
343: /// </summary>
344: /// <param name="irqID">ID of the IRQ</param>
345: /// <returns>True if a IRQ handler was found, false if not</returns>
346: bool InterruptSystem::CallIRQHandler(IRQ_ID irqID)
347: {
348:     uint32 irq = static_cast<int>(irqID);
349:     assert(irq < IRQ_LINES);
350:     IRQHandler* handler = m_irqHandlers[irq];
351:
352:     if (handler != nullptr)
353:     {
354:         (*handler)(m_irqHandlersParam[irq]);
355:
356:         return true;
357:     }
358: #ifndef NDEBUG
359:     LOG_INFO("Unhandled interrupt %d", irq);
360: #endif
361:
362:     DisableIRQ(irqID);
363:
364:     return false;
365: }
366:
367: /// <summary>
368: /// Construct the singleton interrupt system instance if needed, initialize it, and return a reference to the instance
369: ///
370: /// This is a friend function of class InterruptSystem
371: /// </summary>
372: /// <returns>Reference to the singleton interrupt system instance</returns>
373: InterruptSystem& baremetal::GetInterruptSystem()
374: {
375:     static InterruptSystem singleton;
376:     singleton.Initialize();
377:     return singleton;
378: }
```

- Line 74-80: We update the default constructor to initialize the new member variables
- Line 86-92: We implement the specialized constructor. The only difference is the `IMemoryAccess` instance
- Line 96-99: We update the destructor to call `Shutdown()`
- Line 104-145: We update the `Initialize()` method set up the administration for IRQs, disable all interrupt lines, but enable the IRQs system wide
  - Line 106-107: We check whether the system is already initialized, and return if so
  - Line 109-110: We zero out the vectors with interrupt handlers and their parameters
  - Line 112: We call `DisableInterrupts()` to disable all interrupts
  - Line 115: For Raspberry Pi 3, there is nothing lef to do
  - Line 117-141: For Raspberry Pi 4 / 5 we still need some initialization, like in the previous version.
Notice that some initialization is moved to `DisableInterrupts()`, and we use `EnableInterrupts()` to re-enable the GIC distributor and controller
- Line 150-156: We implement the `Shutdown()` method
  - Line 152: We disable all interrupts at system level
  - Line 154: We disable all interrupts at interrupt controller level
  - Line 155: We set the initialization flag to false
- Line 161-183: We implement the `DisableInterrupts()` method
  - Line 164-169: For Raspberry Pi 3, we disable the FIQ, and reset all interrupt enable bits for the IRQ1, IRQ2, Basic IRQ, and ARM local interrupts
  - Line 173-181: We disable the GIC distributor and controller, and reset all interrupt enable bits for interrupts
- Line 188-195: We implement the `EnableInterrupts()` method
  - Line 192-193: We enable the GIC distributor and controller
- Line 205-215: We update the `RegisterIRQHandler()` method to administer the registered IRQ handler, and enable its IRQ line
- Line 223-233: We update the `UnregisterIRQHandler()` method to remove the registered IRQ handler, and disable its IRQ line
- Line 242-254: We implement the `RegisterFIQHandler()` method to administer the registered FIQ handler, and enable it
- Line 259-268: We implement the `UnregisterIRQHandler()` method to remove the registered FIQ handler, and disable it
- Line 275-339: We update the `InterruptHandler()` method
  - Line 278-309: For Raspberry Pi 3
    - Line 278-284: We check whether the ARM local timer fired, and call its interrupt handler if so
    - Line 286-289: We read all interrupt pending bits for the IRQ1, IRQ2 and Basic IRQ groups
    - Line 291-309: We scan through all pending bits, if one is set, we call its interrupt handler and return.
So at most one interrupt handler is called
  - Line 313-336: For Raspberry Pi 4 / 5
    - Line 315: We read the IAR register to find which interrupt we need to handle
    - Line 316-327: If the interrupt id is within limits, we call its interrupt handler, and acknowledge the interrupt
    - Line 331-335: If not, and debug mode is on, we check that this is a spurious interrupt, and print a message with the interrupt id
- Line 346-365: We implement the `CallIRQHandler()` method
  - Line 348-349: We perform a sanity check that the interrupt is within limits
  - Line 350: We retrieve the interrupt handler pointer
  - Line 352-357: If the interrupt handler is installed, we call it and return true
  - Line 359: If no handler is installed and debug mode is on, we print a message
  - Line 362-364: As at this point the interrupt is not handled, we disable the interrupt for the future and return false

### Interrupt.h {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTH}

We'll add definitions for all IRQ and FIQ interrupts.

Update the file `code/libraries/baremetal/include/baremetal/Interrupts.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Interrupts.h
46: #if BAREMETAL_RPI_TARGET == 3
47:
48: // IRQs
49: /// @brief Number of IRQ lines per register for regular ARM_IRQ1 and ARM_IRQ2
50: #define ARM_IRQS_PER_REG            32
51: /// @brief Number of IRQ lines for ARM_IRQ_BASIC
52: #define ARM_IRQS_BASIC_REG          8
53: /// @brief Number of IRQ lines for ARM_IRQ_LOCAL
54: #define ARM_IRQS_LOCAL_REG          12
55:
56: /// @brief Number of normal IRQ registers (IRQ1, IRQ2, Basic IRQ)
57: #define ARM_IRQS_NUM_REGS 3
58:
59: /// @brief Total number of IRQ lines Raspberry Pi 3
60: #define IRQ_LINES                   (ARM_IRQS_PER_REG * 2 + ARM_IRQS_BASIC_REG + ARM_IRQS_LOCAL_REG)
61:
62: /// @brief Offset for interrupts in IRQ1 group
63: #define ARM_IRQ1_BASE               0
64: /// @brief Offset for interrupts in IRQ2 group
65: #define ARM_IRQ2_BASE               (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
66: /// @brief Offset for interrupts in Basic IRQ group
67: #define ARM_IRQ_BASIC_BASE          (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
68: /// @brief Offset for interrupts in ARM local group
69: #define ARM_IRQ_LOCAL_BASE          (ARM_IRQ_BASIC_BASE + ARM_IRQS_BASIC_REG)
70: /// @brief Calculate IRQ number for IRQ in IRQ1 group
71: #define ARM_IRQ1(n)                 (ARM_IRQ1_BASE + (n))
72: /// @brief Calculate IRQ number for IRQ in IRQ2 group
73: #define ARM_IRQ2(n)                 (ARM_IRQ2_BASE + (n))
74: /// @brief Calculate IRQ number for IRQ in Basic IRQ group
75: #define ARM_IRQ_BASIC(n)             (ARM_IRQ_BASIC_BASE + (n))
76: /// @brief Calculate IRQ number for IRQ in ARM local group
77: #define ARM_IRQ_LOCAL(n)            (ARM_IRQ_LOCAL_BASE + (n))
78:
79: /// @brief Check whether an IRQ is pending (only for IRQ1, IRQ2, Basic IRQ group)
80: #define ARM_IC_IRQ_PENDING(irq)     ((irq) < ARM_IRQ2_BASE              \
81:                                     ? RPI_INTRCTRL_IRQ_PENDING_1        \
82:                                     : ((irq) < ARM_IRQ_BASIC_BASE        \
83:                                         ? RPI_INTRCTRL_IRQ_PENDING_2    \
84:                                         : RPI_INTRCTRL_IRQ_BASIC_PENDING))
85: /// @brief Enable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
86: #define ARM_IC_IRQS_ENABLE(irq)     ((irq) < ARM_IRQ2_BASE              \
87:                                     ? RPI_INTRCTRL_ENABLE_IRQS_1        \
88:                                     : ((irq) < ARM_IRQ_BASIC_BASE        \
89:                                     ? RPI_INTRCTRL_ENABLE_IRQS_2        \
90:                                     : RPI_INTRCTRL_ENABLE_BASIC_IRQS))
91: /// @brief Disable an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
92: #define ARM_IC_IRQS_DISABLE(irq)    ((irq) < ARM_IRQ2_BASE              \
93:                                     ? RPI_INTRCTRL_DISABLE_IRQS_1       \
94:                                     : ((irq) < ARM_IRQ_BASIC_BASE        \
95:                                     ? RPI_INTRCTRL_DISABLE_IRQS_2       \
96:                                     : RPI_INTRCTRL_DISABLE_BASIC_IRQS))
97: /// @brief Calculate bitmask for an IRQ (only for IRQ1, IRQ2, Basic IRQ group)
98: #define ARM_IRQ_MASK(irq)           BIT1((irq) & (ARM_IRQS_PER_REG-1))
99:
100: /// <summary>
101: /// @brief IRQ interrupt numbers
102: /// </summary>
103: enum class IRQ_ID
104: {
105:     /// @brief BCM timer 0
106:     IRQ_TIMER0                      = ARM_IRQ1(0x00),       // bcm2835-system-timer
107:     /// @brief BCM timer 1
108:     IRQ_TIMER1                      = ARM_IRQ1(0x01),
109:     /// @brief BCM timer 2
110:     IRQ_TIMER2                      = ARM_IRQ1(0x02),
111:     /// @brief BCM timer 3
112:     IRQ_TIMER3                      = ARM_IRQ1(0x03),
113:     /// @brief H.264 codec 0
114:     IRQ_CODEC0                      = ARM_IRQ1(0x04),
115:     /// @brief H.264 codec 1
116:     IRQ_CODEC1                      = ARM_IRQ1(0x05),
117:     /// @brief H.264 codec 2
118:     IRQ_CODEC2                      = ARM_IRQ1(0x06),
119:     /// @brief JPEG
120:     IRQ_JPEG                        = ARM_IRQ1(0x07),
121:     /// @brief ISP
122:     IRQ_ISP                         = ARM_IRQ1(0x08),
123:     /// @brief USB
124:     IRQ_USB                         = ARM_IRQ1(0x09),       // bcm2708-usb
125:     /// @brief VideoCore 3D
126:     IRQ_3D                          = ARM_IRQ1(0x0A),       // vc4-v3d
127:     /// @brief TXP / Transpose
128:     IRQ_TRANSPOSER                  = ARM_IRQ1(0x0B),       // bcm2835-txp
129:     /// @brief Multicore sync 0
130:     IRQ_MULTICORESYNC0              = ARM_IRQ1(0x0C),
131:     /// @brief Multicore sync 1
132:     IRQ_MULTICORESYNC1              = ARM_IRQ1(0x0D),
133:     /// @brief Multicore sync 2
134:     IRQ_MULTICORESYNC2              = ARM_IRQ1(0x0E),
135:     /// @brief Multicore sync 3
136:     IRQ_MULTICORESYNC3              = ARM_IRQ1(0x0F),
137:     /// @brief DMA channel 0 interrupt
138:     IRQ_DMA0                        = ARM_IRQ1(0x10),
139:     /// @brief DMA channel 1 interrupt
140:     IRQ_DMA1                        = ARM_IRQ1(0x11),
141:     /// @brief DMA channel 2 interrupt, I2S PCM TX
142:     IRQ_DMA2                        = ARM_IRQ1(0x12),
143:     /// @brief DMA channel 3 interrupt, I2S PCM RX
144:     IRQ_DMA3                        = ARM_IRQ1(0x13),
145:     /// @brief DMA channel 4 interrupt, SMI
146:     IRQ_DMA4                        = ARM_IRQ1(0x14),
147:     /// @brief DMA channel 5 interrupt, PWM
148:     IRQ_DMA5                        = ARM_IRQ1(0x15),
149:     /// @brief DMA channel 6 interrupt, SPI TX
150:     IRQ_DMA6                        = ARM_IRQ1(0x16),
151:     /// @brief DMA channel 7 interrupt, SPI RX
152:     IRQ_DMA7                        = ARM_IRQ1(0x17),
153:     /// @brief DMA channel 8 interrupt
154:     IRQ_DMA8                        = ARM_IRQ1(0x18),
155:     /// @brief DMA channel 9 interrupt
156:     IRQ_DMA9                        = ARM_IRQ1(0x19),
157:     /// @brief DMA channel 10 interrupt
158:     IRQ_DMA10                       = ARM_IRQ1(0x1A),
159:     /// @brief DMA channel 11-14 interrupt, EMMC
160:     IRQ_DMA11                       = ARM_IRQ1(0x1B),
161:     /// @brief DMA channel 11-14 interrupt, UART TX
162:     IRQ_DMA12                       = ARM_IRQ1(0x1B),
163:     /// @brief DMA channel 11-14 interrupt, undocumented
164:     IRQ_DMA13                       = ARM_IRQ1(0x1B),
165:     /// @brief DMA channel 11-14 interrupt, UART RX
166:     IRQ_DMA14                       = ARM_IRQ1(0x1B),
167:     /// @brief DMA channel shared interrupt
168:     IRQ_DMA_SHARED                  = ARM_IRQ1(0x1C),
169:     /// @brief AUX interrupt (UART1, SPI0/1)
170:     IRQ_AUX                         = ARM_IRQ1(0x1D),
171:     /// @brief ARM interrupt
172:     IRQ_ARM                         = ARM_IRQ1(0x1E),
173:     /// @brief VPU interrupt
174:     IRQ_VPUDMA                      = ARM_IRQ1(0x1F),
175:
176:     /// @brief USB Host Port interrupt
177:     IRQ_HOSTPORT                    = ARM_IRQ2(0x00),
178:     /// @brief JPEHVS interruptG
179:     IRQ_VIDEOSCALER                 = ARM_IRQ2(0x01),
180:     /// @brief CCP2TX interrupt
181:     IRQ_CCP2TX                      = ARM_IRQ2(0x02),
182:     /// @brief SDC interrupt
183:     IRQ_SDC                         = ARM_IRQ2(0x03),
184:     /// @brief DSI 0 (display) interrupt
185:     IRQ_DSI0                        = ARM_IRQ2(0x04),
186:     /// @brief AVE interrupt
187:     IRQ_AVE                         = ARM_IRQ2(0x05),
188:     /// @brief CSI 0 (camera) interrupt
189:     IRQ_CAM0                        = ARM_IRQ2(0x06),
190:     /// @brief CSI 1 (camera) interrupt
191:     IRQ_CAM1                        = ARM_IRQ2(0x07),
192:     /// @brief HDMI 0 interrupt
193:     IRQ_HDMI0                       = ARM_IRQ2(0x08),
194:     /// @brief HDMI 1 interrupt
195:     IRQ_HDMI1                       = ARM_IRQ2(0x09),
196:     /// @brief GPU pixel valve 2 interrupt
197:     IRQ_PIXELVALVE2                 = ARM_IRQ2(0x0A),
198:     /// @brief I2C / SPI slave interrupt
199:     IRQ_I2CSPISLV                   = ARM_IRQ2(0x0B),
200:     /// @brief DSI 1 (display) interrupt
201:     IRQ_DSI1                        = ARM_IRQ2(0x0C),
202:     /// @brief GPU pixel valve 0 interrupt
203:     IRQ_PIXELVALVE0                 = ARM_IRQ2(0x0D),
204:     /// @brief GPU pixel valve 1 interrupt
205:     IRQ_PIXELVALVE1                 = ARM_IRQ2(0x0E),
206:     /// @brief CPR interrupt
207:     IRQ_CPR                         = ARM_IRQ2(0x0F),
208:     /// @brief SMI (firmware) interrupt
209:     IRQ_SMI                         = ARM_IRQ2(0x10),
210:     /// @brief GPIO 0 interrupt
211:     IRQ_GPIO0                       = ARM_IRQ2(0x11),
212:     /// @brief GPIO 1 interrupt
213:     IRQ_GPIO1                       = ARM_IRQ2(0x12),
214:     /// @brief GPIO 2 interrupt
215:     IRQ_GPIO2                       = ARM_IRQ2(0x13),
216:     /// @brief GPIO 3 interrupt
217:     IRQ_GPIO3                       = ARM_IRQ2(0x14),
218:     /// @brief I2C interrupt
219:     IRQ_I2C                         = ARM_IRQ2(0x15),
220:     /// @brief SPI interrupt
221:     IRQ_SPI                         = ARM_IRQ2(0x16),
222:     /// @brief I2S interrupt
223:     IRQ_I2SPCM                      = ARM_IRQ2(0x17),
224:     /// @brief SD host interrupt
225:     IRQ_SDHOST                      = ARM_IRQ2(0x18),
226:     /// @brief PL011 UART interrupt (UART0)
227:     IRQ_UART                        = ARM_IRQ2(0x19),
228:     /// @brief SLIMBUS interrupt
229:     IRQ_SLIMBUS                     = ARM_IRQ2(0x1A),
230:     /// @brief GPU? vector interrupt
231:     IRQ_VEC                         = ARM_IRQ2(0x1B),
232:     /// @brief CPG interrupt
233:     IRQ_CPG                         = ARM_IRQ2(0x1C),
234:     /// @brief RNG (random number generator) interrupt
235:     IRQ_RNG                         = ARM_IRQ2(0x1D),
236:     /// @brief EMMC interrupt
237:     IRQ_ARASANSDIO                  = ARM_IRQ2(0x1E),
238:     /// @brief AVSPMON interrupt
239:     IRQ_AVSPMON                     = ARM_IRQ2(0x1F),
240:
241:     /// @brief ARM timer interrupt
242:     IRQ_ARM_TIMER                   = ARM_IRQ_BASIC(0x00),
243:     /// @brief ARM mailbox interrupt
244:     IRQ_ARM_MAILBOX                 = ARM_IRQ_BASIC(0x01),
245:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
246:     IRQ_ARM_DOORBELL_0              = ARM_IRQ_BASIC(0x02),
247:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
248:     IRQ_ARM_DOORBELL_1              = ARM_IRQ_BASIC(0x03),
249:     /// @brief VPU halted 0 interrupt
250:     IRQ_VPU0_HALTED                 = ARM_IRQ_BASIC(0x04),
251:     /// @brief VPU halted 1 interrupt
252:     IRQ_VPU1_HALTED                 = ARM_IRQ_BASIC(0x05),
253:     /// @brief Illegal type 0 interrupt
254:     IRQ_ILLEGAL_TYPE0               = ARM_IRQ_BASIC(0x06),
255:     /// @brief Illegal type 1 interrupt
256:     IRQ_ILLEGAL_TYPE1               = ARM_IRQ_BASIC(0x07),
257:
258:     /// @brief Non secure Physical ARM timer
259:     IRQ_LOCAL_CNTPS                 = ARM_IRQ_LOCAL(0x00), // armv7-timer
260:     /// @brief Secure Physical ARM timer
261:     IRQ_LOCAL_CNTPNS                = ARM_IRQ_LOCAL(0x01),
262:     /// @brief Hypervisor Physical ARM timer
263:     IRQ_LOCAL_CNTHP                 = ARM_IRQ_LOCAL(0x02),
264:     /// @brief Virtual ARM timer
265:     IRQ_LOCAL_CNTV                  = ARM_IRQ_LOCAL(0x03),
266:     /// @brief Mailbox 0
267:     IRQ_LOCAL_MAILBOX0              = ARM_IRQ_LOCAL(0x04),
268:     /// @brief Mailbox 1
269:     IRQ_LOCAL_MAILBOX1              = ARM_IRQ_LOCAL(0x05),
270:     /// @brief Mailbox 2
271:     IRQ_LOCAL_MAILBOX2              = ARM_IRQ_LOCAL(0x06),
272:     /// @brief Mailbox 3
273:     IRQ_LOCAL_MAILBOX3              = ARM_IRQ_LOCAL(0x07),
274:     /// @brief Cascaded GPU interrupts
275:     IRQ_LOCAL_GPU                   = ARM_IRQ_LOCAL(0x08),
276:     /// @brief Performance Monitoring Unit
277:     IRQ_LOCAL_PMU                   = ARM_IRQ_LOCAL(0x09),
278:     /// @brief AXI bus idle, on core 0 only
279:     IRQ_LOCAL_AXI_IDLE              = ARM_IRQ_LOCAL(0x0A),
280:     /// @brief Local timer
281:     IRQ_LOCAL_LOCALTIMER            = ARM_IRQ_LOCAL(0x0B),
282: };
283:
284: /// <summary>
285: /// @brief FIQ interrupt numbers
286: /// </summary>
287: enum class FIQ_ID
288: {
289:     /// @brief BCM timer 0
290:     FIQ_TIMER0                      = ARM_IRQ1(0x00),
291:     /// @brief BCM timer 1
292:     FIQ_TIMER1                      = ARM_IRQ1(0x01),
293:     /// @brief BCM timer 2
294:     FIQ_TIMER2                      = ARM_IRQ1(0x02),
295:     /// @brief BCM timer 3
296:     FIQ_TIMER3                      = ARM_IRQ1(0x03),
297:     /// @brief H.264 codec 0
298:     FIQ_CODEC0                      = ARM_IRQ1(0x04),
299:     /// @brief H.264 codec 1
300:     FIQ_CODEC1                      = ARM_IRQ1(0x05),
301:     /// @brief H.264 codec 2
302:     FIQ_CODEC2                      = ARM_IRQ1(0x06),
303:     /// @brief JPEG
304:     FIQ_JPEG                        = ARM_IRQ1(0x07),
305:     /// @brief ISP
306:     FIQ_ISP                         = ARM_IRQ1(0x08),
307:     /// @brief USB
308:     FIQ_USB                         = ARM_IRQ1(0x09),
309:     /// @brief VideoCore 3D
310:     FIQ_3D                          = ARM_IRQ1(0x0A),
311:     /// @brief TXP / Transpose
312:     FIQ_TRANSPOSER                  = ARM_IRQ1(0x0B),
313:     /// @brief Multicore sync 0
314:     FIQ_MULTICORESYNC0              = ARM_IRQ1(0x0C),
315:     /// @brief Multicore sync 1
316:     FIQ_MULTICORESYNC1              = ARM_IRQ1(0x0D),
317:     /// @brief Multicore sync 2
318:     FIQ_MULTICORESYNC2              = ARM_IRQ1(0x0E),
319:     /// @brief Multicore sync 3
320:     FIQ_MULTICORESYNC3              = ARM_IRQ1(0x0F),
321:     /// @brief DMA channel 0 interrupt
322:     FIQ_DMA0                        = ARM_IRQ1(0x10),
323:     /// @brief DMA channel 1 interrupt
324:     FIQ_DMA1                        = ARM_IRQ1(0x11),
325:     /// @brief DMA channel 2 interrupt, I2S PCM TX
326:     FIQ_DMA2                        = ARM_IRQ1(0x12),
327:     /// @brief DMA channel 3 interrupt, I2S PCM RX
328:     FIQ_DMA3                        = ARM_IRQ1(0x13),
329:     /// @brief DMA channel 4 interrupt, SMI
330:     FIQ_DMA4                        = ARM_IRQ1(0x14),
331:     /// @brief DMA channel 5 interrupt, PWM
332:     FIQ_DMA5                        = ARM_IRQ1(0x15),
333:     /// @brief DMA channel 6 interrupt, SPI TX
334:     FIQ_DMA6                        = ARM_IRQ1(0x16),
335:     /// @brief DMA channel 7 interrupt, SPI RX
336:     FIQ_DMA7                        = ARM_IRQ1(0x17),
337:     /// @brief DMA channel 8 interrupt
338:     FIQ_DMA8                        = ARM_IRQ1(0x18),
339:     /// @brief DMA channel 9 interrupt
340:     FIQ_DMA9                        = ARM_IRQ1(0x19),
341:     /// @brief DMA channel 10 interrupt
342:     FIQ_DMA10                       = ARM_IRQ1(0x1A),
343:     /// @brief DMA channel 11-14 interrupt, EMMC
344:     FIQ_DMA11                       = ARM_IRQ1(0x1B),
345:     /// @brief DMA channel 11-14 interrupt, UART TX
346:     FIQ_DMA12                       = ARM_IRQ1(0x1B),
347:     /// @brief DMA channel 11-14 interrupt, undocumented
348:     FIQ_DMA13                       = ARM_IRQ1(0x1B),
349:     /// @brief DMA channel 11-14 interrupt, UART RX
350:     FIQ_DMA14                       = ARM_IRQ1(0x1B),
351:     /// @brief DMA channel shared interrupt
352:     FIQ_DMA_SHARED                  = ARM_IRQ1(0x1C),
353:     /// @brief AUX interrupt (UART1, SPI0/1)
354:     FIQ_AUX                         = ARM_IRQ1(0x1D),
355:     /// @brief ARM interrupt
356:     FIQ_ARM                         = ARM_IRQ1(0x1E),
357:     /// @brief VPU interrupt
358:     FIQ_VPUDMA                      = ARM_IRQ1(0x1F),
359:
360:     /// @brief USB Host Port interrupt
361:     FIQ_HOSTPORT                    = ARM_IRQ2(0x00),
362:     /// @brief JPEHVS interruptG
363:     FIQ_VIDEOSCALER                 = ARM_IRQ2(0x01),
364:     /// @brief CCP2TX interrupt
365:     FIQ_CCP2TX                      = ARM_IRQ2(0x02),
366:     /// @brief SDC interrupt
367:     FIQ_SDC                         = ARM_IRQ2(0x03),
368:     /// @brief DSI 0 (display) interrupt
369:     FIQ_DSI0                        = ARM_IRQ2(0x04),
370:     /// @brief AVE interrupt
371:     FIQ_AVE                         = ARM_IRQ2(0x05),
372:     /// @brief CSI 0 (camera) interrupt
373:     FIQ_CAM0                        = ARM_IRQ2(0x06),
374:     /// @brief CSI 1 (camera) interrupt
375:     FIQ_CAM1                        = ARM_IRQ2(0x07),
376:     /// @brief HDMI 0 interrupt
377:     FIQ_HDMI0                       = ARM_IRQ2(0x08),
378:     /// @brief HDMI 1 interrupt
379:     FIQ_HDMI1                       = ARM_IRQ2(0x09),
380:     /// @brief GPU pixel valve 2 interrupt
381:     FIQ_PIXELVALVE2                 = ARM_IRQ2(0x0A),
382:     /// @brief I2C / SPI slave interrupt
383:     FIQ_I2CSPISLV                   = ARM_IRQ2(0x0B),
384:     /// @brief DSI 1 (display) interrupt
385:     FIQ_DSI1                        = ARM_IRQ2(0x0C),
386:     /// @brief GPU pixel valve 0 interrupt
387:     FIQ_PIXELVALVE0                 = ARM_IRQ2(0x0D),
388:     /// @brief GPU pixel valve 1 interrupt
389:     FIQ_PIXELVALVE1                 = ARM_IRQ2(0x0E),
390:     /// @brief CPR interrupt
391:     FIQ_CPR                         = ARM_IRQ2(0x0F),
392:     /// @brief SMI (firmware) interrupt
393:     FIQ_SMI                         = ARM_IRQ2(0x10),
394:     /// @brief GPIO 0 interrupt
395:     FIQ_GPIO0                       = ARM_IRQ2(0x11),
396:     /// @brief GPIO 1 interrupt
397:     FIQ_GPIO1                       = ARM_IRQ2(0x12),
398:     /// @brief GPIO 2 interrupt
399:     FIQ_GPIO2                       = ARM_IRQ2(0x13),
400:     /// @brief GPIO 3 interrupt
401:     FIQ_GPIO3                       = ARM_IRQ2(0x14),
402:     /// @brief I2C interrupt
403:     FIQ_I2C                         = ARM_IRQ2(0x15),
404:     /// @brief SPI interrupt
405:     FIQ_SPI                         = ARM_IRQ2(0x16),
406:     /// @brief I2S interrupt
407:     FIQ_I2SPCM                      = ARM_IRQ2(0x17),
408:     /// @brief SD host interrupt
409:     FIQ_SDHOST                      = ARM_IRQ2(0x18),
410:     /// @brief PL011 UART interrupt (UART0)
411:     FIQ_UART                        = ARM_IRQ2(0x19),
412:     /// @brief SLIMBUS interrupt
413:     FIQ_SLIMBUS                     = ARM_IRQ2(0x1A),
414:     /// @brief GPU? vector interrupt
415:     FIQ_VEC                         = ARM_IRQ2(0x1B),
416:     /// @brief CPG interrupt
417:     FIQ_CPG                         = ARM_IRQ2(0x1C),
418:     /// @brief RNG (random number generator) interrupt
419:     FIQ_RNG                         = ARM_IRQ2(0x1D),
420:     /// @brief EMMC interrupt
421:     FIQ_ARASANSDIO                  = ARM_IRQ2(0x1E),
422:     /// @brief AVSPMON interrupt
423:     FIQ_AVSPMON                     = ARM_IRQ2(0x1F),
424:
425:     /// @brief ARM timer interrupt
426:     FIQ_ARM_TIMER                   = ARM_IRQ_BASIC(0x00),
427:     /// @brief ARM mailbox interrupt
428:     FIQ_ARM_MAILBOX                 = ARM_IRQ_BASIC(0x01),
429:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
430:     FIQ_ARM_DOORBELL_0              = ARM_IRQ_BASIC(0x02),
431:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
432:     FIQ_ARM_DOORBELL_1              = ARM_IRQ_BASIC(0x03),
433:     /// @brief VPU halted 0 interrupt
434:     FIQ_VPU0_HALTED                 = ARM_IRQ_BASIC(0x04),
435:     /// @brief VPU halted 1 interrupt
436:     FIQ_VPU1_HALTED                 = ARM_IRQ_BASIC(0x05),
437:     /// @brief Illegal type 0 interrupt
438:     FIQ_ILLEGAL_TYPE0               = ARM_IRQ_BASIC(0x06),
439:     /// @brief Illegal type 1 interrupt
440:     FIQ_ILLEGAL_TYPE1               = ARM_IRQ_BASIC(0x07),
441:
442:     /// @brief Secure Physical ARM timer
443:     FIQ_LOCAL_CNTPS                 = ARM_IRQ_LOCAL(0x00),
444:     /// @brief Non secure Physical ARM timer
445:     FIQ_LOCAL_CNTPNS                = ARM_IRQ_LOCAL(0x01),
446:     /// @brief Hypervisor Physical ARM timer
447:     FIQ_LOCAL_CNTHP                 = ARM_IRQ_LOCAL(0x02),
448:     /// @brief Virtual ARM timer
449:     FIQ_LOCAL_CNTV                  = ARM_IRQ_LOCAL(0x03),
450:     /// @brief Mailbox 0
451:     FIQ_LOCAL_MAILBOX0              = ARM_IRQ_LOCAL(0x04),
452:     /// @brief Mailbox 1
453:     FIQ_LOCAL_MAILBOX1              = ARM_IRQ_LOCAL(0x05),
454:     /// @brief Mailbox 2
455:     FIQ_LOCAL_MAILBOX2              = ARM_IRQ_LOCAL(0x06),
456:     /// @brief Mailbox 3
457:     FIQ_LOCAL_MAILBOX3              = ARM_IRQ_LOCAL(0x07),
458:     /// @brief Cascaded GPU interrupts
459:     FIQ_LOCAL_GPU                   = ARM_IRQ_LOCAL(0x08),
460:     /// @brief Performance Monitoring Unit
461:     FIQ_LOCAL_PMU                   = ARM_IRQ_LOCAL(0x09),
462:     /// @brief AXI bus idle, on core 0 only
463:     FIQ_LOCAL_AXI_IDLE              = ARM_IRQ_LOCAL(0x0A),
464:     /// @brief Local timer
465:     FIQ_LOCAL_LOCALTIMER            = ARM_IRQ_LOCAL(0x0B),
466: };
467:
468: #else
469:
470: // IRQs
471: /// @brief Software generated interrupt, per core
472: #define GIC_SGI(n)          (0  + (n))
473: /// @brief Private peripheral interrupt, per core
474: #define GIC_PPI(n)          (16 + (n))
475: /// @brief Shared peripheral interrupt, shared between cores
476: #define GIC_SPI(n)          (32 + (n))
477:
478: /// @brief Total number of IRQ lines Raspberry Pi 4 and 5
479: #define IRQ_LINES           256
480:
481: /// <summary>
482: /// @brief IRQ interrupt numbers
483: /// </summary>
484: enum class IRQ_ID
485: {
486:     /// @brief Hypervisor Physical ARM timer
487:     IRQ_LOCAL_CNTHP         = GIC_PPI(0x0A),
488:     /// @brief Virtual ARM timer
489:     IRQ_LOCAL_CNTV          = GIC_PPI(0x0B),
490:     /// @brief Secure Physical ARM timer
491:     IRQ_LOCAL_CNTPS         = GIC_PPI(0x0D),
492:     /// @brief Non secure Physical ARM timer
493:     IRQ_LOCAL_CNTPNS        = GIC_PPI(0x0E),
494:
495:     /// @brief ARM core mailbox 0, core 0
496:     IRQ_LOCAL_MAILBOX_0_0   = GIC_SPI(0x00),
497:     /// @brief ARM core mailbox 1, core 0
498:     IRQ_LOCAL_MAILBOX_1_0   = GIC_SPI(0x01),
499:     /// @brief ARM core mailbox 2, core 0
500:     IRQ_LOCAL_MAILBOX_2_0   = GIC_SPI(0x02),
501:     /// @brief ARM core mailbox 3, core 0
502:     IRQ_LOCAL_MAILBOX_3_0   = GIC_SPI(0x03),
503:     /// @brief ARM core mailbox 0, core 1
504:     IRQ_LOCAL_MAILBOX_0_1   = GIC_SPI(0x04),
505:     /// @brief ARM core mailbox 1, core 1
506:     IRQ_LOCAL_MAILBOX_1_1   = GIC_SPI(0x05),
507:     /// @brief ARM core mailbox 2, core 1
508:     IRQ_LOCAL_MAILBOX_2_1   = GIC_SPI(0x06),
509:     /// @brief ARM core mailbox 3, core 1
510:     IRQ_LOCAL_MAILBOX_3_1   = GIC_SPI(0x07),
511:     /// @brief ARM core mailbox 0, core 2
512:     IRQ_LOCAL_MAILBOX_0_2   = GIC_SPI(0x08),
513:     /// @brief ARM core mailbox 1, core 2
514:     IRQ_LOCAL_MAILBOX_1_2   = GIC_SPI(0x09),
515:     /// @brief ARM core mailbox 2, core 2
516:     IRQ_LOCAL_MAILBOX_2_2   = GIC_SPI(0x0A),
517:     /// @brief ARM core mailbox 3, core 2
518:     IRQ_LOCAL_MAILBOX_3_2   = GIC_SPI(0x0B),
519:     /// @brief ARM core mailbox 0, core 3
520:     IRQ_LOCAL_MAILBOX_0_3   = GIC_SPI(0x0C),
521:     /// @brief ARM core mailbox 1, core 3
522:     IRQ_LOCAL_MAILBOX_1_3   = GIC_SPI(0x0D),
523:     /// @brief ARM core mailbox 2, core 3
524:     IRQ_LOCAL_MAILBOX_2_3   = GIC_SPI(0x0E),
525:     /// @brief ARM core mailbox 3, core 3
526:     IRQ_LOCAL_MAILBOX_3_3   = GIC_SPI(0x0F),
527:
528:     /// @brief PMU (performance monitoring unit) core 0 interrupt
529:     IRQ_PMU0                = GIC_SPI(0x10),
530:     /// @brief PMU (performance monitoring unit) core 1 interrupt
531:     IRQ_PMU1                = GIC_SPI(0x11),
532:     /// @brief PMU (performance monitoring unit) core 2 interrupt
533:     IRQ_PMU2                = GIC_SPI(0x12),
534:     /// @brief PMU (performance monitoring unit) core 3 interrupt
535:     IRQ_PMU3                = GIC_SPI(0x13),
536:     /// @brief AXI bus error
537:     IRQ_LOCAL_AXI_ERR       = GIC_SPI(0x14),
538:     /// @brief ARM mailbox interrupt
539:     IRQ_ARM_MAILBOX         = GIC_SPI(0x21),
540:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
541:     IRQ_ARM_DOORBELL_0      = GIC_SPI(0x22),
542:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
543:     IRQ_ARM_DOORBELL_1      = GIC_SPI(0x23),
544:     /// @brief VPU halted 0 interrupt
545:     IRQ_VPU0_HALTED         = GIC_SPI(0x24),
546:     /// @brief VPU halted 1 interrupt
547:     IRQ_VPU1_HALTED         = GIC_SPI(0x25),
548:     /// @brief ARM address error interrupt
549:     IRQ_ILLEGAL_TYPE0       = GIC_SPI(0x26),
550:     /// @brief ARM AXI error interrupt
551:     IRQ_ILLEGAL_TYPE1       = GIC_SPI(0x27),
552:     /// @brief USB Host port interrupt
553:     IRQ_HOSTPORT            = GIC_SPI(0x28),
554:
555:     /// @brief BCM system timer 0 interrupt, when compare value is hit
556:     IRQ_TIMER0              = GIC_SPI(0x40),
557:     /// @brief BCM system timer 1 interrupt, when compare value is hit
558:     IRQ_TIMER1              = GIC_SPI(0x41),
559:     /// @brief BCM system timer 2 interrupt, when compare value is hit
560:     IRQ_TIMER2              = GIC_SPI(0x42),
561:     /// @brief BCM system timer 3 interrupt, when compare value is hit
562:     IRQ_TIMER3              = GIC_SPI(0x43),
563:     /// @brief H.264 codec 0 interrupt
564:     IRQ_CODEC0              = GIC_SPI(0x44),
565:     /// @brief H.264 codec 1 interrupt
566:     IRQ_CODEC1              = GIC_SPI(0x45),
567:     /// @brief H.264 codec 2 interrupt
568:     IRQ_CODEC2              = GIC_SPI(0x46),
569:     /// @brief JPEG interrupt
570:     IRQ_JPEG                = GIC_SPI(0x49),
571:     /// @brief ISP interrupt
572:     IRQ_ISP                 = GIC_SPI(0x49),
573:     /// @brief USB interrupt
574:     IRQ_USB                 = GIC_SPI(0x49),
575:     /// @brief VideoCore 3D interrupt
576:     IRQ_3D                  = GIC_SPI(0x4A),
577:     /// @brief GPU transposer interrupt
578:     IRQ_TRANSPOSER          = GIC_SPI(0x4B),
579:     /// @brief Multicore sync 0 interrupt
580:     IRQ_MULTICORESYNC0      = GIC_SPI(0x4C),
581:     /// @brief Multicore sync 1 interrupt
582:     IRQ_MULTICORESYNC1      = GIC_SPI(0x4D),
583:     /// @brief Multicore sync 2 interrupt
584:     IRQ_MULTICORESYNC2      = GIC_SPI(0x4E),
585:     /// @brief Multicore sync 3 interrupt
586:     IRQ_MULTICORESYNC3      = GIC_SPI(0x4F),
587:     /// @brief DMA channel 0 interrupt
588:     IRQ_DMA0                = GIC_SPI(0x50),
589:     /// @brief DMA channel 1 interrupt
590:     IRQ_DMA1                = GIC_SPI(0x51),
591:     /// @brief DMA channel 2 interrupt, I2S PCM TX
592:     IRQ_DMA2                = GIC_SPI(0x52),
593:     /// @brief DMA channel 3 interrupt, I2S PCM RX
594:     IRQ_DMA3                = GIC_SPI(0x53),
595:     /// @brief DMA channel 4 interrupt, SMI
596:     IRQ_DMA4                = GIC_SPI(0x54),
597:     /// @brief DMA channel 5 interrupt, PWM
598:     IRQ_DMA5                = GIC_SPI(0x55),
599:     /// @brief DMA channel 6 interrupt, SPI TX
600:     IRQ_DMA6                = GIC_SPI(0x56),
601:     /// @brief DMA channel 7/8 interrupt, SPI RX
602:     IRQ_DMA7                = GIC_SPI(0x57),
603:     /// @brief DMA channel 7/8 interrupt, undocumented
604:     IRQ_DMA8                = GIC_SPI(0x57),
605:     /// @brief DMA channel 9/10 interrupt, undocumented
606:     IRQ_DMA9                = GIC_SPI(0x58),
607:     /// @brief DMA channel 9/10 interrupt, HDMI
608:     IRQ_DMA10               = GIC_SPI(0x58),
609:     /// @brief DMA channel 11 interrupt, EMMC
610:     IRQ_DMA11               = GIC_SPI(0x59),
611:     /// @brief DMA channel 12 interrupt, UART TX
612:     IRQ_DMA12               = GIC_SPI(0x5A),
613:     /// @brief DMA channel 13 interrupt, undocumented
614:     IRQ_DMA13               = GIC_SPI(0x5B),
615:     /// @brief DMA channel 14 interrupt, UART RX
616:     IRQ_DMA14               = GIC_SPI(0x5C),
617:     /// @brief AUX UART 1 / SPI 0/1 interrupt
618:     IRQ_AUX_UART            = GIC_SPI(0x5D),
619:     /// @brief AUX UART 1 / SPI 0/1 interrupt
620:     IRQ_AUX_SPI             = GIC_SPI(0x5D),
621:     /// @brief ARM interrupt
622:     IRQ_ARM                 = GIC_SPI(0x5E),
623:     /// @brief DMA channel 15 interrupt
624:     IRQ_DMA15               = GIC_SPI(0x5F),
625:     /// @brief HMDI CEC interrupt
626:     IRQ_HDMI_CEC            = GIC_SPI(0x60),
627:     /// @brief HVS (video scaler) interrupt
628:     IRQ_VIDEOSCALER         = GIC_SPI(0x61),
629:     /// @brief Video decoder interrupt
630:     IRQ_DECODER             = GIC_SPI(0x62),
631:     /// @brief DSI 0 (display) interrupt
632:     IRQ_DSI0                = GIC_SPI(0x64),
633:     /// @brief GPU pixel valve 2 interrupt
634:     IRQ_PIXELVALVE2         = GIC_SPI(0x65),
635:     /// @brief CSI 0 (camera) interrupt
636:     IRQ_CAM0                = GIC_SPI(0x66),
637:     /// @brief CSI 1 (camera) interrupt
638:     IRQ_CAM1                = GIC_SPI(0x67),
639:     /// @brief HDMI 0 interrupt
640:     IRQ_HDMI0               = GIC_SPI(0x68),
641:     /// @brief HDMI 1 interrupt
642:     IRQ_HDMI1               = GIC_SPI(0x69),
643:     /// @brief GPU pixel valve 3 interrupt
644:     IRQ_PIXELVALVE3         = GIC_SPI(0x6A),
645:     /// @brief SPI BSC slave interrupt
646:     IRQ_SPI_BSC             = GIC_SPI(0x6A),
647:     /// @brief DSI 1 (display) interrupt
648:     IRQ_DSI1                = GIC_SPI(0x6C),
649:     /// @brief GPU pixel valve 0 interrupt
650:     IRQ_PIXELVALVE0         = GIC_SPI(0x6D),
651:     /// @brief GPU pixel valve 1 / 4 interrupt
652:     IRQ_PIXELVALVE1         = GIC_SPI(0x6E),
653:     /// @brief CPR interrupt
654:     IRQ_CPR                 = GIC_SPI(0x6F),
655:     /// @brief SMI (firmware) interrupt
656:     IRQ_FIRMWARE            = GIC_SPI(0x70),
657:     /// @brief GPIO 0 interrupt
658:     IRQ_GPIO0               = GIC_SPI(0x71),
659:     /// @brief GPIO 1 interrupt
660:     IRQ_GPIO1               = GIC_SPI(0x72),
661:     /// @brief GPIO 2 interrupt
662:     IRQ_GPIO2               = GIC_SPI(0x73),
663:     /// @brief GPIO 3 interrupt
664:     IRQ_GPIO3               = GIC_SPI(0x74),
665:     /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
666:     IRQ_I2C                 = GIC_SPI(0x75),
667:     /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
668:     IRQ_SPI                 = GIC_SPI(0x76),
669:     /// @brief I2S interrupt
670:     IRQ_I2SPCM              = GIC_SPI(0x77),
671:     /// @brief SD host interrupt
672:     IRQ_SDHOST              = GIC_SPI(0x78),
673:     /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
674:     IRQ_UART                = GIC_SPI(0x79),
675:     /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
676:     IRQ_SLIMBUS             = GIC_SPI(0x7A),
677:     /// @brief GPU? VEC interrupt
678:     IRQ_VEC                 = GIC_SPI(0x7B),
679:     /// @brief CPG interrupt
680:     IRQ_CPG                 = GIC_SPI(0x7C),
681:     /// @brief RNG (random number generator) interrupt
682:     IRQ_RNG                 = GIC_SPI(0x7D),
683:     /// @brief EMMC / EMMC2 interrupt
684:     IRQ_ARASANSDIO          = GIC_SPI(0x7E),
685:     /// @brief Ethernet PCIe secure interrupt
686:     IRQ_ETH_PCIE_S          = GIC_SPI(0x7F),
687:     /// @brief PCI Express AVS interrupt
688:     IRQ_AVS                 = GIC_SPI(0x89),
689:     /// @brief PCI Express Ethernet A interrupt
690:     IRQ_PCIE_INTA           = GIC_SPI(0x8F),
691:     /// @brief PCI Express Ethernet B interrupt
692:     IRQ_PCIE_INTB           = GIC_SPI(0x90),
693:     /// @brief PCI Express Ethernet C interrupt
694:     IRQ_PCIE_INTC           = GIC_SPI(0x91),
695:     /// @brief PCI Express Ethernet D interrupt
696:     IRQ_PCIE_INTD           = GIC_SPI(0x92),
697:     /// @brief PCI Express Host A interrupt
698:     IRQ_PCIE_HOST_INTA      = GIC_SPI(0x93),
699:     /// @brief PCI Express Host MSI interrupt
700:     IRQ_PCIE_HOST_MSI       = GIC_SPI(0x94),
701:     /// @brief Ethernet interrupt
702:     IRQ_GENET_0_A           = GIC_SPI(0x9D),
703:     /// @brief Ethernet interrupt
704:     IRQ_GENET_0_B           = GIC_SPI(0x9E),
705:     /// @brief USB XHCI interrupt
706:     IRQ_XHCI_INTERNAL       = GIC_SPI(0xB0),
707: };
708:
709: /// <summary>
710: /// @brief IRQ interrupt numbers
711: /// </summary>
712: enum class FIQ_ID
713: {
714:     /// @brief Hypervisor Physical ARM timer
715:     FIQ_LOCAL_CNTHP         = GIC_PPI(0x0A),
716:     /// @brief Virtual ARM timer
717:     FIQ_LOCAL_CNTV          = GIC_PPI(0x0B),
718:     /// @brief Secure Physical ARM timer
719:     FIQ_LOCAL_CNTPS         = GIC_PPI(0x0D),
720:     /// @brief Non secure Physical ARM timer
721:     FIQ_LOCAL_CNTPNS        = GIC_PPI(0x0E),
722:
723:     /// @brief ARM core mailbox 0, core 0
724:     FIQ_LOCAL_MAILBOX_0_0   = GIC_SPI(0x00),
725:     /// @brief ARM core mailbox 1, core 0
726:     FIQ_LOCAL_MAILBOX_1_0   = GIC_SPI(0x01),
727:     /// @brief ARM core mailbox 2, core 0
728:     FIQ_LOCAL_MAILBOX_2_0   = GIC_SPI(0x02),
729:     /// @brief ARM core mailbox 3, core 0
730:     FIQ_LOCAL_MAILBOX_3_0   = GIC_SPI(0x03),
731:     /// @brief ARM core mailbox 0, core 1
732:     FIQ_LOCAL_MAILBOX_0_1   = GIC_SPI(0x04),
733:     /// @brief ARM core mailbox 1, core 1
734:     FIQ_LOCAL_MAILBOX_1_1   = GIC_SPI(0x05),
735:     /// @brief ARM core mailbox 2, core 1
736:     FIQ_LOCAL_MAILBOX_2_1   = GIC_SPI(0x06),
737:     /// @brief ARM core mailbox 3, core 1
738:     FIQ_LOCAL_MAILBOX_3_1   = GIC_SPI(0x07),
739:     /// @brief ARM core mailbox 0, core 2
740:     FIQ_LOCAL_MAILBOX_0_2   = GIC_SPI(0x08),
741:     /// @brief ARM core mailbox 1, core 2
742:     FIQ_LOCAL_MAILBOX_1_2   = GIC_SPI(0x09),
743:     /// @brief ARM core mailbox 2, core 2
744:     FIQ_LOCAL_MAILBOX_2_2   = GIC_SPI(0x0A),
745:     /// @brief ARM core mailbox 3, core 2
746:     FIQ_LOCAL_MAILBOX_3_2   = GIC_SPI(0x0B),
747:     /// @brief ARM core mailbox 0, core 3
748:     FIQ_LOCAL_MAILBOX_0_3   = GIC_SPI(0x0C),
749:     /// @brief ARM core mailbox 1, core 3
750:     FIQ_LOCAL_MAILBOX_1_3   = GIC_SPI(0x0D),
751:     /// @brief ARM core mailbox 2, core 3
752:     FIQ_LOCAL_MAILBOX_2_3   = GIC_SPI(0x0E),
753:     /// @brief ARM core mailbox 3, core 3
754:     FIQ_LOCAL_MAILBOX_3_3   = GIC_SPI(0x0F),
755:
756:     /// @brief PMU (performance monitoring unit) core 0 interrupt
757:     FIQ_PMU0                = GIC_SPI(0x10),
758:     /// @brief PMU (performance monitoring unit) core 1 interrupt
759:     FIQ_PMU1                = GIC_SPI(0x11),
760:     /// @brief PMU (performance monitoring unit) core 2 interrupt
761:     FIQ_PMU2                = GIC_SPI(0x12),
762:     /// @brief PMU (performance monitoring unit) core 3 interrupt
763:     FIQ_PMU3                = GIC_SPI(0x13),
764:     /// @brief AXI bus error
765:     FIQ_LOCAL_AXI_ERR       = GIC_SPI(0x14),
766:     /// @brief ARM mailbox interrupt
767:     FIQ_ARM_MAILBOX         = GIC_SPI(0x21),
768:     /// @brief ARM doorbell (VCHIQ) 0 interrupt
769:     FIQ_ARM_DOORBELL_0      = GIC_SPI(0x22),
770:     /// @brief ARM doorbell (VCHIQ) 1 interrupt
771:     FIQ_ARM_DOORBELL_1      = GIC_SPI(0x23),
772:     /// @brief VPU halted 0 interrupt
773:     FIQ_VPU0_HALTED         = GIC_SPI(0x24),
774:     /// @brief VPU halted 1 interrupt
775:     FIQ_VPU1_HALTED         = GIC_SPI(0x25),
776:     /// @brief ARM address error interrupt
777:     FIQ_ILLEGAL_TYPE0       = GIC_SPI(0x26),
778:     /// @brief ARM AXI error interrupt
779:     FIQ_ILLEGAL_TYPE1       = GIC_SPI(0x27),
780:     /// @brief USB Host port interrupt
781:     FIQ_HOSTPORT            = GIC_SPI(0x28),
782:
783:     /// @brief BCM system timer 0 interrupt, when compare value is hit
784:     FIQ_TIMER0              = GIC_SPI(0x40),
785:     /// @brief BCM system timer 1 interrupt, when compare value is hit
786:     FIQ_TIMER1              = GIC_SPI(0x41),
787:     /// @brief BCM system timer 2 interrupt, when compare value is hit
788:     FIQ_TIMER2              = GIC_SPI(0x42),
789:     /// @brief BCM system timer 3 interrupt, when compare value is hit
790:     FIQ_TIMER3              = GIC_SPI(0x43),
791:     /// @brief H.264 codec 0 interrupt
792:     FIQ_CODEC0              = GIC_SPI(0x44),
793:     /// @brief H.264 codec 1 interrupt
794:     FIQ_CODEC1              = GIC_SPI(0x45),
795:     /// @brief H.264 codec 2 interrupt
796:     FIQ_CODEC2              = GIC_SPI(0x46),
797:     /// @brief JPEG interrupt
798:     FIQ_JPEG                = GIC_SPI(0x49),
799:     /// @brief ISP interrupt
800:     FIQ_ISP                 = GIC_SPI(0x49),
801:     /// @brief USB interrupt
802:     FIQ_USB                 = GIC_SPI(0x49),
803:     /// @brief VideoCore 3D interrupt
804:     FIQ_3D                  = GIC_SPI(0x4A),
805:     /// @brief GPU transposer interrupt
806:     FIQ_TRANSPOSER          = GIC_SPI(0x4B),
807:     /// @brief Multicore sync 0 interrupt
808:     FIQ_MULTICORESYNC0      = GIC_SPI(0x4C),
809:     /// @brief Multicore sync 1 interrupt
810:     FIQ_MULTICORESYNC1      = GIC_SPI(0x4D),
811:     /// @brief Multicore sync 2 interrupt
812:     FIQ_MULTICORESYNC2      = GIC_SPI(0x4E),
813:     /// @brief Multicore sync 3 interrupt
814:     FIQ_MULTICORESYNC3      = GIC_SPI(0x4F),
815:     /// @brief DMA channel 0 interrupt
816:     FIQ_DMA0                = GIC_SPI(0x50),
817:     /// @brief DMA channel 1 interrupt
818:     FIQ_DMA1                = GIC_SPI(0x51),
819:     /// @brief DMA channel 2 interrupt, I2S PCM TX
820:     FIQ_DMA2                = GIC_SPI(0x52),
821:     /// @brief DMA channel 3 interrupt, I2S PCM RX
822:     FIQ_DMA3                = GIC_SPI(0x53),
823:     /// @brief DMA channel 4 interrupt, SMI
824:     FIQ_DMA4                = GIC_SPI(0x54),
825:     /// @brief DMA channel 5 interrupt, PWM
826:     FIQ_DMA5                = GIC_SPI(0x55),
827:     /// @brief DMA channel 6 interrupt, SPI TX
828:     FIQ_DMA6                = GIC_SPI(0x56),
829:     /// @brief DMA channel 7/8 interrupt, SPI RX
830:     FIQ_DMA7                = GIC_SPI(0x57),
831:     /// @brief DMA channel 7/8 interrupt, undocumented
832:     FIQ_DMA8                = GIC_SPI(0x57),
833:     /// @brief DMA channel 9/10 interrupt, undocumented
834:     FIQ_DMA9                = GIC_SPI(0x58),
835:     /// @brief DMA channel 9/10 interrupt, HDMI
836:     FIQ_DMA10               = GIC_SPI(0x58),
837:     /// @brief DMA channel 11 interrupt, EMMC
838:     FIQ_DMA11               = GIC_SPI(0x59),
839:     /// @brief DMA channel 12 interrupt, UART TX
840:     FIQ_DMA12               = GIC_SPI(0x5A),
841:     /// @brief DMA channel 13 interrupt, undocumented
842:     FIQ_DMA13               = GIC_SPI(0x5B),
843:     /// @brief DMA channel 14 interrupt, UART RX
844:     FIQ_DMA14               = GIC_SPI(0x5C),
845:     /// @brief AUX UART 1 / SPI 0/1 interrupt
846:     FIQ_AUX_UART            = GIC_SPI(0x5D),
847:     /// @brief AUX UART 1 / SPI 0/1 interrupt
848:     FIQ_AUX_SPI             = GIC_SPI(0x5D),
849:     /// @brief ARM interrupt
850:     FIQ_ARM                 = GIC_SPI(0x5E),
851:     /// @brief DMA channel 15 interrupt
852:     FIQ_DMA15               = GIC_SPI(0x5F),
853:     /// @brief HMDI CEC interrupt
854:     FIQ_HDMI_CEC            = GIC_SPI(0x60),
855:     /// @brief HVS (video scaler) interrupt
856:     FIQ_VIDEOSCALER         = GIC_SPI(0x61),
857:     /// @brief Video decoder interrupt
858:     FIQ_DECODER             = GIC_SPI(0x62),
859:     /// @brief DSI 0 (display) interrupt
860:     FIQ_DSI0                = GIC_SPI(0x64),
861:     /// @brief GPU pixel valve 2 interrupt
862:     FIQ_PIXELVALVE2         = GIC_SPI(0x65),
863:     /// @brief CSI 0 (camera) interrupt
864:     FIQ_CAM0                = GIC_SPI(0x66),
865:     /// @brief CSI 1 (camera) interrupt
866:     FIQ_CAM1                = GIC_SPI(0x67),
867:     /// @brief HDMI 0 interrupt
868:     FIQ_HDMI0               = GIC_SPI(0x68),
869:     /// @brief HDMI 1 interrupt
870:     FIQ_HDMI1               = GIC_SPI(0x69),
871:     /// @brief GPU pixel valve 3 interrupt
872:     FIQ_PIXELVALVE3         = GIC_SPI(0x6A),
873:     /// @brief SPI BSC slave interrupt
874:     FIQ_SPI_BSC             = GIC_SPI(0x6A),
875:     /// @brief DSI 1 (display) interrupt
876:     FIQ_DSI1                = GIC_SPI(0x6C),
877:     /// @brief GPU pixel valve 0 interrupt
878:     FIQ_PIXELVALVE0         = GIC_SPI(0x6D),
879:     /// @brief GPU pixel valve 1 / 4 interrupt
880:     FIQ_PIXELVALVE1         = GIC_SPI(0x6E),
881:     /// @brief CPR interrupt
882:     FIQ_CPR                 = GIC_SPI(0x6F),
883:     /// @brief SMI (firmware) interrupt
884:     FIQ_FIRMWARE            = GIC_SPI(0x70),
885:     /// @brief GPIO 0 interrupt
886:     FIQ_GPIO0               = GIC_SPI(0x71),
887:     /// @brief GPIO 1 interrupt
888:     FIQ_GPIO1               = GIC_SPI(0x72),
889:     /// @brief GPIO 2 interrupt
890:     FIQ_GPIO2               = GIC_SPI(0x73),
891:     /// @brief GPIO 3 interrupt
892:     FIQ_GPIO3               = GIC_SPI(0x74),
893:     /// @brief I2C interrupt (logical OR of all I2C bus interrupts)
894:     FIQ_I2C                 = GIC_SPI(0x75),
895:     /// @brief SPI interrupt (logical OR of all SPI bus interrupts)
896:     FIQ_SPI                 = GIC_SPI(0x76),
897:     /// @brief I2S interrupt
898:     FIQ_I2SPCM              = GIC_SPI(0x77),
899:     /// @brief SD host interrupt
900:     FIQ_SDHOST              = GIC_SPI(0x78),
901:     /// @brief PL011 UART interrupt (logical OR of all SPI bus interrupts)
902:     FIQ_UART                = GIC_SPI(0x79),
903:     /// @brief SLIMBUS interrupt, (logical or of all PCIe ethernet interrupts?)
904:     FIQ_SLIMBUS             = GIC_SPI(0x7A),
905:     /// @brief GPU? VEC interrupt
906:     FIQ_VEC                 = GIC_SPI(0x7B),
907:     /// @brief CPG interrupt
908:     FIQ_CPG                 = GIC_SPI(0x7C),
909:     /// @brief RNG (random number generator) interrupt
910:     FIQ_RNG                 = GIC_SPI(0x7D),
911:     /// @brief EMMC / EMMC2 interrupt
912:     FIQ_ARASANSDIO          = GIC_SPI(0x7E),
913:     /// @brief Ethernet PCIe secure interrupt
914:     FIQ_ETH_PCIE_S          = GIC_SPI(0x7F),
915:     /// @brief PCI Express AVS interrupt
916:     FIQ_AVS                 = GIC_SPI(0x89),
917:     /// @brief PCI Express Ethernet A interrupt
918:     FIQ_PCIE_INTA           = GIC_SPI(0x8F),
919:     /// @brief PCI Express Ethernet B interrupt
920:     FIQ_PCIE_INTB           = GIC_SPI(0x90),
921:     /// @brief PCI Express Ethernet C interrupt
922:     FIQ_PCIE_INTC           = GIC_SPI(0x91),
923:     /// @brief PCI Express Ethernet D interrupt
924:     FIQ_PCIE_INTD           = GIC_SPI(0x92),
925:     /// @brief PCI Express Host A interrupt
926:     FIQ_PCIE_HOST_INTA      = GIC_SPI(0x93),
927:     /// @brief PCI Express Host MSI interrupt
928:     FIQ_PCIE_HOST_MSI       = GIC_SPI(0x94),
929:     /// @brief Ethernet interrupt
930:     FIQ_GENET_0_A           = GIC_SPI(0x9D),
931:     /// @brief Ethernet interrupt
932:     FIQ_GENET_0_B           = GIC_SPI(0x9E),
933:     /// @brief USB XHCI interrupt
934:     FIQ_XHCI_INTERNAL       = GIC_SPI(0xB0),
935: };
936:
937: #endif
```

We'll not get into each and every IRQ or FIQ id, but give some descriptions on macros used

- Line 48-466: We define IRQ related macros for Raspberry PI 3
  - Line 49-54: We define the number of IRQ supported by IRQ 1/2, IRQ basic and ARM local IRQ
  - Line 57: We define the number of BCM interrupt registers
  - Line 60: We define the total count of IRQ interrupts supported by the system for Raspberry 3
  - Line 62-77: We define the ID range for IRQ 1/2, IRQ basic and ARM local IRQ
  - Line 80-84: We define a macro to access the IRQ pending register for BCM IRQ lines
  - Line 86-90: We define a macro to access the IRQ enable register for BCM IRQ lines
  - Line 92-96: We define a macro to access the IRQ disable register for BCM IRQ lines
  - Line 98: We define a macro to determine the mask for the selected IRQ id to access the IRQ pending / enable / disable register
  - Line 103-282: We define all currently supported IRQ for Raspberry 3
  - Line 287-466: We define all currently supported FIQ for Raspberry 3
- Line 470-935: We define IRQ related macros for Raspberry PI 4
  - Line 471-476: We define macros to access software generated interrupts, private per core interrupts and interrupts shared by all cores for Raspberry 4
  - Line 479: We define the total count of IRQ interrupts supported by the system for Raspberry 4
  - Line 484-707: We define all currently supported IRQ for Raspberry 4 / 5
  - Line 712-935: We define all currently supported FIQ for Raspberry 4 / 5

### System.cpp {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_INTERRUPTH}

As we've added the `Shutdown()` method to `InterruptSystem`, it is wise to make sure we nicely shutdown the interrupt system before halting or rebooting.
Let's add a call for this.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
128: void System::Halt()
129: {
130:     LOG_INFO("Halt");
131:     GetInterruptSystem().Shutdown();
132:     Timer::WaitMilliSeconds(WaitTime);
...
152: void System::Reboot()
153: {
154:     LOG_INFO("Reboot");
155:     GetInterruptSystem().Shutdown();
156:     Timer::WaitMilliSeconds(WaitTime);
...
```

### Update CMake file {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_UPDATE_CMAKE_FILE}

As we have not added any source files to the `baremetal` library, we do not need to update its CMake file.

### Update application code {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_UPDATE_APPLICATION_CODE}

We'll make a few small changes to use the new `InterruptSystem` implementation.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/Console.h>
4: #include <baremetal/InterruptHandler.h>
5: #include <baremetal/Interrupts.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/System.h>
8: #include <baremetal/Timer.h>
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

### Configuring, building and debugging {#TUTORIAL_18_INTERRUPTS_INTERRUPT_SYSTEM___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

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

Next: [19-timer-extension](19-timer-extension.md)
