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

#include <sensor/KY-040.h>

#include <baremetal/Assert.h>
#include <baremetal/Logger.h>

/// @brief Define log name
LOG_MODULE("KY-040");

namespace baremetal {
namespace sensor {

static const unsigned SwitchDebounceDelayMilliseconds       = 50;
static const unsigned SwitchTickDelayMilliseconds           = 500;
static const unsigned SwitchClickMaxDelayMilliseconds       = 200;
static const unsigned SwitchDoubleClickMaxDelayMilliseconds = 400;

enum class EncoderState
{
    Start,        // CLK high, DT high
    CWStart,      // CLK high, DT down
    CWClockFall,  // CLK down, DT low
    CWDataRise,   // CLK low, DT up
    CCWStart,     // CLK down, DT high
    CCWDataFall,  // CLK up DT down
    CCWClockRise, // CLK up, DT low
    Invalid,
    Unknown
};

enum class SwitchEvent
{
    Down,
    Up,
    Click,
    DblClick,
    Tick,
    Unknown
};

enum class SwitchState
{
    Start,
    Down,
    Click,
    Click2,
    Click3,
    Hold,
    Invalid,
    Unknown
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// cppcheck-suppress unusedFunction
static const char *EncoderStateToString(EncoderState state)
{
    switch (state)
    {
    case EncoderState::Start:
        return "Start";
    case EncoderState::CWStart:
        return "CWStart";
    case EncoderState::CWClockFall:
        return "CWClockFall";
    case EncoderState::CWDataRise:
        return "CWDataRise";
    case EncoderState::CCWStart:
        return "CCWStart";
    case EncoderState::CCWDataFall:
        return "CCWDataFall";
    case EncoderState::CCWClockRise:
        return "CCWClockRise";
    case EncoderState::Invalid:
        return "Invalid";
    case EncoderState::Unknown:
    default:
        break;
    }
    return "Unknown";
}
#pragma GCC diagnostic pop

const char            *KY040::EventToString(KY040::Event event)
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// cppcheck-suppress unusedFunction
static const char *SwitchEventToString(SwitchEvent event)
{
    switch (event)
    {
    case SwitchEvent::Down:
        return "Down";
    case SwitchEvent::Up:
        return "Up";
    case SwitchEvent::Click:
        return "Click";
    case SwitchEvent::DblClick:
        return "DblClick";
    case SwitchEvent::Tick:
        return "Tick";
    case SwitchEvent::Unknown:
    default:
        break;
    }
    return "Unknown";
}

// cppcheck-suppress unusedFunction
static const char *SwitchStateToString(SwitchState state)
{
    switch (state)
    {
    case SwitchState::Start:
        return "Start";
    case SwitchState::Down:
        return "Down";
    case SwitchState::Click:
        return "Click";
    case SwitchState::Click2:
        return "Click2";
    case SwitchState::Click3:
        return "Click3";
    case SwitchState::Hold:
        return "Hold";
    case SwitchState::Invalid:
        return "Invalid";
    case SwitchState::Unknown:
    default:
        break;
    }
    return "Unknown";
}
#pragma GCC diagnostic    pop

static const KY040::Event s_encoderOutput[static_cast<size_t>(EncoderState::Unknown)][2][2] = {
    // {{CLK=0/DT=0,           CLK=0/DT=1},             {CLK=1/DT=0,             CLK=1/DT=1}}

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}}, // Start

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},         // CWStart
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},         // CWClockFall
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateClockwise}}, // CWDataRise

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                // CCWStart
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}},                // CCWDataFall
    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::RotateCounterclockwise}}, // CCWClockRise

    {{KY040::Event::Unknown, KY040::Event::Unknown}, {KY040::Event::Unknown, KY040::Event::Unknown}} // Invalid
};
static KY040::Event GetEncoderOutput(EncoderState state, bool clkValue, bool dtValue)
{
    return s_encoderOutput[static_cast<size_t>(state)][clkValue][dtValue];
}

static const EncoderState s_encoderNextState[static_cast<size_t>(EncoderState::Unknown)][2][2] = {
    // {{CLK=0/DT=0,           CLK=0/DT=1},                    {CLK=1/DT=0,                  CLK=1/DT=1}}

    {{EncoderState::Invalid, EncoderState::CCWStart},
     {EncoderState::CWStart, EncoderState::Start}}, // Start (1, 1), this is the default state between two clicks

    {{EncoderState::CWClockFall, EncoderState::CWDataRise}, {EncoderState::CWStart, EncoderState::Start}},   // CWStart (1, 0)
    {{EncoderState::CWClockFall, EncoderState::CWDataRise}, {EncoderState::CWStart, EncoderState::Invalid}}, // CWClockFall (0, 0)
    {{EncoderState::CWClockFall, EncoderState::CWDataRise}, {EncoderState::Invalid, EncoderState::Start}},   // CWDataRise (0, 1)

    {{EncoderState::CCWDataFall, EncoderState::CCWStart}, {EncoderState::CCWClockRise, EncoderState::Start}},   // CCWStart (0, 1)
    {{EncoderState::CCWDataFall, EncoderState::CCWStart}, {EncoderState::CCWClockRise, EncoderState::Invalid}}, // CCWDataFall (0, 0)
    {{EncoderState::CCWDataFall, EncoderState::Invalid}, {EncoderState::CCWClockRise, EncoderState::Start}},    // CCWClockRise (1, 0)

    {{EncoderState::Invalid, EncoderState::Invalid}, {EncoderState::Invalid, EncoderState::Start}} // Invalid
};
static EncoderState GetEncoderNextState(EncoderState state, bool clkValue, bool dtValue)
{
    return s_encoderNextState[static_cast<size_t>(state)][clkValue][dtValue];
}

static const KY040::Event s_switchOutput[static_cast<size_t>(SwitchState::Unknown)][static_cast<size_t>(SwitchEvent::Unknown)] = {
    // {Down,               Up,                    Click,                           DoubleClick,                     Tick}

    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick, KY040::Event::SwitchDoubleClick, KY040::Event::Unknown},     // SwitchState::Start
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchHold},  // SwitchState::Down
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchClick, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchClick}, // SwitchState::Click
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchDoubleClick, KY040::Event::SwitchDoubleClick,
     KY040::Event::SwitchDoubleClick}, // SwitchState::Click2
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchTripleClick, KY040::Event::SwitchTripleClick,
     KY040::Event::SwitchTripleClick},                                                                                      // SwitchState::Click3
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::SwitchHold}, // SwitchState::Hold
    {KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown, KY040::Event::Unknown}     // SwitchState::Invalid
};
static KY040::Event GetSwitchOutput(SwitchState state, SwitchEvent event)
{
    return s_switchOutput[static_cast<size_t>(state)][static_cast<size_t>(event)];
}

static const SwitchState s_nextSwitchState[static_cast<size_t>(SwitchState::Unknown)][static_cast<size_t>(SwitchEvent::Unknown)] = {
    // {Down,              Up,                  Click,               DoubleClick,          Tick}

    {SwitchState::Down, SwitchState::Start, SwitchState::Click, SwitchState::Click2, SwitchState::Start},   // SwitchState::Start
    {SwitchState::Down, SwitchState::Start, SwitchState::Click, SwitchState::Click2, SwitchState::Hold},    // SwitchState::Down
    {SwitchState::Down, SwitchState::Start, SwitchState::Click, SwitchState::Click2, SwitchState::Invalid}, // SwitchState::Click
    {SwitchState::Down, SwitchState::Click2, SwitchState::Click2, SwitchState::Click3, SwitchState::Hold},  // SwitchState::Click2
    {SwitchState::Down, SwitchState::Start, SwitchState::Click3, SwitchState::Click3, SwitchState::Hold},   // SwitchState::Click3
    {SwitchState::Down, SwitchState::Start, SwitchState::Click, SwitchState::Click2, SwitchState::Hold},    // SwitchState::Hold
    {SwitchState::Down, SwitchState::Start, SwitchState::Click, SwitchState::Click2, SwitchState::Invalid}  // SwitchState::Invalid
};
static SwitchState GetSwitchNextState(SwitchState state, SwitchEvent event)
{
    return s_nextSwitchState[static_cast<size_t>(state)][static_cast<size_t>(event)];
}

KY040::KY040(uint8 clkPin, uint8 dtPin, uint8 swPin)
    : m_initialized{}
    , m_clkPin(clkPin, GPIOMode::InputPullUp)
    , m_dtPin(dtPin, GPIOMode::InputPullUp)
    , m_swPin(swPin, GPIOMode::InputPullUp)
    , m_encoderState{EncoderState::Start}
    , m_switchState{SwitchState::Start}
    , m_holdCounter{}
    , m_debounceTimerHandle{}
    , m_tickTimerHandle{}
    , m_currentPressTicks{}
    , m_currentReleaseTicks{}
    , m_lastPressTicks{}
    , m_lastReleaseTicks{}

    , m_eventHandler{}
    , m_eventParam{}
{
    LOG_DEBUG("KY040 constructor");
}

KY040::~KY040()
{
    LOG_DEBUG("KY040 destructor");
    if (m_initialized)
    {
        m_clkPin.DisableInterrupt();
        m_clkPin.DisableInterrupt2();
        m_clkPin.DisconnectInterrupt();
        m_dtPin.DisableInterrupt();
        m_dtPin.DisableInterrupt2();
        m_dtPin.DisconnectInterrupt();
        m_swPin.DisableInterrupt();
        m_swPin.DisableInterrupt2();
        m_swPin.DisconnectInterrupt();
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

bool KY040::Initialize()
{
    if (m_initialized)
        return true;

    LOG_DEBUG("KY040 Initialize");
    m_clkPin.ConnectInterrupt(EncoderInterruptHandler, this);
    m_dtPin.ConnectInterrupt(EncoderInterruptHandler, this);
    m_swPin.ConnectInterrupt(SwitchInterruptHandler, this);

    m_clkPin.EnableInterrupt(GPIOInterrupt::FallingEdge);
    m_clkPin.EnableInterrupt2(GPIOInterrupt::RisingEdge);

    m_dtPin.EnableInterrupt(GPIOInterrupt::FallingEdge);
    m_dtPin.EnableInterrupt2(GPIOInterrupt::RisingEdge);

    m_swPin.EnableInterrupt(GPIOInterrupt::FallingEdge);
    m_swPin.EnableInterrupt2(GPIOInterrupt::RisingEdge);

    m_initialized = true;
    return true;
}

void KY040::RegisterEventHandler(EventHandler *handler, void *param)
{
    assert(!m_eventHandler);
    m_eventHandler = handler;
    assert(m_eventHandler);
    m_eventParam = param;
}

void KY040::UnregisterEventHandler(EventHandler *handler)
{
    assert(m_eventHandler = handler);
    m_eventHandler = nullptr;
    m_eventParam = nullptr;
}

void KY040::EncoderInterruptHandler(void *param)
{
    KY040 *pThis = reinterpret_cast<KY040 *>(param);
    assert(pThis != nullptr);

    auto clkValue = pThis->m_clkPin.Get();
    auto dtValue  = pThis->m_dtPin.Get();
    LOG_DEBUG("KY040 CLK: %d", clkValue);
    LOG_DEBUG("KY040 DT:  %d", dtValue);
    assert(pThis->m_encoderState < EncoderState::Unknown);

#if DEBUG_TRACE
    GetConsole().Write("KY040 Current state: ");
    GetConsole().Write(EncoderStateToString(pThis->m_encoderState));
    GetConsole().Write("\n");
#endif
    Event event           = GetEncoderOutput(pThis->m_encoderState, clkValue, dtValue);
    pThis->m_encoderState = GetEncoderNextState(pThis->m_encoderState, clkValue, dtValue);
    LOG_DEBUG("KY040 Event: %s", EventToString(event));
    LOG_DEBUG("KY040 Next state: %s", EncoderStateToString(pThis->m_encoderState));

    if ((event != Event::Unknown) && (pThis->m_eventHandler != nullptr))
    {
        (*pThis->m_eventHandler)(event, pThis->m_eventParam);
    }
}

void KY040::SwitchInterruptHandler(void *param)
{
    LOG_DEBUG("KY040 SwitchInterruptHandler");
    KY040 *pThis = reinterpret_cast<KY040 *>(param);
    assert(pThis != nullptr);

    /// Get Switch state (false = pressed, true = released)
    bool swValue = pThis->m_swPin.Get();
    if (swValue)
    {
        pThis->m_currentReleaseTicks = GetTimer().GetTicks();
    }
    else
    {
        pThis->m_currentPressTicks = GetTimer().GetTicks();
    }

    if (pThis->m_debounceTimerHandle)
    {
        LOG_DEBUG("KY040 Cancel debounce timer");
        GetTimer().CancelKernelTimer(pThis->m_debounceTimerHandle);
    }

    LOG_DEBUG("KY040 Start debounce timer");
    pThis->m_debounceTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchDebounceDelayMilliseconds), SwitchDebounceHandler, nullptr, pThis);
}

void KY040::SwitchDebounceHandler(KernelTimerHandle handle, void *param, void *context)
{
    LOG_DEBUG("KY040 Timeout debounce timer");
    KY040 *pThis = reinterpret_cast<KY040 *>(context);
    assert(pThis != nullptr);
    pThis->SwitchDebounceHandler(handle, param);
}

void KY040::SwitchDebounceHandler(KernelTimerHandle handle, void *param)
{
    m_debounceTimerHandle = 0;

    bool swValue     = m_swPin.Get();
    auto event       = swValue ? Event::SwitchUp : Event::SwitchDown;
    auto switchEvent = swValue ? SwitchEvent::Up : SwitchEvent::Down;
    if (swValue)
    {
        if (m_currentReleaseTicks - m_lastPressTicks < MSEC2TICKS(SwitchClickMaxDelayMilliseconds))
        {
            switchEvent = SwitchEvent::Click;
        }
    }
    else
    {
        if (m_currentPressTicks - m_lastPressTicks < MSEC2TICKS(SwitchDoubleClickMaxDelayMilliseconds))
        {
            switchEvent = SwitchEvent::DblClick;
        }
    }
    LOG_DEBUG("KY040 SW                : %d", swValue);
    LOG_DEBUG("KY040 SW LastDown       : %d", m_lastPressTicks);
    LOG_DEBUG("KY040 SW LastUp         : %d", m_lastReleaseTicks);
    LOG_DEBUG("KY040 SW CurrentDown    : %d", m_currentPressTicks);
    LOG_DEBUG("KY040 SW CurrentUp      : %d", m_currentReleaseTicks);
    if (swValue)
    {
        m_lastReleaseTicks = m_currentReleaseTicks;
    }
    else
    {
        m_lastPressTicks = m_currentPressTicks;
    }

    LOG_DEBUG("KY040 Event             : %s", EventToString(event));
    LOG_DEBUG("KY040 Switch Event      : %s", SwitchEventToString(switchEvent));
    if (m_eventHandler)
    {
        (*m_eventHandler)(event, m_eventParam);
    }

    if (m_tickTimerHandle)
    {
        GetTimer().CancelKernelTimer(m_tickTimerHandle);
    }

    if (!swValue) // If pressed, check for hold
        m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchTickHandler, nullptr, this);

    HandleSwitchEvent(switchEvent);
}

void KY040::SwitchTickHandler(KernelTimerHandle handle, void *param, void *context)
{
    LOG_DEBUG("KY040 Timeout tick timer");
    KY040 *pThis = reinterpret_cast<KY040 *>(context);
    assert(pThis != nullptr);

    pThis->SwitchTickHandler(handle, param);
}

void KY040::SwitchTickHandler(KernelTimerHandle handle, void *param)
{
    // Timer timed out, so we need to generate a tick
    m_tickTimerHandle = GetTimer().StartKernelTimer(MSEC2TICKS(SwitchTickDelayMilliseconds), SwitchTickHandler, nullptr, this);

    HandleSwitchEvent(SwitchEvent::Tick);
}

void KY040::HandleSwitchEvent(SwitchEvent switchEvent)
{
    assert(switchEvent < SwitchEvent::Unknown);

    LOG_DEBUG("KY040 Current state     : %s", SwitchStateToString(m_switchState));
    LOG_DEBUG("KY040 Switch Event      : %s", SwitchEventToString(switchEvent));
    Event       event     = GetSwitchOutput(m_switchState, switchEvent);
    SwitchState nextState = GetSwitchNextState(m_switchState, switchEvent);

    LOG_DEBUG("KY040 Event             : %s", EventToString(event));
    LOG_DEBUG("KY040 Next state        : %s", SwitchStateToString(nextState));

    if (nextState == SwitchState::Hold)
    {
        if (m_switchState != SwitchState::Hold)
        {
            m_holdCounter = 0;
        }

        m_holdCounter++;
    }

    m_switchState = nextState;

    if ((event != Event::Unknown) && ((event != Event::SwitchHold) || !(m_holdCounter & 1)) && // Emit hold event each second (timer tick is half a second)
        (m_eventHandler != nullptr))
    {
        (*m_eventHandler)(event, m_eventParam);
    }
}

} // namespace sensor
} // namespace baremetal
