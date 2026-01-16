//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : ITextDisplay.h
//
// Namespace   : device
//
// Class       : ITextDisplay
//
// Description : Generic LCD text display interface
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

#include "stdlib/Types.h"

namespace device
{

/// <summary>
/// Generic character matrix LCD display interface
/// </summary>
class ITextDisplay
{
  public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ITextDisplay() = default;

    /// <summary>
    /// Retrieve the number of display columns
    /// </summary>
    /// <returns>Number of display columns</returns>
    virtual uint8 GetNumColumns() const = 0;
    /// <summary>
    /// Retrieve the number of display rows
    /// </summary>
    /// <returns>Number of display rows</returns>
    virtual uint8 GetNumRows() const = 0;

    /// <summary>
    /// Initialize the device
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// Set backlight on or off
    /// </summary>
    /// <param name="on">If true backlight is switched on, if false it is switched off</param>
    virtual void SetBacklight(bool on) = 0;
    /// <summary>
    /// Returns the current backlight status
    /// </summary>
    /// <returns>True if backlight is on, false otherwise</returns>
    virtual bool IsBacklightOn() const = 0;

    /// <summary>
    /// Write a character to the display, and update the cursor location
    ///
    /// The character is written in the buffer, depending on the shift it may or may not be visible
    /// </summary>
    /// <param name="value">Character to write</param>
    virtual void Write(char value) = 0;
    /// <summary>
    /// Write text to display, and update the cursor location
    ///
    /// The characters are written in the buffer, depending on the shift they may or may not be visible
    /// </summary>
    /// <param name="text">Text to write</param>
    /// <param name="count">Number of characters to write</param>
    virtual void Write(const char *text, size_t count) = 0;
    /// <summary>
    /// Write text to display, and update the cursor location
    ///
    /// The characters are written in the buffer, depending on the shift they may or may not be visible
    /// </summary>
    /// <param name="text">Text to write</param>
    virtual void Write(const char *text) = 0;
};

} // namespace device
