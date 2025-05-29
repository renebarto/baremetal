//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : IGPIOManager.h
//
// Namespace   : baremetal
//
// Class       : IGPIOManager
//
// Description : GPIO control abstract interface
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

#include <stdlib/Types.h>

/// @file
/// GPIO configuration and control

namespace baremetal {

class IGPIOPin;

/// @brief Handles configuration, setting and getting GPIO controls
/// This is a singleton class, created as soon as GetIGPIOManager() is called
class IGPIOManager
{
public:
    /// <summary>
    /// Default destructor needed for abstract interface
    /// </summary>
    virtual ~IGPIOManager() = default;

    /// <summary>
    /// Initialize GPIO manager
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// Connect the GPIO pin interrupt for the specified pin
    /// </summary>
    /// <param name="pin">GPIO pin to connect interrupt for</param>
    virtual void ConnectInterrupt(IGPIOPin *pin) = 0;
    /// <summary>
    /// Disconnect the GPIO pin interrupt for the specified pin
    /// </summary>
    /// <param name="pin">GPIO pin to disconnect interrupt for</param>
    virtual void DisconnectInterrupt(const IGPIOPin *pin) = 0;

    /// <summary>
    /// GPIO pin interrupt handler, called by the static entry point GPIOInterruptHandler()
    /// </summary>
    virtual void InterruptHandler() = 0;

    /// <summary>
    /// Switch all GPIO pins to input mode, without pull-up or pull-down
    /// </summary>
    virtual void AllOff() = 0;
};

} // namespace baremetal
