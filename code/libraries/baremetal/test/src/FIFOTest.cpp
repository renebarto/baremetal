//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : FIFOTest.cpp
//
// Namespace   : baremetal
//
// Class       : FIFOTest
//
// Description : FIFO tests
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

#include "baremetal/Logger.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("FIFOTest");

using namespace unittest;

namespace baremetal {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class FIFOTest : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(FIFOTest, FIFOIsInitiallyEmpty)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    EXPECT_TRUE(fifo.IsEmpty());
    EXPECT_FALSE(fifo.IsFull());
    EXPECT_TRUE(fifo.IsOneQuarterOrLessFull());
    EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
    EXPECT_EQ(size_t{0}, fifo.UsedSpace());
    EXPECT_EQ(size_t{4}, fifo.FreeSpace());
}

TEST_FIXTURE(FIFOTest, FIFOOneQuarterFull)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    fifo.Write(0x12);
    EXPECT_FALSE(fifo.IsEmpty());
    EXPECT_FALSE(fifo.IsFull());
    EXPECT_TRUE(fifo.IsOneQuarterOrLessFull());
    EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
    EXPECT_EQ(size_t{1}, fifo.UsedSpace());
    EXPECT_EQ(size_t{3}, fifo.FreeSpace());
}

TEST_FIXTURE(FIFOTest, FIFOHalfFull)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    fifo.Write(0x12);
    fifo.Write(0x34);
    EXPECT_FALSE(fifo.IsEmpty());
    EXPECT_FALSE(fifo.IsFull());
    EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
    EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
    EXPECT_EQ(size_t{2}, fifo.UsedSpace());
    EXPECT_EQ(size_t{2}, fifo.FreeSpace());
}

TEST_FIXTURE(FIFOTest, FIFOThreeQuartersFull)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    fifo.Write(0x12);
    fifo.Write(0x34);
    fifo.Write(0x56);
    EXPECT_FALSE(fifo.IsEmpty());
    EXPECT_FALSE(fifo.IsFull());
    EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
    EXPECT_TRUE(fifo.IsThreeQuartersOrMoreFull());
    EXPECT_EQ(size_t{3}, fifo.UsedSpace());
    EXPECT_EQ(size_t{1}, fifo.FreeSpace());
}

TEST_FIXTURE(FIFOTest, FIFOFull)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    fifo.Write(0x12);
    fifo.Write(0x34);
    fifo.Write(0x56);
    fifo.Write(0x78);
    EXPECT_FALSE(fifo.IsEmpty());
    EXPECT_TRUE(fifo.IsFull());
    EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
    EXPECT_TRUE(fifo.IsThreeQuartersOrMoreFull());
    EXPECT_EQ(size_t{4}, fifo.UsedSpace());
    EXPECT_EQ(size_t{0}, fifo.FreeSpace());
}

TEST_FIXTURE(FIFOTest, FIFOFullThenFlushed)
{
    constexpr size_t N = 4;
    FIFO<N> fifo;
    fifo.Write(0x12);
    fifo.Write(0x34);
    fifo.Write(0x56);
    fifo.Write(0x78);
    fifo.Flush();
    EXPECT_TRUE(fifo.IsEmpty());
    EXPECT_EQ(size_t{0}, fifo.UsedSpace());
    EXPECT_EQ(size_t{4}, fifo.FreeSpace());
}

} // suite Baremetal

} // namespace test
} // namespace baremetal
