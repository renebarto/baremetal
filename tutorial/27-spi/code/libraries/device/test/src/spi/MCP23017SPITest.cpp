//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MCP23017SPITest.cpp
//
// Namespace   : baremetal
//
// Class       : MCP23017SPITest
//
// Description : MCP23017 SPI tests
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

#include "device/mocks/MemoryAccessMCP23017SPIMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/SPIMaster.h"
#include "device/spi/MCP23017SPI.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MCP23017SPITest");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class MCP23017SPITest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(MCP23017SPITest, SetLEDSOn)
{
    uint8 deviceIndex = 0;
    SPI_CEIndex ceIndex = SPI_CEIndex::CE0;
    MemoryAccessMCP23017SPIMock memoryAccess;
    {
        MCP23017SPI expander(memoryAccess);

        if (!expander.Initialize(deviceIndex, ceIndex))
        {
            LOG_INFO("Cannot initialize expander");
        }

        expander.GetPortAValue();
        expander.SetPortADirections(MCP23017PinDirection::Out);
        expander.SetPortBDirections(MCP23017PinDirection::Out);
        expander.SetPortAValue(0x55);
        expander.SetPortBValue(0xAA);
    }
    size_t indexSPIOps{};

    EXPECT_EQ(size_t{138}, memoryAccess.GetNumSPIMasterOperations());
    
    // Initialize
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetClockDivider, deviceIndex, 700 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x0A }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x2C }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // GetPortValue()
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 2 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x41 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x12 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 1 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortADirections(MCP23017PinDirection::Out)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x0C }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortBDirections(MCP23017PinDirection::Out)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x0D }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x01 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortAValue(0x55)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x12 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x55 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortBValue(0xAA)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x13 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0xAA }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortADirections(MCP23017PinDirection::In)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x0C }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0xFF }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    // SetPortBDirections(MCP23017PinDirection::In)
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x0D }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::SetDataLength, deviceIndex, 3 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::StartTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearRxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ClearTxFIFO, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x40 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0x01 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::WriteFIFO, deviceIndex, 0xFF }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::ReadFIFO, deviceIndex, 0x00 }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));
    EXPECT_EQ((SPIMasterOperation{ SPIMasterOperationCode::EndTransfer, deviceIndex }), memoryAccess.GetSPIMasterOperation(indexSPIOps++));

    EXPECT_EQ(memoryAccess.GetNumSPIMasterOperations(), indexSPIOps);
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
