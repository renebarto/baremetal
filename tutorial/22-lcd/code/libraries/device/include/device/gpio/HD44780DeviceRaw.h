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

#pragma once

#include <device/display/HD44780Device.h>

#include <baremetal/PhysicalGPIOPin.h>

/// @file
/// HD44780 based GPIO LCD character display (max 40x4 characters)

namespace device {

/// <summary>
/// GPIO controlled HD44780 based LCD display
/// </summary>
class HD44780DeviceRaw : public HD44780Device
{
private:
    /// @brief GPIO pin for D4
    baremetal::PhysicalGPIOPin m_d4Pin;
    /// @brief GPIO pin for D5
    baremetal::PhysicalGPIOPin m_d5Pin;
    /// @brief GPIO pin for D6
    baremetal::PhysicalGPIOPin m_d6Pin;
    /// @brief GPIO pin for D7
    baremetal::PhysicalGPIOPin m_d7Pin;
    /// @brief GPIO pin for E (Enable / Clock)
    baremetal::PhysicalGPIOPin m_enPin;
    /// @brief GPIO pin for RS (Register Select)
    baremetal::PhysicalGPIOPin m_rsPin;

public:
    HD44780DeviceRaw(baremetal::IMemoryAccess &memoryAccess, uint8 numColumns, uint8 numRows, uint8 d4Pin, uint8 d5Pin, uint8 d6Pin, uint8 d7Pin, uint8 enPin, uint8 rsPin,
                     CharacterSize characterSize = CharacterSize::Size5x8);
    HD44780DeviceRaw(const HD44780DeviceRaw&) = delete;
    ~HD44780DeviceRaw();

    HD44780DeviceRaw &operator=(const HD44780DeviceRaw &other) = delete;

protected:
    void WriteHalfByte(uint8 data) override;
};

} // namespace device
