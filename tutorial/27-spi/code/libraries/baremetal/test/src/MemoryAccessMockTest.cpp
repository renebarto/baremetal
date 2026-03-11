//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : MemoryAccessMockTest.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccessMockTest
//
// Description : Memory access mock tests
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

#include "baremetal/mocks/MemoryAccessMock.h"

#include "baremetal/BCMRegisters.h"
#include "baremetal/Logger.h"
#include "baremetal/PhysicalGPIOPin.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MemoryAccessMockTest");

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class MemoryAccessMockTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

class MemoryAccessTestMock : public MemoryAccessMock
{
public:
    uint32 m_GPIO_GPPUD{};
    uint32 m_GPIO_GPPUDCLK0{};
    uint32 m_GPIO_GPPUPPDN0{};
    uint32 m_GPIO_GPFSEL0{};
    uint32 OnRead(regaddr address) override
    {
        uint32 value{};
#if BAREMETAL_RPI_TARGET == 3
        if (address == RPI_GPIO_GPPUD)
            value = m_GPIO_GPPUD;
        else if (address == RPI_GPIO_GPPUDCLK0)
            value = m_GPIO_GPPUDCLK0;
#else
        if (address == RPI_GPIO_GPPUPPDN0)
            value = m_GPIO_GPPUPPDN0;
#endif
        if (address == RPI_GPIO_GPFSEL0)
            value = m_GPIO_GPFSEL0;
        TRACE_DEBUG("Read(%p, %02x))", address, value);
        return value;
    }
    void OnWrite(regaddr address, uint32 value) override
    {
        TRACE_DEBUG("Write(%p, %02x))", address, value);
#if BAREMETAL_RPI_TARGET == 3
        if (address == RPI_GPIO_GPPUD)
            m_GPIO_GPPUD = value;
        else if (address == RPI_GPIO_GPPUDCLK0)
            m_GPIO_GPPUDCLK0 = value;
#else
        if (address == RPI_GPIO_GPPUPPDN0)
            m_GPIO_GPPUPPDN0 = value;
#endif
        if (address == RPI_GPIO_GPFSEL0)
            m_GPIO_GPFSEL0 = value;

        return;
    }
};

TEST_FIXTURE(MemoryAccessMockTest, SimpleTest)
{
    MemoryAccessTestMock memoryAccess;
    PhysicalGPIOPin pin(memoryAccess);

    pin.AssignPin(0);
    pin.SetMode(GPIOMode::AlternateFunction0);

    size_t index{};
#if BAREMETAL_RPI_TARGET == 3
    EXPECT_EQ(size_t{5}, memoryAccess.GetNumMemoryOperations());
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUD, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000001, true }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUD);
    EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUDCLK0);
#else
    EXPECT_EQ(size_t{4}, memoryAccess.GetNumMemoryOperations());
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0 }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUPPDN0);
#endif
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0 }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0, 0x00000004, true }), memoryAccess.GetMemoryOperation(index++));
    EXPECT_EQ(uint32{0x00000004}, memoryAccess.m_GPIO_GPFSEL0);
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
