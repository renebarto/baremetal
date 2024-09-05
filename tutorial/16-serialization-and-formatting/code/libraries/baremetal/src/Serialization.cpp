//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Serialization.cpp
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

#include <baremetal/Serialization.h>

#include <baremetal/Util.h>

/// @file
/// Type serialization functions implementation

namespace baremetal {

/// @brief Write characters with base above 10 as uppercase or not
static bool Uppercase = true;

static string SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
static string SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);

/// <summary>
/// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
/// </summary>
/// <param name="value">Digit value</param>
/// <returns>Converted digit character</returns>
static constexpr char GetDigit(uint8 value)
{
    return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
}

/// <summary>
/// Calculated the amount of digits needed to represent an unsigned value of bits using base
/// </summary>
/// <param name="bits">Size of integer in bits</param>
/// <param name="base">Base to be used</param>
/// <returns>Maximum amount of digits needed</returns>
static constexpr int BitsToDigits(int bits, int base)
{
    int result = 0;
    uint64 value = 0xFFFFFFFFFFFFFFFF;
    if (bits < 64)
        value &= ((1ULL << bits) - 1);

    while (value > 0)
    {
        value /= base;
        result++;
    }

    return result;
}

/// <summary>
/// Serialize a character value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, if negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <returns>Serialized string value</returns>
string Serialize(char value, int width)
{
    string result;

    int    numDigits = 0;
    bool   negative = (value < 0);
    uint64 absVal = static_cast<uint64>(negative ? -value : value);
    uint64 divisor = 1;
    int    absWidth = (width < 0) ? -width : width;
    while (absVal >= divisor)
    {
        divisor *= 10;
        ++numDigits;
    }

    if (numDigits == 0)
    {
        result = "0";
        return result;
    }
    if (negative)
    {
        result += '-';
        absWidth--;
    }
    while (numDigits > 0)
    {
        divisor /= 10;
        int digit = (absVal / divisor) % 10;
        result += GetDigit(digit);
        --numDigits;
    }
    return result.align(width);
}

/// <summary>
/// Serialize a 8 bit signed value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(int8 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalInt(value, width, base, showBase, leadingZeros, 8);
}

/// <summary>
/// Serialize a 8 bit unsigned value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(uint8 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 8);
}

/// <summary>
/// Serialize a 16 bit signed value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(int16 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalInt(value, width, base, showBase, leadingZeros, 16);
}

/// <summary>
/// Serialize a 16 bit unsigned value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(uint16 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 16);
}

/// <summary>
/// Serialize a 32 bit signed value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(int32 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalInt(value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// Serialize a 32 bit unsigned value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(uint32 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// Serialize a 64 bit signed value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(int64 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalInt(value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// Serialize a 64 bit unsigned value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <returns>Serialized string value</returns>
string Serialize(uint64 value, int width, int base, bool showBase, bool leadingZeros)
{
    return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// Serialize a float value to string. The value will be printed as a fixed point number.
///
/// Width specifies the minimum width in characters. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Precision specifies the number of digits behind the decimal pointer
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="precision">Number of digits after the decimal point to use (limited to 7 decimals</param>
/// <returns>Serialized string value</returns>
string Serialize(float value, int width, int precision)
{
    bool negative{};
    if (value < 0)
    {
        negative = true;
        value = -value;
    }

    // We can only print values with integral parts up to what uint64 can hold
    if (value > static_cast<float>(static_cast<uint64>(-1)))
    {
        return string("overflow");
    }

    string result;
    if (negative)
        result += '-';

    uint64 integralPart = static_cast<uint64>(value);
    result += Serialize(integralPart, 0, 10, false, false);
    const int MaxPrecision = 7;

    if (precision != 0)
    {
        result += '.';

        if (precision > MaxPrecision)
        {
            precision = MaxPrecision;
        }

        uint64 precisionPower10 = 1;
        for (int i = 1; i <= precision; i++)
        {
            precisionPower10 *= 10;
        }

        value -= static_cast<float>(integralPart);
        value *= static_cast<float>(precisionPower10);

        string fractional = Serialize(static_cast<uint64>(value + 0.5F), 0, 10, false, false);
        result += fractional;
        precision -= fractional.length();
        while (precision--)
        {
            result += '0';
        }
    }
    return result.align(width);
}

/// <summary>
/// Serialize a double value to string. The value will be printed as a fixed point number.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Precision specifies the number of digits behind the decimal pointer
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="precision">Number of digits after the decimal point to use (limited to 10 decimals</param>
/// <returns>Serialized string value</returns>
string Serialize(double value, int width, int precision)
{
    bool negative{};
    if (value < 0)
    {
        negative = true;
        value = -value;
    }

    // We can only print values with integral parts up to what uint64 can hold
    if (value > static_cast<double>(static_cast<uint64>(-1)))
    {
        return string("overflow");
    }

    string result;
    if (negative)
        result += '-';

    uint64 integralPart = static_cast<uint64>(value);
    result += Serialize(integralPart, 0, 10, false, false);
    const int MaxPrecision = 14;

    if (precision != 0)
    {
        result += '.';

        if (precision > MaxPrecision)
        {
            precision = MaxPrecision;
        }

        uint64 precisionPower10 = 1;
        for (int i = 1; i <= precision; i++)
        {
            precisionPower10 *= 10;
        }

        value -= static_cast<double>(integralPart);
        value *= static_cast<double>(precisionPower10);

        string fractional = Serialize(static_cast<uint64>(value + 0.5), 0, 10, false, false);
        result += fractional;
        precision -= fractional.length();
        while (precision--)
        {
            result += '0';
        }
    }
    return result.align(width);
}

/// <summary>
/// Serialize a string to string.
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// If requested, the string is placed between double quotes (").
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="quote">If true places string between double quotes</param>
/// <returns>Serialized string value</returns>
string Serialize(const string& value, int width, bool quote)
{
    return Serialize(value.data(), width, quote);
}

/// <summary>
/// Serialize a string to string
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// If requested, the string is placed between double quotes (").
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <param name="quote">If true places string between double quotes</param>
/// <returns>Serialized string value</returns>
string Serialize(const char* value, int width, bool quote)
{
    string result;

    if (quote)
        result += '\"';
    result += value;
    if (quote)
        result += '\"';

    return result.align(width);
}

/// <summary>
/// Serialize a const void pointer to string
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <returns>Serialized string value</returns>
string Serialize(const void* value, int width)
{
    string result;

    if (value != nullptr)
    {
        result = Serialize(reinterpret_cast<uintptr>(value), 16, 16, true, true);
    }
    else
    {
        result = "null";
    }

    return result.align(width);
}

/// <summary>
/// Serialize a void pointer to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
/// <returns>Serialized string value</returns>
string Serialize(void* value, int width)
{
    return Serialize(const_cast<const void*>(value), width);
}

/// <summary>
/// Internal serialization function returning string, to be used for all signed values.
///
/// Serialize a signed value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <param name="numBits">Specifies the number of bits used for the value</param>
/// <returns>Serialized stirng</returns>
static string SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
{
    if ((base < 2) || (base > 36))
        return {};

    int       numDigits = 0;
    bool      negative = (value < 0);
    uint64    absVal = static_cast<uint64>(negative ? -value : value);
    uint64    divisor = 1;
    uint64    divisorHigh = 0;
    uint64    divisorLast = 1;
    size_t    absWidth = (width < 0) ? -width : width;
    const int maxDigits = BitsToDigits(numBits, base);
    while ((divisorHigh == 0) && (absVal >= divisor) && (numDigits <= maxDigits))
    {
        divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
        divisorLast = divisor;
        divisor *= base;
        ++numDigits;
    }
    divisor = divisorLast;

    size_t numChars = (numDigits > 0) ? numDigits : 1;
    if (showBase)
    {
        numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
    }
    if (negative)
    {
        numChars++;
    }
    if (absWidth > numChars)
        numChars = absWidth;
    // Leave one character for \0
    string result;
    result.reserve(numChars + 1);

    if (negative)
    {
        result += '-';
    }

    if (showBase)
    {
        if (base == 2)
        {
            result += "0b";
        }
        else if (base == 8)
        {
            result += '0';
        }
        else if (base == 16)
        {
            result += "0x";
        }
    }
    if (leadingZeros)
    {
        if (absWidth == 0)
            absWidth = maxDigits;
        for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
        {
            result += '0';
        }
    }
    else
    {
        if (numDigits == 0)
        {
            result += '0';
        }
    }
    while (numDigits > 0)
    {
        int digit = (absVal / divisor) % base;
        result += GetDigit(digit);
        --numDigits;
        divisor /= base;
    }
    return result.align(width);
}

/// <summary>
/// Internal serialization function returning string, to be used for all unsigned values.
///
/// Serialize a unsigned value to string.
///
/// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
/// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
///
/// Base is the digit base, which can range from 2 to 36.
/// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
/// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
/// <param name="numBits">Specifies the number of bits used for the value</param>
/// <returns>Serialized stirng</returns>
static string SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
{
    if ((base < 2) || (base > 36))
        return {};

    int       numDigits = 0;
    uint64    divisor = 1;
    uint64    divisorLast = 1;
    uint64    divisorHigh = 0;
    size_t    absWidth = (width < 0) ? -width : width;
    const int maxDigits = BitsToDigits(numBits, base);
    while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
    {
        divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
        divisorLast = divisor;
        divisor *= base;
        ++numDigits;
    }
    divisor = divisorLast;

    size_t numChars = (numDigits > 0) ? numDigits : 1;
    if (showBase)
    {
        numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
    }
    if (absWidth > numChars)
        numChars = absWidth;
    // Leave one character for \0
    string result;
    result.reserve(numChars + 1);

    if (showBase)
    {
        if (base == 2)
        {
            result += "0b";
        }
        else if (base == 8)
        {
            result += '0';
        }
        else if (base == 16)
        {
            result += "0x";
        }
    }
    if (leadingZeros)
    {
        if (absWidth == 0)
            absWidth = maxDigits;
        for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
        {
            result += '0';
        }
    }
    else
    {
        if (numDigits == 0)
        {
            result += '0';
        }
    }
    while (numDigits > 0)
    {
        int digit = (value / divisor) % base;
        result += GetDigit(digit);
        --numDigits;
        divisor /= base;
    }
    return result.align(width);
}

} // namespace baremetal
