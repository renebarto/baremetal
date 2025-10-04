//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "baremetal/String.h"
#include "stdlib/Types.h"

/// @file
/// Type serialization functions

namespace baremetal {

// Specializations

// Every serialization specialization uses a width parameter, which is used for expansion
// width < 0 Left aligned
// width > 0 right aligned
// width < actual length no alignment

/// <summary>
/// Serialize boolean
/// </summary>
/// <param name="value">Value</param>
/// <returns>Resulting string</returns>
inline String Serialize(const bool& value)
{
    return String(value ? "true" : "false");
}

String Serialize(char value, int width = 0);
String Serialize(int8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(uint8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(int16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(uint16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(int32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(uint32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(int64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
String Serialize(uint64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);

/// <summary>
/// Serialize long long int value, type specific specialization
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Resulting string</returns>
inline String Serialize(long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
{
    return Serialize(static_cast<int64>(value), width, base, showBase, leadingZeros);
}

/// <summary>
/// Serialize unsigned long long int value, type specific specialization
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Resulting string</returns>
inline String Serialize(unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
{
    return Serialize(static_cast<uint64>(value), width, base, showBase, leadingZeros);
}

String Serialize(float value, int width = 0, int precision = 16);
String Serialize(double value, int width = 0, int precision = 16);
String Serialize(const String& value, int width = 0, bool quote = false);
String Serialize(const char* value, int width = 0, bool quote = false);
String Serialize(const void* value, int width = 0);
String Serialize(void* value, int width = 0);

} // namespace baremetal
