//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : SPIMasterTest.cpp
//
// Namespace   : baremetal
//
// Class       : SPIMasterTest
//
// Description : SPI master tests
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

#include "baremetal/mocks/MemoryAccessSPIMasterMock.h"

#include "stdlib/Util.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/SPIMaster.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("SPIMasterTest");

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class SPIMasterTest : public TestFixture
{
private:
    LogSeverity m_previousLevel;

public:
    void SetUp() override
    {
        m_previousLevel = GetLogger().SetLogLevel(LogSeverity::Debug);
    }
    void TearDown() override
    {
        GetLogger().SetLogLevel(m_previousLevel);
    }
};

class MemoryAccessSPIMasterMockNoReply : public MemoryAccessSPIMasterMock
{
public:
    MemoryAccessSPIMasterMockNoReply()
        : MemoryAccessSPIMasterMock()
    {
        SetSendRecvDataByteCallback(OnSendRecvData);
    }
    static void OnSendRecvData(SPIMasterRegisters &registers, uint8 dataOut, uint8& dataIn)
    {
        dataIn = 0xFF;
    }
};

class MemoryAccessSPIMasterMockDevicePresent : public MemoryAccessSPIMasterMock
{
public:
    MemoryAccessSPIMasterMockDevicePresent()
        : MemoryAccessSPIMasterMock()
    {
        SetSendRecvDataByteCallback(OnSendRecvData);
    }
    static void OnSendRecvData(SPIMasterRegisters &registers, uint8 dataOut, uint8& dataIn)
    {
        dataIn = dataOut;
    }
};

TEST_FIXTURE(SPIMasterTest, NoDevice)
{
    uint8 deviceIndex = 0;
    SPI_CEIndex ceIndex{ SPI_CEIndex::CE0 };
    MemoryAccessSPIMasterMockNoReply memoryAccess;
    {
        SPIMaster master(memoryAccess);

        master.Initialize(deviceIndex);

        uint8 dataOut[3]{0x00, 0x00, 0x00};
        uint8 dataIn[3]{};
        master.WriteRead(ceIndex, dataOut, dataIn, sizeof(dataOut));

        uint8 dataRef[3]{ 0xFF, 0xFF, 0xFF};
        EXPECT_EQ(0, memcmp(dataRef, dataIn, sizeof(dataRef)));

    }
    size_t indexGPIOOps{};
    size_t indexSPIOps{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{40}, memoryAccess.GetNumGPIOOperations());
    
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#else
    EXPECT_EQ(size_t{20}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#endif

    EXPECT_EQ(size_t{12}, memoryAccess.GetNumSPIMasterOperations());

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetClockDivider, deviceIndex, 700 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0xFF }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0xFF }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0xFF }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
}

TEST_FIXTURE(SPIMasterTest, FoundDevice)
{
    uint8 deviceIndex = 0;
    SPI_CEIndex ceIndex{ SPI_CEIndex::CE0 };
    MemoryAccessSPIMasterMockDevicePresent memoryAccess;
    {
        SPIMaster master(memoryAccess);

        master.Initialize(deviceIndex);

        uint8 dataOut[3]{0x00, 0x01, 0x02};
        uint8 dataIn[3]{};
        master.WriteRead(ceIndex, dataOut, dataIn, sizeof(dataOut));

        uint8 dataRef[3]{0x00, 0x01, 0x02};
        EXPECT_EQ(0, memcmp(dataRef, dataIn, sizeof(dataRef)));

    }
    size_t indexGPIOOps{};
    size_t indexSPIOps{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{40}, memoryAccess.GetNumGPIOOperations());
    
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#else
    EXPECT_EQ(size_t{20}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000007, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000008, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000009, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000A, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000B, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000007, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000007, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000008, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000008, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000009, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000009, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000A, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000A, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0000000B, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0000000B, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#endif

    EXPECT_EQ(size_t{12}, memoryAccess.GetNumSPIMasterOperations());

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetClockDivider, deviceIndex, 700 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x01 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x02 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x01 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x02 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
