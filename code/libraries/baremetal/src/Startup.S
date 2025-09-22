//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Startup.S
//
// Namespace   : -
//
// Class       : -
//
// Description : Startup code. This is the entry point to any executable. It puts all cores except core 0 in sleep mode.
//               For core 0, it sets the stack pointer to just below the code (as the stack grows down), and then calls main().
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

#include "baremetal/SysConfig.h"

.macro armv8_switch_to_el1_m, xreg1, xreg2

    // Initialize Generic Timers
    mrs     \xreg1, cnthctl_el2
    orr     \xreg1, \xreg1, #0x3        // Enable EL1 access to timers
    msr     cnthctl_el2, \xreg1
    msr     cntvoff_el2, xzr

    // Initilize MPID/MPIDR registers
    mrs     \xreg1, midr_el1
    mrs     \xreg2, mpidr_el1
    msr     vpidr_el2, \xreg1
    msr     vmpidr_el2, \xreg2

    // Disable coprocessor traps
    mov     \xreg1, #0x33ff
    msr     cptr_el2, \xreg1            // Disable coprocessor traps to EL2
    msr     hstr_el2, xzr               // Disable coprocessor traps to EL2
    mov     \xreg1, #3 << 20
    msr     cpacr_el1, \xreg1           // Enable FP/SIMD at EL1

    // Initialize HCR_EL2
    mov     \xreg1, #(1 << 31)          // 64bit EL1
    msr     hcr_el2, \xreg1

    // SCTLR_EL1 initialization
    //
    // setting RES1 bits (29,28,23,22,20,11) to 1
    // and RES0 bits (31,30,27,21,17,13,10,6) +
    // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
    // CP15BEN,SA0,SA,C,A,M to 0
    mov     \xreg1, #0x0800
    movk    \xreg1, #0x30d0, lsl #16
    msr     sctlr_el1, \xreg1

    // Return to the EL1_SP1 mode from EL2
    mov     \xreg1, #0x3c4
    msr     spsr_el2, \xreg1            // EL1_SP0 | D | A | I | F
    adr     \xreg1, label1
    msr     elr_el2, \xreg1
    eret
label1:
.endm

.section .init

    .globl _start
_start:                                 // normally entered from armstub8 in EL2 after boot
    // Read MPIDR_EL1 register, low 7 bits contain core id (as we have 4 cores, we mask only lowest two bits)
    mrs     x1, mpidr_el1
    // Mask away everything but the core id
    and     x1, x1, #3
    // If core id is 0, continue
    cbz     x1, core0
    // If core id > 0, start wait loop
waitevent:
    wfe
    b       waitevent

core0:
    // core 0
    mrs     x0, CurrentEL               // check if already in EL1t mode?
    cmp     x0, #4
    beq     EL1

    ldr     x0, =MEM_EXCEPTION_STACK    // IRQ, FIQ and exception handler run in EL1h
    msr     sp_el1, x0                  // init their stack

    armv8_switch_to_el1_m x0, x1

EL1:
    ldr     x0, =MEM_KERNEL_STACK       // main thread runs in EL1t and uses sp_el0
    mov     sp, x0                      // init its stack

    // clear bss
    // Load bss start
    ldr     x1, =__bss_start
    // Load bss size (size is number of 8 byte blocks in bss section)
    ldr     w2, =__bss_size // In 8 byte chunks, so actual size is __bss_size * 8
    // If bss is empty
    cbz     w2, empty_bss

clear_bss_loop:
    // Store 0 in x1 location for 8 bytes, increment x1 by 8
    str     xzr, [x1], #8
    // Count down number of blocks
    sub     w2, w2, #1
    // Loop as long as the end is not reached
    cbnz    w2, clear_bss_loop

    // jump to C code, should not return
empty_bss:
    bl      main                        // Jump to main()
    // For fail safety, halt this core too
    b       waitevent

// End
