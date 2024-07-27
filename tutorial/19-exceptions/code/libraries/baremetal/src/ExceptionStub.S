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
    vector  UnexpectedStub
    vector  UnexpectedStub
    vector  SErrorStub

    // from current EL with sp_elx, x != 0  (mode El1h, El2h, EL3h)
    vector  SynchronousStub
    vector  UnexpectedStub
    vector  UnexpectedStub
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

// End
