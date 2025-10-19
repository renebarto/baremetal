//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "baremetal/Format.h"

#include "baremetal/Serialization.h"
#include "stdlib/Util.h"

namespace baremetal {

/// @brief Size of buffer for destination
const size_t BufferSize = 4096;

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
/// Print a formatted string to a buffer
///
/// This uses variable arguments
/// </summary>
/// <param name="buffer">Pointer to destination buffer</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="format">Format string</param>
void Format(char* buffer, size_t bufferSize, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    FormatV(buffer, bufferSize, format, args);

    va_end(args);
}

/// <summary>
/// Print a formatted string to a buffer
///
/// This uses a variable argument list
/// </summary>
/// <param name="buffer">Pointer to destination buffer</param>
/// <param name="bufferSize">Size of the buffer</param>
/// <param name="format">Format string</param>
/// <param name="args">Variable arguments list</param>
void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args)
{
    if (buffer == nullptr)
        return;
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
                    Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'f':
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
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
                    Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
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
                    Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 's':
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
                    Append(buffer, bufferSize, str);
                }
                break;

            case 'u':
                if (haveLongLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
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
                    Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else if (haveLong)
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
                    Append(buffer, bufferSize, str);
                }
                else
                {
                    char str[BufferSize]{};
                    Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
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
                    Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
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
