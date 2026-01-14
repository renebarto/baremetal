//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : ErrorCodes.h
//
// Namespace   : baremetal
//
// Class       : -
//
// Description : Error codes
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
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

#pragma once

/// @brief Generic error group
#define GENERIC_ERROR           0x0000
/// @brief Unexpected null pointer specified
#define GENERIC_NULL_POINTER    -(GENERIC_ERROR + 1)

/// @brief I2C error group
#define I2C_BASE                0x1000
/// @brief Invalid parameter
#define I2C_MASTER_INVALID_PARM -(I2C_BASE + 1)
/// @brief Received a NACK
#define I2C_MASTER_ERROR_NACK   -(I2C_BASE + 2)
/// @brief Received clock stretch timeout
#define I2C_MASTER_ERROR_CLKT   -(I2C_BASE + 3)
/// @brief Not all data has been sent/received
#define I2C_MASTER_DATA_LEFT    -(I2C_BASE + 4)
