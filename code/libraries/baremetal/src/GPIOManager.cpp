//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : GPIOManager.cpp
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

#include "baremetal/GPIOManager.h"

#include "baremetal/ARMInstructions.h"
#include "baremetal/BCMRegisters.h"
#include "baremetal/InterruptHandler.h"
#include "baremetal/Logger.h"
#include "baremetal/MemoryAccess.h"
#include "baremetal/PhysicalGPIOPin.h"
#include "baremetal/Timer.h"

using namespace baremetal;

static const int WaitCycles = 150;

static const IRQ_ID GPIO_IRQ{IRQ_ID::IRQ_GPIO3}; // shared IRQ line for all GPIOs

static void GPIOInterruptHandler(void* param);

/// @brief Define log name
LOG_MODULE("GPIOManager");

/// <summary>
/// Create a GPIO control. Note that the constructor is private, so GetGPIOManager() is needed to instantiate the GPIO control
/// </summary>
GPIOManager::GPIOManager()
    : m_isInitialized{}
    , m_pins{}
    , m_memoryAccess{GetMemoryAccess()}
{
}

/// <summary>
/// Create a GPIO control with a custom memory access instance (for testing)
/// </summary>
/// <param name="memoryAccess">MemoryAccess instance to use for the GPIOManager</param>
GPIOManager::GPIOManager(IMemoryAccess& memoryAccess)
    : m_isInitialized{}
    , m_pins{}
    , m_memoryAccess{memoryAccess}
{
}

/// <summary>
/// GPIO manager destructor
/// </summary>
GPIOManager::~GPIOManager()
{
#ifndef NDEBUG
    for (uint8 pin = 0; pin < NUM_GPIO; pin++)
    {
        assert(m_pins[pin] == nullptr);
    }
#endif

    if (m_isInitialized)
    {
        GetInterruptSystem().UnregisterIRQHandler(GPIO_IRQ);
    }
}

/// <summary>
/// Initialize GPIO manager
/// </summary>
void GPIOManager::Initialize()
{
    if (m_isInitialized)
        return;
    GetInterruptSystem().RegisterIRQHandler(GPIO_IRQ, GPIOInterruptHandler, this);
    m_isInitialized = true;
}

/// <summary>
/// Connect the GPIO pin interrupt for the specified pin
/// </summary>
/// <param name="pin">GPIO pin to connect interrupt for</param>
void GPIOManager::ConnectInterrupt(IGPIOPin* pin)
{
    assert(m_isInitialized);

    assert(pin != nullptr);
    auto pinNumber = pin->GetPinNumber();
    assert(pinNumber < NUM_GPIO);

    assert(m_pins[pinNumber] == nullptr);
    m_pins[pinNumber] = pin;
}

/// <summary>
/// Disconnect the GPIO pin interrupt for the specified pin
/// </summary>
/// <param name="pin">GPIO pin to disconnect interrupt for</param>
void GPIOManager::DisconnectInterrupt(const IGPIOPin* pin)
{
    assert(m_isInitialized);

    assert(pin != nullptr);
    auto pinNumber = pin->GetPinNumber();
    assert(pinNumber < NUM_GPIO);

    assert(m_pins[pinNumber] != nullptr);
    m_pins[pinNumber] = nullptr;
}

/// <summary>
/// Switch all GPIO pins to input mode, without pull-up or pull-down
/// </summary>
void GPIOManager::AllOff()
{
    // power off gpio pins (but not VCC pins)
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL0, 0); // All pins input
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL1, 0);
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL2, 0);
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL3, 0);
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL4, 0);
    m_memoryAccess.Write32(RPI_GPIO_GPFSEL5, 0);
#if BAREMETAL_RPI_TARGET == 3
    m_memoryAccess.Write32(RPI_GPIO_GPPUD, 0); // All mode Off
    Timer::WaitCycles(WaitCycles);
    m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK0, 0xFFFFFFFF); // Trigger clock
    m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK1, 0xFFFFFFFF);
    Timer::WaitCycles(WaitCycles);
    m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK0, 0); // Release clock
    m_memoryAccess.Write32(RPI_GPIO_GPPUDCLK1, 0);
#else
    m_memoryAccess.Write32(RPI_GPIO_GPPUPPDN0, 0); // All mode Off
    m_memoryAccess.Write32(RPI_GPIO_GPPUPPDN1, 0);
#endif
}

/// <summary>
/// GPIO pin interrupt handler, called by the static entry point GPIOInterruptHandler()
/// </summary>
void GPIOManager::InterruptHandler()
{
    assert(m_isInitialized);

    uint32 eventStatus = m_memoryAccess.Read32(RPI_GPIO_GPEDS0);

    uint8 pinNumber = 0;
    while (pinNumber < NUM_GPIO)
    {
        if (eventStatus & 1)
        {
            break;
        }
        eventStatus >>= 1;

        if (++pinNumber % 32 == 0)
        {
            eventStatus = m_memoryAccess.Read32(RPI_GPIO_GPEDS1);
        }
    }

    if (pinNumber < NUM_GPIO)
    {
        auto pin = m_pins[pinNumber];
        if (pin != nullptr)
        {
            pin->InterruptHandler();

            if (pin->GetAutoAcknowledgeInterrupt())
            {
                pin->AcknowledgeInterrupt();
            }
        }
        else
        {
            LOG_ERROR("No pin found for interrupt");

            uint32 regOffset{static_cast<uint32>((pinNumber / 32) * 4)};
            uint32 regMask{static_cast<uint32>(1 << (pinNumber % 32))};
            m_memoryAccess.Write32(RPI_GPIO_GPEDS0 + regOffset, regMask);
        }
    }
}

/// <summary>
/// Disable all GPIO interrupt types for the specified in number
/// </summary>
/// <param name="pinNumber">Pin number of GPIO to disable interrupts for</param>
void GPIOManager::DisableAllInterrupts(uint8 pinNumber)
{
    assert(pinNumber < NUM_GPIO);

    uint32 mask = 1 << (pinNumber % 32);

    for (auto reg = RPI_GPIO_GPREN0 + (pinNumber / 32) * 4; reg < RPI_GPIO_GPAFEN0 + 4; reg += 12)
    {
        m_memoryAccess.Write32(reg, m_memoryAccess.Read32(reg) & ~mask);
    }
}

/// <summary>
/// GPIO IRQ entry pointer, calls Interrupt handle on the GPIOManager instance passed through param
/// </summary>
/// <param name="param">Pointer to GPIOManager instance</param>
void GPIOInterruptHandler(void* param)
{
    GPIOManager* pThis = reinterpret_cast<GPIOManager*>(param);
    assert(pThis != nullptr);

    pThis->InterruptHandler();
}

/// <summary>
/// Create a singleton GPIOManager if neededm and return the singleton instance
/// </summary>
/// <returns>A reference to the singleton GPIOManager.</returns>
GPIOManager& baremetal::GetGPIOManager()
{
    static GPIOManager control;
    control.Initialize();
    return control;
}
