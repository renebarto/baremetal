//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Format.cpp
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : String formatting using standard argument handling
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

#include <baremetal/Format.h>

#include <baremetal/Serialization.h>
#include <baremetal/String.h>
#include <baremetal/Util.h>

/// @file
/// Formatting functionality implementation

namespace baremetal {

/// @brief Write characters with base above 10 as uppercase or not
static bool           Uppercase = true;

static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);

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
/// PrintValue a 32 bit signed value to buffer.
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
static void PrintValue(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
{
    PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// PrintValue a 32 bit unsigned value to buffer.
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
static void PrintValue(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
{
    PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
}

/// <summary>
/// PrintValue a 64 bit signed value to buffer.
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
static void PrintValue(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
{
    PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// PrintValue a 64 bit unsigned value to buffer.
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
static void PrintValue(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
{
    PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
}

/// <summary>
/// PrintValue long long int value, type specific specialization
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
inline static void PrintValue(char* buffer, size_t bufferSize, long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
{
    PrintValue(buffer, bufferSize, static_cast<int64>(value), width, base, showBase, leadingZeros);
}
/// <summary>
/// PrintValue unsigned long long int value, type specific specialization
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">Value to be serialized</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
inline static void PrintValue(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
{
    PrintValue(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
}

/// <summary>
/// PrintValue a double value to buffer. The value will be printed as a fixed point number.
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
static void PrintValue(char* buffer, size_t bufferSize, double value, int width, int precision)
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
    PrintValue(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
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

        PrintValue(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
        strncat(buffer, tmpBuffer, bufferSize);
        precision -= strlen(tmpBuffer);
        while (precision--)
        {
            strncat(buffer, "0", bufferSize);
        }
    }
}

/// <summary>
/// PrintValue a const char * value to buffer. The value can be quoted.
///
/// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
/// If quote is true, the string is printed within double quotes (\")
/// </summary>
/// <param name="buffer">Pointer to buffer receiving the characters written</param>
/// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
/// <param name="value">String to be serialized</param>
/// <param name="width">Unused</param>
/// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
static void PrintValue(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
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
/// PrintValue a signed value to buffer.
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
static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
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
/// PrintValue a unsigned value to buffer.
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
static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
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

/// <summary>
/// Size of buffer used by FormatNoAllocV internally
/// </summary>
static const size_t BufferSize = 4096;

/// <summary>
/// Append a character to the buffer
/// </summary>
/// <param name="buffer">Buffer to write to</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="c">Character to append</param>
static void Append(char* buffer, size_t bufferSize, char c)
{
    size_t len = strlen(buffer);
    char* p = buffer + len;
    if (static_cast<size_t>(p - buffer) < bufferSize)
    {
        *p++ = c;
    }
    if (static_cast<size_t>(p - buffer) < bufferSize)
    {
        *p = '\0';
    }
}

/// <summary>
/// Append a set of identical characters to the buffer
/// </summary>
/// <param name="buffer">Buffer to write to</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="count">Number of characters to append</param>
/// <param name="c">Character to append</param>
static void Append(char* buffer, size_t bufferSize, size_t count, char c)
{
    size_t len = strlen(buffer);
    char* p = buffer + len;
    while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
    {
        *p++ = c;
        --count;
    }
    if (static_cast<size_t>(p - buffer) < bufferSize)
    {
        *p = '\0';
    }
}

/// <summary>
/// Append a string to the buffer
/// </summary>
/// <param name="buffer">Buffer to write to</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="str">String to append</param>
static void Append(char* buffer, size_t bufferSize, const char* str)
{
    strncat(buffer, str, bufferSize);
}

/// <summary>
/// Format a string
///
/// This version of Format uses the string class, and thus allocates memory
/// </summary>
/// <param name="format">Format string (printf like)</param>
/// <returns>Resulting string</returns>
string Format(const char* format, ...)
{
    va_list var;
    va_start(var, format);

    string result = FormatV(format, var);

    va_end(var);

    return result;
}

/// <summary>
/// Format a string
///
/// This version of FormatV uses the string class, and thus allocates memory
/// </summary>
/// <param name="format">Format string (printf like)</param>
/// <param name="args">Variable argument list</param>
/// <returns>Resulting string</returns>
string FormatV(const char* format, va_list args)
{
    string result;

    while (*format != '\0')
    {
        if (*format == '%')
        {
            if (*++format == '%')
            {
                result += '%';
                format++;
                continue;
            }

            bool alternate = false;
            if (*format == '#')
            {
                alternate = true;
                format++;
            }

            bool left = false;
            if (*format == '-')
            {
                left = true;
                format++;
            }

            bool leadingZero = false;
            if (*format == '0')
            {
                leadingZero = true;
                format++;
            }

            size_t width = 0;
            while (('0' <= *format) && (*format <= '9'))
            {
                width = width * 10 + (*format - '0');
                format++;
            }

            unsigned precision = 6;
            if (*format == '.')
            {
                format++;
                precision = 0;
                while ('0' <= *format && *format <= '9')
                {
                    precision = precision * 10 + (*format - '0');

                    format++;
                }
            }

            bool haveLong{};
            bool haveLongLong{};

            if (*format == 'l')
            {
                if (*(format + 1) == 'l')
                {
                    haveLongLong = true;

                    format++;
                }
                else
                {
                    haveLong = true;
                }

                format++;
            }

            switch (*format)
            {
            case 'c':
            {
                char ch = static_cast<char>(va_arg(args, int));
                if (left)
                {
                    result += ch;
                    if (width > 1)
                    {
                        result.append(width - 1, ' ');
                    }
                }
                else
                {
                    if (width > 1)
                    {
                        result.append(width - 1, ' ');
                    }
                    result += ch;
                }
            }
            break;

            case 'd':
            case 'i':
                if (haveLongLong)
                {
                    result.append(Serialize(va_arg(args, int64), left ? -width : width, 10, false, leadingZero));
                }
                else if (haveLong)
                {
                    result.append(Serialize(va_arg(args, int32), left ? -width : width, 10, false, leadingZero));
                }
                else
                {
                    result.append(Serialize(va_arg(args, int), left ? -width : width, 10, false, leadingZero));
                }
                break;

            case 'f':
            {
                result.append(Serialize(va_arg(args, double), left ? -width : width, precision));
            }
            break;

            case 'b':
                if (alternate)
                {
                    result.append("0b");
                }
                if (haveLongLong)
                {
                    result.append(Serialize(va_arg(args, uint64), left ? -width : width, 2, false, leadingZero));
                }
                else if (haveLong)
                {
                    result.append(Serialize(va_arg(args, uint32), left ? -width : width, 2, false, leadingZero));
                }
                else
                {
                    result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero));
                }
                break;

            case 'o':
                if (alternate)
                {
                    result.append("0");
                }
                if (haveLongLong)
                {
                    result.append(Serialize(va_arg(args, uint64), left ? -width : width, 8, false, leadingZero));
                }
                else if (haveLong)
                {
                    result.append(Serialize(va_arg(args, uint32), left ? -width : width, 8, false, leadingZero));
                }
                else
                {
                    result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero));
                }
                break;

            case 's':
            {
                result.append(Serialize(va_arg(args, const char*), left ? -width : width, false));
            }
            break;

            case 'u':
                if (haveLongLong)
                {
                    result.append(Serialize(va_arg(args, uint64), left ? -width : width, 10, false, leadingZero));
                }
                else if (haveLong)
                {
                    result.append(Serialize(va_arg(args, uint32), left ? -width : width, 10, false, leadingZero));
                }
                else
                {
                    result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero));
                }
                break;

            case 'x':
            case 'X':
                if (alternate)
                {
                    result.append("0x");
                }
                if (haveLongLong)
                {
                    result.append(Serialize(va_arg(args, uint64), left ? -width : width, 16, false, leadingZero));
                }
                else if (haveLong)
                {
                    result.append(Serialize(va_arg(args, uint32), left ? -width : width, 16, false, leadingZero));
                }
                else
                {
                    result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero));
                }
                break;

            case 'p':
                if (alternate)
                {
                    result.append("0x");
                }
                {
                    result.append(Serialize(va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero));
                }
                break;

            default:
                result += '%';
                result += *format;
                break;
            }
        }
        else
        {
            result += *format;
        }

        format++;
    }

    return result;
}

/// <summary>
/// Format a string
///
/// This version of Format writes directly to a buffer, and does not allocate memory
/// </summary>
/// <param name="buffer">Buffer to write to</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="format">Format string (printf like)</param>
void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...)
{
    va_list var;
    va_start(var, format);

    FormatNoAllocV(buffer, bufferSize, format, var);

    va_end(var);
}

/// <summary>
/// Format a string
///
/// This version of FormatV writes directly to a buffer, and does not allocate memory
/// </summary>
/// <param name="buffer">Buffer to write to</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="format">Format string (printf like)</param>
/// <param name="args">Variable argument list</param>
void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args)
{
    buffer[0] = '\0';

    while (*format != '\0')
    {
        if (*format == '%')
        {
            if (*++format == '%')
            {
                Append(buffer, bufferSize, '%');
                format++;
                continue;
            }

            bool alternate = false;
            if (*format == '#')
            {
                alternate = true;
                format++;
            }

            bool left = false;
            if (*format == '-')
            {
                left = true;
                format++;
            }

            bool leadingZero = false;
            if (*format == '0')
            {
                leadingZero = true;
                format++;
            }

            size_t width = 0;
            while (('0' <= *format) && (*format <= '9'))
            {
                width = width * 10 + (*format - '0');
                format++;
            }

            unsigned precision = 6;
            if (*format == '.')
            {
                format++;
                precision = 0;
                while ('0' <= *format && *format <= '9')
                {
                    precision = precision * 10 + (*format - '0');

                    format++;
                }
            }

            bool haveLong{};
            bool haveLongLong{};

            if (*format == 'l')
            {
                if (*(format + 1) == 'l')
                {
                    haveLongLong = true;

                    format++;
                }
                else
                {
                    haveLong = true;
                }

                format++;
            }

            switch (*format)
            {
            case 'c':
                {
                    char ch = static_cast<char>(va_arg(args, int));
                    if (left)
                    {
                        Append(buffer, bufferSize, ch);
                        if (width > 1)
                        {
                            Append(buffer, bufferSize, width - 1, ' ');
                        }
                    }
                    else
                    {
                        if (width > 1)
                        {
                            Append(buffer, bufferSize, width - 1, ' ');
                        }
                        Append(buffer, bufferSize, ch);
                    }
                }
                break;

            case 'd':
            case 'i':
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'f':
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'b':
                if (alternate)
                {
                    Append(buffer, bufferSize, "0b");
                }
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'o':
                if (alternate)
                {
                    Append(buffer, bufferSize, '0');
                }
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 's':
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'u':
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'x':
            case 'X':
                if (alternate)
                {
                    Append(buffer, bufferSize, "0x");
                }
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'p':
                if (alternate)
                {
                    Append(buffer, bufferSize, "0x");
                }
                {
                    char str[BufferSize]{};
                    PrintValue(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            default:
                Append(buffer, bufferSize, '%');
                Append(buffer, bufferSize, *format);
                break;
            }
        }
        else
        {
            Append(buffer, bufferSize, *format);
        }

        format++;
    }
}

} // namespace baremetal
