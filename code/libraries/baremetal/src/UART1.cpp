//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : UART1.cpp
//
// Namespace   : baremetal
//
// Class       : UART1
//
// Description : RPI UART1 class
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

#include <baremetal/UART1.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/BCMRegisters.h>

/// @brief Total count of GPIO pins, numbered from 0 through 53
#define NUM_GPIO 54

namespace baremetal {

static const int NumWaitCycles = 150;

static void WaitCycles(uint32 numCycles)
{
    if (numCycles)
    {
        while (numCycles--)
        {
            NOP();
        }
    }
}

UART1::UART1()
    : m_initialized{}
{
}

// Set baud rate and characteristics (115200 8N1) and map to GPIO
void UART1::Initialize()
{
    if (m_initialized)
        return;

    // initialize UART
    auto value = *reinterpret_cast<uint32 volatile *>(RPI_AUX_ENABLE);
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_ENABLE) = value | RPI_AUX_ENABLE_UART1;    // enable UART1, AUX mini uart
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_CNTL) = 0;                              // Disable Tx, Rx
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_LCR) = RPI_AUX_MU_LCR_DATA_SIZE_8;      // 8 bit mode
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_MCR) = RPI_AUX_MU_MCR_RTS_HIGH;         // RTS high
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_IER) = 0;                               // Disable interrupts
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_IIR) = RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR;
                                                                                            // Clear FIFO
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_BAUD) = 270;                            // 250 MHz / (8 * (baud + 1)) = 250000000 / (8 * 271) =  115313 -> 115200 baud

    SetMode(14, GPIOMode::AlternateFunction5);

    SetMode(15, GPIOMode::AlternateFunction5);
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_CNTL) = RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX;
                                                                                            // Enable Tx, Rx
    m_initialized = true;
}

// Write a character

void UART1::Write(char c)
{
    // wait until we can send
    do
    {
        NOP();
    } while (!(*reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_TX_EMPTY));
    // Check Tx FIFO empty
    // Write the character to the buffer
    *reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_IO) = static_cast<unsigned int>(c);
}

// Receive a character

char UART1::Read()
{
    char r;
    // wait until something is in the buffer
    do
    {
        NOP();
    } while (!(*reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_RX_READY)); // Check Rx FIFO holds data
    // Read it and return
    r = static_cast<char>(*reinterpret_cast<uint32 volatile *>(RPI_AUX_MU_IO));
    // Convert carriage return to newline
    return r;
}

void UART1::WriteString(const char *str)
{
    while (*str)
    {
        // convert newline to carriage return + newline
        if (*str == '\n')
            Write('\r');
        Write(*str++);
    }
}

bool UART1::SetMode(uint8 pinNumber, GPIOMode mode)
{
    if (pinNumber >= NUM_GPIO)
        return false;
    if (mode >= GPIOMode::Unknown)
        return false;
    if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
    {
        if (!SetPullMode(pinNumber, GPIOPullMode::Off))
            return false;

        if (!SetFunction(pinNumber, static_cast<GPIOFunction>(static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0) +
                                    static_cast<unsigned>(GPIOFunction::AlternateFunction0))))
            return false;
    }
    else if (GPIOMode::Output == mode)
    {
        if (!SetPullMode(pinNumber, GPIOPullMode::Off))
            return false;

        if (!SetFunction(pinNumber, GPIOFunction::Output))
            return false;
    }
    else
    {
        if (!SetPullMode(pinNumber, (mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off))
            return false;
        if (!SetFunction(pinNumber, GPIOFunction::Input))
            return false;
    }
    if (mode == GPIOMode::Output)
        Off(pinNumber, mode);
    return true;
}

bool UART1::SetFunction(uint8 pinNumber, GPIOFunction function)
{
    if (pinNumber >= NUM_GPIO)
        return false;
    if (function >= GPIOFunction::Unknown)
        return false;

    uintptr selectRegister = RPI_GPIO_GPFSEL0 + (pinNumber / 10) * 4;
    uint32  shift          = (pinNumber % 10) * 3;

    static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};

    uint32 value = *reinterpret_cast<uint32 volatile *>(selectRegister);
    value &= ~(7 << shift);
    value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
    *reinterpret_cast<uint32 volatile *>(selectRegister) = value;
    return true;
}

bool UART1::SetPullMode(uint8 pinNumber, GPIOPullMode pullMode)
{
    if (pullMode >= GPIOPullMode::Unknown)
        return false;

    if (pinNumber >= NUM_GPIO)
        return false;
#if RPI_TARGET <= 3
    uintptr clkRegister = RPI_GPIO_GPPUDCLK0 + (pinNumber / 32) * 4;
    uint32  shift       = pinNumber % 32;

    *reinterpret_cast<uint32 volatile *>(RPI_GPIO_GPPUD) = static_cast<uint32>(pullMode);
    WaitCycles(NumWaitCycles);
    *reinterpret_cast<uint32 volatile *>(clkRegister) = static_cast<uint32>(1 << shift);
    WaitCycles(NumWaitCycles);
    *reinterpret_cast<uint32 volatile *>(clkRegister) = 0;
#else
    uintptr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16) * 4;
    unsigned              shift   = (m_pinNumber % 16) * 2;

    static const unsigned ModeMap[3] = {0, 2, 1};

    uint32                value = m_memoryAccess.Read32(modeReg);
    value &= ~(3 << shift);
    value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
    m_memoryAccess.Write32(modeReg, value);
#endif

    return true;
}

bool UART1::Off(uint8 pinNumber, GPIOMode mode)
{
    if (pinNumber >= NUM_GPIO)
        return false;

    // Output level can be set in input mode for subsequent switch to output
    if (mode >= GPIOMode::AlternateFunction0)
        return false;

    unsigned regOffset = (pinNumber / 32) * 4;
    uint32 regMask = 1 << (pinNumber % 32);

    bool value = false;

    uintptr setClrReg = (value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;

    *reinterpret_cast<uint32 volatile *>(setClrReg) = regMask;

    return true;
}

} // namespace baremetal