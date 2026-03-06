//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23017I2CTest.cpp
//
// Namespace   : baremetal
//
// Class       : MCP23017I2CTest
//
// Description : MCP23017 tests
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

#include "device/mocks/MemoryAccessMCP23017I2CMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/I2CMaster.h"
#include "device/i2c/MCP23017I2C.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MCP23017I2CTest");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class MCP23017I2CTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(MCP23017I2CTest, SetLEDSOn)
{
    uint8 busIndex = 1;
    uint8 address{0x20};
    MemoryAccessMCP23017I2CMock memoryAccess;
    {
        MCP23017I2C expander(memoryAccess);

        if (!expander.Initialize(busIndex, address))
        {
            LOG_INFO("Cannot initialize expander");
        }

        expander.GetPortAValue();
        expander.SetPortADirections(MCP23017PinDirection::Out);
        expander.SetPortBDirections(MCP23017PinDirection::Out);
        expander.SetPortAValue(0x55);
        expander.SetPortBValue(0xAA);
    }
    size_t indexI2COps{};

    EXPECT_EQ(size_t{126}, memoryAccess.GetNumI2CMasterOperations());
    
    // Initialize
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0A }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x2C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    // GetPortAValue()
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x12 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetReadMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ReadFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortADirections(MCP23017PinDirection::Out)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetWriteMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortBDirections(MCP23017PinDirection::Out)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0D }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x01 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortAValue(0x55)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x12 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x55 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortBValue(0xAA)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x13 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xAA }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortADirections(MCP23017PinDirection::In)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xFF }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortBDirections(MCP23017PinDirection::In)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0D }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x01 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xFF }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    EXPECT_EQ(memoryAccess.GetNumI2CMasterOperations(), indexI2COps);
    size_t indexMCP23017Ops{};

    EXPECT_EQ(size_t{12}, memoryAccess.GetNumMCP23017Operations());

    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIOCON, 0x2C }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));

    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::ReadGPIOA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));

    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));

    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPIOA, 0x55 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPIOB, 0xAA }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));

    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRA, 0xFF }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
    EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRB, 0xFF }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));

    EXPECT_EQ(memoryAccess.GetNumMCP23017Operations(), indexMCP23017Ops);
}

} // suite Baremetal

} // namespace test
} // namespace device
