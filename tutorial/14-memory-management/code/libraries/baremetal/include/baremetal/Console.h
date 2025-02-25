//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Console.h
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

#pragma once

#include <stdlib/Types.h>
#include <baremetal/CharDevice.h>

/// @file
/// Console
///
/// Used to write logging information. Supports use of ANSI color coding

namespace baremetal {

/// @brief  Console color (foreground or background)
enum class ConsoleColor
{
    /// @brief Default brings the color back to the color originally set
    Default = -1,
    /// @brief Black color
    Black   = 0,
    /// @brief Red color
    Red,
    /// @brief Green color
    Green,
    /// @brief Yellow color
    Yellow,
    /// @brief Blue color
    Blue,
    /// @brief Magenta color
    Magenta,
    /// @brief Cyan color
    Cyan,
    /// @brief Light gray color
    LightGray,
    /// @brief Dark gray color (light version of Black)
    DarkGray,
    /// @brief Bright red color (light version of Red)
    BrightRed,
    /// @brief Bright green color (light version of Green)
    BrightGreen,
    /// @brief Bright yellow color (light version of Yellow)
    BrightYellow,
    /// @brief Bright blue color (light version of Blue)
    BrightBlue,
    /// @brief Bright magenta color (light version of Magenta)
    BrightMagenta,
    /// @brief Bright cyan color (light version of Cyan)
    BrightCyan,
    /// @brief White color (light version of LightGray)
    White,
};

/// @brief Class to output to the console.
///
/// This is a singleton, in that it is not possible to create a default instance (GetConsole() needs to be used for this)
class Console
{
    /// <summary>
    /// Construct the singleton Console instance if needed, and return a reference to the instance. This is a friend function of class Console
    /// </summary>
    /// <returns>Reference to the singleton Console instance</returns>
    friend Console &GetConsole();

private:
    /// @brief Character device to write to
    CharDevice *m_device;

    explicit Console(CharDevice *device);

public:
    void AssignDevice(CharDevice *device);
    void SetTerminalColor(ConsoleColor foregroundColor = ConsoleColor::Default, ConsoleColor backgroundColor = ConsoleColor::Default);
    void ResetTerminalColor();

    void Write(const char *str, ConsoleColor foregroundColor, ConsoleColor backgroundColor = ConsoleColor::Default);
    void Write(const char *str);

    char ReadChar();
    void WriteChar(char ch);

private:
};

Console &GetConsole();

} // namespace baremetal
