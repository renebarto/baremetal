//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Serialization.h
//
// Namespace   : serialization
//
// Class       : -
//
// Description : Serialization of types to character buffer
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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

#include <baremetal/Types.h>

/// @file
/// Type serialization functions

namespace baremetal {

/// <summary>
/// Serialize a 32 bits unsigned value to buffer.
/// 
/// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
/// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned. 
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// 
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
/// <summary>
/// Serialize a 64 bits unsigned value to buffer.
/// 
/// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
/// Width specifies the minimum width in characters. The value is always written right aligned. 
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// 
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros);

} // namespace baremetal
