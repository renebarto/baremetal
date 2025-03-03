//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : KY-040.h
//
// Namespace   : baremetal::sensor
//
// Class       : KY040
//
// Description : KY-040 rotary encoder support
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

#include <stdlib/Types.h>
#include <baremetal/PhysicalGPIOPin.h>
#include <baremetal/Timer.h>

/// @file
/// Abstract GPIO pin. Could be either a virtual or physical pin

namespace baremetal {
namespace sensor {

enum class EncoderState;
enum class SwitchEvent;
enum class SwitchState;

/// @brief KY040 rotary encoder
class KY040
{
public:
    enum class Event
    {
        RotateClockwise,
        RotateCounterclockwise,

        SwitchDown,
        SwitchUp,
        SwitchClick,
        SwitchDoubleClick,
        SwitchTripleClick,
        SwitchHold, ///< generated each second

        Unknown
    };

    using EventHandler = void(Event event, void *param);

private:
    bool              m_initialized;
    PhysicalGPIOPin   m_clkPin;
    PhysicalGPIOPin   m_dtPin;
    PhysicalGPIOPin   m_swPin;
    EncoderState      m_encoderState;
    SwitchState       m_switchState;
    unsigned          m_holdCounter;
    KernelTimerHandle m_debounceTimerHandle;
    KernelTimerHandle m_tickTimerHandle;
    unsigned          m_currentPressTicks;
    unsigned          m_currentReleaseTicks;
    unsigned          m_lastPressTicks;
    unsigned          m_lastReleaseTicks;

    EventHandler     *m_eventHandler;
    void             *m_eventParam;

public:
    /// @brief Create an instance of a KY040 rotary encoder
    /// @param clkPin CLK input GPIO number
    /// @param dtPin  DT input GPIO number
    /// @param swPin  SW input GPIO number
    KY040(uint8 clkPin, uint8 dtPin, uint8 swPin);
    virtual ~KY040();

    bool               Initialize();

    void               RegisterEventHandler(EventHandler *handler, void *param);
    void               UnregisterEventHandler(EventHandler *handler);
    static const char *EventToString(Event event);

private:
    static void EncoderInterruptHandler(void *param);
    static void SwitchInterruptHandler(void *param);
    static void SwitchDebounceHandler(KernelTimerHandle handle, void *param, void *context);
    void        SwitchDebounceHandler(KernelTimerHandle handle, void *param);
    static void SwitchTickHandler(KernelTimerHandle handle, void *param, void *context);
    void        SwitchTickHandler(KernelTimerHandle handle, void *param);
    void        HandleSwitchEvent(SwitchEvent switchEvent);
};

} // namespace sensor
} // namespace baremetal
