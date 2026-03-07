//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MCP23017Test.cpp
//
// Namespace   : baremetal
//
// Class       : MCP23017Test
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

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "device/expander/MCP23017.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MCP23017Test");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// <summary>
/// MCP23017 implementation overriding register access for testing
/// </summary>
class MCP23017Impl
    : public MCP23017
{
private:
    /// @brief Storage for MCP23017 registers
    uint8 m_registers[static_cast<size_t>(MCP23017RegisterIndex::OLATB) + 1];

public:
    /// <summary>
    /// Constructor for MCP23017Impl class
    /// </summary>
    MCP23017Impl()
        : m_registers{ 
            0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            }
    {
    }
    uint8 ReadRegister(MCP23017RegisterIndex registerAddress) override
    {
        assert((registerAddress >= MCP23017RegisterIndex::IODIRA) && (registerAddress <= MCP23017RegisterIndex::OLATB));
        return m_registers[static_cast<size_t>(registerAddress)];
    }
    void WriteRegister(MCP23017RegisterIndex registerAddress, uint8 byte) override
    {
        assert((registerAddress >= MCP23017RegisterIndex::IODIRA) && (registerAddress <= MCP23017RegisterIndex::OLATB));
        if ((registerAddress == MCP23017RegisterIndex::IOCONA) || (registerAddress == MCP23017RegisterIndex::IOCONB))
            m_registers[MCP23017RegisterIndex::IOCONA] = m_registers[MCP23017RegisterIndex::IOCONB] = byte;
        else
            m_registers[static_cast<size_t>(registerAddress)] = byte;            
    }
    /// <summary>
    /// Index operator for reading register values
    /// </summary>
    /// <param name="registerAddress">Register index</param>
    /// <returns>Register value</returns>
    uint8 operator[](MCP23017RegisterIndex registerAddress)
    {
        return ReadRegister(registerAddress);
    }
};

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

class MCP23017Test : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(MCP23017Test, Initialize)
{
    MCP23017Impl expander;

    expander.Initialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));
}

TEST_FIXTURE(MCP23017Test, SetPortOut)
{
    MCP23017Impl expander;

    expander.Initialize();

    expander.SetPortADirections(MCP23017PinDirection::Out);
    expander.SetPortBDirections(MCP23017PinDirection::Out);

    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));
}

TEST_FIXTURE(MCP23017Test, SetPortValue)
{
    MCP23017Impl expander;

    expander.Initialize();

    expander.SetPortADirections(MCP23017PinDirection::Out);
    expander.SetPortBDirections(MCP23017PinDirection::Out);
    expander.SetPortAValue(0x55);
    expander.SetPortBValue(0xAA);

    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b01010101}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b10101010}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRA]));
    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23017RegisterIndex::IODIRB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::IPOLB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPINTENB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::DEFVALB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCONB]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONA]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23017RegisterIndex::IOCONB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::GPPUB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTFB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::INTCAPB]));
    EXPECT_EQ(int{0b01010101}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOA]));
    EXPECT_EQ(int{0b10101010}, static_cast<int>(expander[MCP23017RegisterIndex::GPIOB]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATA]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23017RegisterIndex::OLATB]));
}

} // suite Baremetal

} // namespace test
} // namespace device
