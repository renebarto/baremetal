//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : HD44780DisplayI2C.h
//
// Namespace   : device
//
// Class       : HD44780DisplayI2C
//
// Description : HD44780 based LCD generic display (max 40x4) with I2C piggyback
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

#include "device/display/HD44780Display.h"

/// @file
/// HD44780 based I2C LCD character display (max 40x4 characters) with I2C piggyback

namespace baremetal {

class II2CMaster;

} // namespace baremetal

namespace device {

/// <summary>
/// I2C controlled HD44780 based LCD character display
/// </summary>
class HD44780DisplayI2C : public HD44780Display
{
private:
    /// @brief I2C master interface
    baremetal::II2CMaster& m_i2cMaster;
    /// @brief I2C address of the LCD controller
    uint8                  m_address;
    /// @brief Backlight status
    bool                   m_backlightOn;

public:
    HD44780DisplayI2C(baremetal::II2CMaster& i2cMaster, uint8 address, uint8 numColumns, uint8 numRows, CharacterSize characterSize = CharacterSize::Size5x8);

    ~HD44780DisplayI2C();

    void SetBacklight(bool on) override;
    bool IsBacklightOn() const override;

protected:
    void WriteHalfByte(uint8 data) override;
};

} // namespace device
