//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
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

#include <baremetal/String.h>
#include <baremetal/Serialization.h>

using nullptr_t = decltype(nullptr);

template<typename To>
inline To ImplicitCast_(To x) { return x; }

template <typename T>
void PrintTo(const T& value, baremetal::string& s)
{
    s = baremetal::Serialize(value);
}
void PrintTo(unsigned char c, baremetal::string& s);
void PrintTo(signed char c, baremetal::string& s);
inline void PrintTo(char c, baremetal::string& s)
{
    PrintTo(static_cast<unsigned char>(c), s);
}

inline void PrintTo(bool x, baremetal::string& s)
{
    s = (x ? "true" : "false");
}

void PrintTo(const char* str, baremetal::string& s);
inline void PrintTo(char* str, baremetal::string& s)
{
    PrintTo(ImplicitCast_<const char*>(str), s);
}

// signed/unsigned char is often used for representing binary data, so
// we print pointers to it as void* to be safe.
inline void PrintTo(const signed char* str, baremetal::string& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
inline void PrintTo(signed char* str, baremetal::string& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
inline void PrintTo(const unsigned char* str, baremetal::string& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}
inline void PrintTo(unsigned char* str, baremetal::string& s)
{
    PrintTo(ImplicitCast_<const void*>(str), s);
}

void PrintStringTo(const baremetal::string& str, baremetal::string& s);
inline void PrintTo(const baremetal::string& str, baremetal::string& s)
{
    PrintStringTo(str, s);
}

inline void PrintTo(nullptr_t /*p*/, baremetal::string& s)
{
    PrintStringTo(baremetal::string("null"), s);
}

template <typename T>
class UniversalPrinter
{
public:
    // Note: we deliberately don't call this PrintTo(), as that name
    // conflicts with ::testing::internal::PrintTo in the body of the
    // function.
    static void Print(const T& value, baremetal::string& s)
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

// Implements printing a reference type T&.
template <typename T>
class UniversalPrinter<T&>
{
public:
    static void Print(const T& value, baremetal::string& s)
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

template <typename T>
void UniversalPrint(const T& value, baremetal::string& s)
{
    typedef T T1;
    UniversalPrinter<T1>::Print(value, s);
}

template <typename T>
class UniversalTersePrinter
{
public:
    static void Print(const T& value, baremetal::string& s)
    {
        UniversalPrint(value, s);
    }
};
template <typename T>
class UniversalTersePrinter<T&>
{
public:
    static void Print(const T& value, baremetal::string& s)
    {
        UniversalPrint(value, s);
    }
};
template <>
class UniversalTersePrinter<const char*>
{
public:
    static void Print(const char* str, baremetal::string& s)
    {
        if (str == nullptr)
        {
            s = "null";
        }
        else
        {
            UniversalPrint(baremetal::string(str), s);
        }
    }
};
template <>
class UniversalTersePrinter<char*>
{
public:
    // cppcheck-suppress constParameterPointer
    static void Print(char* str, baremetal::string& s)
    {
        UniversalTersePrinter<const char*>::Print(str, s);
    }
};

template <typename T>
inline baremetal::string PrintToString(const T& value)
{
    baremetal::string s;
    UniversalTersePrinter<T>::Print(value, s);
    return s;
}
