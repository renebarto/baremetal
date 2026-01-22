//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : GPIOManager.h
//
// Namespace   : baremetal
//
// Class       : GPIOManager
//
// Description : GPIO control
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

#include "baremetal/IGPIOManager.h"
#include "baremetal/PhysicalGPIOPin.h"
#include "stdlib/Types.h"

/// @file
/// GPIO configuration and control

namespace baremetal {

class IMemoryAccess;

/// @brief Handles configuration, setting and getting GPIO controls
/// This is a singleton class, created as soon as GetGPIOManager() is called
class GPIOManager : public IGPIOManager
{
    /// <summary>
    /// Construct the singleton GPIOManager instance if needed, and return a reference to the instance. This is a friend function of class GPIOManager
    /// </summary>
    /// <returns>Reference to the singleton GPIOManager instance</returns>
    friend GPIOManager& GetGPIOManager();

private:
    /// @brief True if class is already initialized
    bool m_isInitialized;
    /// @brief Array of all registered GPIO pins (nullptr if the GPIO is not registered)
    IGPIOPin* m_pins[NUM_GPIO];
    /// @brief Memory access interface
    IMemoryAccess& m_memoryAccess;

    GPIOManager();

public:
    explicit GPIOManager(IMemoryAccess& memoryAccess);
    ~GPIOManager();

    void Initialize() override;

    void ConnectInterrupt(IGPIOPin* pin) override;
    void DisconnectInterrupt(const IGPIOPin* pin) override;

    void InterruptHandler() override;

    void AllOff() override;

    void DisableAllInterrupts(uint8 pinNumber);

private:
};

GPIOManager& GetGPIOManager();

} // namespace baremetal
