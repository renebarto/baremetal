//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : SysConfig.h
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : System configuration defines
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

/// @file
/// System configuration parameters. This file will include MemoryMap.h to set the defaults if not overridden.

/// @brief Number of cores to use (if ARM_ALLOW_MULTI_CORE is defined)
#define CORES    4

/// @brief Size of 1 Megabyte
#define MEGABYTE 0x100000
/// @brief Size of 1 Gigabyte
#define GIGABYTE 0x40000000ULL

/// @brief KERNEL_MAX_SIZE is the maximum allowed size of a built kernel image.
///
/// If your kernel image contains big data areas it may be required to
/// increase this value. The value must be a multiple of 16 KByte.
#ifndef KERNEL_MAX_SIZE
#define KERNEL_MAX_SIZE (2 * MEGABYTE)
#endif

/// @brief HEAP_DEFAULT_NEW defines the default heap to be used for the "new"
/// operator, if a memory type is not explicitly specified. Possible
/// values are HEAP_LOW (memory below 1 GByte), HEAP_HIGH (memory above
/// 1 GByte) or HEAP_ANY (memory above 1 GB, if available, or memory
/// below 1 GB otherwise). This value can be set to HEAP_ANY for
/// a virtually unified heap, which uses the whole available memory
/// space. Because this may cause problems with some devices, which
/// explicitly need low memory for DMA, this value defaults to HEAP_LOW.
/// This setting is only of importance for the Raspberry Pi 4.
#ifndef HEAP_DEFAULT_NEW
#define HEAP_DEFAULT_NEW HeapType::LOW
#endif

/// @brief HEAP_DEFAULT_MALLOC defines the heap to be used for malloc() and
/// calloc() calls. See the description of HEAP_DEFAULT_NEW for details!
/// Modifying this setting is not recommended, because there are device
/// drivers, which require to allocate low memory for DMA purpose using
/// malloc(). This setting is only of importance for the Raspberry Pi 4.
#ifndef HEAP_DEFAULT_MALLOC
#define HEAP_DEFAULT_MALLOC HeapType::LOW
#endif

/// @brief HEAP_BLOCK_BUCKET_SIZES configures the heap allocator, which is the
/// base of dynamic memory management ("new" operator and malloc()). The
/// heap allocator manages free memory blocks in a number of free lists
/// (buckets). Each free list contains blocks of a specific size. On
/// block allocation the requested block size is rounded up to the
/// size of next available bucket size. If the requested size is greater
/// than the largest available bucket size, the block can be allocated,
/// but the memory space is lost, if the block will be freed later.
/// Because the block buckets have to be walked through on each allocate
/// and free operation, it is preferable to have only a few buckets.
/// With this option you can configure the bucket sizes, so that they
/// fit best for your application needs. You have to define a comma
/// separated list of increasing bucket sizes. All sizes must be a
/// multiple of 64. Up to 20 sizes can be defined.
#ifndef HEAP_BLOCK_BUCKET_SIZES
#define HEAP_BLOCK_BUCKET_SIZES 0x40, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x80000
#endif

/// @brief Set part to be used by GPU (normally set in config.txt)
#ifndef GPU_MEM_SIZE
#define GPU_MEM_SIZE (64 * MEGABYTE)
#endif

#include "baremetal/MemoryMap.h"
