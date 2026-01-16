//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : HD44780DisplayTest.cpp
//
// Namespace   : baremetal
//
// Class       : HD44780DisplayTest
//
// Description : HD44780Display tests
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

#include "device/mocks/HD44780DisplayMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/I2CMaster.h"
#include "device/display/HD44780Display.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("HD44780DisplayTest");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class HD44780DisplayTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};


TEST_FIXTURE(HD44780DisplayTest, Construction)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        EXPECT_EQ(NumRows, controller.GetNumRows());
        EXPECT_EQ(NumColumns, controller.GetNumColumns());
    }
}

TEST_FIXTURE(HD44780DisplayTest, Initialize)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{4}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Command, 0x00000003 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Command, 0x00000003 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Command, 0x00000003 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Command, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);
}

} // suite Baremetal

} // namespace test
} // namespace device
