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

#include "baremetal/Logger.h"
#include "stdlib/Util.h"
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
    
    EXPECT_EQ(size_t{6}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, ClearDisplay)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.ClearDisplay();

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{8}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandReturnHome }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, Home)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.Home();

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{7}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandReturnHome }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, ShiftDisplayLeftRight)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.ShiftDisplay(-1);
    controller.ShiftDisplay(2);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{9}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift, 0x08 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift, 0x0C }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift, 0x0C }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, DisplayOnOff)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.SetDisplayEnabled(true);
    controller.SetDisplayEnabled(false);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{8}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000004 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, CursorOffLineBlock)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.SetCursorMode(HD44780Display::CursorMode::Hide);
    controller.SetCursorMode(HD44780Display::CursorMode::Line);
    controller.SetCursorMode(HD44780Display::CursorMode::Blink);
    controller.SetCursorMode(HD44780Display::CursorMode::BlinkLine);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{10}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000001 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000003 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, CursorPosition)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    controller.SetCursorPosition(1, 0);
    controller.SetCursorPosition(2, 1);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{8}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandSetDDRAMAddress, 0x00000001 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandSetDDRAMAddress, 0x00000042 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, DefineCharacter)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    uint8 char08[8] = 
    {
        0b00000,
        0b00100,
        0b01010,
        0b10001,
        0b10001, 
        0b01010,
        0b00100,
        0b00000
    };
    uint8 char09[8] = 
    {
        0b00000, 
        0b01010, 
        0b11111, 
        0b11111, 
        0b11111, 
        0b01110, 
        0b00100, 
        0b00000
    };
    controller.DefineCharFont(0x08, char08);
    controller.DefineCharFont(0x09, char09);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{24}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));

    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandSetCGRAMAddress, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));

    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000004 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000000A }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000011 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000011 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000000A }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000004 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));

    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandSetCGRAMAddress, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));

    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000000A }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000001F }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000001F }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000001F }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x0000000E }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000004 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

TEST_FIXTURE(HD44780DisplayTest, WriteText)
{
    const uint8 NumRows = 2;
    const uint8 NumColumns = 16;
    HD44780DisplayMock controller(NumColumns, NumRows);
    {
        controller.Initialize();
    }
    const char* text = "Hello";
    controller.Write(text);

    size_t indexHD44780DisplayOps{};
    
    EXPECT_EQ(size_t{11}, controller.GetNumOperations());
    
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit1 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandInit2 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandFunctionSet, 0x00000008 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandDisplayControl, 0x00000000 }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandClearDisplay }), controller.GetOperation(indexHD44780DisplayOps++));
    EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::CommandEntryModeSet, 0x00000002 }), controller.GetOperation(indexHD44780DisplayOps++));
    for (size_t i = 0; i < strlen(text); ++i)
    {
        EXPECT_EQ((HD44780DisplayOperation{ HD44780DisplayOperationCode::Data, static_cast<uint32>(text[i]) }), controller.GetOperation(indexHD44780DisplayOps++));
    }
    
    EXPECT_EQ(controller.GetNumOperations(), indexHD44780DisplayOps);

    uint8 column{};
    uint8 row{};
    controller.GetCursorPosition(column, row);
    EXPECT_EQ(uint8{0}, column);
    EXPECT_EQ(uint8{0}, row);
}

} // suite Baremetal

} // namespace test
} // namespace device
