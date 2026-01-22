//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : PhysicalGPIOPinTest.cpp
//
// Namespace   : baremetal
//
// Class       : PhysicalGPIOPinTest
//
// Description : GPIO memory access mock tests
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

#include "baremetal/mocks/MemoryAccessGPIOMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/PhysicalGPIOPin.h"

#include "unittest/unittest.h"

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class PhysicalGPIOPinTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(PhysicalGPIOPinTest, SimpleTest)
{
    MemoryAccessGPIOMock memoryAccess;
    PhysicalGPIOPin pin(memoryAccess);

    pin.AssignPin(0);
    pin.SetMode(GPIOMode::AlternateFunction0);

    size_t index{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{4}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(index++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x1 }), memoryAccess.GetGPIOOperation(index++));
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x0 }), memoryAccess.GetGPIOOperation(index++));
#else
    EXPECT_EQ(size_t{2}, memoryAccess.GetNumGPIOOperations());
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0, 0x0 }), memoryAccess.GetGPIOOperation(index++));
#endif
    EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(index++));
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
