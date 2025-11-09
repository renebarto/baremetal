//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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

#include "baremetal/Console.h"

#include "baremetal/Serialization.h"
#include "baremetal/Timer.h"
#include "baremetal/UART0.h"
#include "baremetal/UART1.h"
#include "stdlib/Util.h"

/// @file
/// Console implementation

namespace baremetal {

/// <summary>
/// Determine ANSI color string for specified color
/// </summary>
/// <param name="color">ANSI color to use</param>
/// <returns>ANSI color string</returns>
static const char* GetAnsiColorCode(ConsoleColor color)
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

Console* Console::s_instance{};

/// <summary>
/// Create a console linked to the specified character device. Note that the constructor is private, so GetConsole() is needed to instantiate the
/// console
/// </summary>
/// <param name="device">Character device used for output</param>
Console::Console(Device* device)
    : m_defaultDevice{device}
    , m_device{device}
{
    s_instance = this;
}

/// <summary>
/// Change the attached device
/// </summary>
/// <param name="device">Character device to be set to for output</param>
void Console::AssignDevice(Device* device)
{
    m_device = device;
    if (device == nullptr)
        m_device = m_defaultDevice;
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
void Console::Write(const char* str, ConsoleColor foregroundColor, ConsoleColor backgroundColor /*= ConsoleColor::Default*/)
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
void Console::Write(const char* str)
{
    if (m_device != nullptr)
    {
        m_device->Write(str, strlen(str));
    }
}

/// <summary>
/// Flush the device buffers
/// </summary>
void Console::Flush()
{
    if (m_device != nullptr)
    {
        m_device->Flush();
    }
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
        m_device->Read(&ch, 1);
    }
    return ch;
}

/// Write a single character.
/// </summary>
/// <param name="ch">Character to be written</param>
void Console::WriteChar(char ch)
{
    if (m_device != nullptr)
    {
        m_device->Write(&ch, 1);
    }
}

/// <summary>
/// Retrieve the singleton console
///
/// Creates a static instance of Console, and returns a reference to it.
/// </summary>
/// <returns>A reference to the singleton console.</returns>
Console& GetConsole()
{
    return *Console::s_instance;
}

} // namespace baremetal
