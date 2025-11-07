//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MachineInfo.cpp
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

#include "baremetal/MachineInfo.h"

#include "baremetal/Assert.h"
#include "baremetal/Console.h"
#include "baremetal/Mailbox.h"
#include "baremetal/MemoryAccess.h"
#include "stdlib/Util.h"

/// @file
/// Machine info retrieval implementation

namespace baremetal {

/// <summary>
/// Raspberry Pi board information
/// </summary>
struct BoardInfo
{
    /// @brief Board type
    unsigned type;
    /// @brief Board model
    BoardModel model;
    /// @brief Board major revision number
    unsigned majorRevision;
};

/// @brief Mapping from raw board revision to board model and major revision number
static BoardInfo s_boardInfo[]{
    {0,  BoardModel::RaspberryPi_A,              1},
    {1,  BoardModel::RaspberryPi_BRelease2MB512, 1}, // can be other revision
    {2,  BoardModel::RaspberryPi_APlus,          1},
    {3,  BoardModel::RaspberryPi_BPlus,          1},
    {4,  BoardModel::RaspberryPi_2B,             2},
    {6,  BoardModel::RaspberryPi_CM,             1},
    {8,  BoardModel::RaspberryPi_3B,             3},
    {9,  BoardModel::RaspberryPi_Zero,           1},
    {10, BoardModel::RaspberryPi_CM3,            3},
    {12, BoardModel::RaspberryPi_ZeroW,          1},
    {13, BoardModel::RaspberryPi_3BPlus,         3},
    {14, BoardModel::RaspberryPi_3APlus,         3},
    {16, BoardModel::RaspberryPi_CM3Plus,        3},
    {17, BoardModel::RaspberryPi_4B,             4},
    {18, BoardModel::RaspberryPi_Zero2W,         3},
    {19, BoardModel::RaspberryPi_400,            4},
    {20, BoardModel::RaspberryPi_CM4,            4},
    {21, BoardModel::RaspberryPi_CM4S,           4},
    {99, BoardModel::RaspberryPi_5B,             4}
};

/// <summary>
/// Mapping from BoardModel to board name
///
/// Must match BoardModel one to one
/// </summary>
/* clang-format off */
static const char* m_boardName[] =
{
    "Raspberry Pi Model A",
    "Raspberry Pi Model B R1",
    "Raspberry Pi Model B R2",
    "Raspberry Pi Model B R2",
    "Raspberry Pi Model A+",
    "Raspberry Pi Model B+",
    "Raspberry Pi Zero",
    "Raspberry Pi Zero W",
    "Raspberry Pi Zero 2 W",
    "Raspberry Pi 2 Model B",
    "Raspberry Pi 3 Model B",
    "Raspberry Pi 3 Model A+",
    "Raspberry Pi 3 Model B+",
    "Compute Module",
    "Compute Module 3",
    "Compute Module 3+",
    "Raspberry Pi 4 Model B",
    "Raspberry Pi 400",
    "Compute Module 4",
    "Compute Module 4S",
    "Raspberry Pi 5 Model B",
    "Unknown",
};

/// <summary>
/// Mapping from SoC type to SoC name
///
/// Must match SoCType one to one
/// </summary>
static const char* s_SoCName[] =
{
    "BCM2835",
    "BCM2836",
    "BCM2837",
    "BCM2711",
    "BCM2712",
    "Unknown",
};
/* clang-format on */

/// <summary>
/// Constructs a default MachineInfo instance (a singleton). Note that the constructor is private, so GetMachineInfo() is needed to instantiate the
/// MachineInfo.
/// </summary>
MachineInfo::MachineInfo()
    : m_initialized{}
    , m_memoryAccess{GetMemoryAccess()}
    , m_revisionRaw{}
    , m_boardModel{BoardModel::Unknown}
    , m_boardModelMajor{}
    , m_boardModelRevision{}
    , m_SoCType{SoCType::Unknown}
    , m_ramSize{}
    , m_boardSerial{}
    , m_fwRevision{}
    , m_macAddress{}
    , m_armBaseAddress{}
    , m_armMemorySize{}
    , m_vcBaseAddress{}
    , m_vcMemorySize{}
{
}

/// <summary>
/// Constructs a specialized MachineInfo instance which injects a custom IMemoryAccess instance. This is intended for testing.
/// </summary>
/// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
MachineInfo::MachineInfo(IMemoryAccess& memoryAccess)
    : m_initialized{}
    , m_memoryAccess{memoryAccess}
    , m_revisionRaw{}
    , m_boardModel{BoardModel::Unknown}
    , m_boardModelMajor{}
    , m_boardModelRevision{}
    , m_SoCType{SoCType::Unknown}
    , m_ramSize{}
    , m_boardSerial{}
    , m_fwRevision{}
    , m_macAddress{}
    , m_armBaseAddress{}
    , m_armMemorySize{}
    , m_vcBaseAddress{}
    , m_vcMemorySize{}
{
}

/// <summary>
/// Initialize a MachineInfo instance
///
/// The member variable m_initialized is used to guard against multiple initialization.
/// The initialization will determine information concerning the board as well as memory and division between ARM and VideoCore, and store this for
/// later retrieval
/// </summary>
/// <returns>Returns true on success, false on failure</returns>
bool MachineInfo::Initialize()
{
    if (!m_initialized)
    {
        Mailbox mailbox{MailboxChannel::ARM_MAILBOX_CH_PROP_OUT};
        RPIProperties properties(mailbox);

        if (!properties.GetFirmwareRevision(m_fwRevision))
        {
            GetConsole().Write("Failed to retrieve FW revision\n");
        }

        if (!properties.GetBoardRevision(m_revisionRaw))
        {
            GetConsole().Write("Failed to retrieve board revision\n");
        }

        if (!properties.GetBoardSerial(m_boardSerial))
        {
            GetConsole().Write("Failed to retrieve board serial number\n");
        }

        if (!properties.GetBoardMACAddress(m_macAddress))
        {
            GetConsole().Write("Failed to retrieve MAC address\n");
        }

        if (!properties.GetARMMemory(m_armBaseAddress, m_armMemorySize))
        {
            GetConsole().Write("Failed to retrieve ARM memory info\n");
        }

        if (!properties.GetVCMemory(m_vcBaseAddress, m_vcMemorySize))
        {
            GetConsole().Write("Failed to retrieve VC memory info\n");
        }

        unsigned type = (static_cast<unsigned>(m_revisionRaw) >> 4) & 0xFF;
        size_t index{};
        size_t count = sizeof(s_boardInfo) / sizeof(s_boardInfo[0]);
        for (index = 0; index < count; ++index)
        {
            if (s_boardInfo[index].type == type)
            {
                break;
            }
        }

        if (index >= count)
        {
            return false;
        }

        m_boardModel = s_boardInfo[index].model;
        m_boardModelMajor = s_boardInfo[index].majorRevision;
        m_boardModelRevision = (static_cast<unsigned>(m_revisionRaw) & 0xF) + 1;
        m_SoCType = static_cast<SoCType>((static_cast<unsigned>(m_revisionRaw) >> 12) & 0xF);
        m_ramSize = 256 << ((static_cast<unsigned>(m_revisionRaw) >> 20) & 7);
        if (m_boardModel == BoardModel::RaspberryPi_BRelease2MB512 && m_ramSize == 256)
        {
            m_boardModel = (m_boardModelRevision == 1) ? BoardModel::RaspberryPi_BRelease1MB256 : BoardModel::RaspberryPi_BRelease2MB256;
        }
        if (static_cast<unsigned>(m_SoCType) >= static_cast<unsigned>(SoCType::Unknown))
        {
            m_SoCType = SoCType::Unknown;
        }

        m_initialized = true;
    }
    return true;
}

/// <summary>
/// Returns board model
/// </summary>
/// <returns>Board model</returns>
BoardModel MachineInfo::GetModel()
{
    return m_boardModel;
}

/// <summary>
/// Returns board name
/// </summary>
/// <returns>Board name</returns>
const char* MachineInfo::GetName()
{
    return m_boardName[static_cast<size_t>(m_boardModel)];
}

/// <summary>
/// Returns the major board model number
/// </summary>
/// <returns>Major board model number</returns>
uint32 MachineInfo::GetModelMajor()
{
    return m_boardModelMajor;
}

/// <summary>
/// Returns the board model revision
/// </summary>
/// <returns>Board model revision</returns>
uint32 MachineInfo::GetModelRevision()
{
    return m_boardModelRevision;
}

/// <summary>
/// Returns the SoC type
/// </summary>
/// <returns>SoC type</returns>
SoCType MachineInfo::GetSoCType()
{
    return m_SoCType;
}

/// <summary>
/// Returns the SoC name
/// </summary>
/// <returns>SoC name</returns>
const char* MachineInfo::GetSoCName()
{
    return s_SoCName[static_cast<size_t>(m_SoCType)];
}

/// <summary>
/// Returns the amount of RAM on board in Mb
/// </summary>
/// <returns>RAM size in Mb</returns>
uint32 MachineInfo::GetRAMSize()
{
    return m_ramSize;
}

/// <summary>
/// Returns the board serial number
/// </summary>
/// <returns>Board serial number</returns>
uint64 MachineInfo::GetSerial()
{
    return m_boardSerial;
}

/// <summary>
/// Returns the board FW revision
/// </summary>
/// <returns>Board FW revision</returns>
uint32 MachineInfo::GetFWRevision()
{
    return m_fwRevision;
}

/// <summary>
/// Returns the raw board revision
/// </summary>
/// <returns>Raw board revision</returns>
BoardRevision MachineInfo::GetBoardRevision()
{
    return m_revisionRaw;
}

/// <summary>
/// Returns the MAC address for the network interface
/// </summary>
/// <param name="macAddress">Network MAC address</param>
void MachineInfo::GetMACAddress(uint8 macAddress[6])
{
    memcpy(macAddress, m_macAddress, sizeof(m_macAddress));
}

/// <summary>
/// Returns the ARM memory base address
/// </summary>
/// <returns>ARM memory base address</returns>
uint32 MachineInfo::GetARMMemoryBaseAddress()
{
    return m_armBaseAddress;
}

/// <summary>
/// Returns the amount of memory assigned to the ARM cores in bytes
/// </summary>
/// <returns>Amount of memory assigned to the ARM cores in bytes</returns>
uint32 MachineInfo::GetARMMemorySize()
{
    return m_armMemorySize;
}

/// <summary>
/// Returns the VideoCore memory base address
/// </summary>
/// <returns>VideoCore memory base address</returns>
uint32 MachineInfo::GetVCMemoryBaseAddress()
{
    return m_vcBaseAddress;
}

/// <summary>
/// Returns the amount of memory assigned to the VideoCore in bytes
/// </summary>
/// <returns>Amount of memory assigned to the VideoCore in bytes</returns>
uint32 MachineInfo::GetVCMemorySize()
{
    return m_vcMemorySize;
}

/// <summary>
/// Determine and return the clock rate for a specific clock, or return an estimate
/// </summary>
/// <param name="clockID"></param>
/// <returns></returns>
unsigned MachineInfo::GetClockRate(ClockID clockID) const
{
    Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
    RPIProperties properties(mailbox);
    uint32 clockRate{};
    if (properties.GetClockRate(clockID, clockRate))
        return clockRate;
    if (properties.GetMeasuredClockRate(clockID, clockRate))
        return clockRate;

    // if clock rate can not be requested, use a default rate
    unsigned result = 0;

    switch (clockID)
    {
    case ClockID::EMMC:
    case ClockID::EMMC2:
        result = 100000000;
        break;

    case ClockID::UART:
        result = 48000000;
        break;

    case ClockID::CORE:
        result = 300000000; /// \todo Check this
        break;

    case ClockID::PIXEL_BVB:
        break;

    default:
        assert(0);
        break;
    }

    return result;
}

/// <summary>
/// Create the singleton MachineInfo instance if needed, initialize it, and return a reference
/// </summary>
/// <returns>Singleton MachineInfo reference</returns>
MachineInfo& GetMachineInfo()
{
    static MachineInfo machineInfo;
    machineInfo.Initialize();
    return machineInfo;
}

} // namespace baremetal
