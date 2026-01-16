//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DisplayI2CTest.cpp
//
// Namespace   : baremetal
//
// Class       : HD44780DisplayI2CTest
//
// Description : HD44780DisplayI2C tests
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

#include "device/mocks/HD44780DisplayI2CMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/I2CMaster.h"
#include "device/i2c/HD44780DisplayI2C.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("HD44780DisplayI2CTest");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class HD44780DisplayI2CTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(HD44780DisplayI2CTest, Construction)
{
    uint8 address{0x27};
    HD44780DisplayI2CMock i2cMaster(address);
    {
        const uint8 NumRows = 2;
        const uint8 NumColumns = 16;
        HD44780DisplayI2C controller(i2cMaster, address, NumColumns, NumRows);

        EXPECT_EQ(NumRows, controller.GetNumRows());
        EXPECT_EQ(NumColumns, controller.GetNumColumns());
    }
}

TEST_FIXTURE(HD44780DisplayI2CTest, Initialize)
{
    uint8 address{0x27};
    HD44780DisplayI2CMock i2cMaster(address);
    {
        const uint8 NumRows = 2;
        const uint8 NumColumns = 16;
        HD44780DisplayI2C controller(i2cMaster, address, NumColumns, NumRows);
        controller.Initialize();
    }

    size_t indexHD44780DisplayI2COps{};
    
    EXPECT_EQ(size_t{6}, i2cMaster.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000002 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::SetBacklight, 0x00000001 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::SetBacklight, 0x00000000 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    
    EXPECT_EQ(i2cMaster.GetNumOperations(), indexHD44780DisplayI2COps);
}

TEST_FIXTURE(HD44780DisplayI2CTest, SetBacklight)
{
    uint8 address{0x27};
    HD44780DisplayI2CMock i2cMaster(address);
    {
        HD44780DisplayI2C controller(i2cMaster, address, 16, 2);

        controller.Initialize();
        EXPECT_TRUE(controller.IsBacklightOn());
        controller.SetBacklight(false);
        EXPECT_FALSE(controller.IsBacklightOn());
    }

    size_t indexHD44780DisplayI2COps{};
    
    EXPECT_EQ(size_t{6}, i2cMaster.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000003 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::Command, 0x00000002 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::SetBacklight, 0x00000001 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    
    EXPECT_EQ((HD44780DisplayI2COperation{ HD44780DisplayI2COperationCode::SetBacklight, 0x00000000 }), i2cMaster.GetOperation(indexHD44780DisplayI2COps++));
    
    EXPECT_EQ(i2cMaster.GetNumOperations(), indexHD44780DisplayI2COps);
}

} // suite Baremetal

} // namespace test
} // namespace device
