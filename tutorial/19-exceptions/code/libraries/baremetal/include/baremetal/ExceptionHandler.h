//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ExceptionHandler.h
//
// Namespace   : -
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

#pragma once

#include <baremetal/Types.h>
#include <baremetal/System.h>

/// @file
/// Exception handler function

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Exception abort frame
///
/// Storage for register value in case of exception, in order to recover
struct AbortFrame
{
    /// @brief Exception Syndrome Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
    uint64	esr_el1;
    /// @brief Saved Program Status Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_SPSR_EL1_REGISTER
    uint64	spsr_el1;
    /// @brief General-purpose register, Link Register
    uint64	x30;
    /// @brief Exception Link Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW_ELR_EL1_REGISTER
    uint64	elr_el1;
    /// @brief Stack Pointer (EL0). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
    uint64	sp_el0;
    /// @brief Stack Pointer (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
    uint64	sp_el1;
    /// @brief Fault Address Register (EL1). See \ref ARM_REGISTERS_REGISTER_OVERVIEW
    uint64	far_el1;
    /// @brief Unused valuem used to align to 64 bytes
    uint64	unused;
}
/// @brief Just specifies the struct is packed
PACKED;

/// @brief Handles an exception, with the abort frame passed in.
///
/// The exception handler is called from assembly code (ExceptionStub.S)
/// @param exceptionID Exception type being thrown (one of EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS, EXCEPTION_SYSTEM_ERROR)
/// @param abortFrame  Filled in AbortFrame instance.
void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);

/// @brief Handles an interrupt.
///
/// The interrupt handler is called from assembly code (ExceptionStub.S)
void InterruptHandler();

#ifdef __cplusplus
}
#endif

namespace baremetal {

/// @brief Exception handling system. Handles ARM processor exceptions
/// This is a singleton class, created as soon as GetExceptionSystem() is called
class ExceptionSystem
{
    /// <summary>
    /// Construct the singleton exception system instance if needed, and return a reference to the instance. This is a friend function of class ExceptionSystem
    /// </summary>
    /// <returns>Reference to the singleton system instance</returns>
    friend ExceptionSystem& GetExceptionSystem();

private:
    ExceptionSystem();

public:
    ~ExceptionSystem();

    void Throw(unsigned exceptionID, AbortFrame* abortFrame);
};

/// <summary>
/// Injectable exception handler
/// </summary>
/// <param name="exceptionID">ID of exception (EXCEPTION_UNEXPECTED, EXCEPTION_SYNCHRONOUS or EXCEPTION_SYSTEM_ERROR)</param>
/// <param name="abortFrame">Stored state information at the time of exception</param>
/// <returns>ReturnCode::ExitHalt if the system should be halted, ReturnCode::ExitReboot if the system should reboot</returns>
using ExceptionPanicHandler = ReturnCode(unsigned exceptionID, AbortFrame* abortFrame);

const char* GetExceptionType(unsigned exceptionID);

/// <summary>
/// Register a panic handler
/// </summary>
/// <param name="handler">Exception panic handler</param>
/// <returns>The previously set handler</returns>
ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler);

ExceptionSystem& GetExceptionSystem();

} // namespace baremetal