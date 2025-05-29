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

#pragma once

#include <device/display/ILCDDevice.h>

/// @file
/// HD44780 based LCD character display (max 40x4 characters)

namespace device {

/// <summary>
/// Hitachi HD44780 based LCD controller
/// </summary>
class HD44780Device : public ILCDDevice
{
public:
    /// <summary>
    /// Selection of the register to address in the LCD controller
    /// </summary>
    enum class RegisterSelect
    {
        /// @brief Command register
        Cmd,
        /// @brief Data register
        Data,
    };
    /// <summary>
    /// Selection of data transfer mode. The HD44780 controller can be either used in 4 bit mode (using D7-D4) and 8 bit mode (using D7-D0)
    /// </summary>
    enum class DataMode : uint8
    {
        /// @brief 4 bit transfer mode
        Mode4Bit,
        /// @brief 8 bit transfer mode
        Mode8Bit,
    };
    /// <summary>
    /// Mode on writing
    /// </summary>
    enum class ShiftMode : uint8
    {
        /// @brief Shift display on write
        DisplayShift,
        /// @brief Move cursor on write
        CursorMove,
    };

private:
    /// <summary>
    /// Display configuration
    /// </summary>
    enum class LCDLines : uint8
    {
        /// @brief Single row display
        LinesSingle,
        /// @brief Multi row display
        LinesMulti,
    };
    /// @brief Number of rows on the display
    uint8         m_numRows;
    /// @brief Number of columns on the display
    uint8         m_numColumns;
    /// @brief Data transfer mode (4 or 8 bit)
    DataMode      m_dataMode;
    /// @brief Character size (5x8 or 5x10)
    CharacterSize m_characterSize;
    /// @brief Whether display has single or multiple lines
    LCDLines      m_lineMode;
    /// @brief Cursor mode (none, line, block)
    CursorMode    m_cursorMode;
    /// @brief Display mode (internal combination of display and cursor mode)
    uint8         m_displayMode;
    /// @brief Cursor position column
    uint8         m_cursorX;
    /// @brief Cursor position row
    uint8         m_cursorY;

    /// <summary>
    /// Row offsets for multi line display (max 4 lines)
    /// </summary>
    /// <returns>Row offsets for multi line display</returns>
    uint8         m_rowOffsets[4] __attribute__((aligned(8)));

public:
    HD44780Device(uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8,
                  DataMode dataMode = DataMode::Mode4Bit);

    virtual ~HD44780Device();

    uint8 GetNumColumns() const override;
    uint8 GetNumRows() const override;

    void Initialize() override;
    void SetBacklight(bool on) override;
    bool IsBacklightOn() const override;
    bool IsDataMode4Bits() const;

    void  Write(char value) override;
    void  Write(unsigned posX, unsigned posY, char ch) override;
    void  Write(const char *text, size_t count) override;
    void  Write(const char *text) override;
    void  Write(unsigned posX, unsigned posY, const char *text) override;

    void  ClearDisplay() override;
    void  Home() override;
    void  ShiftDisplay(int amount) override;
    void  SetDisplayEnabled(bool on) override;
    void  SetCursorMode(CursorMode mode) override;
    void  SetCursorPosition(unsigned cursorX, unsigned cursorY) override;

    void  DefineCharFont(char ch, const uint8 FontData[8]) override;

    void  DisplayControl(DisplayMode displayMode, CursorMode cursorMode);

protected:
    /// <summary>
    /// Write a half byte to the display
    ///
    /// This is used when using 4 bit mode. The upper 4 bits of the byte are used for data, the lower 4 bits contain the control bits (register select, read/write, enable, backlight) <br/>
    /// Bit 0: Instruction (0) or Data (1) <br/>
    /// Bit 1: Read (1) or write (0) <br/>
    /// Bit 2: Enable bit (will first be set on, then off) to latch data <br/>
    /// Bit 3: Backlight on (1) or off (0) <br/>
    /// Bit 4-7: 4 bits of the data to write (first the upper 4 bits, then the lower 4 bits are sent)
    /// </summary>
    /// <param name="data">Byte to be written (including control bits in lower 4 bits, data in upper 4 bits)</param>
    virtual void WriteHalfByte(uint8 data) = 0;

private:
    void         WriteInstruction(uint8 value);
    void         WriteData(uint8 value);
    void         WriteByte(uint8 data, RegisterSelect mode);
    void         UpdateDisplayControl();
    void         CursorOrDisplayShift(ShiftMode mode, uint8 direction);
    void         SetCGRAM_Address(uint8 address);
    void         SetDDRAM_Address(uint8 address);
    uint8        ConvertCursorMode() const;
};

} // namespace device
