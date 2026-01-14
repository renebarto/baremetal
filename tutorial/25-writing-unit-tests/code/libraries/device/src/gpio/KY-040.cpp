//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : KY-040.cpp
//
// Namespace   : device
//
// Class       : KY040
//
// Description : KY-040 rotary encoder support
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

#include "device/gpio/KY-040.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"

/// @file
/// KY-040 rotary switch support imlementation.

/// @brief Define log name
LOG_MODULE("KY-040");

using namespace baremetal;

namespace device {

/// @brief Time delay for debounding switch button
static const unsigned SwitchDebounceDelayMilliseconds = 50;
/// @brief Tick delay for determining if switch button was held down
static const unsigned SwitchTickDelayMilliseconds = 1000;
/// @brief Maximum delay between press and release for a click
static const unsigned SwitchClickMaxDelayMilliseconds = 300;
/// @brief Maximum delay between two presses for a double click (or triple click)
static const unsigned SwitchDoubleClickMaxDelayMilliseconds = 800;

/// <summary>
/// Switch encoder internal state
/// </summary>
enum class SwitchEncoderState
{
    /// @brief CLK high, DT high
    Start,
    /// @brief CLK high, DT down
    CWStart,
    /// @brief CLK down, DT low
    CWDataFall,
    /// @brief CLK low, DT up
    CWClockRise,
    /// @brief CLK down, DT high
    CCWStart,
    /// @brief CLK up DT down
    CCWClockFall,
    /// @brief CLK up, DT low
    CCWDataRise,
    /// @brief Invalid state
    Invalid,
    /// @brief Unknown
    Unknown
};

/// <summary>
/// Switch button internal event
/// </summary>
enum class SwitchButtonEvent
{
    /// @brief Switch button is down
    Down,
    /// @brief Switch button is up
    Up,
    /// @brief Switch button is clicked
    Click,
    /// @brief Switch button is double clicked
    DblClick,
    /// @brief Switch button is held down for at least SwitchTickDelayMilliseconds milliseconds
    Tick,
    /// @brief Unknown event
    Unknown
};

/// <summary>
/// Switch button internal state
/// </summary>
enum class SwitchButtonState
{
    /// @brief Initial state
    Start,
    /// @brief Switch button is down
    Down,
    /// @brief Switch button is clicked
    Click,
    /// @brief Switch button is down for the second time within SwitchDoubleClickMaxDelayMilliseconds milliseconds
    Click2,
    /// @brief Switch button is down for the third time, since second time within SwitchDoubleClickMaxDelayMilliseconds milliseconds
    Click3,
    /// @brief Switch button is held down
    Hold,
    /// @brief Invalid state
    Invalid,
    /// @brief Unknown state
    Unknown
};

/// <summary>
/// Convert rotary switch encoder state to a string
/// </summary>
/// <param name="state">Switch encode state</param>
/// <returns>String representing state</returns>
static const char* EncoderStateToString(SwitchEncoderState state)
{
    switch (state)
    {
    case SwitchEncoderState::Start:
        return "Start";
    case SwitchEncoderState::CWStart:
        return "CWStart";
    case SwitchEncoderState::CWDataFall:
        return "CWDataFall";
    case SwitchEncoderState::CWClockRise:
        return "CWClockRise";
    case SwitchEncoderState::CCWStart:
        return "CCWStart";
    case SwitchEncoderState::CCWClockFall:
        return "CCWClockFall";
    case SwitchEncoderState::CCWDataRise:
        return "CCWDataRise";
    case SwitchEncoderState::Invalid:
        return "Invalid";
    case SwitchEncoderState::Unknown:
    default:
        break;
    }
    return "Unknown";
}

/// <summary>
/// Convert rotary switch event to a string
/// </summary>
/// <param name="event">Event type</param>
/// <returns>String representing event</returns>
const char* KY040::EventToString(KY040::Event event)
{
    switch (event)
    {
    case KY040::Event::RotateClockwise:
        return "RotateClockwise";
    case KY040::Event::RotateCounterclockwise:
        return "RotateCounterclockwise";
    case KY040::Event::SwitchDown:
        return "SwitchDown";
    case KY040::Event::SwitchUp:
        return "SwitchUp";
    case KY040::Event::SwitchClick:
        return "SwitchClick";
    case KY040::Event::SwitchDoubleClick:
        return "SwitchDoubleClick";
    case KY040::Event::SwitchTripleClick:
        return "SwitchTripleClick";
    case KY040::Event::SwitchHold:
        return "SwitchHold";
    case KY040::Event::Unknown:
    default:
        break;
    }
    return "Unknown";
}

/// <summary>
/// Convert internal switch button event to a string
/// </summary>
/// <param name="event">Event button event</param>
/// <returns>String representing event</returns>
static const char* SwitchButtonEventToString(SwitchButtonEvent event)
{
    switch (event)
    {
    case SwitchButtonEvent::Down:
        return "Down";
    case SwitchButtonEvent::Up:
        return "Up";
    case SwitchButtonEvent::Click:
        return "Click";
    case SwitchButtonEvent::DblClick:
        return "DblClick";
    case SwitchButtonEvent::Tick:
        return "Tick";
    case SwitchButtonEvent::Unknown:
    default:
        break;
    }
    return "Unknown";
}

/// <summary>
/// Convert internal switch button state to a string
/// </summary>
/// <param name="state">Event button state</param>
/// <returns>String representing state</returns>
static const char* SwitchButtonStateToString(SwitchButtonState state)
{
    switch (state)
    {
    case SwitchButtonState::Start:
        return "Start";
    case SwitchButtonState::Down:
        return "Down";
    case SwitchButtonState::Click:
        return "Click";
    case SwitchButtonState::Click2:
        return "Click2";
    case SwitchButtonState::Click3:
        return "Click3";
    case SwitchButtonState::Hold:
        return "Hold";
    case SwitchButtonState::Invalid:
        return "Invalid";
    case SwitchButtonState::Unknown:
    default:
        break;
    }
    return "Unknown";
}

/// <summary>
/// Lookup table for rotary switch to create an event from an the status of the CLK and DT GPIO inputs when in a certain internal state
/// </summary>
static const KY040::Event s_encoderOutput[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
    //  {{CLK=0/DT=0,            CLK=0/DT=1},            {CLK=1/DT=0,            CLK=1/DT=1}}

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // Start

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CWStart
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CWDataFall
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateClockwise}       }, // CWClockRise

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CCWStart
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }, // CCWClockFall
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateCounterclockwise}}, // CCWDataRise

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}               }  // Invalid
};
/// <summary>
/// Get an event for the rotary switch
/// </summary>
/// <param name="state">Rotary switch internal state</param>
/// <param name="clkValue">Value of CLK GPIO input</param>
/// <param name="dtValue">Value of DT GPIO input</param>
/// <returns>Resulting event</returns>
static KY040::Event GetEncoderOutput(SwitchEncoderState state, bool clkValue, bool dtValue)
{
    return s_encoderOutput[static_cast<size_t>(state)][clkValue][dtValue];
}

/// <summary>
/// Lookup table for rotary switch to create an new internal state from an the status of the CLK and DT GPIO inputs when in a certain internal state
/// </summary>
static const SwitchEncoderState s_encoderNextState[static_cast<size_t>(SwitchEncoderState::Unknown)][2][2] = {
    //  {{CLK=0/DT=0,                       CLK=0/DT=1},                      {CLK=1/DT=0,                      CLK=1/DT=1}}

    {{SwitchEncoderState::Invalid, SwitchEncoderState::CWStart},          {SwitchEncoderState::CCWStart, SwitchEncoderState::Start}     }, // Start (1, 1), this is the default state between two clicks

    {{SwitchEncoderState::CWDataFall, SwitchEncoderState::CWStart},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}  }, // CWStart (1, 0)
    {{SwitchEncoderState::CWDataFall, SwitchEncoderState::CWStart},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Invalid}}, // CWDataFall (0, 0)
    {{SwitchEncoderState::CWDataFall, SwitchEncoderState::Invalid},       {SwitchEncoderState::CWClockRise, SwitchEncoderState::Start}  }, // CWClockRise (0, 1)

    {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart, SwitchEncoderState::Start}     }, // CCWStart (0, 1)
    {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::CCWStart, SwitchEncoderState::Invalid}   }, // CCWClockFall (0, 0)
    {{SwitchEncoderState::CCWClockFall, SwitchEncoderState::CCWDataRise}, {SwitchEncoderState::Invalid, SwitchEncoderState::Start}      }, // CCWDataRise (1, 0)

    {{SwitchEncoderState::Invalid, SwitchEncoderState::Invalid},          {SwitchEncoderState::Invalid, SwitchEncoderState::Start}      }  // Invalid
};
/// <summary>
/// Get new internal state for the rotary switch
/// </summary>
/// <param name="state">Rotary switch internal state</param>
/// <param name="clkValue">Value of CLK GPIO input</param>
/// <param name="dtValue">Value of DT GPIO input</param>
/// <returns>New internal state</returns>
static SwitchEncoderState GetEncoderNextState(SwitchEncoderState state, bool clkValue, bool dtValue)
{
    return s_encoderNextState[static_cast<size_t>(state)][clkValue][dtValue];
}

/// <summary>
/// Event lookup for handling switch button state versus switch button event
///
/// Every row signifies a beginning internalstate, every column signifies an internal event, values in the table determine the resulting event
/// </summary>
static const KY040::Event s_switchOutput[static_cast<size_t>(SwitchButtonState::Unknown)][static_cast<size_t>(SwitchButtonEvent::Unknown)] = {
    // {Down,               Up,                    Click,                           DoubleClick,                     Tick}

    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::Unknown          }, // SwitchButtonState::Start
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchHold       }, // SwitchButtonState::Down
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick,       KY040::Event::SwitchDoubleClick, KY040::Event::SwitchClick      }, // SwitchButtonState::Click
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick}, // SwitchButtonState::Click2
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick}, // SwitchButtonState::Click3
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::SwitchHold       }, // SwitchButtonState::Hold
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown,           KY040::Event::Unknown,           KY040::Event::Unknown          }  // SwitchButtonState::Invalid
};
/// <summary>
/// Determine the event for the current internal state and internal event
/// </summary>
/// <param name="state">Current internal state</param>
/// <param name="event">Current internal event</param>
/// <returns>Resulting event</returns>
static KY040::Event GetSwitchOutput(SwitchButtonState state, SwitchButtonEvent event)
{
    return s_switchOutput[static_cast<size_t>(state)][static_cast<size_t>(event)];
}

/// <summary>
/// State machine for handling switch button state
///
/// Every row signifies a beginning internalstate, every column signifies an internal event, values in the table determine the resulting internal state
/// </summary>
static const SwitchButtonState s_nextSwitchState[static_cast<size_t>(SwitchButtonState::Unknown)][static_cast<size_t>(SwitchButtonEvent::Unknown)] = {
    // {Down,              Up,                  Click,               DoubleClick,          Tick}

    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Start  }, // SwitchButtonState::Start
    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Hold   }, // SwitchButtonState::Down
    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Invalid}, // SwitchButtonState::Click
    {SwitchButtonState::Down, SwitchButtonState::Click2, SwitchButtonState::Click2, SwitchButtonState::Click3, SwitchButtonState::Hold   }, // SwitchButtonState::Click2
    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click3, SwitchButtonState::Click3, SwitchButtonState::Hold   }, // SwitchButtonState::Click3
    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Hold   }, // SwitchButtonState::Hold
    {SwitchButtonState::Down, SwitchButtonState::Start,  SwitchButtonState::Click,  SwitchButtonState::Click2, SwitchButtonState::Invalid}  // SwitchButtonState::Invalid
};
/// <summary>
/// Determine the next internal state for the current internal state and internal event
/// </summary>
/// <param name="state">Current internal state</param>
/// <param name="event">Current internal event</param>
/// <returns>Resulting state</returns>
static SwitchButtonState GetSwitchNextState(SwitchButtonState state, SwitchButtonEvent event)
{
    return s_nextSwitchState[static_cast<size_t>(state)][static_cast<size_t>(event)];
}

/// <summary>
/// Constructor for KY040 class
/// </summary>
/// <param name="clkPin">GPIO pin number for CLK input</param>
/// <param name="dtPin">GPIO pin number for DT input</param>
/// <param name="swPin">GPIO pin number for SW input</param>
/// <param name="memoryAccess">MemoryAccess instance to be used for register access</param>
KY040::KY040(uint8 clkPin, uint8 dtPin, uint8 swPin, IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
    : m_isInitialized{}
    , m_clkPin(clkPin, GPIOMode::InputPullUp, memoryAccess)
    , m_dtPin(dtPin, GPIOMode::InputPullUp, memoryAccess)
    , m_swPin(swPin, GPIOMode::InputPullUp, memoryAccess)
    , m_switchEncoderState{SwitchEncoderState::Start}
    , m_switchButtonState{SwitchButtonState::Start}
    , m_debounceTimerHandle{}
    , m_tickTimerHandle{}
    , m_currentPressTicks{}
    , m_currentReleaseTicks{}
    , m_lastPressTicks{}
    , m_lastReleaseTicks{}
    , m_eventHandler{}
    , m_eventHandlerParam{}
{
    TRACE_DEBUG("KY040 constructor");
}

/// <summary>
/// Destructor for KY040 class
/// </summary>
KY040::~KY040()
{
    TRACE_DEBUG("KY040 destructor");
    Uninitialize();
}

/// <summary>
/// Initialized the KY040 rotary switch
/// </summary>
void KY040::Initialize()
{
    if (m_isInitialized)
        return;

    TRACE_DEBUG("KY040 Initialize");
    m_clkPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
    m_dtPin.ConnectInterrupt(SwitchEncoderInterruptHandler, this);
    m_swPin.ConnectInterrupt(SwitchButtonInterruptHandler, this);

    m_clkPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
    m_dtPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);
    m_swPin.EnableInterrupt(GPIOInterruptTypes::FallingEdge | GPIOInterruptTypes::RisingEdge);

    m_isInitialized = true;
}

/// <summary>
/// Uninitialize the KY040 rotary switch
/// </summary>
void KY040::Uninitialize()
{
    if (m_isInitialized)
    {
        TRACE_DEBUG("Disconnect CLK pin");
        m_clkPin.DisableAllInterrupts();
        TRACE_DEBUG("Disconnect DT pin");
        m_dtPin.DisableAllInterrupts();
        TRACE_DEBUG("Disconnect SW pin");
        m_swPin.DisableAllInterrupts();
        m_swPin.DisconnectInterrupt();
        m_isInitialized = false;
    }
    if (m_debounceTimerHandle)
    {
        GetTimer().CancelKernelTimer(m_debounceTimerHandle);
    }
    if (m_tickTimerHandle)
    {
        GetTimer().CancelKernelTimer(m_tickTimerHandle);
    }
}

/// <summary>
/// Register an event handler for the rotary switch
/// </summary>
/// <param name="handler">Handler function</param>
/// <param name="param">Parameter to pass to handler function</param>
void KY040::RegisterEventHandler(EventHandler* handler, void* param)
{
    assert(!m_eventHandler);
    m_eventHandler = handler;
    assert(m_eventHandler);
    m_eventHandlerParam = param;
}

/// <summary>
/// Unregister event handler for the rotary switch
/// </summary>
/// <param name="handler">Handler function</param>
void KY040::UnregisterEventHandler(EventHandler* handler)
{
    assert(m_eventHandler = handler);
    m_eventHandler = nullptr;
    m_eventHandlerParam = nullptr;
}

/// <summary>
/// Global GPIO pin interrupt handler for switch encoder
/// </summary>
/// <param name="pin">GPIO pin for the button encoder inputs</param>
/// <param name="param">Parameter for the interrupt handler, which is a pointer to the class instance</param>
void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin, void* param)
{
    KY040* pThis = reinterpret_cast<KY040*>(param);
    assert(pThis != nullptr);
    pThis->SwitchEncoderInterruptHandler(pin);
}

/// <summary>
/// GPIO pin interrupt handler for switch encoder
/// </summary>
/// <param name="pin">GPIO pin for the button encoder inputs</param>
void KY040::SwitchEncoderInterruptHandler(baremetal::IGPIOPin* pin)
{
    auto clkValue = m_clkPin.Get();
    auto dtValue = m_dtPin.Get();
    TRACE_DEBUG("KY040 CLK: %d", clkValue);
    TRACE_DEBUG("KY040 DT:  %d", dtValue);
    assert(m_switchEncoderState < SwitchEncoderState::Unknown);

    TRACE_DEBUG("KY040 Current state: %s", EncoderStateToString(m_switchEncoderState));
    Event event = GetEncoderOutput(m_switchEncoderState, clkValue, dtValue);
    m_switchEncoderState = GetEncoderNextState(m_switchEncoderState, clkValue, dtValue);
    TRACE_DEBUG("KY040 Event: %s", EventToString(event));
    TRACE_DEBUG("KY040 Next state: %s", EncoderStateToString(m_switchEncoderState));

    if ((event != Event::Unknown) && (m_eventHandler != nullptr))
    {
        (*m_eventHandler)(event, m_eventHandlerParam);
    }
}

/// <summary>
/// Global GPIO pin interrupt handler for the switch button
/// </summary>
/// <param name="pin">GPIO pin for the button switch</param>
/// <param name="param">Parameter for the interrupt handler, which is a pointer to the class instance</param>
void KY040::SwitchButtonInterruptHandler(IGPIOPin* pin, void* param)
{
    KY040* pThis = reinterpret_cast<KY040*>(param);
    assert(pThis != nullptr);
    pThis->SwitchButtonInterruptHandler(pin);
}

/// <summary>
/// GPIO pin interrupt handlerthe switch button
/// </summary>
/// <param name="pin">GPIO pin for the button switch</param>
void KY040::SwitchButtonInterruptHandler(IGPIOPin* pin)
{
    TRACE_DEBUG("KY040 SwitchButtonInterruptHandler");
    assert(pin != nullptr);

    /// Get Switch state (false = pressed, true = released)
    bool swValue = pin->Get();
    if (swValue)
    {
        m_currentReleaseTicks = GetTimer().GetTicks();
    }
    else
    {
        m_currentPressTicks = GetTimer().GetTicks();
    }

    if (m_debounceTimerHandle)
    {
        TRACE_DEBUG("KY040 Cancel debounce timer");
        GetTimer().CancelKernelTimer(m_debounceTimerHandle);
    }

    TRACE_DEBUG("KY040 Start debounce timer");
    m_debounceTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchDebounceDelayMilliseconds), SwitchButtonDebounceHandler, nullptr, this);
}

/// <summary>
/// Global switch button debounce handler, called by the switch button debounce timer on timeout
///
/// Will call the class internal switch button debounce handler
/// </summary>
/// <param name="handle">Kernel timer handle</param>
/// <param name="param">Timer handler parameter</param>
/// <param name="context">Timer handler context</param>
void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void* param, void* context)
{
    KY040* pThis = reinterpret_cast<KY040*>(context);
    assert(pThis != nullptr);
    pThis->SwitchButtonDebounceHandler(handle, param);
}

/// <summary>
/// Switch button debounce handler, called by the global switch button debounce handler on timeout
/// </summary>
/// <param name="handle">Kernel timer handle</param>
/// <param name="param">Timer handler parameter</param>
void KY040::SwitchButtonDebounceHandler(KernelTimerHandle handle, void* param)
{
    TRACE_DEBUG("KY040 Timeout debounce timer");
    m_debounceTimerHandle = 0;

    bool swValue = m_swPin.Get();
    auto event = swValue ? Event::SwitchUp : Event::SwitchDown;
    auto switchButtonEvent = swValue ? SwitchButtonEvent::Up : SwitchButtonEvent::Down;
    if (swValue)
    {
        if (m_currentReleaseTicks - m_lastPressTicks < MSEC2TICKS(SwitchClickMaxDelayMilliseconds))
        {
            switchButtonEvent = SwitchButtonEvent::Click;
        }
    }
    else
    {
        if (m_currentPressTicks - m_lastPressTicks < MSEC2TICKS(SwitchDoubleClickMaxDelayMilliseconds))
        {
            switchButtonEvent = SwitchButtonEvent::DblClick;
        }
    }
    TRACE_DEBUG("KY040 SW                : %d", swValue);
    TRACE_DEBUG("KY040 SW LastDown       : %d", m_lastPressTicks);
    TRACE_DEBUG("KY040 SW LastUp         : %d", m_lastReleaseTicks);
    TRACE_DEBUG("KY040 SW CurrentDown    : %d", m_currentPressTicks);
    TRACE_DEBUG("KY040 SW CurrentUp      : %d", m_currentReleaseTicks);
    if (swValue)
    {
        m_lastReleaseTicks = m_currentReleaseTicks;
    }
    else
    {
        m_lastPressTicks = m_currentPressTicks;
    }

    TRACE_DEBUG("KY040 Event             : %s", EventToString(event));
    TRACE_DEBUG("KY040 Switch Event      : %s", SwitchButtonEventToString(switchButtonEvent));
    if (m_eventHandler)
    {
        (*m_eventHandler)(event, m_eventHandlerParam);
    }

    if (m_tickTimerHandle)
    {
        GetTimer().CancelKernelTimer(m_tickTimerHandle);
    }

    if (!swValue) // If pressed, check for hold
        m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchButtonTickHandler, nullptr, this);

    HandleSwitchButtonEvent(switchButtonEvent);
}

/// <summary>
/// Global switch button tick handler, called by the switch button tick timer on timeout
///
/// Will call the class internal switch button tick handler
/// </summary>
/// <param name="handle">Kernel timer handle</param>
/// <param name="param">Timer handler parameter</param>
/// <param name="context">Timer handler context</param>
void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void* param, void* context)
{
    KY040* pThis = reinterpret_cast<KY040*>(context);
    assert(pThis != nullptr);

    pThis->SwitchButtonTickHandler(handle, param);
}

/// <summary>
/// Switch button tick handler, called by the global switch button tick handler on timeout
/// </summary>
/// <param name="handle">Kernel timer handle</param>
/// <param name="param">Timer handler parameter</param>
void KY040::SwitchButtonTickHandler(KernelTimerHandle handle, void* param)
{
    TRACE_DEBUG("KY040 Timeout tick timer");
    // Timer timed out, so we need to generate a tick
    m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchButtonTickHandler, nullptr, this);

    HandleSwitchButtonEvent(SwitchButtonEvent::Tick);
}

/// <summary>
/// Handle a switch button event
///
/// Updates the internal state of switch button, and generates the proper event
/// </summary>
/// <param name="switchButtonEvent">Internal switch button event</param>
void KY040::HandleSwitchButtonEvent(SwitchButtonEvent switchButtonEvent)
{
    assert(switchButtonEvent < SwitchButtonEvent::Unknown);

    TRACE_DEBUG("KY040 Current state     : %s", SwitchButtonStateToString(m_switchButtonState));
    TRACE_DEBUG("KY040 Switch Event      : %s", SwitchButtonEventToString(switchButtonEvent));
    Event event = GetSwitchOutput(m_switchButtonState, switchButtonEvent);
    SwitchButtonState nextState = GetSwitchNextState(m_switchButtonState, switchButtonEvent);

    TRACE_DEBUG("KY040 Event             : %s", EventToString(event));
    TRACE_DEBUG("KY040 Next state        : %s", SwitchButtonStateToString(nextState));

    m_switchButtonState = nextState;

    if ((event != Event::Unknown) && (m_eventHandler != nullptr))
    {
        (*m_eventHandler)(event, m_eventHandlerParam);
    }
}

} // namespace device
