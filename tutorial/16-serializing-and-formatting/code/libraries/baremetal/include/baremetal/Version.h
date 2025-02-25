//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Version.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Baremetal version information
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

#pragma once

/// @file
/// Build version

/// @brief Platform name
#define BAREMETAL_NAME              "Baremetal"

/// @brief Major version number (specified by define at compile time)
#define BAREMETAL_MAJOR_VERSION     BAREMETAL_MAJOR
/// @brief Minor version number (specified by define at compile time)
#define BAREMETAL_MINOR_VERSION     BAREMETAL_MINOR
/// @brief Level version number (specified by define at compile time)
#define BAREMETAL_LEVEL_VERSION     BAREMETAL_LEVEL
/// @brief Version string
#define BAREMETAL_VERSION_STRING    GetVersion()

namespace baremetal {

void SetupVersion();
const char* GetVersion();

}
