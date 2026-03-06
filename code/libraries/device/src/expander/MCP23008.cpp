//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23008.cpp
//
// Namespace   : device
//
// Class       : MCP23008
//
// Description : MCP23008 expander functionality
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

#include "device/expander/MCP23008.h"

#include "baremetal/Logger.h"

using namespace device;
using namespace baremetal;

/// @file
/// MCP 23008 expander implementation

/// @brief Define log name
LOG_MODULE("MCP23008");

/// <summary>
/// Constructor for MCP23008 class
/// </summary>
MCP23008::MCP23008()
{
}

/// <summary>
/// Destructor for MCP23008 class
/// </summary>
MCP23008::~MCP23008()
{
}

/// <summary>
/// Initialize the MCP23008 expander
/// </summary>
/// <returns>True on success, false otherwise</returns>
bool MCP23008::Initialize()
{
    LOG_INFO("Initialize");

    WriteRegister(IOCON, IOCON_SEQOP | IOCON_HAEN | IOCON_ODR);
    return true;
}

/// <summary>
/// Uninitialize MCP23008. Set pins to input mode.
/// </summary>
void MCP23008::Uninitialize()
{
    SetPortDirections(MCP23008PinDirection::In);
}

/// <summary>
/// Get the I/O pin direction for the specified pin on the MCP23008
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="direction">Holds pin direction on return</param>
void MCP23008::GetPinDirection(MCP23008Pin pinNumber, MCP23008PinDirection& direction)
{
    uint8 data = ReadRegister(IODIR);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    direction = (data & (1 << pinShift)) ? MCP23008PinDirection::In : MCP23008PinDirection::In;
}

/// <summary>
/// Set the I/O pin direction for the specified pin on the MCP23008
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="direction">Pin direction to set</param>
void MCP23008::SetPinDirection(MCP23008Pin pinNumber, const MCP23008PinDirection& direction)
{
    uint8 data = ReadRegister(IODIR);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    uint8 mask = 1 << pinShift;
    uint8 pinData = (direction == MCP23008PinDirection::In) ? mask : 0;
    WriteRegister(IODIR, (data & ~mask) | pinData);
}

/// <summary>
/// Get the value of an input pin on the MCP23008
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <returns>Requested pin value</returns>
bool MCP23008::GetPinValue(MCP23008Pin pinNumber)
{
    uint8 data = ReadRegister(GPIO);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    return (data & (1 << pinShift));
}

/// <summary>
/// Set the value of an output pin on the MCP23008
/// </summary>
/// <param name="pinNumber">Pin index (0-7 on Port A, 8-15 on Port B)</param>
/// <param name="on">Value for output pin</param>
void MCP23008::SetPinValue(MCP23008Pin pinNumber, bool on)
{
    uint8 data = ReadRegister(GPIO);
    uint8 pinShift = static_cast<uint8>(pinNumber) % 8;
    uint8 mask = 1 << pinShift;
    uint8 pinData = on ? mask : 0;
    WriteRegister(GPIO, (data & ~mask) | pinData);
}

/// <summary>
/// Set I/O pin directions for all pins
/// </summary>
/// <param name="direction">Direction for pins</param>
void MCP23008::SetPortDirections(const MCP23008PinDirection& direction)
{
    WriteRegister(GPPU, 0x00);
    WriteRegister(IODIR, direction == MCP23008PinDirection::In ? 0xFF : 0x00);
}

/// <summary>
/// Get value for all pins.
/// Bit 0 is the value for pin 0, etc.
/// </summary>
/// <returns></returns>
uint8 MCP23008::GetPortValue()
{
    return ReadRegister(GPIO);
}

/// <summary>
/// Set value for all pins
/// </summary>
/// <param name="data">Value for pins. Bit 0 is the value for pin 0, etc.</param>
void MCP23008::SetPortValue(uint8 data)
{
    WriteRegister(GPIO, data);
}
