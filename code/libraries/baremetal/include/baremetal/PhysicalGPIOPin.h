//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : PhysicalGPIOPin.h
//
// Namespace   : baremetal
//
// Class       : PhysicalGPIOPin
//
// Description : Physical GPIO pin
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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

#include <baremetal/IGPIOPin.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Physical GPIO pin

namespace baremetal {

/// <summary>
/// Physical GPIO pin (i.e. available on GPIO header) 
/// </summary>
class PhysicalGPIOPin : public IGPIOPin
{
private:
    /// @brief Configured GPIO pin number (0..53)
    uint8                 m_pinNumber;
    /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
    GPIOMode              m_mode;
    /// @brief Configured GPIO function.
    GPIOFunction          m_function;
    /// @brief Configured GPIO pull mode (only for input function).
    GPIOPullMode          m_pullMode;
    /// @brief Current value of the GPIO pin (true for on, false for off).
    bool                  m_value;
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess& m_memoryAccess;

public:
    /// <summary>
    /// Creates a virtual GPIO pin 
    /// </summary>
    /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
    PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());

    /// <summary>
    /// Creates a virtual GPIO pin 
    /// </summary>
    /// <param name="pinNumber">GPIO pin number (0..53)</param>
    /// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
    /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());

    /// <summary>
    /// Return the configured GPIO pin number
    /// </summary>
    /// <returns>GPIO pin number (0..53)</returns>
    uint8 GetPinNumber() const override;
    /// <summary>
    /// Assign a GPIO pin
    /// </summary>
    /// <param name="pinNumber">GPIO pin number to set (0..53)</param>
    /// <returns>Return true on success, false on failure</returns>
    bool AssignPin(uint8 pinNumber) override;

    /// <summary>
    /// Switch GPIO on
    /// </summary>
    void On() override;
    /// <summary>
    /// Switch GPIO off
    /// </summary>
    void Off() override;
    /// <summary>
    /// Get GPIO value
    /// </summary>
    /// <returns>The status of the configured GPIO pin. Returns true if on, false if off</returns>
    bool Get() override;
    /// <summary>
    /// Set GPIO on (true) or off (false)
    /// </summary>
    /// <param name="on">Value to set GPIO pin to (true for on, false for off).</param>
    void Set(bool on) override;
    /// <summary>
    /// Invert GPIO value on->off off->on
    /// </summary>
    void Invert() override;

    ///     /// <summary>
    /// Get the mode for the GPIO pin
    /// </summary>
    /// <returns>Currently set mode for the configured GPIO pin</returns>
    GPIOMode GetMode();
    /// <summary>
    /// Set the mode for the GPIO pin
    /// </summary>
    /// <param name="mode">Mode to be set for the configured GPIO. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
    /// <returns>Return true on success, false on failure</returns>
    bool SetMode(GPIOMode mode);
    /// <summary>
    /// Get GPIO pin function
    /// </summary>
    /// <returns>Function set for the configured GPIO pin</returns>
    GPIOFunction GetFunction();
    /// <summary>
    /// Get GPIO pin pull mode
    /// </summary>
    /// <returns>Pull mode set for the configured GPIO pin</returns>
    GPIOPullMode GetPullMode();
    /// <summary>
    /// Set GPIO pin pull mode
    /// </summary>
    /// <param name="pullMode">Pull mode to be set for the configured GPIO pin</param>
    void SetPullMode(GPIOPullMode pullMode);

private:
    /// <summary>
    /// Set GPIO pin function
    /// </summary>
    /// <param name="function">Function to be set for the configured GPIO pin</param>
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
