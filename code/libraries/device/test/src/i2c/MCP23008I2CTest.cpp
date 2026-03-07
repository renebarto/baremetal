//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MCP23008I2CTest.cpp
//
// Namespace   : baremetal
//
// Class       : MCP23008I2CTest
//
// Description : MCP23008 I2C tests
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

#include "device/mocks/MemoryAccessMCP23008I2CMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/I2CMaster.h"
#include "device/i2c/MCP23008I2C.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MCP23008I2CTest");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class MCP23008I2CTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(MCP23008I2CTest, SetLEDSOn)
{
    uint8 busIndex = 1;
    uint8 address{0x20};
    MemoryAccessMCP23008I2CMock memoryAccess;
    {
        MCP23008I2C expander(memoryAccess);

        if (!expander.Initialize(busIndex, address))
        {
            LOG_INFO("Cannot initialize expander");
        }

        expander.GetPortValue();
        expander.SetPortDirections(MCP23008PinDirection::Out);
        expander.SetPortValue(0x55);
    }
    size_t indexI2COps{};

    EXPECT_EQ(size_t{76}, memoryAccess.GetNumI2CMasterOperations());
    
    // Initialize
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x05 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x2C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    // GetPortValue()
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x09 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetReadMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ReadFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortDirections(MCP23008PinDirection::Out)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x06 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
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

    // SetPortValue(0x55)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x09 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x55 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));

    // SetPortDirections(MCP23008PinDirection::In)
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
    EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x06 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
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

    EXPECT_EQ(memoryAccess.GetNumI2CMasterOperations(), indexI2COps);
    size_t indexMCP23008Ops{};

    EXPECT_EQ(size_t{7}, memoryAccess.GetNumMCP23008Operations());

    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteIOCON, 0x2C }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));

    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::ReadGPIO, 0x00 }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));

    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteGPPU, 0x00 }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));
    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteIODIR, 0x00 }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));

    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteGPIO, 0x55 }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));

    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteGPPU, 0x00 }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));
    EXPECT_EQ((MCP23008Operation{ MCP23008OperationCode::WriteIODIR, 0xFF }), memoryAccess.GetMCP23008Operation(indexMCP23008Ops++));

    EXPECT_EQ(memoryAccess.GetNumMCP23008Operations(), indexMCP23008Ops);
}

} // suite Baremetal

} // namespace test
} // namespace device
