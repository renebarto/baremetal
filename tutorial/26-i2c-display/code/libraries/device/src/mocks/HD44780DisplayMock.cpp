//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2026 Rene Barto
//
// File        : HD44780DisplayMock.cpp
//
// Namespace   : device
//
// Class       : HD44780DisplayMock
//
// Description : HD44780Display mock for HD44780 based character display
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

#include "baremetal/Assert.h"
#include "baremetal/Format.h"
#include "baremetal/Logger.h"
#include "baremetal/String.h"
#include "device/display/HD44780Display.h"

/// @file
/// HD44780DisplayMock

/// @brief Define log name
LOG_MODULE("HD44780DisplayMock");

using namespace baremetal;
using namespace device;

/// @brief RS (Register Select) bit when writing 4 bit values
static const uint8 RSBit = BIT1(0);

/// <summary>
/// Convert operation code to string
/// </summary>
/// <param name="code">Operation cpde</param>
/// <returns>String representing operator code</returns>
static String OperationCodeToString(HD44780DisplayOperationCode code)
{
    String result{};
    switch (code)
    {
    case HD44780DisplayOperationCode::CommandInit1:
        result = "Init1";
        break;
    case HD44780DisplayOperationCode::CommandInit2:
        result = "Init2";
        break;
    case HD44780DisplayOperationCode::CommandClearDisplay:
        result = "CommandClearDisplay";
        break;
    case HD44780DisplayOperationCode::CommandReturnHome:
        result = "CommandReturnHome";
        break;
    case HD44780DisplayOperationCode::CommandEntryModeSet:
        result = "CommandEntryModeSet";
        break;
    case HD44780DisplayOperationCode::CommandDisplayControl:
        result = "CommandDisplayControl";
        break;
    case HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift:
        result = "CommandDisplayCursorMoveOrShift";
        break;
    case HD44780DisplayOperationCode::CommandFunctionSet:
        result = "CommandFunctionSet";
        break;
    case HD44780DisplayOperationCode::CommandSetCGRAMAddress:
        result = "CommandSetCGRAMAddress";
        break;
    case HD44780DisplayOperationCode::CommandSetDDRAMAddress:
        result = "CommandSetDDRAMAddress";
        break;
    case HD44780DisplayOperationCode::Data:
        result = "Data";
        break;
    }
    return result;
}

/// <summary>
/// Serialize a GPIO memory access operation to string
/// </summary>
/// <param name="value">Value to be serialized</param>
/// <returns>Resulting string</returns>
String baremetal::Serialize(const HD44780DisplayOperation &value)
{
    String result = Format("Operation=%s, ", OperationCodeToString(value.operation).c_str());
    switch (value.operation)
    {
    case HD44780DisplayOperationCode::CommandInit1:
    case HD44780DisplayOperationCode::CommandInit2:
    case HD44780DisplayOperationCode::CommandClearDisplay:
        break;
    case HD44780DisplayOperationCode::CommandReturnHome:
    case HD44780DisplayOperationCode::CommandEntryModeSet:
    case HD44780DisplayOperationCode::CommandDisplayControl:
    case HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift:
    case HD44780DisplayOperationCode::CommandFunctionSet:
    case HD44780DisplayOperationCode::CommandSetCGRAMAddress:
    case HD44780DisplayOperationCode::CommandSetDDRAMAddress:
    case HD44780DisplayOperationCode::Data:
        {
            String line = Format("%02x", value.argument);
            result += line;
        }
        break;

    default:
        result += Format("Argument=%d", value.argument);
        break;
    }
    return result;
}

/// <summary>
/// HD44780DisplayMock constructor
/// </summary>
/// <param name="numColumns">    Number of display text columns</param>
/// <param name="numRows">       Number of display text rows</param>
HD44780DisplayMock::HD44780DisplayMock(uint8 numColumns, uint8 numRows)
    : HD44780Display{numColumns, numRows, HD44780Display::CharacterSize::Size5x8, HD44780Display::DataMode::Mode4Bit}
    , m_ops{}
    , m_numOps{}
{
}

/// <summary>
/// Return number of registered memory access operations
/// </summary>
/// <returns>Number of registered memory access operations</returns>
size_t HD44780DisplayMock::GetNumOperations() const
{
    return m_numOps;
}

/// <summary>
/// Retrieve a registered memory access operation from the list
/// </summary>
/// <param name="index">Index of operation</param>
/// <returns>Requested memory access operation</returns>
const HD44780DisplayOperation &HD44780DisplayMock::GetOperation(size_t index) const
{
    assert(index < m_numOps);
    return m_ops[index];
}

static bool s_firstHalfByte = true;
static uint8 s_currentByte = 0;

void HD44780DisplayMock::WriteHalfByte(uint8 data)
{
    if (s_firstHalfByte)
    {
        s_currentByte = (data & 0xF0);
        s_firstHalfByte = false;
    }
    else
    {
        s_currentByte |= (data & 0xF0) >> 4;
        s_firstHalfByte = true;
        if ((data & RSBit) == 0)
        {
            // Full command byte received
            if (s_currentByte == 0x33)
                AddOperation({ HD44780DisplayOperationCode::CommandInit1 });
            else if (s_currentByte == 0x32)
                AddOperation({ HD44780DisplayOperationCode::CommandInit2 });
            else if (s_currentByte & 0x80)
                AddOperation({ HD44780DisplayOperationCode::CommandSetDDRAMAddress, static_cast<uint32>(s_currentByte & 0x7F) });
            else if (s_currentByte & 0x40)
                AddOperation({ HD44780DisplayOperationCode::CommandSetCGRAMAddress, static_cast<uint32>(s_currentByte & 0x3F) });
            else if (s_currentByte & 0x20)
                AddOperation({ HD44780DisplayOperationCode::CommandFunctionSet, static_cast<uint32>(s_currentByte & 0x1F) });
            else if (s_currentByte & 0x10)
                AddOperation({ HD44780DisplayOperationCode::CommandDisplayCursorMoveOrShift, static_cast<uint32>(s_currentByte & 0x0F) });
            else if (s_currentByte & 0x08)
                AddOperation({ HD44780DisplayOperationCode::CommandDisplayControl, static_cast<uint32>(s_currentByte & 0x07) });
            else if (s_currentByte & 0x04)
                AddOperation({ HD44780DisplayOperationCode::CommandEntryModeSet, static_cast<uint32>(s_currentByte & 0x03) });
            else if (s_currentByte & 0x02)
                AddOperation({ HD44780DisplayOperationCode::CommandReturnHome, static_cast<uint32>(s_currentByte & 0x01) });
            else if (s_currentByte & 0x01)
                AddOperation({ HD44780DisplayOperationCode::CommandClearDisplay });
        }
        else
        {
            AddOperation({ HD44780DisplayOperationCode::Data, static_cast<uint32>(s_currentByte) });
        }
    }
}

/// <summary>
/// Add a memory access operation to the list
/// </summary>
/// <param name="operation">Operation to add</param>
void HD44780DisplayMock::AddOperation(const HD44780DisplayOperation& operation)
{
    assert(m_numOps < BufferSize);
    m_ops[m_numOps++] = operation;
}
