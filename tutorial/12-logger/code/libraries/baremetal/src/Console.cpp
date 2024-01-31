//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : Console.cpp
//
// Namespace   : baremetal
//
// Class       : Console
//
// Description : Console writer with ANSI color support
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

#include <baremetal/Console.h>

#include <baremetal/Serialization.h>
#include <baremetal/Timer.h>
#include <baremetal/UART0.h>
#include <baremetal/UART1.h>
#include <baremetal/Util.h>

namespace baremetal {

static const char *GetAnsiColorCode(ConsoleColor color)
{
    switch (color)
    {
    case ConsoleColor::Black:
    case ConsoleColor::DarkGray:
        return "0";
    case ConsoleColor::Red:
    case ConsoleColor::BrightRed:
        return "1";
    case ConsoleColor::Green:
    case ConsoleColor::BrightGreen:
        return "2";
    case ConsoleColor::Yellow:
    case ConsoleColor::BrightYellow:
        return "3";
    case ConsoleColor::Blue:
    case ConsoleColor::BrightBlue:
        return "4";
    case ConsoleColor::Magenta:
    case ConsoleColor::BrightMagenta:
        return "5";
    case ConsoleColor::Cyan:
    case ConsoleColor::BrightCyan:
        return "6";
    case ConsoleColor::LightGray:
    case ConsoleColor::White:
        return "7";
    default:
        return 0;
    };
}

/// <summary>
/// Create a console linked to the specified character device. Note that the constructor is private, so GetConsole() is needed to instantiate the console
/// </summary>
/// <param name="device">Character device used for output</param>
Console::Console(CharDevice *device)
    : m_device{device}
{
}

/// <summary>
/// Change the attached device
/// </summary>
/// <param name="device">Character device to be set to for output</param>
void Console::AssignDevice(CharDevice *device)
{
    m_device = device;
}

/// <summary>
/// Set console foreground and background color (will output ANSI color codes)
/// </summary>
/// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
/// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
void Console::SetTerminalColor(ConsoleColor foregroundColor /*= ConsoleColor::Default*/, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
{
    Write("\033[0");
    if (foregroundColor != ConsoleColor::Default)
    {
        if (foregroundColor <= ConsoleColor::LightGray)
        {
            Write(";3");
        }
        else
        {
            Write(";9");
        }
        Write(GetAnsiColorCode(foregroundColor));
    }
    if (backgroundColor != ConsoleColor::Default)
    {
        if (backgroundColor <= ConsoleColor::LightGray)
        {
            Write(";4");
        }
        else
        {
            Write(";10");
        }
        Write(GetAnsiColorCode(backgroundColor));
    }
    Write("m");
}

/// <summary>
/// Reset console foreground and background back to original (will output ANSI color codes)
/// </summary>
void Console::ResetTerminalColor()
{
    SetTerminalColor();
}

/// <summary>
/// Write a string, using the specified foreground and background color
/// </summary>
/// <param name="str">String to be written</param>
/// <param name="foregroundColor">Foreground color to use. Default brings the color back to the color originally set</param>
/// <param name="backgroundColor">Background color to use. Default brings the color back to the color originally set</param>
void Console::Write(const char *str, ConsoleColor foregroundColor, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
{
    static volatile bool inUse{};

    while (inUse)
    {
        Timer::WaitMilliSeconds(1);
    }
    inUse = true;

    SetTerminalColor(foregroundColor, backgroundColor);
    Write(str);
    SetTerminalColor();

    inUse = false;
}

/// <summary>
/// Write a string without changing the foreground and background color
/// </summary>
/// <param name="str">String to be written</param>
void Console::Write(const char *str)
{
    while (*str)
    {
        // convert newline to carriage return + newline
        if (*str == '\n')
            WriteChar('\r');
        WriteChar(*str++);
    }
}

/// <summary>
/// Write an unsigned 8 bit integer value formatted as specified
/// </summary>
/// <param name="value">Value to be written</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
void Console::Write(uint8 value, int width, int base, bool showBase, bool leadingZeros)
{
    const size_t BufferSize = 256;
    char buffer[BufferSize];

    Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
    Write(buffer);
}

/// <summary>
/// Write an unsigned 32 bit integer value formatted as specified
/// </summary>
/// <param name="value">Value to be written</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
void Console::Write(uint32 value, int width, int base, bool showBase, bool leadingZeros)
{
    const size_t BufferSize = 256;
    char buffer[BufferSize];
    
    Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
    Write(buffer);
}

/// <summary>
/// Write an unsigned 64 bit integer value formatted as specified
/// </summary>
/// <param name="value">Value to be written</param>
/// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
/// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
/// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
/// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
void Console::Write(uint64 value, int width, int base, bool showBase, bool leadingZeros)
{
    const size_t BufferSize = 256;
    char buffer[BufferSize];

    Serialize(buffer, BufferSize, value, width, base, showBase, leadingZeros);
    Write(buffer);
}

static const size_t WordsPerRow     = 8;
static const size_t MaxDisplayWords = 2048;

/// <summary>
/// Write a const array of unsigned 32 bit values. 
/// Values are separated by spaces in rows of 8, each row prepended with the offset of the start of the row.
/// The maximum count of values shown is 2048.
/// </summary>
/// <param name="value">Pointer to array of values to be written</param>
/// <param name="size">Count of values in array</param>
void                Console::Write(const uint32 *value, size_t size)
{
    if (m_device == nullptr)
        return;
    if (value != nullptr)
    {
        Write("\r\n");
        size_t displayWords = (size < MaxDisplayWords) ? size : MaxDisplayWords;
        // cppcheck-suppress knownConditionTrueFalse
        for (size_t i = 0; i < displayWords; i += WordsPerRow)
        {
            Write(i, 0, 16, false, true);
            Write("  ");
            for (size_t j = 0; j < WordsPerRow; ++j)
            {
                if (j != 0)
                    WriteChar(' ');
                if ((i + j) < displayWords)
                    Write(value[i + j], 8, 16, false, true);
                else
                    Write("        ");
            }
            Write("\r\n");
        }
    }
    else
    {
        Write("null\r\n");
    }
}

/// <summary>
/// Write a non-const array of unsigned 32 bit values. 
/// Values are separated by spaces in rows of 8, each row prepended with the offset of the start of the row.
/// The maximum count of values shown is 2048.
/// </summary>
/// <param name="value">Pointer to array of values to be written</param>
/// <param name="size">Count of values in array</param>
void Console::Write(uint32 *value, size_t size)
{
    Write(reinterpret_cast<const uint32 *>(value), size);
}

static const size_t BytesPerRow     = 16;
static const size_t MaxDisplayBytes = 4096;

/// <summary>
/// Write a const byte array.
/// 
/// Bytes are separated by spaces in rows of 16, each row prepended with the offset of the start of the row.
/// At the end of every row, the bytes are also shown as characters, if printable, otherwise as '.'.
/// The maximum count of bytes shown is 4096.
/// </summary>
/// <param name="value">Pointer to array of bytes to be written</param>
/// <param name="size">Count of bytes in array</param>
void                Console::Write(const uint8 *value, size_t size)
{
    if (m_device == nullptr)
        return;
    if (value != nullptr)
    {
        Write("\r\n");
        size_t displayBytes = (size < MaxDisplayBytes) ? size : MaxDisplayBytes;
        // cppcheck-suppress knownConditionTrueFalse
        for (size_t i = 0; i < displayBytes; i += BytesPerRow)
        {
            Write(i, 8, 16, false, true);
            Write("  ");
            for (size_t j = 0; j < BytesPerRow; ++j)
            {
                if (j != 0)
                    WriteChar(' ');
                if ((i + j) < displayBytes)
                    Write(static_cast<uint8>(value[i + j]), 2, 16, false, true);
                else
                    Write("  ");
            }
            Write("  ");
            for (size_t j = 0; j < BytesPerRow; ++j)
            {
                if (j != 0)
                    WriteChar(' ');
                if ((i + j) < displayBytes)
                {
                    auto ch = value[i + j];
                    WriteChar(((ch >= 32) && (ch < 127)) ? static_cast<char>(ch) : '.');
                }
                else
                {
                    WriteChar(' ');
                }
            }
            Write("\r\n");
        }
    }
    else
    {
        Write("null\r\n");
    }
}

/// <summary>
/// Write a non-const byte array.
/// 
/// Bytes are separated by spaces in rows of 16, each row prepended with the offset of the start of the row.
/// At the end of every row, the bytes are also shown as characters, if printable, otherwise as '.'.
/// The maximum count of bytes shown is 4096.
/// </summary>
/// <param name="value">Pointer to array of bytes to be written</param>
/// <param name="size">Count of bytes in array</param>
void Console::Write(uint8 *value, size_t size)
{
    if (m_device == nullptr)
        return;
    Write(reinterpret_cast<const uint8 *>(value), size);
}

/// <summary>
/// Write a boolean value.
/// 
/// Will write "true" if the value is true, "false" otherwise
/// </summary>
/// <param name="value">Value to be written</param>
void Console::Write(bool value)
{
    if (m_device == nullptr)
        return;
    Write(value ? "true" : "false");
}

/// <summary>
/// Read a character
/// </summary>
/// <returns>Character received</returns>
char Console::ReadChar()
{
    char ch{};
    if (m_device != nullptr)
    {
        ch = m_device->Read();
    }
    return ch;
}

/// <summary>
/// Write a single character.
/// </summary>
/// <param name="ch">Character to be written</param>
void Console::WriteChar(char ch)
{
    if (m_device != nullptr)
    {
        m_device->Write(ch);
    }
}

/// <summary>
/// Retrieve the singleton console
///
/// Creates a static instance of Console, and returns a reference to it.
/// </summary>
/// <returns>A reference to the singleton console.</returns>
Console &GetConsole()
{
#if defined(BAREMETAL_CONSOLE_UART0)
    static UART0&  uart = GetUART0();
    static Console console(&uart);
#elif defined(BAREMETAL_CONSOLE_UART1)
    static UART1&  uart = GetUART1();
    static Console console(&uart);
#else
    static Console console(nullptr);
#endif
    return console;
}

} // namespace baremetal
