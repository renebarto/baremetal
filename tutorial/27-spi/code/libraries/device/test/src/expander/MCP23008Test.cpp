//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : MCP23008Test.cpp
//
// Namespace   : baremetal
//
// Class       : MCP23008Test
//
// Description : MCP23008 tests
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
#include "device/expander/MCP23008.h"

#include "unittest/unittest.h"

/// @brief Define log name
LOG_MODULE("MCP23008Test");

using namespace baremetal;
using namespace unittest;

namespace device {
namespace test {

/// @brief Baremetal test suite
TEST_SUITE(Baremetal)
{

/// <summary>
/// MCP23008 implementation overriding register access for testing
/// </summary>
class MCP23008Impl
    : public MCP23008
{
private:
    /// @brief Storage for MCP23008 registers
    uint8 m_registers[static_cast<size_t>(MCP23008RegisterIndex::OLAT) + 1];

public:
    /// <summary>
    /// Constructor for MCP23008Impl class
    /// </summary>
    MCP23008Impl()
        : m_registers{ 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    {
    }
    uint8 ReadRegister(MCP23008RegisterIndex registerAddress) override
    {
        assert((registerAddress >= MCP23008RegisterIndex::IODIR) && (registerAddress <= MCP23008RegisterIndex::OLAT));
        return m_registers[static_cast<size_t>(registerAddress)];
    }
    void WriteRegister(MCP23008RegisterIndex registerAddress, uint8 byte) override
    {
        assert((registerAddress >= MCP23008RegisterIndex::IODIR) && (registerAddress <= MCP23008RegisterIndex::OLAT));
        m_registers[static_cast<size_t>(registerAddress)] = byte;
    }
    uint8 operator[](MCP23008RegisterIndex registerAddress)
    {
        return ReadRegister(registerAddress);
    }
};

class MCP23008Test : public TestFixture
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_FIXTURE(MCP23008Test, Initialize)
{
    MCP23008Impl expander;

    expander.Initialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));
}

TEST_FIXTURE(MCP23008Test, SetPortOut)
{
    MCP23008Impl expander;

    expander.Initialize();

    expander.SetPortDirections(MCP23008PinDirection::Out);

    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));
}

TEST_FIXTURE(MCP23008Test, SetPortValue)
{
    MCP23008Impl expander;

    expander.Initialize();

    expander.SetPortDirections(MCP23008PinDirection::Out);
    expander.SetPortValue(0x55);

    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b01010101}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));

    expander.Uninitialize();

    EXPECT_EQ(int{0b11111111}, static_cast<int>(expander[MCP23008RegisterIndex::IODIR]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::IPOL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPINTEN]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::DEFVAL]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCON]));
    EXPECT_EQ(int{0b00101100}, static_cast<int>(expander[MCP23008RegisterIndex::IOCON]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::GPPU]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTF]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::INTCAP]));
    EXPECT_EQ(int{0b01010101}, static_cast<int>(expander[MCP23008RegisterIndex::GPIO]));
    EXPECT_EQ(int{0b00000000}, static_cast<int>(expander[MCP23008RegisterIndex::OLAT]));
}

} // suite Baremetal

} // namespace test
} // namespace device
