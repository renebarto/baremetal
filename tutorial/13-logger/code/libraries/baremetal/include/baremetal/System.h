//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : System.h
//
// Namespace   : baremetal
//
// Class       : System
//
// Description : Generic character read / write device interface
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

#include "stdlib/Types.h"

/// @file
/// System startup / shutdown functionality

namespace baremetal {

class IMemoryAccess;

/// <summary>
/// System startup / shutdown handling class
/// </summary>
class System
{
    /// <summary>
    /// Construct the singleton System instance if needed, and return a reference to the instance. This is a friend function of class System
    /// </summary>
    /// <returns>Reference to the singleton system instance</returns>
    friend System& GetSystem();

private:
    /// @brief Memory access interface reference for accessing registers.
    IMemoryAccess& m_memoryAccess;

    System();

public:
    System(IMemoryAccess& memoryAccess);

    [[noreturn]] void Halt();
    [[noreturn]] void Reboot();
};

System& GetSystem();

} // namespace baremetal

/// <summary>
/// Return code for main() function
/// </summary>
enum class ReturnCode
{
    /// @brief If main() returns this, the system will be halted
    ExitHalt,
    /// @brief If main() returns this, the system will be rebooted
    ExitReboot,
};

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Forward declared main() function
/// </summary>
/// <returns>Integer cast of ReturnCode</returns>
extern int main();
/// <summary>
/// System initialization function. This is the entry point of the C / C++ code for the system for Core 0
/// </summary>
[[noreturn]] void sysinit();

#ifdef __cplusplus
}
#endif
