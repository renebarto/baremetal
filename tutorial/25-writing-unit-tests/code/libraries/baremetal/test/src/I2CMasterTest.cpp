//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : I2CMasterTest.cpp
//
// Namespace   : baremetal
//
// Class       : I2CMasterTest
//
// Description : I2C master tests
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

#include "baremetal/mocks/MemoryAccessI2CMasterMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/I2CMaster.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("I2CMasterTest");

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class I2CMasterTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

class MemoryAccessI2CMasterMockNoReply : public MemoryAccessI2CMasterMock
{
public:
    MemoryAccessI2CMasterMockNoReply()
        : MemoryAccessI2CMasterMock()
    {
        SetSendAddressByteCallback(OnSendAddress);
        SetRecvDataByteCallback(OnRecvData);
        SetSendDataByteCallback(OnSendData);
    }
    static bool OnSendAddress(I2CMasterRegisters &registers, uint8 address)
    {
        return false;
    }
    static bool OnRecvData(I2CMasterRegisters& registers, uint8& data)
    {
        return false;
    }
    static bool OnSendData(I2CMasterRegisters& registers, uint8 data)
    {
        return false;
    }
};

class MemoryAccessI2CMasterMockDevicePresent : public MemoryAccessI2CMasterMock
{
public:
    static bool bytesWritten;
    MemoryAccessI2CMasterMockDevicePresent()
        : MemoryAccessI2CMasterMock()
    {
        SetSendAddressByteCallback(OnSendAddress);
        SetRecvDataByteCallback(OnRecvData);
        SetSendDataByteCallback(OnSendData);
    }
    static bool OnSendAddress(I2CMasterRegisters &registers, uint8 address)
    {
        return true;
    }
    static bool OnRecvData(I2CMasterRegisters& registers, uint8& data)
    {
        data = 0x00;
        if (bytesWritten)
        {
            data = 0x01;
            bytesWritten = false;
        }
        return true;
    }
    static bool OnSendData(I2CMasterRegisters& registers, uint8 data)
    {
        bytesWritten = true;
        return true;
    }
};
bool MemoryAccessI2CMasterMockDevicePresent::bytesWritten = false;

TEST_FIXTURE(I2CMasterTest, NoDevice)
{
    uint8 busIndex = 1;
    uint8 address{ 0x20 };
    MemoryAccessI2CMasterMockNoReply memoryAccess;
    {
        I2CMaster master(memoryAccess);

        master.Initialize(busIndex);

        EXPECT_FALSE(master.Scan(address));

    }
    size_t indexGPIOOps{};
    size_t indexI2COps{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{16}, memoryAccess.GetNumGPIOOperations());
    
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#else
    EXPECT_EQ(size_t{8}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#endif

    EXPECT_EQ(size_t{13}, memoryAccess.GetNumI2CMasterOperations());
    
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
}

TEST_FIXTURE(I2CMasterTest, FoundDevice)
{
    uint8 busIndex = 1;
    uint8 address{ 0x20 };
    MemoryAccessI2CMasterMockDevicePresent memoryAccess;
    {
        I2CMaster master(memoryAccess);

        master.Initialize(busIndex);

        EXPECT_TRUE(master.Scan(address));
        uint8 dataRead{};
        EXPECT_EQ(size_t{1}, master.Read(address, dataRead));
        EXPECT_EQ(uint8{0x01}, dataRead);
    }
    size_t indexGPIOOps{};
    size_t indexI2COps{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{16}, memoryAccess.GetNumGPIOOperations());

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));

    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#else
    EXPECT_EQ(size_t{8}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
#endif

    EXPECT_EQ(size_t{21}, memoryAccess.GetNumI2CMasterOperations());
    
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetReadMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ReadFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
