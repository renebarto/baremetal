//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : MachineInfo.h
//
// Namespace   : baremetal
//
// Class       : MachineInfo
//
// Description : Basic machine info
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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

#include <baremetal/Mailbox.h>
#include <baremetal/RPIProperties.h>

/// @file
/// Machine info retrieval

namespace baremetal {

/// <summary>
/// Type of SoC used. See @ref RASPBERRY_PI_BAREMETAL_DEVELOPMENT_SOC_FOR_EACH_BOARD for more information
/// </summary>
enum class SoCType
{
    /// @ brief BCM2835 as used in Raspberry Pi Model 1 boards
    BCM2835,
    /// @ brief BCM2835 as used in older Raspberry Pi Model 2 boards
    BCM2836,
    /// @ brief BCM2835 as used in newer Raspberry Pi Model 2 and Raspberry Pi Model 3 boards
    BCM2837,
    /// @ brief BCM2835 as used in Raspberry Pi Model 4 boards
    BCM2711,
    /// @ brief BCM2835 as used in Raspberry Pi Model 5 boards
    BCM2712,
    /// @brief SoC unknown / not set / invalid
    Unknown,
};

/// <summary>
/// Retrieves system info using the mailbox mechanism
///
/// Note that this class is created as a singleton, using the GetMachineInfo() function.
/// </summary>
class MachineInfo
{
    /// <summary>
    /// Retrieves the singleton MachineInfo instance. It is created in the first call to this function. This is a friend function of class MachineInfo
    /// </summary>
    /// <returns>A reference to the singleton MachineInfo</returns>
    friend MachineInfo &GetMachineInfo();

private:
    /// @brief Flags if device was initialized. Used to guard against multiple initialization
    bool          m_initialized;
    /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
    IMemoryAccess& m_memoryAccess;
    /// @brief Raw revision code retrieved through the mailbox
    BoardRevision m_revisionRaw;
    /// @brief Board model determined from the raw revision code
    BoardModel    m_boardModel;
    /// @brief Board model major number determined from the raw revision code
    uint32        m_boardModelMajor;
    /// @brief Board model revision number determined from the raw revision code
    uint32        m_boardModelRevision;
    /// @brief Board SoC type determined from the raw revision code
    SoCType       m_SoCType;
    /// @brief Amount of physical RAM determined from the raw revision code (in Mb)
    uint32        m_ramSize;
    /// @brief Board serial number retrieved through the mailbox
    uint64        m_boardSerial;
    /// @brief Board FW revision number retrieved through the mailbox
    uint32        m_fwRevision;
    /// @brief Ethernet MAC address retrieved through the mailbox
    uint8         m_macAddress[6];
    /// @brief ARM assigned memory base address retrieved through the mailbox
    uint32        m_armBaseAddress;
    /// @brief ARM assigned memory size retrieved through the mailbox
    uint32        m_armMemorySize;
    /// @brief VideoCore assigned memory base address retrieved through the mailbox
    uint32        m_vcBaseAddress;
    /// @brief VideoCore assigned memory size retrieved through the mailbox
    uint32        m_vcMemorySize;

    MachineInfo();

public:
    MachineInfo(IMemoryAccess& memoryAccess);
    bool          Initialize();

    BoardModel    GetModel();
    const char   *GetName();
    uint32        GetModelMajor();
    uint32        GetModelRevision();
    SoCType       GetSoCType();
    const char   *GetSoCName();
    uint32        GetRAMSize();
    uint64        GetSerial();
    uint32        GetFWRevision();
    void          GetMACAddress(uint8 macAddress[6]);
    uint32        GetARMMemoryBaseAddress();
    uint32        GetARMMemorySize();
    uint32        GetVCMemoryBaseAddress();
    uint32        GetVCMemorySize();
    unsigned      GetClockRate(ClockID clockID) const; // See RPIPropertiesInterface (PROPTAG_GET_CLOCK_RATE)

    BoardRevision GetBoardRevision();
};

MachineInfo &GetMachineInfo();

} // namespace baremetal
