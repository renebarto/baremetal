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

#include <baremetal/SysConfig.h>

/// @file
/// System startup assembly code

.macro armv8_switch_to_el1_m, xreg1, xreg2

    // Initialize Generic Timers
    mrs \xreg1, cnthctl_el2             // Read Counter-timer Hypervisor Control register (EL2)
    orr \xreg1, \xreg1, #0x3            // Enable EL1 access to timers. Sets bit 1 (EL0VCTEN) and 0 (EL0PCTEN). This disables trapping access to CNTVCT_EL0, CNTFRQ_EL0, CNTPCT_EL0, CNTFRQ_EL0 in EL0
    msr cnthctl_el2, \xreg1             // Write Counter-timer Hypervisor Control register (EL2)
    msr cntvoff_el2, xzr                // Write 0 to Counter-timer Virtual Offset register. This resets the offset of the virtual timer

    // Initilize MPID/MPIDR registers
    mrs \xreg1, midr_el1                // Read Main ID Register (EL1)
    mrs \xreg2, mpidr_el1               // Read Multiprocessor Affinity Register (EL1)
    msr vpidr_el2, \xreg1               // Write Virtualization Processor ID Register (EL2)
    msr vmpidr_el2, \xreg2              // Virtualization Multiprocessor ID Register

    // Disable coprocessor traps
    // Set RES1 bits (13,9,7..0) + TSM,TZ to 1
    // Set RES0 bits (63..32,29..21,19..14,11) +
    // TCPAC,TAM,TTA,TFP, to 0
    // Enables access to CPACR_EL1, AMUSERENR_EL0, AMCFGR_EL0, AMCGCR_EL0, AMCNTENCLR0_EL0, AMCNTENCLR1_EL0,
    // AMCNTENSET0_EL0, AMCNTENSET1_EL0, AMCR_EL0, AMEVCNTR0<n>_EL0, AMEVCNTR1<n>_EL0, AMEVTYPER0<n>_EL0, AMEVTYPER1<n>_EL0, FPCR, FPSR, FPEXC32_EL2)
    mov \xreg1, #0x33ff
    msr cptr_el2, \xreg1                // Write Architectural Feature Trap Register (EL2). Disable coprocessor traps to EL2
    msr hstr_el2, xzr                   // Write Hypervisor System Trap Register (EL2). Disable coprocessor traps to EL2
    mov \xreg1, #3 << 20
    msr cpacr_el1, \xreg1               // Enable FP/SIMD at EL1

    // Initialize HCR_EL2
    mov \xreg1, #(1 << 31)              // 64bit EL1
    msr hcr_el2, \xreg1                 // Write Hypervisor Configuration Register (EL2). Set bit 31 (RW), which enforces EL1 to be in AArch64 mode

    // SCTLR_EL1 initialization
    //
    // setting RES1 bits (29,28,23,22,20,11) to 1
    // and RES0 bits (31,30,27,21,17,13,10,6) +
    // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
    // CP15BEN,SA0,SA,C,A,M to 0
    mov \xreg1, #0x0800
    movk \xreg1, #0x30d0, lsl #16
    msr sctlr_el1, \xreg1               // Write System Control Register (EL1)

    // Return to the EL1_SP1 mode from EL2
    // Set RES1 bits (none) + D,A,I,F,M[2] to 1
    // Set RES0 bits (63..32,27..26,19..14) +
    // N,Z,C,V,TCO,DIT,UAO,PAN,SS,IL,ALLINT,SSBS,BTYPE,M[4:3],M[1:0] to 0
    // Save and restore D,A,I,F bits, move to EL1t
    mov \xreg1, #0x3c4
    msr spsr_el2, \xreg1                // Write Saved Program Status Register (EL2), EL1_SP0 | D | A | I | F
    adr \xreg1, label1                  // Set return address when EL2 exception occurs
    msr elr_el2, \xreg1                 // Write Exception Link Register (EL2)
    eret
label1:
.endm

.section .init

    .globl _start
_start:                                 // Normally entered from armstub8 in EL2 after boot
    mrs x0, CurrentEL                   // Check if already in EL1t mode?
    cmp x0, #4
    beq EL1

    ldr x0, =MEM_EXCEPTION_STACK        // IRQ, FIQ and exception handler run in EL1h
    msr sp_el1, x0                      // Write Stack Pointer (EL1), init El1 stack

    ldr	x0, =VectorTable                // Init exception vector table for EL2
    msr	vbar_el2, x0                    // Write Vector Base Address Register (EL2)

    armv8_switch_to_el1_m x0, x1        // Move to EL1

EL1:
    ldr x0, =MEM_KERNEL_STACK           // Main thread runs in EL1t and uses sp_el0
    mov sp, x0                          // init its stack

    ldr	x0, =VectorTable	            // Init exception vector table for EL1
    msr	vbar_el1, x0                    // Write Vector Base Address Register (EL1)

    b sysinit                           // Jump to C/C++ code

// End
