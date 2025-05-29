//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DeviceRaw.h
//
// Namespace   : baremetal::display
//
// Class       : HD44780DeviceRaw
//
// Description : HD44780 based 16x2 LCD display with direct (raw) interface (no I2C piggyback)
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

#include <device/gpio/HD44780DeviceRaw.h>

#include <baremetal/Timer.h>

using namespace baremetal;

/// @file
/// HD44780 based GPIO 16x2 LCD display

namespace device {

/// <summary>
/// Constructor
///
/// \note Driver uses 4-bit mode, pins D0-D3 are not used.
/// \note We don't support reading from GPIO controlled displays
/// </summary>
/// <param name="memoryAccess">  Memory access interface for GPIO pins</param>
/// <param name="numColumns">    Display size in number of columns (max. 40)</param>
/// <param name="numRows">       Display size in number of rows (max. 4)</param>
/// <param name="d4Pin">         GPIO pin number of Data 4 pin (Broadcom GPIO number)</param>
/// <param name="d5Pin">         GPIO pin number of Data 5 pin (Broadcom GPIO number)</param>
/// <param name="d6Pin">         GPIO pin number of Data 6 pin (Broadcom GPIO number)</param>
/// <param name="d7Pin">         GPIO pin number of Data 7 pin (Broadcom GPIO number)</param>
/// <param name="enPin">         GPIO pin number of Enable pin (Broadcom GPIO number)</param>
/// <param name="rsPin">         GPIO pin number of Register Select pin (Broadcom GPIO number)</param>
/// <param name="characterSize"> Character size, either 5x8 pixels (default) or 5x10 pixels</param>
HD44780DeviceRaw::HD44780DeviceRaw(IMemoryAccess &memoryAccess, uint8 numColumns, uint8 numRows, uint8 d4Pin, uint8 d5Pin, uint8 d6Pin, uint8 d7Pin, uint8 enPin, uint8 rsPin,
                                   CharacterSize characterSize /*= CharacterSize::CharacterSize::Size5x8*/)
    : HD44780Device{numColumns, numRows, characterSize, DataMode::Mode4Bit}
    , m_d4Pin{d4Pin, GPIOMode::Output, memoryAccess}
    , m_d5Pin{d5Pin, GPIOMode::Output, memoryAccess}
    , m_d6Pin{d6Pin, GPIOMode::Output, memoryAccess}
    , m_d7Pin{d7Pin, GPIOMode::Output, memoryAccess}
    , m_enPin{enPin, GPIOMode::Output, memoryAccess}
    , m_rsPin{rsPin, GPIOMode::Output, memoryAccess}
{
    m_enPin.Off();
    m_rsPin.Off();
}

/// <summary>
/// Destructor
///
/// Switches all pins back to input mode
/// </summary>
HD44780DeviceRaw::~HD44780DeviceRaw()
{
    DisplayControl(DisplayMode::Hide, CursorMode::Hide);

    m_d4Pin.SetMode(GPIOMode::Input);
    m_d5Pin.SetMode(GPIOMode::Input);
    m_d6Pin.SetMode(GPIOMode::Input);
    m_d7Pin.SetMode(GPIOMode::Input);
    m_enPin.SetMode(GPIOMode::Input);
    m_rsPin.SetMode(GPIOMode::Input);
}

/// <summary>
/// Write a 4 bit value
///
/// The top 4 bits are the data written, bit 0 acts as the register select bit (0 = instruction, 1 = data)
/// | Output pin | Display connection | Function |
/// |------------|--------------------|----------|
/// | 0          | RS                 | Register Select (0 = instruction, 1 = data |
/// | 1          | RW                 | Read/Write (0 = write, 1 = read)           |
/// | 2          | E                  | Enable: Starts data read / write           |
/// | 3          | BL                 | Backlight (0 = off, 1 = on)                |
/// | 4          | D4                 | Data bit 4 (4 bit access only)             |
/// | 5          | D5                 | Data bit 5 (4 bit access only)             |
/// | 6          | D6                 | Data bit 6 (4 bit access only)             |
/// | 7          | D7                 | Data bit 7 (4 bit access only)             |
///
/// </summary>
/// <param name="data">Value to write</param>
void HD44780DeviceRaw::WriteHalfByte(uint8 data)
{
    // Set Register Select bit
    m_rsPin.Set((data & 0x01) ? true : false);
    // Set Bit D7-D4
    m_d4Pin.Set((data & 0x10) ? true : false);
    m_d5Pin.Set((data & 0x20) ? true : false);
    m_d6Pin.Set((data & 0x40) ? true : false);
    m_d7Pin.Set((data & 0x80) ? true : false);

    // Enable high: start data write
    m_enPin.On();
    Timer::WaitMicroSeconds(1);
    // Enable high: finish data write
    m_enPin.Off();
    Timer::WaitMicroSeconds(50);
    // Fall back to instruction
    m_rsPin.Off();
}

} // namespace device
