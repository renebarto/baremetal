//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : HD44780Display.cpp
//
// Namespace   : device
//
// Class       : HD44780Display
//
// Description : HD44780 based LCD generic display (max 40x4)
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

#include "device/display/HD44780Display.h"

#include "baremetal/Logger.h"
#include "baremetal/Timer.h"
#include "stdlib/Util.h"

using namespace baremetal;

/// @file
/// HD44780 based I2C LCD character display (max 40x4 characters)

/// @brief Define log name
LOG_MODULE("HD44780Display");

namespace device {

/// @brief Bit pattern used for Command or Instruction. This refers to the value of the RS pin
static const uint8 MODE_CMD  = (0x00);
/// @brief Bit pattern used for Data. This refers to the value of the RS pin
static const uint8 MODE_DATA = (0x01);

/// <summary>
/// Constructor
/// </summary>
HD44780Display::HD44780Display(uint8 numColumns, uint8 numRows, DataMode dataMode /*= DataMode::Mode4Bit*/)
    : m_numRows{numRows}
    , m_numColumns{numColumns}
    , m_dataMode{dataMode}
{
    assert((numRows >= 1) && (numRows <= 4));
}

/// <summary>
/// Destructor
/// </summary>
HD44780Display::~HD44780Display()
{
    // Don't write anymore, as derived class is already destroyed
}

/// <summary>
/// Return number of display columns
/// </summary>
/// <returns>Number of display columns</returns>
uint8 HD44780Display::GetNumColumns() const
{
    return m_numColumns;
}

/// <summary>
/// Return number of display rows
/// </summary>
/// <returns>Number of display rows</returns>
uint8 HD44780Display::GetNumRows() const
{
    return m_numRows;
}

/// <summary>
/// Initialize the display
/// </summary>
void HD44780Display::Initialize()
{
    // Setup initial display configuration
    // Initialization sequence (see page 45 / 46 of Hitachi HD44780 display controller datasheet
    // Choose 4 or 8 bit mode
    if (IsDataMode4Bits())
    {
        WriteHalfByte(0x30);
        Timer::WaitMicroSeconds(4500);
        WriteHalfByte(0x30);
        Timer::WaitMicroSeconds(100);
        WriteHalfByte(0x30);
        // See page 46 Hitachi HD44780 display controller datasheet
        WriteHalfByte(0x20);
    }
    else
    {
        WriteInstruction(0x03);
        Timer::WaitMicroSeconds(4500);
        WriteInstruction(0x03);
        Timer::WaitMicroSeconds(4500);
        WriteInstruction(0x03);
    }

    SetBacklight(true);
}

/// <summary>
/// Switch backlight on or off
/// </summary>
/// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
void HD44780Display::SetBacklight(bool on)
{
    // Default implementation does not support backlight
    (void)on;
}

/// <summary>
/// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
/// </summary>
/// <returns>True if backlight is on, false otherwise</returns>
bool HD44780Display::IsBacklightOn() const
{
    // Default implementation does not support backlight
    return false;
}

/// <summary>
/// Write single character to display
/// </summary>
/// <param name="value"></param>
void HD44780Display::Write(char value)
{
    // Write a raw character byte to the LCD16X2.
    WriteData(static_cast<uint8>(value));
}

/// <summary>
/// Write multiple characters to display
/// </summary>
/// <param name="text">Point to string to write to display</param>
/// <param name="count">Count of characters to write to display</param>
void HD44780Display::Write(const char *text, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        Write(text[i]);
    }
}

/// <summary>
/// Write multiple characters to display
/// </summary>
/// <param name="text">Point to string to write to display</param>
void HD44780Display::Write(const char *text)
{
    Write(text, strlen(text));
}

/// <summary>
/// Return true if we configured the display to use 4 bit mode
/// </summary>
/// <returns>True if using 4 bit mode, false otherwise</returns>
bool HD44780Display::IsDataMode4Bits() const
{
    return m_dataMode == DataMode::Mode4Bit;
}

/// <summary>
/// Write instruction to HD44780 display controller
/// </summary>
/// <param name="value">Instruction byte (see LCD_x)</param>
void HD44780Display::WriteInstruction(uint8 value)
{
    WriteByte(value, RegisterSelect::Cmd);
}

/// <summary>
/// Write data to HD44780 display controller
/// </summary>
/// <param name="value">Data byte, character to be written</param>
void HD44780Display::WriteData(uint8 value)
{
    WriteByte(value, RegisterSelect::Data);
}

/// <summary>
/// Write a byte to the display controller
/// </summary>
/// <param name="data">Data byte to send</param>
/// <param name="mode">If equal to RegisterSelect::Cmd, this is an instruction, if equal to RegisterSelect::Data, this is data</param>
void HD44780Display::WriteByte(uint8 data, RegisterSelect mode)
{
    uint8 modeBits = (mode == RegisterSelect::Data) ? MODE_DATA : MODE_CMD;
    //LOG_INFO("Write %s byte %02x", (mode == RegisterSelect::Cmd ? "instruction" : "data"), data);
    if (m_dataMode == DataMode::Mode4Bit)
    {
        WriteHalfByte(modeBits | (data & 0xF0));
        WriteHalfByte(modeBits | ((data << 4) & 0xF0));
    }
    else
    {
        assert(false);
    }
}

} // namespace device
