//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : PhysicalGPIOPin.cpp
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

#include <baremetal/PhysicalGPIOPin.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>
#include <baremetal/MemoryAccess.h>
#include <baremetal/Timer.h>

/// @file
/// Physical GPIO pin implementation

/// @brief Total count of GPIO pins, numbered from 0 through 53
#define NUM_GPIO 54

namespace baremetal {

#if BAREMETAL_RPI_TARGET == 3
/// @brief Number of cycles to wait when setting pull mode for GPIO pin (Raspberry Pi 3 only)
static const int NumWaitCycles = 150;
#endif // BAREMETAL_RPI_TARGET == 3

/// <summary>
/// Creates a virtual GPIO pin 
/// </summary>
/// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
    : m_pinNumber{ NUM_GPIO }
    , m_mode{ GPIOMode::Unknown }
    , m_function{ GPIOFunction::Unknown }
    , m_pullMode{ GPIOPullMode::Unknown }
    , m_value{}
    , m_memoryAccess{ memoryAccess }
{
}

/// <summary>
/// Creates a virtual GPIO pin 
/// </summary>
/// <param name="pinNumber">GPIO pin number (0..53)</param>
/// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
/// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
    : m_pinNumber{ NUM_GPIO }
    , m_mode{ GPIOMode::Unknown }
    , m_value{}
    , m_memoryAccess{ memoryAccess }
{
    AssignPin(pinNumber);
    SetMode(mode);
}

/// <summary>
/// Return the configured GPIO pin number
/// </summary>
/// <returns>GPIO pin number (0..53)</returns>
uint8 PhysicalGPIOPin::GetPinNumber() const
{
    return m_pinNumber;
}

/// <summary>
/// Assign a GPIO pin
/// </summary>
/// <param name="pinNumber">GPIO pin number to set (0..53)</param>
/// <returns>Return true on success, false on failure</returns>
bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
{
    // Check if pin already assigned
    if (m_pinNumber != NUM_GPIO)
        return false;
    m_pinNumber = pinNumber;

    return true;
}

/// <summary>
/// Switch GPIO on
/// </summary>
void PhysicalGPIOPin::On()
{
    Set(true);
}

/// <summary>
/// Switch GPIO off
/// </summary>
void PhysicalGPIOPin::Off()
{
    Set(false);
}

/// <summary>
/// Get GPIO value
/// </summary>
/// <returns>The status of the configured GPIO pin. Returns true if on, false if off</returns>
bool PhysicalGPIOPin::Get()
{
    // Check if pin is assigned
    if (m_pinNumber >= NUM_GPIO)
        return false;

    if ((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown))
    {
        uint32 regOffset = (m_pinNumber / 32);
        uint32 regMask = 1 << (m_pinNumber % 32);
        return (m_memoryAccess.Read32(RPI_GPIO_GPLEV0 + regOffset) & regMask) ? true : false;
    }
    return m_value;
}

/// <summary>
/// Set GPIO on (true) or off (false)
/// </summary>
/// <param name="on">Value to set GPIO pin to (true for on, false for off).</param>
void PhysicalGPIOPin::Set(bool on)
{
    // Check if pin is assigned
    if (m_pinNumber >= NUM_GPIO)
        return;

    // Check if mode is output
    if (m_mode == GPIOMode::Output)
        return;

    m_value = on;

    uint32 regOffset = (m_pinNumber / 32);
    uint32 regMask = 1 << (m_pinNumber % 32);
    regaddr regAddress = (m_value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;

    m_memoryAccess.Write32(regAddress, regMask);
}

/// <summary>
/// Invert GPIO value on->off off->on
/// </summary>
void PhysicalGPIOPin::Invert()
{
    Set(!Get());
}

/// <summary>
/// Get the mode for the GPIO pin
/// </summary>
/// <returns>Currently set mode for the configured GPIO pin</returns>
GPIOMode PhysicalGPIOPin::GetMode()
{
    return m_mode;
}

/// <summary>
/// Convert GPIO mode to GPIO function. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
/// </summary>
/// <param name="mode">GPIO mode</param>
/// <returns>GPIO function</returns>
static GPIOFunction ConvertGPIOModeToFunction(GPIOMode mode)
{
    if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
    {
        unsigned alternateFunctionIndex = static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0);
        return static_cast<GPIOFunction>(static_cast<unsigned>(GPIOFunction::AlternateFunction0) + alternateFunctionIndex);
    }
    else if (GPIOMode::Output == mode)
    {
        return GPIOFunction::Output;
    }
    return GPIOFunction::Input;
}

/// <summary>
/// Set the mode for the GPIO pin
/// </summary>
/// <param name="mode">Mode to be set for the configured GPIO. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
/// <returns>Return true on success, false on failure</returns>
bool PhysicalGPIOPin::SetMode(GPIOMode mode)
{
    // Check if pin is assigned
    if (m_pinNumber >= NUM_GPIO)
        return false;

    // Check if mode is valid
    if (mode >= GPIOMode::Unknown)
        return false;

    if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
    {
        SetPullMode(GPIOPullMode::Off);

        SetFunction(ConvertGPIOModeToFunction(mode));
    }
    else if (GPIOMode::Output == mode)
    {
        SetPullMode(GPIOPullMode::Off);

        SetFunction(ConvertGPIOModeToFunction(mode));
    }
    else
    {
        SetPullMode((mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off);
        SetFunction(ConvertGPIOModeToFunction(mode));
    }
    m_mode = mode;
    if (m_mode == GPIOMode::Output)
        Off();
    return true;
}

/// <summary>
/// Get GPIO pin function
/// </summary>
/// <returns>Function set for the configured GPIO pin</returns>
GPIOFunction PhysicalGPIOPin::GetFunction()
{
    return m_function;
}

/// <summary>
/// Get GPIO pin pull mode
/// </summary>
/// <returns>Pull mode set for the configured GPIO pin</returns>
GPIOPullMode PhysicalGPIOPin::GetPullMode()
{
    return m_pullMode;
}

/// <summary>
/// Set GPIO pin function
/// </summary>
/// <param name="function">Function to be set for the configured GPIO pin</param>
void PhysicalGPIOPin::SetFunction(GPIOFunction function)
{
    // Check if pin is assigned
    if (m_pinNumber >= NUM_GPIO)
        return;

    // Check if mode is valid
    if (function >= GPIOFunction::Unknown)
        return;

    regaddr selectRegister = RPI_GPIO_GPFSEL0 + (m_pinNumber / 10);
    uint32  shift = (m_pinNumber % 10) * 3;

    static const unsigned FunctionMap[] = { 0, 1, 4, 5, 6, 7, 3, 2 };

    uint32 value = m_memoryAccess.Read32(selectRegister);
    value &= ~(7 << shift);
    value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
    m_memoryAccess.Write32(selectRegister, value);
    m_function = function;
}

/// <summary>
/// Set GPIO pin pull mode
/// </summary>
/// <param name="pullMode">Pull mode to be set for the configured GPIO pin</param>
void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
{
    // Check if pin is assigned
    if (m_pinNumber >= NUM_GPIO)
        return;

    // Check if mode is valid
    if (pullMode >= GPIOPullMode::Unknown)
        return;

#if BAREMETAL_RPI_TARGET == 3
    regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32);
    uint32  shift = m_pinNumber % 32;

    m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
    Timer::WaitCycles(NumWaitCycles);
    m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
    Timer::WaitCycles(NumWaitCycles);
    m_memoryAccess.Write32(clkRegister, 0);
#else
    regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16);
    unsigned              shift = (m_pinNumber % 16) * 2;

    static const unsigned ModeMap[3] = { 0, 2, 1 };

    uint32                value = m_memoryAccess.Read32(modeReg);
    value &= ~(3 << shift);
    value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
    m_memoryAccess.Write32(modeReg, value);
#endif

    m_pullMode = pullMode;
}

} // namespace baremetal
