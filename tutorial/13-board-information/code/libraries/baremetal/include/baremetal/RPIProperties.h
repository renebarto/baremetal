//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : RPIProperties.h
//
// Namespace   : baremetal
//
// Class       : RPIProperties
//
// Description : Access to BCM2835/2836/2837/2711/2712 properties using mailbox
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

#include <baremetal/IMailbox.h>
#include <baremetal/Types.h>

/// @file
/// Top level functionality handling for Raspberry Pi Mailbox

namespace baremetal {

/// @brief Raspberry Pi board model
enum class BoardModel : uint32
{
    /// @brief Raspberry Pi 1 Model A
    RaspberryPi_A,
    /// @brief Raspberry Pi 1 Model B release 1 with 256 Mb RAM
    RaspberryPi_BRelease1MB256,
    /// @brief Raspberry Pi 1 Model B release 2 with 256 Mb RAM
    RaspberryPi_BRelease2MB256,
    /// @brief Raspberry Pi 1 Model B with 512 Mb RAM
    RaspberryPi_BRelease2MB512,
    /// @brief Raspberry Pi 1 Model A+
    RaspberryPi_APlus,
    /// @brief Raspberry Pi 1 Model B+
    RaspberryPi_BPlus,
    /// @brief Raspberry Pi Zero
    RaspberryPi_Zero,
    /// @brief Raspberry Pi Zero W
    RaspberryPi_ZeroW,
    /// @brief Raspberry Pi Zero 2 W
    RaspberryPi_Zero2W,
    /// @brief Raspberry Pi 2 Movel B
    RaspberryPi_2B,
    /// @brief Raspberry Pi 3 Model B
    RaspberryPi_3B,
    /// @brief Raspberry Pi 3 Model A+
    RaspberryPi_3APlus,
    /// @brief Raspberry Pi 3 Model B+
    RaspberryPi_3BPlus,
    /// @brief Raspberry Pi Compute Module 1
    RaspberryPi_CM,
    /// @brief Raspberry Pi Compute Module 3
    RaspberryPi_CM3,
    /// @brief Raspberry Pi Compute Module 3+
    RaspberryPi_CM3Plus,
    /// @brief Raspberry Pi 4 Model B
    RaspberryPi_4B,
    /// @brief Raspberry Pi 400
    RaspberryPi_400,
    /// @brief Raspberry Pi Compute Module 4
    RaspberryPi_CM4,
    /// @brief Raspberry Pi Compute Module 4S
    RaspberryPi_CM4S,
    /// @brief Raspberry Pi 5 Model B
    RaspberryPi_5B,
    /// @brief Model unknown / not set / invalid
    Unknown
};

/// <summary>
/// Raspberry Pi board revision number
/// </summary>
enum class BoardRevision : uint32
{
    /// @brief Raspberry Pi 1 Model B Revision 1
    RaspberryPi_BRev1        = 0x00000002,
    /// @brief Raspberry Pi 1 Model B Revision 1 no fuse
    RaspberryPi_BRev1NoFuse  = 0x00000003,
    /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
    RaspberryPi_BRev2_256_1  = 0x00000004,
    /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
    RaspberryPi_BRev2_256_2  = 0x00000005,
    /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
    RaspberryPi_BRev2_256_3  = 0x00000006,
    /// @brief Raspberry Pi 1 Model A Revision 1
    RaspberryPi_A_1          = 0x00000007,
    /// @brief Raspberry Pi 1 Model A Revision 1
    RaspberryPi_A_2          = 0x00000008,
    /// @brief Raspberry Pi 1 Model A Revision 1
    RaspberryPi_A_3          = 0x00000009,
    /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
    RaspberryPi_BRev2_512_1  = 0x0000000D,
    /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
    RaspberryPi_BRev2_512_2  = 0x0000000E,
    /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
    RaspberryPi_BRev2_512_3  = 0x0000000F,
    /// @brief Raspberry Pi 1 Model B+
    RaspberryPi_BPlus_1      = 0x00000010,
    /// @brief Raspberry Pi 1 Model B+
    RaspberryPi_BPlus_2      = 0x00000013,
    /// @brief Raspberry Pi 1 Model B+
    RaspberryPi_BPlus_3      = 0x00900032,
    /// @brief Raspberry Pi 1 Compute Module 1
    RaspberryPi_CM_1         = 0x00000011,
    /// @brief Raspberry Pi 1 Compute Module 1
    RaspberryPi_CM_2         = 0x00000014,
    /// @brief Raspberry Pi 1 Model A+ Revision 1 256 Mb RAM
    RaspberryPi_APlus_256    = 0x00000012,
    /// @brief Raspberry Pi 1 Model A+ Revision 1 512 Mb RAM
    RaspberryPi_APlus_512    = 0x00000015,
    /// @brief Raspberry Pi 2 Model B Revision 1.1.1
    RaspberryPi_2BRev1_1_1   = 0x00A01041,
    /// @brief Raspberry Pi 2 Model B Revision 1.1.2
    RaspberryPi_2BRev1_1_2   = 0x00A21041,
    /// @brief Raspberry Pi 2 Model B Revision 1.2
    RaspberryPi_2BRev1_2     = 0x00A22042,
    /// @brief Raspberry Pi Zero Revision 1.2
    RaspberryPi_ZeroRev1_2   = 0x00900092,
    /// @brief Raspberry Pi Zero Revision 1.4
    RaspberryPi_ZeroRev1_4   = 0x00900093,
    /// @brief Raspberry Pi Zero W
    RaspberryPi_ZeroW        = 0x009000C1,
    /// @brief Raspberry Pi 3 Model B Revision 1
    RaspberryPi_3B_1         = 0x00A02082,
    /// @brief Raspberry Pi 3 Model B Revision 2
    RaspberryPi_3B_2         = 0x00A22082,
    /// @brief Raspberry Pi 3 Model B+ Revision 1
    RaspberryPi_3BPlus       = 0x00A200D3,
    /// @brief Raspberry Pi 4 Model B Revision 1.1 1 Gb RAM
    RaspberryPi_4BRev1_1_1Gb = 0x00A03111,
    /// @brief Raspberry Pi 4 Model B Revision 1.1 2 Gb RAM
    RaspberryPi_4BRev1_1_2Gb = 0x00B03111,
    /// @brief Raspberry Pi 4 Model B Revision 1.2 2 Gb RAM
    RaspberryPi_4BRev1_2_2Gb = 0x00B03112,
    /// @brief Raspberry Pi 4 Model B Revision 1.1 4 Gb RAM
    RaspberryPi_4BRev1_1_4Gb = 0x00C03111,
    /// @brief Raspberry Pi 4 Model B Revision 1.2 4 Gb RAM
    RaspberryPi_4BRev1_2_4Gb = 0x00C03112,
    /// @brief Raspberry Pi 4 Model B Revision 1.4 4 Gb RAM
    RaspberryPi_4BRev1_4_4Gb = 0x00C03114,
    /// @brief Raspberry Pi 4 Model B Revision 1.4 8 Gb RAM
    RaspberryPi_4BRev1_4_8Gb = 0x00D03114,
    /// @brief Raspberry Pi 400
    RaspberryPi_400          = 0x00C03130,
    /// @brief Raspberry Pi Zero Model 2 W
    RaspberryPi_Zero2W       = 0x00902120,
    /// @brief Raspberry Pi 5 Model B Revision 1, code TBD
    RaspberryPi_5B = 0x00000000,
};

/// <summary>
/// Clock ID number. Used to retrieve and set the clock frequency for several clocks
/// </summary>
enum class ClockID : uint32
{
    /// @brief EMMC clock
    EMMC      = 1,
    /// @brief UART0 clock
    UART      = 2,
    /// @brief ARM processor clock
    ARM       = 3,
    /// @brief Core SoC clock
    CORE      = 4,
    /// @brief EMMC clock 2
    EMMC2     = 12,
    /// @brief Pixel clock
    PIXEL_BVB = 14,
};

/// <summary>
/// Top level functionality for requests on Mailbox interface
/// </summary>
class RPIProperties
{
private:
    /// @brief Reference to mailbox for functions requested
    IMailbox &m_mailbox;

public:
    explicit RPIProperties(IMailbox &mailbox);

    bool GetFirmwareRevision(uint32& revision);
    bool GetBoardModel(BoardModel& model);
    bool GetBoardRevision(BoardRevision& revision);
    bool GetBoardMACAddress(uint8 address[6]);
    bool GetBoardSerial(uint64& serial);
    bool GetARMMemory(uint32& baseAddress, uint32& size);
    bool GetVCMemory(uint32& baseAddress, uint32& size);
    bool GetClockRate(ClockID clockID, uint32& freqHz);
    bool GetMeasuredClockRate(ClockID clockID, uint32& freqHz);
    bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
};

} // namespace baremetal
