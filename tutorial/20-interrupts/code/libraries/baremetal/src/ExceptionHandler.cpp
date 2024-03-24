//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ExceptionHandler.cpp
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : Exception handler
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

#include <baremetal/ExceptionHandler.h>

#include <baremetal/ARMInstructions.h>
#include <baremetal/Assert.h>
#include <baremetal/Logger.h>

/// @file
/// Exception handler function implementation

/// @brief Define log name
LOG_MODULE("ExceptionHandler");

using namespace baremetal;

void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame)
{
    baremetal::GetExceptionSystem().Throw(exceptionID, abortFrame);
}

void InterruptHandler()
{
}

namespace baremetal {

/// <summary>
/// Exception panic handler
///
/// If set, the panic handler is called first, and if it returns false, the system is not halted (as it would by default)
/// </summary>
static ExceptionPanicHandler* s_exceptionPanicHandler{};

/// <summary>
/// Names exception types
///
/// Order must match exception identifiers in baremetal/Exception.h
/// </summary>
static const char* s_exceptionName[] =
{
    "Unexpected exception",
    "Synchronous exception",
    "System error"
};

/// <summary>
/// Constructor
///
/// Note that the constructor is private, so GetExceptionSystem() is needed to instantiate the exception handling system
/// </summary>
ExceptionSystem::ExceptionSystem()
{
}

/// <summary>
/// Destructor
/// </summary>
ExceptionSystem::~ExceptionSystem()
{
}

/// <summary>
/// Throw an exception to the exception system
/// </summary>
/// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
/// <param name="abortFrame">Stored state information at the time of exception</param>
void ExceptionSystem::Throw(unsigned exceptionID, AbortFrame* abortFrame)
{
    assert(abortFrame != nullptr);

    uint64 sp = abortFrame->sp_el0;
    if (SPSR_EL1_M30(abortFrame->spsr_el1) == SPSR_EL1_M30_EL1h)		// EL1h mode?
    {
        sp = abortFrame->sp_el1;
    }

    uint64 EC = ESR_EL1_EC(abortFrame->esr_el1);
    uint64 ISS = ESR_EL1_ISS(abortFrame->esr_el1);

    uint64 FAR = 0;
    if ((ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL)
        || (ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL <= EC && EC <= ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL))
    {
        FAR = abortFrame->far_el1;
    }

    ReturnCode action = ReturnCode::ExitHalt;
    if (s_exceptionPanicHandler != nullptr)
    {
        action = (*s_exceptionPanicHandler)(exceptionID, abortFrame);
    }

    if (action == ReturnCode::ExitHalt)
        LOG_PANIC("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
            s_exceptionName[exceptionID],
            abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
    else
    {
        LOG_ERROR("%s (PC %016llx, EC %016llx, ISS %016llx, FAR %016llx, SP %016llx, LR %016llx, SPSR %016llx)",
            s_exceptionName[exceptionID],
            abortFrame->elr_el1, EC, ISS, FAR, sp, abortFrame->x30, abortFrame->spsr_el1);
        GetSystem().Reboot();
    }
}

/// @brief Convert exception ID to a string
/// @param exceptionID  ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)
/// @return String representation of exception ID
const char* GetExceptionType(unsigned exceptionID)
{
    return s_exceptionName[exceptionID];
}

/// @brief Register an exception callback function, and return the previous one.
/// @param handler      Exception handler callback function to register
/// @return Previously set exception handler callback function
ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler)
{
    auto result = s_exceptionPanicHandler;
    s_exceptionPanicHandler = handler;
    return result;
}

/// @brief Retrieve the singleton exception handling system
///
/// Creates a static instance of ExceptionSystem, and returns a reference to it.
/// @return A reference to the singleton exception handling system.
ExceptionSystem& GetExceptionSystem()
{
    static ExceptionSystem system;
    return system;
}

} // namespace baremetal
