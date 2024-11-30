//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : start.S
//
// Namespace   : -
//
// Class       : -
//
// Description : Startup code. This is the entry point to any executable. It puts all cores except core 0 in sleep mode.
//               For core 0, it sets the stack pointer to just below the code (as the stack grows down), and then calls main().
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

/// @brief Start of kernel code
#define MEM_KERNEL_START        _start
/// @brief Maximum size of code
#define KERNEL_MAX_SIZE         0x200000 // Max 2 Mb
/// @brief Kernel code end
#define MEM_KERNEL_END          (MEM_KERNEL_START + KERNEL_MAX_SIZE)

/// @brief Size of kernel stack for a single core
#define KERNEL_STACK_SIZE       0x20000 // 128 Kb
/// @brief Kernel stack top (stack grows down)
#define MEM_KERNEL_STACK        (MEM_KERNEL_END + KERNEL_STACK_SIZE)

/// @brief Our code starts in section .init
.section ".init"

.global _start

/// @brief Startup starts here
_start:
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

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =MEM_KERNEL_STACK
    mov     sp, x1

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
    // Loop as loop as the end is not reached
    cbnz    w2, clear_bss_loop

    // jump to C code, should not return
empty_bss:
    bl      main
    // For fail safety, halt this core too
    b       waitevent
