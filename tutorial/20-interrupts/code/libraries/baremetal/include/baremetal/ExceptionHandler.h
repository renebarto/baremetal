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

/// <summary>
/// Exception abort frame
///
/// Storage for register value in case of exception, in order to recover
/// </summary>
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

void ExceptionHandler(uint64 exceptionID, AbortFrame* abortFrame);

#ifdef __cplusplus
}
#endif

namespace baremetal {

/// <summary>
/// Exception handling system. Handles ARM processor exceptions
/// 
/// This is a singleton class, created as soon as GetExceptionSystem() is called
/// </summary>
class ExceptionSystem
{
    /// <summary>
    /// Construct the singleton ExceptionSystem instance if needed, and return a reference to the instance. This is a friend function of class ExceptionSystem
    /// </summary>
    /// <returns>Reference to the singleton ExceptionSystem instance</returns>
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

ExceptionPanicHandler* RegisterExceptionPanicHandler(ExceptionPanicHandler* handler);

ExceptionSystem& GetExceptionSystem();

} // namespace baremetal
