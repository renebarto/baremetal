//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780Device.h
//
// Namespace   : baremetal::display
//
// Class       : HD44780Device
//
// Description : HD44780 based 16x2 LCD generic display
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

#include <device/display/HD44780Device.h>

#include <baremetal/Logger.h>
#include <baremetal/Timer.h>
#include <stdlib/Util.h>

using namespace baremetal;

/// @file
/// HD44780 based 16x2 LCD generic display

/// @brief Define log name
LOG_MODULE("HD44780Device");

namespace device {

/// @brief Bit pattern used for Command or Instruction. This refers to the value of the RS pin
static const uint8 MODE_CMD  = (0x00);
/// @brief Bit pattern used for Data. This refers to the value of the RS pin
static const uint8 MODE_DATA = (0x01);

/// @brief Clear Display command
///
/// See [HD44780 (Dot Matrix Liquid Crystal Display Controller/Driver) documentation](pdf/HD44780.pdf), page 24/25 <br/>
/// Overwrite display with blank characters and reset cursor position. <br/>
/// Clear display <br/>
/// 0 0 0 0 0 0 0 1 <br/>
/// Clears entire display and sets DDRAM address 0 in address counter.
static const uint8 LCD_CLEARDISPLAY = (0x01);
/// @brief Home command
///
/// Set cursor to initial position and reset any shifting. <br/>
/// Return home <br/>
/// 0 0 0 0 0 0 1 * <br/>
/// Sets DDRAM address 0 in address counter. Also returns display from being shifted to original position. DDRAM contents remain unchanged.
static const uint8 LCD_RETURNHOME = (0x02);
/// @brief Entry mode set
///
/// Sets cursor move direction and specifies display shift. These operations are performed during data write and read. <br/>
/// Entry mode set <br/>
/// 0 0 0 0 0 1 I/D S <br/>
/// If S = 1, shift display right (I/D = 0) or left (I/D = 1) on write. <br/>
/// If S = 0, move cursor left (I/D = 0) or right (I/D = 1) on write.
static const uint8 LCD_ENTRYMODESET         = (0x04);
/// @brief Entry mode set, cursor move right / shift right on write
static const uint8 LCD_ENTRYMODE_MOVE_RIGHT = (0x02);
/// @brief Entry mode set, cursor move left / shift left on write
static const uint8 LCD_ENTRYMODE_MOVE_LEFT  = (0x00);
/// @brief Entry mode set, shift mode
static const uint8 LCD_ENTRYMODE_SHIFT      = (0x01);
/// @brief Entry mode set, cursor move mode
static const uint8 LCD_ENTRYMODE_NO_SHIFT   = (0x00);
/// @brief Display control
///
/// Sets cursor move direction and specifies display shift. These operations are performed during data write and read. <br/>
/// Display on/off control <br/>
/// 0 0 0 0 1 D C B <br/>
/// If D = 1, display is on, if D = 0, display is off <br/>
/// If C = 1, cursor is on, if C = 0, cursor is off <br/>
/// If B = 1, cursor is blinking, if B = 0, cursor is not blinking
static const uint8 LCD_DISPLAYCONTROL                 = (0x08);
/// @brief Display control, display on
static const uint8 LCD_DISPLAYCONTROL_ON              = (0x04);
/// @brief Display control, display off
static const uint8 LCD_DISPLAYCONTROL_OFF             = (0x00);
/// @brief Display control, cursor on
static const uint8 LCD_DISPLAYCONTROL_CURSOR_ON       = (0x02);
/// @brief Display control, cursor off
static const uint8 LCD_DISPLAYCONTROL_CURSOR_OFF      = (0x00);
/// @brief Display control, cursor blinking
static const uint8 LCD_DISPLAYCONTROL_CURSOR_BLINK    = (0x01);
/// @brief Display control, cursor not blinking
static const uint8 LCD_DISPLAYCONTROL_CURSOR_NO_BLINK = (0x00);
/// @brief Display control, cursor move or display shift
///
/// Cursor or display shift <br/>
/// 0 0 0 1 S/C R/L * * <br/>
/// Moves cursor and shifts display without changing DDRAM contents. <br/>
/// if S/C = 0, the cursor is moved to the left (R/L = 0) or right (R/L = 1) <br/>
/// if S/C = 1, the entire screen is moved to the left (R/L = 0) or right (R/L = 1)
static const uint8 LCD_CURSORSHIFT              = (0x10);
/// @brief Display shift
static const uint8 LCD_CURSORSHIFT_SCREEN       = (0x08);
/// @brief Display shift left
static const uint8 LCD_CURSORSHIFT_SCREEN_LEFT  = (0x08);
/// @brief Display shift right
static const uint8 LCD_CURSORSHIFT_SCREEN_RIGHT = (0x0C);
/// @brief Cursor move
static const uint8 LCD_CURSORSHIFT_CURSOR       = (0x00);
/// @brief Cursor move left
static const uint8 LCD_CURSORSHIFT_CURSOR_LEFT  = (0x00);
/// @brief Cursor move right
static const uint8 LCD_CURSORSHIFT_CURSOR_RIGHT = (0x04);
/// @brief Set display configuration
///
/// Write configuration to display <br/>
/// Function set <br/>
/// 0 0 1 DL N F * * <br/>
/// Sets interface data length (DL), number of display lines (N), and character font (F). <br/>
/// Needs to be done immediately after the initialization sequence and cannot be changed. <br/>
/// If DL = 1, set 8 bit mode, if DL = 0 set 4 bit mode (this is used for I2C devices and devices with only 4 data lanes) <br/>
/// If N = 1, set 2 (or more) line mode, if N = 0 set 1 line mode (this enables if desired 5x10 characters) <br/>
/// If F = 1, set 5x10 character size, if F = 0 set 5x8 character size. If N=1 only 5x8 character size is supported
static const uint8 LCD_FUNCTIONSET          = (0x20);
/// @brief Set 8 bit transfer mode
static const uint8 LCD_FUNCTIONSET_8BITMODE = (0x10);
/// @brief Set 4 bit transfer mode
static const uint8 LCD_FUNCTIONSET_4BITMODE = (0x00);
/// @brief Set display configuration to multiple lines
static const uint8 LCD_FUNCTIONSET_2LINE    = (0x08);
/// @brief Set display configuration to single line
static const uint8 LCD_FUNCTIONSET_1LINE    = (0x00);
/// @brief Set display font to 5x10 pixels (only for single line display)
static const uint8 LCD_FUNCTIONSET_5x10DOTS = (0x04);
/// @brief Set display font to 5x8 pixels (default for multiple line display)
static const uint8 LCD_FUNCTIONSET_5x8DOTS  = (0x00);
/// @brief Set CGRAM start address
///
/// Set CGRAM address <br/>
/// 0 1 ACG ACG ACG ACG ACG ACG <br/>
/// Sets CGRAM address. CGRAM data is sent and received after this setting. <br/>
/// Sets the initial address for data write, the address is incremented after each write.
static const uint8 LCD_SETCGRAMADDR = (0x40);
/// @brief Set DDRAM start address
///
/// Set DDRAM address <br/>
/// 1 ADD ADD ADD ADD ADD ADD ADD <br/>
/// Sets DDRAM address. DDRAM data is sent and received after this setting. <br/>
/// Sets the initial address for data write, the address is incremented after each write.
static const uint8 LCD_SETDDRAMADDR = (0x80);

/// <summary>
/// Constructor
/// </summary>
/// <param name="numColumns">Display size in number of columns (max. 40)</param>
/// <param name="numRows">Display size in number of rows (max. 4). Note that the total number of characters is limited to 80 (so e.g. max 4x20)</param>
/// <param name="characterSize">Size of characters (5x8 or 5x10 pixels, must be 5x8 pixels for multiline displays)</param>
/// <param name="dataMode">Specifies 4 or 8 bit mode</param>
HD44780Device::HD44780Device(uint8 numColumns, uint8 numRows, CharacterSize characterSize /*= CharacterSize::Size5x8*/,
                             DataMode dataMode /*= DataMode::LCD_4BITMODE*/)
    : m_numRows{numRows}
    , m_numColumns{numColumns}
    , m_dataMode{dataMode}
    , m_characterSize{characterSize}
    , m_lineMode{}
    , m_cursorMode{}
    , m_displayMode{}
    , m_cursorX{}
    , m_cursorY{}
    , m_rowOffsets{}
{
    if (numRows == 1)
    {
        m_lineMode = LCDLines::LinesSingle;
    }
    else if ((numRows >= 2) && (numRows <= 4))
    {
        m_lineMode = LCDLines::LinesMulti;
        // Force to 5x8 characters, 5x10 is not supported for more than 1 line.
        m_characterSize = CharacterSize::Size5x8;
    }
    assert((m_characterSize == CharacterSize::Size5x8) || (m_characterSize == CharacterSize::Size5x10));
    assert((numRows >= 1) && (numRows <= 4));
    for (uint8 i = 0; i < numRows; ++i)
    {
        if (i % 2 == 1)
            m_rowOffsets[i] = (i * 0x40); // 0x40 is the default offset for the next row when row is odd
        if (i >= 2)
            m_rowOffsets[i] += m_numColumns; // For rows 2 and 3, add the number of columns to the offset
    }
    assert((m_characterSize == CharacterSize::Size5x8) || (numRows == 1));
}

/// <summary>
/// Destructor
/// </summary>
HD44780Device::~HD44780Device()
{
    // Don't write anymore, as derived class is already destroyed
}

/// <summary>
/// Return number of display columns
/// </summary>
/// <returns>Number of display columns</returns>
uint8 HD44780Device::GetNumColumns() const
{
    return m_numColumns;
}

/// <summary>
/// Return number of display rows
/// </summary>
/// <returns>Number of display rows</returns>
uint8 HD44780Device::GetNumRows() const
{
    return m_numRows;
}

/// <summary>
/// Initializat the display
/// </summary>
void HD44780Device::Initialize()
{
    // Setup initial display configuration
    uint8 displayFunction = (m_dataMode == DataMode::Mode8Bit) ? LCD_FUNCTIONSET_8BITMODE : LCD_FUNCTIONSET_4BITMODE;
    displayFunction |= (m_lineMode == LCDLines::LinesMulti) ? LCD_FUNCTIONSET_2LINE : LCD_FUNCTIONSET_1LINE;
    displayFunction |= (m_characterSize == CharacterSize::Size5x10) ? LCD_FUNCTIONSET_5x10DOTS : LCD_FUNCTIONSET_5x8DOTS;

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
    WriteInstruction(LCD_FUNCTIONSET | displayFunction);
    Timer::WaitMicroSeconds(50);
    WriteInstruction(LCD_DISPLAYCONTROL | LCD_DISPLAYCONTROL_OFF);
    WriteInstruction(LCD_CLEARDISPLAY);
    WriteInstruction(LCD_ENTRYMODESET | LCD_ENTRYMODE_MOVE_RIGHT); // set move cursor right, do not shift display
}

/// <summary>
/// Switch backlight on or off
/// </summary>
/// <param name="on">If true, switch backlight on, otherwise switch backlight off</param>
void HD44780Device::SetBacklight(bool on)
{
    // Default implementation does not support backlight
    (void)on;
}

/// <summary>
/// Return true if backlight is on. By default, this is always false, unless the device actually supports backlight control.
/// </summary>
/// <returns>True if backlight is on, false otherwise</returns>
bool HD44780Device::IsBacklightOn() const
{
    // Default implementation does not support backlight
    return false;
}

/// <summary>
/// Return true if we configured the display to use 4 bit mode
/// </summary>
/// <returns>True if using 4 bit mode, false otherwise</returns>
bool HD44780Device::IsDataMode4Bits() const
{
    return m_dataMode == DataMode::Mode4Bit;
}

/// <summary>
/// Write single character to display
/// </summary>
/// <param name="value"></param>
void HD44780Device::Write(char value)
{
    // Write a raw character byte to the LCD16X2.
    WriteData(static_cast<uint8>(value));
    if (m_cursorX < m_numColumns - 1)
    {
        m_cursorX += 1;
    }
    else
    {
        // At end of line : go to left side next row. Wrap around to first row if on last row.
        m_cursorY = (m_cursorY + 1) % m_numRows;
        m_cursorX = 0;
    }
}

/// <summary>
/// Write a character at a specific position on the display
/// </summary>
/// <param name="posX"></param>
/// <param name="posY"></param>
/// <param name="ch"></param>
void HD44780Device::Write(unsigned posX, unsigned posY, char ch)
{
    SetCursorPosition(posX, posY);
    Write(ch);
}

/// <summary>
/// Write multiple characters to display
/// </summary>
/// <param name="text">Point to string to write to display</param>
/// <param name="count">Count of characters to write to display</param>
void HD44780Device::Write(const char *text, size_t count)
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
void HD44780Device::Write(const char *text)
{
    Write(text, strlen(text));
}

/// <summary>
/// Write multiple characters to display
/// </summary>
/// <param name="posX"></param>
/// <param name="posY"></param>
/// <param name="text">Point to string to write to display</param>
void HD44780Device::Write(unsigned posX, unsigned posY, const char *text)
{
    Write(text, strlen(text));
}

/// <summary>
/// Clear the display contents, and position the cursor home
/// </summary>
void HD44780Device::ClearDisplay()
{
    WriteInstruction(LCD_CLEARDISPLAY);
    Timer::WaitMicroSeconds(2000);
    Home();
}

/// <summary>
/// Move the cursor to the home position
/// </summary>
void HD44780Device::Home()
{
    WriteInstruction(LCD_RETURNHOME);
    Timer::WaitMicroSeconds(2000);
    m_cursorX = 0;
    m_cursorY = 0;
}

/// <summary>
/// Shift display contents horizontally
/// </summary>
/// <param name="amount">Amound to shift by. If negative shifts left by -amount characters, if positive shifts right by amount characters. A value of 0 means no change.</param>
void HD44780Device::ShiftDisplay(int amount)
{
    if (amount == 0)
        return;

    auto direction = (amount > 0) ? LCD_CURSORSHIFT_CURSOR_RIGHT : LCD_CURSORSHIFT_CURSOR_LEFT;
    if (amount < 0)
        amount = -amount;
    for (int i = 0; i < amount; ++i)
    {
        CursorOrDisplayShift(ShiftMode::DisplayShift, direction);
        Timer::WaitMicroSeconds(50);
    }
}

/// <summary>
/// Set display on or off
/// </summary>
/// <param name="on">If true switch display to show mode, otherwise switch to hide mode</param>
void HD44780Device::SetDisplayEnabled(bool on)
{
    m_displayMode = (on) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;
    UpdateDisplayControl();
}

/// <summary>
/// Set cursor mode
/// </summary>
/// <param name="mode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
void HD44780Device::SetCursorMode(CursorMode mode)
{
    m_cursorMode = mode;
    UpdateDisplayControl();
}

/// <summary>
/// Change the cursor position
/// </summary>
/// <param name="cursorX">New cursor column</param>
/// <param name="cursorY">New cursor row</param>
void HD44780Device::SetCursorPosition(unsigned cursorX, unsigned cursorY)
{
    SetDDRAM_Address(m_rowOffsets[cursorY] + cursorX);
    m_cursorX = cursorX;
    m_cursorY = cursorY;
}

/// <summary>
/// Define a user defined character
///
/// Only characters 0x80-0x87 are user defined. The font data is 8 lines, each line is one byte. Only 5x8 pixel
/// characters are supported for now.
/// </summary>
/// <param name="ch">Character to define</param>
/// <param name="FontData">Font data. For every line there is one byte, only the low order 5 bits are used</param>
void HD44780Device::DefineCharFont(char ch, const uint8 FontData[8])
{
    uint8 data = static_cast<uint8>(ch);
    if (data < 0x80 || data > 0x87)
    {
        return;
    }
    data -= 0x80;

    SetCGRAM_Address(data << 3);

    for (unsigned line = 0; line <= 7; line++)
    {
        WriteData(FontData[line] & 0x1F);
    }
}

/// <summary>
/// Set display mode and cursor mode
/// </summary>
/// <param name="displayMode">Display on (DisplayMode::Show) or off (DisplayMode::Hide)</param>
/// <param name="cursorMode">Cursor hidden (CursorMode::Hide), line cursor (CursorMode::Line) or blinking block cursor (CursorMode::Blink)</param>
void HD44780Device::DisplayControl(DisplayMode displayMode, CursorMode cursorMode)
{
    m_cursorMode  = cursorMode;
    m_displayMode = (displayMode == DisplayMode::Show) ? LCD_DISPLAYCONTROL_ON : LCD_DISPLAYCONTROL_OFF;

    UpdateDisplayControl();
}

/// <summary>
/// Write instruction to HD44780 display controller
/// </summary>
/// <param name="value">Instruction byte (see LCD_x)</param>
void HD44780Device::WriteInstruction(uint8 value)
{
    WriteByte(value, RegisterSelect::Cmd);
}

/// <summary>
/// Write data to HD44780 display controller
/// </summary>
/// <param name="value">Data byte, character to be written</param>
void HD44780Device::WriteData(uint8 value)
{
    WriteByte(value, RegisterSelect::Data);
}

/// <summary>
/// Write a byte to the display controller
/// </summary>
/// <param name="data">Data byte to send</param>
/// <param name="mode">If equal to RegisterSelect::Cmd, this is an instruction, if equal to RegisterSelect::Data, this is data</param>
void HD44780Device::WriteByte(uint8 data, RegisterSelect mode)
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

/// <summary>
/// Update the display control register with the current display mode and cursor mode
/// </summary>
void HD44780Device::UpdateDisplayControl()
{
    WriteInstruction(LCD_DISPLAYCONTROL | m_displayMode | ConvertCursorMode());
}

/// <summary>
/// Shift cursor or display contents horizontally
/// </summary>
/// <param name="mode">If equal to ShiftMode::DisplayShift, the display contents are shifted, if equal to ShiftMode::CursorMove move the cursor to a new location</param>
/// <param name="direction">Direction to move. If equal to LCD_CURSORSHIFT_CURSOR_RIGHT moves to the right, otherwise moves to the left.</param>
void HD44780Device::CursorOrDisplayShift(ShiftMode mode, uint8 direction)
{
    WriteInstruction(LCD_CURSORSHIFT | ((mode == ShiftMode::DisplayShift) ? LCD_CURSORSHIFT_SCREEN : LCD_CURSORSHIFT_CURSOR) | direction);
}

/// <summary>
/// Set dislay Character Generator RAM (CGRAM) address
///
/// Data written subsequently is written to CGRAM starting at the specified address, incrementing by 1 for each byte
/// written.
/// </summary>
/// <param name="address">New CGRAM address (6 bits)</param>
void HD44780Device::SetCGRAM_Address(uint8 address)
{
    // Set CGRAM address
    // 0 1 ACG ACG ACG ACG ACG ACG
    // Sets CGRAM address.CGRAM data is sent and received after this setting.
    WriteInstruction(LCD_SETCGRAMADDR | (address & 0x3F));
}

/// <summary>
/// Set Display Data RAM (DDRAM) address
///
/// Data written subsequently is written to DDRAM (visible display) starting at the specified address, incrementing by 1 for each byte
/// written.
/// </summary>
/// <param name="address">New display RAM address (7 bits)</param>
void HD44780Device::SetDDRAM_Address(uint8 address)
{
    // Set DDRAM address
    // 1 ADD ADD ADD ADD ADD ADD ADD
    // Sets DDRAM address. DDRAM data is sent and received after this setting.
    WriteInstruction(LCD_SETDDRAMADDR | (address & 0x7F));
}

/// <summary>
/// Convert the currently set cursor mode to display control register bits
/// </summary>
/// <returns>Cursor mode bits for display control register</returns>
uint8 HD44780Device::ConvertCursorMode() const
{
    return (m_cursorMode == CursorMode::Blink) ? LCD_DISPLAYCONTROL_CURSOR_BLINK
                                               : ((m_cursorMode == CursorMode::Line) ? LCD_DISPLAYCONTROL_CURSOR_ON : LCD_DISPLAYCONTROL_CURSOR_OFF);
}

} // namespace device
