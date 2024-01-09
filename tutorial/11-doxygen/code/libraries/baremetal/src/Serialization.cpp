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

#include <baremetal/Serialization.h>

namespace baremetal {

static bool           Uppercase = true;

inline constexpr int NextPowerOf2Bits(size_t value)
{
    int bitCount{ 0 };
    size_t temp = value;
    while (temp >= 1)
    {
        ++bitCount;
        temp >>= 1;
    }
    return bitCount;
}

static constexpr char GetDigit(uint8 value)
{
    // cppcheck-suppress knownConditionTrueFalse
    return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
}

static constexpr int BitsToDigits(int bits, int base)
{
    int baseBits = NextPowerOf2Bits(base - 1);
    return (bits + baseBits - 1) / baseBits;
}

void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
{
    if ((base < 2) || (base > 36))
        return;

    int       numDigits = 0;
    uint64    divisor   = 1;
    size_t    absWidth  = (width < 0) ? -width : width;
    const int numBits   = 32;
    while ((value >= divisor) && (numDigits <= BitsToDigits(numBits, base)))
    {
        divisor *= base;
        ++numDigits;
    }

    size_t numChars = (numDigits > 0) ? numDigits : 1;
    if (showBase)
    {
        numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
    }
    if (absWidth > numChars)
        numChars = absWidth;
    if (numChars > bufferSize - 1) // Leave one character for \0
        return;

    char* bufferPtr = buffer;

    switch (base)
    {
    case 10:
        {
            if (leadingZeros)
            {
                if (absWidth == 0)
                    absWidth = BitsToDigits(numBits, base);
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
                divisor /= base;
                int digit = (value / divisor) % base;
                *bufferPtr++ = GetDigit(digit);
                --numDigits;
            }
        }
        break;
    default:
        {
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
                    absWidth = BitsToDigits(numBits, base);
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
                divisor /= base;
                int digit = (value / divisor) % base;
                *bufferPtr++ = GetDigit(digit);
                --numDigits;
            }
        }
        break;
    }
    *bufferPtr++ = '\0';
}

void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
{
    if ((base < 2) || (base > 36))
        return;

    int       numDigits = 0;
    uint64    divisor = 1;
    size_t    absWidth = (width < 0) ? -width : width;
    const int numBits = 64;
    while ((value >= divisor) && (numDigits <= BitsToDigits(numBits, base)))
    {
        divisor *= base;
        ++numDigits;
    }

    size_t numChars = (numDigits > 0) ? numDigits : 1;
    if (showBase)
    {
        numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
    }
    if (absWidth > numChars)
        numChars = absWidth;
    if (numChars > bufferSize - 1) // Leave one character for \0
        return;

    char* bufferPtr = buffer;

    switch (base)
    {
    case 10:
    {
        if (leadingZeros)
        {
            if (absWidth == 0)
                absWidth = BitsToDigits(numBits, base);
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
            divisor /= base;
            int digit = (value / divisor) % base;
            *bufferPtr++ = GetDigit(digit);
            --numDigits;
        }
    }
    break;
    default:
    {
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
                absWidth = BitsToDigits(numBits, base);
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
            divisor /= base;
            int digit = (value / divisor) % base;
            *bufferPtr++ = GetDigit(digit);
            --numDigits;
        }
    }
    break;
    }
    *bufferPtr++ = '\0';
}

} // namespace baremetal
