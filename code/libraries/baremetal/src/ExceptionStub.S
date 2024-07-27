//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ExceptionStub.S
//
// Namespace   : -
//
// Class       : -
//
// Description : Exception stub. This defines the functions to be called in case a processor exception is thown.
//               Note: this file is based on the Circle startup assembly file by Rene Stange.
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#include <baremetal/BCMRegisters.h>
#include <baremetal/Exception.h>
#include <baremetal/SysConfig.h>

/// @file
/// Exception stub assembly code

.macro	vector handler

    .align	7

    b \handler                          // Jump to handler

.endm

.macro  stub name, exception, handler

    .globl  \name
\name:
    mrs     x0, esr_el1                 // Read Exception Syndrome Register (EL1)
    mrs     x1, spsr_el1                // Read Saved Program Status Register (EL1)
    mov     x2, x30                     // X30=Link Register(LR)
    mrs     x3, elr_el1                 // Read Exception Link Register (EL1)
    mrs     x4, sp_el0                  // Read Stack Pointer (EL0)
    mov     x5, sp                      // Get Stack Pointer (EL1)
    mrs     x6, far_el1                 // Fault Address Register (EL1)

    str     x6, [sp, #-16]!             // Store X6 on stack, decrement stack
    stp     x4, x5, [sp, #-16]!         // Store X4,X5 on stack, decrement stack
    stp     x2, x3, [sp, #-16]!         // Store X2,X3 on stack, decrement stack
    stp     x0, x1, [sp, #-16]!         // Store X0,X1 on stack, decrement stack

    mov     x0, #\exception             // Get exception function pointer
    mov     x1, sp                      // Save stack pointer
    b       \handler                    // Calls handler(uint64 exceptionID, AbortFrame* abortFrame). x0 = exception is exceptionID, x1 = sp = abortFrame. Never returns

.endm

    .text

    .align  11

    .globl  VectorTable
VectorTable:

    // from current EL with sp_el0 (mode EL0, El1t, El2t, EL3t)
    vector  SynchronousStub
    vector  IRQStub
    vector  FIQStub
    vector  SErrorStub

    // from current EL with sp_elx, x != 0  (mode El1h, El2h, EL3h)
    vector  SynchronousStub
    vector  IRQStub
    vector  FIQStub
    vector  SErrorStub

    // from lower EL, target EL minus 1 is AArch64
    vector  HVCStub
    vector  UnexpectedStub
    vector  UnexpectedStub
    vector  UnexpectedStub

    // from lower EL, target EL minus 1 is AArch32
    vector  UnexpectedStub
    vector  UnexpectedStub
    vector  UnexpectedStub
    vector  UnexpectedStub

//*************************************************
// IRQ stub
//*************************************************
.macro irq_stub name, handler
    .globl  \name
\name:
    stp     x29, x30, [sp, #-16]!       // Save x29, x30 onto stack
    mrs     x29, elr_el1                // Read Exception Link Register (EL1)
    mrs     x30, spsr_el1               // Read Saved Program Status Register (EL1)
    stp     x29, x30, [sp, #-16]!       // Save onto stack
    msr     DAIFClr, #1                 // Enable FIQ

#ifdef BAREMETAL_SAVE_VFP_REGS_ON_IRQ
    stp     q30, q31, [sp, #-32]!       // Save q0-q31 onto stack
    stp     q28, q29, [sp, #-32]!
    stp     q26, q27, [sp, #-32]!
    stp     q24, q25, [sp, #-32]!
    stp     q22, q23, [sp, #-32]!
    stp     q20, q21, [sp, #-32]!
    stp     q18, q19, [sp, #-32]!
    stp     q16, q17, [sp, #-32]!
    stp     q14, q15, [sp, #-32]!
    stp     q12, q13, [sp, #-32]!
    stp     q10, q11, [sp, #-32]!
    stp     q8, q9, [sp, #-32]!
    stp     q6, q7, [sp, #-32]!
    stp     q4, q5, [sp, #-32]!
    stp     q2, q3, [sp, #-32]!
    stp     q0, q1, [sp, #-32]!
#endif // BAREMETAL_SAVE_VFP_REGS_ON_IRQ
    stp     x27, x28, [sp, #-16]!       // Save x0-x28 onto stack
    stp     x25, x26, [sp, #-16]!
    stp     x23, x24, [sp, #-16]!
    stp     x21, x22, [sp, #-16]!
    stp     x19, x20, [sp, #-16]!
    stp     x17, x18, [sp, #-16]!
    stp     x15, x16, [sp, #-16]!
    stp     x13, x14, [sp, #-16]!
    stp     x11, x12, [sp, #-16]!
    stp     x9, x10, [sp, #-16]!
    stp     x7, x8, [sp, #-16]!
    stp     x5, x6, [sp, #-16]!
    stp     x3, x4, [sp, #-16]!
    stp     x1, x2, [sp, #-16]!
    str     x0, [sp, #-16]!

    bl      \handler                    // Call interrupt handler

    ldr     x0, [sp], #16               // Restore x0-x28 from stack
    ldp     x1, x2, [sp], #16
    ldp     x3, x4, [sp], #16
    ldp     x5, x6, [sp], #16
    ldp     x7, x8, [sp], #16
    ldp     x9, x10, [sp], #16
    ldp     x11, x12, [sp], #16
    ldp     x13, x14, [sp], #16
    ldp     x15, x16, [sp], #16
    ldp     x17, x18, [sp], #16
    ldp     x19, x20, [sp], #16
    ldp     x21, x22, [sp], #16
    ldp     x23, x24, [sp], #16
    ldp     x25, x26, [sp], #16
    ldp     x27, x28, [sp], #16
#ifdef BAREMETAL_SAVE_VFP_REGS_ON_IRQ
    ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
    ldp     q2, q3, [sp], #32
    ldp     q4, q5, [sp], #32
    ldp     q6, q7, [sp], #32
    ldp     q8, q9, [sp], #32
    ldp     q10, q11, [sp], #32
    ldp     q12, q13, [sp], #32
    ldp     q14, q15, [sp], #32
    ldp     q16, q17, [sp], #32
    ldp     q18, q19, [sp], #32
    ldp     q20, q21, [sp], #32
    ldp     q22, q23, [sp], #32
    ldp     q24, q25, [sp], #32
    ldp     q26, q27, [sp], #32
    ldp     q28, q29, [sp], #32
    ldp     q30, q31, [sp], #32
#endif // BAREMETAL_SAVE_VFP_REGS_ON_IRQ

    msr     DAIFSet, #1                 // Disable FIQ
    ldp     x29, x30, [sp], #16         // Restore from stack
    msr     elr_el1, x29                // Restore Exception Link Register (EL1)
    msr     spsr_el1, x30               // Restore Saved Program Status Register (EL1)
    ldp     x29, x30, [sp], #16         // restore x29, x30 from stack

    eret                                // Restore previous EL

.endm

//*************************************************
// FIQ stub
//*************************************************
    .globl  FIQStub
FIQStub:
#ifdef BAREMETAL_SAVE_VFP_REGS_ON_FIQ
    stp     q30, q31, [sp, #-32]!       // Save q0-q31 onto stack
    stp     q28, q29, [sp, #-32]!
    stp     q26, q27, [sp, #-32]!
    stp     q24, q25, [sp, #-32]!
    stp     q22, q23, [sp, #-32]!
    stp     q20, q21, [sp, #-32]!
    stp     q18, q19, [sp, #-32]!
    stp     q16, q17, [sp, #-32]!
    stp     q14, q15, [sp, #-32]!
    stp     q12, q13, [sp, #-32]!
    stp     q10, q11, [sp, #-32]!
    stp     q8, q9, [sp, #-32]!
    stp     q6, q7, [sp, #-32]!
    stp     q4, q5, [sp, #-32]!
    stp     q2, q3, [sp, #-32]!
    stp     q0, q1, [sp, #-32]!
#endif // BAREMETAL_SAVE_VFP_REGS_ON_FIQ
    stp     x29, x30, [sp, #-16]!       // Save x0-x28 onto stack
    stp     x27, x28, [sp, #-16]!
    stp     x25, x26, [sp, #-16]!
    stp     x23, x24, [sp, #-16]!
    stp     x21, x22, [sp, #-16]!
    stp     x19, x20, [sp, #-16]!
    stp     x17, x18, [sp, #-16]!
    stp     x15, x16, [sp, #-16]!
    stp     x13, x14, [sp, #-16]!
    stp     x11, x12, [sp, #-16]!
    stp     x9, x10, [sp, #-16]!
    stp     x7, x8, [sp, #-16]!
    stp     x5, x6, [sp, #-16]!
    stp     x3, x4, [sp, #-16]!
    stp     x1, x2, [sp, #-16]!
    str     x0, [sp, #-16]!

    ldr     x2, =s_fiqData
    ldr     x1, [x2]                    // Get s_fiqData.handler
    cmp     x1, #0                      // Is handler set?
    b.eq    no_fiq_handler
    ldr     x0, [x2, #8]                // Get s_fiqData.param
    blr     x1                          // Call handler

restore_after_fiq_handler:
    ldr     x0, [sp], #16               // Restore x0-x28 from stack
    ldp     x1, x2, [sp], #16
    ldp     x3, x4, [sp], #16
    ldp     x5, x6, [sp], #16
    ldp     x7, x8, [sp], #16
    ldp     x9, x10, [sp], #16
    ldp     x11, x12, [sp], #16
    ldp     x13, x14, [sp], #16
    ldp     x15, x16, [sp], #16
    ldp     x17, x18, [sp], #16
    ldp     x19, x20, [sp], #16
    ldp     x21, x22, [sp], #16
    ldp     x23, x24, [sp], #16
    ldp     x25, x26, [sp], #16
    ldp     x27, x28, [sp], #16
    ldp     x29, x30, [sp], #16
#ifdef BAREMETAL_SAVE_VFP_REGS_ON_FIQ
    ldp     q0, q1, [sp], #32           // Restore q0-q31 from stack
    ldp     q2, q3, [sp], #32
    ldp     q4, q5, [sp], #32
    ldp     q6, q7, [sp], #32
    ldp     q8, q9, [sp], #32
    ldp     q10, q11, [sp], #32
    ldp     q12, q13, [sp], #32
    ldp     q14, q15, [sp], #32
    ldp     q16, q17, [sp], #32
    ldp     q18, q19, [sp], #32
    ldp     q20, q21, [sp], #32
    ldp     q22, q23, [sp], #32
    ldp     q24, q25, [sp], #32
    ldp     q26, q27, [sp], #32
    ldp     q28, q29, [sp], #32
    ldp     q30, q31, [sp], #32
#endif // BAREMETAL_SAVE_VFP_REGS_ON_FIQ

    eret                                // Restore previous EL

no_fiq_handler:
    ldr     x1, =RPI_INTRCTRL_FIQ_CONTROL // Disable FIQ (if handler is not set)
    mov     w0, #0
    str     w0, [x1]
    b       restore_after_fiq_handler

/*
 * HVC stub
 */
HVCStub:                                // Return to EL2h mode
    mrs     x0, spsr_el2                // Read Saved Program Status Register (EL2)
    bic     x0, x0, #0xF                // Clear bit 3:0
    mov     x1, #9
    orr     x0, x0, x1                  // Set bit 3 and bit 0 -> EL2h
    msr     spsr_el2, x0                // Write Saved Program Status Register (EL2)
    eret                                // Move to EL2h

    .data

    .align  3

    .globl  s_fiqData
s_fiqData:                              // Matches FIQData:
    .quad   0                           // handler
    .quad   0                           // param
    .word   0                           // fiqID (unused)

//*************************************************
// Abort stubs
//*************************************************
    stub        UnexpectedStub,     EXCEPTION_UNEXPECTED,   ExceptionHandler
    stub        SynchronousStub,    EXCEPTION_SYNCHRONOUS,  ExceptionHandler
    stub        SErrorStub,         EXCEPTION_SYSTEM_ERROR, ExceptionHandler
    irq_stub    IRQStub,                                    InterruptHandler

// End
