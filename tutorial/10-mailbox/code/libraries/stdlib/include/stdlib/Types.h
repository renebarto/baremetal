//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Types.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Common types, platform dependent
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
/// Standard types

/// @brief Unsigned 8 bit integer
typedef unsigned char uint8;
/// @brief Unsigned 16 bit integer
typedef unsigned short uint16;
/// @brief Unsigned 32 bit integer
typedef unsigned int uint32;
/// @brief Unsigned 64 bit integer
typedef unsigned long uint64;

/// @brief Signed 8 bit integer
typedef signed char int8;
/// @brief Signed 16 bit integer
typedef signed short int16;
/// @brief Signed 32 bit integer
typedef signed int int32;
/// @brief Signed 64 bit integer
typedef signed long int64;

/// @brief Pointer as signed 64 bit integer
typedef int64 intptr;
/// @brief Pointer as unsigned 64 bit integer
typedef uint64 uintptr;
/// @brief Unsigned size type
typedef uint64 size_type;
/// @brief Signed size type
typedef int64 ssize_type;

/// @brief Unsigned size
typedef size_type size_t;
/// @brief Signed size
typedef ssize_type ssize_t;

/// @brief Pointer to unsigned volatile byte (for registers)
typedef volatile uint8 *regaddr;
