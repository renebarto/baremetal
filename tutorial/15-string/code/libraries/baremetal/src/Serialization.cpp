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
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
static bool           Uppercase = true;

static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);


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
/// Serialize a 8 bit unsigned value to buffer.
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
void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros)
{
    SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 8);
}

/// <summary>
/// Serialize a 32 bit signed value to buffer.
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
void Serialize(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
{
    SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// Serialize a 32 bit unsigned value to buffer.
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
void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
{
    SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// Serialize a 64 bit signed value to buffer.
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
void Serialize(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
{
    SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// Serialize a 64 bit unsigned value to buffer.
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
void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
{
    SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// Serialize a double value to buffer. The value will be printed as a fixed point number.
///
/// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
/// Width is currently unused.
/// Precision specifies the number of digits behind the decimal pointer
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Unused</param>
/// <param name="precision">Number of digits after the decimal point to use</param>
void Serialize(char* buffer, size_t bufferSize, double value, int width, int precision)
{
    bool negative{};
    if (value < 0)
    {
        negative = true;
        value = -value;
    }

    if (bufferSize == 0)
        return;

    // We can only print values with integral parts up to what uint64 can hold
    if (value > static_cast<double>(static_cast<uint64>(-1)))
    {
        strncpy(buffer, "overflow", bufferSize);
        return;
    }

    *buffer = '\0';
    if (negative)
        strncpy(buffer, "-", bufferSize);

    uint64 integralPart = static_cast<uint64>(value);
    const size_t TmpBufferSize = 32;
    char tmpBuffer[TmpBufferSize];
    Serialize(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
    strncat(buffer, tmpBuffer, bufferSize);
    const int MaxPrecision = 7;

    if (precision != 0)
    {
        strncat(buffer, ".", bufferSize);

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

        Serialize(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
        strncat(buffer, tmpBuffer, bufferSize);
        precision -= strlen(tmpBuffer);
        while (precision--)
        {
            strncat(buffer, "0", bufferSize);
        }
    }
}

/// <summary>
/// Serialize a const char * value to buffer. The value can be quoted.
///
/// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
/// If quote is true, the string is printed within double quotes (\")
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">String to be serialized</param>
/// <param name="width">Unused</param>
/// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
void Serialize(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
{
    size_t numChars = strlen(value);
    if (quote)
        numChars += 2;

    // Leave one character for \0
    if (numChars > bufferSize - 1)
        return;

    char* bufferPtr = buffer;

    if (quote)
        *bufferPtr++ = '\"';
    while (*value)
    {
        *bufferPtr++ = *value++;
    }
    if (quote)
        *bufferPtr++ = '\"';
}

/// <summary>
/// Internal serialization function, to be used for all signed values.
///
/// Serialize a signed value to buffer.
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
/// <param name="numBits">Specifies the number of bits used for the value</param>
static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
{
    if ((base < 2) || (base > 36))
        return;

    int       numDigits = 0;
    bool      negative = (value < 0);
    uint64    absVal = static_cast<uint64>(negative ? -value : value);
    uint64    divisor = 1;
    uint64    divisorLast = 1;
    size_t    absWidth = (width < 0) ? -width : width;
    const int maxDigits = BitsToDigits(numBits, base);
    while ((absVal >= divisor) && (numDigits <= maxDigits))
    {
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
    if (numChars > bufferSize - 1)
        return;

    char* bufferPtr = buffer;
    if (negative)
    {
        *bufferPtr++ = '-';
    }

    if (showBase)
    {
        if (base == 2)
        {
            *bufferPtr++ = '0';
            *bufferPtr++ = 'b';
        }
        else if (base == 8)
        {
            *bufferPtr++ = '0';
        }
        else if (base == 16)
        {
            *bufferPtr++ = '0';
            *bufferPtr++ = 'x';
        }
    }
    if (leadingZeros)
    {
        if (absWidth == 0)
            absWidth = maxDigits;
        for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
        {
            *bufferPtr++ = '0';
        }
    }
    else
    {
        if (numDigits == 0)
        {
            *bufferPtr++ = '0';
        }
    }
    while (numDigits > 0)
    {
        int digit = (absVal / divisor) % base;
        *bufferPtr++ = GetDigit(digit);
        --numDigits;
        divisor /= base;
    }
    *bufferPtr++ = '\0';
}

/// <summary>
/// Internal serialization function, to be used for all unsigned values.
///
/// Serialize a unsigned value to buffer.
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
/// <param name="numBits">Specifies the number of bits used for the value</param>
static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
{
    if ((base < 2) || (base > 36))
        return;

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
    if (numChars > bufferSize - 1)
        return;

    char* bufferPtr = buffer;

    if (showBase)
    {
        if (base == 2)
        {
            *bufferPtr++ = '0';
            *bufferPtr++ = 'b';
        }
        else if (base == 8)
        {
            *bufferPtr++ = '0';
        }
        else if (base == 16)
        {
            *bufferPtr++ = '0';
            *bufferPtr++ = 'x';
        }
    }
    if (leadingZeros)
    {
        if (absWidth == 0)
            absWidth = maxDigits;
        for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
        {
            *bufferPtr++ = '0';
        }
    }
    else
    {
        if (numDigits == 0)
        {
            *bufferPtr++ = '0';
        }
    }
    while (numDigits > 0)
    {
        int digit = (value / divisor) % base;
        *bufferPtr++ = GetDigit(digit);
        --numDigits;
        divisor /= base;
    }
    *bufferPtr++ = '\0';
}

} // namespace baremetal
