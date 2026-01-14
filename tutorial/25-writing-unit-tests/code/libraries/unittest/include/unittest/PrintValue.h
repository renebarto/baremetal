//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : PrintValue.h
//
// Namespace   : unittest
//
// Class       : -
//
// Description : Value printer
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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

#include "baremetal/Serialization.h"
#include "baremetal/String.h"

/// @file
/// Print values to string

/// @brief Used to denote a nullptr type
using nullptr_t = decltype(nullptr);

/// <summary>
/// Direct cast to another type
/// </summary>
/// <typeparam name="To">Type to cast to</typeparam>
/// <param name="x">Value to be casted</param>
/// <returns>Casted value</returns>
template <typename To>
inline To ImplicitCast_(To x)
{
    return x;
}

/// <summary>
/// Print a value to string using a serializer
/// </summary>
/// <typeparam name="T">Type of value to print</typeparam>
/// <param name="value">Value to print</param>
/// <param name="s">Resulting string</param>
template <typename T>
void PrintTo(const T& value, baremetal::String& s)
{
    s = baremetal::Serialize(value);
}
/// <summary>
/// Print a unsigned character value to string
/// </summary>
/// <param name="c">Value to print</param>
/// <param name="s">Resulting string</param>
void PrintTo(unsigned char c, baremetal::String& s);
/// <summary>
/// Print a signed character value to string
/// </summary>
/// <param name="c">Value to print</param>
/// <param name="s">Resulting string</param>
void PrintTo(signed char c, baremetal::String& s);
/// <summary>
/// Print a character value to string
/// </summary>
/// <param name="c">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(char c, baremetal::String& s)
{
    PrintTo(static_cast<unsigned char>(c), s);
}

/// <summary>
/// Print a boolean value to string
/// </summary>
/// <param name="x">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(bool x, baremetal::String& s)
{
    s = (x ? "true" : "false");
}

/// <summary>
/// Print a const char* to string
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
void PrintTo(const char* str, baremetal::String& s);
/// <summary>
/// Print a char* to string
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(char* str, baremetal::String& s)
{
    PrintTo(ImplicitCast_<const char*>(str), s);
}

/// <summary>
/// Print a signed char* to string
///
/// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(const signed char* str, baremetal::String& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
/// <summary>
/// Print a signed char* to string
///
/// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(signed char* str, baremetal::String& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
/// <summary>
/// Print a signed char* to string
///
/// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(const unsigned char* str, baremetal::String& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
/// <summary>
/// Print a signed char* to string
///
/// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(unsigned char* str, baremetal::String& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}

/// <summary>
/// Print a string to string
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
void PrintStringTo(const baremetal::String& str, baremetal::String& s);
/// <summary>
/// Print a string to string
/// </summary>
/// <param name="str">Value to print</param>
/// <param name="s">Resulting string</param>
inline void PrintTo(const baremetal::String& str, baremetal::String& s)
{
    PrintStringTo(str, s);
}

/// <summary>
/// Print a nullptr to string
/// </summary>
/// <param name="s">Resulting string</param>
inline void PrintTo(nullptr_t /*p*/, baremetal::String& s)
{
    PrintStringTo(baremetal::String("null"), s);
}

/// <summary>
/// Universal printer class, using PrintTo()
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <typename T>
class UniversalPrinter
{
public:
    /// <summary>
    /// Print a type T to string
    /// </summary>
    /// <param name="value">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(const T& value, baremetal::String& s)
    {
        // By default, ::testing::internal::PrintTo() is used for printing
        // the value.
        //
        // Thanks to Koenig look-up, if T is a class and has its own
        // PrintTo() function defined in its namespace, that function will
        // be visible here.  Since it is more specific than the generic ones
        // in ::testing::internal, it will be picked by the compiler in the
        // following statement - exactly what we want.
        PrintTo(value, s);
    }
};

/// <summary>
/// Universal printer class for reference type, using PrintTo()
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <typename T>
class UniversalPrinter<T&>
{
public:
    /// <summary>
    /// Print a reference to string
    /// </summary>
    /// <param name="value">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(const T& value, baremetal::String& s)
    {
        // Prints the address of the value.  We use reinterpret_cast here
        // as static_cast doesn't compile when T is a function type.
        s = "@";
        s.append(baremetal::Serialize(reinterpret_cast<const void*>(&value)));
        s.append(" ");

        // Then prints the value itself.
        PrintTo(value, s);
    }
};

/// <summary>
/// Universal print to string function, uses UniversalPrinter
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
/// <param name="value">Value to print</param>
/// <param name="s">Resulting string</param>
template <typename T>
void UniversalPrint(const T& value, baremetal::String& s)
{
    typedef T T1;
    UniversalPrinter<T1>::Print(value, s);
}

/// <summary>
/// Universal terse printer class, uses UniversalPrint
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <typename T>
class UniversalTersePrinter
{
public:
    /// <summary>
    /// Print a type T to string
    /// </summary>
    /// <param name="value">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(const T& value, baremetal::String& s)
    {
        UniversalPrint(value, s);
    }
};
/// <summary>
/// Universal terse printer class for reference, uses UniversalPrint
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <typename T>
class UniversalTersePrinter<T&>
{
public:
    /// <summary>
    /// Print a reference to string
    /// </summary>
    /// <param name="value">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(const T& value, baremetal::String& s)
    {
        UniversalPrint(value, s);
    }
};
/// <summary>
/// Universal terse printer class for const char*, uses UniversalPrint
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <>
class UniversalTersePrinter<const char*>
{
public:
    /// <summary>
    /// Print a reference to string
    /// </summary>
    /// <param name="str">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(const char* str, baremetal::String& s)
    {
        if (str == nullptr)
        {
            s = "null";
        }
        else
        {
            UniversalPrint(baremetal::String(str), s);
        }
    }
};
/// <summary>
/// Universal terse printer class for char*, uses UniversalPrint
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
template <>
class UniversalTersePrinter<char*>
{
public:
    /// <summary>
    /// Print a reference to string
    /// </summary>
    /// <param name="str">Value to print</param>
    /// <param name="s">Resulting string</param>
    static void Print(char* str, baremetal::String& s)
    {
        UniversalTersePrinter<const char*>::Print(str, s);
    }
};

/// <summary>
/// String print, uses UniversalPrinter
/// </summary>
/// <typeparam name="T">Type value to print</typeparam>
/// <param name="value">Value to print</param>
/// <returns>Resulting string</returns>
template <typename T>
inline baremetal::String PrintToString(const T& value)
{
    baremetal::String s;
    UniversalTersePrinter<T>::Print(value, s);
    return s;
}
