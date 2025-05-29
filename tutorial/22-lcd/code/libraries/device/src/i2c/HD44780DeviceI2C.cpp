//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DeviceI2C.h
//
// Namespace   : baremetal::display
//
// Class       : HD44780DeviceI2C
//
// Description : HD44780 based 16x2 LCD display with I2C piggyback
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

#include <device/i2c/HD44780DeviceI2C.h>

#include <baremetal/I2CMaster.h>
#include <baremetal/Timer.h>

using namespace baremetal;

/// @file
/// HD44780 based I2C 16x2 LCD display

namespace device {

// CharLCD via PCF8574 I2C port expander (for 16X2 LCD display
//  Pin mapping::
//  7  | 6  | 5  | 4  | 3  | 2  | 1  | 0
//  D3 | D2 | D1 | D0 | BK | EN | RW | RS
//
//  D3 : D7 (first write) and D3 (second write)
//  D2 : D6 (first write) and D2 (second write)
//  D1 : D5 (first write) and D1 (second write)
//  D0 : D4 (first write) and D0 (second write)
//  BK : Backlight off (0) or on (1)
//  EN : Enable. Needs to be strobed high to write
//  RW : Read (0) or Write (1)
//  RS : Instruction (0) or Data (1)

// PCF8574 I2C multiplexer signal mapping to HD44780 display
/// @brief RS pin is bit 0
static const uint8 PCF_RS = BIT1(0); // RS pin
/// @brief RW pin is bit 1
static const uint8 PCF_RW = BIT1(1); // RW pin
/// @brief EN pin is bit 2
static const uint8 PCF_EN = BIT1(2); // EN pin
/// @brief Backlight pin is bit 3
static const uint8 PCF_BK = BIT1(3); // Backlight pin

// Flags for RS pin modes
/// @brief Instruction register select (RS pin low)
static const uint8 RS_INSTRUCTION = (0x00);
/// @brief Data register select (RS pin high)
static const uint8 RS_DATA        = PCF_RS;

// Flags for backlight control
/// @brief Backlight on
const uint8 LCD_BACKLIGHT   = PCF_BK;
/// @brief Backlight off
const uint8 LCD_NOBACKLIGHT = 0x00;

/// <summary>
/// Constructor
///
/// \note Driver uses 4-bit mode, pins D0-D3 are not used.
/// </summary>
/// <param name="i2cMaster">     I2C master interface</param>
/// <param name="address">       I2C device address</param>
/// <param name="numColumns">    Display size in number of columns (max. 40)</param>
/// <param name="numRows">       Display size in number of rows (max. 4)</param>
/// <param name="characterSize">Character size, either 5x8 pixels (default) or 5x10 pixels</param>
HD44780DeviceI2C::HD44780DeviceI2C(II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows,
                                   CharacterSize characterSize /*= CharacterSize::Size5x8*/)
    : HD44780Device(numColumns, numRows, characterSize, DataMode::Mode4Bit)
    , m_i2cMaster(i2cMaster)
    , m_address(address)
    , m_backlightOn{}
{
}

/// <summary>
/// Destructor
///
/// Resets device back to 8 bit interface
/// </summary>
HD44780DeviceI2C::~HD44780DeviceI2C()
{
    SetBacklight(false);
    DisplayControl(DisplayMode::Hide, CursorMode::Hide);
}

/// <summary>
/// Switch backlight on or off
/// </summary>
/// <param name="on">If true, backlight is switched on, otherwise it is off</param>
void HD44780DeviceI2C::SetBacklight(bool on)
{
    if (on != m_backlightOn)
    {
        uint8 byte = (on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
        // We write a single byte with all other bits off. This will have no effect to the controller state, except for the backlight
        m_i2cMaster.Write(m_address, &byte, 1);
        m_backlightOn = on;
    }
}

/// <summary>
/// Returns backlight on / off status
/// </summary>
/// <returns>True if backlight is on, false otherwise</returns>
bool HD44780DeviceI2C::IsBacklightOn() const
{
    return m_backlightOn;
}

/// <summary>
/// Write a 4 bit value
///
/// The most significant 4 bits are the data written, bit 0 acts as the register select bit (0 = instruction, 1 = data)
/// </summary>
/// <param name="data">Value to write</param>
void HD44780DeviceI2C::WriteHalfByte(uint8 data)
{
    uint8 byte = data | (m_backlightOn ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
    // Pulse the `enable` flag to process value.
    uint8 value = byte | PCF_EN;
    m_i2cMaster.Write(m_address, &value, 1);
    Timer::WaitMicroSeconds(1);
    value = byte & ~PCF_EN;
    m_i2cMaster.Write(m_address, &value, 1);
    // Wait for command to complete.
    Timer::WaitMicroSeconds(100);
}

} // namespace device
