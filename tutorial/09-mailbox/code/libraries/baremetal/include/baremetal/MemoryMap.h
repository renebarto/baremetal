//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : MemoryMap.h
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : Memory layout definitions
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

#pragma once

// Default RAM memory size (minimum size as of RPI 3)
#define MEM_SIZE (1024 * MEGABYTE)
// Part of RAM mapped to GPU (minimum size is 64Mb) (if not already specified in SysConfig.h)
#if !defined(GPU_MEM_SIZE)
#define GPU_MEM_SIZE (64 * MEGABYTE) // set in config.txt
#endif
// Part of RAM mapped to ARM (this is the total amount - amount allocated to GPU)
#define ARM_MEM_SIZE (MEM_SIZE - GPU_MEM_SIZE) // normally overwritten

// Memory reserved for paging
#define PAGE_RESERVE (16 * MEGABYTE)

// Size of every page
#define PAGE_SIZE    0x10000 // 64Kb

// Maximum size of the kernel space (if not already specified in SysConfig.h)
#if !defined(KERNEL_MAX_SIZE)
#define KERNEL_MAX_SIZE (2 * MEGABYTE)
#endif
// Memory reserved for the stack (this memory is reserved for every core)
#define KERNEL_STACK_SIZE       0x20000 // 128 Kb
// Memory reserved for the exception stack (this memory is reserved for every core)
#define EXCEPTION_STACK_SIZE    0x8000  // 32 Kb
// Location where the kernel starts. This is also the location where the code starts
#define MEM_KERNEL_START        0x80000
// End of kernel space (start + size)
#define MEM_KERNEL_END          (MEM_KERNEL_START + KERNEL_MAX_SIZE)
// Top of stack for core 0 (stack grows down)
#define MEM_KERNEL_STACK        (MEM_KERNEL_END + KERNEL_STACK_SIZE)
// Top of exception stack for core 0 (stack grows down). Also includes the stacks for cores 1..CORES-1
#define MEM_EXCEPTION_STACK     (MEM_KERNEL_STACK + KERNEL_STACK_SIZE * (CORES - 1) + EXCEPTION_STACK_SIZE)
// Top of exception stack for all cores (stack grows down). Also includes the exception stacks for cores 1..CORES-1
#define MEM_EXCEPTION_STACK_END (MEM_EXCEPTION_STACK + EXCEPTION_STACK_SIZE * (CORES - 1))

#if BAREMETAL_RPI_TARGET == 3
// Size reserved for coherent memory
#define COHERENT_REGION_SIZE 1 * MEGABYTE
#else
// Region reserved for coherent memory (memory shared between ARM and GPU). We reserve 4 Mb, but make sure then end is rounded
#define COHERENT_REGION_SIZE 4 * MEGABYTE
#endif

// Region reserved for coherent memory rounded to 1 Mb, with >= 1Mb gap
#define MEM_COHERENT_REGION ((MEM_EXCEPTION_STACK_END + 2 * MEGABYTE) & ~(MEGABYTE - 1))
