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

/// @brief Set part to be used by GPU (normally set in config.txt)
#ifndef GPU_MEM_SIZE
#define GPU_MEM_SIZE (64 * MEGABYTE)
#endif

#include "baremetal/MemoryMap.h"
